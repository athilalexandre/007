#ifndef _HAL_AUDIO_H_
#define _HAL_AUDIO_H_

#include <ultra64.h>

#ifdef __cplusplus
extern "C" {
#endif

void hal_audio_init(void);
void hal_audio_step(void);

#ifdef __cplusplus
}
#endif

#endif /* _HAL_AUDIO_H_ */