/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
**/

#ifndef _SEMAPHORE_H_
#define _SEMAPHORE_H_

#include "kstruct.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The implement function lists for rtos kernal internal use.
 */
u32_t _impl_semaphore_os_id_to_number(os_id_t id);
os_id_t _impl_semaphore_init(u8_t initialCount, u8_t limitCount, b_t permit, const char_t *pName);
u32p_t _impl_semaphore_take(os_id_t id, u32_t timeout_ms);
u32p_t _impl_semaphore_give(os_id_t id);
u32p_t _impl_semaphore_flush(os_id_t id);

#ifdef __cplusplus
}
#endif

#endif /* _SEMAPHORE_H_ */
