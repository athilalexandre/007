#ifndef _STDDEF_H_
#define _STDDEF_H_

#ifndef NULL
#define NULL ((void *)0)
#endif

#ifndef _SIZE_T_DEFINED
#define _SIZE_T_DEFINED
typedef unsigned long size_t;
#endif

#ifndef _PTRDIFF_T_DEFINED
#define _PTRDIFF_T_DEFINED
typedef long ptrdiff_t;
#endif

#ifndef offsetof
#define offsetof(s, m) ((size_t)&(((s *)0)->m))
#endif

#endif