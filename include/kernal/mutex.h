/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 **/

#ifndef _MUXTEX_H_
#define _MUXTEX_H_

#include "kstruct.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The implement function lists for rtos kernal internal use.
 */
u32_t _impl_mutex_os_id_to_number(os_id_t id);
os_id_t _impl_mutex_init(const char_t *pName);
u32p_t _impl_mutex_lock(os_id_t id);
u32p_t _impl_mutex_unlock(os_id_t id);

#ifdef __cplusplus
}
#endif

#endif /* _MUXTEX_H_ */
