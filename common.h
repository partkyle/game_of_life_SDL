#include "types.h"

#define KB(Value) ((Value)*1024LL)
#define MB(Value) (KB(Value)*1024LL)
#define GB(Value) (MB(Value)*1024LL)
#define TB(Value) (GB(Value)*1024LL)

#define array_count(array) (sizeof(array) / sizeof((array)[0]))

#define assert(expression) if(!(expression)) {*(volatile int *)0 = 0;}

#define swap(A, B) \
do { \
    typeof((A)) _tmp = (A); \
    (A) = (B); \
    (B) = _tmp; \
} while(0)
