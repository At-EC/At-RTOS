/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 **/

#ifndef _KSTRUCT_H_
#define _KSTRUCT_H_

#include "typedef.h"
#include "linker.h"
#include "unique.h"
#include "configuration.h"

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

typedef void (*pCallbackFunc_t)(void);
typedef void (*pTimer_callbackFunc_t)(void);
typedef void (*pThread_callbackFunc_t)(os_id_t);
typedef void (*pThread_entryFunc_t)(void);
typedef void (*pEvent_callbackFunc_t)(void);

typedef struct {
    /* A common struct head to link with other context */
    linker_head_t head;

    b_t isCycle;

    u32_t timeout_ms;

    u64_t duration_us;

    struct callFunc {
        list_node_t node;

        union {
            pTimer_callbackFunc_t pTimer;

            pThread_callbackFunc_t pThread;
        };
    } call;
} timer_context_t;

typedef struct {
    /* A common struct head to link with other context */
    linker_head_t head;

    u8_t initialCount;

    u8_t limitCount;

    b_t isPermit;

    u32_t timeout_ms;

    list_t blockingThreadHead;
} semaphore_context_t;

typedef struct {
    /* A common struct head to link with other context */
    linker_head_t head;

    os_id_t holdThreadId;

    os_priority_t originalPriority;

    list_t blockingThreadHead;
} mutex_context_t;

typedef struct {
    const u8_t *pUserBufferAddress;
    u16_t userBufferSize;
    b_t toFront;
    b_t fromBack;
} action_queue_t;

typedef struct {
    void **ppUserMemAddress;
} action_pool_t;

typedef struct {
    /* A common struct head to link with other context */
    linker_head_t head;

    const void *pQueueBufferAddress;

    u16_t elementLength;

    u16_t elementNumber;

    u16_t leftPosition;

    u16_t rightPosition;

    u16_t cacheSize;

    /* The blocked thread list */
    list_t inBlockingThreadListHead;

    /* The blocked thread list */
    list_t outBlockingThreadListHead;
} queue_context_t;

typedef struct {
    linker_head_t head;

    const void *pMemAddress;

    u16_t elementLength;

    u16_t elementNumber;

    u32_t elementFreeBits;

    list_t blockingThreadHead;
} pool_context_t;

typedef struct {
    /* Current thread listen which bits in the event */
    u32_t listen;

    /* If the trigger is not zero, All changed bits seen can wake up the thread to handle event */
    u32_t trigger;

    /* The value store which bits store until it's meet with the group setting it's can be clean */
    u32_t *pStore;
} action_event_t;

typedef struct {
    /* A common struct head to link with other context */
    linker_head_t head;

    /* The event value */
    u32_t set;

    /* Indicate which bit change can trigger event callback function */
    u32_t edge;

    /* When the event change that meet with edge setting, the function will be called */
    pEvent_callbackFunc_t pCallbackFunc;

    /* The blocked thread list */
    list_t blockingThreadHead;
} event_context_t;

typedef struct {
    list_t *pToList;

    u32_t timeout_us;

    pThread_callbackFunc_t pTimeoutCallFun;
} thread_exit_t;

typedef struct {
    os_id_t release;

    u32_t result;

    pThread_callbackFunc_t pEntryCallFun;
} thread_entry_t;

typedef struct {
    u32_t pend_ms;

    u32_t run_ms;

    u32_t exit_ms;

    u32_t active_ms;

    u32_t cycle_ms;

    u16_t percent;
} analyze_t;

typedef struct {
    os_id_t hold;

#if defined KTRACE
    analyze_t analyze;
#endif

    union {
        thread_exit_t exit;

        thread_entry_t entry;
    };
} action_schedule_t;

typedef struct {
    /* A common struct head to link with other context */
    linker_head_t head;

    os_priority_t priority;

    pThread_entryFunc_t pEntryFunc;

    u32_t *pStackAddr;

    u32_t stackSize;

    u32_t PSPStartAddr;

    action_event_t event;

    action_queue_t queue;

    action_pool_t pool;

    action_schedule_t schedule;
} thread_context_t;

/** @brief The kernal object type enum. */
enum {
    KERNAL_MEMBER_THREAD = 0u,
    KERNAL_MEMBER_TIMER_INTERNAL,
    KERNAL_MEMBER_TIMER,
    KERNAL_MEMBER_SEMAPHORE,
    KERNAL_MEMBER_MUTEX,
    KERNAL_MEMBER_EVENT,
    KERNAL_MEMBER_QUEUE,
    KERNAL_MEMBER_POOL,
    KERNAL_MEMBER_NUMBER,
};

enum {
    KERNAL_MEMBER_LIST_THREAD_WAIT = 0u,
    KERNAL_MEMBER_LIST_THREAD_ENTRY,
    KERNAL_MEMBER_LIST_THREAD_EXIT,

    KERNAL_MEMBER_LIST_TIMER_STOP,
    KERNAL_MEMBER_LIST_TIMER_WAIT,
    KERNAL_MEMBER_LIST_TIMER_END,
    KERNAL_MEMBER_LIST_TIMER_PEND,
    KERNAL_MEMBER_LIST_TIMER_RUN,

    KERNAL_MEMBER_LIST_SEMAPHORE_LOCK,
    KERNAL_MEMBER_LIST_SEMAPHORE_UNLOCK,

    KERNAL_MEMBER_LIST_MUTEX_LOCK,
    KERNAL_MEMBER_LIST_MUTEX_UNLOCK,

    KERNAL_MEMBER_LIST_EVENT_INACTIVE,
    KERNAL_MEMBER_LIST_EVENT_ACTIVE,

    KERNAL_MEMBER_LIST_QUEUE_INIT,

    KERNAL_MEMBER_LIST_POOL_INIT,
    KERNAL_MEMBER_LIST_NUMBER,
};

typedef struct {
    u32_t mem;
    u32_t list;
} kernal_member_setting_t;

typedef struct {
    const u8_t *pMemoryContainer;

    const list_t *pListContainer;

    kernal_member_setting_t *pSetting;
} kernal_member_t;

/** @brief The rtos kernal structure. */
typedef struct {
    /* The current running thread */
    os_id_t current;

    list_t list;

    kernal_member_t member;

    /* The kernal already start to do schedule */
    b_t run;

    u32_t pendsv_ms;
} kernal_context_t;

typedef struct {
    union {
        u32_t size;
        u8_t priority;
        const char_t *pName;
    };
} os_thread_symbol_t;

/* End of section using anonymous unions */
#if defined(__CC_ARM)
#pragma pop
#elif defined(__TASKING__)
#pragma warning restore
#endif

#ifdef __cplusplus
}
#endif

#endif /* _KSTRUCT_H_ */
