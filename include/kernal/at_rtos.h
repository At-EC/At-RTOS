/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef _AT_RTOS_H_
#define _AT_RTOS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "basic.h"
#include "unique.h"
#include "configuration.h"

#define ATOS_THREAD_DEFINE(thread_name, stack_size, thread_priority)    static os_thread_symbol_t thread_name[((u32_t)(stack_size) / sizeof(u32_t))] = { \
    [0] = {.size = stack_size},                                                                                                                           \
    [1] = {.priority = thread_priority},                                                                                                                  \
    [2] = {.pName = #thread_name},                                                                                                                         \
};                                                                                                                                                        \
S_ASSERT(((stack_size) >= STACK_SIZE_MINIMUM) , "The thread stack size must be higher than STACK_SIZE_MINIMUM");                                          \
S_ASSERT((((thread_priority) >= OS_PRIORITY_USER_THREAD_HIGHEST_LEVEL) && ((thread_priority) <= OS_PRIORITY_USER_THREAD_LOWEST_LEVEL)) ,                  \
         "The thread priority is out of the system design")

#include "thread.h"

/**
 * @brief Initialize a thread, and put it to pending list that are ready to run.
 *
 * @param pEntryFun The pointer of the thread entry function. Thread function must be designed to never return.
 * @param pStackAddr The pointer of the thread stack address. The stack memory must be predefined and allocated in your system.
 * @param stackSize The size of the the stack is base on your specified variable.
 * @param priority The thread priority specified the thread's priority when the AtOS do kernal schedule.
 * @param pName The thread name.
 *
 * @return The value of thread unique id.
 **
 * demo usage:
 *#include "at_rtos.h"
 *
 *ATOS_THREAD_DEFINE(sample_thread, 512, 5);
 *
 * void thread_sample_function(void)
 * {
 *     while(1)
 *     {
 *         AtOS.thread_sleep(1000u);
 *     }
 * }
 *
 * int main(void)
 * {
 *     os_thread_id_t sample_id = thread_init(sample_thread, thread_sample_function);
 *     if (os_id_is_invalid(sample_id))
 *     {
 *         printf("Thread %s init failed\n", sample_id.pName);
 *     }
 *     ...
 * }
 *
 */
static inline os_thread_id_t thread_init(os_thread_symbol_t* pThread_symbol, pThread_entryFunc_t pEntryFun)
{
    os_thread_id_t id = {0u};
    u32_t* pStackAddress = (u32_t*)pThread_symbol;
    u32_t stackSize = (u32_t)pThread_symbol[0].size;
    u8_t priority = (u8_t)pThread_symbol[1].priority;
    const char_t* pName = (const char_t*)pThread_symbol[2].pName;

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
 *     while(1)
 *     {
 *          thread_sleep(1000); // Put the thread to sleep mode 1 sec.
 *     }
 * }
 */
static inline u32p_t thread_sleep(u32_t ms)
{
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
 *    thread_resume(sample_id); // The variable sample_id created in above thread init demo.
 */
static inline u32p_t thread_resume(os_thread_id_t id)
{
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
 *    thread_suspend(sample_id); // The variable sample_id created in above thread init demo.
 */
static inline u32p_t thread_suspend(os_thread_id_t id)
{
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
 *     while(1)
 *     {
 *          thread_yield(); // Put current thread to sleep mode manually.
 *     }
 * }
 */
static inline u32p_t thread_yield(void)
{
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
 *    thread_delete(sample_id); // The variable sample_id created in above thread init demo.
 */
static inline u32p_t thread_delete(os_thread_id_t id)
{
    return (u32p_t)_impl_thread_delete(id.val);
}

#include "timer.h"

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
 *    os_timer_id_t sample_id = timer_init(sample_timer_function, TRUE, 1000u, "sample");
 *     if (os_id_is_invalid(sample_id))
 *     {
 *         printf("Timer %s init failed\n", sample_id.pName);
 *     }
 *     ...
 */
static inline os_timer_id_t timer_init(pTimer_callbackFunc_t pEntryFun, b_t isCycle, u32_t timeout_ms, const char_t *pName)
{
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
 *     u32p_t postcode = timer_start(sample_id, FALSE, 1000u);
 *     if (PC_IOK(postcode))
 *     {
 *         printf("Timer start successful\n");
 *     }
 */
static inline u32p_t timer_start(os_timer_id_t id, b_t isCycle, u32_t timeout_ms)
{
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
 *     u32p_t postcode = timer_stop(sample_id);
 *     if (PC_IOK(postcode))
 *     {
 *         printf("Timer stop successful\n");
 *     }
 */
static inline u32p_t timer_stop(os_timer_id_t id)
{
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
 *    if(timer_isBusy(sample_id))
 *    {
 *        printf("Timer %s is busy\n", sample_id.pName);
 *    }
 */
static inline u32p_t timer_isBusy(os_timer_id_t id)
{
    return (u32p_t)_impl_timer_status_isBusy(id.val);
}

/**
 * @brief Get the kernal RTOS system time (ms).
 *
 * @return The value of the total system time (ms).
 **
 * demo usage:
 *
 *    printf("The system consume time: %d\n", timer_system_total_ms());
 */
static inline u32_t timer_system_total_ms(void)
{
    return (u32_t)_impl_timer_total_system_get();
}

#include "semaphore.h"

/**
 * @brief Initialize a new semaphore.
 *
 * @param pName The semaphore name.
 * @param initial The initial count that allows the system take.
 * @param limit The maximum count that it's the semaphore's limitation.
 *
 * @return The semaphore unique id.
 **
 * demo usage:
 *
 *    // We init a binary semaphore count.
 *     os_sem_id_t sample_id = sem_init(0u, 1u, "sample");
 *     if (os_id_is_invalid(sample_id))
 *     {
 *         printf("Semaphore %s init failed\n", sample_id.pName);
 *     }
 *     ...
 */
static inline os_sem_id_t sem_init(u8_t initial, u8_t limit, const char_t *pName)
{
    os_sem_id_t id = {0u};

    id.val = _impl_semaphore_init(initial, limit, pName);
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
 *     u32p_t postcode = sem_take(sample_id, 1000u);
 *     if (PC_IOK(postcode))
 *     {
 *         if (postcode == PC_SC_TIMEOUT)
 *         {
 *             printf("Semaphore take wait timeout\n");
 *         }
 *         else
 *         {
 *             printf("Semaphore take successful\n");
 *         }
 *     }
 *     else
 *     {
 *         printf("Semaphore take error: 0x%x\n", postcode);
 *     }
 *
 *     u32p_t postcode = sem_take(sample_id, OS_WAIT_FOREVER);
 *     if (PC_IOK(postcode))
 *     {
 *        printf("Semaphore take successful\n");
 *     }
 *     else
 *     {
 *         printf("Semaphore take error: 0x%x\n", postcode);
 *     }
 */
static inline u32p_t sem_take(os_sem_id_t id, u32_t timeout_ms)
{
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
 *     u32p_t postcode = sem_give(sample_id);
 *     if (PC_IOK(postcode))
 *     {
 *         printf("Semaphore give successful\n");
 *     }
 *     else
 *     {
 *         printf("Semaphore give error: 0x%x\n", postcode);
 *     }
 */
static inline u32p_t sem_give(os_sem_id_t id)
{
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
 *     u32p_t postcode = sem_flush(sample_id);
 *     if (PC_IOK(postcode))
 *     {
 *         printf("Semaphore flush successful\n");
 *     }
 *     else
 *     {
 *         printf("Semaphore flush error: 0x%x\n", postcode);
 *     }
 */
static inline u32p_t sem_flush(os_sem_id_t id)
{
    return (u32p_t)_impl_semaphore_flush(id.val);
}

#include "mutex.h"
/**
 * @brief Initialize a new mutex.
 *
 * @param pName The mutex name.
 *
 * @return The mutex unique id.
 **
 * demo usage:
 *
 *     os_mutex_id_t sample_id = mutex_init("sample");
 *     if (os_id_is_invalid(sample_id))
 *     {
 *         printf("Mutex %s init failed\n", sample_id.pName);
 *     }
 *     ...
 */
static inline os_mutex_id_t mutex_init(const char_t *pName)
{
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
 *     u32p_t postcode = mutex_lock(sample_id);
 *     if (PC_IOK(postcode))
 *     {
 *         printf("Mutex lock successful\n");
 *     }
 *     else
 *     {
 *         printf("Mutex lock error: 0x%x\n", postcode);
 *     }
 */
static inline u32p_t mutex_lock(os_mutex_id_t id)
{
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
 *     u32p_t postcode = mutex_unlock(sample_id);
 *     if (PC_IOK(postcode))
 *     {
 *         printf("Mutex unlock successful\n");
 *     }
 *     else
 *     {
 *         printf("Mutex unlock error: 0x%x\n", postcode);
 *     }
 */
static inline u32p_t mutex_unlock(os_mutex_id_t id)
{
    return (u32p_t)_impl_mutex_unlock(id.val);
}

#include "event.h"
 /**
  * @brief Initialize a new event.
  *
  * @param pName The event name.
  * @param edge Callback function trigger edge condition.
  *
  * @return The event unique id.
 **
 * demo usage:
 *
 *     os_evt_id_t sample_id = evt_init(0u, NULL, "sample");
 *     if (os_id_is_invalid(sample_id))
 *     {
 *         printf("Event %s init failed\n", sample_id.pName);
 *     }
 *     ...
 */
static inline os_evt_id_t evt_init(u32_t edge, pEvent_callbackFunc_t pCallFun, const char_t *pName)
{
    os_msgq_id_t id = {0u};

    id.val = _impl_event_init(edge, pCallFun, pName);
    id.number = _impl_event_os_id_to_number(id.val);
    id.pName = pName;

    return id;
}

/**
 * @brief Set a event value.
 *
 * @param id The event unique id.
 * @param id The event value.
 *
 * @return The result of the operation.
 **
 * demo usage:
 *
 *     u32p_t postcode = evt_set(sample_id, 0x01u);
 *     if (PC_IOK(postcode))
 *     {
 *         printf("Event set successful\n");
 *     }
 *     else
 *     {
 *         printf("Event set error: 0x%x\n", postcode);
 *     }
 */
static inline u32p_t evt_set(os_evt_id_t id, u32_t event)
{
    return (u32p_t)_impl_event_set(id.val, event);
}

/**
 * @brief Wait a target event.
 *
 * @param id The event unique id.
 * @param pEvent The pointer of event value.
 * @param trigger If the trigger is not zero, All changed bits seen can wake up the thread to handle event.
 * @param listen Current thread listen which bits in the event.
 * @param timeout_ms The event wait timeout setting.
 *
 * @return The result of the operation.
 **
 * demo usage:
 *
 *     u32_t event = 0u;
 *     u32p_t postcode = evt_wait(sample_id, &event, 0x01u, 0x01u, OS_WAIT_FOREVER);
 *     if (PC_IOK(postcode))
 *     {
 *         printf("Event wait successful, The event value is 0x%x\n", event);
 *     }
 *     else
 *     {
 *         printf("Event wait error: 0x%x\n", postcode);
 *     }
 *
 *     u32p_t postcode = evt_wait(sample_id, &event, 0x01u, 0x01u, 1000u);
 *     if (PC_IOK(postcode))
 *     {
 *         if (postcode == PC_SC_TIMEOUT)
 *         {
 *             printf("Event wait timeout\n");
 *         }
 *         else
 *         {
 *             printf("Event wait successful, The event value is 0x%x\n", event);
 *         }
 *     }
 *     else
 *     {
 *         printf("Event wait error: 0x%x\n", postcode);
 *     }
 */
static inline u32p_t evt_wait(os_evt_id_t id, u32_t *pEvent, u32_t trigger, u32_t listen, u32_t timeout_ms)
{
    return (u32p_t)_impl_event_wait(id.val, pEvent, trigger, listen, timeout_ms);
}

#include "queue.h"
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
 *     os_msgq_id_t sample_id = msgq_init((u8_t*)g_sample_msgq, 3u, 10u, "sample");
 *     if (os_id_is_invalid(sample_id))
 *     {
 *         printf("Message queue %s init failed\n", sample_id.pName);
 *     }
 */
static inline os_msgq_id_t msgq_init(const void *pQueueBufferAddr, u16_t elementLen, u16_t elementNum, const char_t *pName)
{
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
 * @param timeout_ms The queue send timeout option.
 *
 * @return The result of the operation.
 **
 * demo usage:
 *
 *     u8_t txdata = 0u;
 *     u32p_t postcode = msgq_send(sample_id, &txdata, 0x01u, OS_WAIT_FOREVER);
 *     if (PC_IOK(postcode))
 *     {
 *         printf("Message queue send successful\n");
 *     }
 *     else
 *     {
 *         printf("Message queue send error: 0x%x\n", postcode);
 *     }
 *
 *     postcode = msgq_send(sample_id, &txdata, 0x01u, 1000u);
 *     if (PC_IOK(postcode))
 *     {
 *         if (postcode == PC_SC_TIMEOUT)
 *         {
 *             printf("Message queue send timeout\n");
 *         }
 *         else
 *         {
 *             printf("Message queue send successful\n");
 *         }
 *     }
 *     else
 *     {
 *         printf("Message queue send error: 0x%x\n", postcode);
 *     }
 */
static inline u32p_t msgq_send(os_msgq_id_t id, const u8_t *pUserBuffer, u16_t bufferSize, u32_t timeout_ms)
{
    return (u32p_t)_impl_queue_send(id.val, pUserBuffer, bufferSize, timeout_ms);
}

/**
 * @brief Receive a queue message.
 *
 * @param id The queue unique id.
 * @param pUserBuffer The pointer of the message buffer address.
 * @param bufferSize The queue buffer size.
 * @param timeout_ms The queue send timeout option.
 *
 * @return The result of the operation.
 **
 * demo usage:
 *
 *     u8_t rxdata = 0u;
 *     u32p_t postcode = msgq_receive(sample_id, &rxdata, 0x01u, OS_WAIT_FOREVER);
 *     if (PC_IOK(postcode))
 *     {
 *         printf("Message queue receive successful, the rx data is 0x%x\n", rxdata);
 *     }
 *     else
 *     {
 *         printf("Message queue receive error: 0x%x\n", postcode);
 *     }
 *
 *     postcode = msgq_receive(sample_id, &rxdata, 0x01u, 1000u);
 *     if (PC_IOK(postcode))
 *     {
 *         if (postcode == PC_SC_TIMEOUT)
 *         {
 *             printf("Message queue receive timeout\n");
 *         }
 *         else
 *         {
 *             printf("Message queue receive successful, the rx data is 0x%x\n", rxdata);
 *         }
 *     }
 *     else
 *     {
 *         printf("Message queue receive error: 0x%x\n", postcode);
 *     }
 */
static inline u32p_t msgq_receive(os_msgq_id_t id, const u8_t *pUserBuffer, u16_t bufferSize, u32_t timeout_ms)
{
    return (u32p_t)_impl_queue_receive(id.val, pUserBuffer, bufferSize, timeout_ms);
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
 *     if (os_id_is_invalid(sample_id))
 *     {
 *         printf("Mutex %s init failed\n", sample_id.pName);
 *     }
 *     ...
 */
static inline b_t os_id_is_invalid(struct os_id id)
{
    return (b_t)_impl_kernal_os_id_is_invalid(id);
}

/**
 * @brief The kernal OS start to run.
 **
 * demo usage:
 *
 *     kernal_atos_run();
 *     // Doesn't arrive
 */
static inline u32p_t kernal_atos_run(void)
{
    return _impl_kernal_at_rtos_run();
}

typedef struct
{
    os_thread_id_t (*thread_init)(os_thread_symbol_t*, pThread_entryFunc_t);
    u32p_t         (*thread_sleep)(u32_t);
    u32p_t         (*thread_resume)(os_thread_id_t);
    u32p_t         (*thread_suspend)(os_thread_id_t);
    u32p_t         (*thread_yield)(void);
    u32p_t         (*thread_delete)(os_thread_id_t);

    os_timer_id_t  (*timer_init)(pTimer_callbackFunc_t, b_t, u32_t, const char_t *);
    u32p_t         (*timer_start)(os_timer_id_t, b_t, u32_t);
    u32p_t         (*timer_stop)(os_timer_id_t);
    u32p_t         (*timer_isBusy)(os_timer_id_t);
    u32_t          (*timer_system_total_ms)(void);

    os_sem_id_t    (*sem_init)(u8_t, u8_t, const char_t *);
    u32p_t         (*sem_take)(os_sem_id_t, u32_t);
    u32p_t         (*sem_give)(os_sem_id_t);
    u32p_t         (*sem_flush)(os_sem_id_t);

    os_mutex_id_t (*mutex_init)(const char_t *);
    u32p_t        (*mutex_lock)(os_mutex_id_t);
    u32p_t        (*mutex_unlock)(os_mutex_id_t);

    os_evt_id_t   (*evt_init)(u32_t, pEvent_callbackFunc_t, const char_t *);
    u32p_t        (*evt_set)(os_evt_id_t, u32_t);
    u32p_t        (*evt_wait)(os_evt_id_t, u32_t *, u32_t, u32_t, u32_t);

    os_msgq_id_t  (*msgq_init)(const void *, u16_t, u16_t, const char_t *);
    u32p_t        (*msgq_send)(os_msgq_id_t, const u8_t *, u16_t, u32_t);
    u32p_t        (*msgq_receive)(os_msgq_id_t, const u8_t *, u16_t, u32_t);

    b_t           (*id_isInvalid)(struct os_id);
    u32p_t        (*at_rtos_run)(void);
}at_rtos_api_t;

extern const at_rtos_api_t AtOS;

#ifdef __cplusplus
}
#endif

#endif /* _AT_RTOS_H_ */

