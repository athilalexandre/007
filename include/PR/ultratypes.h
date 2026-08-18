#ifndef _ULTRA64_TYPES_H_
#define _ULTRA64_TYPES_H_

#include <stddef.h>
#include <stdint.h>

#if defined(_LANGUAGE_C) || defined(_LANGUAGE_C_PLUS_PLUS)

typedef unsigned char               u8;  /* unsigned  8-bit */
typedef unsigned short              u16; /* unsigned 16-bit */
typedef unsigned int                u32; /* unsigned 32-bit */
typedef unsigned long long          u64; /* unsigned 64-bit */

typedef signed char                 s8;  /* signed  8-bit */
typedef short                       s16; /* signed 16-bit */
typedef int                         s32; /* signed 32-bit */
typedef long long                   s64; /* signed 64-bit */

typedef volatile unsigned char      vu8;  /* unsigned  8-bit */
typedef volatile unsigned short     vu16; /* unsigned 16-bit */
typedef volatile unsigned int       vu32; /* unsigned 32-bit */
typedef volatile unsigned long long vu64; /* unsigned 64-bit */

typedef volatile signed char        vs8;  /* signed  8-bit */
typedef volatile short              vs16; /* signed 16-bit */
typedef volatile int                vs32; /* signed 32-bit */
typedef volatile long long          vs64; /* signed 64-bit */

typedef float                       f32; /* single prec floating point */
typedef double                      f64; /* double prec floating point */

#endif /* _LANGUAGE_C */

#ifndef TRUE
#    define TRUE 1
#endif

#ifndef FALSE
#    define FALSE 0
#endif

#ifndef NULL
#    define NULL ((void *)0)
#endif

#ifdef TARGET_N64
    typedef u32 size_t;
    typedef s32 ssize_t;
    typedef u32 uintptr_t;
    typedef s32 intptr_t;
    typedef s32 ptrdiff_t;
#endif

#endif /* _ULTRA64_TYPES_H_ */