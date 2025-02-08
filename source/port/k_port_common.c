/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 **/
#include "./port/k_port.h"
#include "./clock/k_clock_tick.h"
#include "k_linker.h"

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
_b_t port_isInInterruptContent(void)
{
    if (__get_IPSR()) {
        return true;
    }

    if (__get_PRIMASK() == B(0)) {
        return true;
    }

    return false;
}

/**
 * @brief To check if it's in kernel thread content.
 */
_b_t port_isInThreadMode(void)
{
    if (__get_IPSR()) {
        return false;
    }
    return true;
}

_u32_t port_irq_disable(void)
{
    _u32_t value = __get_PRIMASK();
    __disable_irq();
    return value;
}

void port_irq_enable(_u32_t value)
{
    __set_PRIMASK(value);
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
    NVIC_SetPriority(SVCall_IRQn, 0u);    // Set SV to lowest possible priority
    NVIC_SetPriority(SysTick_IRQn, 0u);
}

/**
 * @brief Initialize a thread stack frame.
 *
 * @param pEntryFn The entry function pointer.
 * @param pAddress The stack address.
 * @param size The stack size.
 *
 * @return The PSP stack address.
 */
_u32_t port_stack_frame_init(void (*pEntryFn)(void *), _u32_t *pAddress, _u32_t size, void *pArg)
{
    extern void _impl_thread_entry(void (*pEntryFn)(void *), void *pArg);

    os_memset((_uchar_t *)pAddress, STACT_UNUSED_DATA, size);

    _u32_t psp_frame = (_u32_t)pAddress + size - sizeof(stack_snapshot_t);

    psp_frame = STACK_ADDRESS_DOWN(psp_frame);

    ((stack_snapshot_t *)psp_frame)->xPSR = B(24);                        /* xPSR */
    ((stack_snapshot_t *)psp_frame)->R15_PC = (_u32_t)_impl_thread_entry; /* PC   */
    ((stack_snapshot_t *)psp_frame)->R15_PC &= 0xFFFFFFFEu;               /* ARM mode: Clear LSB of address */
    ((stack_snapshot_t *)psp_frame)->R14_LR = 0xFFFFFFFDu;                /* LR   */

    ((stack_snapshot_t *)psp_frame)->R12 = 0x12121212u;     /* R12  */
    ((stack_snapshot_t *)psp_frame)->R3 = 0x03030303u;      /* R3   */
    ((stack_snapshot_t *)psp_frame)->R2 = 0x02020202u;      /* R2   */
    ((stack_snapshot_t *)psp_frame)->R1 = (_u32_t)pArg;     /* R1   */
    ((stack_snapshot_t *)psp_frame)->R0 = (_u32_t)pEntryFn; /* R0   */

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
    ((stack_snapshot_t *)psp_frame)->CONTROL = Bs(1); /* PSP with privileged */
#else
    ((stack_snapshot_t *)psp_frame)->CONTROL = Bs(1) & Bs(0); /* PSP with Unprivileged */
#endif

    ((stack_snapshot_t *)psp_frame)->EXC_RETURN = 0xFFFFFFFDu; /* EXC_RETURN */
#endif

    return (_u32_t)psp_frame;
}

/**
 * @brief Get unused stack size.
 *
 * @param pAddress The stack address.
 *
 * @return The free stack size.
 */
_u32_t port_stack_free_size_get(_u32_t stack_addr)
{
    if (stack_addr == 0) {
        return 0u;
    }

    _u8_t *ptr = (_u8_t *)stack_addr;
    while (*ptr == STACT_UNUSED_DATA) {
        ptr++;
    }

    return (_u32_t)ptr - (_u32_t)stack_addr;
}
