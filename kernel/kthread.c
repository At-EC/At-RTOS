/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 **/

#include "at_rtos.h"
#include "ktype.h"
#include "kernel.h"
#include "timer.h"
#include "postcode.h"

#ifdef __cplusplus
extern "C" {
#endif

#define _PCER PC_IER(PC_OS_CMPT_KERNEL_2)

/* Local defined the kernel thread stack */
static u32_t _kernel_schedule[((u32_t)(KERNEL_SCHEDULE_THREAD_STACK_SIZE) / sizeof(u32_t))] = {0u};
static u32_t _kernel_idle[((u32_t)(KERNEL_IDLE_THREAD_STACK_SIZE) / sizeof(u32_t))] = {0u};

/**
 * Global At_RTOS application interface init.
 */
#if (OS_INTERFACE_EXTERN_USE_ENABLE)
const at_rtos_api_t os = {
    .thread_init = os_thread_init,
    .thread_sleep = os_thread_sleep,
    .thread_resume = os_thread_resume,
    .thread_suspend = os_thread_suspend,
    .thread_yield = os_thread_yield,
    .thread_delete = os_thread_delete,

    .timer_init = os_timer_init,
    .timer_start = os_timer_start,
    .timer_stop = os_timer_stop,
    .timer_busy = os_timer_busy,
    .timer_system_total_ms = os_timer_system_total_ms,

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
    .id_current_thread = os_id_current_thread,

    .trace_firmware = os_trace_firmware_print,
    .trace_postcode = os_trace_postcode_print,
    .trace_kernel = os_trace_kernel_print,
};
#endif

/**
 * Data structure for location kernel thread.
 */
typedef struct {
    /* kernel schedule thread id */
    os_thread_id_t schedule_id;

    /* kernel idle thread id */
    os_thread_id_t idle_id;

    /* kernel schedule semaphore id */
    os_sem_id_t sem_id;
} _kthread_resource_t;

/**
 * Local timer resource
 */
static _kthread_resource_t g_kernel_thread_resource = {
    .schedule_id =
        {
            .pName = "kernel",
            .val = 0u,
            .number = KERNEL_SCHEDULE_THREAD_INSTANCE,
        },
    .idle_id =
        {
            .pName = "idle",
            .val = sizeof(thread_context_t),
            .number = KERNEL_IDLE_THREAD_INSTANCE,
        },
    .sem_id =
        {
            .pName = "kernel",
            .number = KERNEL_SCHEDULE_SEMAPHORE_INSTANCE,
        },
};

/**
 * @brief To issue a kernel message notification.
 */
void kthread_message_notification(void)
{
    PC_IF(os_sem_give(g_kernel_thread_resource.sem_id), PC_PASS)
    {
        /* TODO */
    }
}

/**
 * @brief To check if the kernel message arrived.
 */
i32p_t kthread_message_arrived(void)
{
    return os_sem_take(g_kernel_thread_resource.sem_id, OS_TIME_FOREVER_VAL);
}

/**
 * @brief The AtOS kernel internal use thread and semaphore init.
 */
void kthread_init(void)
{
    ENTER_CRITICAL_SECTION();

    thread_context_t kernel_thread[KERNEL_APPLICATION_THREAD_INSTANCE] = {
        [KERNEL_SCHEDULE_THREAD_INSTANCE] =
            {
                .head =
                    {
                        .id = g_kernel_thread_resource.schedule_id.val,
                        .pName = g_kernel_thread_resource.schedule_id.pName,
                        .linker = LINKER_NULL,
                    },
                .priority =
                    {
                        .level = OS_PRIOTITY_HIGHEST_LEVEL,
                    },
                .pEntryFunc = kernel_schedule_thread,
                .pStackAddr = (u32_t *)&_kernel_schedule[0],
                .stackSize = KERNEL_SCHEDULE_THREAD_STACK_SIZE,
                .PSPStartAddr = (u32_t)kernel_stack_frame_init(kernel_schedule_thread, (u32_t *)&_kernel_schedule[0],
                                                               KERNEL_SCHEDULE_THREAD_STACK_SIZE),

            },

        [KERNEL_IDLE_THREAD_INSTANCE] =
            {
                .head =
                    {
                        .id = g_kernel_thread_resource.idle_id.val,
                        .pName = g_kernel_thread_resource.idle_id.pName,
                        .linker = LINKER_NULL,
                    },
                .priority =
                    {
                        .level = OS_PRIOTITY_LOWEST_LEVEL,
                    },
                .pEntryFunc = kernel_idle_thread,
                .pStackAddr = (u32_t *)&_kernel_idle[0],
                .stackSize = KERNEL_IDLE_THREAD_STACK_SIZE,
                .PSPStartAddr =
                    (u32_t)kernel_stack_frame_init(kernel_idle_thread, (u32_t *)&_kernel_idle[0], KERNEL_IDLE_THREAD_STACK_SIZE),
            },
    };

    thread_context_t *pCurThread = (thread_context_t *)kernel_member_id_toContainerStartAddress(KERNEL_MEMBER_THREAD);
    os_memcpy((u8_t *)pCurThread, (u8_t *)kernel_thread, (sizeof(thread_context_t) * KERNEL_APPLICATION_THREAD_INSTANCE));

    pCurThread = (thread_context_t *)kernel_member_unified_id_toContainerAddress(kernel_thread[KERNEL_SCHEDULE_THREAD_INSTANCE].head.id);
    timer_init_for_thread(kernel_member_unified_id_threadToTimer(pCurThread->head.id));
    kernel_thread_list_transfer_toPend((linker_head_t *)&pCurThread->head);

    pCurThread = (thread_context_t *)kernel_member_unified_id_toContainerAddress(kernel_thread[KERNEL_IDLE_THREAD_INSTANCE].head.id);
    timer_init_for_thread(kernel_member_unified_id_threadToTimer(pCurThread->head.id));
    kernel_thread_list_transfer_toPend((linker_head_t *)&pCurThread->head);

    semaphore_context_t kernel_semaphore[KERNEL_APPLICATION_SEMAPHORE_INSTANCE] = {
        [KERNEL_SCHEDULE_SEMAPHORE_INSTANCE] =
            {
                .head =
                    {
                        .pName = g_kernel_thread_resource.schedule_id.pName,
                        .linker = LINKER_NULL,
                    },
                .remains = 0u,
                .limits = OS_SEM_BINARY,
            },
    };

    semaphore_context_t *pCurSemaphore = (semaphore_context_t *)kernel_member_id_toContainerStartAddress(KERNEL_MEMBER_SEMAPHORE);
    g_kernel_thread_resource.sem_id.val = kernel_member_containerAddress_toUnifiedid((u32_t)pCurSemaphore);
    kernel_semaphore[KERNEL_SCHEDULE_SEMAPHORE_INSTANCE].head.id = g_kernel_thread_resource.sem_id.val;
    os_memcpy((u8_t *)pCurSemaphore, (u8_t *)kernel_semaphore, (sizeof(semaphore_context_t) * KERNEL_APPLICATION_SEMAPHORE_INSTANCE));

    pCurSemaphore =
        (semaphore_context_t *)kernel_member_unified_id_toContainerAddress(kernel_semaphore[KERNEL_SCHEDULE_SEMAPHORE_INSTANCE].head.id);
    kernel_semaphore_list_transfer_toInit((linker_head_t *)&pCurSemaphore->head);
}

#ifdef __cplusplus
}
#endif
