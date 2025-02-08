/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 **/
#ifndef _STATIC_INIT_H_
#define _STATIC_INIT_H_

#include "type_def.h"
#include "k_struct.h"

#if (__ARMCC_VERSION)
#define INIT_SECTION_FUNC _INIT_FUNC_LIST
#define INIT_SECTION_OS_THREAD_STATIC _INIT_OS_THREAD_STATIC
#define INIT_SECTION_OS_THREAD_LIST _INIT_OS_THREAD_LIST
#define INIT_SECTION_OS_TIMER_LIST _INIT_OS_TIMER_LIST
#define INIT_SECTION_OS_SEMAPHORE_LIST _INIT_OS_SEMAPHORE_LIST
#define INIT_SECTION_OS_MUTEX_LIST _INIT_OS_MUTEX_LIST
#define INIT_SECTION_OS_EVENT_LIST  _INIT_OS_EVENT_LIST
#define INIT_SECTION_OS_QUEUE_LIST _INIT_OS_QUEUE_LIST
#define INIT_SECTION_OS_POOL_LIST  _INIT_OS_POOL_LIST
#define INIT_SECTION_OS_PUBLISH_LIST _INIT_OS_PUBLISH_LIST
#define INIT_SECTION_OS_SUBSCRIBE_LIST  _INIT_OS_SUBSCRIBE_LIST
#elif defined(__ICCARM__)
#define INIT_SECTION_FUNC "_INIT_FUNC_LIST"
#pragma section = INIT_SECTION_FUNC
#define INIT_SECTION_OS_THREAD_STATIC "_INIT_OS_THREAD_STATIC"
#pragma section = INIT_SECTION_OS_THREAD_STATIC

#define INIT_SECTION_OS_THREAD_LIST "_INIT_OS_THREAD_LIST"
#pragma section = INIT_SECTION_OS_THREAD_LIST

#define INIT_SECTION_OS_TIMER_LIST "_INIT_OS_TIMER_LIST"
#pragma section = INIT_SECTION_OS_TIMER_LIST

#define INIT_SECTION_OS_SEMAPHORE_LIST "_INIT_OS_SEMAPHORE_LIST"
#pragma section = INIT_SECTION_OS_SEMAPHORE_LIST

#define INIT_SECTION_OS_MUTEX_LIST "_INIT_OS_MUTEX_LIST"
#pragma section = INIT_SECTION_OS_MUTEX_LIST

#define INIT_SECTION_OS_EVENT_LIST  "_INIT_OS_EVENT_LIST"
#pragma section = INIT_SECTION_OS_EVENT_LIST

#define INIT_SECTION_OS_QUEUE_LIST "_INIT_OS_QUEUE_LIST"
#pragma section = INIT_SECTION_OS_QUEUE_LIST

#define INIT_SECTION_OS_POOL_LIST  "_INIT_OS_POOL_LIST"
#pragma section = INIT_SECTION_OS_POOL_LIST

#define INIT_SECTION_OS_PUBLISH_LIST "_INIT_OS_PUBLISH_LIST"
#pragma section = INIT_SECTION_OS_PUBLISH_LIST

#define INIT_SECTION_OS_SUBSCRIBE_LIST  "_INIT_OS_SUBSCRIBE_LIST"
#pragma section = INIT_SECTION_OS_SUBSCRIBE_LIST

#elif defined(__GNUC__)
#error "not supported __GNUC__ compiler"
#else
#error "not supported compiler"
#endif

#if (__ARMCC_VERSION)
#define INIT_SECTION_BEGIN(name) name##$$Base
#define INIT_SECTION_END(name)   name##$$Limit
#define INIT_SECTION(name)       __attribute__((section(#name)))
#define INIT_USED                __attribute__((used))

#define INIT_FUNC_DEFINE(handler, level)                                                                                                   \
    INIT_USED init_func_t _init_##handler##_func INIT_SECTION(_INIT_FUNC_LIST) = {handler, level}

#define INIT_OS_THREAD_RUNTIME_NUM_DEFINE(num)                                                                                             \
    INIT_USED thread_context_t _init_runtime_thread[num] INIT_SECTION(_INIT_OS_THREAD_LIST) = {0}

#define INIT_OS_THREAD_DEFINE(id_name, priority, stack_size, pEntryFn, pArg)                                                               \
    STACK_STATIC_VALUE_DEFINE(id_name##_stack, stack_size);                                                                                \
    INIT_USED thread_context_t _init_##id_name##_thread INIT_SECTION(_INIT_OS_THREAD_LIST) =                                               \
        {.head = {.cs = CS_INITED, .pName = #id_name},                                                                                     \
         .pStackAddr = id_name##_stack,                                                                                                    \
         .stackSize = stack_size,                                                                                                          \
         .pEntryFunc = pEntryFn,                                                                                                           \
         .task = {.prior = priority, .psp = 0u}};                                                                                          \
    INIT_USED thread_context_init_t _init_##id_name##_thread_init INIT_SECTION(_INIT_OS_THREAD_STATIC) =                                   \
        {.p_thread = &_init_##id_name##_thread, .p_arg = pArg};                                                                            \
    os_thread_id_t id_name = {.p_val = (void*)&_init_##id_name##_thread, .pName = #id_name}

#define INIT_OS_TIMER_RUNTIME_NUM_DEFINE(num)                                                                                              \
    INIT_USED timer_context_t _init_runtime_timer[num] INIT_SECTION(_INIT_OS_TIMER_LIST) = {0}

extern void timer_callback_fromTimeOut(void *pNode);
#define INIT_OS_TIMER_DEFINE(id_name, pEntryFunc)                                                                                          \
    INIT_USED timer_context_t _init_##id_name##_timer INIT_SECTION(_INIT_OS_TIMER_LIST) =                                                  \
        {.head = {.cs = CS_INITED, .pName = #id_name},                                                                                     \
         .expire = {.fn = timer_callback_fromTimeOut},                                                                                     \
         .call = {.pTimerCallEntry = pEntryFunc}};                                                                                         \
    os_timer_id_t id_name = {.p_val = (void*)&_init_##id_name##_timer, .pName = #id_name}

#define INIT_OS_SEM_RUNTIME_NUM_DEFINE(num)                                                                                                \
    INIT_USED semaphore_context_t _init_runtime_sem[num] INIT_SECTION(_INIT_OS_SEMAPHORE_LIST) = {0}

#define INIT_OS_SEMAPHORE_DEFINE(id_name, remain, limit)                                                                                   \
    INIT_USED semaphore_context_t _init_##id_name##_sem INIT_SECTION(_INIT_OS_SEMAPHORE_LIST) =                                            \
        {.head = {.cs = CS_INITED, .pName = #id_name},                                                                                     \
         .remains = remain,                                                                                                                \
         .limits = limit};                                                                                                                 \
    os_sem_id_t id_name = {.p_val = (void*)&_init_##id_name##_sem, .pName = #id_name}

#define INIT_OS_MUTEX_RUNTIME_NUM_DEFINE(num)                                                                                              \
    INIT_USED mutex_context_t _init_runtime_mutex[num] INIT_SECTION(_INIT_OS_MUTEX_LIST) = {0}

#define INIT_OS_MUTEX_DEFINE(id_name)                                                                                                      \
    INIT_USED mutex_context_t _init_##id_name##_mutex INIT_SECTION(_INIT_OS_MUTEX_LIST) =                                                  \
        {.head = {.cs = CS_INITED, .pName = #id_name},                                                                                     \
         .locked = false,                                                                                                                  \
         .pHoldTask = NULL,                                                                                                                \
         .originalPriority = OS_PRIOTITY_INVALID_LEVEL};                                                                                   \
    os_mutex_id_t id_name = {.p_val = (void*)&_init_##id_name##_mutex, .pName = #id_name}

#define INIT_OS_EVT_RUNTIME_NUM_DEFINE(num)                                                                                                \
    INIT_USED event_context_t _init_runtime_evt[num] INIT_SECTION(_INIT_OS_EVENT_LIST) = {0}

#define INIT_OS_EVT_DEFINE(id_name, anyMask, modeMask, dirMask, init)                                                                      \
    INIT_USED event_context_t _init_##id_name##_evt INIT_SECTION(_INIT_OS_EVENT_LIST) =                                                    \
        {.head = {.cs = CS_INITED, .pName = #id_name},                                                                                     \
         .value = init,                                                                                                                    \
         .triggered = 0u,                                                                                                                  \
         .anyMask = anyMask,                                                                                                               \
         .modeMask = modeMask,                                                                                                             \
         .dirMask = dirMask};                                                                                                              \
    os_evt_id_t id_name = {.p_val = (void*)&_init_##id_name##_evt, .pName = #id_name}

#define INIT_OS_MSGQ_RUNTIME_NUM_DEFINE(num)                                                                                               \
    INIT_USED queue_context_t _init_runtime_msgq[num] INIT_SECTION(_INIT_OS_QUEUE_LIST) = {0}

#define INIT_OS_MSGQ_DEFINE(id_name, pBufAddr, len, num)                                                                                   \
    INIT_USED queue_context_t _init_##id_name##_msgq INIT_SECTION(_INIT_OS_QUEUE_LIST) =                                                   \
        {.head = {.cs = CS_INITED, .pName = #id_name},                                                                                     \
         .pQueueBufferAddress = pBufAddr,                                                                                                  \
         .elementLength = len,                                                                                                             \
         .elementNumber = num,                                                                                                             \
         .leftPosition = 0u,                                                                                                               \
         .rightPosition = 0u,                                                                                                              \
         .cacheSize = 0u};                                                                                                                 \
    os_msgq_id_t id_name = {.p_val = (void*)&_init_##id_name##_msgq, .pName = #id_name}

#define INIT_OS_POOL_RUNTIME_NUM_DEFINE(num)                                                                                               \
    INIT_USED pool_context_t _init_runtime_pool[num] INIT_SECTION(_INIT_OS_POOL_LIST) = {0}

#define INIT_OS_POOL_DEFINE(id_name, pMemAddr, len, num)                                                                                   \
    INIT_USED pool_context_t _init_##id_name##_pool INIT_SECTION(_INIT_OS_POOL_LIST) =                                                     \
        {.head = {.cs = CS_INITED, .pName = #id_name},                                                                                     \
         .pMemAddress = pMemAddr,                                                                                                          \
         .elementLength = len,                                                                                                             \
         .elementNumber = num,                                                                                                             \
         .elementFreeBits = Bs(0u, (num - 1u))};                                                                                           \
    os_pool_id_t id_name = {.p_val = (void*)&_init_##id_name##_pool, .pName = #id_name}

#define INIT_OS_SUBSCRIBE_RUNTIME_NUM_DEFINE(num)                                                                                          \
    INIT_USED subscribe_context_t _init_runtime_subscribe[num] INIT_SECTION(_INIT_OS_SUBSCRIBE_LIST) = {0}

extern void subscribe_notification(void *pLinker);
#define INIT_OS_SUBSCRIBE_DEFINE(id_name, pDataAddr, size)                                                                                 \
    INIT_USED subscribe_context_t _init_##id_name##_subscribe INIT_SECTION(_INIT_OS_SUBSCRIBE_LIST) =                                      \
        {.head = {.cs = CS_INITED, .pName = #id_name},                                                                                     \
         .pPublisher = NULL,                                                                                                               \
         .accepted = 0u,                                                                                                                   \
         .notify = {.pData = pData, .len = size, muted = false, .fn = subscribe_notification}};                                            \
    os_subscribe_id_t id_name = {.p_val = (void*)&_init_##id_name##_subscribe, .pName = #id_name}

#define INIT_OS_PUBLISH_RUNTIME_NUM_DEFINE(num)                                                                                            \
    INIT_USED publish_context_t _init_runtime_publish[num] INIT_SECTION(_INIT_OS_PUBLISH_LIST) = {0}

#define INIT_OS_PUBLISH_DEFINE(id_name, pDataAddr, size)                                                                                   \
    INIT_USED publish_context_t _init_##id_name##_publish INIT_SECTION(_INIT_OS_PUBLISH_LIST) =                                            \
        {.head = {.cs = CS_INITED, .pName = #id_name}};                                                                                    \
    os_publish_id_t id_name = {.p_val = (void*)&_init_##id_name##_publish, .pName = #id_name}

#elif defined(__ICCARM__)
#pragma diag_suppress = Pm086
#define INIT_SECTION(name)       @name
#define INIT_SECTION_BEGIN(name) __section_begin(name)
#define INIT_SECTION_END(name)  __section_end(name)

#define INIT_FUNC_DEFINE(handler, level)                                                                                                   \
    static __root const init_func_t _init_##handler##_func @ "_INIT_FUNC_LIST" = {handler, level}

#define INIT_OS_THREAD_RUNTIME_NUM_DEFINE(num)                                                                                             \
    static __root thread_context_t _init_runtime_thread[num] @ "_INIT_OS_THREAD_LIST" = {0}

#define INIT_OS_THREAD_DEFINE(id_name, priority, stack_size, pEntryFn, pArg)                                                               \
    STACK_STATIC_VALUE_DEFINE(id_name##_stack, stack_size);                                                                                \
    static __root thread_context_t _init_##id_name##_thread @ "_INIT_OS_THREAD_LIST" =                                                     \
        {.head = {.cs = CS_INITED, .pName = #id_name},                                                                                     \
         .pStackAddr = id_name##_stack,                                                                                                    \
         .stackSize = stack_size,                                                                                                          \
         .pEntryFunc = pEntryFn,                                                                                                           \
         .task = {.prior = priority, .psp = 0u}};                                                                                          \
    static __root thread_context_init_t _init_##id_name##_thread_init @ "_INIT_OS_THREAD_STATIC" =                                         \
        {.p_thread = &_init_##id_name##_thread, .p_arg = pArg};                                                                            \
    os_thread_id_t id_name = {.p_val = (void*)&_init_##id_name##_thread, .pName = #id_name}

#define INIT_OS_TIMER_RUNTIME_NUM_DEFINE(num)                                                                                              \
    static __root timer_context_t _init_runtime_timer[num] @ "_INIT_OS_TIMER_LIST" = {0}

extern void timer_callback_fromTimeOut(void *pNode);
#define INIT_OS_TIMER_DEFINE(id_name, pEntryFunc)                                                                                          \
    static __root timer_context_t _init_##id_name##_timer @ "_INIT_OS_TIMER_LIST" =                                                        \
        {.head = {.cs = CS_INITED, .pName = #id_name},                                                                                     \
         .expire = {.fn = timer_callback_fromTimeOut},                                                                                     \
         .call = {.pTimerCallEntry = pEntryFunc}};                                                                                         \
    os_timer_id_t id_name = {.p_val = (void*)&_init_##id_name##_timer, .pName = #id_name}

#define INIT_OS_SEM_RUNTIME_NUM_DEFINE(num)                                                                                                \
    static __root semaphore_context_t _init_runtime_sem[num] @ "_INIT_OS_SEMAPHORE_LIST" = {0}

#define INIT_OS_SEMAPHORE_DEFINE(id_name, remain, limit)                                                                                   \
    static __root semaphore_context_t _init_##id_name##_sem @ "_INIT_OS_SEMAPHORE_LIST" =                                                  \
        {.head = {.cs = CS_INITED, .pName = #id_name},                                                                                     \
         .remains = remain,                                                                                                                \
         .limits = limit};                                                                                                                 \
    os_sem_id_t id_name = {.p_val = (void*)&_init_##id_name##_sem, .pName = #id_name}

#define INIT_OS_MUTEX_RUNTIME_NUM_DEFINE(num)                                                                                              \
    static __root mutex_context_t _init_runtime_mutex[num] @ "_INIT_OS_MUTEX_LIST" = {0}

#define INIT_OS_MUTEX_DEFINE(id_name)                                                                                                      \
    static __root mutex_context_t _init_##id_name##_mutex @ "_INIT_OS_MUTEX_LIST" =                                                        \
        {.head = {.cs = CS_INITED, .pName = #id_name},                                                                                     \
         .locked = false,                                                                                                                  \
         .pHoldTask = NULL,                                                                                                                \
         .originalPriority = OS_PRIOTITY_INVALID_LEVEL};                                                                                   \
    os_mutex_id_t id_name = {.p_val = (void*)&_init_##id_name##_mutex, .pName = #id_name}

#define INIT_OS_EVT_RUNTIME_NUM_DEFINE(num)                                                                                                \
    static __root event_context_t _init_runtime_evt[num] @ "_INIT_OS_EVENT_LIST" = {0}

#define INIT_OS_EVT_DEFINE(id_name, anyMask, modeMask, dirMask, init)                                                                      \
    static __root event_context_t _init_##id_name##_evt @ "_INIT_OS_EVENT_LIST" =                                                          \
        {.head = {.cs = CS_INITED, .pName = #id_name},                                                                                     \
         .value = init,                                                                                                                    \
         .triggered = 0u,                                                                                                                  \
         .anyMask = anyMask,                                                                                                               \
         .modeMask = modeMask,                                                                                                             \
         .dirMask = dirMask};                                                                                                              \
    os_evt_id_t id_name = {.p_val = (void*)&_init_##id_name##_evt, .pName = #id_name}

#define INIT_OS_MSGQ_RUNTIME_NUM_DEFINE(num)                                                                                               \
    static __root queue_context_t _init_runtime_msgq[num] @ "_INIT_OS_QUEUE_LIST" = {0}

#define INIT_OS_MSGQ_DEFINE(id_name, pBufAddr, len, num)                                                                                   \
    static __root queue_context_t _init_##id_name##_msgq @ "_INIT_OS_QUEUE_LIST" =                                                         \
        {.head = {.cs = CS_INITED, .pName = #id_name},                                                                                     \
         .pQueueBufferAddress = pBufAddr,                                                                                                  \
         .elementLength = len,                                                                                                             \
         .elementNumber = num,                                                                                                             \
         .leftPosition = 0u,                                                                                                               \
         .rightPosition = 0u,                                                                                                              \
         .cacheSize = 0u};                                                                                                                 \
    os_msgq_id_t id_name = {.p_val = (void*)&_init_##id_name##_msgq, .pName = #id_name}

#define INIT_OS_POOL_RUNTIME_NUM_DEFINE(num)                                                                                               \
    static __root pool_context_t _init_runtime_pool[num] @ "_INIT_OS_POOL_LIST" = {0}

#define INIT_OS_POOL_DEFINE(id_name, pMemAddr, len, num)                                                                                   \
    static __root pool_context_t _init_##id_name##_pool @ "_INIT_OS_POOL_LIST" =                                                           \
        {.head = {.cs = CS_INITED, .pName = #id_name},                                                                                     \
         .pMemAddress = pMemAddr,                                                                                                          \
         .elementLength = len,                                                                                                             \
         .elementNumber = num,                                                                                                             \
         .elementFreeBits = Bs(0u, (num - 1u))};                                                                                           \
    os_pool_id_t id_name = {.p_val = (void*)&_init_##id_name##_pool, .pName = #id_name}

#define INIT_OS_SUBSCRIBE_RUNTIME_NUM_DEFINE(num)                                                                                          \
    static __root subscribe_context_t _init_runtime_subscribe[num] @ "_INIT_OS_SUBSCRIBE_LIST" = {0}

    extern void subscribe_notification(void *pLinker);
#define INIT_OS_SUBSCRIBE_DEFINE(id_name, pDataAddr, size)                                                                                 \
    static __root subscribe_context_t _init_##id_name##_subscribe @ "_INIT_OS_SUBSCRIBE_LIST" =                                            \
        {.head = {.cs = CS_INITED, .pName = #id_name},                                                                                     \
         .pPublisher = NULL,                                                                                                               \
         .accepted = 0u,                                                                                                                   \
         .notify = {.pData = pData, .len = size, muted = false, .fn = subscribe_notification}};                                            \
    os_subscribe_id_t id_name = {.p_val = (void*)&_init_##id_name##_subscribe, .pName = #id_name}

#define INIT_OS_PUBLISH_RUNTIME_NUM_DEFINE(num)                                                                                            \
    static __root publish_context_t _init_runtime_publish[num] @ "_INIT_OS_PUBLISH_LIST" = {0}

#define INIT_OS_PUBLISH_DEFINE(id_name, pDataAddr, size)                                                                                   \
    static __root publish_context_t _init_##id_name##_publish @ "_INIT_OS_PUBLISH_LIST" =                                                  \
        {.head = {.cs = CS_INITED, .pName = #id_name}};                                                                                    \
    os_publish_id_t id_name = {.p_val = (void*)&_init_##id_name##_publish, .pName = #id_name}

#pragma diag_default = Pm086
#elif defined(__GNUC__)
#error "not supported __GNUC__ compiler"
#else
#error "not supported compiler"
#endif

#if (__ARMCC_VERSION)
#define INIT_SECTION_FIRST(i_section, o_begin)                                                                                             \
    do {                                                                                                                                   \
        extern const int INIT_SECTION_BEGIN(i_section);                                                                                    \
        o_begin = (_u32_t)&INIT_SECTION_BEGIN(i_section);                                                                                  \
    } while(0)

#define INIT_SECTION_LAST(i_section, o_end)                                                                                                \
    do {                                                                                                                                   \
        extern const int INIT_SECTION_END(i_section);                                                                                      \
        o_end = (_u32_t)&INIT_SECTION_END(i_section);                                                                                      \
    } while(0)

#define INIT_SECTION_FOREACH(section, type, item)                                                                                          \
    extern const int INIT_SECTION_BEGIN(section);                                                                                          \
    extern const int INIT_SECTION_END(section);                                                                                            \
    for (type *item = (type *)&INIT_SECTION_BEGIN(section); item < (type *)&INIT_SECTION_END(section); item++)
#elif defined(__ICCARM__)
#define INIT_SECTION_FIRST(i_section, o_begin)                                                                                             \
    do {                                                                                                                                   \
        o_begin = (_u32_t)INIT_SECTION_BEGIN(i_section);                                                                                   \
    } while(0)

#define INIT_SECTION_LAST(i_section, o_end)                                                                                                \
    do {                                                                                                                                   \
        o_end = (_u32_t)INIT_SECTION_END(i_section);                                                                                       \
    } while(0)

#define INIT_SECTION_FOREACH(i_section, type, item)                                                                                        \
    for (type *item = (type *)INIT_SECTION_BEGIN(i_section); item < (type *)INIT_SECTION_END(i_section); item++)

#elif defined(__GNUC__)
#error "not supported __GNUC__ compiler"
#else
#error "not supported compiler"
#endif

enum {
    INIT_LEVEL_0,
    INIT_LEVEL_1,
    INIT_LEVEL_2,
    INIT_LEVEL_3,
    INIT_LEVEL_4,
    INIT_LEVEL_NUM,
};

typedef void (*init_func)(void);
typedef struct {
    init_func func;
    _u8_t level;
} init_func_t;

void init_func_list(void);
void init_func_level(_u8_t level);
void init_static_thread_list(void);

#endif
