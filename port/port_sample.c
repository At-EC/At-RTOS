/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "clock_tick.h"
#include "kernal.h"
#include "basic.h"
#include "compiler.h"
#include "port.h"

u32_t _impl_kernal_svc_call(u32_t args_0, u32_t args_1, u32_t args_2, u32_t arg_3)
{
    /* TODO */
    return 0u;
}

void port_setPendSV(void)
{
    /* TODO */
}

b_t port_isInInterruptContent(void)
{
    /* TODO */
    return FALSE;
}

b_t port_isInThreadMode(void)
{
    /* TODO */
    return FALSE;
}

void port_interrupt_init(void)
{
    /* TODO */
}

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
