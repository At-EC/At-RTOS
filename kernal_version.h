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

#define ATOS_BUILD_TIME                             "2024-02-13,16:54"
#define ATOS_COMMIT_HEAD_ID                         "e88295a8b83955e6af6a399b637fd58560946457"
#define ATOS_VERSION_MAJOR_NUMBER                   (0u)
#define ATOS_VERSION_MINOR_NUMBER                   (2u)
#define ATOS_VERSION_PATCH_NUMBER                   (1u)

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
