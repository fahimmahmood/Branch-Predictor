#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>
#include <algorithm>
#include <x86intrin.h>

/* Include the branch predictor definition here */
#include "./predictor/smith.cpp"
#include "./predictor/bimodal.cpp"
#include "./predictor/gshare.cpp"
#include "./predictor/hybrid.cpp"

#include "../LLBP/bpmodels/base_predictor.h"
/* Include the branch predictor definition here */

// Macros
#include "parameters.h"

using namespace std;

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

        #ifdef DEBUG_MEMORY_ACCESS
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
#include "./spectre_type_variants.cpp"

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

    // Check argument parameters or use defaults
    char predictor = 's';
    uint64_t cache_hit_threshold = CACHE_HIT_THRESHOLD_DEFAULT;
    int attempts = ATTEMPTS_DEFAULT;
    int training_iterations = TRAINING_ITERATIONS_DEFAULT;
    char improved = 'n';
    int runs = 1;

    if (argc == 7) {
        /* Predictors */
        // smith    --> s
        // bimodal  --> b
        // gshare   --> g
        // hybrid   --> h
        // tage     --> t
        // llbp     --> l
        predictor = (argv[1][0] < 97) ? (argv[1][0] + 32) : argv[1][0];
        cache_hit_threshold = atoi(argv[2]);
        attempts = atoi(argv[3]);
        training_iterations = atoi(argv[4]);
        improved = argv[5][0];
        runs = atoi(argv[6]);
    } else if (argc < 7 && argc > 1) {
        printf("ERROR: bad usage: ./spectre_sim <predictor> <cache hit threshold> <# attempts per byte> <# training iterations> <improved training (y/n)> <# runs> \n");
        printf("Default Usage: ./spectre_sim <smith> <40> <2500> <16> <n> <1>\n");
        printf("List of Predictors:\n- (s)mith\n- (b)imodal\n- (g)share\n- (h)ybrid\n- (t)age\n- (l)lbp\n");
        return -1;
    }

    // Confirm the configuration to the user
    printf("-----------------------------------------------\n");
    char hrp[32];
    switch (predictor) {
        case 's':
            sprintf(hrp, "Smith (4)"); break;
        case 'b':
            sprintf(hrp, "Bimodal (4)"); break;
        case 'g':
            sprintf(hrp, "GShare (11, 5)"); break;
        case 'h':
            sprintf(hrp, "Hybrid (8, 14, 10, 5)"); break;
        case 't':
            sprintf(hrp, "Tage 64k (external)"); break;
        case 'l':
            sprintf(hrp, "LLBP (external)"); break;
        default:
            printf("ERROR: invalid predictor\n");
            printf("List of Predictors:\n- (s)mith\n- (b)imodal\n- (g)share\n- (h)ybrid\n- (t)age\n- (l)lbp\n");
            return -1;
    }
    printf("Running simulator with:\n\tBranch Predictor = %s\n\tCache Hit Threshold = %d ticks\n\tAttempts per Byte = %d\n\tTraining Iterations = %d\n\tImproved Training = %c\n\tConsecutive Runs = %d\n", hrp, cache_hit_threshold, attempts, training_iterations, improved, runs);
    printf("-----------------------------------------------\n");

    // Leak analysis variables
    double total_score = 0;

    // Leak execution variables
    void* p; // generic instantiation of the predictor
    int ptr; // the memory offset we want to look at
    char secret[SECRET_SIZE]; // the set of iteratively leaked characters
    uint8_t c; // the character leaked in an iteration
    int temp; // the actual number of training iterations (improved/not)

    // Begin running the experiments
    for (int t = 0; t < runs; t ++) {
        printf("SPECTRE RUN %d: --------------------------------\n\n", t+1);

        // Instantiate the chosen predictor (and reinitialize it every run to prevent history from carrying over)
        if ( t > 0 ) delete p;
        switch (predictor) {
            case 's':
                p = (NBitSmithPredictor*)new NBitSmithPredictor(4); break;
            case 'b':
                p = (BimodalPredictor*)new BimodalPredictor(4); break;
            case 'g':
                p = (GSharePredictor*)new GSharePredictor(11, 5); break;
            case 'h':
                p = (HybridPredictor*)new HybridPredictor(8, 14, 10, 5); break;
            case 't':
                p = (BasePredictor*)CreateBP("tage64k"); break;
            case 'l':
                p = (BasePredictor*)CreateBP("llbp"); break;
            default:
                printf("ERROR: invalid predictor\n");
                printf("List of Predictors:\n- (s)mith\n- (b)imodal\n- (g)share\n- (h)ybrid\n- (t)age\n- (l)lbp\n");
                return -1;
        }

        ptr = OOB_ARRAY_SIZE; // Reset memory offset index
        // Try to leak each byte in the secret memory
        for (int i = 0; i < SECRET_SIZE - 1; i++) {
            // Improved training iterations
            switch (improved) {
                case 'y':
                    temp = (i+1)*training_iterations; break;
                default:
                    temp = training_iterations; break;
            }

            // Call the overload of the chosen predictor
            switch (predictor) {
                case 's':
                    c = leakSecretChar(ptr, (NBitSmithPredictor*)p, cache_hit_threshold, attempts, temp); break;
                case 'b':
                    c = leakSecretChar(ptr, (BimodalPredictor*)p, cache_hit_threshold, attempts, temp); break;
                case 'g':
                    c = leakSecretChar(ptr, (GSharePredictor*)p, cache_hit_threshold, attempts, temp); break;
                case 'h':
                    c = leakSecretChar(ptr, (HybridPredictor*)p, cache_hit_threshold, attempts, temp); break;
                case 't':
                    c = leakSecretChar(ptr, (BasePredictor*)p, cache_hit_threshold, attempts, temp); break;
                case 'l':
                    c = leakSecretChar(ptr, (BasePredictor*)p, cache_hit_threshold, attempts, temp); break;
            }
            
            // If the secret isn't keyboard-typeable, then just print a space
            secret[i] = (c > 31 && c < 127) ? c : ' ';
            ptr++;
        }
        secret[SECRET_SIZE - 1] = '\0';

        printf("  Leaked Secret: %s\n", secret);
        printf("Original Secret: %s\n\n", original_secret);

        total_score += stringDiff(secret, original_secret, SECRET_SIZE);
    }

    printf("Average leak accuracy: %.2f%\n", total_score / runs);
    
    return 0;
}
