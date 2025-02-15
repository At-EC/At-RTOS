/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 **/
#ifndef _OS_CONFIG_H_
#define _OS_CONFIG_H_

/**
 * If you are use ARM Cortex M seiral architecture, the Cortex-M Core must be specificed as the following list.
 * ARCH_ARM_CORTEX_CM3
 * ARCH_ARM_CORTEX_CM4
 * ARCH_ARM_CORTEX_CM33
 **/
#define ARCH_NATIVE_GCC

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
 */
#define ARCH_SAUREGION_PRESENT    (1u) // SAU regions present
#define ARCH_MPU_PRESENT          (1u) // MPU present
#define ARCH_VTOR_PRESENT         (1u) // VTOR present
#define ARCH_NVIC_PRIO_BITS       (4u) // Number of Bits used for Priority Levels
#define ARCH_Vendor_SysTickConfig (0u) // Set to 1 if different SysTick Config is used
#define ARCH_FPU_PRESENT          (1u) // FPU present
#define ARCH_DSP_PRESENT          (1u) // DSP extension present

/**
 * If you are use ARM Cortex M seiral architecture and use the system tick as the kernel timer.
 * In most cases, PORTAL_SYSTEM_CORE_CLOCK_MHZ must be set to the frequency of the clock
 * that drives the peripheral used to generate the kernels periodic tick interrupt.
 * The default value is set to 120mhz. Your application will certainly need a different value so set this correctly.
 * This is very often, but not always, equal to the main system clock frequency.
 **/
#define PORTAL_SYSTEM_CORE_CLOCK_MHZ (120u)

/**
 * If you are use ARM Cortex M seiral architecture and use the system tick as the kernel timer.
 * The kernels periodic tick interrupt scheduler needs a minimum time to handle the kernel time function,
 * The defaule value is set ot 50us when the frequency is 120mhz. Your application will certainly need a different value so set this
 *correctly. This is very often, but not always, according to the main system clock frequency.
 **/
#define PORTAL_SYSTEM_CLOCK_INTERVAL_MIN_US (50u)

/**
 * This symbol defined the thread instance number that your application is using.
 * The defaule value is set to 1. Your application will certainly need a different value so set this correctly.
 * This is very often, but not always, according to the actual thread instance number that you created.
 **/
#define THREAD_RUNTIME_NUMBER_SUPPORTED (20u)

/**
 * This symbol defined the semaphore instance number that your application is using.
 * The defaule value is set to 1. Your application will certainly need a different value so set this correctly.
 * This is very often, but not always, according to the actual semaphore instance number that you created.
 **/
#define SEMAPHORE_RUNTIME_NUMBER_SUPPORTED (10u)

/**
 * This symbol defined the event instance number that your application is using.
 * The defaule value is set to 1. Your application will certainly need a different value so set this correctly.
 * This is very often, but not always, according to the actual event instance number that you created.
 **/
#define EVENT_RUNTIME_NUMBER_SUPPORTED (10u)

/**
 * This symbol defined the mutex instance number that your application is using.
 * The defaule value is set to 1. Your application will certainly need a different value so set this correctly.
 * This is very often, but not always, according to the actual mutex instance number that you created.
 **/
#define MUTEX_RUNTIME_NUMBER_SUPPORTED (10u)

/**
 * This symbol defined the queue instance number that your application is using.
 * The defaule value is set to 1. Your application will certainly need a different value so set this correctly.
 * This is very often, but not always, according to the actual queue instance number that you created.
 **/
#define QUEUE_RUNTIME_NUMBER_SUPPORTED (10u)

/**
 * This symbol defined the timer instance number that your application is using.
 * The defaule value is set to 1. Your application will certainly need a different value so set this correctly.
 * This is very often, but not always, according to the actual timer instance number that you created.
 **/
#define TIMER_RUNTIME_NUMBER_SUPPORTED (10u)

/**
 * This symbol defined the timer instance number that your application is using.
 * The defaule value is set to 1. Your application will certainly need a different value so set this correctly.
 * This is very often, but not always, according to the actual timer instance number that you created.
 **/
#define POOL_RUNTIME_NUMBER_SUPPORTED (10u)

/**
 * This symbol defined the timer instance number that your application is using.
 * The defaule value is set to 5. Your application will certainly need a
 * different value so set this correctly. This is very often, but not always,
 * according to the actual timer instance number that you created.
 **/
#define PUBLISH_RUNTIME_NUMBER_SUPPORTED (10u)

/**
 * This symbol defined the timer instance number that your application is using.
 * The defaule value is set to 5. Your application will certainly need a
 * different value so set this correctly. This is very often, but not always,
 * according to the actual timer instance number that you created.
 **/
#define SUBSCRIBE_RUNTIME_NUMBER_SUPPORTED (10u)

/**
 * This symbol defined your thread running mode, if the thread runs at the privileged mode.
 * The defaule value is set to 0. Your application will certainly need a different value so set this correctly.
 * This is very often, but not always, according to the security level that you want.
 **/
#define THREAD_PSP_WITH_PRIVILEGED (10u)

/**
 * This symbol defined your thread running mode, if the thread runs at the
 *privileged mode. The defaule value is set to 4. Your application will
 *certainly need a different value so set this correctly. This is very often,
 *but not always, according to the security level that you want.
 **/
#define MALLOC_HEAP_SIZE_SUPPORTED (0x1000)

#endif /* _OS_CONFIG_H_ */
