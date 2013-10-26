#pragma once 

#include <nmmintrin.h>

using namespace std;

static inline int BitCount32(unsigned int i)
{
	i = i - ((i >> 1) & 0x55555555U);
	i = (i & 0x33333333U) + ((i >> 2) & 0x33333333U);
	i = (i + (i >> 4)) & 0x0f0f0f0f;
	i = i + (i >> 8);
	i = i + (i >> 16);
	return (int)(i & 0x3fU);
}

static inline int BitCount16_2(unsigned int a,unsigned int b)
{
	a = a - ((a >> 1) & 0x55555555U);
	a = (a & 0x3333U) + ((a >> 2) & 0x3333U);
	a = (a + (a >> 4)) & 0x0f0f;
	b = b - ((b >> 1) & 0x55555555U);
	b = (b & 0x3333U) + ((b >> 2) & 0x3333U);
	b = (b + (b >> 4)) & 0x0f0f;
	a = a + b;
	a = a + (a >> 8);
	return (int)(a & 0x3fU);
}

#ifdef SSE4_2
inline int MY_POPCOUNT(unsigned int v){return _mm_popcnt_u32(v);}
#else
inline int MY_POPCOUNT(unsigned int v){return BitCount32(v);}
#endif
