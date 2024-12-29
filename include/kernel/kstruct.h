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
#include "ktype.h"
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
typedef void (*pSubscribe_callbackFunc_t)(const void *, u16_t);
typedef void (*pTimeout_callbackFunc_t)(void *);
typedef void (*pNotify_callbackFunc_t)(void *);

#define CS_INITED (1u)

struct base_head {
    u8_t cs; // control and status

    const char_t *pName;
};

struct publish_context {
    struct base_head head;

    list_t subscribeListHead;
};
typedef struct publish_context publish_context_t;

struct notify_callback {
    linker_t linker;

    u32_t updated;

    u32_t muted;

    void *pData;

    u16_t len;

    pNotify_callbackFunc_t fn;
};

struct subscribe_callback {
    list_node_t node;

    pSubscribe_callbackFunc_t pSubCallEntry;
};

typedef struct {
    struct base_head head;

    struct publish_context *pPublisher;

    u32_t accepted;

    struct notify_callback notify;

    struct subscribe_callback call;
} subscribe_context_t;

struct expired_time {
    linker_t linker;

    u64_t duration_us;

    pTimeout_callbackFunc_t fn;
};

struct timer_callback {
    list_node_t node;

    pTimer_callbackFunc_t pTimerCallEntry;
};

typedef struct {
    struct base_head head;

    u8_t control;

    u32_t timeout_ms;

    struct expired_time expire;

    struct timer_callback call;
} timer_context_t;

typedef struct {
    struct base_head head;

    u8_t remains;

    u8_t limits;

    u32_t timeout_ms;

    list_t blockingThreadHead;
} semaphore_context_t;

typedef struct {
    struct base_head head;

    b_t locked;

    struct thread_context *pHoldThread;

    os_priority_t originalPriority;

    list_t blockingThreadHead;
} mutex_context_t;

typedef struct {
    const u8_t *pUsrBuf;
    u16_t size;
    b_t reverse;
} queue_sch_t;

typedef struct {
    struct base_head head;

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
    struct base_head head;

    const void *pMemAddress;

    u16_t elementLength;

    u16_t elementNumber;

    u32_t elementFreeBits;

    list_t blockingThreadHead;
} pool_context_t;

typedef struct {
    /* The listen bits*/
    u32_t listen;

    os_evt_val_t *pEvtVal;
} event_sch_t;

struct event_callback {
    list_node_t node;

    pTimer_callbackFunc_t pEvtCallEntry;
};

typedef struct {
    struct base_head head;

    /* The event signal value */
    u32_t value;

    /* Changed bits always trigger = 1, See dirMask below = 0. */
    u32_t anyMask;

    /* Level trigger = 0, Edge trigger = 1. */
    u32_t modeMask;

    /* Fall or Low trigger = 0, Rise or high trigger = 1. */
    u32_t dirMask;

    /* The triggered value */
    u32_t triggered;

    /* When the event change that meet with edge setting, the function will be called */
    struct event_callback call;

    /* The blocked thread list */
    list_t blockingThreadHead;
} event_context_t;

typedef struct {
    list_t *pToList;

    u32_t timeout_us;
} thread_exit_t;

typedef struct {
    i32p_t result;

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
    void *pPendCtx;

    void *pPendData;

#if defined KTRACE
    analyze_t analyze;
#endif

    union {
        thread_exit_t exit;

        thread_entry_t entry;
    };
} action_schedule_t;



struct thread_context {
    /* A common struct head to link with other context */
    linker_head_t head;

    os_priority_t priority;

    pThread_entryFunc_t pEntryFunc;

    u32_t *pStackAddr;

    u32_t stackSize;

    u32_t psp;

    struct expired_time expire;

    action_schedule_t schedule;
};
typedef struct thread_context thread_context_t;

/** @brief The kernel object type enum. */
enum {
    KERNEL_MEMBER_THREAD = 0u,
    KERNEL_MEMBER_TIMER_INTERNAL,
    KERNEL_MEMBER_TIMER,
    KERNEL_MEMBER_SEMAPHORE,
    KERNEL_MEMBER_MUTEX,
    KERNEL_MEMBER_EVENT,
    KERNEL_MEMBER_QUEUE,
    KERNEL_MEMBER_POOL,
    KERNEL_MEMBER_PUBLISH,
    KERNEL_MEMBER_SUBSCRIBE,
    KERNEL_MEMBER_NUMBER,
};

enum {
    KERNEL_MEMBER_LIST_THREAD_WAIT = 0u,
    KERNEL_MEMBER_LIST_THREAD_ENTRY,
    KERNEL_MEMBER_LIST_THREAD_EXIT,

    KERNEL_MEMBER_LIST_TIMER_STOP,
    KERNEL_MEMBER_LIST_TIMER_WAIT,
    KERNEL_MEMBER_LIST_TIMER_END,
    KERNEL_MEMBER_LIST_TIMER_PEND,
    KERNEL_MEMBER_LIST_TIMER_RUN,

    KERNEL_MEMBER_LIST_SEMAPHORE_INIT,

    KERNEL_MEMBER_LIST_MUTEX_LOCK,
    KERNEL_MEMBER_LIST_MUTEX_UNLOCK,

    KERNEL_MEMBER_LIST_EVENT_INIT,

    KERNEL_MEMBER_LIST_QUEUE_INIT,

    KERNEL_MEMBER_LIST_POOL_INIT,

    KERNEL_MEMBER_LIST_PUBLISH_INIT,
    KERNEL_MEMBER_LIST_PUBLISH_PEND,
    KERNEL_MEMBER_LIST_SUBSCRIBE_INIT,

    KERNEL_MEMBER_LIST_NUMBER,
};

typedef struct {
    u32_t mem;
    u32_t list;
} kernel_member_setting_t;

typedef struct {
    const u8_t *pMemoryContainer;

    const list_t *pListContainer;

    kernel_member_setting_t *pSetting;
} kernel_member_t;

/** @brief The rtos kernel structure. */
typedef struct {
    /* The current running thread */
    os_id_t current;

    list_t list;

    kernel_member_t member;

    /* The kernel already start to do schedule */
    b_t run;

    u32_t pendsv_ms;
} kernel_context_t;

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
