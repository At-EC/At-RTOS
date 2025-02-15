/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 **/
#include "sched_kernel.h"
#include "sched_timer.h"
#include "k_trace.h"
#include "k_malloc.h"
#include "postcode.h"

/**
 * Local unique postcode.
 */
#define PC_EOR PC_IER(PC_OS_CMPT_THREAD_3)

/**
 * @brief Check if the thread unique id if is's invalid.
 *
 * @param id The provided unique id.
 *
 * @return The true indicates the id is invalid, otherwise it valid.
 */
static _b_t _thread_context_isInvalid(thread_context_t *pCurThread)
{
    _u32_t start, end;
    INIT_SECTION_FIRST(INIT_SECTION_OS_THREAD_LIST, start);
    INIT_SECTION_LAST(INIT_SECTION_OS_THREAD_LIST, end);

    return ((_u32_t)pCurThread < start || (_u32_t)pCurThread >= end) ? true : false;
}

/**
 * @brief Check if the timer object if is's initialized.
 *
 * @param id The provided unique id.
 *
 * @return The true is initialized, otherwise is uninitialized.
 */
static _b_t _thread_context_isInit(thread_context_t *pCurThread)
{
    return ((pCurThread) ? (((pCurThread->head.cs) ? (true) : (false))) : false);
}

/**
 * @brief It's sub-routine running at privilege mode.
 *
 * @param pArgs The function argument packages.
 *
 * @return The result of privilege routine.
 */
static _u32_t _thread_init_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    pThread_entryFunc_t pEntryFun = (pThread_entryFunc_t)pArgs[0].ptr_val;
    _u32_t *pAddress = (_u32_t *)pArgs[1].u32_val;
    _u32_t size = (_u32_t)pArgs[2].u32_val;
    _i16_t priority = (_i16_t)pArgs[3].u16_val;
    void *p_arg = (void *)pArgs[4].pv_val;
    const _char_t *pName = (const _char_t *)pArgs[5].pch_val;

    INIT_SECTION_FOREACH(INIT_SECTION_OS_THREAD_LIST, thread_context_t, pCurThread)
    {
        if (_thread_context_isInvalid(pCurThread)) {
            break;
        }

        if (_thread_context_isInit(pCurThread)) {
            continue;
        }

        k_memset((_char_t *)pCurThread, 0x0u, sizeof(thread_context_t));
        pCurThread->head.cs = CS_INITED;
        pCurThread->head.pName = pName;

        pCurThread->pEntryFunc = pEntryFun;
        if (!pAddress) {
            pAddress = (_u32_t *)k_malloc(size);
            if (!pAddress) {
                EXIT_CRITICAL_SECTION();
                return 0u;
            }
        }
        pCurThread->pStackAddr = pAddress;
        pCurThread->stackSize = size;

        pCurThread->task.prior = priority;
        pCurThread->task.psp = (_u32_t)kernel_stack_frame_init(pEntryFun, pAddress, size, p_arg);
        timeout_init(&pCurThread->task.expire, schedule_callback_fromTimeOut);
        schedule_setPend(&pCurThread->task);

        EXIT_CRITICAL_SECTION();
        return (_u32_t)pCurThread;
    }

    EXIT_CRITICAL_SECTION();
    return 0u;
}

/**
 * @brief It's sub-routine running at privilege mode.
 *
 * @param pArgs The function argument packages.
 *
 * @return The result of privilege routine.
 */
static _u32_t _thread_stack_free_get_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    thread_context_t *pCurThread = (thread_context_t *)pArgs[0].u32_val;

    _u32_t free = port_stack_free_size_get(*pCurThread->pStackAddr);

    EXIT_CRITICAL_SECTION();

    return free;
}

/**
 * @brief It's sub-routine running at privilege mode.
 *
 * @param pArgs The function argument packages.
 *
 * @return The result of privilege routine.
 */
static _i32p_t _thread_user_data_register_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();
    thread_context_t *pCurThread = (thread_context_t *)pArgs[0].u32_val;
    void *pData = (void *)pArgs[1].pv_val;

    pCurThread->pUserData = pData;

    EXIT_CRITICAL_SECTION();
    return 0;
}

/**
 * @brief It's sub-routine running at privilege mode.
 *
 * @param pArgs The function argument packages.
 *
 * @return The result of privilege routine.
 */
static _u32_t _thread_user_data_get_privilege_routine(arguments_t *pArgs)
{
    thread_context_t *pCurThread = (thread_context_t *)pArgs[0].u32_val;

    return (_u32_t)pCurThread->pUserData;
}

/**
 * @brief It's sub-routine running at privilege mode.
 *
 * @param pArgs The function argument packages.
 *
 * @return The result of privilege routine.
 */
static _i32p_t _thread_resume_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();
    thread_context_t *pCurThread = (thread_context_t *)pArgs[0].u32_val;
    _i32p_t postcode = 0;

    if (pCurThread == kernel_thread_runContextGet()) {
        EXIT_CRITICAL_SECTION();
        return postcode;
    }
    postcode = schedule_entry_trigger(&pCurThread->task, NULL, 0u);

    EXIT_CRITICAL_SECTION();
    return postcode;
}

/**
 * @brief It's sub-routine running at privilege mode.
 *
 * @param pArgs The function argument packages.
 *
 * @return The result of privilege routine.
 */
static _i32p_t _thread_suspend_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();
    thread_context_t *pCurThread = (thread_context_t *)pArgs[0].u32_val;
    _i32p_t postcode = PC_EOR;

    if (!schedule_hasTwoPendingItem()) {
        EXIT_CRITICAL_SECTION();
        return postcode;
    }
    postcode = schedule_exit_trigger(&pCurThread->task, NULL, NULL, schedule_waitList(), OS_TIME_FOREVER_VAL, false);

    EXIT_CRITICAL_SECTION();
    return postcode;
}

/**
 * @brief It's sub-routine running at privilege mode.
 *
 * @param pArgs The function argument packages.
 *
 * @return The result of privilege routine.
 */
static _i32p_t _thread_yield_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();
    UNUSED_MSG(pArgs);
    thread_context_t *pCurThread = NULL;
    _i32p_t postcode = 0;

    pCurThread = (thread_context_t *)kernel_thread_runContextGet();
    if (!schedule_hasTwoPendingItem()) {
        EXIT_CRITICAL_SECTION();
        return postcode;
    }
    postcode = schedule_exit_trigger(&pCurThread->task, NULL, NULL, schedule_waitList(), OS_TIME_FOREVER_VAL, false);

    EXIT_CRITICAL_SECTION();
    return postcode;
}

/**
 * @brief It's sub-routine running at privilege mode.
 *
 * @param pArgs The function argument packages.
 *
 * @return The result of privilege routine.
 */
static _i32p_t _thread_delete_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();
    thread_context_t *pCurThread = (thread_context_t *)pArgs[0].u32_val;
    _i32p_t postcode = PC_EOR;

    if (!schedule_hasTwoPendingItem()) {
        EXIT_CRITICAL_SECTION();
        return postcode;
    }
    postcode = schedule_exit_trigger(&pCurThread->task, NULL, NULL, NULL, OS_TIME_FOREVER_VAL, true);
    EXIT_CRITICAL_SECTION();
    return postcode;
}

/**
 * @brief It's sub-routine running at privilege mode.
 *
 * @param pArgs The function argument packages.
 *
 * @return The result of privilege routine.
 */
static _i32p_t _thread_sleep_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();
    _u32_t timeout_ms = (_u32_t)pArgs[0].u32_val;
    thread_context_t *pCurThread = NULL;
    _i32p_t postcode = PC_EOR;

    pCurThread = kernel_thread_runContextGet();
    postcode = schedule_exit_trigger(&pCurThread->task, NULL, NULL, schedule_waitList(), timeout_ms, true);

    EXIT_CRITICAL_SECTION();
    return postcode;
}

/**
 * @brief Get the thread name based on provided unique id.
 *
 * @param ctx Thread unique id.
 *
 * @return The name of current thread.
 */
const _char_t *_impl_thread_name_get(_u32_t ctx)
{
    thread_context_t *pCtx = (thread_context_t *)ctx;
    if (_thread_context_isInvalid(pCtx)) {
        return NULL;
    }

    if (!_thread_context_isInit(pCtx)) {
        return NULL;
    }

    return (const _char_t *)((pCtx) ? (pCtx->head.pName) : (NULL));
}

void _impl_thread_static_init(thread_context_t *pCurThread, void *p_arg)
{
    ENTER_CRITICAL_SECTION();

    pCurThread->task.psp = (_u32_t)kernel_stack_frame_init(pCurThread->pEntryFunc, pCurThread->pStackAddr, pCurThread->stackSize, p_arg);
    timeout_init(&pCurThread->task.expire, schedule_callback_fromTimeOut);
    schedule_setPend(&pCurThread->task);

    EXIT_CRITICAL_SECTION();
}

/**
 * @brief Initialize a new thread.
 *
 * @param pEntryFun The thread entry function pointer.
 * @param pAddress The thread stack address.
 * @param size The thread stack size.
 * @param priority The thread priority.
 * @param pName The thread name.
 *
 * @return The value of thread unique id.
 */
_u32_t _impl_thread_init(pThread_entryFunc_t pEntryFun, _u32_t *pAddress, _u32_t size, _i16_t priority, void *pArg, const _char_t *pName)
{
    if (!pEntryFun) {
        return OS_INVALID_ID_VAL;
    }

    if ((size < STACK_SIZE_MINIMUM) || (size > STACK_SIZE_MAXIMUM)) {
        return OS_INVALID_ID_VAL;
    }

    if (priority > 0xFF) {
        return OS_INVALID_ID_VAL;
    }

    if ((priority == OS_PRIOTITY_LOWEST_LEVEL) || (priority == OS_PRIOTITY_HIGHEST_LEVEL)) {
        return OS_INVALID_ID_VAL;
    }

    arguments_t arguments[] = {
        [0] = {.ptr_val = (void *)pEntryFun}, [1] = {.u32_val = (_u32_t)pAddress}, [2] = {.u32_val = (_u32_t)size},
        [3] = {.u16_val = (_i16_t)priority},  [4] = {.pv_val = (void *)pArg},      [5] = {.pch_val = (const void *)pName},
    };

    return kernel_privilege_invoke((const void *)_thread_init_privilege_routine, arguments);
}

/**
 * @brief Get a thread stack free size.
 *
 * @param id The thread unique id.
 *
 * @return The result of thread free size.
 */
_u32_t _impl_thread_stack_free_size_get(_u32_t ctx)
{
    thread_context_t *pCtx = (thread_context_t *)ctx;
    if (_thread_context_isInvalid(pCtx)) {
        return 0;
    }

    if (!_thread_context_isInit(pCtx)) {
        return 0;
    }

    arguments_t arguments[] = {
        [0] = {.u32_val = (_u32_t)ctx},
    };

    return kernel_privilege_invoke((const void *)_thread_stack_free_get_privilege_routine, arguments);
}

/**
 * @brief Add a user thread data.
 *
 * @param id The thread unique id.
 *
 * @return The result of thread user data operation.
 */
_i32p_t _impl_thread_user_data_register(_u32_t ctx, void *pUserData)
{
    thread_context_t *pCtx = (thread_context_t *)ctx;
    if (_thread_context_isInvalid(pCtx)) {
        return PC_EOR;
    }

    if (!_thread_context_isInit(pCtx)) {
        return PC_EOR;
    }

    arguments_t arguments[] = {
        [0] = {.u32_val = (_u32_t)ctx},
        [1] = {.pv_val = (void *)pUserData},
    };

    return kernel_privilege_invoke((const void *)_thread_user_data_register_privilege_routine, arguments);
}

/**
 * @brief Get a user thread data.
 *
 * @param id The thread unique id.
 *
 * @return The result of thread user data operation.
 */
void *_impl_thread_user_data_get(_u32_t ctx)
{
    thread_context_t *pCtx = (thread_context_t *)ctx;
    if (_thread_context_isInvalid(pCtx)) {
        return NULL;
    }

    if (!_thread_context_isInit(pCtx)) {
        return NULL;
    }

    arguments_t arguments[] = {
        [0] = {.u32_val = (_u32_t)ctx},
    };

    return (void *)kernel_privilege_invoke((const void *)_thread_user_data_get_privilege_routine, arguments);
}

/**
 * @brief Resume a thread to run.
 *
 * @param id The thread unique id.
 *
 * @return The result of thread resume operation.
 */
_i32p_t _impl_thread_resume(_u32_t ctx)
{
    thread_context_t *pCtx = (thread_context_t *)ctx;
    if (_thread_context_isInvalid(pCtx)) {
        return PC_EOR;
    }

    if (!_thread_context_isInit(pCtx)) {
        return PC_EOR;
    }

    arguments_t arguments[] = {
        [0] = {.u32_val = (_u32_t)ctx},
    };

    return kernel_privilege_invoke((const void *)_thread_resume_privilege_routine, arguments);
}

/**
 * @brief Suspend a thread to permit another to run.
 *
 * @param id The thread unique id.
 *
 * @return The result of thread suspend operation.
 */
_i32p_t _impl_thread_suspend(_u32_t ctx)
{
    thread_context_t *pCtx = (thread_context_t *)ctx;
    if (_thread_context_isInvalid(pCtx)) {
        return PC_EOR;
    }

    if (!_thread_context_isInit(pCtx)) {
        return PC_EOR;
    }

    arguments_t arguments[] = {
        [0] = {.u32_val = (_u32_t)ctx},
    };

    return kernel_privilege_invoke((const void *)_thread_suspend_privilege_routine, arguments);
}

/**
 * @brief Yield current thread to allow other thread to run.
 *
 * @param id The thread unique id.
 *
 * @return The result of thread yield operation.
 */
_i32p_t _impl_thread_yield(void)
{
    if (!kernel_isInThreadMode()) {
        return PC_EOR;
    }

    return kernel_privilege_invoke((const void *)_thread_yield_privilege_routine, NULL);
}

/**
 * @brief Delete a idle thread, and erase the stack data.
 *
 * @param id The thread unique id.
 *
 * @return The result of thread delete operation.
 */
_i32p_t _impl_thread_delete(_u32_t ctx)
{
    thread_context_t *pCtx = (thread_context_t *)ctx;
    if (_thread_context_isInvalid(pCtx)) {
        return PC_EOR;
    }

    if (!_thread_context_isInit(pCtx)) {
        return PC_EOR;
    }

    arguments_t arguments[] = {
        [0] = {.u32_val = (_u32_t)ctx},
    };

    return kernel_privilege_invoke((const void *)_thread_delete_privilege_routine, arguments);
}

/**
 * @brief Put the current running thread into sleep mode with timeout condition.
 *
 * @param timeout_ms The time user defined.
 *
 * @return The result of thread sleep operation.
 */
_i32p_t _impl_thread_sleep(_u32_t timeout_ms)
{
    if (!timeout_ms) {
        return PC_EOR;
    }

    if (!kernel_isInThreadMode()) {
        return PC_EOR;
    }

    arguments_t arguments[] = {
        [0] = {.u32_val = (_u32_t)timeout_ms},
    };

    return kernel_privilege_invoke((const void *)_thread_sleep_privilege_routine, arguments);
}

void _impl_thread_entry(pThread_entryFunc_t pEntryFn, void *pArg)
{
    pEntryFn(pArg);

    _impl_thread_delete((_u32_t)kernel_thread_runContextGet());
    RUN_UNREACHABLE();
}
