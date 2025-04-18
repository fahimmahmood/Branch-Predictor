#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>
#include <algorithm>
#include <x86intrin.h>

/* Include the branch predictor definition here */
#include "../LLBP/bpmodels/base_predictor.h"
/* Include the branch predictor definition here */

using namespace std;

// Macros
//#define DEBUG 1

#define OOB_ARRAY_SIZE 16           // The out-of-bounds array size is arbitrary
#define CACHE_LINE_PADDING 512      // Modern caches have line width = 64 bytes; 512 is simply to guarantee no two hits are on the same line, since 512 >> 64
#define SECRET_SIZE 27              // Secret string length is also arbitrary

#define TRAINING_ITERATIONS_DEFAULT 16  // The number of iterations performed to train the branch predictor
#define CACHE_HIT_THRESHOLD_DEFAULT 40  // The default cache hit timing threshold in ticks
#define ATTEMPTS_DEFAULT 2500           // The number of successive attempts used to crack a single byte

// Global simulated memory and side channel instantiations
__attribute__((aligned(4096))) uint8_t side_channel_array[256 * CACHE_LINE_PADDING]; // evil gcc directive to (hopefully) keep cache lines page-aligned 

struct SimulatedROMemory {
    uint8_t OOB_array[OOB_ARRAY_SIZE];
    char secret[SECRET_SIZE];

    // Constructor
    SimulatedROMemory(char* secret) {
        // Initialize OOB_array
        for (int i = 0; i < OOB_ARRAY_SIZE; ++i)
            OOB_array[i] = i;

        // Initialize secret
        strcpy(this->secret, secret);
    }

    // read
    //      Returns the simulated read-only memory value stored at the given index (supporting OOB values relative to the OOB array)
    uint8_t read(size_t index) {
        uint8_t* mem_ptr = this->OOB_array;

        #ifdef DEBUG
            printf("[READ @%d : %c]", index, (char)mem_ptr[index]);
        #endif

        return mem_ptr[index];
    }
};
/* Change the secret string value here */
char original_secret[SECRET_SIZE] = "AAAAAAAAAAAAAAAAAAAAAAAAAA";
/* Change the secret string value here */

struct SimulatedROMemory mem(original_secret);

// leakSecretChar
//      Returns the character revealed to have been likely at the out-of-bounds offset index
//          OOB_index --> the out-of-bounds index
//          pred --> the branch predictor to test on
//          cache_hit_threshold --> the maximum cache latency to count as a hit
uint8_t leakSecretChar(size_t OOB_index, BasePredictor* pred, uint64_t cache_hit_threshold, int attempts, int training_iterations) {
    // Static pc value for the branch predictors
    // char pc[32];
    // sprintf(pc, "%lx", (uintptr_t)&leakSecretChar);
    uint64_t pc = reinterpret_cast<uint64_t>(&leakSecretChar);

    bool res;

    // Speculative execution variables
    volatile uint8_t dummy = 0;

    // Reload timing variables
    uint64_t t1, t2;
    unsigned int aux = 0;

    // Analysis variables
    int results[256];
    int indices[256];
    for (int i = 0; i < 256; i++)   results[i] = 0;
    int i, j, random_i;
    int first, second;

    for (int t = 0; t < attempts; t++) {

        // Randomize side channel access indices
        for (i = 0; i < 256; i++) indices[i] = i;
        for (i = 255; i > 0; i--) {
            j = rand() % (i + 1);
            swap(indices[i], indices[j]);
        }

        // (TRAIN) Train the predictor
        for (i = 0; i < training_iterations; i++) {
            res=pred->GetPrediction(pc);
            pred->UpdatePredictor(pc, true /*actual*/, res, 0); 
        }

        // (FLUSH) Flush the side channel from cache
        _mm_mfence(); // wait for current memory operations to complete
        for (i = 0; i < 256; i++) {
            _mm_clflush( &side_channel_array[i * CACHE_LINE_PADDING] );
        }
        _mm_mfence(); // wait for flush to complete

        // (SPECULATIVE EXEC) Simulate speculative execution access
        res=pred->GetPrediction(pc);
        pred->UpdatePredictor(pc, false /*actual*/, res, 0); 
        if (res) {
            // The predictor simulated a mispredicted "taken"; so, we force the speculative execution that follows
            //      (ignoring realistic bounds checking due to simulator limitations)
            dummy &= side_channel_array[mem.read(OOB_index) * CACHE_LINE_PADDING];
        }

        // (RELOAD) Time cache accesses to the side channel
        for (i = 0; i < 256; i++) {
            random_i = indices[i];

            t1 = __rdtscp(&aux); // Get timer start
            dummy &= side_channel_array[random_i * CACHE_LINE_PADDING]; // I love the compiler :)
            t2 = __rdtscp(&aux) - t1; // Get timer elapsed
        
            if ((t2 < cache_hit_threshold) && (t2 > 0)) {
                results[random_i]++;
            }
        }

        // Grab the top two counts to compare for early exit condition
        first = second = -1;
        for (i = 0; i < 256; i++) {
            if ((first < 0) || (results[i] >= results[first])) {
                second = first;
                first = i;
            } else if (second < 0 || results[i] >= results[second]) {
                second = i;
            }
        }

        // Early exit condition (referenced from: https://github.com/crozone/SpectrePoC ; made a huge difference in consistency)
        if (results[first] >= (2 * results[second] + 5) || (results[first] == 2 && results[second] == 0)) break;
    }

    #ifdef DEBUG
        sort(results, results + 256);
        printf("-----------------------------------\n");
        for (i = 0; i < 256; i++) {
            printf("%c\t:\t%d\n", i, results[i]);
        }
        printf("-----------------------------------\n");
    #endif

    // Return the most-likely leaked secret byte
    return first;
}

// stringDiff
//      Get the proportional difference between two equal-length strings
double stringDiff(char* s1, char* s2, int n) {
    int same = 0;
    for (int i=0; i<n; i++) {
        if (s1[i] == s2[i]) same++;
    }

    return (double)((100.0f * same) / n);
}

int main(int argc, char* argv[]) {
    printf("Running improved spectre attack sim on external LLBP or TAGE...\n");

    // Check argument parameters
    uint64_t cache_hit_threshold = CACHE_HIT_THRESHOLD_DEFAULT;
    int attempts = ATTEMPTS_DEFAULT;
    int training_iterations = TRAINING_ITERATIONS_DEFAULT;
    int executions = 1;
    if (argc == 5) {
        cache_hit_threshold = atoi(argv[1]);
        attempts = atoi(argv[2]);
        training_iterations = atoi(argv[3]);
        executions = atoi(argv[4]);
    } else if (argc < 5 && argc > 1) {
        printf("ERROR: bad usage: ./spectre_sim <cache hit threshold> <# attempts per byte> <# training iterations> <# executions>\n");
        return -1;
    }

    // Leak potential analysis variables
    double total_score = 0;

    /* Place the predictor instantiation here */
    BasePredictor* p = CreateBP("llbp");
    /* Place the predictor instantiation here */

    int ptr;
    char secret[SECRET_SIZE];
    uint8_t c;
    for (int t = 0; t < executions; t ++) {
        printf("ATTEMPT %d: -----------------------------------\n\n", t+1);

        delete p;
        p = CreateBP("llbp"); // Reset history tables
        ptr = OOB_ARRAY_SIZE; // Reset offset index

        // Try to leak each byte in the secret
        for (int i = 0; i < SECRET_SIZE - 1; i++) {
            c = leakSecretChar(ptr, p, cache_hit_threshold, attempts, (i+1)*training_iterations);
            secret[i] = (c > 31 && c < 127) ? c : ' ';
            ptr++;
        }
        secret[SECRET_SIZE - 1] = '\0';

        printf("  Leaked Secret: %s\n", secret);
        printf("Original Secret: %s\n\n", original_secret);

        total_score += stringDiff(secret, original_secret, SECRET_SIZE);
    }

    printf("Total leak average: %.2f%\n", total_score / executions);
    
    return 0;
}
