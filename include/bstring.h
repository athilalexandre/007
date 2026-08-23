#ifndef __BSTRING_H__
#define __BSTRING_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

extern void    bcopy(const void *, void *, size_t);
extern int     bcmp(const void *, const void *, size_t);
extern void    bzero(void *, size_t);

#ifdef __cplusplus
}
#endif
#endif /* !__BSTRING_H__ */
