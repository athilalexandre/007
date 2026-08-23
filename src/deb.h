#ifndef _DEB_H_
#define _DEB_H_

#include <stddef.h>
#include <ultra64.h>

void debInit(void);
void debTryAdd(void *data, const char *name);

#endif
