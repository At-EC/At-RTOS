﻿/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
**/

#ifndef _BUILD_VERSION_H_
#define _BUILD_VERSION_H_

#ifdef __cplusplus
extern "C" {
#endif

#define ATOS_BUILD_TIME           "2024-10-19,15:00"
#define ATOS_COMMIT_HEAD_ID       "a60a1a349cce093dc6558368f70570484e2e3e4e"
#define ATOS_VERSION_MAJOR_NUMBER (1u)
#define ATOS_VERSION_MINOR_NUMBER (7u)
#define ATOS_VERSION_PATCH_NUMBER (5u)

#define ATOS_VERSION_MAJOR_NUMBER_MASK (0x03FFu)
#define ATOS_VERSION_MAJOR_NUMBER_POS  (22u)

#define ATOS_VERSION_MINOR_NUMBER_MASK (0x03FFu)
#define ATOS_VERSION_MINOR_NUMBER_POS  (10u)

#define ATOS_VERSION_PATCH_NUMBER_MASK (0x0FFFu)
#define ATOS_VERSION_PATCH_NUMBER_POS  (0u)

#ifdef __cplusplus
}
#endif

#endif
