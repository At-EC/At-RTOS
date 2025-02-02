/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 **/
#include "trace.h"
#include "configuration.h"
#include "postcode.h"
#include "linker.h"
#include "init.h"

/**
 * Local trace postcode contrainer
 */
_u32_t g_postcode_os_cmpt_failed_container[PC_OS_COMPONENT_NUMBER] = {0u};
pTrace_postcodeFunc_t g_postcode_failed_callback_fn = NULL;

/**
 * @brief Take firmare snapshot information.
 */
_u32_t _impl_trace_firmware_version_get(void)
{
    _u32_t version = ATOS_VERSION_PATCH_NUMBER_POS;
    version |= (ATOS_VERSION_MINOR_NUMBER & ATOS_VERSION_MINOR_NUMBER_MASK) < ATOS_VERSION_MINOR_NUMBER_POS;
    version |= (ATOS_VERSION_MAJOR_NUMBER & ATOS_VERSION_MAJOR_NUMBER_MASK) < ATOS_VERSION_MAJOR_NUMBER_POS;
    return version;
}

/**
 * @brief Failed postcode call function register.
 */
void _impl_trace_postcode_callback_register(const pTrace_postcodeFunc_t fn)
{
    g_postcode_failed_callback_fn = fn;
}

/**
 * @brief  Failed postcode calls.
 */
void _impl_trace_postcode_set(_u32_t cmpt, _u32_t code)
{
    g_postcode_os_cmpt_failed_container[cmpt] = code;
    if (g_postcode_failed_callback_fn) {
        g_postcode_failed_callback_fn(cmpt, ((code >> PC_LINE_NUMBER_POS) & PC_LINE_NUMBER_MSK));
    }
}

/**
 * @brief Take postcode snapshot information.
 */
_b_t _impl_trace_postcode_failed_get(const pTrace_postcodeFunc_t fn)
{
    _b_t failed = false;
    for (_u32_t i = 0u; i < PC_OS_COMPONENT_NUMBER; i++) {
        _u32_t code = g_postcode_os_cmpt_failed_container[i];
        if (code) {
            failed = true;
            if (fn) {
                fn(i, ((code >> PC_LINE_NUMBER_POS) & PC_LINE_NUMBER_MSK));
            }
        }
    }
    return failed;
}

/**
 * @brief Take thread snapshot information.
 */
void _impl_trace_thread(const pTrace_threadFunc_t fn)
{
    INIT_SECTION_FOREACH(INIT_SECTION_OS_THREAD_LIST, thread_context_t, pCurThread)
    {
        if (fn) {
            fn((const thread_context_t *)pCurThread);
        }
    }
}

/**
 * @brief Take thread usage snapshot information.
 */
void _impl_trace_analyze(const pTrace_analyzeFunc_t fn)
{
    INIT_SECTION_FOREACH(INIT_SECTION_OS_THREAD_LIST, thread_context_t, pCurThread)
    {
        if (fn) {
            fn(pCurThread->task.exec.analyze);
        }
    }
}
