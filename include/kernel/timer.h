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
 * The implement function lists for rtos kernel internal use.
 */
void timer_init_for_thread(os_id_t id);
void timer_start_for_thread(os_id_t id, u32_t timeout_ms, void (*pCallback)(os_id_t));
u32p_t timer_stop_for_thread(os_id_t id);
b_t timer_busy(os_id_t id);
u32_t timer_total_system_ms_get(void);
u32_t timer_total_system_us_get(void);
u32p_t timer_schedule(void);
void timer_reamining_elapsed_handler(void);
void timer_elapsed_handler(u32_t elapsed_us);

#ifdef __cplusplus
}
#endif

#endif /* _TIMER_H_ */
