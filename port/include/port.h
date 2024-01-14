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
 * Data structure for svc call function arguments
 */
typedef struct
{
    /* The function arguments */
    u32_t u32_val;
}arguments_t;

/**
 * Define the privilege call function interface.
 */
typedef u32_t (*pPrivilege_callFunc_t)(arguments_t *);

/**
 * Define the SVC number 2 for RTOS kernal use.
 */
#define SVC_KERNAL_INVOKE_NUMBER         2

/**
 * Define the common svc call function interface.
 */
#if defined (__CC_ARM)
    __svc(SVC_KERNAL_INVOKE_NUMBER) u32_t _impl_kernal_svc_call(u32_t args_0, u32_t args_1, u32_t args_2, u32_t arg_3);
    __ASM void _impl_port_run_theFirstThread(u32_t sp);
#elif defined (__ICCARM)
    /* TODO */
#elif defined (__GUNC__)
    /* TODO */
#elif defined (__TMS470)
    /* TODO */
#elif defined (__TASKING__)
    /* TODO */
#elif defined (KERNAL_SAMPLE)
    u32_t _impl_kernal_svc_call(u32_t args_0, u32_t args_1, u32_t args_2, u32_t arg_3);
    void _impl_port_run_theFirstThread(u32_t sp);
#else
    #warning Not supported compiler type
#endif

/**
 * The implement function lists for rtos kernal internal use.
 */
void _impl_port_setPendSV(void);
b_t  _impl_port_isInInterruptContent(void);
b_t  _impl_port_isInThreadMode(void);
void _impl_port_interrupt_init(void);

#ifdef __cplusplus
}
#endif

#endif /* _PORT_H_ */
