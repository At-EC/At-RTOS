/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 **/

#ifndef _KTYPE_H_
#define _KTYPE_H_

#include "typedef.h"
#include "linker.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef u32_t os_id_t;

struct os_id {
    u32_t number;
    u32_t val;
    const char_t *pName;
};

struct os_priority {
    u8_t level;
};

struct os_time {
    u32_t ms;
};

typedef struct os_id os_thread_id_t;
typedef struct os_id os_timer_id_t;
typedef struct os_id os_sem_id_t;
typedef struct os_id os_mutex_id_t;
typedef struct os_id os_evt_id_t;
typedef struct os_id os_msgq_id_t;
typedef struct os_id os_pool_id_t;
typedef struct os_id os_publish_id_t;
typedef struct os_id os_subscribe_id_t;

typedef struct os_priority os_priority_t;
typedef struct os_time os_time_t;

typedef struct {
    u32_t value;
    u32_t trigger;
} os_evt_val_t;

#define OS_INVALID_ID_VAL (0xFFFFFFFFu)

#define OS_TIME_INVALID_VAL (0xFFFFFFFFu)
#define OS_TIME_FOREVER_VAL (0xFFFFFFFEu)
#define OS_TIME_NOWAIT_VAL  (0x0u)

#define OS_PRIOTITY_INVALID_LEVEL (0xFFu)
#define OS_PRIOTITY_LOWEST_LEVEL  (0xFEu)
#define OS_PRIOTITY_HIGHEST_LEVEL (0x0u)

#define OS_PRIORITY_INVALID                      (OS_PRIOTITY_INVALID_LEVEL)
#define OS_PRIORITY_KERNEL_THREAD_IDLE_LEVEL     (OS_PRIOTITY_LOWEST_LEVEL)
#define OS_PRIORITY_KERNEL_THREAD_SCHEDULE_LEVEL (OS_PRIOTITY_HIGHEST_LEVEL)
#define OS_PRIORITY_USER_THREAD_LOWEST_LEVEL     (OS_PRIOTITY_LOWEST_LEVEL - 1u)
#define OS_PRIORITY_USER_THREAD_HIGHEST_LEVEL    (OS_PRIOTITY_HIGHEST_LEVEL + 1u)

#define TIMER_CTRL_ONCE_VAL      (0u)
#define TIMER_CTRL_CYCLE_VAL     (1u)
#define TIMER_CTRL_TEMPORARY_VAL (2u)

enum {
    PC_OS_OK = 0,
    PC_OS_WAIT_TIMEOUT,
    PC_OS_WAIT_AVAILABLE,
    PC_OS_WAIT_UNAVAILABLE,
};

u8_t *kernel_member_unified_id_toContainerAddress(u32_t unified_id);
u32_t kernel_member_containerAddress_toUnifiedid(u32_t container_address);
u32_t kernel_member_id_toUnifiedIdStart(u8_t member_id);
u8_t *kernel_member_id_toContainerStartAddress(u32_t member_id);
u8_t *kernel_member_id_toContainerEndAddress(u32_t member_id);
b_t kernel_member_unified_id_isInvalid(u32_t member_id, u32_t unified_id);
u8_t kernel_member_unified_id_toId(u32_t unified_id);
u32_t kernel_member_unified_id_threadToTimer(u32_t unified_id);
u32_t kernel_member_unified_id_timerToThread(u32_t unified_id);
u32_t kernel_member_id_unifiedConvert(u8_t member_id, u32_t unified_id);
os_id_t kernel_thread_runIdGet(void);
b_t kernel_os_id_is_invalid(struct os_id id);

#ifdef __cplusplus
}
#endif

#endif /* _KTYPE_H_ */
