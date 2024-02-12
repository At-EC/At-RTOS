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

/**
 * @brief ARM core systick interrupt handle function.
 */
void SysTick_Handler(void)
{
    _impl_clock_isr();
}

/**
 * @brief ARM core fault interrupt handle function.
 */
void HardFault_Handler(void)
{
    while(1){};
}

/**
 * @brief To check if it's in interrupt content.
 */
b_t _impl_port_isInInterruptContent(void)
{
    if (__get_IPSR())
    {
        return TRUE;
    }

    if (__get_PRIMASK() == SBIT(0))
    {
        return TRUE;
    }

    return FALSE;
}

/**
 * @brief To check if it's in kernal thread content.
 */
b_t _impl_port_isInThreadMode(void)
{
    if (__get_IPSR())
    {
        return FALSE;
    }
    return TRUE;
}

/**
 * @brief ARM core trigger the pendsv interrupt.
 */
void _impl_port_setPendSV(void)
{
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

/**
 * @brief ARM core config kernal thread interrupt priority.
 */
void _impl_port_interrupt_init(void)
{
    NVIC_SetPriority(PendSV_IRQn, 0xFF); // Set PendSV to lowest possible priority
    NVIC_SetPriority(SVCall_IRQn, 0xFF); // Set SV to lowest possible priority
    NVIC_SetPriority(SysTick_IRQn, 0xFFu);
}

#ifdef __cplusplus
}
#endif
