/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef _KERNAL_H_
#define _KERNAL_H_

#include "kstruct.h"
#include "arch.h"
#include "unique.h"
#include "port.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    KERNAL_SCHEDULE_THREAD_INSTANCE = (0u),
    KERNAL_IDLE_THREAD_INSTANCE,
    KERNAL_APPLICATION_THREAD_INSTANCE,
};

enum {
    KERNAL_SCHEDULE_SEMAPHORE_INSTANCE = (0u),
    KERNAL_APPLICATION_SEMAPHORE_INSTANCE,
};

#ifndef KERNAL_THREAD_STACK_SIZE
    #define KERNAL_SCHEDULE_THREAD_STACK_SIZE   (1024u)
#else
    #define KERNAL_SCHEDULE_THREAD_STACK_SIZE   (KERNAL_THREAD_STACK_SIZE)
#endif


#ifndef IDLE_THREAD_STACK_SIZE
    #define KERNAL_IDLE_THREAD_STACK_SIZE       (512u)
#else
    #define KERNAL_IDLE_THREAD_STACK_SIZE       (IDLE_THREAD_STACK_SIZE)
#endif

#define STACT_UNUSED_DATA                       (0xDEu)
#define STACT_UNUSED_FRAME_MARK                 (0xDEDEDEDEu)

#define STACK_ADDRESS_UP(address)               (u32_t)(ROUND_UP((address), STACK_ALIGN))
#define STACK_ADDRESS_DOWN(address)             (u32_t)(ROUND_DOWN((address), STACK_ALIGN))

#define ENTER_CRITICAL_SECTION()                ARCH_ENTER_CRITICAL_SECTION()
#define EXIT_CRITICAL_SECTION()                 ARCH_EXIT_CRITICAL_SECTION()

#define KERNAL_THREAD_MEMORY_SIZE               (sizeof(thread_context_t) * (THREAD_INSTANCE_SUPPORTED_NUMBER + KERNAL_APPLICATION_THREAD_INSTANCE))
#define KERNAL_TIMER_INTERNAL_MEMORY_SIZE       (sizeof(timer_context_t) * (THREAD_INSTANCE_SUPPORTED_NUMBER + KERNAL_APPLICATION_THREAD_INSTANCE))
#define KERNAL_TIMER_MEMORY_SIZE                (sizeof(timer_context_t) * TIMER_INSTANCE_SUPPORTED_NUMBER)
#define KERNAL_SEMAPHORE_MEMORY_SIZE            (sizeof(semaphore_context_t) * (SEMAPHORE_INSTANCE_SUPPORTED_NUMBER + KERNAL_APPLICATION_SEMAPHORE_INSTANCE))
#define KERNAL_MUTEX_MEMORY_SIZE                (sizeof(mutex_context_t) * MUTEX_INSTANCE_SUPPORTED_NUMBER)
#define KERNAL_EVENT_MEMORY_SIZE                (sizeof(event_context_t) * EVENT_INSTANCE_SUPPORTED_NUMBER)
#define KERNAL_QUEUE_MEMORY_SIZE                (sizeof(queue_context_t) * QUEUE_INSTANCE_SUPPORTED_NUMBER)

#define KERNAL_MEMBER_MAP_1                     (KERNAL_THREAD_MEMORY_SIZE)
#define KERNAL_MEMBER_MAP_2                     (KERNAL_MEMBER_MAP_1 + KERNAL_TIMER_INTERNAL_MEMORY_SIZE)
#define KERNAL_MEMBER_MAP_3                     (KERNAL_MEMBER_MAP_2 + KERNAL_TIMER_MEMORY_SIZE)
#define KERNAL_MEMBER_MAP_4                     (KERNAL_MEMBER_MAP_3 + KERNAL_SEMAPHORE_MEMORY_SIZE)
#define KERNAL_MEMBER_MAP_5                     (KERNAL_MEMBER_MAP_4 + KERNAL_MUTEX_MEMORY_SIZE)
#define KERNAL_MEMBER_MAP_6                     (KERNAL_MEMBER_MAP_5 + KERNAL_EVENT_MEMORY_SIZE)
#define KERNAL_MEMBER_MAP_7                     (KERNAL_MEMBER_MAP_6 + KERNAL_QUEUE_MEMORY_SIZE)
#define KERNAL_MEMBER_MAP_NUMBER                (KERNAL_MEMBER_MAP_7)

list_t *_impl_kernal_member_list_get(u8_t member_id, u8_t list_id);
u32_t   _impl_kernal_member_id_unifiedConvert(u8_t member_id, u32_t unified_id);
void    _impl_kernal_thread_list_transfer_toEntry(linker_head_t *pCurHead);
u32p_t  _impl_kernal_thread_exit_trigger(os_id_t id, os_id_t hold, list_t *pToList, u32_t timeout_us, void (*pCallback)(os_id_t));
u32p_t  _impl_kernal_thread_entry_trigger(os_id_t id, os_id_t release, u32_t result, void (*pCallback)(os_id_t));
u32_t   _impl_kernal_schedule_entry_result_read_clean(action_schedule_t* pSchedule);
void    _impl_kernal_thread_list_transfer_toPend(linker_head_t *pCurHead);
list_t* _impl_kernal_list_pendingHeadGet(void);
u32_t   _impl_kernal_stack_frame_init(void (*pEntryFunction)(void), u32_t *pAddress, u32_t size);
b_t     _impl_kernal_isInThreadMode(void);
u32_t   _impl_kernal_thread_schedule_request(void);
void    _impl_kernal_message_notification(void);
void    _impl_kernal_scheduler_inPendSV_c(u32_t **ppCurPsp, u32_t **ppNextPSP);
void    _impl_kernal_privilege_call_inSVC_c(u32_t *svc_args);
u32_t   _impl_kernal_privilege_invoke(const void* pCallFun, arguments_t* pArgs);
void*   _impl_kernal_thread_runContextGet(void);
void    _impl_kernal_atos_schedule_thread(void);
void    _impl_kernal_atos_idle_thread(void);
void    _impl_kernal_semaphore_list_transfer_toLock(linker_head_t *pCurHead);

#ifdef __cplusplus
}
#endif

#endif /* _KERNAL_H_ */

