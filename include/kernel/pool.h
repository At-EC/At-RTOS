/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 **/

#ifndef _POOL_H_
#define _POOL_H_

#include "kstruct.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The implement function lists for rtos kernel internal use.
 */
u32_t _impl_pool_os_id_to_number(os_id_t id);
os_id_t _impl_pool_init(const void *pMemAddr, u16_t elementLen, u16_t elementNum, const char_t *pName);
u32p_t _impl_pool_take(os_id_t id, void **ppUserBuffer, u16_t bufferSize, u32_t timeout_ms);
u32p_t _impl_pool_release(os_id_t id, void **ppUserBuffer);

#ifdef __cplusplus
}
#endif

#endif /* _POOL_H_ */
