/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef _COMPILER_H_
#define _COMPILER_H_

#include "configuration.h"

#if !defined ARCH_NATIVE_GCC
    #include "../arch/arch32/arm/cmsis/include/cmsis_compiler.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* TODO */

#ifdef __cplusplus
}
#endif

#endif /* _COMPILER_H_ */
