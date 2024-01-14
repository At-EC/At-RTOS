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

/**
 * @brief ARM core SVC interrupt handle function.
 */
__ASM void SVC_Handler(void)
{
    /* Before call this the [R0-R3, R12, LR, PC, PSR] store into PSP */
    TST   LR, #0x04                                             /* call from which stack pointer base on the bit 2 of EXC_RETURN (LR) */
    ITE   EQ
    MRSEQ R0, MSP                                               /* Set R0 = MSP */
    MRSNE R0, PSP                                               /* Set R0 = PSP */

    B     __cpp(_impl_kernal_privilege_call_inSVC_c)            /* call _impl_kernal_privilege_call_inSVC_c */

    /**
     * return from exception, restoring {R0-R3, R12, LR, PC, PSR}
     */
    ALIGN 4
}

/**
 * @brief ARM core PendSV interrupt handle function.
 */
__ASM void PendSV_Handler(void)
{
    /**
     * Save current context
     * Before call this the {R0-R3, R12, LR, PC, PSR} store into PSP
    */
    CPSID    I                                                   /* Disable interrupts */
    ISB


    /**
     * Call kernal_scheduler_inPendSV_c
     */
    PUSH     {R0, R1, R12, LR}
    MOV      R0, SP                                              /* R0 points to the argument ppCurPsp  */
    ADD      R1, SP, #4                                          /* R1 points to the argument ppNextPSP */
    BL       __cpp(_impl_kernal_scheduler_inPendSV_c)            /* Call _impl_kernal_scheduler_inPendSV_c */
    POP      {R0, R1, R12, LR}                                   /* R0 = ppCurPsp, R1 = ppNextPSP */

    CMP      R0, R1                                              /* if R0 = R1 */
    BEQ      Exit                                                /* Skip context switch */

    MRS      R2, PSP                                             /* Get current process stack pointer value */

#if defined FPU_ENABLED                                          /* If the Cortex-M is not supported, the ASM instruction will not support VSTMDBEQ */
    TST      LR, #0x10                                           /* Test bit 4 of EXC_RETURN (0: FPU active on exception entry, 1: FPU not active) */
    IT       EQ                                                  /* if (LR[4] == 0) */
    VSTMDBEQ R2!, {S16 - S31}                                    /* Save floating point registers, EQ suffix will save FPU registers {s16 - S31} */
                                                                 /* if bit of LR was zero (S0-S15, FPSCR alread saved by MCU) */
    MRS      R3, CONTROL                                         /* Save CONTROL register in R3 to be pushed on stack - bit 2 (FPCA) indicates floating-point is active */

    STMDB    R2!, {R3 - R11}                                     /* Save CONTROL, {R4 - R11} */
    STMDB    R2!, {LR}                                           /* Save EXC_RETURN in saved context */
#else
    STMDB    R2!, {R4 - R11}                                    /* Save {R4 - R11} */
#endif

    /**
     * Context switching code
    */
    STR      R2, [R0]                                            /* *ppCurPSP = CurPSP */
    LDR      R2, [R1]                                            /* NextPSP = *pNextPSP */

#if defined FPU_ENABLED
    LDMIA    R2!, {LR}                                           /* restore LR */
    LDMIA    R2!, {R3 - R11}                                     /* restore {R3 - R11} */

    MSR      CONTROL, R3                                         /* restore CONTROL register */
    ISB

    TST      LR, #0x10                                           /* Test bit 4. If zero, need to unstack floating point regs */
    IT       EQ
    VLDMIAEQ R2!, {S16 - S31}                                    /* Load floating point registers */
#else
    LDMIA    R2!, {R4 - R11}                                     /* no FPU present - context is {R4 - R11} */
#endif

    MSR      PSP, R2                                             /* Set PSP to next thread */

    /**
     * End of Context switching code
     */
Exit
    CPSIE       I                                                /* Enable interrupts */
    ISB

    BX       LR                                                  /* return from exception, restoring {R0 - R3, R12, LR, PC, PSR} */

    ALIGN 4
}

/**
 * @brief ARM core trigger the first thread to run.
 */
__ASM void _impl_port_run_theFirstThread(u32_t sp)
{
    /**
     * initialize R4-R11 from context frame using passed SP
     */
#if defined FPU_ENABLED
    LDMIA   R0!, {R2 - R11}                                     /* Context includes EXC_RETURN and CONTROL {R4 - R11} */
#else
    LDMIA   R0!, {R4 - R11}                                     /* Context {R4 - R11} */
#endif

    /* MOV     R2, #0xFFFFFFFD */

    MSR     PSP, R0                                             /* load PSP with what is now the current SP value */

    MOV     R1, #3                                              /* set the CONTROL[SPSEL] bit to start using PSP (no FPU active, unpriviledged) */
    MSR     CONTROL, R1
    ISB

    /* STR     R2, [LR] */                                      /* Set EXC_RETURN */
    MOV     LR, #0xFFFFFFFD
    BX      LR                                                  /* return to caller */

    ALIGN 4
}

#ifdef __cplusplus
}
#endif
