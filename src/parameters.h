//#define DEBUG_HIT_THRESHOLD
//#define DEBUG_MEMORY_ACCESS

#define OOB_ARRAY_SIZE 16           // The out-of-bounds array size is arbitrary
#define CACHE_LINE_PADDING 512      // Modern caches have line width = 64 bytes; 512 is simply to guarantee no two hits are on the same line, since 512 >> 64
#define SECRET_SIZE 27              // Secret string length is also arbitrary

#define TRAINING_ITERATIONS_DEFAULT 16  // The number of iterations performed to train the branch predictor
#define CACHE_HIT_THRESHOLD_DEFAULT 60  // The default cache hit timing threshold in ticks
#define ATTEMPTS_DEFAULT 2500           // The number of successive attempts used to crack a single byte