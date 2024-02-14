/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef _CONFIGURATION_H_
#define _CONFIGURATION_H_

#include "atos_configuration.h"
#include "kernal_version.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ENABLED                                       (1u)
#define DISABLED                                      (0u)

#ifndef THREAD_INSTANCE_SUPPORTED_NUMBER
    #define THREAD_INSTANCE_SUPPORTED_NUMBER          (20u)
#endif

#ifndef SEMAPHORE_INSTANCE_SUPPORTED_NUMBER
    #define SEMAPHORE_INSTANCE_SUPPORTED_NUMBER       (10u)
#endif

#ifndef EVENT_INSTANCE_SUPPORTED_NUMBER
    #define EVENT_INSTANCE_SUPPORTED_NUMBER           (10u)
#endif

#ifndef MUTEX_INSTANCE_SUPPORTED_NUMBER
    #define MUTEX_INSTANCE_SUPPORTED_NUMBER           (10u)
#endif

#ifndef QUEUE_INSTANCE_SUPPORTED_NUMBER
    #define QUEUE_INSTANCE_SUPPORTED_NUMBER           (10u)
#endif

#ifndef TIMER_INSTANCE_SUPPORTED_NUMBER
    #define TIMER_INSTANCE_SUPPORTED_NUMBER           (10u)
#endif

#ifndef PORTAL_SYSTEM_CORE_CLOCK_MHZ
    #define PORTAL_SYSTEM_CORE_CLOCK_MHZ              (120u)
#endif

#ifndef PORTAL_SYSTEM_CLOCK_INTERVAL_MIN_US
    #define PORTAL_SYSTEM_CLOCK_INTERVAL_MIN_US       (50u)
#endif

#ifndef AT_RTOS_USE_INTERNAL_IRQ_ENUM
    #define AT_RTOS_USE_INTERNAL_IRQ_ENUM             (DISABLED)
#endif

#ifndef STACK_ALIGN
    #define STACK_ALIGN                               (8u)
#endif

#ifndef STACK_SIZE_MINIMUM
    #define STACK_SIZE_MINIMUM                        (128u)
#endif

#ifndef STACK_SIZE_MAXIMUM
    #define STACK_SIZE_MAXIMUM                        (0xFFFFFFu)
#endif

#ifndef THREAD_PSP_WITH_PRIVILEGED
    #define THREAD_PSP_WITH_PRIVILEGED                (0u)
#endif

#ifndef IDLE_THREAD_STACK_SIZE
    #define IDLE_THREAD_STACK_SIZE                    (512u)
#endif

#ifndef KERNAL_THREAD_STACK_SIZE
    #define KERNAL_THREAD_STACK_SIZE                  (1024u)
#endif

#if !defined __CC_ARM &&  \
    !defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050) && \
    !defined __ICCARM__ &&  \
    !defined __GUNC__ &&  \
    !defined __TMS470__ &&  \
    !defined __TASKING__ &&  \
    !defined ARCH_NATIVE_GCC

    #warning Not supported compiler type

#endif

#ifdef __cplusplus
}
#endif

#endif /* _CONFIGURATION_H_ */

