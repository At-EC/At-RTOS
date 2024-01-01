/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef _KERNAL_H_
#define _KERNAL_H_

#include "member_struct.h"
#include "arch/arch.h"
#include "svc_call.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Define this to support Floating Point Unit available in Cortex-M4/M7
 */
#if (__FPU_PRESENT)
    #define FPU_ENABLED
#else
    #undef FPU_ENABLED
#endif

#define WAIT_FOREVER                            (0xFFFFFFFFu)

#define KERNAL_THREAD_STACK_SIZE                (1024u)
#define KERNAL_THREAD_NAME_STRING               "kernal"

#define STACT_UNUSED_DATA                       (0xDEu)
#define STACT_UNUSED_FRAME_MARK                 (0xDEDEDEDEu)

#define STACK_ADDRESS_UP(address)               (u32_t)(ROUND_UP((address), STACK_ALIGN))
#define STACK_ADDRESS_DOWN(address)             (u32_t)(ROUND_DOWN((address), STACK_ALIGN))

#define ENTER_CRITICAL_SECTION()                ARCH_ENTER_CRITICAL_SECTION()
#define EXIT_CRITICAL_SECTION()                 ARCH_EXIT_CRITICAL_SECTION()

#define KERNAL_THREAD_MEMORY_SIZE               (sizeof(thread_context_t) * THREAD_INSTANCE_SUPPORTED_NUMBER)
#define KERNAL_TIMER_INTERNAL_MEMORY_SIZE       (sizeof(timer_context_t) * THREAD_INSTANCE_SUPPORTED_NUMBER)
#define KERNAL_TIMER_MEMORY_SIZE                (sizeof(timer_context_t) * TIMER_INSTANCE_SUPPORTED_NUMBER)
#define KERNAL_SEMAPHORE_MEMORY_SIZE            (sizeof(semaphore_context_t) * SEMAPHORE_INSTANCE_SUPPORTED_NUMBER)
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
u8_t*   _impl_kernal_member_unified_id_toContainerAddress(u32_t unified_id);
u32_t   _impl_kernal_member_containerAddress_toUnifiedid(u32_t container_address);
u32_t   _impl_kernal_member_id_toUnifiedIdStart(u8_t member_id);
u8_t*   _impl_kernal_member_id_toContainerStartAddress(u32_t member_id);
u8_t*   _impl_kernal_member_id_toContainerEndAddress(u32_t member_id);
b_t     _impl_kernal_member_unified_id_isInvalid(u32_t member_id, u32_t unified_id);
u8_t    _impl_kernal_member_unified_id_toId(u32_t unified_id);
u32_t   _impl_kernal_member_unified_id_threadToTimer(u32_t unified_id);
u32_t   _impl_kernal_member_unified_id_timerToThread(u32_t unified_id);
u32_t   _impl_kernal_member_id_unifiedConvert(u8_t member_id, u32_t unified_id);
void    _impl_kernal_thread_list_transfer_toEntry(linker_head_t *pCurHead);
void    _impl_kernal_thread_list_transfer_toExit(linker_head_t *pCurHead);
u32p_t  _impl_kernal_thread_exit_trigger(os_id_t id, os_id_t hold, list_t *pToList, u32_t timeout_us, void (*pCallback)(os_id_t));
u32p_t  _impl_kernal_thread_entry_trigger(os_id_t id, os_id_t release, u32_t result, void (*pCallback)(os_id_t));
void    _impl_kernal_thread_list_transfer_toPend(linker_head_t *pCurHead);
os_id_t _impl_kernal_thread_runIdGet(void);
list_t* _impl_kernal_list_pendingHeadGet(void);
u32_t   _impl_kernal_stack_frame_init(void (*pEntryFunction)(void), u32_t *pAddress, u32_t size);
b_t     _impl_kernal_isInThreadMode(void);
u32_t   _impl_kernal_thread_schedule_request(void);
b_t     _impl_kernal_rtos_isRun(void);
u32p_t  _impl_kernal_at_rtos_run(void);
void    _impl_kernal_message_notification(void);
b_t     _impl_kernal_os_id_is_invalid(struct os_id id);
void    _impl_kernal_scheduler_inPendSV_c(u32_t **ppCurPsp, u32_t **ppNextPSP);
void    _impl_kernal_privilege_call_inSVC_c(u32_t *svc_args);
u32_t   _impl_kernal_privilege_invoke(const void* pCallFun, arguments_t* pArgs);
void*   _impl_kernal_thread_runContextGet(void);

static __INLINE u32p_t kernal_atos_run(void)
{
    return _impl_kernal_at_rtos_run();
}

#ifdef __cplusplus
}
#endif

#endif /* _KERNAL_H_ */

