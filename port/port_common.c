/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 **/

#include "linker.h"
#include "clock_tick.h"
#include "port.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief ARM core systick interrupt handle function.
 */
void SysTick_Handler(void)
{
    clock_isr();
}

/**
 * @brief ARM core fault interrupt handle function.
 */
void HardFault_Handler(void)
{
    while (1) {};
}

/**
 * @brief To check if it's in interrupt content.
 */
b_t port_isInInterruptContent(void)
{
    if (__get_IPSR()) {
        return TRUE;
    }

    if (__get_PRIMASK() == SBIT(0)) {
        return TRUE;
    }

    return FALSE;
}

/**
 * @brief To check if it's in kernel thread content.
 */
b_t port_isInThreadMode(void)
{
    if (__get_IPSR()) {
        return FALSE;
    }
    return TRUE;
}

/**
 * @brief ARM core trigger the pendsv interrupt.
 */
void port_setPendSV(void)
{
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

/**
 * @brief ARM core config kernel thread interrupt priority.
 */
void port_interrupt_init(void)
{
    NVIC_SetPriority(PendSV_IRQn, 0xFFu); // Set PendSV to lowest possible priority
    NVIC_SetPriority(SVCall_IRQn, 0u); // Set SV to lowest possible priority
    NVIC_SetPriority(SysTick_IRQn, 0u);
}

/**
 * @brief Initialize a thread stack frame.
 *
 * @param pEntryFunction The entry function pointer.
 * @param pAddress The stack address.
 * @param size The stack size.
 *
 * @return The PSP stack address.
 */
u32_t port_stack_frame_init(void (*pEntryFunction)(void), u32_t *pAddress, u32_t size)
{
    os_memset((uchar_t *)pAddress, STACT_UNUSED_DATA, size);

    u32_t psp_frame = (u32_t)pAddress + size - sizeof(stack_snapshot_t);

    psp_frame = STACK_ADDRESS_DOWN(psp_frame);

    ((stack_snapshot_t *)psp_frame)->xPSR = SBIT(24);                /* xPSR */
    ((stack_snapshot_t *)psp_frame)->R15_PC = (u32_t)pEntryFunction; /* PC   */
    ((stack_snapshot_t *)psp_frame)->R14_LR = 0xFFFFFFFDu;           /* LR   */

    ((stack_snapshot_t *)psp_frame)->R12 = 0x12121212u; /* R12  */
    ((stack_snapshot_t *)psp_frame)->R3 = 0x03030303u;  /* R3   */
    ((stack_snapshot_t *)psp_frame)->R2 = 0x02020202u;  /* R2   */
    ((stack_snapshot_t *)psp_frame)->R1 = 0x01010101u;  /* R1   */
    ((stack_snapshot_t *)psp_frame)->R0 = 0x0B0B0B0Bu;  /* R0   */

#if defined(ARCH_ARM_CORTEX_CM0)
    ((stack_snapshot_t *)psp_frame)->cm0.R11 = 0x11111111u; /* R11  */
    ((stack_snapshot_t *)psp_frame)->cm0.R10 = 0x10101010u; /* R10  */
    ((stack_snapshot_t *)psp_frame)->cm0.R9 = 0x09090909u;  /* R9   */
    ((stack_snapshot_t *)psp_frame)->cm0.R8 = 0x08080808u;  /* R8   */
    ((stack_snapshot_t *)psp_frame)->cm0.R7 = 0x07070707u;  /* R7   */
    ((stack_snapshot_t *)psp_frame)->cm0.R6 = 0x06060606u;  /* R6   */
    ((stack_snapshot_t *)psp_frame)->cm0.R5 = 0x05050505u;  /* R5   */
    ((stack_snapshot_t *)psp_frame)->cm0.R4 = 0x04040404u;  /* R4   */
#else
    ((stack_snapshot_t *)psp_frame)->cmx.R11 = 0x11111111u; /* R11  */
    ((stack_snapshot_t *)psp_frame)->cmx.R10 = 0x10101010u; /* R10  */
    ((stack_snapshot_t *)psp_frame)->cmx.R9 = 0x09090909u;  /* R9   */
    ((stack_snapshot_t *)psp_frame)->cmx.R8 = 0x08080808u;  /* R8   */
    ((stack_snapshot_t *)psp_frame)->cmx.R7 = 0x07070707u;  /* R7   */
    ((stack_snapshot_t *)psp_frame)->cmx.R6 = 0x06060606u;  /* R6   */
    ((stack_snapshot_t *)psp_frame)->cmx.R5 = 0x05050505u;  /* R5   */
    ((stack_snapshot_t *)psp_frame)->cmx.R4 = 0x04040404u;  /* R4   */
#endif

#if (__FPU_PRESENT)
#if (THREAD_PSP_WITH_PRIVILEGED)
    ((stack_snapshot_t *)psp_frame)->CONTROL = SBIT(1); /* PSP with privileged */
#else
    ((stack_snapshot_t *)psp_frame)->CONTROL = SBIT(1) & SBIT(0); /* PSP with Unprivileged */
#endif

    ((stack_snapshot_t *)psp_frame)->EXC_RETURN = 0xFFFFFFFDu; /* EXC_RETURN */
#endif

    return (u32_t)psp_frame;
}

#ifdef __cplusplus
}
#endif
