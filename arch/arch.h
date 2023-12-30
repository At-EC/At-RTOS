/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef _ARCH_H_
#define _ARCH_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "os_config.h"

typedef enum IRQn {
    /******  Cortex-Mx Processor Exceptions Numbers
    ***************************************************/
    NonMaskableInt_IRQn = -14,   /*!< 2 Non Maskable Interrupt                         */
    HardFault_IRQn = -13,        /*!<   3  Hard Fault, all classes of Fault        */
    MemoryManagement_IRQn = -12, /*!< 4 Cortex-Mx Memory Management Interrupt          */
    BusFault_IRQn = -11,         /*!< 5 Cortex-Mx Bus Fault Interrupt                  */
    UsageFault_IRQn = -10,       /*!< 6 Cortex-Mx Usage Fault Interrupt                */
    SVCall_IRQn = -5,            /*!< 11 Cortex-Mx SV Call Interrupt                   */
    DebugMonitor_IRQn = -4,      /*!< 12 Cortex-Mx Debug Monitor Interrupt             */
    PendSV_IRQn = -2,            /*!< 14 Cortex-Mx Pend SV Interrupt                   */
    SysTick_IRQn = -1,           /*!< 15 Cortex-Mx System Tick Interrupt               */
} IRQn_Type;

#ifndef __MPU_PRESENT
    #define __MPU_PRESENT               1
#endif

#ifndef __NVIC_PRIO_BITS
    #define __NVIC_PRIO_BITS            8
#endif

#if defined __v7em_f5ss ||  \
    defined __v7em_f5sh ||  \
    defined __v7em_f5ds ||  \
    defined __v7em_f5dh

    #ifndef __ICACHE_PRESENT
        #define __ICACHE_PRESENT        0
    #endif

    #ifndef __DCACHE_PRESENT
        #define __DCACHE_PRESENT        0
    #endif

    #ifndef __DTCM_PRESENT
        #define __DTCM_PRESENT          0
    #endif

    #ifndef __CM7_REV
        #define __CM7_REV               1
    #endif

    #ifndef __Vendor_SysTickConfig
        #define __Vendor_SysTickConfig  0
    #endif
#endif

#if defined __v7m
    #define ARCH "v7m"

    #define __FPU_PRESENT 0

    #if !defined ARM_MATH_CM3
        #define ARM_MATH_CM3 1
    #endif
    #undef __FPU_USED
    #include "arch/cmsis/Include/core_cm3.h"

#elif defined __v7em

    #define ARCH "v7em"
    #define __FPU_PRESENT 0
    #if !defined ARM_MATH_CM4
        #define ARM_MATH_CM4 1
    #endif
    #undef __FPU_USED
    #include "arch/cmsis/Include/core_cm4.h"

#elif defined __v7em_f4ss
    #define ARCH "v7em_f4ss"

    #define __FPU_PRESENT 1
    #if !defined ARM_MATH_CM4
        #define ARM_MATH_CM4 1
    #endif
    #undef __FPU_USED
    #include "arch/cmsis/Include/core_cm4.h"

#elif defined __v7em_f4sh
    #define ARCH "v7em_f4sh"

    #define __FPU_PRESENT 1
    #if !defined ARM_MATH_CM4
        #define ARM_MATH_CM4 1
    #endif
    #undef __FPU_USED
    #include "arch/cmsis/Include/core_cm4.h"

#elif defined __v7em_f5ss
    #define ARCH "v7em_f5ss"

    #define __CHECK_DEVICE_DEFINES
    #define __FPU_PRESENT 1
    #if !defined ARM_MATH_CM7
        #define ARM_MATH_CM7 1
    #endif
    #undef __FPU_USED
    #include "arch/cmsis/Include/core_cm7.h"

#elif defined __v7em_f5sh
    #define ARCH "v7em_f5sh"

    #define __CHECK_DEVICE_DEFINES
    #define __FPU_PRESENT 1
    #if !defined ARM_MATH_CM7
        #define ARM_MATH_CM7 1
    #endif
    #undef __FPU_USED
    #include "arch/cmsis/Include/core_cm7.h"

#elif defined __v7em_f5ds
    #define ARCH "v7em_f5ds"

    #define __CHECK_DEVICE_DEFINES
    #define __FPU_PRESENT 1
    #if !defined ARM_MATH_CM7
        #define ARM_MATH_CM7 1
    #endif
    #undef __FPU_USED
    #include "arch/cmsis/Include/core_cm7.h"

#elif defined __v7em_f5dh
    #define ARCH "v7em_f5dh"

    #define __CHECK_DEVICE_DEFINES
    #define __FPU_PRESENT 1
    #define ARM_MATH_CM7 1
    #undef __FPU_USED
    #include "arch/cmsis/Include/core_cm7.h"

#else
    #error "No ARM Arch is defined"
#endif

#define ARCH_ENTER_CRITICAL_SECTION()   vu32_t PRIMASK_Bit = __get_PRIMASK();        \
                                          __disable_irq();                           \
                                          __DSB();                                   \
                                          __ISB();

#define ARCH_EXIT_CRITICAL_SECTION()    __set_PRIMASK(PRIMASK_Bit);                  \
                                          __DSB();                                   \
                                          __ISB();

#ifdef __cplusplus
}
#endif

#endif /* _ARCH_H_ */
