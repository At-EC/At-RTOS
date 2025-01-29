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

#define ENTER_CRITICAL_SECTION() u32_t __val = port_irq_disable()
#define EXIT_CRITICAL_SECTION()  port_irq_enable(__val)
#define CRITICAL_SECTION()       for (u32_t ___val = port_irq_disable(), u8_t __i = 0; __i < 0; port_irq_enable(___val), __i++)

thread_context_t *kernel_thread_runContextGet(void);
list_t *kernel_member_list_get(u8_t member_id, u8_t list_id);
void kernel_thread_list_transfer_toEntry(linker_head_t *pCurHead);
i32p_t schedule_exit_trigger(struct schedule_task *pTask, void *pHoldCtx, void *pHoldData, list_t *pToList, u32_t timeout_ms,
                             b_t immediately);
i32p_t schedule_entry_trigger(struct schedule_task *pTask, pTask_callbackFunc_t callback, u32_t result);
void schedule_callback_fromTimeOut(void *pNode);
void schedule_setPend(struct schedule_task *pTask);
list_t *schedule_waitList(void);
b_t schedule_hasTwoPendingItem(void);
i32p_t kernel_schedule_result_take(void);
u32_t kernel_stack_frame_init(void (*pEntryFunction)(void), u32_t *pAddress, u32_t size);
b_t kernel_isInThreadMode(void);
i32p_t kernel_thread_schedule_request(void);
void kernel_message_notification(void);
void kernel_scheduler_inPendSV_c(u32_t **ppCurPsp, u32_t **ppNextPSP);
void kernel_privilege_call_inSVC_c(u32_t *svc_args);
i32p_t kernel_privilege_invoke(const void *pCallFun, arguments_t *pArgs);
void kernel_schedule_thread(void);
void kernel_idle_thread(void);
void kthread_message_notification(void);
i32p_t kthread_message_arrived(void);
void kthread_message_idle_loop_fn(void);

#endif /* _KERNEL_H_ */
