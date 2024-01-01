/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef _SVC_CALL_H_
#define _SVC_CALL_H_

#include "type.h"
#include "arch/arch.h"

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
__svc(SVC_KERNAL_INVOKE_NUMBER) u32_t _impl_kernal_svc_call(u32_t args_0, u32_t args_1, u32_t args_2, u32_t arg_3);

#ifdef __cplusplus
}
#endif

#endif /* _SVC_CALL_H_ */

