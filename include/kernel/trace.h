/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 **/
#ifndef _TRACE_H_
#define _TRACE_H_

#include "type_def.h"
#include "linker.h"
#include "kstruct.h"

typedef void (*pTrace_postcodeFunc_t)(u32_t, u32_t);
typedef void (*pTrace_threadFunc_t)(const thread_context_t *pThread);
typedef void (*pTrace_analyzeFunc_t)(const struct call_analyze analyze);

u32_t _impl_trace_firmware_version_get(void);
void _impl_trace_postcode_callback_register(const pTrace_postcodeFunc_t fn);
b_t _impl_trace_postcode_failed_get(const pTrace_postcodeFunc_t fn);
void _impl_trace_thread(const pTrace_threadFunc_t fn);
void _impl_trace_analyze(const pTrace_analyzeFunc_t fn);

#endif /* _TRACE_H_ */
