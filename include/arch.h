/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 **/

#ifndef _ARCH_H_
#define _ARCH_H_

#include "configuration.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum IRQn {
    /******  Cortex-Mx Processor Exceptions Numbers
    ***************************************************/
    NonMaskableInt_IRQn = -14,   /*!< 2 Non Maskable Interrupt                         */
    HardFault_IRQn = -13,        /*!< 3  Hard Fault, all classes of Fault              */
    MemoryManagement_IRQn = -12, /*!< 4 Cortex-Mx Memory Management Interrupt          */
    BusFault_IRQn = -11,         /*!< 5 Cortex-Mx Bus Fault Interrupt                  */
    UsageFault_IRQn = -10,       /*!< 6 Cortex-Mx Usage Fault Interrupt                */
    SVCall_IRQn = -5,            /*!< 11 Cortex-Mx SV Call Interrupt                   */
    DebugMonitor_IRQn = -4,      /*!< 12 Cortex-Mx Debug Monitor Interrupt             */
    PendSV_IRQn = -2,            /*!< 14 Cortex-Mx Pend SV Interrupt                   */
    SysTick_IRQn = -1,           /*!< 15 Cortex-Mx System Tick Interrupt               */
} IRQn_Type;

/* Configuration of the Cortex-M Processor and Core Peripherals */
/* SAU regions present */
#if !defined(__SAUREGION_PRESENT)
#if defined(ARCH_SAUREGION_PRESENT)
#define __SAUREGION_PRESENT ARCH_SAUREGION_PRESENT
#endif
#endif

/* MPU present */
#if !defined(__MPU_PRESENT)
#if defined(ARCH_MPU_PRESENT)
#define __MPU_PRESENT ARCH_MPU_PRESENT
#endif
#endif

/* MPU present */
#if !defined(__VTOR_PRESENT)
#if defined(ARCH_VTOR_PRESENT)
#define __VTOR_PRESENT ARCH_VTOR_PRESENT
#endif
#endif

/* VTOR present */
#if !defined(__VTOR_PRESENT)
#if defined(ARCH_VTOR_PRESENT)
#define __VTOR_PRESENT ARCH_VTOR_PRESENT
#endif
#endif

/* DSP extension present */
#if !defined(__DSP_PRESENT)
#if defined(ARCH_DSP_PRESENT)
#define __DSP_PRESENT ARCH_DSP_PRESENT
#endif
#endif

/* FPU present */
#if !defined(__FPU_PRESENT)
#if defined(ARCH_FPU_PRESENT)
#define __FPU_PRESENT ARCH_FPU_PRESENT
#endif
#endif

/* Number of Bits used for Priority Levels */
#if !defined(__NVIC_PRIO_BITS)
#if defined(ARCH_NVIC_PRIO_BITS)
#define __NVIC_PRIO_BITS ARCH_NVIC_PRIO_BITS
#else
#error "No __NVIC_PRIO_BITS or ARCH_NVIC_PRIO_BITS is defined in head of this file"
#endif
#endif

/* Set to 1 if different SysTick Config is used */
#if !defined(__Vendor_SysTickConfig)
#if defined(ARCH_Vendor_SysTickConfig)
#define __Vendor_SysTickConfig ARCH_Vendor_SysTickConfig
#else
#error "No __Vendor_SysTickConfig or ARCH_Vendor_SysTickConfig is defined in head of this file"
#endif
#endif

#if defined(ARCH_ARM_CORTEX_CM0)
#include "../arch/arch32/arm/cmsis/include/core_cm0.h"

#elif defined(ARCH_ARM_CORTEX_CM0plus)
#include "../arch/arch32/arm/cmsis/include/core_cm0plus.h"

#elif defined(ARCH_ARM_CORTEX_CM3)
#include "../arch/arch32/arm/cmsis/include/core_cm3.h"

#elif defined(ARCH_ARM_CORTEX_CM4)
#include "../arch/arch32/arm/cmsis/include/core_cm4.h"

#elif defined(ARCH_ARM_CORTEX_CM23)
#include "../arch/arch32/arm/cmsis/include/core_cm23.h"

#elif defined(ARCH_ARM_CORTEX_CM33)
#include "../arch/arch32/arm/cmsis/include/core_cm33.h"

#elif defined ARCH_ARM_CORTEX_CM7
#include "../arch/arch32/arm/cmsis/include/core_cm7.h"

#elif defined ARCH_NATIVE_GCC
// Nothing to do
#else
#error "No ARM Arch is defined in head of this file"
#endif

#ifdef __cplusplus
}
#endif

#endif /* _ARCH_H_ */
