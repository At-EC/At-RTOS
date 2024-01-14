/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef _KERNAL_THREAD_H_
#define _KERNAL_THREAD_H_

#include "arch.h"
#include "type.h"

#ifdef __cplusplus
extern "C" {
#endif

void  _impl_kernal_thread_message_notification(void);
u32_t _impl_kernal_thread_message_arrived(void);
void  _impl_kernal_thread_init(void);

#ifdef __cplusplus
}
#endif

#endif /* _KERNAL_THREAD_H_ */

