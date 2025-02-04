/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 **/
#ifndef _PORT_H_
#define _PORT_H_

#include "type_def.h"
#include "./arch/arch.h"

#define STACT_UNUSED_DATA       (0xDEu)
#define STACT_UNUSED_FRAME_MARK (0xDEDEDEDEu)

#define STACK_ADDRESS_UP(address)   (_u32_t)(ROUND_UP((address), STACK_ALIGN))
#define STACK_ADDRESS_DOWN(address) (_u32_t)(ROUND_DOWN((address), STACK_ALIGN))

#define PORT_ENTER_CRITICAL_SECTION() _u32_t __val = port_irq_disable()
#define PORT_EXIT_CRITICAL_SECTION()  port_irq_enable(__val)

/**
 * Define the SVC number 2 for RTOS kernel use.
 */
#define SVC_KERNEL_INVOKE_NUMBER 2

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
        _u32_t u32_val;

        _u16_t u16_val;

        _u8_t u8_val;

        _b_t b_val;

        void *pv_val;

        const void *ptr_val;

        const _char_t *pch_val;
    };
} arguments_t;

/**
 * Define the privilege call function interface.
 */
typedef _u32_t (*pPrivilege_callFunc_t)(arguments_t *);

/**
 * Define the ARM core register layout.
 */
typedef struct {
    _u32_t R4;
    _u32_t R5;
    _u32_t R6;
    _u32_t R7;
    _u32_t R8;
    _u32_t R9;
    _u32_t R10;
    _u32_t R11;
} cmx_t;

typedef struct {
    _u32_t R8;
    _u32_t R9;
    _u32_t R10;
    _u32_t R11;
    _u32_t R4;
    _u32_t R5;
    _u32_t R6;
    _u32_t R7;
} cm0_t;

/**
 * Define the register table of ARM Core.
 */
typedef struct {
#if (__FPU_PRESENT)
    _u32_t EXC_RETURN; /* LR */
    _u32_t CONTROL;    /* CONTROL */
#endif

    union {
        cmx_t cmx;
        cm0_t cm0;
    };

    _u32_t R0;  /* N-32 */
    _u32_t R1;  /* N-28 */
    _u32_t R2;  /* N-24 */
    _u32_t R3;  /* N-20 */
    _u32_t R12; /* N-16 */

    _u32_t R14_LR; /* N-12. Link Register */
    _u32_t R15_PC; /* N-8. Program Counter */
    _u32_t xPSR;   /* N-4. Program status register */
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
__svc(SVC_KERNEL_INVOKE_NUMBER) _i32p_t kernel_svc_call(_u32_t args_0, _u32_t args_1, _u32_t args_2, _u32_t args_3);

/**
 * @brief Schedule the first thread.
 */
__asm void port_run_theFirstThread(_u32_t sp);

#if 0 /* Disable it to use CMSIS Library */
/**
 * @brief To check if it's in interrupt content.
 */
static inline _b_t port_isInInterruptContent(void)
{
    register _u32_t reg_ipsr __asm("ipsr");
    if (reg_ipsr) {
        return true;
    }

    register _u32_t reg_primask __asm("primask");
    if (reg_primask == 0x01u) {
        return true;
    }

    return false;
}

/**
 * @brief To check if it's in kernel thread content.
 */
static inline _b_t port_isInThreadMode(void)
{
    register _u32_t reg_ipsr __asm("ipsr");
    if (reg_ipsr) {
        return false;
    }

    return true;
}
#endif

#elif (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)

/**
 * @brief Trigger system svc call.
 */
static inline _i32p_t kernel_svc_call(_u32_t args_0, _u32_t args_1, _u32_t args_2, _u32_t args_3)
{
    register _u32_t r0 __asm__("r0") = args_0;
    register _u32_t r1 __asm__("r1") = args_1;
    register _u32_t r2 __asm__("r2") = args_2;
    register _u32_t r3 __asm__("r3") = args_3;

    __asm __volatile("svc %1" : "+r"(r0) : "i"(SVC_KERNEL_INVOKE_NUMBER), "r"(r0), "r"(r1), "r"(r2), "r"(r3) : "memory");

    return r0;
}

/**
 * @brief Schedule the first thread.
 */
void port_run_theFirstThread(_u32_t sp);

#if 0 /* Disable it to use CMSIS Library */
/**
 * @brief To check if it's in interrupt content.
 */
static inline _b_t port_isInInterruptContent(void)
{
    _u32_t ipsr;

    __asm__ volatile("mrs %0, IPSR\n\t" : "=r"(ipsr));
    if (ipsr) {
        return true;
    }

    _u32_t primask;

    __ASM volatile("MRS %0, primask" : "=r"(primask));
    if (primask) {
        return true;
    }

    return false;
}

/**
 * @brief To check if it's in kernel thread content.
 */
static inline _b_t port_isInThreadMode(void)
{
    _u32_t ipsr;

    __asm__ volatile("mrs %0, IPSR\n\t" : "=r"(ipsr));
    if (reg_ipsr) {
        return false;
    }

    return true;
}
#endif

#elif defined(__ICCARM__)
#pragma swi_number = SVC_KERNEL_INVOKE_NUMBER

/**
 * @brief Trigger system svc call.
 */
__swi _i32p_t kernel_svc_call(_u32_t args_0, _u32_t args_1, _u32_t args_2, _u32_t args_3);

/**
 * @brief Schedule the first thread.
 */
void port_run_theFirstThread(_u32_t sp);

#if 0 /* Disable it to use CMSIS Library */
/**
 * @brief To check if it's in interrupt content.
 */
static inline _b_t port_isInInterruptContent(void)
{
    register _u32_t reg_ipsr = __arm_rsr("IPSR");
    if (reg_ipsr) {
        return true;
    }

    register _u32_t reg_primask = __arm_rsr("PRIMASK");
    if (reg_primask == 0x01u) {
        return true;
    }

    return false;
}

/**
 * @brief To check if it's in kernel thread content.
 */
static inline _b_t port_isInThreadMode(void)
{
    register _u32_t reg_ipsr = __arm_rsr("IPSR");
    if (reg_ipsr) {
        return false;
    }

    return true;
}

static inline _u32_t port_irq_disable(void)
{
    register _u32_t reg_primask = __arm_rsr("PRIMASK");
    __disable_irq();
    return reg_primask;
}

static inline void port_irq_enable(_u32_t value)
{
    __arm_wsr("PRIMASK", (value));
}

#endif

#elif defined(__GUNC__)
/* TODO */

#elif defined(__TMS470__)
/* TODO */

#elif defined(__TASKING__)
/* TODO */

#elif defined(ARCH_NATIVE_GCC)
_i32p_t kernel_svc_call(_u32_t args_0, _u32_t args_1, _u32_t args_2, _u32_t args_3);
void port_run_theFirstThread(_u32_t sp);

#else
#warning Not supported compiler type
#endif

/**
 * The implement function lists for rtos kernel internal use.
 */
_b_t port_isInInterruptContent(void);
_b_t port_isInThreadMode(void);
_u32_t port_irq_disable(void);
void port_irq_enable(_u32_t value);
void port_setPendSV(void);
void port_interrupt_init(void);
_u32_t port_stack_frame_init(void (*pEntryFn)(void *), _u32_t *pAddress, _u32_t size, void *pArg);
_u32_t port_stack_free_size_get(_u32_t stack_addr);

#endif /* _PORT_H_ */
