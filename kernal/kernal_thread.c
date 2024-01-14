/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */
#include "at_rtos.h"
#include "idle.h"
#include "unique.h"
#include "kernal.h"
#include "kernal_thread.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Local defined the kernal thread stack */
ATOS_STACK_DEFINE(g_kernal_schedule_stack, KERNAL_SCHEDULE_THREAD_STACK_SIZE);
ATOS_STACK_DEFINE(g_kernal_idle_stack, KERNAL_IDLE_THREAD_STACK_SIZE);

#define _PC_CMPT_FAILED       PC_FAILED(PC_CMPT_KERNAL)

/**
 * Global At_RTOS application interface init.
 */
const at_rtos_api_t AtOS =
{
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

    .semaphore_init = semaphore_init,
    .semaphore_take = semaphore_take,
    .semaphore_give = semaphore_give,
    .semaphore_flush = semaphore_flush,

    .mutex_init = mutex_init,
    .mutex_lock = mutex_lock,
    .mutex_unlock = mutex_unlock,

    .event_init = event_init,
    .event_set = event_set,
    .event_wait = event_wait,

    .queue_init = queue_init,
    .queue_send = queue_send,
    .queue_receive = queue_receive,

    .os_id_is_invalid = os_id_is_invalid,
    .kernal_atos_run = kernal_atos_run,
};

/**
 * Data structure for location kernal thread.
 */
typedef struct
{
    /* kernal schedule thread id */
    os_thread_id_t schedule_id;

    /* kernal idle thread id */
    os_thread_id_t idle_id;

    /* kernal schedule semaphore id */
    os_semaphore_id_t sem_id;
}_kernal_thread_resource_t;

/**
 * Local timer resource
 */
static _kernal_thread_resource_t g_kernal_thread_resource = {
    .schedule_id =
    {
        .pName = "kernal",
        .val = 0u,
        .number = KERNAL_SCHEDULE_THREAD_INSTANCE,
    },
    .idle_id =
    {
        .pName = "idle",
        .val = sizeof(thread_context_t),
        .number = KERNAL_IDLE_THREAD_INSTANCE,
    },
    .sem_id =
    {
        .pName = "kernal",
        .number = KERNAL_SCHEDULE_SEMAPHORE_INSTANCE,
    }
};

/**
 * @brief To issue a kernal message notification.
 */
void _impl_kernal_thread_message_notification(void)
{
    u32p_t postcode = semaphore_give(g_kernal_thread_resource.sem_id);
    if (PC_IER(postcode))
    {
        /* TODO */
    }
}

/**
 * @brief To check if the kernal message arrived.
 */
u32_t _impl_kernal_thread_message_arrived(void)
{
    return semaphore_take(g_kernal_thread_resource.sem_id, OS_TIME_FOREVER_VAL);
}

/**
 * @brief The AtOS kernal internal use thread and semaphore init.
 */
void _impl_kernal_thread_init(void)
{
    ENTER_CRITICAL_SECTION();

    thread_context_t kernal_thread[KERNAL_APPLICATION_THREAD_INSTANCE] =
    {
        [KERNAL_SCHEDULE_THREAD_INSTANCE] =
        {
            .head = {
                .id = g_kernal_thread_resource.schedule_id.val,
                .pName = g_kernal_thread_resource.schedule_id.pName,
                .linker = LINKER_NULL,
            },
            .priority = {
                .level = OS_PRIORITY_KERNAL_THREAD_SCHEDULE_LEVEL,
            },
            .pEntryFunc = _impl_kernal_atos_schedule_thread,
            .pStackAddr = g_kernal_schedule_stack,
            .stackSize = KERNAL_SCHEDULE_THREAD_STACK_SIZE,
            .PSPStartAddr = (u32_t)_impl_kernal_stack_frame_init(_impl_kernal_atos_schedule_thread, g_kernal_schedule_stack, KERNAL_SCHEDULE_THREAD_STACK_SIZE),

        },

        [KERNAL_IDLE_THREAD_INSTANCE] =
        {
            .head = {
                .id = g_kernal_thread_resource.idle_id.val,
                .pName = g_kernal_thread_resource.idle_id.pName,
                .linker = LINKER_NULL,
            },
            .priority = {
                .level = OS_PRIORITY_KERNAL_THREAD_IDLE_LEVEL,
            },
            .pEntryFunc = _impl_kernal_atos_idle_thread,
            .pStackAddr = g_kernal_idle_stack,
            .stackSize = KERNAL_IDLE_THREAD_STACK_SIZE,
            .PSPStartAddr = (u32_t)_impl_kernal_stack_frame_init(_impl_kernal_atos_idle_thread, g_kernal_idle_stack, KERNAL_IDLE_THREAD_STACK_SIZE),
        },
    };

    thread_context_t *pCurThread = (thread_context_t *)_impl_kernal_member_id_toContainerStartAddress(KERNAL_MEMBER_THREAD);
    _memcpy((u8_t*)pCurThread, (u8_t*)kernal_thread, (sizeof(thread_context_t)*KERNAL_APPLICATION_THREAD_INSTANCE));

    pCurThread = (thread_context_t*)_impl_kernal_member_unified_id_toContainerAddress(kernal_thread[KERNAL_SCHEDULE_THREAD_INSTANCE].head.id);
    _impl_thread_timer_init(_impl_kernal_member_unified_id_threadToTimer(pCurThread->head.id));
    _impl_kernal_thread_list_transfer_toPend((linker_head_t*)&pCurThread->head);

    pCurThread = (thread_context_t*)_impl_kernal_member_unified_id_toContainerAddress(kernal_thread[KERNAL_IDLE_THREAD_INSTANCE].head.id);
    _impl_thread_timer_init(_impl_kernal_member_unified_id_threadToTimer(pCurThread->head.id));
    _impl_kernal_thread_list_transfer_toPend((linker_head_t*)&pCurThread->head);

    semaphore_context_t kernal_semaphore[KERNAL_APPLICATION_SEMAPHORE_INSTANCE] =
    {
        [KERNAL_SCHEDULE_SEMAPHORE_INSTANCE] =
        {
            .head = {
                .pName = g_kernal_thread_resource.schedule_id.pName,
                .linker = LINKER_NULL,
            },
            .availableCount = 0u,
            .limitationCount = OS_SEMPHORE_TICKET_BINARY,
        },
    };

    semaphore_context_t *pCurSemaphore = (semaphore_context_t *)_impl_kernal_member_id_toContainerStartAddress(KERNAL_MEMBER_SEMAPHORE);
    g_kernal_thread_resource.sem_id.val = _impl_kernal_member_containerAddress_toUnifiedid((u32_t)pCurSemaphore);
    kernal_semaphore[KERNAL_SCHEDULE_SEMAPHORE_INSTANCE].head.id = g_kernal_thread_resource.sem_id.val;
    _memcpy((u8_t*)pCurSemaphore, (u8_t*)kernal_semaphore, (sizeof(semaphore_context_t)*KERNAL_APPLICATION_SEMAPHORE_INSTANCE));

    pCurSemaphore = (semaphore_context_t*)_impl_kernal_member_unified_id_toContainerAddress(kernal_semaphore[KERNAL_SCHEDULE_SEMAPHORE_INSTANCE].head.id);
    _impl_kernal_semaphore_list_transfer_toLock((linker_head_t*)&pCurSemaphore->head);
}

#ifdef __cplusplus
}
#endif
