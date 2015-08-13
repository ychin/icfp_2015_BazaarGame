// HoneyRunner Precompile.h
// Stuff from XEngine we need for command-line app

#include <string.h>
#include <malloc.h>
#include <ctype.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include "windows.h"

typedef unsigned int   uint32;
typedef unsigned long long uint64;
typedef          long long int64_t;

#include "Random.h"
#include "Vec2I.h"
#include "Vec3I.h"

#define XHalt() exit(0);
#define XAssert assert
#define XTrace(...) /*nothing*/

#define XMalloc malloc
#define XFree free

inline int Min(int x, int y) {
	return (x < y) ? x : y;
}
inline int Max(int x, int y) {
	return (x < y) ? y : x;
}

inline float Clamp(float t, float min, float max)
{
	XAssert(min <= max);
	if (t < min) return min;
	if (t > max) return max;
	return t;
}



