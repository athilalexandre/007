#ifndef _HAL_INPUT_H_
#define _HAL_INPUT_H_

#include <ultra64.h>

#ifdef __cplusplus
extern "C" {
#endif

void hal_input_init(void);
void hal_input_set_buttons(u16 buttons, s8 stickX, s8 stickY);
u16 hal_input_get_buttons(void);
s8 hal_input_get_stick_x(void);
s8 hal_input_get_stick_y(void);

#ifdef __cplusplus
}
#endif

#endif /* _HAL_INPUT_H_ */