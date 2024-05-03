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

#ifdef __cplusplus
extern "C" {
#endif

#define OS_THREAD_DEFINE(thread_name, stack_size, thread_priority)                                                                         \
    static os_thread_symbol_t thread_name[((u32_t)(stack_size) / sizeof(u32_t))] = {                                                       \
        [0] = {.size = stack_size},                                                                                                        \
        [1] = {.priority = thread_priority},                                                                                               \
        [2] = {.pName = #thread_name},                                                                                                     \
    };                                                                                                                                     \
    S_ASSERT(((stack_size) >= STACK_SIZE_MINIMUM), "The thread stack size must be higher than STACK_SIZE_MINIMUM");                        \
    S_ASSERT(                                                                                                                              \
        (((thread_priority) >= OS_PRIORITY_USER_THREAD_HIGHEST_LEVEL) && ((thread_priority) <= OS_PRIORITY_USER_THREAD_LOWEST_LEVEL)),     \
        "The thread priority is out of the system design")

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
 **
 * demo usage:
 *#include "at_rtos.h"
 *
 * OS_THREAD_DEFINE(sample_thread, 512, 5);
 *
 * void thread_sample_function(void)
 * {
 *     while(1) {
 *         os.thread_sleep(1000u);
 *     }
 * }
 *
 * int main(void)
 * {
 *     os_thread_id_t sample_id = os_thread_init(sample_thread, thread_sample_function);
 *     if (os_id_is_invalid(sample_id)) {
 *         printf("Thread %s init failed\n", sample_id.pName);
 *     }
 *     ...
 * }
 *
 */
static inline os_thread_id_t os_thread_init(os_thread_symbol_t *pThread_symbol, pThread_entryFunc_t pEntryFun)
{
    extern u32_t _impl_thread_os_id_to_number(os_id_t id);
    extern os_id_t _impl_thread_init(void (*pThread_entryFunc_t)(void), u32_t *pAddress, u32_t size, u16_t priority, const char_t *pName);

    os_thread_id_t id = {0u};
    u32_t *pStackAddress = (u32_t *)pThread_symbol;
    u32_t stackSize = (u32_t)pThread_symbol[0].size;
    u8_t priority = (u8_t)pThread_symbol[1].priority;
    const char_t *pName = (const char_t *)pThread_symbol[2].pName;

    id.val = _impl_thread_init(pEntryFun, pStackAddress, stackSize, priority, pName);
    id.number = _impl_thread_os_id_to_number(id.val);
    id.pName = pName;

    return id;
}

/**
 * @brief Put the current running thread into sleep mode with timeout condition.
 *
 * @param timeout_ms The time user defined.
 *
 * @return The result of thread sleep operation.
 **
 * demo usage:
 *
 * void thread_sample_function(void)
 * {
 *     while(1) {
 *          os_thread_sleep(1000); // Put the thread to sleep mode 1 sec.
 *     }
 * }
 */
static inline u32p_t os_thread_sleep(u32_t ms)
{
    extern u32p_t _impl_thread_sleep(u32_t ms);

    return (u32p_t)_impl_thread_sleep(ms);
}

/**
 * @brief Resume a thread to run.
 *
 * @param id The thread unique id.
 *
 * @return The result of thread resume operation.
 **
 * demo usage:
 *
 *    os_thread_resume(sample_id); // The variable sample_id created in above thread init demo.
 */
static inline u32p_t os_thread_resume(os_thread_id_t id)
{
    extern u32p_t _impl_thread_resume(os_id_t id);

    return (u32p_t)_impl_thread_resume(id.val);
}

/**
 * @brief Suspend a thread to permit another to run.
 *
 * @param id The thread unique id.
 *
 * @return The result of thread suspend operation.
 **
 * demo usage:
 *
 *    os_thread_suspend(sample_id); // The variable sample_id created in above thread init demo.
 */
static inline u32p_t os_thread_suspend(os_thread_id_t id)
{
    extern u32p_t _impl_thread_suspend(os_id_t id);

    return (u32p_t)_impl_thread_suspend(id.val);
}

/**
 * @brief Yield current thread to allow other thread to run.
 *
 * @param id The thread unique id.
 *
 * @return The result of thread yield operation.
 **
 * demo usage:
 *
 * void thread_sample_function(void)
 * {
 *     while(1) {
 *          os_thread_yield(); // Put current thread to sleep mode manually.
 *     }
 * }
 */
static inline u32p_t os_thread_yield(void)
{
    extern u32p_t _impl_thread_yield(void);

    return (u32p_t)_impl_thread_yield();
}

/**
 * @brief Delete a sleep mode thread, and erase the stack data, and that can't be recovered.
 *
 * @param id The thread unique id.
 *
 * @return The result of thread delete operation.
 **
 * demo usage:
 *
 *    os_thread_delete(sample_id); // The variable sample_id created in above thread init demo.
 */
static inline u32p_t os_thread_delete(os_thread_id_t id)
{
    extern u32p_t _impl_thread_delete(os_id_t id);

    return (u32p_t)_impl_thread_delete(id.val);
}

/**
 * @brief Initialize a timer.
 *
 * @param pCallFun The timer entry function pointer.
 * @param isCycle It indicates the timer if it's cycle repeat.
 * @param timeout_ms The expired time.
 * @param pName The timer's name, it supported NULL pointer.
 *
 * @return The value of the timer unique id.
 **
 * demo usage:
 *
 *    void sample_timer_function(void)
 *    {
 *        // The function will be called per 1 seconds.
 *    }
 *
 *    os_timer_id_t sample_id = os_timer_init(sample_timer_function, TRUE, 1000u, "sample");
 *     if (os_id_is_invalid(sample_id)) {
 *         printf("Timer %s init failed\n", sample_id.pName);
 *     }
 *     ...
 */
static inline os_timer_id_t os_timer_init(pTimer_callbackFunc_t pEntryFun, b_t isCycle, u32_t timeout_ms, const char_t *pName)
{
    extern u32_t _impl_timer_os_id_to_number(u32_t id);
    extern os_id_t _impl_timer_init(pTimer_callbackFunc_t pCallFun, b_t isCycle, u32_t timeout_ms, const char_t *pName);

    os_timer_id_t id = {0u};

    id.val = _impl_timer_init(pEntryFun, isCycle, timeout_ms, pName);
    id.number = _impl_timer_os_id_to_number(id.val);
    id.pName = pName;

    return id;
}

/**
 * @brief Timer starts operation, be careful if the timer's last time isn't expired or be handled,
 *        the new resume will override it.
 *
 * @param id The timer unique id.
 * @param isCycle It indicates the timer if it's cycle repeat.
 * @param timeout_ms The timer expired time.
 *
 * @return The result of timer start operation.
 **
 * demo usage:
 *
 *     u32p_t postcode = os_timer_start(sample_id, FALSE, 1000u);
 *     if (PC_IOK(postcode)) {
 *         printf("Timer start successful\n");
 *     }
 */
static inline u32p_t os_timer_start(os_timer_id_t id, b_t isCycle, u32_t timeout_ms)
{
    extern u32p_t _impl_timer_start(os_id_t id, b_t isCycle, u32_t timeout_ms);

    return (u32p_t)_impl_timer_start(id.val, isCycle, timeout_ms);
}

/**
 * @brief timer stops operation.
 *
 * @param id The timer unique id.
 *
 * @return The result of timer stop operation.
 **
 * demo usage:
 *
 *     u32p_t postcode = os_timer_stop(sample_id);
 *     if (PC_IOK(postcode)) {
 *         printf("Timer stop successful\n");
 *     }
 */
static inline u32p_t os_timer_stop(os_timer_id_t id)
{
    extern u32p_t _impl_timer_stop(os_id_t id);

    return (u32p_t)_impl_timer_stop(id.val);
}

/**
 * @brief Check the timer to confirm if it's already scheduled in the waiting list.
 *
 * @param id The timer unique id.
 *
 * @return The true result indicates time busy, otherwise is free status.
 **
 * demo usage:
 *
 *    if(os_timer_busy(sample_id)) {
 *        printf("Timer %s is busy\n", sample_id.pName);
 *    }
 */
static inline u32p_t os_timer_busy(os_timer_id_t id)
{
    extern b_t _impl_timer_busy(os_id_t id);

    return (u32p_t)_impl_timer_busy(id.val);
}

/**
 * @brief Get the kernel RTOS system time (ms).
 *
 * @return The value of the total system time (ms).
 **
 * demo usage:
 *
 *    printf("The system consume time: %d\n", os_timer_system_total_ms());
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
 **
 * demo usage:
 *
 *    // Init a binary semaphore count.
 *     os_sem_id_t sample_id = os_sem_init(0u, 1u, FALSE, "sample");
 *     if (os_id_is_invalid(sample_id)) {
 *         printf("Semaphore %s init failed\n", sample_id.pName);
 *     }
 *     ...
 */
static inline os_sem_id_t os_sem_init(u8_t remain, u8_t limit, const char_t *pName)
{
    extern u32_t _impl_semaphore_os_id_to_number(os_id_t id);
    extern os_id_t _impl_semaphore_init(u8_t remainCount, u8_t limitCount, const char_t *pName);

    os_sem_id_t id = {0u};

    id.val = _impl_semaphore_init(remain, limit, pName);
    id.number = _impl_semaphore_os_id_to_number(id.val);
    id.pName = pName;

    return id;
}

/**
 * @brief Take a semaphore count away with timeout option.
 *
 * @param id The semaphore unique id.
 *
 * @return The result of the operation.
 **
 * demo usage:
 *
 *     u32p_t postcode = os_sem_take(sample_id, 1000u);
 *     if (PC_IOK(postcode)) {
 *         if (postcode == PC_SC_TIMEOUT) {
 *             printf("Semaphore take wait timeout\n");
 *         } else {
 *             printf("Semaphore take successful\n");
 *         }
 *     } else {
 *         printf("Semaphore take error: 0x%x\n", postcode);
 *     }
 *
 *     u32p_t postcode = os_sem_take(sample_id, OS_WAIT_FOREVER);
 *     if (PC_IOK(postcode)) {
 *        printf("Semaphore take successful\n");
 *     } else {
 *         printf("Semaphore take error: 0x%x\n", postcode);
 *     }
 */
static inline u32p_t os_sem_take(os_sem_id_t id, u32_t timeout_ms)
{
    extern u32p_t _impl_semaphore_take(os_id_t id, u32_t timeout_ms);

    return (u32p_t)_impl_semaphore_take(id.val, timeout_ms);
}

/**
 * @brief Give the semaphore to release the avaliable count.
 *
 * @param id The semaphore unique id.
 *
 * @return The result of the operation.
 **
 * demo usage:
 *
 *     u32p_t postcode = os_sem_give(sample_id);
 *     if (PC_IOK(postcode)) {
 *         printf("Semaphore give successful\n");
 *     } else {
 *         printf("Semaphore give error: 0x%x\n", postcode);
 *     }
 */
static inline u32p_t os_sem_give(os_sem_id_t id)
{
    extern u32_t _impl_semaphore_give(os_id_t id);

    return (u32p_t)_impl_semaphore_give(id.val);
}

/**
 * @brief Flush the semaphore to release all the avaliable count.
 *
 * @param id The semaphore unique id.
 *
 * @return The result of the operation.
 **
 * demo usage:
 *
 *     u32p_t postcode = os_sem_flush(sample_id);
 *     if (PC_IOK(postcode)) {
 *         printf("Semaphore flush successful\n");
 *     } else {
 *         printf("Semaphore flush error: 0x%x\n", postcode);
 *     }
 */
static inline u32p_t os_sem_flush(os_sem_id_t id)
{
    extern u32_t _impl_semaphore_flush(os_id_t id);

    return (u32p_t)_impl_semaphore_flush(id.val);
}

/**
 * @brief Initialize a new mutex.
 *
 * @param pName The mutex name.
 *
 * @return The mutex unique id.
 **
 * demo usage:
 *
 *     os_mutex_id_t sample_id = os_mutex_init("sample");
 *     if (os_id_is_invalid(sample_id)) {
 *         printf("Mutex %s init failed\n", sample_id.pName);
 *     }
 *     ...
 */
static inline os_mutex_id_t os_mutex_init(const char_t *pName)
{
    extern u32_t _impl_mutex_os_id_to_number(os_id_t id);
    extern os_id_t _impl_mutex_init(const char_t *pName);

    os_mutex_id_t id = {0u};

    id.val = _impl_mutex_init(pName);
    id.number = _impl_mutex_os_id_to_number(id.val);
    id.pName = pName;

    return id;
}

/**
 * @brief Mutex lock to avoid another thread access this resource.
 *
 * @param id The mutex unique id.
 *
 * @return The result of the operation.
 **
 * demo usage:
 *
 *     u32p_t postcode = os_mutex_lock(sample_id);
 *     if (PC_IOK(postcode)) {
 *         printf("Mutex lock successful\n");
 *     } else {
 *         printf("Mutex lock error: 0x%x\n", postcode);
 *     }
 */
static inline u32p_t os_mutex_lock(os_mutex_id_t id)
{
    extern u32p_t _impl_mutex_lock(os_id_t id);

    return (u32p_t)_impl_mutex_lock(id.val);
}

/**
 * @brief Mutex unlock to allow another access the resource.
 *
 * @param id The mutex unique id.
 *
 * @return The result of the operation.
 **
 * demo usage:
 *
 *     u32p_t postcode = os_mutex_unlock(sample_id);
 *     if (PC_IOK(postcode)) {
 *         printf("Mutex unlock successful\n");
 *     } else {
 *         printf("Mutex unlock error: 0x%x\n", postcode);
 *     }
 */
static inline u32p_t os_mutex_unlock(os_mutex_id_t id)
{
    extern u32p_t _impl_mutex_unlock(os_id_t id);

    return (u32p_t)_impl_mutex_unlock(id.val);
}

/**
 * @brief Initialize a new event.
 *
 * @param edgeMask Specific the event desired condition of edge or level.
 * @param clrDisMask Disable automatically clear the set events.
 * @param pName The event name.
 *
 * @return The event unique id.
 *
 **
 * demo usage:
 *
 *     os_evt_id_t sample_id = os_evt_init(0u, 0u, "sample");
 *     if (os_id_is_invalid(sample_id)) {
 *         printf("Event %s init failed\n", sample_id.pName);
 *     }
 *     ...
 */
static inline os_evt_id_t os_evt_init(u32_t edgeMask, u32_t clrDisMask, const char_t *pName)
{
    extern u32_t _impl_event_os_id_to_number(os_id_t id);
    extern os_id_t _impl_event_init(u32_t edgeMask, u32_t clrDisMask, const char_t *pName);

    os_msgq_id_t id = {0u};

    id.val = _impl_event_init(edgeMask, clrDisMask, pName);
    id.number = _impl_event_os_id_to_number(id.val);
    id.pName = pName;

    return id;
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
 **
 * demo usage:
 *
 *     u32p_t postcode = os_evt_set(sample_id, 0x01u, 0u, 0u);
 *     if (PC_IOK(postcode)) {
 *         printf("Event set successful\n");
 *     } else {
 *         printf("Event set error: 0x%x\n", postcode);
 *     }
 */
static inline u32p_t os_evt_set(os_evt_id_t id, u32_t set, u32_t clear, u32_t toggle)
{
    extern u32p_t _impl_event_set(os_id_t id, u32_t set, u32_t clear, u32_t toggle);

    return (u32p_t)_impl_event_set(id.val, set, clear, toggle);
}

/**
 * @brief Wait a desired single or group event bits.
 *
 * @param id The event unique id.
 * @param pEvtData The pointer of event value.
 * @param desired_val If the desired is not zero, All changed bits seen can wake up the thread to handle event.
 * @param listen_mask Current thread listen which bits in the event.
 * @param group_mask To define a group event.
 * @param timeout_ms The event wait timeout setting.
 *
 * @return The result of the operation.
 **
 * demo usage:
 *
 *     os_evt_val_t evt_val = {0u};
 *     u32p_t postcode = os_evt_wait(sample_id, &evt_val, 0xFFFFFFFu, 0x01u, 0x01u, OS_WAIT_FOREVER);
 *     if (PC_IOK(postcode)) {
 *         printf("Event wait successful, The event value is 0x%x\n", evt_val->value);
 *     } else {
 *         printf("Event wait error: 0x%x\n", postcode);
 *     }
 *
 *     postcode = os_evt_wait(sample_id, &evt_val, 0xFFFFFFFu, 0x03u, 0x03u, 1000u);
 *     if (PC_IOK(postcode)) {
 *         if (postcode == PC_SC_TIMEOUT) {
 *             printf("Event wait timeout\n");
 *         } else {
 *             printf("Event wait successful, The event value is 0x%x\n", evt_val->value);
 *         }
 *     } else {
 *         printf("Event wait error: 0x%x\n", postcode);
 *     }
 */
static inline u32p_t os_evt_wait(os_evt_id_t id, os_evt_val_t *pEvtData, u32_t desired_val, u32_t listen_mask, u32_t group_mask,
                                 u32_t timeout_ms)
{
    extern u32p_t _impl_event_wait(os_id_t id, os_evt_val_t * pEvtData, u32_t desired_val, u32_t listen_mask, u32_t group_mask,
                                   u32_t timeout_ms);

    if (pEvtData) {
        pEvtData->depth.enable = FALSE;
    }
    return (u32p_t)_impl_event_wait(id.val, pEvtData, desired_val, listen_mask, group_mask, timeout_ms);
}

/**
 * @brief Wait a desired single or group event bits,
 *        but the depth function will search the past event value that from the last waiting timing.
 *
 * @param id The event unique id.
 * @param pEvtData The pointer of event value.
 * @param desired_val If the desired is not zero, All changed bits seen can wake up the thread to handle event.
 * @param listen_mask Current thread listen which bits in the event.
 * @param group_mask To define a group event.
 * @param timeout_ms The event wait timeout setting.
 *
 * @return The result of the operation.
 **
 * demo usage:
 *
 *     os_evt_val_t evt_val = {0u};
 *     u32p_t postcode = os_evt_wait_depth(sample_id, &evt_val, 0xFFFFFFFu, 0x03u, 0x03u, 1000u);
 *     if (PC_IOK(postcode)) {
 *         if (postcode == PC_SC_TIMEOUT) {
 *             printf("Event wait timeout\n");
 *         } else {
 *             printf("Event wait successful, The event value is 0x%x\n", evt_val->value);
 *         }
 *     } else {
 *         printf("Event wait error: 0x%x\n", postcode);
 *     }
 */
static inline u32p_t os_evt_wait_depth(os_evt_id_t id, os_evt_val_t *pEvtData, u32_t desired_val, u32_t listen_mask, u32_t group_mask,
                                       u32_t timeout_ms)
{
    extern u32p_t _impl_event_wait(os_id_t id, os_evt_val_t * pEvtData, u32_t desired_val, u32_t listen_mask, u32_t group_mask,
                                   u32_t timeout_ms);

    if (pEvtData) {
        pEvtData->depth.enable = TRUE;
    }
    return (u32p_t)_impl_event_wait(id.val, pEvtData, desired_val, listen_mask, group_mask, timeout_ms);
}

/**
 * @brief Initialize a new queue.
 *
 * @param pName The queue name.
 * @param pQueueBufferAddr The pointer of the queue buffer.
 * @param elementLen The element size.
 * @param elementNum The element number.
 *
 * @return The queue unique id.
 **
 * demo usage:
 *     static u8_t g_sample_msgq[3 * 10] = {0u};
 *
 *     os_msgq_id_t sample_id = os_msgq_init((u8_t*)g_sample_msgq, 3u, 10u, "sample");
 *     if (os_id_is_invalid(sample_id)) {
 *         printf("Message queue %s init failed\n", sample_id.pName);
 *     }
 */
static inline os_msgq_id_t os_msgq_init(const void *pQueueBufferAddr, u16_t elementLen, u16_t elementNum, const char_t *pName)
{
    extern u32_t _impl_queue_os_id_to_number(os_id_t id);
    extern os_id_t _impl_queue_init(const void *pQueueBufferAddr, u16_t elementLen, u16_t elementNum, const char_t *pName);

    os_msgq_id_t id = {0u};

    id.val = _impl_queue_init(pQueueBufferAddr, elementLen, elementNum, pName);
    id.number = _impl_queue_os_id_to_number(id.val);
    id.pName = pName;

    return id;
}

/**
 * @brief Send a queue message.
 *
 * @param id The queue unique id.
 * @param pUserBuffer The pointer of the message buffer address.
 * @param bufferSize The queue buffer size.
 * @param isToFront The direction of the message operation.
 * @param timeout_ms The queue send timeout option.
 *
 * @return The result of the operation.
 **
 * demo usage:
 *
 *     u8_t txdata = 0u;
 *     u32p_t postcode = os_msgq_put(sample_id, &txdata, 0x01u, FALSE, OS_WAIT_FOREVER);
 *     if (PC_IOK(postcode)) {
 *         printf("Message queue send successful\n");
 *     } else {
 *         printf("Message queue send error: 0x%x\n", postcode);
 *     }
 *
 *     postcode = os_msgq_put(sample_id, &txdata, 0x01u, TRUE, 1000u);
 *     if (PC_IOK(postcode)) {
 *         if (postcode == PC_SC_TIMEOUT) {
 *             printf("Message queue send timeout\n");
 *         } else {
 *             printf("Message queue send successful\n");
 *         }
 *     } else {
 *         printf("Message queue send error: 0x%x\n", postcode);
 *     }
 */
static inline u32p_t os_msgq_put(os_msgq_id_t id, const u8_t *pUserBuffer, u16_t bufferSize, b_t isToFront, u32_t timeout_ms)
{
    extern u32p_t _impl_queue_send(os_id_t id, const u8_t *pUserBuffer, u16_t bufferSize, b_t isToFront, u32_t timeout_ms);

    return (u32p_t)_impl_queue_send(id.val, pUserBuffer, bufferSize, isToFront, timeout_ms);
}

/**
 * @brief Receive a queue message.
 *
 * @param id The queue unique id.
 * @param pUserBuffer The pointer of the message buffer address.
 * @param bufferSize The queue buffer size.
 * @param isFromBack The direction of the message operation.
 * @param timeout_ms The queue send timeout option.
 *
 * @return The result of the operation.
 **
 * demo usage:
 *
 *     u8_t rxdata = 0u;
 *     u32p_t postcode = os_msgq_get(sample_id, &rxdata, 0x01u, TRUE, OS_WAIT_FOREVER);
 *     if (PC_IOK(postcode)) {
 *         printf("Message queue receive successful, the rx data is 0x%x\n", rxdata);
 *     } else {
 *         printf("Message queue receive error: 0x%x\n", postcode);
 *     }
 *
 *     postcode = os_msgq_get(sample_id, &rxdata, 0x01u, FALSE, 1000u);
 *     if (PC_IOK(postcode)) {
 *         if (postcode == PC_SC_TIMEOUT) {
 *             printf("Message queue receive timeout\n");
 *         } else {
 *             printf("Message queue receive successful, the rx data is 0x%x\n", rxdata);
 *         }
 *     } else {
 *         printf("Message queue receive error: 0x%x\n", postcode);
 *     }
 */
static inline u32p_t os_msgq_get(os_msgq_id_t id, const u8_t *pUserBuffer, u16_t bufferSize, b_t isFromBack, u32_t timeout_ms)
{
    extern u32p_t _impl_queue_receive(os_id_t id, const u8_t *pUserBuffer, u16_t bufferSize, b_t isFromBack, u32_t timeout_ms);

    return (u32p_t)_impl_queue_receive(id.val, pUserBuffer, bufferSize, isFromBack, timeout_ms);
}

/**
 * @brief Initialize a new pool.
 *
 * @param pName The pool name.
 * @param pMemAddress The pointer of the pool buffer.
 * @param elementLen The element size.
 * @param elementNum The element number.
 *
 * @return The pool unique id.
 **
 * demo usage:
 *     static u8_t g_sample_pool[3 * 10] = {0u};
 *
 *     os_pool_id_t sample_id = os_pool_init((const void*)g_sample_pool, 10u, 3u, "sample");
 *     if (os_id_is_invalid(sample_id)) {
 *         printf("Memory pool %s init failed\n", sample_id.pName);
 *     }
 */
static inline os_pool_id_t os_pool_init(const void *pMemAddress, u16_t elementLen, u16_t elementNum, const char_t *pName)
{
    extern u32_t _impl_pool_os_id_to_number(os_id_t id);
    extern os_id_t _impl_pool_init(const void *pMemAddr, u16_t elementLen, u16_t elementNum, const char_t *pName);

    os_pool_id_t id = {0u};

    id.val = _impl_pool_init(pMemAddress, elementLen, elementNum, pName);
    id.number = _impl_pool_os_id_to_number(id.val);
    id.pName = pName;

    return id;
}

/**
 * @brief Take a message pool resource.
 *
 * @param id The pool unique id.
 * @param ppUserBuffer The dual pointer of the message memory address.
 * @param pBufferSize The pointer of the message memory size.
 * @param timeout_ms The pool take timeout option.
 *
 * @return The result of the operation.
 **
 * demo usage:
 *
 *     u8_t* pTakeMem = NULL;
 *     u32p_t postcode = os_pool_take(sample_id, (void **)&pTakeMem, 10u, OS_WAIT_FOREVER);
 *     if (PC_IOK(postcode)) {
 *         printf("Memory pool take successful\n");
 *     } else {
 *         printf("Memory pool take error: 0x%x\n", postcode);
 *     }
 *
 *     u32p_t postcode = os_pool_take(sample_id, (void **)&pTakeMem, 10u, 2000u);
 *     if (PC_IOK(postcode)) {
 *         if (postcode == PC_SC_TIMEOUT) {
 *             printf("Memory pool take timeout\n");
 *         } else {
 *             printf("Memory pool take successful\n");
 *         }
 *     } else {
 *         printf("Memory pool take error: 0x%x\n", postcode);
 *     }
 */
static inline u32p_t os_pool_take(os_pool_id_t id, void **ppUserBuffer, u16_t bufferSize, u32_t timeout_ms)
{
    extern u32p_t _impl_pool_take(os_id_t id, void **ppUserBuffer, u16_t bufferSize, u32_t timeout_ms);

    return (u32p_t)_impl_pool_take(id.val, ppUserBuffer, bufferSize, timeout_ms);
}

/**
 * @brief Release memory pool.
 *
 * @param id The pool unique id.
 * @param ppUserBuffer The dual pointer of the message memory address.
 *
 * @return The result of the operation.
 **
 * demo usage:
 *
 *     u8_t* pTakeMem = NULL;
 *     u32p_t postcode = os_pool_take(sample_id, (void **)&pTakeMem, 10u, OS_WAIT_FOREVER);
 *     if (PC_IOK(postcode)) {
 *         printf("Memory pool take successful\n");
 *
 *         u32p_t postcode = os_pool_release(sample_id, (void **)&pTakeMem);
 *         if (PC_IOK(postcode)) {
 *             printf("Memory pool release successful\n");
 *         } else {
 *              printf("Memory pool release error: 0x%x\n", postcode);
 *         }
 *
 *     } else {
 *         printf("Memory pool take error: 0x%x\n", postcode);
 *     }
 */
static inline u32p_t os_pool_release(os_pool_id_t id, void **ppUserBuffer)
{
    extern u32p_t _impl_pool_release(os_id_t id, void **ppUserBuffer);

    return (u32p_t)_impl_pool_release(id.val, ppUserBuffer);
}

/**
 * @brief Check if the thread unique id if is's invalid.
 *
 * @param id The provided unique id.
 *
 * @return The value of true is invalid, otherwise is valid.
 **
 * demo usage:
 *
 *     os_mutex_id_t sample_id = mutex_init("sample");
 *     if (os_id_is_invalid(sample_id)) {
 *         printf("Mutex %s init failed\n", sample_id.pName);
 *     }
 *     ...
 */
static inline b_t os_id_is_invalid(struct os_id id)
{
    return (b_t)kernel_os_id_is_invalid(id);
}

/**
 * @brief Get the current running thread id.
 *
 * @return The id of current running thread.
 **
 * demo usage:
 *
 *     os_thread_id_t current = os_id_current_thread();
 *     printf("At-RTOS kernel current running thread is %s\n", current.pName);
 *     ...
 */
static inline os_thread_id_t os_id_current_thread(void)
{
    extern const char_t *_impl_thread_name_get(os_id_t id);
    extern os_id_t kernel_thread_runIdGet(void);
    extern u32_t _impl_thread_os_id_to_number(os_id_t id);

    os_thread_id_t id = {0u};
    id.val = kernel_thread_runIdGet();
    id.number = _impl_thread_os_id_to_number(id.val);
    id.pName = _impl_thread_name_get(id.val);

    return id;
}

/**
 * @brief The kernel OS start to run.
 **
 * demo usage:
 *
 *     os_kernel_run();
 *     // Doesn't arrive
 */
static inline u32p_t os_kernel_run(void)
{
    extern u32p_t _impl_kernel_at_rtos_run(void);

    return _impl_kernel_at_rtos_run();
}

/**
 * @brief To check if the kernel OS is running.
 *
 * return The true indicates the kernel OS is running.
 **
 * demo usage:
 *
 *     if (os_kernel_is_running()) {
 *         printf("At-RTOS kernel is running\n");
 *     }
 *     ...
 */
static inline b_t os_kernel_is_running(void)
{
    extern b_t _impl_kernel_rtos_isRun(void);

    return (b_t)(_impl_kernel_rtos_isRun() ? (TRUE) : (FALSE));
}

/**
 * @brief Print At-RTOS firmware information.
 **
 * demo usage:
 *
 *     os_trace_firmware_print();
 */
static inline void os_trace_firmware_print(void)
{
    _impl_trace_firmware_snapshot_print();
}

/**
 * @brief Print At-RTOS failed postcode value.
 **
 * demo usage:
 *
 *     os_trace_postcode_print();
 */
static inline void os_trace_postcode_print(void)
{
    _impl_trace_postcode_snapshot_print();
}

/**
 * @brief Print At-RTOS kernel information.
 **
 * demo usage:
 *
 *     os_trace_kernel_print();
 */
static inline void os_trace_kernel_print(void)
{
    _impl_trace_kernel_snapshot_print();
}

/* It defined the AtOS extern symbol for convenience use, but it has extra memory consumption */
#if (OS_INTERFACE_EXTERN_USE_ENABLE)
typedef struct {
    os_thread_id_t (*thread_init)(os_thread_symbol_t *, pThread_entryFunc_t);
    u32p_t (*thread_sleep)(u32_t);
    u32p_t (*thread_resume)(os_thread_id_t);
    u32p_t (*thread_suspend)(os_thread_id_t);
    u32p_t (*thread_yield)(void);
    u32p_t (*thread_delete)(os_thread_id_t);

    os_timer_id_t (*timer_init)(pTimer_callbackFunc_t, b_t, u32_t, const char_t *);
    u32p_t (*timer_start)(os_timer_id_t, b_t, u32_t);
    u32p_t (*timer_stop)(os_timer_id_t);
    u32p_t (*timer_busy)(os_timer_id_t);
    u32_t (*timer_system_total_ms)(void);

    os_sem_id_t (*sem_init)(u8_t, u8_t, const char_t *);
    u32p_t (*sem_take)(os_sem_id_t, u32_t);
    u32p_t (*sem_give)(os_sem_id_t);
    u32p_t (*sem_flush)(os_sem_id_t);

    os_mutex_id_t (*mutex_init)(const char_t *);
    u32p_t (*mutex_lock)(os_mutex_id_t);
    u32p_t (*mutex_unlock)(os_mutex_id_t);

    os_evt_id_t (*evt_init)(u32_t, u32_t, const char_t *);
    u32p_t (*evt_set)(os_evt_id_t, u32_t, u32_t, u32_t);
    u32p_t (*evt_wait)(os_evt_id_t, os_evt_val_t *, u32_t, u32_t, u32_t, u32_t);
    u32p_t (*evt_wait_depth)(os_evt_id_t, os_evt_val_t *, u32_t, u32_t, u32_t, u32_t);

    os_msgq_id_t (*msgq_init)(const void *, u16_t, u16_t, const char_t *);
    u32p_t (*msgq_put)(os_msgq_id_t, const u8_t *, u16_t, b_t, u32_t);
    u32p_t (*msgq_get)(os_msgq_id_t, const u8_t *, u16_t, b_t, u32_t);

    os_pool_id_t (*pool_init)(const void *, u16_t, u16_t, const char_t *);
    u32p_t (*pool_take)(os_pool_id_t, void **, u16_t, u32_t);
    u32p_t (*pool_release)(os_pool_id_t, void **);

    b_t (*id_isInvalid)(struct os_id);
    os_thread_id_t (*id_current_thread)(void);
    u32p_t (*schedule_run)(void);
    b_t (*schedule_is_running)(void);

    void (*trace_firmware)(void);
    void (*trace_postcode)(void);
    void (*trace_kernel)(void);
} at_rtos_api_t;

extern const at_rtos_api_t os;
#endif

#ifdef __cplusplus
}
#endif

#endif /* _AT_RTOS_H_ */
