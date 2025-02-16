/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 **/
#ifndef _K_STRUCT_H_
#define _K_STRUCT_H_

#include "type_def.h"
#include "k_linker.h"
#include "k_type.h"
#include "k_config.h"

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
typedef void (*pTimer_callbackFunc_t)(void *);
typedef void (*pTask_callbackFunc_t)(void *);
typedef void (*pThread_entryFunc_t)(void *);
typedef void (*pEvent_callbackFunc_t)(void);
typedef void (*pSubscribe_callbackFunc_t)(const void *, _u16_t);
typedef void (*pTimeout_callbackFunc_t)(void *);
typedef void (*pNotify_callbackFunc_t)(void *);

struct base_head {
    _u8_t cs; // control and status

    const _char_t *pName;
};

struct publish_context {
    struct base_head head;

    list_t q_list;
};
typedef struct publish_context publish_context_t;

struct notify_callback {
    linker_t linker;

    _u32_t updated;

    _u32_t muted;

    void *pData;

    _u16_t len;

    pNotify_callbackFunc_t fn;
};

struct subscribe_callback {
    list_node_t node;

    pSubscribe_callbackFunc_t pSubCallEntry;
};

typedef struct {
    struct base_head head;

    struct publish_context *pPublisher;

    _u32_t accepted;

    struct notify_callback notify;

    struct subscribe_callback call;
} subscribe_context_t;

struct expired_time {
    linker_t linker;

    _u64_t duration_us;

    pTimeout_callbackFunc_t fn;
};

struct timer_callback {
    list_node_t node;

    void *pUserData;

    pTimer_callbackFunc_t pTimerCallEntry;
};

typedef struct {
    struct base_head head;

    _u8_t control;

    _u32_t timeout_ms;

    struct expired_time expire;

    struct timer_callback call;
} timer_context_t;

typedef struct {
    struct base_head head;

    _u8_t remains;

    _u8_t limits;

    _u32_t timeout_ms;

    list_t q_list;
} semaphore_context_t;

typedef struct {
    struct base_head head;

    _b_t locked;

    struct schedule_task *pHoldTask;

    _i16_t originalPriority;

    list_t q_list;
} mutex_context_t;

typedef struct {
    const _u8_t *pUsrBuf;
    _u16_t size;
    _b_t reverse;
} queue_sch_t;

typedef struct {
    struct base_head head;

    void *pQueueBufferAddress;

    _u16_t elementLength;

    _u16_t elementNumber;

    _u16_t leftPosition;

    _u16_t rightPosition;

    _u16_t cacheSize;

    list_t in_QList;

    list_t out_QList;
} queue_context_t;

typedef struct {
    struct base_head head;

    void *pMemAddress;

    _u16_t elementLength;

    _u16_t elementNumber;

    _u32_t elementFreeBits;

    list_t q_list;
} pool_context_t;

typedef struct {
    /* The listen bits*/
    _u32_t listen;

    struct evt_val *pEvtVal;
} event_sch_t;

struct event_callback {
    list_node_t node;

    pTimer_callbackFunc_t pEvtCallEntry;
};

typedef struct {
    struct base_head head;

    /* The event signal value */
    _u32_t value;

    /* Changed bits always trigger = 1, See dirMask below = 0. */
    _u32_t anyMask;

    /* Level trigger = 0, Edge trigger = 1. */
    _u32_t modeMask;

    /* Fall or Low trigger = 0, Rise or high trigger = 1. */
    _u32_t dirMask;

    /* The triggered value */
    _u32_t triggered;

    /* When the event change that meet with edge setting, the function will be called */
    struct event_callback call;

    list_t q_list;
} event_context_t;

struct call_exit {
    list_t *pToList;

    _u32_t timeout_ms;
};

struct call_entry {
    _i32p_t result;

    pTask_callbackFunc_t fun;
};

struct call_analyze {
    _u32_t last_pend_ms;

    _u32_t last_active_ms;

    _u32_t last_run_ms;

    _u32_t total_run_ms;
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

    _u32_t psp;

    _i16_t prior;

    void *pPendCtx;

    void *pPendData;

    struct call_exec exec;

    struct expired_time expire;
};

struct thread_context {
    struct base_head head;

    pThread_entryFunc_t pEntryFunc;

    _u32_t *pStackAddr;

    _u32_t stackSize;

    void *pUserData;

    struct schedule_task task;
};
typedef struct thread_context thread_context_t;

typedef struct {
    struct thread_context *p_thread;
    void *p_arg;
} thread_context_init_t;

/** @brief The rtos kernel structure. */
typedef struct {
    struct schedule_task *pTask;

    _b_t run;

    _u32_t pendsv_ms;

} kernel_context_t;

/* End of section using anonymous unions */
#if defined(__CC_ARM)
#pragma pop
#elif defined(__TASKING__)
#pragma warning restore
#endif

#endif /* _K_STRUCT_H_ */
