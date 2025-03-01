/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 **/
#ifndef _SCHED_KERNEL_H_
#define _SCHED_KERNEL_H_

#include "./arch/k_arch.h"
#include "./port/k_port.h"
#include "./clock/k_clock_tick.h"
#include "k_struct.h"
#include "k_type.h"
#include "k_malloc.h"
#include "static_init.h"

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

_u32_t impl_kernel_irq_disable(void);
void impl_kernel_irq_enable(_u32_t val);

#define ENTER_CRITICAL_SECTION() _u32_t __val = impl_kernel_irq_disable()
#define EXIT_CRITICAL_SECTION()  impl_kernel_irq_enable(__val)
#define CRITICAL_SECTION()                                                                                                                 \
    for (struct _foreach_item __item = {.i = 0, .u32_val = impl_kernel_irq_disable()}; !__item.i;                                          \
         impl_kernel_irq_enable(__item.u32_val), __item.i++)

thread_context_t *kernel_thread_runContextGet(void);
list_t *kernel_member_list_get(_u8_t member_id, _u8_t list_id);
void kernel_thread_list_transfer_toEntry(linker_head_t *pCurHead);
_i32p_t schedule_exit_trigger(struct schedule_task *pTask, void *pHoldCtx, void *pHoldData, list_t *pToList, _u32_t timeout_ms,
                              _b_t immediately);
_i32p_t schedule_entry_trigger(struct schedule_task *pTask, pTask_callbackFunc_t callback, _u32_t result);
void schedule_callback_fromTimeOut(void *pNode);
void schedule_setPend(struct schedule_task *pTask);
list_t *schedule_waitList(void);
_b_t schedule_hasTwoPendingItem(void);
_i32p_t kernel_schedule_result_take(void);
_u32_t kernel_stack_frame_init(void (*pEntryFn)(void *), _u32_t *pAddress, _u32_t size, void *p_arg);
_b_t kernel_isInThreadMode(void);
_i32p_t kernel_thread_schedule_request(void);
void kernel_message_notification(void);
void kernel_scheduler_inPendSV_c(_u32_t **ppCurPsp, _u32_t **ppNextPSP);
void kernel_privilege_call_inSVC_c(_u32_t *svc_args);
_i32p_t kernel_privilege_invoke(const void *pCallFun, arguments_t *pArgs);
void kernel_schedule_thread(void *p_arg);
void kernel_idle_thread(void *p_arg);
void kthread_message_notification(void);
_i32p_t kthread_message_arrived(void);
void kthread_message_idle_loop_fn(void);

#endif /* _SCHED_KERNEL_H_ */
