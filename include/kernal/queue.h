/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
**/

#ifndef _QUEUE_H_
#define _QUEUE_H_

#include "kstruct.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The implement function lists for rtos kernal internal use.
 */
u32_t _impl_queue_os_id_to_number(os_id_t id);
os_id_t _impl_queue_init(const void *pQueueBufferAddr, u16_t elementLen, u16_t elementNum, const char_t *pName);
u32p_t _impl_queue_send(os_id_t id, const u8_t *pUserBuffer, u16_t bufferSize, u32_t timeout_ms);
u32p_t _impl_queue_receive(os_id_t id, const u8_t *pUserBuffer, u16_t bufferSize, u32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* _QUEUE_H_ */
