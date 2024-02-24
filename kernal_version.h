/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef _KERNAL_VERSION_H_
#define _KERNAL_VERSION_H_

#ifdef __cplusplus
extern "C" {
#endif

#define ATOS_BUILD_TIME                             "2024-02-24,16:23"
#define ATOS_COMMIT_HEAD_ID                         "4cc83d444fe68e5a8a58ff05e6900b72e0691cc0"
#define ATOS_VERSION_MAJOR_NUMBER                   (1u)
#define ATOS_VERSION_MINOR_NUMBER                   (1u)
#define ATOS_VERSION_PATCH_NUMBER                   (8u)

#define ATOS_VERSION_MAJOR_NUMBER_MASK              (0x03FFu)
#define ATOS_VERSION_MAJOR_NUMBER_POS               (22u)

#define ATOS_VERSION_MINOR_NUMBER_MASK              (0x03FFu)
#define ATOS_VERSION_MINOR_NUMBER_POS               (10u)

#define ATOS_VERSION_PATCH_NUMBER_MASK              (0x0FFFu)
#define ATOS_VERSION_PATCH_NUMBER_POS               (0u)

#ifdef __cplusplus
}
#endif

#endif
