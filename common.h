#include "types.h"
#include <string.h>

#define KB(Value) ((Value)*1024LL)
#define MB(Value) (KB(Value)*1024LL)
#define GB(Value) (MB(Value)*1024LL)
#define TB(Value) (GB(Value)*1024LL)

#define array_count(array) (sizeof(array) / sizeof((array)[0]))

#define assert(expression) if(!(expression)) {*(volatile int *)0 = 0;}

// NOTE(partkyle): taken from
// http://stackoverflow.com/questions/3982348/implement-generic-swap-macro-in-c
#define swap(x,y) do \
   { unsigned char swap_temp[sizeof(x) == sizeof(y) ? (signed)sizeof(x) : -1]; \
     memcpy(swap_temp,&y,sizeof(x)); \
     memcpy(&y,&x,       sizeof(x)); \
     memcpy(&x,swap_temp,sizeof(x)); \
    } while(0)
