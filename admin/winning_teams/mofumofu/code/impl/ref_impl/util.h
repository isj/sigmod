#pragma once


//#define PROFILING_MODE

#ifdef PROFILING_MODE
#define MY_INLINE 

#else

#ifdef  __GNUC__
#define MY_INLINE inline  __attribute__(( always_inline)) 
#else
#define MY_INLINE inline
#endif

#endif


#ifdef  __GNUC__
#define ALWAYS_INLINE inline  __attribute__(( always_inline)) 
#else
#define ALWAYS_INLINE inline
#endif

#ifdef _MSC_VER

#define MY_ASSUME __assume(0)
#define ALIGNED(x) __declspec(align(x))


#ifndef _SSIZE_T_DEFINED
#ifdef _WIN64
typedef long long int ssize_t;
#else
typedef int ssize_t;
#endif
#define _SSIZE_T_DEFINED

#endif

#endif



#ifdef  __GNUC__
#define likely(x)	__builtin_expect(!!(x), 1)
#define unlikely(x)	__builtin_expect(!!(x), 0)
#define MY_ASSUME assert(0)
#define ALIGNED(x) __attribute__ ((aligned (x)))

#include <stdlib.h>
static inline void *_aligned_malloc(size_t size, size_t alignment)
{
    void *p;
    int ret = posix_memalign(&p, alignment, size);
    return (ret == 0) ? p : 0;
}

static inline void _aligned_free(void * mem){
	free(mem);
}
#    define bsf(x) __builtin_ctz(x)

#else
#define likely(x)	x
#define unlikely(x)	x

#    define bsf(x) (_BitScanForward((unsigned long*)(&x), x), x)

#endif

