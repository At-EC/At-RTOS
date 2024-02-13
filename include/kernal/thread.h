/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef _THREAD_H_
#define _THREAD_H_

#include "kstruct.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The implement function lists for rtos kernal internal use.
 */
u32_t   _impl_thread_os_id_to_number(os_id_t id);
os_id_t _impl_thread_init(void (*pThread_entryFunc_t)(void), u32_t *pAddress, u32_t size, u16_t priority, const char_t *pName);
u32p_t  _impl_thread_sleep(u32_t ms);
u32p_t  _impl_thread_resume(os_id_t id);
u32p_t  _impl_thread_suspend(os_id_t id);
u32p_t  _impl_thread_yield(void);
u32p_t  _impl_thread_delete(os_id_t id);

#ifdef __cplusplus
}
#endif

#endif /* _THREAD_H_ */
