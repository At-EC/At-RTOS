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
#define PC_EOR PC_IER(PC_OS_CMPT_POOL_9)

/**
 * @brief Check if the pool unique id if is's invalid.
 *
 * @param id The provided unique id.
 *
 * @return The true is invalid, otherwise is valid.
 */
static _b_t _pool_context_isInvalid(pool_context_t *pCurPool)
{
    _u32_t start, end;
    INIT_SECTION_FIRST(INIT_SECTION_OS_POOL_LIST, start);
    INIT_SECTION_LAST(INIT_SECTION_OS_POOL_LIST, end);

    return ((_u32_t)pCurPool < start || (_u32_t)pCurPool >= end) ? true : false;
}

/**
 * @brief Check if the pool object if is's initialized.
 *
 * @param id The provided unique id.
 *
 * @return The true is initialized, otherwise is uninitialized.
 */
static _b_t _pool_context_isInit(pool_context_t *pCurPool)
{
    return ((pCurPool) ? (((pCurPool->head.cs) ? (true) : (false))) : false);
}

/**
 * @brief Take a memory pool address.
 *
 * @param pCurPool The current pool context.
 *
 * @return The memory pool address.
 */
static void *_mem_take(pool_context_t *pCurPool)
{
    _u32_t free = pCurPool->elementFreeBits;
    _u32_t num = pCurPool->elementNumber;
    void *pMemTake = NULL;
    _u8_t i = 0u;

    do {
        if (!(free & B(i))) {
            continue;
        }

        pMemTake = (void *)((_u32_t)((i * pCurPool->elementLength) + (_u32_t)pCurPool->pMemAddress));
        os_memset((_char_t *)pMemTake, 0x0u, pCurPool->elementLength);
        pCurPool->elementFreeBits &= ~B(i);
        break;

    } while (i++ < num);

    return pMemTake;
}

/**
 * @brief Release a memory pool address.
 *
 * @param pCurPool The current pool context.
 *
 * @return The result of memory pool address release.
 */
static bool _mem_release(pool_context_t *pCurPool, void *pUserMem)
{
    _u32_t free = pCurPool->elementFreeBits;
    _u32_t num = pCurPool->elementNumber;
    void *pMemTake = NULL;
    _u8_t i = 0u;

    do {
        if (free & B(i)) {
            continue;
        }

        pMemTake = (void *)((_u32_t)((i * pCurPool->elementLength) + (_u32_t)pCurPool->pMemAddress));
        if (pMemTake == pUserMem) {
            os_memset((_char_t *)pMemTake, 0x0u, pCurPool->elementLength);
            pCurPool->elementFreeBits |= B(i);
            break;
        }
    } while (i++ < num);

    return (pMemTake == pUserMem) ? (true) : (false);
}

/**
 * @brief The pool schedule routine execute the the pendsv context.
 *
 * @param id The unique id of the entry thread.
 */
static void _pool_schedule(void *pTask)
{
    struct schedule_task *pCurTask = (struct schedule_task *)pTask;
    timeout_remove(&pCurTask->expire, true);
    pool_context_t *pCurPool = (pool_context_t *)pCurTask->pPendCtx;
    void **ppUserMemAddress = (void **)pCurTask->pPendData;
    if (!*ppUserMemAddress) {
        return;
    }
    *ppUserMemAddress = _mem_take(pCurPool);
    if (!*ppUserMemAddress) {
        pCurTask->exec.entry.result = PC_EOR;
    } else {
        pCurTask->exec.entry.result = 0;
    }
}

/**
 * @brief It's sub-routine running at privilege mode.
 *
 * @param pArgs The function argument packages.
 *
 * @return The result of privilege routine.
 */
static _u32_t _pool_init_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    const void *pMemAddr = (const void *)(pArgs[0].ptr_val);
    _u16_t elementLen = (_u16_t)(pArgs[1].u16_val);
    _u16_t elementNum = (_u16_t)(pArgs[2].u16_val);
    const _char_t *pName = (const _char_t *)(pArgs[3].pch_val);

    INIT_SECTION_FOREACH(INIT_SECTION_OS_POOL_LIST, pool_context_t, pCurPool)
    {
        if (_pool_context_isInvalid(pCurPool)) {
            break;
        }

        if (_pool_context_isInit(pCurPool)) {
            continue;
        }
        os_memset((_char_t *)pCurPool, 0x0u, sizeof(pool_context_t));
        pCurPool->head.cs = CS_INITED;
        pCurPool->head.pName = pName;

        pCurPool->pMemAddress = pMemAddr;
        pCurPool->elementLength = elementLen;
        pCurPool->elementNumber = elementNum;
        pCurPool->elementFreeBits = Bs(0u, (elementNum - 1u));

        EXIT_CRITICAL_SECTION();
        return (_u32_t)pCurPool;
    };

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
static _i32p_t _pool_take_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    pool_context_t *pCurPool = (pool_context_t *)pArgs[0].u32_val;
    void **ppUserBuffer = (void **)pArgs[1].pv_val;
    _u16_t bufferSize = (_u16_t)pArgs[2].u16_val;
    _u32_t timeout_ms = (_u32_t)pArgs[3].u32_val;
    thread_context_t *pCurThread = NULL;
    _i32p_t postcode = 0;

    pCurThread = kernel_thread_runContextGet();
    if (bufferSize > pCurPool->elementLength) {
        EXIT_CRITICAL_SECTION();
        return PC_EOR;
    }

    if (!pCurPool->elementFreeBits) {
        if ((timeout_ms == OS_TIME_NOWAIT_VAL) && (!kernel_isInThreadMode())) {
            EXIT_CRITICAL_SECTION();
            return PC_EOR;
        }
        postcode = schedule_exit_trigger(&pCurThread->task, pCurPool, ppUserBuffer, &pCurPool->q_list, timeout_ms, true);
        PC_IF(postcode, PC_PASS)
        {
            postcode = PC_OS_WAIT_UNAVAILABLE;
        }
    } else {
        *ppUserBuffer = _mem_take(pCurPool);

        if (!*ppUserBuffer) {
            postcode = PC_EOR;
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
static _i32p_t _pool_release_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    pool_context_t *pCurPool = (pool_context_t *)pArgs[0].u32_val;
    void **ppUserBuffer = (void **)pArgs[1].ptr_val;
    _i32p_t postcode = 0;

    if (!_mem_release(pCurPool, *ppUserBuffer)) {
        EXIT_CRITICAL_SECTION();
        return PC_EOR;
    }
    *ppUserBuffer = NULL;

    /* Try to wakeup a blocking thread */
    list_iterator_t it = {0u};
    list_t *pList = (list_t *)&pCurPool->q_list;
    list_iterator_init(&it, pList);
    struct schedule_task *pCurTask = (struct schedule_task *)list_iterator_next(&it);
    if (pCurTask) {
        postcode = schedule_entry_trigger(pCurTask, _pool_schedule, 0u);
    }

    EXIT_CRITICAL_SECTION();
    return postcode;
}

/**
 * @brief Initialize a new pool.
 *
 * @param pName The pool name.
 * @param pMemAddr The pointer of the pool buffer.
 * @param elementLen The element size.
 * @param elementNum The element number.
 *
 * @return The pool unique id.
 */
_u32_t _impl_pool_init(const void *pMemAddr, _u16_t elementLen, _u16_t elementNum, const _char_t *pName)
{
    if (!pMemAddr) {
        return OS_INVALID_ID_VAL;
    }

    if (!elementLen) {
        return OS_INVALID_ID_VAL;
    }

    if (!elementNum) {
        return OS_INVALID_ID_VAL;
    }

    if (elementNum > U32_B) {
        return OS_INVALID_ID_VAL;
    }

    arguments_t arguments[] = {
        [0] = {.ptr_val = (const void *)pMemAddr},
        [1] = {.u16_val = (_u16_t)elementLen},
        [2] = {.u16_val = (_u16_t)elementNum},
        [3] = {.pch_val = (const _char_t *)pName},
    };

    return kernel_privilege_invoke((const void *)_pool_init_privilege_routine, arguments);
}

/**
 * @brief Take a message pool resource.
 *
 * @param id The pool unique id.
 * @param ppUserBuffer The dual pointer of the message memory address.
 * @param pBufferSize The pointer of the message memory size.
 * @param timeout_ms The pool take timeout option.
 *
 * @return The result of the operation.
 */
_i32p_t _impl_pool_take(_u32_t ctx, void **ppUserBuffer, _u16_t bufferSize, _u32_t timeout_ms)
{
    pool_context_t *pCtx = (pool_context_t *)ctx;
    if (_pool_context_isInvalid(pCtx)) {
        return PC_EOR;
    }

    if (!_pool_context_isInit(pCtx)) {
        return PC_EOR;
    }

    if (!kernel_isInThreadMode()) {
        if (timeout_ms != OS_TIME_NOWAIT_VAL) {
            return PC_EOR;
        }
    }

    arguments_t arguments[] = {
        [0] = {.u32_val = (_u32_t)ctx},
        [1] = {.pv_val = (void **)ppUserBuffer},
        [2] = {.u16_val = (_u16_t)bufferSize},
        [3] = {.u32_val = (_u32_t)timeout_ms},
    };

    _i32p_t postcode = kernel_privilege_invoke((const void *)_pool_take_privilege_routine, arguments);

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
 * @brief Release memory pool.
 *
 * @param id The pool unique id.
 * @param ppUserBuffer The dual pointer of the message memory address.
 *
 * @return The result of the operation.
 */
_i32p_t _impl_pool_release(_u32_t ctx, void **ppUserBuffer)
{
    pool_context_t *pCtx = (pool_context_t *)ctx;
    if (_pool_context_isInvalid(pCtx)) {
        return PC_EOR;
    }

    if (!_pool_context_isInit(pCtx)) {
        return PC_EOR;
    }

    if (*ppUserBuffer == NULL) {
        return PC_EOR;
    }

    arguments_t arguments[] = {
        [0] = {.u32_val = (_u32_t)ctx},
        [1] = {.pv_val = (void **)ppUserBuffer},
    };

    return kernel_privilege_invoke((const void *)_pool_release_privilege_routine, arguments);
}
