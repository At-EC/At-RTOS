/**
* Copyright (c) Riven Zheng (zhengheiot@gmail.com).
*
* This source code is licensed under the MIT license found in the
* LICENSE file in the root directory of this source tree.
*/

#ifndef _ATOS_VERSION_H_
#define _ATOS_VERSION_H_

#ifdef __cplusplus
extern "C" {
#endif

#define ATOS_BUILD_TIME                             "2024-01-21,21:44"

#define ATOS_HEAD_COMMIT_ID                         "255ac3c16ac993f95ea978c7efb4fefea19e8c58"

#define ATOS_VERSION_STRING                         "0.0.40.0"
#define ATOS_VERSION_PRODUCTION_RELEASE_NUMBER      (0u)
#define ATOS_VERSION_OFFICIAL_RELEASE_NUMBER        (0u)
#define ATOS_VERSION_CHANGES_NUMBER                 (40u)
#define ATOS_VERSION_CATEGORIES_NUMBER              (0u)

#define ATOS_VERSION_CATEGORIES_MASK                (0x0Fu)
#define ATOS_VERSION_CATEGORIES_POS                 (0u)

#define ATOS_VERSION_CHANGES_MASK                   (0x3FFu)
#define ATOS_VERSION_CHANGES_POS                    (4u)

#define ATOS_VERSION_OFFICIAL_RELEASE_MASK          (0x3FFu)
#define ATOS_VERSION_OFFICIAL_RELEASE_POS           (14u)

#define ATOS_VERSION_PRODUCTION_RELEASE_MASK        (0xFFu)
#define ATOS_VERSION_PRODUCTION_RELEASE_POS         (24u)

#ifdef __cplusplus
}
#endif

#endif
