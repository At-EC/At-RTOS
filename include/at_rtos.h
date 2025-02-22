/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 **/
#ifndef _AT_RTOS_H_
#define _AT_RTOS_H_

#include "k_type.h"
#include "k_struct.h"
#include "k_config.h"
#include "k_trace.h"
#include "postcode.h"
#include "static_init.h"

#if (OS_TYPEDEF_ENABLED)
typedef _char_t char_t;
typedef _uchar_t uchar_t;
typedef _u8_t u8_t;
typedef _u16_t u16_t;
typedef _u32_t u32_t;
typedef _u64_t u64_t;
typedef _i8_t i8_t;
typedef _i16_t i16_t;
typedef _i32_t i32_t;
typedef _i64_t i64_t;
typedef _b_t b_t;
typedef _i32p_t i32p_t;
#endif

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

#if (OS_ID_ENHANCEMENT_ENABLED)
#define OS_ID_NODATA (0)
#else
#define OS_ID_NODATA (1)
#endif

#if (OS_ID_NODATA)
typedef void *os_thread_id_t;
typedef void *os_timer_id_t;
typedef void *os_sem_id_t;
typedef void *os_mutex_id_t;
typedef void *os_evt_id_t;
typedef void *os_msgq_id_t;
typedef void *os_pool_id_t;
typedef void *os_publish_id_t;
typedef void *os_subscribe_id_t;
#else
typedef struct os_id os_thread_id_t;
typedef struct os_id os_timer_id_t;
typedef struct os_id os_sem_id_t;
typedef struct os_id os_mutex_id_t;
typedef struct os_id os_evt_id_t;
typedef struct os_id os_msgq_id_t;
typedef struct os_id os_pool_id_t;
typedef struct os_id os_publish_id_t;
typedef struct os_id os_subscribe_id_t;
#endif

typedef struct evt_val os_evt_val_t;

#define OS_ID_SET(p_handle, u32_value) p_handle->u32_val = (u32_value)

#define OS_PRIORITY_INVALID             (OS_PRIOTITY_INVALID_LEVEL)
#define OS_PRIORITY_APPLICATION_HIGHEST (OS_PRIORITY_APPLICATION_HIGHEST_LEVEL)
#define OS_PRIORITY_APPLICATION_LOWEST  (OS_PRIORITY_APPLICATION_LOWEST_LEVEL)

#define OS_PRIORITY_PREEMPT_SET(p)     (p)
#define OS_PRIORITY_COOPERATION_SET(c) (-(OS_PRIOTITY_COOPERATION_NUM - (c)))

#define OS_STACK_INIT(name, size)                                     STACK_STATIC_VALUE_DEFINE(name, size)
#define OS_THREAD_INIT(id_name, priority, stack_size, pEntryFn, pArg) INIT_OS_THREAD_DEFINE(id_name, priority, stack_size, pEntryFn, pArg)
#define OS_TIMER_INIT(id_name, pEntryFunc)                            INIT_OS_TIMER_DEFINE(id_name, pEntryFunc)
#define OS_SEMAPHORE_INIT(id_name, remain, limit)                     INIT_OS_SEMAPHORE_DEFINE(id_name, remain, limit)
#define OS_MUTEX_INIT(id_name)                                        INIT_OS_MUTEX_DEFINE(id_name)
#define OS_EVT_INIT(id_name, anyMask, modeMask, dirMask, init)        INIT_OS_EVT_DEFINE(id_name, anyMask, modeMask, dirMask, init)
#define OS_MSGQ_INIT(id_name, pBufAddr, len, num)                     INIT_OS_MSGQ_DEFINE(id_name, pBufAddr, len, num)
#define OS_POOL_INIT(id_name, pMemAddr, len, num)                     INIT_OS_POOL_DEFINE(id_name, pMemAddr, len, num)
#define OS_SUBSCRIBE_INIT(id_name, pDataAddr, size)                   INIT_OS_SUBSCRIBE_DEFINE(id_name, pDataAddr, size)
#define OS_PUBLISH_INIT(id_name, pDataAddr, size)                     INIT_OS_PUBLISH_DEFINE(id_name, pDataAddr, size)

#define OS_FUNC_PRIO_0              (INIT_LEVEL_0)
#define OS_FUNC_PRIO_1              (INIT_LEVEL_1)
#define OS_FUNC_PRIO_2              (INIT_LEVEL_2)
#define OS_FUNC_PRIO_3              (INIT_LEVEL_3)
#define OS_FUNC_PRIO_4              (INIT_LEVEL_4)
#define OS_FUNC_INIT(init_fn, prio) INIT_FUNC_DEFINE(init_fn, prio)

extern _u32_t impl_kernel_irq_disable(void);
extern void impl_kernel_irq_enable(_u32_t val);
struct foreach_item {
    u8_t i;
    u32_t u32_val;
};
#define OS_CRITICAL_SECTION()                                                                                                              \
    for (struct foreach_item __item = {.i = 0, .u32_val = impl_kernel_irq_disable()}; !__item.i;                                           \
         impl_kernel_irq_enable(__item.u32_val), __item.i++)
#define OS_ENTER_CRITICAL_SECTION() u32_t __val = impl_kernel_irq_disable()
#define OS_EXIT_CRITICAL_SECTION()  impl_kernel_irq_enable(__val)

/**
 * @brief Initialize a thread, and put it to pending list that are ready to run.
 *
 * @param pEntryFun The pointer of the thread entry function. Thread function must be designed to never return.
 * @param pStackAddr The pointer of the thread stack address. The stack memory must be predefined and allocated in your system or kernel
 * will allocate it internally.
 * @param stackSize The size of the the stack is base on your specified variable.
 * @param priority The thread priority specified the thread's priority when the AtOS do kernel schedule.
 * @param pArg The thread entry function argument.
 * @param pName The thread name.
 *
 * @return The value of thread unique id.
 */
static inline os_thread_id_t os_thread_init(u32_t *pStackAddr, u32_t size, i16_t priority, pThread_entryFunc_t pEntryFun, void *pArg,
                                            const char_t *pName)
{
    extern u32_t _impl_thread_init(pThread_entryFunc_t pEntryFun, u32_t * pAddress, u32_t size, _i16_t priority, void *pArg,
                                   const _char_t *pName);

#if (OS_ID_NODATA)
    return (os_thread_id_t)_impl_thread_init(pEntryFun, pStackAddr, size, priority, pArg, pName);
#else
    os_thread_id_t id = {0u};
    id.u32_val = _impl_thread_init(pEntryFun, pStackAddr, size, priority, pArg, pName);
    id.pName = pName;
    return id;
#endif
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

#if (OS_ID_NODATA)
    return (i32p_t)_impl_thread_user_data_register((u32_t)id, pUserData);
#else
    return (i32p_t)_impl_thread_user_data_register(id.u32_val, pUserData);
#endif
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

#if (OS_ID_NODATA)
    return (void *)_impl_thread_user_data_get((u32_t)id);
#else
    return (void *)_impl_thread_user_data_get(id.u32_val);
#endif
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

#if (OS_ID_NODATA)
    return (i32p_t)_impl_thread_resume((u32_t)id);
#else
    return (i32p_t)_impl_thread_resume(id.u32_val);
#endif
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

#if (OS_ID_NODATA)
    return (i32p_t)_impl_thread_suspend((u32_t)id);
#else
    return (i32p_t)_impl_thread_suspend(id.u32_val);
#endif
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

    i32p_t pc = 0;
#if (OS_ID_NODATA)
    pc = _impl_thread_delete((u32_t)id);
    id = NULL;
#else
    pc = _impl_thread_delete(id.u32_val);
    id.p_val = NULL;
#endif
    return pc;
}

/**
 * @brief Delete current running thread.
 *
 * @return The result of thread delete operation.
 */
static inline i32p_t os_thread_delete_self(void)
{
    extern i32p_t _impl_thread_delete(u32_t ctx);
    extern thread_context_t *kernel_thread_runContextGet(void);

    return (i32p_t)_impl_thread_delete((u32_t)kernel_thread_runContextGet());
}

/**
 * @brief Get the current running thread id.
 *
 * @return The running thread context.
 */
static inline os_thread_id_t os_thread_id_self(void)
{
    extern thread_context_t *kernel_thread_runContextGet(void);
    extern const _char_t *_impl_thread_name_get(u32_t ctx);

#if (OS_ID_NODATA)
    return (os_thread_id_t)kernel_thread_runContextGet();
#else
    os_thread_id_t id = {0u};
    id.u32_val = (u32_t)kernel_thread_runContextGet();
    id.pName = _impl_thread_name_get(id.u32_val);
    return id;
#endif
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
static inline os_thread_id_t *os_thread_idle_id_probe(void)
{
    extern os_thread_id_t *_impl_idle_thread_id_get(void);
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

#if (OS_ID_NODATA)
    return (u32_t)_impl_thread_stack_free_size_get((u32_t)id);
#else
    return (u32_t)_impl_thread_stack_free_size_get(id.u32_val);
#endif
}

/**
 * @brief Initialize a new timer, or allocate a temporary timer to run.
 *
 * @param pCallFun The timer entry function pointer.
 * @param pUserData The timer callback user data.
 * @param pName The timer's name, it supported NULL pointer.
 *
 * @return The value of the timer unique id.
 */
static inline os_timer_id_t os_timer_init(pTimer_callbackFunc_t pEntryFun, void *pUserData, const char_t *pName)
{
    extern u32_t _impl_timer_init(pTimer_callbackFunc_t pCallFun, void *pUserData, const char_t *pName);

#if (OS_ID_NODATA)
    return (os_timer_id_t)_impl_timer_init(pEntryFun, pUserData, pName);
#else
    os_timer_id_t id = {0u};
    id.u32_val = _impl_timer_init(pEntryFun, pUserData, pName);
    id.pName = pName;
    return id;
#endif
}

/**
 * @brief Allocate a temporary timer to run and release it when it stops.
 *
 * @param pCallFun The timer entry function pointer.
 * @param pUserData The timer callback user data.
 * @param pName The timer's name, it supported NULL pointer.
 *
 * @return The value of the timer unique id.
 */
static inline os_timer_id_t os_timer_automatic(pTimer_callbackFunc_t pEntryFun, void *pUserData, const char_t *pName)
{
    extern u32_t _impl_timer_automatic(pTimer_callbackFunc_t pCallFun, void *pUserData, const char_t *pName);

#if (OS_ID_NODATA)
    return (os_timer_id_t)_impl_timer_automatic(pEntryFun, pUserData, pName);
#else
    os_timer_id_t id = {0u};
    id.u32_val = _impl_timer_automatic(pEntryFun, pUserData, pName);
    id.pName = pName;
    return id;
#endif
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

#if (OS_ID_NODATA)
    return (i32p_t)_impl_timer_start((u32_t)id, (u8_t)control, (u32_t)timeout_ms);
#else
    return (i32p_t)_impl_timer_start(id.u32_val, (u8_t)control, (u32_t)timeout_ms);
#endif
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

#if (OS_ID_NODATA)
    return (i32p_t)_impl_timer_stop((u32_t)id);
#else
    return (i32p_t)_impl_timer_stop(id.u32_val);
#endif
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

#if (OS_ID_NODATA)
    return (i32p_t)_impl_timer_busy((u32_t)id);
#else
    return (i32p_t)_impl_timer_busy(id.u32_val);
#endif
}

/**
 * @brief Check the timer to confirm if it's already scheduled in the waiting list.
 *
 * @param id The timer unique id.
 *
 * @return The true result indicates time busy, otherwise is free status.
 */
static inline i32p_t os_timer_delete(os_timer_id_t id)
{
    extern i32p_t _impl_timer_delete(u32_t ctx);

    i32p_t pc = 0;
#if (OS_ID_NODATA)
    pc = _impl_timer_delete((u32_t)id);
    id = NULL;
#else
    pc = _impl_timer_delete(id.u32_val);
    id.p_val = NULL;
#endif

    return pc;
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
 * @brief System busy wait (us).
 *
 * @return The result of system busy wait.
 */
static inline u32_t os_timer_system_busy_wait(u32_t us)
{
    extern u32_t _impl_system_busy_wait(u32_t us);

    return _impl_system_busy_wait(us);
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

#if (OS_ID_NODATA)
    return (os_sem_id_t)_impl_semaphore_init(remain, limit, pName);
#else
    os_sem_id_t id = {0u};
    id.u32_val = _impl_semaphore_init(remain, limit, pName);
    id.pName = pName;

    return id;
#endif
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

#if (OS_ID_NODATA)
    return (i32p_t)_impl_semaphore_take((u32_t)id, timeout_ms);
#else
    return (i32p_t)_impl_semaphore_take(id.u32_val, timeout_ms);
#endif
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

#if (OS_ID_NODATA)
    return (i32p_t)_impl_semaphore_give((u32_t)id);
#else
    return (i32p_t)_impl_semaphore_give(id.u32_val);
#endif
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

#if (OS_ID_NODATA)
    return (i32p_t)_impl_semaphore_flush((u32_t)id);
#else
    return (i32p_t)_impl_semaphore_flush(id.u32_val);
#endif
}

/**
 * @brief Semaphore delete.
 *
 * @param id The semaphore unique id.
 *
 * @return The result of the operation.
 */
static inline i32p_t os_sem_delete(os_sem_id_t id)
{
    extern i32p_t _impl_semaphore_delete(u32_t ctx);

    i32p_t pc = 0;
#if (OS_ID_NODATA)
    pc = _impl_semaphore_delete((u32_t)id);
    id = NULL;
#else
    pc = _impl_semaphore_delete(id.u32_val);
    id.p_val = NULL;
#endif
    return pc;
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

#if (OS_ID_NODATA)
    return (os_mutex_id_t)_impl_mutex_init(pName);
#else
    os_mutex_id_t id = {0u};
    id.u32_val = _impl_mutex_init(pName);
    id.pName = pName;

    return id;
#endif
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

#if (OS_ID_NODATA)
    return (i32p_t)_impl_mutex_lock((u32_t)id);
#else
    return (i32p_t)_impl_mutex_lock(id.u32_val);
#endif
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

#if (OS_ID_NODATA)
    return (i32p_t)_impl_mutex_unlock((u32_t)id);
#else
    return (i32p_t)_impl_mutex_unlock(id.u32_val);
#endif
}

/**
 * @brief Mutex delete.
 *
 * @param id The mutex unique id.
 *
 * @return The result of the operation.
 */
static inline i32p_t os_mutex_delete(os_mutex_id_t id)
{
    extern i32p_t _impl_mutex_delete(u32_t ctx);

    i32p_t pc = 0;
#if (OS_ID_NODATA)
    pc = _impl_mutex_delete((u32_t)id);
    id = NULL;
#else
    pc = _impl_mutex_delete(id.u32_val);
    id.p_val = NULL;
#endif
    return pc;
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

#if (OS_ID_NODATA)
    return (os_evt_id_t)_impl_event_init(anyMask, modeMask, dirMask, init, pName);
#else
    os_evt_id_t id = {0u};
    id.u32_val = _impl_event_init(anyMask, modeMask, dirMask, init, pName);
    id.pName = pName;

    return id;
#endif
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

#if (OS_ID_NODATA)
    return (i32p_t)_impl_event_value_get((u32_t)id, pValue);
#else
    return (i32p_t)_impl_event_value_get(id.u32_val, pValue);
#endif
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

#if (OS_ID_NODATA)
    return (i32p_t)_impl_event_set((u32_t)id, set, clear, toggle);
#else
    return (i32p_t)_impl_event_set(id.u32_val, set, clear, toggle);
#endif
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

#if (OS_ID_NODATA)
    return (i32p_t)_impl_event_wait((u32_t)id, pEvtData, listen_mask, (u32_t)timeout_ms);
#else
    return (i32p_t)_impl_event_wait(id.u32_val, pEvtData, listen_mask, (u32_t)timeout_ms);
#endif
}

/**
 * @brief Event delete.
 *
 * @param id The event unique id.
 *
 * @return The result of the operation.
 */
static inline i32p_t os_evt_delete(os_evt_id_t id)
{
    extern i32p_t _impl_event_delete(u32_t ctx);

    i32p_t pc = 0;
#if (OS_ID_NODATA)
    pc = _impl_event_delete((u32_t)id);
    id = NULL;
#else
    pc = _impl_event_delete(id.u32_val);
    id.p_val = NULL;
#endif
    return pc;
}

/**
 * @brief Initialize a new queue.
 *
 * @param pName The queue name.
 * @param pStackAddr The pointer of the queue buffer address. The queue buffer must be predefined and allocated in your system or kernel
 * will allocate it internally.
 * @param len The element size.
 * @param num The element number.
 *
 * @return The queue unique id.
 */
static inline os_msgq_id_t os_msgq_init(const void *pBufferAddr, u16_t len, u16_t num, const char_t *pName)
{
    extern u32_t _impl_queue_init(const void *pQueueBufferAddr, u16_t elementLen, u16_t elementNum, const char_t *pName);

#if (OS_ID_NODATA)
    return (os_msgq_id_t)_impl_queue_init(pBufferAddr, len, num, pName);
#else
    os_msgq_id_t id = {0u};
    id.u32_val = _impl_queue_init(pBufferAddr, len, num, pName);
    id.pName = pName;

    return id;
#endif
}

/**
 * @brief Send a queue message.
 *
 * @param id The queue unique id.
 * @param pUserBuffer The pointer of the message buffer address.
 * @param size The queue buffer size, the default 0 means that it use the init item size.
 * @param isToFront The direction of the message operation.
 * @param timeout_ms The queue send timeout option.
 *
 * @return The result of the operation.
 */
static inline i32p_t os_msgq_put(os_msgq_id_t id, const u8_t *pUserBuffer, u16_t size, b_t isToFront, os_timeout_t timeout_ms)
{
    extern i32p_t _impl_queue_send(u32_t ctx, const u8_t *pUserBuffer, u16_t bufferSize, b_t isToFront, u32_t timeout_ms);

#if (OS_ID_NODATA)
    return (i32p_t)_impl_queue_send((u32_t)id, pUserBuffer, size, isToFront, (u32_t)timeout_ms);
#else
    return (i32p_t)_impl_queue_send(id.u32_val, pUserBuffer, size, isToFront, (u32_t)timeout_ms);
#endif
}

/**
 * @brief Receive a queue message.
 *
 * @param id The queue unique id.
 * @param pUserBuffer The pointer of the message buffer address.
 * @param size The queue buffer size, the default 0 means that it use the init item size.
 * @param isFromBack The direction of the message operation.
 * @param timeout_ms The queue send timeout option.
 *
 * @return The result of the operation.
 */
static inline i32p_t os_msgq_get(os_msgq_id_t id, const u8_t *pUserBuffer, u16_t size, b_t isFromBack, os_timeout_t timeout_ms)
{
    extern i32p_t _impl_queue_receive(u32_t ctx, const u8_t *pUserBuffer, u16_t bufferSize, b_t isFromBack, u32_t timeout_ms);

#if (OS_ID_NODATA)
    return (i32p_t)_impl_queue_receive((u32_t)id, pUserBuffer, size, isFromBack, (u32_t)timeout_ms);
#else
    return (i32p_t)_impl_queue_receive(id.u32_val, pUserBuffer, size, isFromBack, (u32_t)timeout_ms);
#endif
}

/**
 * @brief Delete a message queue.
 *
 * @param ctx The queue unique id.
 *
 * @return The result of the operation.
 */
static inline i32p_t os_msgq_delete(os_msgq_id_t id)
{
    extern i32p_t _impl_queue_delete(u32_t ctx);

    i32p_t pc = 0;
#if (OS_ID_NODATA)
    pc = _impl_queue_delete((u32_t)id);
    id = NULL;
#else
    pc = _impl_queue_delete(id.u32_val);
    id.p_val = NULL;
#endif
    return pc;
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

#if (OS_ID_NODATA)
    return (u32_t)_impl_queue_num_probe((u32_t)id);
#else
    return (u32_t)_impl_queue_num_probe(id.u32_val);
#endif
}

/**
 * @brief Initialize a new pool.
 *
 * @param pName The pool name.
 * @param pMemAddr The pointer of the pool buffer address. The pool memory must be predefined and allocated in your system or kernel
 * will allocate it internally.
 * @param size The element size.
 * @param num The element number.
 *
 * @return The pool unique id.
 */
static inline os_pool_id_t os_pool_init(const void *pMemAddr, u16_t size, u16_t num, const char_t *pName)
{
    extern u32_t _impl_pool_init(const void *pMemAddr, u16_t elementLen, u16_t elementNum, const char_t *pName);

#if (OS_ID_NODATA)
    return (os_pool_id_t)_impl_pool_init(pMemAddr, size, num, pName);
#else
    os_pool_id_t id = {0u};
    id.u32_val = _impl_pool_init(pMemAddr, size, num, pName);
    id.pName = pName;

    return id;
#endif
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

#if (OS_ID_NODATA)
    return (i32p_t)_impl_pool_take((u32_t)id, ppUserMem, size, (u32_t)timeout_ms);
#else
    return (i32p_t)_impl_pool_take(id.u32_val, ppUserMem, size, (u32_t)timeout_ms);
#endif
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

#if (OS_ID_NODATA)
    return (i32p_t)_impl_pool_release((u32_t)id, ppUserMem);
#else
    return (i32p_t)_impl_pool_release(id.u32_val, ppUserMem);
#endif
}

/**
 * @brief Delete memory pool.
 *
 * @param id The pool unique id.
 *
 * @return The result of the operation.
 */
static inline i32p_t os_pool_delete(os_pool_id_t id)
{
    extern i32p_t _impl_pool_delete(u32_t ctx);

    i32p_t pc = 0;
#if (OS_ID_NODATA)
    pc = _impl_pool_delete((u32_t)id);
    id = NULL;
#else
    pc = _impl_pool_delete(id.u32_val);
    id.p_val = NULL;
#endif
    return pc;
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

#if (OS_ID_NODATA)
    return (os_publish_id_t)_impl_publish_init(pName);
#else
    os_publish_id_t id = {0u};
    id.u32_val = _impl_publish_init(pName);
    id.pName = pName;

    return id;
#endif
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

#if (OS_ID_NODATA)
    return _impl_publish_data_submit((u32_t)id, pData, size);
#else
    return _impl_publish_data_submit(id.u32_val, pData, size);
#endif
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

#if (OS_ID_NODATA)
    return (os_subscribe_id_t)_impl_subscribe_init(pDataAddr, size, pName);
#else
    os_subscribe_id_t id = {0u};
    id.u32_val = _impl_subscribe_init(pDataAddr, size, pName);
    id.pName = pName;

    return id;
#endif
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

#if (OS_ID_NODATA)
    return _impl_subscribe_data_is_ready((u32_t)id);
#else
    return _impl_subscribe_data_is_ready(id.u32_val);
#endif
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

#if (OS_ID_NODATA)
    return _impl_subscribe_register((u32_t)subscribe_id, (u32_t)publish_id, isMute, pNotificationHandler);
#else
    return _impl_subscribe_register(subscribe_id.u32_val, publish_id.u32_val, isMute, pNotificationHandler);
#endif
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

#if (OS_ID_NODATA)
    return _impl_subscribe_data_apply((u32_t)subscribe_id, pData, pDataLen);
#else
    return _impl_subscribe_data_apply(subscribe_id.u32_val, pData, pDataLen);
#endif
}

/**
 * @brief Check if the thread unique id if is's invalid.
 *
 * @param id The provided unique id.
 *
 * @return The value of true is invalid, otherwise is valid.
 */
#if (OS_ID_NODATA)
static inline b_t os_id_is_invalid(void *id)
{
    return (id == NULL) ? true : false;
}
#else
static inline b_t os_id_is_invalid(struct os_id id)
{
    return (b_t)kernel_os_id_is_invalid(id);
}
#endif

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
 * @brief Force kernel schedule stop.
 */
static inline void os_kernel_lock(void)
{
    extern void _impl_kernel_schedule_lock(void);

    _impl_kernel_schedule_lock();
}

/**
 * @brief Kernel schedule recovery.
 */
static inline void os_kernel_unlock(void)
{
    extern void _impl_kernel_schedule_unlock(void);

    _impl_kernel_schedule_unlock();
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

/* It defined the AtOS extern symbol for convenience use, but it has extra memory consumption */
#if (OS_API_ENABLED)
typedef struct {
    os_thread_id_t (*thread_init)(u32_t *, u32_t, i16_t, pThread_entryFunc_t, void *, const char_t *);
    i32p_t (*thread_sleep)(u32_t);
    i32p_t (*thread_resume)(os_thread_id_t);
    i32p_t (*thread_suspend)(os_thread_id_t);
    i32p_t (*thread_yield)(void);
    i32p_t (*thread_delete)(os_thread_id_t);
    i32p_t (*thread_delete_self)(void);
    os_thread_id_t (*thread_id_self)(void);
    i32p_t (*thread_user_data_set)(os_thread_id_t, void *);
    void *(*thread_user_data_get)(os_thread_id_t);
    void (*thread_idle_fn_register)(const pThread_entryFunc_t);
    os_thread_id_t *(*thread_idle_id_probe)(void);
    u32_t (*thread_stack_free_size_probe)(os_thread_id_t);

    os_timer_id_t (*timer_init)(pTimer_callbackFunc_t, void *, const char_t *);
    os_timer_id_t (*timer_automatic)(pTimer_callbackFunc_t, void *, const char_t *);
    i32p_t (*timer_start)(os_timer_id_t, os_timer_ctrl_t, os_timeout_t);
    i32p_t (*timer_stop)(os_timer_id_t);
    i32p_t (*timer_busy)(os_timer_id_t);
    i32p_t (*timer_delete)(os_timer_id_t);
    u32_t (*timer_system_total_ms)(void);
    u32_t (*timer_system_busy_wait)(u32_t);

    os_sem_id_t (*sem_init)(u8_t, u8_t, const char_t *);
    i32p_t (*sem_take)(os_sem_id_t, os_timeout_t);
    i32p_t (*sem_give)(os_sem_id_t);
    i32p_t (*sem_flush)(os_sem_id_t);
    i32p_t (*sem_delete)(os_sem_id_t);

    os_mutex_id_t (*mutex_init)(const char_t *);
    i32p_t (*mutex_lock)(os_mutex_id_t);
    i32p_t (*mutex_unlock)(os_mutex_id_t);
    i32p_t (*mutex_delete)(os_mutex_id_t);

    os_evt_id_t (*evt_init)(u32_t, u32_t, u32_t, u32_t, const char_t *);
    i32p_t (*evt_set)(os_evt_id_t, u32_t, u32_t, u32_t);
    i32p_t (*evt_wait)(os_evt_id_t, os_evt_val_t *, u32_t, os_timeout_t);
    i32p_t (*evt_delete)(os_evt_id_t);

    os_msgq_id_t (*msgq_init)(const void *, u16_t, u16_t, const char_t *);
    i32p_t (*msgq_put)(os_msgq_id_t, const u8_t *, u16_t, b_t, os_timeout_t);
    i32p_t (*msgq_get)(os_msgq_id_t, const u8_t *, u16_t, b_t, os_timeout_t);
    i32p_t (*msgq_delete)(os_msgq_id_t);
    u32_t (*msgq_num_probe)(os_msgq_id_t);

    os_pool_id_t (*pool_init)(const void *, u16_t, u16_t, const char_t *);
    i32p_t (*pool_take)(os_pool_id_t, void **, u16_t, os_timeout_t);
    i32p_t (*pool_release)(os_pool_id_t, void **);
    i32p_t (*pool_delete)(os_pool_id_t);

    os_publish_id_t (*publish_init)(const char_t *);
    i32p_t (*publish_data_submit)(os_publish_id_t, const void *, u16_t);
    os_subscribe_id_t (*subscribe_init)(void *, u16_t, const char_t *);
    i32p_t (*subscribe_register)(os_subscribe_id_t, os_publish_id_t, b_t, pSubscribe_callbackFunc_t);
    i32p_t (*subscribe_data_apply)(os_subscribe_id_t, void *, u16_t *);
    b_t (*subscribe_data_is_ready)(os_subscribe_id_t);

#if (OS_ID_NODATA)
    b_t (*id_isInvalid)(void *);
#else
    b_t (*id_isInvalid)(struct os_id);
#endif
    i32p_t (*schedule_run)(void);
    b_t (*schedule_is_running)(void);
    void (*schedule_lock)(void);
    void (*schedule_unlock)(void);

    void (*trace_versison)(void);
    void (*trace_postcode_fn_register)(const pTrace_postcodeFunc_t);
    b_t (*trace_postcode)(const pTrace_postcodeFunc_t);
    void (*trace_thread)(const pTrace_threadFunc_t);
    void (*trace_time)(const pTrace_analyzeFunc_t);
} at_rtos_api_t;

extern const at_rtos_api_t os;
#endif

#endif /* _AT_RTOS_H_ */
