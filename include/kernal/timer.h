/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
**/

#ifndef _TIMER_H_
#define _TIMER_H_

#include "kstruct.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The implement function lists for rtos kernal internal use.
 */
u32p_t _impl_kernal_timer_schedule_request(void);
void _impl_thread_timer_init(os_id_t id);
void _impl_thread_timer_start(os_id_t id, u32_t timeout_ms, void (*pCallback)(os_id_t));
u32_t _impl_timer_os_id_to_number(os_id_t id);
b_t _impl_timer_status_isBusy(os_id_t id);
os_id_t _impl_timer_init(pTimer_callbackFunc_t pCallFun, b_t isCycle, u32_t timeout_ms, const char_t *pName);
u32p_t _impl_timer_start(os_id_t id, b_t isCycle, u32_t timeout_ms);
u32p_t _impl_timer_stop(os_id_t id);
u32_t _impl_timer_total_system_get(void);
void _impl_timer_reamining_elapsed_handler(void);
void _impl_timer_elapsed_handler(u32_t elapsed_us);

#ifdef __cplusplus
}
#endif

#endif /* _TIMER_H_ */
