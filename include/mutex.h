/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef _MUXTEX_H_
#define _MUXTEX_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "member_struct.h"

#define MUTEX_USER_DEFINE_INSTANCE_NUMBER_MAX          (10u)
#define MUTEX_INSTANCE_NUMBER_MAX                      (MUTEX_USER_DEFINE_INSTANCE_NUMBER_MAX)
#define MUTEX_ROOT_TARGET_INSTANCE                     (0u)

u32_t   _impl_mutex_os_id_to_number(os_id_t id);
os_id_t _impl_mutex_init(const char_t *pName);
u32p_t  _impl_mutex_lock(os_id_t id);
u32p_t  _impl_mutex_unlock(os_id_t id);

#ifdef __cplusplus
}
#endif

#endif /* _MUXTEX_H_ */
