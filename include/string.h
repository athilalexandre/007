#ifndef _LOCAL_STRING_H_
#define _LOCAL_STRING_H_

#ifdef TARGET_WEB
#include_next <string.h>
#else
#include <PR/ultratypes.h>

extern void *memcpy(void *, const void *, size_t);
extern unsigned char *strchr(const unsigned char *, int);
extern size_t strlen(const unsigned char *);
#endif

#endif