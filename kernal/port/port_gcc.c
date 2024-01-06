/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "clock_systick.h"
#include "kernal.h"
#include "basic.h"
#include "compiler.h"

void SysTick_Handler(void)
{
    _impl_clock_isr();
}

void HardFault_Handler(void)
{
   /* TODO */
}

void SVC_Handler(void)
{
   /* TODO */
}

void PendSV_Handler(void)
{
   /* TODO */
}

void kernal_run_theFirstThread(u32_t sp)
{
   /* TODO */
}

#ifdef __cplusplus
}
#endif
