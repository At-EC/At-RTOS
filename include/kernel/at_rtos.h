/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 **/
#ifndef _AT_RTOS_H_
#define _AT_RTOS_H_

#include "ktype.h"
#include "kstruct.h"
#include "configuration.h"
#include "postcode.h"
#include "trace.h"
#include "init.h"

#define OS_PC_OK          (PC_OS_OK)
#define OS_PC_TIMEOUT     (PC_OS_WAIT_TIMEOUT)
#define OS_PC_AVAILABLE   (PC_OS_WAIT_AVAILABLE)
#define OS_PC_UNAVAILABLE (PC_OS_WAIT_UNAVAILABLE)
#define OS_PC_NODATA      (PC_OS_WAIT_NODATA)

#define OS_ID_INVALID (OS_INVALID_ID_VAL)

#define OS_SEM_LIMIT_BINARY (1u)

#define OS_TIME_NOWAIT       (OS_TIME_NOWAIT_VAL)
#define OS_TIME_WAIT_FOREVER (OS_TIME_FOREVER_VAL)
typedef u32_t os_timeout_t;

typedef enum {
    OS_TIMER_CTRL_ONCE = (TIMER_CTRL_ONCE_VAL),
    OS_TIMER_CTRL_CYCLE = (TIMER_CTRL_CYCLE_VAL),
    OS_TIMER_CTRL_TEMPORARY = (TIMER_CTRL_TEMPORARY_VAL),
} os_timer_ctrl_t;

typedef struct os_id os_thread_id_t;
typedef struct os_id os_timer_id_t;
typedef struct os_id os_sem_id_t;
typedef struct os_id os_mutex_id_t;
typedef struct os_id os_evt_id_t;
typedef struct os_id os_msgq_id_t;
typedef struct os_id os_pool_id_t;
typedef struct os_id os_publish_id_t;
typedef struct os_id os_subscribe_id_t;

typedef struct evt_val os_evt_val_t;

#define OS_ID_SET(p_handle, u32_value) p_handle->u32_val = (u32_value)

#define OS_PRIORITY_INVALID             (OS_PRIOTITY_INVALID_LEVEL)
#define OS_PRIORITY_APPLICATION_HIGHEST (OS_PRIORITY_APPLICATION_HIGHEST_LEVEL)
#define OS_PRIORITY_APPLICATION_LOWEST  (OS_PRIORITY_APPLICATION_LOWEST_LEVEL)

#define OS_PRIORITY_PREEMPT_SET(p)     (p)
#define OS_PRIORITY_COOPERATION_SET(c) (-(OS_PRIOTITY_COOPERATION_NUM - (c)))

#define OS_STACK_INIT(name, size)                               STACK_STATIC_VALUE_DEFINE(name, size)
#define OS_THREAD_INIT(id_name, priority, stack_size, pEntryFn) INIT_OS_THREAD_DEFINE(id_name, priority, stack_size, pEntryFn)
#define OS_TIMER_INIT(id_name, pEntryFunc)                      INIT_OS_TIMER_DEFINE(id_name, pEntryFunc)
#define OS_SEMAPHORE_INIT(id_name, remain, limit)               INIT_OS_SEMAPHORE_DEFINE(id_name, remain, limit)
#define OS_MUTEX_INIT(id_name)                                  INIT_OS_MUTEX_DEFINE(id_name)
#define OS_EVT_INIT(id_name, anyMask, modeMask, dirMask, init)  INIT_OS_EVT_DEFINE(id_name, anyMask, modeMask, dirMask, init)
#define OS_MSGQ_INIT(id_name, pBufAddr, len, num)               INIT_OS_MSGQ_DEFINE(id_name, pBufAddr, len, num)
#define OS_POOL_INIT(id_name, pMemAddr, len, num)               INIT_OS_POOL_DEFINE(id_name, pMemAddr, len, num)
#define OS_SUBSCRIBE_INIT(id_name, pDataAddr, size)             INIT_OS_SUBSCRIBE_DEFINE(id_name, pDataAddr, size)
#define OS_PUBLISH_INIT(id_name, pDataAddr, size)               INIT_OS_PUBLISH_DEFINE(id_name, pDataAddr, size)

#define OS_ENTER_CRITICAL_SECTION() ENTER_CRITICAL_SECTION()
#define OS_EXIT_CRITICAL_SECTION()  EXIT_CRITICAL_SECTION()
#define OS_CRITICAL_SECTION()       CRITICAL_SECTION()

/**
 * @brief Initialize a thread, and put it to pending list that are ready to run.
 *
 * @param pEntryFun The pointer of the thread entry function. Thread function must be designed to never return.
 * @param pStackAddr The pointer of the thread stack address. The stack memory must be predefined and allocated in your system.
 * @param stackSize The size of the the stack is base on your specified variable.
 * @param priority The thread priority specified the thread's priority when the AtOS do kernel schedule.
 * @param pName The thread name.
 *
 * @return The value of thread unique id.
 */
static inline os_thread_id_t os_thread_init(u32_t *pStackAddr, u32_t size, i16_t priority, pThread_entryFunc_t pEntryFun,
                                            const char_t *pName)
{
    extern u32_t _impl_thread_init(pThread_entryFunc_t pEntryFun, u32_t * pAddress, u32_t size, i16_t priority, const char_t *pName);

    os_thread_id_t id = {0u};
    id.u32_val = _impl_thread_init(pEntryFun, pStackAddr, size, priority, pName);
    id.pName = pName;

    return id;
}

/**
 * @brief Add a user thread data.
 *
 * @param id The thread unique id.
 *
 * @return The result of thread data operation.
 */
static inline i32p_t os_thread_user_data_set(os_thread_id_t id, void *pUserData)
{
    extern i32p_t _impl_thread_user_data_register(u32_t ctx, void *pUserData);

    return (i32p_t)_impl_thread_user_data_register(id.u32_val, pUserData);
}

/**
 * @brief Get a user thread data.
 *
 * @param id The thread unique id.
 *
 * @return The result of thread data operation.
 */
static inline void *os_thread_user_data_get(os_thread_id_t id)
{
    extern void *_impl_thread_user_data_get(u32_t ctx);

    return (void *)_impl_thread_user_data_get(id.u32_val);
}

/**
 * @brief Put the current running thread into sleep mode with timeout condition.
 *
 * @param timeout_ms The time user defined.
 *
 * @return The result of thread sleep operation.
 */
static inline i32p_t os_thread_sleep(u32_t ms)
{
    extern i32p_t _impl_thread_sleep(u32_t ms);

    return (i32p_t)_impl_thread_sleep(ms);
}

/**
 * @brief Resume a thread to run.
 *
 * @param id The thread unique id.
 *
 * @return The result of thread resume operation.
 */
static inline i32p_t os_thread_resume(os_thread_id_t id)
{
    extern i32p_t _impl_thread_resume(u32_t ctx);

    return (i32p_t)_impl_thread_resume(id.u32_val);
}

/**
 * @brief Suspend a thread to permit another to run.
 *
 * @param id The thread unique id.
 *
 * @return The result of thread suspend operation.
 */
static inline i32p_t os_thread_suspend(os_thread_id_t id)
{
    extern i32p_t _impl_thread_suspend(u32_t ctx);

    return (i32p_t)_impl_thread_suspend(id.u32_val);
}

/**
 * @brief Yield current thread to allow other thread to run.
 *
 * @param id The thread unique id.
 *
 * @return The result of thread yield operation.
 */
static inline i32p_t os_thread_yield(void)
{
    extern i32p_t _impl_thread_yield(void);

    return (i32p_t)_impl_thread_yield();
}

/**
 * @brief Delete a sleep mode thread, and erase the stack data, and that can't be recovered.
 *
 * @param id The thread unique id.
 *
 * @return The result of thread delete operation.
 */
static inline i32p_t os_thread_delete(os_thread_id_t id)
{
    extern i32p_t _impl_thread_delete(u32_t ctx);

    return (i32p_t)_impl_thread_delete(id.u32_val);
}

/**
 * @brief Idle thread callback function register.
 *
 * @param fn The invoke function.
 */
static inline void os_thread_idle_callback_register(const pThread_entryFunc_t loop_fn)
{
    extern void _impl_kthread_idle_user_callback_register(const pThread_entryFunc_t fn);

    _impl_kthread_idle_user_callback_register(loop_fn);
}

/**
 * @brief Get the idle thread id.
 *
 * @return The value of thread unique id.
 */
static inline os_thread_id_t os_thread_idle_id_probe(void)
{
    extern os_thread_id_t _impl_idle_thread_id_get(void);
    return _impl_idle_thread_id_get();
}

/**
 * @brief Add a user thread data.
 *
 * @param id The thread unique id.
 *
 * @return The result of thread data operation.
 */
static inline u32_t os_thread_stack_free_size_probe(os_thread_id_t id)
{
    extern u32_t _impl_thread_stack_free_size_get(u32_t ctx);

    return (u32_t)_impl_thread_stack_free_size_get(id.u32_val);
}

/**
 * @brief Initialize a new timer, or allocate a temporary timer to run.
 *
 * @param pCallFun The timer entry function pointer.
 * @param control It defines the timer running mode.
 * @param timeout_ms The expired time.
 * @param pName The timer's name, it supported NULL pointer.
 *
 * @return The value of the timer unique id.
 */
static inline os_timer_id_t os_timer_init(pTimer_callbackFunc_t pEntryFun, const char_t *pName)
{
    extern u32_t _impl_timer_init(pTimer_callbackFunc_t pCallFun, const char_t *pName);

    os_timer_id_t id = {0u};
    id.u32_val = _impl_timer_init(pEntryFun, pName);
    id.pName = pName;

    return id;
}

/**
 * @brief Allocate a temporary timer to run and release it when it stops.
 *
 * @param pCallFun The timer entry function pointer.
 * @param pName The timer's name, it supported NULL pointer.
 *
 * @return The value of the timer unique id.
 */
static inline os_timer_id_t os_timer_automatic(pTimer_callbackFunc_t pEntryFun, const char_t *pName)
{
    extern u32_t _impl_timer_automatic(pTimer_callbackFunc_t pCallFun, const char_t *pName);

    os_timer_id_t id = {0u};
    id.u32_val = _impl_timer_automatic(pEntryFun, pName);
    id.pName = pName;

    return id;
}

/**
 * @brief Timer starts operation, be careful if the timer's last time isn't expired or be handled,
 *        the new resume will override it.
 *
 * @param id The timer unique id.
 * @param control It defines the timer running mode.
 * @param timeout_ms The timer expired time.
 *
 * @return The result of timer start operation.
 */
static inline i32p_t os_timer_start(os_timer_id_t id, os_timer_ctrl_t control, os_timeout_t timeout_ms)
{
    extern i32p_t _impl_timer_start(u32_t ctx, u8_t control, u32_t timeout_ms);

    return (i32p_t)_impl_timer_start(id.u32_val, (u8_t)control, (u32_t)timeout_ms);
}

/**
 * @brief timer stops operation.
 *
 * @param id The timer unique id.
 *
 * @return The result of timer stop operation.
 */
static inline i32p_t os_timer_stop(os_timer_id_t id)
{
    extern i32p_t _impl_timer_stop(u32_t ctx);

    return (i32p_t)_impl_timer_stop(id.u32_val);
}

/**
 * @brief Check the timer to confirm if it's already scheduled in the waiting list.
 *
 * @param id The timer unique id.
 *
 * @return The true result indicates time busy, otherwise is free status.
 */
static inline i32p_t os_timer_busy(os_timer_id_t id)
{
    extern b_t _impl_timer_busy(u32_t ctx);

    return (i32p_t)_impl_timer_busy(id.u32_val);
}

/**
 * @brief Get the kernel RTOS system time (ms).
 *
 * @return The value of the total system time (ms).
 */
static inline u32_t os_timer_system_total_ms(void)
{
    extern u32_t _impl_timer_total_system_ms_get(void);

    return (u32_t)_impl_timer_total_system_ms_get();
}

/**
 * @brief Initialize a new semaphore.
 *
 * @param remain The initial count that allows the system take.
 * @param limit The maximum count that it's the semaphore's limitation.
 * @param pName The semaphore name.
 *
 * @return The semaphore unique id.
 */
static inline os_sem_id_t os_sem_init(u8_t remain, u8_t limit, const char_t *pName)
{
    extern u32_t _impl_semaphore_init(u8_t remainCount, u8_t limitCount, const char_t *pName);

    os_sem_id_t id = {0u};
    id.u32_val = _impl_semaphore_init(remain, limit, pName);
    id.pName = pName;

    return id;
}

/**
 * @brief Take a semaphore count away with timeout option.
 *
 * @param id The semaphore unique id.
 *
 * @return The result of the operation.
 */
static inline i32p_t os_sem_take(os_sem_id_t id, u32_t timeout_ms)
{
    extern i32p_t _impl_semaphore_take(u32_t ctx, u32_t timeout_ms);

    return (i32p_t)_impl_semaphore_take(id.u32_val, timeout_ms);
}

/**
 * @brief Give the semaphore to release the avaliable count.
 *
 * @param id The semaphore unique id.
 *
 * @return The result of the operation.
 */
static inline i32p_t os_sem_give(os_sem_id_t id)
{
    extern i32p_t _impl_semaphore_give(u32_t ctx);

    return (i32p_t)_impl_semaphore_give(id.u32_val);
}

/**
 * @brief Flush the semaphore to release all the avaliable count.
 *
 * @param id The semaphore unique id.
 *
 * @return The result of the operation.
 */
static inline i32p_t os_sem_flush(os_sem_id_t id)
{
    extern i32p_t _impl_semaphore_flush(u32_t ctx);

    return (i32p_t)_impl_semaphore_flush(id.u32_val);
}

/**
 * @brief Initialize a new mutex.
 *
 * @param pName The mutex name.
 *
 * @return The mutex unique id.
 */
static inline os_mutex_id_t os_mutex_init(const char_t *pName)
{
    extern u32_t _impl_mutex_init(const char_t *pName);

    os_mutex_id_t id = {0u};
    id.u32_val = _impl_mutex_init(pName);
    id.pName = pName;

    return id;
}

/**
 * @brief Mutex lock to avoid another thread access this resource.
 *
 * @param id The mutex unique id.
 *
 * @return The result of the operation.
 */
static inline i32p_t os_mutex_lock(os_mutex_id_t id)
{
    extern i32p_t _impl_mutex_lock(u32_t ctx);

    return (i32p_t)_impl_mutex_lock(id.u32_val);
}

/**
 * @brief Mutex unlock to allow another access the resource.
 *
 * @param id The mutex unique id.
 *
 * @return The result of the operation.
 */
static inline i32p_t os_mutex_unlock(os_mutex_id_t id)
{
    extern i32p_t _impl_mutex_unlock(u32_t ctx);

    return (i32p_t)_impl_mutex_unlock(id.u32_val);
}

/**
 * @brief Initialize a new event.
 *
 * @param anyMask: Changed bits always trigger = 1. otherwise, see dirMask below = 0.
 * @param modeMask: Level trigger = 0, Edge trigger = 1.
 * @param dirMask: Fall or Low trigger = 0, Rise or high trigger = 1.
 * @param init: The init signal value.
 * @param pName: The event name.
 *
 * @return The event unique id.
 *     ...
 */
static inline os_evt_id_t os_evt_init(u32_t anyMask, u32_t modeMask, u32_t dirMask, u32_t init, const char_t *pName)
{
    extern u32_t _impl_event_init(u32_t anyMask, u32_t modeMask, u32_t dirMask, u32_t init, const char_t *pName);

    os_msgq_id_t id = {0u};
    id.u32_val = _impl_event_init(anyMask, modeMask, dirMask, init, pName);
    id.pName = pName;

    return id;
}

/**
 * @brief Read or write the event signal value.
 *
 * @param id: Event unique id.
 * @param pValue: The pointer of the private event value.
 *
 * @return The result of the operation.
 */
static inline i32p_t os_evt_value_get(os_evt_id_t id, u32_t *pValue)
{
    extern i32p_t _impl_event_value_get(u32_t ctx, u32_t * pValue);

    return (i32p_t)_impl_event_value_get(id.u32_val, pValue);
}

/**
 * @brief Set/clear/toggle a event bits.
 *
 * @param id The event unique id.
 * @param set The event value bits set.
 * @param clear The event value bits clear.
 * @param toggle The event value bits toggle.
 *
 * @return The result of the operation.
 */
static inline i32p_t os_evt_set(os_evt_id_t id, u32_t set, u32_t clear, u32_t toggle)
{
    extern i32p_t _impl_event_set(u32_t ctx, u32_t set, u32_t clear, u32_t toggle);

    return (i32p_t)_impl_event_set(id.u32_val, set, clear, toggle);
}

/**
 * @brief Wait a trigger event.
 *
 * @param id The event unique id.
 * @param pEvtData The pointer of event value.
 * @param listen_mask Current thread listen which bits in the event.
 * @param timeout_ms The event wait timeout setting.
 *
 * @return The result of the operation.
 */
static inline i32p_t os_evt_wait(os_evt_id_t id, os_evt_val_t *pEvtData, u32_t listen_mask, os_timeout_t timeout_ms)
{
    extern i32p_t _impl_event_wait(u32_t ctx, struct evt_val * pEvtData, u32_t listen_mask, u32_t timeout_ms);

    return (i32p_t)_impl_event_wait(id.u32_val, pEvtData, listen_mask, (u32_t)timeout_ms);
}

/**
 * @brief Initialize a new queue.
 *
 * @param pName The queue name.
 * @param pBufferAddr The pointer of the queue buffer.
 * @param len The element size.
 * @param num The element number.
 *
 * @return The queue unique id.
 */
static inline os_msgq_id_t os_msgq_init(const void *pBufferAddr, u16_t len, u16_t num, const char_t *pName)
{
    extern u32_t _impl_queue_init(const void *pQueueBufferAddr, u16_t elementLen, u16_t elementNum, const char_t *pName);

    os_msgq_id_t id = {0u};
    id.u32_val = _impl_queue_init(pBufferAddr, len, num, pName);
    id.pName = pName;

    return id;
}

/**
 * @brief Send a queue message.
 *
 * @param id The queue unique id.
 * @param pUserBuffer The pointer of the message buffer address.
 * @param size The queue buffer size.
 * @param isToFront The direction of the message operation.
 * @param timeout_ms The queue send timeout option.
 *
 * @return The result of the operation.
 */
static inline i32p_t os_msgq_put(os_msgq_id_t id, const u8_t *pUserBuffer, u16_t size, b_t isToFront, os_timeout_t timeout_ms)
{
    extern i32p_t _impl_queue_send(u32_t ctx, const u8_t *pUserBuffer, u16_t bufferSize, b_t isToFront, u32_t timeout_ms);

    return (i32p_t)_impl_queue_send(id.u32_val, pUserBuffer, size, isToFront, (u32_t)timeout_ms);
}

/**
 * @brief Receive a queue message.
 *
 * @param id The queue unique id.
 * @param pUserBuffer The pointer of the message buffer address.
 * @param size The queue buffer size.
 * @param isFromBack The direction of the message operation.
 * @param timeout_ms The queue send timeout option.
 *
 * @return The result of the operation.
 */
static inline i32p_t os_msgq_get(os_msgq_id_t id, const u8_t *pUserBuffer, u16_t size, b_t isFromBack, os_timeout_t timeout_ms)
{
    extern i32p_t _impl_queue_receive(u32_t ctx, const u8_t *pUserBuffer, u16_t bufferSize, b_t isFromBack, u32_t timeout_ms);

    return (i32p_t)_impl_queue_receive(id.u32_val, pUserBuffer, size, isFromBack, (u32_t)timeout_ms);
}

/**
 * @brief Get the received message number.
 *
 * @param ctx The queue unique id.
 *
 * @return The result of the operation.
 */
static inline u32_t os_msgq_num_probe(os_msgq_id_t id)
{
    extern u32_t _impl_queue_num_probe(u32_t ctx);

    return (u32_t)_impl_queue_num_probe(id.u32_val);
}

/**
 * @brief Initialize a new pool.
 *
 * @param pName The pool name.
 * @param pMemAddr The pointer of the pool buffer.
 * @param size The element size.
 * @param num The element number.
 *
 * @return The pool unique id.
 */
static inline os_pool_id_t os_pool_init(const void *pMemAddr, u16_t size, u16_t num, const char_t *pName)
{
    extern u32_t _impl_pool_init(const void *pMemAddr, u16_t elementLen, u16_t elementNum, const char_t *pName);

    os_pool_id_t id = {0u};
    id.u32_val = _impl_pool_init(pMemAddr, size, num, pName);
    id.pName = pName;

    return id;
}

/**
 * @brief Take a message pool resource.
 *
 * @param id The pool unique id.
 * @param ppUserMem The dual pointer of the message memory address.
 * @param size The pointer of the message memory size.
 * @param timeout_ms The pool take timeout option.
 *
 * @return The result of the operation.
 */
static inline i32p_t os_pool_take(os_pool_id_t id, void **ppUserMem, u16_t size, os_timeout_t timeout_ms)
{
    extern i32p_t _impl_pool_take(u32_t ctx, void **ppUserBuffer, u16_t bufferSize, u32_t timeout_ms);

    return (i32p_t)_impl_pool_take(id.u32_val, ppUserMem, size, (u32_t)timeout_ms);
}

/**
 * @brief Release memory pool.
 *
 * @param id The pool unique id.
 * @param ppUserMem The dual pointer of the message memory address.
 *
 * @return The result of the operation.
 */
static inline i32p_t os_pool_release(os_pool_id_t id, void **ppUserMem)
{
    extern i32p_t _impl_pool_release(u32_t ctx, void **ppUserBuffer);

    return (i32p_t)_impl_pool_release(id.u32_val, ppUserMem);
}

/**
 * @brief Initialize a new publish.
 *
 * @param pName The publish name.
 *
 * @return The publish unique id.
 */
static inline os_publish_id_t os_publish_init(const char_t *pName)
{
    extern u32_t _impl_publish_init(const char_t *pName);

    os_publish_id_t id = {0u};
    id.u32_val = _impl_publish_init(pName);
    id.pName = pName;

    return id;
}

/**
 * @brief Publisher Submits the report data.
 *
 * @param id The publish unique id.
 * @param pData The pointer of the data buffer address.
 * @param size The data buffer size.
 *
 * @return Value The result of the publisher data operation.
 */
static inline i32p_t os_publish_data_submit(os_publish_id_t id, const void *pData, u16_t size)
{
    extern i32p_t _impl_publish_data_submit(u32_t pub_ctx, const void *pPublishData, u16_t publishSize);

    return _impl_publish_data_submit(id.u32_val, pData, size);
}

/**
 * @brief Initialize a new subscribe.
 *
 * @param pDataAddr The pointer of the data buffer address.
 * @param size The data buffer size.
 * @param pName The subscribe name.
 *
 * @return Value The result fo subscribe init operation.
 */
static inline os_subscribe_id_t os_subscribe_init(void *pDataAddr, u16_t size, const char_t *pName)
{
    extern u32_t _impl_subscribe_init(void *pDataAddr, u16_t dataSize, const char_t *pName);

    os_subscribe_id_t id = {0u};
    id.u32_val = _impl_subscribe_init(pDataAddr, size, pName);
    id.pName = pName;

    return id;
}

/**
 * @brief To check if the publisher submits new data and that is not applied by subscriber.
 *
 * @param subscribe_id The subscribe unique id.
 *
 * @return Value The result of subscribe data is ready.
 */
static inline b_t os_subscribe_data_is_ready(os_subscribe_id_t id)
{
    extern b_t _impl_subscribe_data_is_ready(u32_t sub_ctx);

    return _impl_subscribe_data_is_ready(id.u32_val);
}

/**
 * @brief The subscribe register the corresponding publish.
 *
 * @param subscribe_id The subscribe unique id.
 * @param publish_id The publish unique id.
 * @param isMute The set of notification operation.
 * @param pFuncHandler The notification function handler pointer.
 *
 * @return Value The result fo subscribe init operation.
 */
static inline i32p_t os_subscribe_register(os_subscribe_id_t subscribe_id, os_publish_id_t publish_id, b_t isMute,
                                           pSubscribe_callbackFunc_t pNotificationHandler)
{
    extern i32p_t _impl_subscribe_register(u32_t sub_ctx, u32_t pub_ctx, b_t isMute, pSubscribe_callbackFunc_t pNotificationHandler);

    return _impl_subscribe_register(subscribe_id.u32_val, publish_id.u32_val, isMute, pNotificationHandler);
}

/**
 * @brief The subscriber wait publisher put new data with a timeout option.
 *
 * @param subscribe_id The subscribe unique id.
 * @param pData The pointer of data buffer.
 * @param pDataLen The pointer of data buffer len.
 *
 * @return Value The result of subscribe init operation.
 */
static inline i32p_t os_subscribe_data_apply(os_subscribe_id_t subscribe_id, void *pData, u16_t *pDataLen)
{
    extern i32p_t _impl_subscribe_data_apply(u32_t sub_ctx, void *pDataBuffer, u16_t *pDataLen);

    return _impl_subscribe_data_apply(subscribe_id.u32_val, pData, pDataLen);
}

/**
 * @brief Check if the thread unique id if is's invalid.
 *
 * @param id The provided unique id.
 *
 * @return The value of true is invalid, otherwise is valid.
 */
static inline b_t os_id_is_invalid(struct os_id id)
{
    return (b_t)kernel_os_id_is_invalid(id);
}

/**
 * @brief Get the current running thread context.
 *
 * @return The running thread context.
 */
static inline thread_context_t *os_thread_self_probe(void)
{
    extern thread_context_t *kernel_thread_runContextGet(void);
    return kernel_thread_runContextGet();
}

/**
 * @brief The kernel OS start to run.
 */
static inline i32p_t os_kernel_run(void)
{
    extern i32p_t _impl_kernel_at_rtos_run(void);

    return _impl_kernel_at_rtos_run();
}

/**
 * @brief To check if the kernel OS is running.
 *
 * return The true indicates the kernel OS is running.
 */
static inline b_t os_kernel_is_running(void)
{
    extern b_t _impl_kernel_rtos_isRun(void);

    return (b_t)(_impl_kernel_rtos_isRun() ? (TRUE) : (FALSE));
}

/**
 * @brief Trace At-RTOS firmware version.
 *
 * @return The value of firmware version number.
 */
static inline void os_trace_firmware_version(void)
{
    u32_t _impl_trace_firmware_version_get();
}

/**
 * @brief Trace At-RTOS failed postcode callback function register.
 *
 * @param fn The invoke function.
 */
static inline void os_trace_postcode_callback_register(const pTrace_postcodeFunc_t fn)
{
    _impl_trace_postcode_callback_register(fn);
}

/**
 * @brief Trace At-RTOS failed postcode value.
 *
 * @param fn The invoke function.
 *
 * @return The value of true is failed, otherwise is pass.
 */
static inline b_t os_trace_failed_postcode(const pTrace_postcodeFunc_t fn)
{
    return _impl_trace_postcode_failed_get(fn);
}

/**
 * @brief Trace At-RTOS each thread context.
 *
 * @param fn The invoke function.
 */
static inline void os_trace_foreach_thread(const pTrace_threadFunc_t fn)
{
    _impl_trace_thread(fn);
}

/**
 * @brief Trace At-RTOS kernel thread time usage.
 *
 * @param fn The invoke function.
 */
static inline void os_trace_analyze(const pTrace_analyzeFunc_t fn)
{
    _impl_trace_analyze(fn);
}

/**
 * @brief Force kernel object free, must to confirm no thread blocking in this object
 *
 * @param id The kernel object unique id.
 */
static inline void os_object_free_force(struct os_id id)
{
    extern void _impl_kernel_object_free(u32_t ctx);

    return (u32_t)_impl_kernel_object_free(id.u32_val);
}

/* It defined the AtOS extern symbol for convenience use, but it has extra memory consumption */
#if (OS_API_ENABLE)
typedef struct {
    os_thread_id_t (*thread_init)(u32_t *, u32_t, i16_t, pThread_entryFunc_t, const char_t *);
    i32p_t (*thread_sleep)(u32_t);
    i32p_t (*thread_resume)(os_thread_id_t);
    i32p_t (*thread_suspend)(os_thread_id_t);
    i32p_t (*thread_yield)(void);
    i32p_t (*thread_delete)(os_thread_id_t);
    i32p_t (*thread_user_data_set)(os_thread_id_t, void *);
    void *(*thread_user_data_get)(os_thread_id_t);
    void (*thread_idle_fn_register)(const pThread_entryFunc_t);
    os_thread_id_t (*thread_idle_id_probe)(void);
    u32_t (*thread_stack_free_size_probe)(os_thread_id_t);

    os_timer_id_t (*timer_init)(pTimer_callbackFunc_t, const char_t *);
    os_timer_id_t (*timer_automatic)(pTimer_callbackFunc_t, const char_t *);
    i32p_t (*timer_start)(os_timer_id_t, os_timer_ctrl_t, os_timeout_t);
    i32p_t (*timer_stop)(os_timer_id_t);
    i32p_t (*timer_busy)(os_timer_id_t);
    u32_t (*timer_system_total_ms)(void);

    os_sem_id_t (*sem_init)(u8_t, u8_t, const char_t *);
    i32p_t (*sem_take)(os_sem_id_t, os_timeout_t);
    i32p_t (*sem_give)(os_sem_id_t);
    i32p_t (*sem_flush)(os_sem_id_t);

    os_mutex_id_t (*mutex_init)(const char_t *);
    i32p_t (*mutex_lock)(os_mutex_id_t);
    i32p_t (*mutex_unlock)(os_mutex_id_t);

    os_evt_id_t (*evt_init)(u32_t, u32_t, u32_t, u32_t, const char_t *);
    i32p_t (*evt_set)(os_evt_id_t, u32_t, u32_t, u32_t);
    i32p_t (*evt_wait)(os_evt_id_t, os_evt_val_t *, u32_t, os_timeout_t);

    os_msgq_id_t (*msgq_init)(const void *, u16_t, u16_t, const char_t *);
    i32p_t (*msgq_put)(os_msgq_id_t, const u8_t *, u16_t, b_t, os_timeout_t);
    i32p_t (*msgq_get)(os_msgq_id_t, const u8_t *, u16_t, b_t, os_timeout_t);
    u32_t (*msgq_num_probe)(os_msgq_id_t);

    os_pool_id_t (*pool_init)(const void *, u16_t, u16_t, const char_t *);
    i32p_t (*pool_take)(os_pool_id_t, void **, u16_t, os_timeout_t);
    i32p_t (*pool_release)(os_pool_id_t, void **);

    os_publish_id_t (*publish_init)(const char_t *);
    i32p_t (*publish_data_submit)(os_publish_id_t, const void *, u16_t);
    os_subscribe_id_t (*subscribe_init)(void *, u16_t, const char_t *);
    i32p_t (*subscribe_register)(os_subscribe_id_t, os_publish_id_t, b_t, pSubscribe_callbackFunc_t);
    i32p_t (*subscribe_data_apply)(os_subscribe_id_t, void *, u16_t *);
    b_t (*subscribe_data_is_ready)(os_subscribe_id_t);

    b_t (*id_isInvalid)(struct os_id);
    thread_context_t *(*current_thread)(void);
    i32p_t (*schedule_run)(void);
    b_t (*schedule_is_running)(void);

    void (*trace_versison)(void);
    void (*trace_postcode_fn_register)(const pTrace_postcodeFunc_t);
    b_t (*trace_postcode)(const pTrace_postcodeFunc_t);
    void (*trace_thread)(const pTrace_threadFunc_t);
    void (*trace_time)(const pTrace_analyzeFunc_t);

    void (*object_free)(struct os_id);
} at_rtos_api_t;
#endif

extern const at_rtos_api_t os;

#endif /* _AT_RTOS_H_ */
