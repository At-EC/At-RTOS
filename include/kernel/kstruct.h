/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 **/
#ifndef _KSTRUCT_H_
#define _KSTRUCT_H_

#include "type_def.h"
#include "linker.h"
#include "ktype.h"
#include "configuration.h"

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
typedef void (*pTask_callbackFunc_t)(void *);
typedef void (*pThread_entryFunc_t)(void);
typedef void (*pEvent_callbackFunc_t)(void);
typedef void (*pSubscribe_callbackFunc_t)(const void *, u16_t);
typedef void (*pTimeout_callbackFunc_t)(void *);
typedef void (*pNotify_callbackFunc_t)(void *);

struct base_head {
    u8_t cs; // control and status

    const char_t *pName;
};

struct publish_context {
    struct base_head head;

    list_t q_list;
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

    list_t q_list;
} semaphore_context_t;

typedef struct {
    struct base_head head;

    b_t locked;

    struct schedule_task *pHoldTask;

    i16_t originalPriority;

    list_t q_list;
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

    list_t in_QList;

    list_t out_QList;
} queue_context_t;

typedef struct {
    struct base_head head;

    const void *pMemAddress;

    u16_t elementLength;

    u16_t elementNumber;

    u32_t elementFreeBits;

    list_t q_list;
} pool_context_t;

typedef struct {
    /* The listen bits*/
    u32_t listen;

    struct evt_val *pEvtVal;
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

    list_t q_list;
} event_context_t;

struct call_exit {
    list_t *pToList;

    u32_t timeout_ms;
};

struct call_entry {
    i32p_t result;

    pTask_callbackFunc_t fun;
};

struct call_analyze {
    u32_t last_pend_ms;

    u32_t last_active_ms;

    u32_t last_run_ms;

    u32_t total_run_ms;
};

struct call_exec {
    union {
        struct call_exit exit;

        struct call_entry entry;
    };

    struct call_analyze analyze;
};

struct schedule_task {
    linker_t linker;

    u32_t psp;

    i16_t prior;

    void *pPendCtx;

    void *pPendData;

    struct call_exec exec;

    struct expired_time expire;
};

struct thread_context {
    struct base_head head;

    pThread_entryFunc_t pEntryFunc;

    u32_t *pStackAddr;

    u32_t stackSize;

    void *pUserData;

    struct schedule_task task;
};
typedef struct thread_context thread_context_t;

typedef struct {
    struct thread_context *pThread;
} thread_context_init_t;

/** @brief The rtos kernel structure. */
typedef struct {
    struct schedule_task *pTask;

    b_t run;

    u32_t pendsv_ms;

} kernel_context_t;

/* End of section using anonymous unions */
#if defined(__CC_ARM)
#pragma pop
#elif defined(__TASKING__)
#pragma warning restore
#endif

#endif /* _KSTRUCT_H_ */
