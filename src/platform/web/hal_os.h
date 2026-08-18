#ifndef _HAL_OS_H_
#define _HAL_OS_H_

#include <ultra64.h>

#ifdef __cplusplus
extern "C" {
#endif

void hal_os_set_rom_data(const u8 *data, size_t size);
u32 hal_os_get_rom_status(void);
size_t hal_os_get_rom_size(void);
const u8 *hal_os_get_rom_buffer(void);
void hal_os_reset(void);

#ifdef __cplusplus
}
#endif

#endif /* _HAL_OS_H_ */