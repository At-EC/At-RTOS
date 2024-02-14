/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef _PORT_H_
#define _PORT_H_

#include "type.h"
#include "arch.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Define the privilege call function interface.
 */
typedef u32_t (*pPrivilege_callFunc_t)(arguments_t *);

/**
 * Define the SVC number 2 for RTOS kernal use.
 */
#define SVC_KERNAL_INVOKE_NUMBER         2

/* Start of section using anonymous unions */
#if defined (__CC_ARM)
    #pragma push
    #pragma anon_unions
#elif (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
    /* anonymous unions are enabled by default */
#elif defined (__ICCARM__)
    #pragma language=extended
#elif defined (__GUNC__)
    /* anonymous unions are enabled by default */
#elif defined (__TMS470__)
    /* anonymous unions are enabled by default */
#elif defined (__TASKING__)
    #pragma warning 586
#elif defined (ARCH_NATIVE_GCC)
    /* Nothing to do */
#else
    #warning Not supported compiler type
#endif

/**
 * Define the register table of ARM Core.
 */
typedef struct
{
#if ( __FPU_PRESENT )
    u32_t EXC_RETURN;   /* LR */
    u32_t CONTROL;      /* CONTROL */
#endif

    union
    {
        cmx_t cmx;
        cm0_t cm0;
    };

    u32_t R0;            /* N-32 */
    u32_t R1;            /* N-28 */
    u32_t R2;            /* N-24 */
    u32_t R3;            /* N-20 */
    u32_t R12;           /* N-16 */

    u32_t R14_LR;        /* N-12. Link Register */
    u32_t R15_PC;        /* N-8. Program Counter */
    u32_t xPSR;          /* N-4. Program status register */
} stack_snapshot_t;

/* End of section using anonymous unions */
#if defined ( __CC_ARM )
    #pragma pop
#elif ( __ARMCC_VERSION ) && ( __ARMCC_VERSION >= 6010050 )
    /* anonymous unions are enabled by default */
#elif defined ( __ICCARM__ )
        /* leave anonymous unions enabled */
#elif defined ( __GUNC__ )
        /* anonymous unions are enabled by default */
#elif defined ( __TMS470__ )
        /* anonymous unions are enabled by default */
#elif defined ( __TASKING__ )
    #pragma warning restore
#elif defined ( ARCH_NATIVE_GCC )
    /* Nothing to do */
#else
    #warning Not supported compiler type
#endif

/**
 * Define the common svc call function interface.
 */
#if defined ( __CC_ARM )
    __svc(SVC_KERNAL_INVOKE_NUMBER) u32_t _impl_kernal_svc_call(u32_t args_0, u32_t args_1, u32_t args_2, u32_t args_3);
    __ASM void _impl_port_run_theFirstThread(u32_t sp);
    
#elif ( __ARMCC_VERSION ) && ( __ARMCC_VERSION >= 6010050 )
    static inline u32_t _impl_kernal_svc_call(u32_t args_0, u32_t args_1, u32_t args_2, u32_t args_3)
    {
        register u32_t r0 __asm__("r0") = args_0;
        register u32_t r1 __asm__("r1") = args_1;
        register u32_t r2 __asm__("r2") = args_2;
        register u32_t r3 __asm__("r3") = args_3;
        
        __asm__ __volatile__("svc %0" ::"i"(SVC_KERNAL_INVOKE_NUMBER), "r"(r0), "r"(r1), "r"(r2), "r"(r3)
                             : "memory");
        /* return a operation result from the specific register r0 */
        u32_t ret;
        __asm__ volatile("mov %0, r0" : "=r"(ret));
        return ret;
    } 
    void _impl_port_run_theFirstThread(u32_t sp);
    
#elif defined ( __ICCARM__ )
    #pragma swi_number = SVC_KERNAL_INVOKE_NUMBER
    __swi u32_t _impl_kernal_svc_call(u32_t args_0, u32_t args_1, u32_t args_2, u32_t args_3);
    void _impl_port_run_theFirstThread(u32_t sp);
    
#elif defined ( __GUNC__ )
    /* TODO */
    
#elif defined ( __TMS470__ )
    /* TODO */
    
#elif defined ( __TASKING__ )
    /* TODO */
    
#elif defined ( ARCH_NATIVE_GCC )
    u32_t _impl_kernal_svc_call(u32_t args_0, u32_t args_1, u32_t args_2, u32_t args_3);
    void _impl_port_run_theFirstThread(u32_t sp);
    
#else
    #warning Not supported compiler type
#endif

/**
 * The implement function lists for rtos kernal internal use.
 */
void  _impl_port_setPendSV(void);
b_t   _impl_port_isInInterruptContent(void);
b_t   _impl_port_isInThreadMode(void);
void  _impl_port_interrupt_init(void);
u32_t _impl_port_stack_frame_init(void (*pEntryFunction)(void), u32_t *pAddress, u32_t size);

#ifdef __cplusplus
}
#endif

#endif /* _PORT_H_ */
