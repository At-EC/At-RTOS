/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 **/

#include "at_rtos.h"
#include "ktype.h"
#include "kernel.h"
#include "kthread.h"
#include "postcode.h"

#ifdef __cplusplus
extern "C" {
#endif

#define _PC_CMPT_FAILED PC_FAILED(PC_CMPT_KERNEL)

/* Local defined the kernel thread stack */
static u32_t _kernel_schedule[((u32_t)(KERNEL_SCHEDULE_THREAD_STACK_SIZE) / sizeof(u32_t))] = {0u};
static u32_t _kernel_idle[((u32_t)(KERNEL_IDLE_THREAD_STACK_SIZE) / sizeof(u32_t))] = {0u};

/**
 * Global At_RTOS application interface init.
 */
#if (OS_INTERFACE_EXTERN_USE_ENABLE)
const at_rtos_api_t AtOS = {
    .thread_init = thread_init,
    .thread_sleep = thread_sleep,
    .thread_resume = thread_resume,
    .thread_suspend = thread_suspend,
    .thread_yield = thread_yield,
    .thread_delete = thread_delete,

    .timer_init = timer_init,
    .timer_start = timer_start,
    .timer_stop = timer_stop,
    .timer_isBusy = timer_isBusy,
    .timer_system_total_ms = timer_system_total_ms,

    .sem_init = sem_init,
    .sem_take = sem_take,
    .sem_give = sem_give,
    .sem_flush = sem_flush,

    .mutex_init = mutex_init,
    .mutex_lock = mutex_lock,
    .mutex_unlock = mutex_unlock,

    .evt_init = evt_init,
    .evt_set = evt_set,
    .evt_wait = evt_wait,

    .msgq_init = msgq_init,
    .msgq_put = msgq_put,
    .msgq_get = msgq_get,

    .pool_init = pool_init,
    .pool_take = pool_take,
    .pool_release = pool_release,

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
} _kernel_thread_resource_t;

/**
 * Local timer resource
 */
static _kernel_thread_resource_t g_kernel_thread_resource = {
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
void _impl_kernel_thread_message_notification(void)
{
    u32p_t postcode = sem_give(g_kernel_thread_resource.sem_id);
    if (PC_IER(postcode)) {
        /* TODO */
    }
}

/**
 * @brief To check if the kernel message arrived.
 */
u32_t _impl_kernel_thread_message_arrived(void)
{
    return sem_take(g_kernel_thread_resource.sem_id, OS_TIME_FOREVER_VAL);
}

/**
 * @brief The AtOS kernel internal use thread and semaphore init.
 */
void _impl_kernel_thread_init(void)
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
                        .level = OS_PRIORITY_KERNEL_THREAD_SCHEDULE_LEVEL,
                    },
                .pEntryFunc = _impl_kernel_thread_schedule,
                .pStackAddr = (u32_t *)&_kernel_schedule[0],
                .stackSize = KERNEL_SCHEDULE_THREAD_STACK_SIZE,
                .PSPStartAddr = (u32_t)_impl_kernel_stack_frame_init(_impl_kernel_thread_schedule, (u32_t *)&_kernel_schedule[0],
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
                        .level = OS_PRIORITY_KERNEL_THREAD_IDLE_LEVEL,
                    },
                .pEntryFunc = _impl_kernel_thread_idle,
                .pStackAddr = (u32_t *)&_kernel_idle[0],
                .stackSize = KERNEL_IDLE_THREAD_STACK_SIZE,
                .PSPStartAddr = (u32_t)_impl_kernel_stack_frame_init(_impl_kernel_thread_idle, (u32_t *)&_kernel_idle[0],
                                                                     KERNEL_IDLE_THREAD_STACK_SIZE),
            },
    };

    thread_context_t *pCurThread = (thread_context_t *)_impl_kernel_member_id_toContainerStartAddress(KERNEL_MEMBER_THREAD);
    _memcpy((u8_t *)pCurThread, (u8_t *)kernel_thread, (sizeof(thread_context_t) * KERNEL_APPLICATION_THREAD_INSTANCE));

    pCurThread =
        (thread_context_t *)_impl_kernel_member_unified_id_toContainerAddress(kernel_thread[KERNEL_SCHEDULE_THREAD_INSTANCE].head.id);
    _impl_thread_timer_init(_impl_kernel_member_unified_id_threadToTimer(pCurThread->head.id));
    _impl_kernel_thread_list_transfer_toPend((linker_head_t *)&pCurThread->head);

    pCurThread = (thread_context_t *)_impl_kernel_member_unified_id_toContainerAddress(kernel_thread[KERNEL_IDLE_THREAD_INSTANCE].head.id);
    _impl_thread_timer_init(_impl_kernel_member_unified_id_threadToTimer(pCurThread->head.id));
    _impl_kernel_thread_list_transfer_toPend((linker_head_t *)&pCurThread->head);

    semaphore_context_t kernel_semaphore[KERNEL_APPLICATION_SEMAPHORE_INSTANCE] = {
        [KERNEL_SCHEDULE_SEMAPHORE_INSTANCE] =
            {
                .head =
                    {
                        .pName = g_kernel_thread_resource.schedule_id.pName,
                        .linker = LINKER_NULL,
                    },
                .initialCount = 0u,
                .limitCount = OS_SEMPHORE_TICKET_BINARY,
                .isPermit = FALSE, /* reduce useless thread calling */
            },
    };

    semaphore_context_t *pCurSemaphore = (semaphore_context_t *)_impl_kernel_member_id_toContainerStartAddress(KERNEL_MEMBER_SEMAPHORE);
    g_kernel_thread_resource.sem_id.val = _impl_kernel_member_containerAddress_toUnifiedid((u32_t)pCurSemaphore);
    kernel_semaphore[KERNEL_SCHEDULE_SEMAPHORE_INSTANCE].head.id = g_kernel_thread_resource.sem_id.val;
    _memcpy((u8_t *)pCurSemaphore, (u8_t *)kernel_semaphore, (sizeof(semaphore_context_t) * KERNEL_APPLICATION_SEMAPHORE_INSTANCE));

    pCurSemaphore = (semaphore_context_t *)_impl_kernel_member_unified_id_toContainerAddress(
        kernel_semaphore[KERNEL_SCHEDULE_SEMAPHORE_INSTANCE].head.id);
    _impl_kernel_semaphore_list_transfer_toLock((linker_head_t *)&pCurSemaphore->head);
}

#ifdef __cplusplus
}
#endif
