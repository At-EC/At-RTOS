/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 **/

#ifndef _CONFIGURATION_H_
#define _CONFIGURATION_H_

#include "atos_configuration.h"
#include "build_version.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ENABLED  (1u)
#define DISABLED (0u)

#ifndef THREAD_INSTANCE_SUPPORTED_NUMBER
#define THREAD_INSTANCE_SUPPORTED_NUMBER (20u)
#endif

#ifndef SEMAPHORE_INSTANCE_SUPPORTED_NUMBER
#define SEMAPHORE_INSTANCE_SUPPORTED_NUMBER (10u)
#endif

#ifndef EVENT_INSTANCE_SUPPORTED_NUMBER
#define EVENT_INSTANCE_SUPPORTED_NUMBER (10u)
#endif

#ifndef MUTEX_INSTANCE_SUPPORTED_NUMBER
#define MUTEX_INSTANCE_SUPPORTED_NUMBER (10u)
#endif

#ifndef QUEUE_INSTANCE_SUPPORTED_NUMBER
#define QUEUE_INSTANCE_SUPPORTED_NUMBER (10u)
#endif

#ifndef TIMER_INSTANCE_SUPPORTED_NUMBER
#define TIMER_INSTANCE_SUPPORTED_NUMBER (10u)
#endif

#ifndef PORTAL_SYSTEM_CORE_CLOCK_MHZ
#define PORTAL_SYSTEM_CORE_CLOCK_MHZ (120u)
#endif

#ifndef PORTAL_SYSTEM_CLOCK_INTERVAL_MIN_US
#define PORTAL_SYSTEM_CLOCK_INTERVAL_MIN_US (50u)
#endif

#ifndef AT_RTOS_USE_INTERNAL_IRQ_ENUM
#define AT_RTOS_USE_INTERNAL_IRQ_ENUM (DISABLED)
#endif

#ifndef STACK_ALIGN
#define STACK_ALIGN (8u)
#endif

#ifndef STACK_SIZE_MINIMUM
#define STACK_SIZE_MINIMUM (128u)
#endif

#ifndef STACK_SIZE_MAXIMUM
#define STACK_SIZE_MAXIMUM (0xFFFFFFu)
#endif

#ifndef THREAD_PSP_WITH_PRIVILEGED
#define THREAD_PSP_WITH_PRIVILEGED (0u)
#endif

#ifndef IDLE_THREAD_STACK_SIZE
#define IDLE_THREAD_STACK_SIZE (512u)
#endif

#ifndef KERNAL_THREAD_STACK_SIZE
#define KERNAL_THREAD_STACK_SIZE (1024u)
#endif

/* Configuration of the Cortex-M Processor and Core Peripherals.
 * You should check the chip header file or datasheet to check the following declaration symbol that support ARM Cortex-M Processor and Core
 * Peripherals, and put it here. It looks like this.
 *
 * #define __SAUREGION_PRESENT                          (1u) // SAU regions present
 * #define __MPU_PRESENT                                (1u) // MPU present
 * #define __VTOR_PRESENT                               (1u) // VTOR present
 * #define __NVIC_PRIO_BITS                             (4u) // Number of Bits used for Priority Levels
 * #define __Vendor_SysTickConfig                       (0u) // Set to 1 if different SysTick Config is used
 * #define __FPU_PRESENT                                (1u) // FPU present
 * #define __DSP_PRESENT                                (1u) // DSP extension present
 *
 * Or you can manually defined it according to your using ARM Cortex M seiral chip here with the following declaration symbol.
 *
 * #define ARCH_SAUREGION_PRESENT                        (1u) // SAU regions present
 * #define ARCH_MPU_PRESENT                              (1u) // MPU present
 * #define ARCH_VTOR_PRESENT                             (1u) // VTOR present
 * #define ARCH_NVIC_PRIO_BITS                           (4u) // Number of Bits used for Priority Levels
 * #define ARCH_Vendor_SysTickConfig                     (0u) // Set to 1 if different SysTick Config is used
 * #define ARCH_FPU_PRESENT                              (1u) // FPU present
 * #define ARCH_DSP_PRESENT                              (1u) // DSP extension present
 *
 * There is no default setting, which must be set in the atos_configuration.h file.
 */

#if !defined __CC_ARM && !defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050) && !defined __ICCARM__ && !defined __GUNC__ &&          \
    !defined __TMS470__ && !defined __TASKING__ && !defined ARCH_NATIVE_GCC

#warning Not supported compiler type
#endif

/* It defined the AtOS extern symbol for convenience use, but it has extra memory consumption */
#ifndef OS_INTERFACE_EXTERN_USE_ENABLE
#define OS_INTERFACE_EXTERN_USE_ENABLE (ENABLED)
#endif

#ifdef __cplusplus
}
#endif

#endif /* _CONFIGURATION_H_ */
