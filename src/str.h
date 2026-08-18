#ifndef _STR_H_
#define _STR_H_

#include <ultra64.h>

#ifndef _SIZE_T_DEFINED
#define _SIZE_T_DEFINED
typedef unsigned long size_t;
#endif

char *strcpy(char *dst, const char *src);
char *strncpy(char *dst, const char *src, size_t n);
char *strcat(char *dst, const char *src);
int strcmp(const char* str1, const char* str2);
int strncmp(const char *str1, const char *str2, size_t n);
long int strtol(const char *str, char **endptr, int base);

#endif