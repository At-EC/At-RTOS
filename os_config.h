/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef _OS_CONFIG_H_
#define _OS_CONFIG_H_

#define AT_RTOS_FEATURE_ENABLE                    (1u)
#define AT_RTOS_FEATURE_DISABLE                   (0u)

#define THREAD_INSTANCE_SUPPORTED_NUMBER          (15u)
#define SEMAPHORE_INSTANCE_SUPPORTED_NUMBER       (10u)
#define EVENT_INSTANCE_SUPPORTED_NUMBER           (10u)
#define MUTEX_INSTANCE_SUPPORTED_NUMBER           (10u)
#define QUEUE_INSTANCE_SUPPORTED_NUMBER           (10u)
#define TIMER_INSTANCE_SUPPORTED_NUMBER           (10u)

#define PORTAL_SYSTEM_CORE_CLOCK_MHZ              (120u)
#define PORTAL_SYSTEM_CLOCK_INTERVAL_MIN_US       (50u)

#define AT_RTOS_USE_INTERNAL_IRQ_ENUM             (AT_RTOS_FEATURE_DISABLE)

#define STACK_ALIGN                     		  (8u)
#define STACK_SIZE_MINIMUM              		  (128u)
#define STACK_SIZE_MAXIMUM              		  (0xFFFFFFu)

#define THREAD_PSP_WITH_PRIVILEGED                (0u)

#define __v7em

#endif

