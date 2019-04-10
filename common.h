#ifndef COMMON_H
#define COMMON_H
#include "types.h"
#include <string.h>

#define KB(Value) ((Value)*1024LL)
#define MB(Value) (KB(Value)*1024LL)
#define GB(Value) (MB(Value)*1024LL)
#define TB(Value) (GB(Value)*1024LL)

#define array_count(array) (sizeof(array) / sizeof((array)[0]))

#define assert(expression) if(!(expression)) {*(volatile int *)0 = 0;}

#include <algorithm>


#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#endif