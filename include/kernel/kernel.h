/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 **/

#ifndef _KERNEL_H_
#define _KERNEL_H_

#include "kstruct.h"
#include "arch.h"
#include "ktype.h"
#include "port.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    KERNEL_SCHEDULE_THREAD_INSTANCE = (0u),
    KERNEL_IDLE_THREAD_INSTANCE,
    KERNEL_APPLICATION_THREAD_INSTANCE,
};

enum {
    KERNEL_SCHEDULE_SEMAPHORE_INSTANCE = (0u),
    KERNEL_APPLICATION_SEMAPHORE_INSTANCE,
};

#ifndef KERNEL_THREAD_STACK_SIZE
#define KERNEL_SCHEDULE_THREAD_STACK_SIZE (1024u)
#else
#define KERNEL_SCHEDULE_THREAD_STACK_SIZE (KERNEL_THREAD_STACK_SIZE)
#endif

#ifndef IDLE_THREAD_STACK_SIZE
#define KERNEL_IDLE_THREAD_STACK_SIZE (512u)
#else
#define KERNEL_IDLE_THREAD_STACK_SIZE (IDLE_THREAD_STACK_SIZE)
#endif

#define ENTER_CRITICAL_SECTION() ARCH_ENTER_CRITICAL_SECTION()
#define EXIT_CRITICAL_SECTION()  ARCH_EXIT_CRITICAL_SECTION()

#define KERNEL_THREAD_MEMORY_SIZE (sizeof(thread_context_t) * (THREAD_INSTANCE_SUPPORTED_NUMBER + KERNEL_APPLICATION_THREAD_INSTANCE))
#define KERNEL_TIMER_INTERNAL_MEMORY_SIZE                                                                                                  \
    (sizeof(timer_context_t) * (THREAD_INSTANCE_SUPPORTED_NUMBER + KERNEL_APPLICATION_THREAD_INSTANCE))
#define KERNEL_TIMER_MEMORY_SIZE (sizeof(timer_context_t) * TIMER_INSTANCE_SUPPORTED_NUMBER)
#define KERNEL_SEMAPHORE_MEMORY_SIZE                                                                                                       \
    (sizeof(semaphore_context_t) * (SEMAPHORE_INSTANCE_SUPPORTED_NUMBER + KERNEL_APPLICATION_SEMAPHORE_INSTANCE))
#define KERNEL_MUTEX_MEMORY_SIZE     (sizeof(mutex_context_t) * MUTEX_INSTANCE_SUPPORTED_NUMBER)
#define KERNEL_EVENT_MEMORY_SIZE     (sizeof(event_context_t) * EVENT_INSTANCE_SUPPORTED_NUMBER)
#define KERNEL_QUEUE_MEMORY_SIZE     (sizeof(queue_context_t) * QUEUE_INSTANCE_SUPPORTED_NUMBER)
#define KERNEL_POOL_MEMORY_SIZE      (sizeof(pool_context_t) * POOL_INSTANCE_SUPPORTED_NUMBER)
#define KERNEL_PUBLISH_MEMORY_SIZE   (sizeof(publish_context_t) * PUBLISH_INSTANCE_SUPPORTED_NUMBER)
#define KERNEL_SUBSCIRBE_MEMORY_SIZE (sizeof(subscribe_context_t) * SUBSCRIBE_INSTANCE_SUPPORTED_NUMBER)

#define KERNEL_MEMBER_MAP_1      (KERNEL_THREAD_MEMORY_SIZE)
#define KERNEL_MEMBER_MAP_2      (KERNEL_MEMBER_MAP_1 + KERNEL_TIMER_INTERNAL_MEMORY_SIZE)
#define KERNEL_MEMBER_MAP_3      (KERNEL_MEMBER_MAP_2 + KERNEL_TIMER_MEMORY_SIZE)
#define KERNEL_MEMBER_MAP_4      (KERNEL_MEMBER_MAP_3 + KERNEL_SEMAPHORE_MEMORY_SIZE)
#define KERNEL_MEMBER_MAP_5      (KERNEL_MEMBER_MAP_4 + KERNEL_MUTEX_MEMORY_SIZE)
#define KERNEL_MEMBER_MAP_6      (KERNEL_MEMBER_MAP_5 + KERNEL_EVENT_MEMORY_SIZE)
#define KERNEL_MEMBER_MAP_7      (KERNEL_MEMBER_MAP_6 + KERNEL_QUEUE_MEMORY_SIZE)
#define KERNEL_MEMBER_MAP_8      (KERNEL_MEMBER_MAP_7 + KERNEL_POOL_MEMORY_SIZE)
#define KERNEL_MEMBER_MAP_9      (KERNEL_MEMBER_MAP_8 + KERNEL_PUBLISH_MEMORY_SIZE)
#define KERNEL_MEMBER_MAP_10     (KERNEL_MEMBER_MAP_9 + KERNEL_SUBSCIRBE_MEMORY_SIZE)
#define KERNEL_MEMBER_MAP_NUMBER (KERNEL_MEMBER_MAP_10 + 1u)

thread_context_t *kernel_thread_runContextGet(void);
list_t *kernel_member_list_get(u8_t member_id, u8_t list_id);
void kernel_thread_list_transfer_toEntry(linker_head_t *pCurHead);
i32p_t kernel_thread_exit_trigger(os_id_t id, os_id_t hold, list_t *pToList, u32_t timeout_us, void (*pCallback)(os_id_t));
i32p_t kernel_thread_entry_trigger(os_id_t id, os_id_t release, u32_t result, void (*pCallback)(os_id_t));
u32_t kernel_schedule_entry_result_take(action_schedule_t *pSchedule);
void kernel_thread_list_transfer_toPend(linker_head_t *pCurHead);
list_t *kernel_list_pendingHeadGet(void);
u32_t kernel_stack_frame_init(void (*pEntryFunction)(void), u32_t *pAddress, u32_t size);
b_t kernel_isInThreadMode(void);
i32p_t kernel_thread_schedule_request(void);
void kernel_message_notification(void);
void kernel_scheduler_inPendSV_c(u32_t **ppCurPsp, u32_t **ppNextPSP);
u32_t kernel_schedule_time_get(void);
u32_t kernel_thread_use_percent_take(os_id_t id);
void kernel_privilege_call_inSVC_c(u32_t *svc_args);
i32p_t kernel_privilege_invoke(const void *pCallFun, arguments_t *pArgs);
void kernel_semaphore_list_transfer_toInit(linker_head_t *pCurHead);
void kernel_schedule_thread(void);
void kernel_idle_thread(void);
void kthread_init(void);
void kthread_message_notification(void);
i32p_t kthread_message_arrived(void);

#ifdef __cplusplus
}
#endif

#endif /* _KERNEL_H_ */
