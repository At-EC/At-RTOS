/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 **/
#ifndef _SCHED_TIMER_H_
#define _SCHED_TIMER_H_

#include "k_struct.h"

void timeout_set(struct expired_time *pExpire, _u32_t timeout_ms, _b_t immediately);
void timeout_remove(struct expired_time *pExpire, _b_t immediately);
_u32_t timer_total_system_ms_get(void);
_u32_t timer_total_system_us_get(void);
_i32p_t timer_schedule(void);
void timer_reamining_elapsed_handler(void);
void timeout_handler(_u32_t elapsed_us);
void timeout_init(struct expired_time *pExpire, pTimeout_callbackFunc_t fun);

#endif /* _SCHED_TIMER_H_ */
