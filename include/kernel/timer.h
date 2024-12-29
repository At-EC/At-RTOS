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
void timeout_set(struct expired_time *pExpire, u32_t timeout_ms, b_t immediately);
void timeout_remove(struct expired_time *pExpire, b_t immediately);
b_t timer_busy(os_id_t id);
u32_t timer_total_system_ms_get(void);
u32_t timer_total_system_us_get(void);
i32p_t timer_schedule(void);
void timer_reamining_elapsed_handler(void);
void timeout_handler(u32_t elapsed_us);
void timeout_init(struct expired_time *pExpire, pTimeout_callbackFunc_t fun);

#ifdef __cplusplus
}
#endif

#endif /* _TIMER_H_ */
