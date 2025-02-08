/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 **/
#include "sched_kernel.h"
#include "sched_timer.h"
#include "k_trace.h"
#include "postcode.h"

/**
 * Local unique postcode.
 */
#define PC_EOR PC_IER(PC_OS_CMPT_MUTEX_5)

/**
 * @brief Check if the mutex unique id if is's invalid.
 *
 * @param id The provided unique id.
 *
 * @return The true is invalid, otherwise is valid.
 */
static _b_t _mutex_context_isInvalid(mutex_context_t *pCurMutex)
{
    _u32_t start, end;
    INIT_SECTION_FIRST(INIT_SECTION_OS_MUTEX_LIST, start);
    INIT_SECTION_LAST(INIT_SECTION_OS_MUTEX_LIST, end);

    return ((_u32_t)pCurMutex < start || (_u32_t)pCurMutex >= end) ? true : false;
}

/**
 * @brief Check if the mutex object if is's initialized.
 *
 * @param id The provided unique id.
 *
 * @return The true is initialized, otherwise is uninitialized.
 */
static _b_t _mutex_context_isInit(mutex_context_t *pCurMutex)
{
    return ((pCurMutex) ? (((pCurMutex->head.cs) ? (true) : (false))) : false);
}

/**
 * @brief It's sub-routine running at privilege mode.
 *
 * @param pArgs The function argument packages.
 *
 * @return The result of privilege routine.
 */
static _u32_t _mutex_init_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    const _char_t *pName = (const _char_t *)(pArgs[0].pch_val);

    INIT_SECTION_FOREACH(INIT_SECTION_OS_MUTEX_LIST, mutex_context_t, pCurMutex)
    {
        if (_mutex_context_isInvalid(pCurMutex)) {
            break;
        }

        if (_mutex_context_isInit(pCurMutex)) {
            continue;
        }

        k_memset((_char_t *)pCurMutex, 0x0u, sizeof(mutex_context_t));
        pCurMutex->head.cs = CS_INITED;
        pCurMutex->head.pName = pName;

        pCurMutex->locked = false;
        pCurMutex->pHoldTask = NULL;
        pCurMutex->originalPriority = OS_PRIOTITY_INVALID_LEVEL;

        EXIT_CRITICAL_SECTION();
        return (_u32_t)pCurMutex;
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
static _i32p_t _mutex_lock_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    mutex_context_t *pCurMutex = (mutex_context_t *)pArgs[0].u32_val;
    thread_context_t *pCurThread = NULL;
    _i32p_t postcode = 0;

    pCurThread = kernel_thread_runContextGet();
    if (pCurMutex->locked == true) {
        struct schedule_task *pLockTask = pCurMutex->pHoldTask;
        if (pCurThread->task.prior < pLockTask->prior) {
            pLockTask->prior = pCurThread->task.prior;
        }
        postcode = schedule_exit_trigger(&pCurThread->task, pCurMutex, NULL, &pCurMutex->q_list, 0u, true);

        EXIT_CRITICAL_SECTION();
        return postcode;
    }

    /* Highest priority inheritance */
    pCurMutex->pHoldTask = &pCurThread->task;
    pCurMutex->originalPriority = pCurThread->task.prior;
    pCurMutex->locked = true;

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
static _i32p_t _mutex_unlock_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    mutex_context_t *pCurMutex = (mutex_context_t *)pArgs[0].u32_val;
    _i32p_t postcode = 0;

    struct schedule_task *pCurTask = (struct schedule_task *)list_head(&pCurMutex->q_list);
    struct schedule_task *pLockTask = pCurMutex->pHoldTask;
    /* priority recovery */
    pLockTask->prior = pCurMutex->originalPriority;
    if (!pCurTask) {
        // no blocking thread
        pCurMutex->originalPriority = OS_PRIOTITY_INVALID_LEVEL;
        pCurMutex->pHoldTask = NULL;
        pCurMutex->locked = false;
    } else {
        /* The next thread take the ticket */
        pCurMutex->pHoldTask = pCurTask;
        pCurMutex->originalPriority = pCurTask->prior;
        postcode = schedule_entry_trigger(pCurTask, NULL, 0u);
    }

    EXIT_CRITICAL_SECTION();
    return postcode;
}

/**
 * @brief Initialize a new mutex.
 *
 * @param pName The mutex name.
 *
 * @return The mutex unique id.
 */
_u32_t _impl_mutex_init(const _char_t *pName)
{
    arguments_t arguments[] = {
        [0] = {.pch_val = (const _char_t *)pName},
    };

    return kernel_privilege_invoke((const void *)_mutex_init_privilege_routine, arguments);
}

/**
 * @brief Mutex lock to avoid another thread access this resource.
 *
 * @param id The mutex unique id.
 *
 * @return The result of the operation.
 */
_i32p_t _impl_mutex_lock(_u32_t ctx)
{
    mutex_context_t *pCtx = (mutex_context_t *)ctx;
    if (_mutex_context_isInvalid(pCtx)) {
        return PC_EOR;
    }

    if (!_mutex_context_isInit(pCtx)) {
        return PC_EOR;
    }

    if (!kernel_isInThreadMode()) {
        return PC_EOR;
    }

    arguments_t arguments[] = {
        [0] = {.u32_val = (_u32_t)ctx},
    };

    return kernel_privilege_invoke((const void *)_mutex_lock_privilege_routine, arguments);
}

/**
 * @brief Mutex unlock to allow another access the resource.
 *
 * @param id The mutex unique id.
 *
 * @return The result of the operation.
 */
_i32p_t _impl_mutex_unlock(_u32_t ctx)
{
    mutex_context_t *pCtx = (mutex_context_t *)ctx;
    if (_mutex_context_isInvalid(pCtx)) {
        return PC_EOR;
    }

    if (!_mutex_context_isInit(pCtx)) {
        return PC_EOR;
    }

    arguments_t arguments[] = {
        [0] = {.u32_val = (_u32_t)ctx},
    };

    return kernel_privilege_invoke((const void *)_mutex_unlock_privilege_routine, arguments);
}
