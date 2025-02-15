/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 **/
#ifndef _K_MALLOC_H_
#define _K_MALLOC_H_

#include "type_def.h"
#include "k_linker.h"
#include "k_config.h"

#ifndef K_MALLOC_HEAP_SIZE_CONFIG
#define K_MALLOC_HEAP_SIZE_CONFIG (MALLOC_HEAP_SIZE_SUPPORTED)
#endif

void *k_malloc(unsigned int size);
void k_free(void *p_addr);
_u32_t k_free_size(void);
_b_t k_allocated(void *p_addr);

#endif
