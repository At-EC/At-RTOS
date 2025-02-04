/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 **/
#include "kernel.h"
#include "timer.h"
#include "postcode.h"
#include "trace.h"

/**
 * Local unique postcode.
 */
#define PC_EOR PC_IER(PC_OS_CMPT_SEMAPHORE_4)

/**
 * @brief Check if the semaphore unique id if is's invalid.
 *
 * @param id The provided unique id.
 *
 * @return The true is invalid, otherwise is valid.
 */
static _b_t _semaphore_context_isInvalid(semaphore_context_t *pCurSemaphore)
{
    _u32_t start, end;
    INIT_SECTION_FIRST(INIT_SECTION_OS_SEMAPHORE_LIST, start);
    INIT_SECTION_LAST(INIT_SECTION_OS_SEMAPHORE_LIST, end);

    return ((_u32_t)pCurSemaphore < start || (_u32_t)pCurSemaphore >= end) ? true : false;
}

/**
 * @brief Check if the semaphore object if is's initialized.
 *
 * @param id The provided unique id.
 *
 * @return The true is initialized, otherwise is uninitialized.
 */
static _b_t _semaphore_context_isInit(semaphore_context_t *pCurSemaphore)
{
    return ((pCurSemaphore) ? (((pCurSemaphore->head.cs) ? (true) : (false))) : false);
}

/**
 * @brief The semaphore schedule routine execute the the pendsv context.
 *
 * @param id The unique id of the entry thread.
 */
static void _semaphore_schedule(void *pTask)
{
    struct schedule_task *pCurTask = (struct schedule_task *)pTask;
    timeout_remove(&pCurTask->expire, true);
    semaphore_context_t *pCurSemaphore = (semaphore_context_t *)pCurTask->pPendCtx;
    /* If the PC arrive, the semaphore will be available and can be acquired */
    pCurSemaphore->remains--; // The semaphore has available count
    pCurTask->exec.entry.result = 0;
}

/**
 * @brief It's sub-routine running at privilege mode.
 *
 * @param pArgs The function argument packages.
 *
 * @return The result of privilege routine.
 */
static _u32_t _semaphore_init_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    _u8_t initialCount = (_u8_t)(pArgs[0].u8_val);
    _u8_t limitCount = (_u8_t)(pArgs[1].u8_val);
    const _char_t *pName = (const _char_t *)(pArgs[2].pch_val);

    INIT_SECTION_FOREACH(INIT_SECTION_OS_SEMAPHORE_LIST, semaphore_context_t, pCurSemaphore)
    {
        if (_semaphore_context_isInvalid(pCurSemaphore)) {
            break;
        }

        if (_semaphore_context_isInit(pCurSemaphore)) {
            continue;
        }

        os_memset((_char_t *)pCurSemaphore, 0x0u, sizeof(semaphore_context_t));
        pCurSemaphore->head.cs = CS_INITED;
        pCurSemaphore->head.pName = pName;
        pCurSemaphore->remains = initialCount;
        pCurSemaphore->limits = limitCount;

        EXIT_CRITICAL_SECTION();
        return (_u32_t)pCurSemaphore;
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
static _i32p_t _semaphore_take_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    semaphore_context_t *pCurSemaphore = (semaphore_context_t *)pArgs[0].u32_val;
    _u32_t timeout_ms = (_u32_t)pArgs[1].u32_val;
    thread_context_t *pCurThread = NULL;
    _i32p_t postcode = PC_OS_WAIT_AVAILABLE;

    pCurThread = kernel_thread_runContextGet();
    if (!pCurSemaphore->remains) {
        /* No availabe count */
        postcode = schedule_exit_trigger(&pCurThread->task, pCurSemaphore, NULL, &pCurSemaphore->q_list, timeout_ms, true);
        PC_IF(postcode, PC_PASS)
        {
            postcode = PC_OS_WAIT_UNAVAILABLE;
        }

        EXIT_CRITICAL_SECTION();
        return postcode;
    }

    /* The semaphore has available count */
    pCurSemaphore->remains--;

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
static _i32p_t _semaphore_give_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    semaphore_context_t *pCurSemaphore = (semaphore_context_t *)pArgs[0].u32_val;
    _i32p_t postcode = 0;

    if (pCurSemaphore->remains < pCurSemaphore->limits) {
        pCurSemaphore->remains++;

        struct schedule_task *pCurTask = (struct schedule_task *)list_head(&pCurSemaphore->q_list);
        if (pCurTask) {
            postcode = schedule_entry_trigger(pCurTask, _semaphore_schedule, 0u);
        }
    }

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
static _i32p_t _semaphore_flush_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    semaphore_context_t *pCurSemaphore = (semaphore_context_t *)pArgs[0].u32_val;
    _i32p_t postcode = 0;

    list_iterator_t it = {0u};
    list_t *pQList = (list_t *)&pCurSemaphore->q_list;
    list_iterator_init(&it, pQList);
    struct schedule_task *pCurTask = (struct schedule_task *)list_iterator_next(&it);
    while (pCurTask) {
        pCurSemaphore->remains++;
        postcode = schedule_entry_trigger(pCurTask, _semaphore_schedule, 0u);
        if (PC_IER(postcode)) {
            break;
        }
        pCurTask = (struct schedule_task *)list_iterator_next(&it);
    }

    EXIT_CRITICAL_SECTION();
    return postcode;
}

/**
 * @brief Initialize a new semaphore.
 *
 * @param remainCount The initial count that allows the system take.
 * @param limitCount The maximum count that it's the semaphore's limitation.
 * @param pName The semaphore name.
 *
 * @return The semaphore unique id.
 */
_u32_t _impl_semaphore_init(_u8_t remainCount, _u8_t limitCount, const _char_t *pName)
{
    if (!limitCount) {
        return OS_INVALID_ID_VAL;
    }

    if (remainCount > limitCount) {
        return OS_INVALID_ID_VAL;
    }

    arguments_t arguments[] = {
        [0] = {.u8_val = (_u8_t)remainCount},
        [1] = {.u8_val = (_u8_t)limitCount},
        [2] = {.pch_val = (const _char_t *)pName},
    };

    return kernel_privilege_invoke((const void *)_semaphore_init_privilege_routine, arguments);
}

/**
 * @brief Take the semaphore away with timeout option.
 *
 * @param ctx The semaphore unique id.
 *
 * @return The result of the operation.
 */
_i32p_t _impl_semaphore_take(_u32_t ctx, _u32_t timeout_ms)
{
    semaphore_context_t *pCtx = (semaphore_context_t *)ctx;
    if (_semaphore_context_isInvalid(pCtx)) {
        return PC_EOR;
    }

    if (!_semaphore_context_isInit(pCtx)) {
        return PC_EOR;
    }

    if (!timeout_ms) {
        return PC_EOR;
    }

    if (!kernel_isInThreadMode()) {
        return PC_EOR;
    }

    arguments_t arguments[] = {
        [0] = {.u32_val = (_u32_t)ctx},
        [1] = {.u32_val = (_u32_t)timeout_ms},
    };

    _i32p_t postcode = kernel_privilege_invoke((const void *)_semaphore_take_privilege_routine, arguments);

    ENTER_CRITICAL_SECTION();

    if (postcode == PC_OS_WAIT_UNAVAILABLE) {
        postcode = kernel_schedule_result_take();
    }

    PC_IF(postcode, PC_PASS_INFO)
    {
        if (postcode != PC_OS_WAIT_TIMEOUT) {
            postcode = 0;
        }
    }

    EXIT_CRITICAL_SECTION();
    return postcode;
}

/**
 * @brief Give the semaphore to release the avaliable count.
 *
 * @param id The semaphore unique id.
 *
 * @return The result of the operation.
 */
_i32p_t _impl_semaphore_give(_u32_t ctx)
{
    semaphore_context_t *pCtx = (semaphore_context_t *)ctx;
    if (_semaphore_context_isInvalid(pCtx)) {
        return PC_EOR;
    }

    if (!_semaphore_context_isInit(pCtx)) {
        return PC_EOR;
    }

    arguments_t arguments[] = {
        [0] = {.u32_val = (_u32_t)ctx},
    };

    return kernel_privilege_invoke((const void *)_semaphore_give_privilege_routine, arguments);
}

/**
 * @brief Flush the semaphore to release all the avaliable count.
 *
 * @param id The semaphore unique id.
 *
 * @return The result of the operation.
 */
_i32p_t _impl_semaphore_flush(_u32_t ctx)
{
    semaphore_context_t *pCtx = (semaphore_context_t *)ctx;
    if (_semaphore_context_isInvalid(pCtx)) {
        return PC_EOR;
    }

    if (!_semaphore_context_isInit(pCtx)) {
        return PC_EOR;
    }

    arguments_t arguments[] = {
        [0] = {.u32_val = (_u32_t)ctx},
    };

    return kernel_privilege_invoke((const void *)_semaphore_flush_privilege_routine, arguments);
}
