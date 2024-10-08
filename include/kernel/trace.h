/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 **/

#ifndef _TRACE_H_
#define _TRACE_H_

#include "typedef.h"
#include "linker.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Start of section using anonymous unions */
#if defined(__CC_ARM)
#pragma push
#pragma anon_unions
#elif defined(__ICCARM__)
#pragma language = extended
#elif defined(__TASKING__)
#pragma warning 586
#endif

typedef struct {
    u32_t priority;
    u32_t current_psp;
    u32_t ram;
    u32_t cpu;
    u32_t delay;
} thread_snapshot_t;

typedef struct {
    u16_t initial_count;
    u16_t limit_count;
    u32_t timeout_ms;
    list_t wait_list;
} semaphore_snapshot_t;

typedef struct {
    u32_t holdThreadId;

    u32_t originalPriority;

    list_t wait_list;
} mutex_snapshot_t;

typedef struct {
    u32_t set;

    u32_t edge;

    list_t wait_list;
} event_snapshot_t;

typedef struct {
    u32_t cacheSize;

    list_t in_list;

    list_t out_list;
} queue_snapshot_t;

typedef struct {
    u32_t free;

    list_t wait_list;
} pool_snapshot_t;

typedef struct {
    u8_t cycle;

    u32_t timeout_ms;
} timer_snapshot_t;

typedef struct {
    u32_t refresh_count;
} publish_snapshot_t;

typedef struct {
    u32_t id;
    const char_t *pName;
    const char_t *pState;
    union {
        thread_snapshot_t thread;

        semaphore_snapshot_t semaphore;

        mutex_snapshot_t mutex;

        event_snapshot_t event;

        queue_snapshot_t queue;

        pool_snapshot_t pool;

        timer_snapshot_t timer;

        publish_snapshot_t publish;
    };
} kernel_snapshot_t;

/* End of section using anonymous unions */
#if defined(__CC_ARM)
#pragma pop
#elif defined(__TASKING__)
#pragma warning restore
#endif

b_t thread_snapshot(u32_t instance, kernel_snapshot_t *pMsgs);
b_t semaphore_snapshot(u32_t instance, kernel_snapshot_t *pMsgs);
b_t mutex_snapshot(u32_t instance, kernel_snapshot_t *pMsgs);
b_t event_snapshot(u32_t instance, kernel_snapshot_t *pMsgs);
b_t queue_snapshot(u32_t instance, kernel_snapshot_t *pMsgs);
b_t pool_snapshot(u32_t instance, kernel_snapshot_t *pMsgs);
b_t timer_snapshot(u32_t instance, kernel_snapshot_t *pMsgs);
b_t publish_snapshot(u32_t instance, kernel_snapshot_t *pMsgs);

void _impl_trace_firmware_snapshot_print(void);
void _impl_trace_postcode_snapshot_print(void);
void _impl_trace_kernel_snapshot_print(void);

#ifdef __cplusplus
}
#endif

#endif /* _TRACE_H_ */
