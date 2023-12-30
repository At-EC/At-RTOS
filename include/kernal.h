/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef _KERNAL_H_
#define _KERNAL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "member_struct.h"
#include "arch/arch.h"

#if (__FPU_PRESENT)
    #define FPU_ENABLED                /**< #define this to support Floating Point Unit available in Cortex-M4/M7                                         */
#else
    #undef FPU_ENABLED
#endif

#define WAIT_FOREVER                    		(0xFFFFFFFFu)

#define KERNAL_THREAD_STACK_SIZE        		(1024u)
#define KERNAL_THREAD_NAME_STRING       		"kernal"

#define STACT_UNUSED_DATA               		(0xDEu)
#define STACT_UNUSED_FRAME_MARK         		(0xDEDEDEDEu)

#define STACK_ADDRESS_UP(address)       		(u32_t)(ROUND_UP((address), STACK_ALIGN))
#define STACK_ADDRESS_DOWN(address)     		(u32_t)(ROUND_DOWN((address), STACK_ALIGN))

#define ENTER_CRITICAL_SECTION()        		ARCH_ENTER_CRITICAL_SECTION()
#define EXIT_CRITICAL_SECTION()         		ARCH_EXIT_CRITICAL_SECTION()

#define KERNAL_THREAD_MEMORY_SIZE           	(sizeof(thread_context_t) * THREAD_INSTANCE_SUPPORTED_NUMBER)
#define KERNAL_TIMER_INTERNAL_MEMORY_SIZE   	(sizeof(timer_context_t) * THREAD_INSTANCE_SUPPORTED_NUMBER)
#define KERNAL_TIMER_MEMORY_SIZE            	(sizeof(timer_context_t) * TIMER_INSTANCE_SUPPORTED_NUMBER)
#define KERNAL_SEMAPHORE_MEMORY_SIZE        	(sizeof(semaphore_context_t) * SEMAPHORE_INSTANCE_SUPPORTED_NUMBER)
#define KERNAL_MUTEX_MEMORY_SIZE            	(sizeof(mutex_context_t) * MUTEX_INSTANCE_SUPPORTED_NUMBER)
#define KERNAL_EVENT_MEMORY_SIZE            	(sizeof(event_context_t) * EVENT_INSTANCE_SUPPORTED_NUMBER)
#define KERNAL_QUEUE_MEMORY_SIZE            	(sizeof(queue_context_t) * QUEUE_INSTANCE_SUPPORTED_NUMBER)

#define KERNAL_MEMBER_MAP_1                 	(KERNAL_THREAD_MEMORY_SIZE)
#define KERNAL_MEMBER_MAP_2                 	(KERNAL_MEMBER_MAP_1 + KERNAL_TIMER_INTERNAL_MEMORY_SIZE)
#define KERNAL_MEMBER_MAP_3                 	(KERNAL_MEMBER_MAP_2 + KERNAL_TIMER_MEMORY_SIZE)
#define KERNAL_MEMBER_MAP_4                 	(KERNAL_MEMBER_MAP_3 + KERNAL_SEMAPHORE_MEMORY_SIZE)
#define KERNAL_MEMBER_MAP_5                 	(KERNAL_MEMBER_MAP_4 + KERNAL_MUTEX_MEMORY_SIZE)
#define KERNAL_MEMBER_MAP_6                 	(KERNAL_MEMBER_MAP_5 + KERNAL_EVENT_MEMORY_SIZE)
#define KERNAL_MEMBER_MAP_7                 	(KERNAL_MEMBER_MAP_6 + KERNAL_QUEUE_MEMORY_SIZE)
#define KERNAL_MEMBER_MAP_NUMBER            	(KERNAL_MEMBER_MAP_7)

typedef struct
{
    u32_t u32_val;
}arguments_t;

typedef u32_t (*pPrivilege_callFunc_t)(arguments_t *);

#define SVC_KERNAL_INVOKE_NUMBER			         2
__svc(SVC_KERNAL_INVOKE_NUMBER) u32_t kernal_svc_call(u32_t args_0, u32_t args_1, u32_t args_2, u32_t arg_3);

#pragma pack(1)

// S_ASSERT(sizeof(stack_snapshot_t) == 0x48u, "The size of struct stack_snapshot_t must be equal to 0x48u");

#pragma pack()


list_t *kernal_member_list_get(u8_t member_id, u8_t list_id);
u8_t*   kernal_member_unified_id_toContainerAddress(u32_t unified_id);
u32_t   kernal_member_containerAddress_toUnifiedid(u32_t container_address);
u32_t   kernal_member_id_toUnifiedIdStart(u8_t member_id);
u32_t   kernal_member_id_toUnifiedIdEnd(u8_t member_id);
u32_t   kernal_member_id_toUnifiedIdRange(u8_t member_id);
u8_t*   kernal_member_id_toContainerStartAddress(u32_t member_id);
u8_t*   kernal_member_id_toContainerEndAddress(u32_t member_id);
b_t     kernal_member_unified_id_isInvalid(u32_t member_id, u32_t unified_id);
u8_t    kernal_member_unified_id_toId(u32_t unified_id);
u32_t   kernal_member_unified_id_threadToTimer(u32_t unified_id);
u32_t   kernal_member_unified_id_timerToThread(u32_t unified_id);
u32_t   kernal_member_id_unifiedConvert(u8_t member_id, u32_t unified_id);

void   kernal_thread_list_transfer_toExit(linker_head_t *pCurHead);
void   kernal_thread_list_transfer_toEntry(linker_head_t *pCurHead);
u32p_t kernal_thread_exit_trigger(os_id_t id, os_id_t hold, list_t *pToList, u32_t timeout_us, void (*pCallback)(os_id_t));
u32p_t kernal_thread_entry_trigger(os_id_t id, os_id_t release, u32_t result, void (*pCallback)(os_id_t));
void   kernal_thread_list_transfer_toPend(linker_head_t *pCurHead);

os_id_t kernal_thread_runIdGet(void);
thread_context_t* kernal_thread_runContextGet(void);
list_t* kernal_list_pendingHeadGet(void);
u32_t   kernal_stack_frame_init(void (*pEntryFunction)(void), u32_t *pAddress, u32_t size);
b_t     kernal_isInPrivilegeMode(void);
b_t     kernal_isInThreadMode(void);
u32_t   kernal_thread_schedule_request(void);
void    kernal_setPendSV(void);
u32p_t  kernal_rtos_run(void);
b_t     kernal_rtos_isRun(void);
u32_t   kernal_start_privilege_routine(u32_t args_0, u32_t args_1, u32_t args_2, u32_t args_3);
u32_t   kernal_invoke(u8_t svc_number, u32_t args_0, u32_t args_1, u32_t args_2, u32_t args_3);
void    kernal_message_notification(void);

b_t    _impl_kernal_os_id_is_invalid(struct os_id id);

void   kernal_scheduler_inPendSV_c(u32_t **ppCurPsp, u32_t **ppNextPSP);
void   kernal_privilege_call_inSVC_c(u32_t *svc_args);
u32_t _impl_kernal_privilege_invoke(const void* pCallFun, arguments_t* pArgs);
static __INLINE u32_t kernal_privilege_invoke(const void* pCallFun, arguments_t* pArgs)
{
	return _impl_kernal_privilege_invoke(pCallFun, pArgs);
}

#ifdef __cplusplus
}
#endif

#endif /* _KERNAL_H_ */

