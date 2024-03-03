/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 **/

#ifdef __cplusplus
extern "C" {
#endif

#include "port.h"

#if defined ( __FPU_PRESENT )
#define FPU_ENABLED  __FPU_PRESENT
#else
#define FPU_ENABLED  0
#endif

/**
 * @brief ARM core SVC interrupt handle function.
 */
void SVC_Handler( void )
{
    /* Before call this the [R0-R3, R12, LR, PC, PSR] store into PSP */
    __asm volatile
    (
        "   .syntax unified                                 \n"
        "   .extern _impl_kernal_privilege_call_inSVC_c     \n"
        "                                                   \n"
        "   tst   lr, #4                                    \n" /* call from which stack pointer base on the bit 2 of EXC_RETURN (LR) */
        "   ite   eq                                        \n"
        "   mrseq r0, msp                                   \n" /* Set R0 = MSP */
        "   mrsne r0, psp                                   \n" /* Set R0 = PSP */
        "   b     _impl_kernal_privilege_call_inSVC_c       \n" /* call _impl_kernal_privilege_call_inSVC_c */
        "                                                   \n"
        "   .align 4                                        \n"
    );
    /**
     * return from exception, restoring {R0-R3, R12, LR, PC, PSR}
     */
}

/**
 * @brief ARM core PendSV interrupt handle function.
 */
void PendSV_Handler(void)
{
    /**
     * Save current context
     * Before call this the {R0-R3, R12, LR, PC, PSR} store into PSP
    */
    __asm volatile
    (
        "   .syntax unified                                 \n"
        "   .extern _impl_kernal_scheduler_inPendSV_c       \n"
        "                                                   \n"
        "    cpsid    i                                     \n"  /* Disable interrupts */
        "    isb                                            \n"
        "                                                   \n"
        /**
         * Call kernal_scheduler_inPendSV_c
         */
        "    push     {r0, r1, r12, lr}                     \n"
        "    mov      r0, sp                                \n" /* R0 points to the argument ppCurPsp  */
        "    add      r1, sp, #4                            \n" /* R1 points to the argument ppNextPSP */
        "    bl       _impl_kernal_scheduler_inPendSV_c     \n" /* Call _impl_kernal_scheduler_inPendSV_c */
        "    pop      {r0, r1, r12, lr}                     \n" /* R0 = ppCurPsp, R1 = ppNextPSP */
        "                                                   \n"
        "    cmp      r0, r1                                \n" /* if R0 = R1 */
        "    beq      exit                                  \n" /* Skip context switch */
        "                                                   \n"
        "    mrs      r2, psp                               \n" /* Get current process stack pointer value */
        "                                                   \n"
        #if ( FPU_ENABLED )                                  /* If the Cortex-M is not supported, the ASM instruction will not support VSTMDBEQ */
            "    tst      lr, 0x10                          \n" /* Test bit 4 of EXC_RETURN (0: FPU active on exception entry, 1: FPU not active) */
            "    it       eq                                \n" /* if (LR[4] == 0) */
            "    vstmdbeq r2!, {S16 -S31}                   \n" /* Save floating point registers, EQ suffix will save FPU registers {s16 - S31} */
            "                                               \n" /* if bit of LR was zero (S0-S15, FPSCR alread saved by MCU) */
            "    mrs      r3, control                       \n" /* Save CONTROL register in R3 to be pushed on stack - bit 2 (FPCA) indicates floating-point is active */
            "                                               \n"
            "    stmdb    r2!, {R3 - R11}                   \n" /* Save CONTROL, {R4 - R11} */
            "    stmdb    r2!, {LR}                         \n" /* Save EXC_RETURN in saved context */
        #else
            "    stmdb    r2!, {r4 - r11}                   \n" /* Save {R4 - R11} */
        #endif
        /**
         * Context switching code
         */
        "    str      r2, [r0]                              \n" /* *ppCurPSP = CurPSP */
        "    ldr      r2, [r1]                              \n" /* NextPSP = *pNextPSP */
        "                                                   \n"        
        #if ( FPU_ENABLED )                                     /* If the Cortex-M is not supported, the ASM instruction will not support VSTMDBEQ */
            "    ldmia    r2!, {lr}                         \n" /* restore LR */
            "    ldmia    r2!, {r3 - r11}                   \n" /* restore {R3 - R11} */
            "                                               \n"
            "    msr      control, r3                       \n" /* restore CONTROL register */
            "    isb                                        \n"
            "                                               \n" /* Save CONTROL register in R3 to be pushed on stack - bit 2 (FPCA) indicates floating-point is active */
            "    tst      lr, #0x10                         \n" /* Test bit 4. If zero, need to unstack floating point regs */
            "    it       eq                                \n"
            "    vldmiaeq r2!, {s16 - s31}                  \n" /* Load floating point registers */
        #else
            "    ldmia    r2!, {r4 - r11}                   \n" /* no FPU present - context is {R4 - R11} */
        #endif
        "                                                   \n"
        "    msr      psp, r2                               \n" /* Set PSP to next thread */    
        /**
         * End of Context switching code
         */
         "exit:                                              \n"
        "    cpsie    I                                     \n" /* Enable interrupts */
        "    isb                                            \n"
        "                                                   \n"
        "    bx       lr                                    \n" /* return from exception, restoring {R0 - R3, R12, LR, PC, PSR} */
        "                                                   \n"
        "    .align 4                                       \n"
        );
}

/**
 * @brief ARM core trigger the first thread to run.
 */
void _impl_port_run_theFirstThread(u32_t sp)
{
    /**
     * initialize R4-R11 from context frame using passed SP
     */
    __asm volatile
    (
        "   .syntax unified                                 \n"
        "                                                   \n"
        #if ( FPU_ENABLED )
            " ldmia   r0!, {r2 - r11}                       \n"         /* Context includes EXC_RETURN and CONTROL {R4 - R11} */
        #else
            " ldmia   r0!, {r4 - r11}                       \n"         /* Context {R4 - R11} */
        #endif
        "    msr   psp, r0                                  \n"         /* load PSP with what is now the current SP value */

        "    mov   r1, #3                                   \n"         /* set the CONTROL[SPSEL] bit to start using PSP (no FPU active, unpriviledged) */
        "    msr   control, r1                              \n"
        "    isb                                            \n"

        "    mov   lr, #0xFFFFFFFD                          \n"
        "    bx    lr                                       \n"         /* return to caller */
        "   .align 4                                        \n"
    );
    /**
     * return from exception, restoring {R0-R3, R12, LR, PC, PSR}
     */
}

#ifdef __cplusplus
}
#endif
