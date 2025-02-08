/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 **/
#ifndef _K_CLOCK_TICK_H_
#define _K_CLOCK_TICK_H_

#include "type_def.h"

/**
 * Function pointer structure for the clock tells how much time has passed.
 */
typedef void (*time_report_handler_t)(_u32_t);

/**
 * The implement function lists for rtos kernel internal use.
 */
void clock_isr(void);
void clock_time_interval_set(_u32_t interval_us);
_u32_t clock_time_elapsed_get(void);
_u32_t clock_time_get(void);
void clock_time_enable(void);
void clock_time_disable(void);
void clock_time_init(time_report_handler_t pTime_function);

#endif /* _K_CLOCK_TICK_H_ */
