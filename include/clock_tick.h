/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
**/

#ifndef _CLOCK_TICK_H_
#define _CLOCK_TICK_H_

#include "typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Function pointer structure for the clock tells how much time has passed.
 */
typedef void (*time_report_handler_t)(u32_t);

/**
 * The implement function lists for rtos kernal internal use.
 */
void _impl_clock_isr(void);
void _impl_clock_time_interval_set(u32_t interval_us);
u32_t _impl_clock_time_elapsed_get(void);
u32_t _impl_clock_time_get(void);
void _impl_clock_time_enable(void);
void _impl_clock_time_disable(void);
void _impl_clock_time_init(time_report_handler_t pTime_function);

#ifdef __cplusplus
}
#endif

#endif /* _CLOCK_TICK_H_ */
