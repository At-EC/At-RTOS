/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 **/

#ifndef _EVENT_H_
#define _EVENT_H_

#include "kstruct.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The implement function lists for rtos kernel internal use.
 */
u32_t _impl_event_os_id_to_number(os_id_t id);
os_id_t _impl_event_init(u32_t edgeMask, u32_t clearMask, const char_t *pName);
u32p_t _impl_event_set(os_id_t id, u32_t set, u32_t clear, u32_t toggle);
u32p_t _impl_event_wait(os_id_t id, os_evt_val_t *pEvtData, u32_t desired_val, u32_t listen_mask, u32_t group_mask, u32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* _EVENT_H_ */
