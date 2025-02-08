/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 **/
#include "k_type.h"
#include "sched_kernel.h"
#include "sched_timer.h"
#include "at_rtos.h"

INIT_OS_THREAD_RUNTIME_NUM_DEFINE(THREAD_RUNTIME_NUMBER_SUPPORTED);
INIT_OS_TIMER_RUNTIME_NUM_DEFINE(TIMER_RUNTIME_NUMBER_SUPPORTED);
INIT_OS_SEM_RUNTIME_NUM_DEFINE(SEMAPHORE_RUNTIME_NUMBER_SUPPORTED);
INIT_OS_MUTEX_RUNTIME_NUM_DEFINE(MUTEX_RUNTIME_NUMBER_SUPPORTED);
INIT_OS_EVT_RUNTIME_NUM_DEFINE(EVENT_RUNTIME_NUMBER_SUPPORTED);
INIT_OS_MSGQ_RUNTIME_NUM_DEFINE(QUEUE_RUNTIME_NUMBER_SUPPORTED);
INIT_OS_POOL_RUNTIME_NUM_DEFINE(POOL_RUNTIME_NUMBER_SUPPORTED);
INIT_OS_PUBLISH_RUNTIME_NUM_DEFINE(PUBLISH_RUNTIME_NUMBER_SUPPORTED);
INIT_OS_SUBSCRIBE_RUNTIME_NUM_DEFINE(SUBSCRIBE_RUNTIME_NUMBER_SUPPORTED);

INIT_OS_THREAD_DEFINE(kernel_th, OS_PRIORITY_KERNEL_SCHEDULE_LEVEL, KERNEL_SCHEDULE_THREAD_STACK_SIZE, kernel_schedule_thread, NULL);
INIT_OS_THREAD_DEFINE(idle_th, OS_PRIORITY_KERNEL_IDLE_LEVEL, KERNEL_IDLE_THREAD_STACK_SIZE, kernel_idle_thread, NULL);
INIT_OS_SEMAPHORE_DEFINE(kernel_sem, 0u, OS_SEM_LIMIT_BINARY);

static pThread_entryFunc_t g_idle_thread_user_entry_fn = NULL;

/**
 * Global At_RTOS application interface init.
 */
#ifdef OS_API_ENABLED
const at_rtos_api_t os = {
    .thread_init = os_thread_init,
    .thread_sleep = os_thread_sleep,
    .thread_resume = os_thread_resume,
    .thread_suspend = os_thread_suspend,
    .thread_yield = os_thread_yield,
    .thread_delete = os_thread_delete,
    .thread_idle_fn_register = os_thread_idle_callback_register,
    .thread_user_data_set = os_thread_user_data_set,
    .thread_user_data_get = os_thread_user_data_get,
    .thread_idle_id_probe = os_thread_idle_id_probe,
    .thread_stack_free_size_probe = os_thread_stack_free_size_probe,

    .timer_init = os_timer_init,
    .timer_automatic = os_timer_automatic,
    .timer_start = os_timer_start,
    .timer_stop = os_timer_stop,
    .timer_busy = os_timer_busy,
    .timer_system_total_ms = os_timer_system_total_ms,
    .timer_system_busy_wait = os_timer_system_busy_wait,

    .sem_init = os_sem_init,
    .sem_take = os_sem_take,
    .sem_give = os_sem_give,
    .sem_flush = os_sem_flush,

    .mutex_init = os_mutex_init,
    .mutex_lock = os_mutex_lock,
    .mutex_unlock = os_mutex_unlock,

    .evt_init = os_evt_init,
    .evt_set = os_evt_set,
    .evt_wait = os_evt_wait,

    .msgq_init = os_msgq_init,
    .msgq_put = os_msgq_put,
    .msgq_get = os_msgq_get,
    .msgq_num_probe = os_msgq_num_probe,

    .pool_init = os_pool_init,
    .pool_take = os_pool_take,
    .pool_release = os_pool_release,

    .publish_init = os_publish_init,
    .publish_data_submit = os_publish_data_submit,
    .subscribe_init = os_subscribe_init,
    .subscribe_register = os_subscribe_register,
    .subscribe_data_apply = os_subscribe_data_apply,
    .subscribe_data_is_ready = os_subscribe_data_is_ready,

    .id_isInvalid = os_id_is_invalid,
    .schedule_run = os_kernel_run,
    .schedule_is_running = os_kernel_is_running,
    .schedule_lock = os_kernel_lock,
    .schedule_unlock = os_kernel_unlock,
    .current_thread = os_thread_self_probe,

    .trace_versison = os_trace_firmware_version,
    .trace_postcode_fn_register = os_trace_postcode_callback_register,
    .trace_postcode = os_trace_failed_postcode,
    .trace_thread = os_trace_foreach_thread,
    .trace_time = os_trace_analyze,

    .object_free = os_object_free_force,
};
#endif

/**
 * @brief To issue a kernel message notification.
 */
void kthread_message_notification(void)
{
    PC_IF(os_sem_give(kernel_sem), PC_PASS)
    {
        /* TODO */
    }
}

/**
 * @brief To check if the kernel message arrived.
 */
_i32p_t kthread_message_arrived(void)
{
    return os_sem_take(kernel_sem, OS_TIME_FOREVER_VAL);
}

/**
 * @brief User register idle function callback.
 */
void kthread_message_idle_loop_fn(void)
{
    if (g_idle_thread_user_entry_fn) {
        g_idle_thread_user_entry_fn(NULL);
    }
}

/**
 * @brief Register idle thread user fucntion.
 */
void _impl_kthread_idle_user_callback_register(const pThread_entryFunc_t fn)
{
    g_idle_thread_user_entry_fn = fn;
}

/**
 * @brief Get the idle thread id.
 */
os_thread_id_t *_impl_idle_thread_id_get(void)
{
    return &idle_th;
}
