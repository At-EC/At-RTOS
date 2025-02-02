/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 **/

#ifndef _KTYPE_H_
#define _KTYPE_H_

#include "type_def.h"
#include "linker.h"

#define CS_INITED                              (1u)
#define STACK_STATIC_VALUE_DEFINE(stack, size) u32_t stack[((u32_t)(size) / sizeof(u32_t))] = {0};

#define OS_INVALID_ID_VAL (0xFFFFFFFFu)

#define OS_TIME_INVALID_VAL (0xFFFFFFFFu)
#define OS_TIME_FOREVER_VAL (0xFFFFFFFEu)
#define OS_TIME_NOWAIT_VAL  (0x0u)

#define OS_PRIOTITY_NUM             (256)
#define OS_PRIOTITY_PREEMPT_NUM     (OS_PRIOTITY_NUM - 1)
#define OS_PRIOTITY_COOPERATION_NUM (OS_PRIOTITY_NUM)

#define OS_PRIOTITY_INVALID_LEVEL (OS_PRIOTITY_PREEMPT_NUM + 1)
#define OS_PRIOTITY_LOWEST_LEVEL  (OS_PRIOTITY_PREEMPT_NUM)
#define OS_PRIOTITY_HIGHEST_LEVEL (-OS_PRIOTITY_COOPERATION_NUM)

#define OS_PRIORITY_KERNEL_IDLE_LEVEL         (OS_PRIOTITY_LOWEST_LEVEL)
#define OS_PRIORITY_KERNEL_SCHEDULE_LEVEL     (OS_PRIOTITY_HIGHEST_LEVEL)
#define OS_PRIORITY_APPLICATION_HIGHEST_LEVEL (OS_PRIOTITY_HIGHEST_LEVEL + 1)
#define OS_PRIORITY_APPLICATION_LOWEST_LEVEL  (OS_PRIOTITY_LOWEST_LEVEL - 1)

#define TIMER_CTRL_ONCE_VAL      (0u)
#define TIMER_CTRL_CYCLE_VAL     (1u)
#define TIMER_CTRL_TEMPORARY_VAL (2u)

enum {
    PC_OS_OK = 0,
    PC_OS_WAIT_TIMEOUT,
    PC_OS_WAIT_AVAILABLE,
    PC_OS_WAIT_UNAVAILABLE,
    PC_OS_WAIT_NODATA,
};

#if defined(__CC_ARM)
#pragma push
#pragma anon_unions
#elif defined(__ICCARM__)
#pragma language = extended
#elif defined(__TASKING__)
#pragma warning 586
#endif

struct os_id {
    union {
        u32_t u32_val;
        void *p_val;
    };
    const char_t *pName;
};

struct evt_val {
    u32_t value;
    u32_t trigger;
};

/* End of section using anonymous unions */
#if defined(__CC_ARM)
#pragma pop
#elif defined(__TASKING__)
#pragma warning restore
#endif

struct foreach_item {
    u8_t i;
    u32_t u32_val;
};

static inline b_t kernel_os_id_is_invalid(struct os_id id)
{
    if (id.p_val == NULL) {
        return true;
    }

    if (id.u32_val == OS_INVALID_ID_VAL) {
        return true;
    }

    return false;
}

#endif /* _KTYPE_H_ */
