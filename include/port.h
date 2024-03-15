/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 **/

#ifndef _PORT_H_
#define _PORT_H_

#include "typedef.h"
#include "arch.h"

#ifdef __cplusplus
extern "C" {
#endif

#define STACT_UNUSED_DATA       (0xDEu)
#define STACT_UNUSED_FRAME_MARK (0xDEDEDEDEu)

#define STACK_ADDRESS_UP(address)   (u32_t)(ROUND_UP((address), STACK_ALIGN))
#define STACK_ADDRESS_DOWN(address) (u32_t)(ROUND_DOWN((address), STACK_ALIGN))

/**
 * Define the SVC number 2 for RTOS kernal use.
 */
#define SVC_KERNAL_INVOKE_NUMBER 2

/* Start of section using anonymous unions */
#if defined(__CC_ARM)
#pragma push
#pragma anon_unions
#elif defined(__ICCARM__)
#pragma language = extended
#elif defined(__TASKING__)
#pragma warning 586
#endif

/**
 * Data structure for svc call function arguments
 */
typedef struct {
    union {
        /* The function arguments */
        u32_t u32_val;

        u16_t u16_val;

        u8_t u8_val;

        b_t b_val;

        const void *ptr_val;

        const char_t *pch_val;
    };
} arguments_t;

/**
 * Define the privilege call function interface.
 */
typedef u32_t (*pPrivilege_callFunc_t)(arguments_t *);

/**
 * Define the ARM core register layout.
 */
typedef struct {
    u32_t R4;
    u32_t R5;
    u32_t R6;
    u32_t R7;
    u32_t R8;
    u32_t R9;
    u32_t R10;
    u32_t R11;
} cmx_t;

typedef struct {
    u32_t R8;
    u32_t R9;
    u32_t R10;
    u32_t R11;
    u32_t R4;
    u32_t R5;
    u32_t R6;
    u32_t R7;
} cm0_t;

/**
 * Define the register table of ARM Core.
 */
typedef struct {
#if (__FPU_PRESENT)
    u32_t EXC_RETURN; /* LR */
    u32_t CONTROL;    /* CONTROL */
#endif

    union {
        cmx_t cmx;
        cm0_t cm0;
    };

    u32_t R0;  /* N-32 */
    u32_t R1;  /* N-28 */
    u32_t R2;  /* N-24 */
    u32_t R3;  /* N-20 */
    u32_t R12; /* N-16 */

    u32_t R14_LR; /* N-12. Link Register */
    u32_t R15_PC; /* N-8. Program Counter */
    u32_t xPSR;   /* N-4. Program status register */
} stack_snapshot_t;

/* End of section using anonymous unions */
#if defined(__CC_ARM)
#pragma pop
#elif defined(__TASKING__)
#pragma warning restore
#endif

/**
 * Define the common svc call function interface.
 */
#if defined(__CC_ARM)

/**
 * @brief Trigger system svc call.
 */
__svc(SVC_KERNAL_INVOKE_NUMBER) u32_t _impl_kernal_svc_call(u32_t args_0, u32_t args_1, u32_t args_2, u32_t args_3);

/**
 * @brief Schedule the first thread.
 */
__asm void _impl_port_run_theFirstThread(u32_t sp);

#if 0 /* Disable it to use CMSIS Library */
/**
 * @brief To check if it's in interrupt content.
 */
static inline b_t _impl_port_isInInterruptContent(void)
{
    register u32_t reg_ipsr __asm("ipsr");
    if (reg_ipsr) {
        return TRUE;
    }

    register u32_t reg_primask __asm("primask");
    if (reg_primask == 0x01u) {
        return TRUE;
    }

    return FALSE;
}

/**
 * @brief To check if it's in kernal thread content.
 */
static inline b_t _impl_port_isInThreadMode(void)
{
    register u32_t reg_ipsr __asm("ipsr");
    if (reg_ipsr) {
        return FALSE;
    }

    return TRUE;
}
#endif

#elif (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)

/**
 * @brief Trigger system svc call.
 */
static inline u32_t _impl_kernal_svc_call(u32_t args_0, u32_t args_1, u32_t args_2, u32_t args_3)
{
    register u32_t r0 __asm__("r0") = args_0;
    register u32_t r1 __asm__("r1") = args_1;
    register u32_t r2 __asm__("r2") = args_2;
    register u32_t r3 __asm__("r3") = args_3;

    __asm __volatile("svc %1" : "+r"(r0) : "i"(SVC_KERNAL_INVOKE_NUMBER), "r"(r0), "r"(r1), "r"(r2), "r"(r3) : "memory");

    return r0;
}

/**
 * @brief Schedule the first thread.
 */
void _impl_port_run_theFirstThread(u32_t sp);

#if 0 /* Disable it to use CMSIS Library */
/**
 * @brief To check if it's in interrupt content.
 */
static inline b_t _impl_port_isInInterruptContent(void)
{
    u32_t ipsr;

    __asm__ volatile("mrs %0, IPSR\n\t" : "=r"(ipsr));
    if (ipsr) {
        return TRUE;
    }

    u32_t primask;

    __ASM volatile("MRS %0, primask" : "=r"(primask));
    if (primask) {
        return TRUE;
    }

    return FALSE;
}

/**
 * @brief To check if it's in kernal thread content.
 */
static inline b_t _impl_port_isInThreadMode(void)
{
    u32_t ipsr;

    __asm__ volatile("mrs %0, IPSR\n\t" : "=r"(ipsr));
    if (reg_ipsr) {
        return FALSE;
    }

    return TRUE;
}
#endif

#elif defined(__ICCARM__)
#pragma swi_number = SVC_KERNAL_INVOKE_NUMBER

/**
 * @brief Trigger system svc call.
 */
__swi u32_t _impl_kernal_svc_call(u32_t args_0, u32_t args_1, u32_t args_2, u32_t args_3);

/**
 * @brief Schedule the first thread.
 */
void _impl_port_run_theFirstThread(u32_t sp);

#if 0 /* Disable it to use CMSIS Library */
/**
 * @brief To check if it's in interrupt content.
 */
static inline b_t _impl_port_isInInterruptContent(void)
{
    register u32_t reg_ipsr = __arm_rsr("IPSR");
    if (reg_ipsr) {
        return TRUE;
    }

    register u32_t reg_primask = __arm_rsr("PRIMASK");
    if (reg_primask == 0x01u) {
        return TRUE;
    }

    return FALSE;
}

/**
 * @brief To check if it's in kernal thread content.
 */
static inline b_t _impl_port_isInThreadMode(void)
{
    register u32_t reg_ipsr = __arm_rsr("IPSR");
    if (reg_ipsr) {
        return FALSE;
    }

    return TRUE;
}
#endif

#elif defined(__GUNC__)
/* TODO */

#elif defined(__TMS470__)
/* TODO */

#elif defined(__TASKING__)
/* TODO */

#elif defined(ARCH_NATIVE_GCC)
u32_t _impl_kernal_svc_call(u32_t args_0, u32_t args_1, u32_t args_2, u32_t args_3);
void _impl_port_run_theFirstThread(u32_t sp);

#else
#warning Not supported compiler type
#endif

/**
 * The implement function lists for rtos kernal internal use.
 */
b_t _impl_port_isInInterruptContent(void);
b_t _impl_port_isInThreadMode(void);
void _impl_port_setPendSV(void);
void _impl_port_interrupt_init(void);
u32_t _impl_port_stack_frame_init(void (*pEntryFunction)(void), u32_t *pAddress, u32_t size);

#ifdef __cplusplus
}
#endif

#endif /* _PORT_H_ */
