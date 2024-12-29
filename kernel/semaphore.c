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

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Local unique postcode.
 */
#define PC_EOR PC_IER(PC_OS_CMPT_SEMAPHORE_4)

static void _semaphore_schedule(os_id_t id);

/**
 * @brief Get the semaphore context based on provided unique id.
 *
 * @param id The timer unique id.
 *
 * @return The pointer of the current unique id timer context.
 */

static semaphore_context_t *_semaphore_context_get(os_id_t id)
{
    return (semaphore_context_t *)(kernel_member_unified_id_toContainerAddress(id));
}

/**
 * @brief Check if the semaphore unique id if is's invalid.
 *
 * @param id The provided unique id.
 *
 * @return The true is invalid, otherwise is valid.
 */
static b_t _semaphore_id_isInvalid(u32_t id)
{
    return kernel_member_unified_id_isInvalid(KERNEL_MEMBER_SEMAPHORE, id);
}

/**
 * @brief Check if the semaphore object if is's initialized.
 *
 * @param id The provided unique id.
 *
 * @return The true is initialized, otherwise is uninitialized.
 */
static b_t _semaphore_id_isInit(i32_t id)
{
    semaphore_context_t *pCurSem = _semaphore_context_get(id);

    return ((pCurSem) ? (((pCurSem->head.cs) ? (TRUE) : (FALSE))) : FALSE);
}

/**
 * @brief Get the semaphore blocking thread list head address.
 *
 * @return The blocking thread list head address.
 */
static list_t *_semaphore_list_blockingHeadGet(os_id_t id)
{
    semaphore_context_t *pCurSemaphore = _semaphore_context_get(id);

    return (list_t *)((pCurSemaphore) ? (&pCurSemaphore->blockingThreadHead) : (NULL));
}

/**
 * @brief Pick up a highest priority thread that blocking by the semaphore pending list.
 *
 * @param The semaphore unique id.
 *
 * @return The highest blocking thread head.
 */
static linker_head_t *_semaphore_linker_head_fromBlocking(os_id_t id)
{
    ENTER_CRITICAL_SECTION();

    list_t *pListPending = (list_t *)_semaphore_list_blockingHeadGet(id);

    EXIT_CRITICAL_SECTION();
    return (linker_head_t *)(pListPending->pHead);
}

/**
 * @brief The semaphore schedule routine execute the the pendsv context.
 *
 * @param id The unique id of the entry thread.
 */
static void _semaphore_schedule(os_id_t id)
{
    thread_context_t *pEntryThread = (thread_context_t *)(kernel_member_unified_id_toContainerAddress(id));
    timeout_remove(&pEntryThread->expire, true);
    semaphore_context_t *pCurSemaphore = (semaphore_context_t *)pEntryThread->schedule.pPendCtx;
    /* If the PC arrive, the semaphore will be available and can be acquired */
    pCurSemaphore->remains--; // The semaphore has available count
    pEntryThread->schedule.entry.result = 0;
}

/**
 * @brief It's sub-routine running at privilege mode.
 *
 * @param pArgs The function argument packages.
 *
 * @return The result of privilege routine.
 */
static u32_t _semaphore_init_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    u8_t initialCount = (u8_t)(pArgs[0].u8_val);
    u8_t limitCount = (u8_t)(pArgs[1].u8_val);
    const char_t *pName = (const char_t *)(pArgs[2].pch_val);
    u32_t internal = 0u;
    u32_t endAddr = 0u;
    semaphore_context_t *pCurSemaphore = NULL;

    internal = (sizeof(semaphore_context_t) * KERNEL_APPLICATION_SEMAPHORE_INSTANCE);
    pCurSemaphore = (semaphore_context_t *)(kernel_member_id_toContainerStartAddress(KERNEL_MEMBER_SEMAPHORE) + internal);
    endAddr = (u32_t)kernel_member_id_toContainerEndAddress(KERNEL_MEMBER_SEMAPHORE);

    do {
        os_id_t id = kernel_member_containerAddress_toUnifiedid((u32_t)pCurSemaphore);
        if (_semaphore_id_isInvalid(id)) {
            break;
        }

        if (_semaphore_id_isInit(id)) {
            continue;
        }

        os_memset((char_t *)pCurSemaphore, 0x0u, sizeof(semaphore_context_t));
        pCurSemaphore->head.cs = CS_INITED;
        pCurSemaphore->head.pName = pName;
        pCurSemaphore->remains = initialCount;
        pCurSemaphore->limits = limitCount;

        EXIT_CRITICAL_SECTION();
        return id;
    } while ((u32_t)++pCurSemaphore < endAddr);

    EXIT_CRITICAL_SECTION();
    return OS_INVALID_ID_VAL;
}

/**
 * @brief It's sub-routine running at privilege mode.
 *
 * @param pArgs The function argument packages.
 *
 * @return The result of privilege routine.
 */
static i32p_t _semaphore_take_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    os_id_t id = (os_id_t)pArgs[0].u32_val;
    u32_t timeout_ms = (u32_t)pArgs[1].u32_val;
    semaphore_context_t *pCurSemaphore = NULL;
    thread_context_t *pCurThread = NULL;
    i32p_t postcode = PC_OS_WAIT_AVAILABLE;

    pCurThread = kernel_thread_runContextGet();
    pCurSemaphore = _semaphore_context_get(id);
    if (!pCurSemaphore->remains) {
        /* No availabe count */
        postcode = kernel_thread_exit_trigger(pCurThread, pCurSemaphore, _semaphore_list_blockingHeadGet(id), timeout_ms);
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
static i32p_t _semaphore_give_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    os_id_t id = (os_id_t)pArgs[0].u32_val;
    i32p_t postcode = 0;

    semaphore_context_t *pCurSemaphore = _semaphore_context_get(id);
    if (pCurSemaphore->remains < pCurSemaphore->limits) {
        pCurSemaphore->remains++;

        thread_context_t *pSemaphoreHighestBlockingThread = (thread_context_t *)_semaphore_linker_head_fromBlocking(id);
        if (pSemaphoreHighestBlockingThread) {
            postcode = kernel_thread_entry_trigger(pSemaphoreHighestBlockingThread, 0, _semaphore_schedule);
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
static i32p_t _semaphore_flush_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    semaphore_context_t *pCurSemaphore = NULL;
    thread_context_t *pCurThread = NULL;
    os_id_t id = (os_id_t)pArgs[0].u32_val;
    i32p_t postcode = 0;

    pCurSemaphore = _semaphore_context_get(id);
    list_iterator_t it = {0u};
    list_iterator_init(&it, _semaphore_list_blockingHeadGet(id));
    pCurThread = (thread_context_t *)list_iterator_next(&it);
    while (pCurThread) {
        pCurSemaphore->remains++;
        postcode = kernel_thread_entry_trigger(pCurThread, 0, _semaphore_schedule);
        if (PC_IER(postcode)) {
            break;
        }
        pCurThread = (thread_context_t *)list_iterator_next(&it);
    }

    EXIT_CRITICAL_SECTION();
    return postcode;
}

/**
 * @brief Convert the internal os id to kernel member number.
 *
 * @param id The provided unique id.
 *
 * @return The semaphore member's number.
 */
u32_t _impl_semaphore_os_id_to_number(os_id_t id)
{
    if (_semaphore_id_isInvalid(id)) {
        return 0u;
    }

    return (u32_t)((id - kernel_member_id_toUnifiedIdStart(KERNEL_MEMBER_SEMAPHORE)) / sizeof(semaphore_context_t));
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
os_id_t _impl_semaphore_init(u8_t remainCount, u8_t limitCount, const char_t *pName)
{
    if (!limitCount) {
        return OS_INVALID_ID_VAL;
    }

    if (remainCount > limitCount) {
        return OS_INVALID_ID_VAL;
    }

    arguments_t arguments[] = {
        [0] = {.u8_val = (u8_t)remainCount},
        [1] = {.u8_val = (u8_t)limitCount},
        [2] = {.pch_val = (const char_t *)pName},
    };

    return kernel_privilege_invoke((const void *)_semaphore_init_privilege_routine, arguments);
}

/**
 * @brief Take the semaphore away with timeout option.
 *
 * @param id The semaphore unique id.
 *
 * @return The result of the operation.
 */
i32p_t _impl_semaphore_take(os_id_t id, u32_t timeout_ms)
{
    if (_semaphore_id_isInvalid(id)) {
        return PC_EOR;
    }

    if (!_semaphore_id_isInit(id)) {
        return PC_EOR;
    }

    if (!timeout_ms) {
        return PC_EOR;
    }

    if (!kernel_isInThreadMode()) {
        return PC_EOR;
    }

    arguments_t arguments[] = {
        [0] = {.u32_val = (u32_t)id},
        [1] = {.u32_val = (u32_t)timeout_ms},
    };

    i32p_t postcode = kernel_privilege_invoke((const void *)_semaphore_take_privilege_routine, arguments);

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
i32p_t _impl_semaphore_give(os_id_t id)
{
    if (_semaphore_id_isInvalid(id)) {
        return PC_EOR;
    }

    if (!_semaphore_id_isInit(id)) {
        return PC_EOR;
    }

    arguments_t arguments[] = {
        [0] = {.u32_val = (u32_t)id},
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
i32p_t _impl_semaphore_flush(os_id_t id)
{
    if (_semaphore_id_isInvalid(id)) {
        return PC_EOR;
    }

    if (!_semaphore_id_isInit(id)) {
        return PC_EOR;
    }

    arguments_t arguments[] = {
        [0] = {.u32_val = (u32_t)id},
    };

    return kernel_privilege_invoke((const void *)_semaphore_flush_privilege_routine, arguments);
}

#ifdef __cplusplus
}
#endif
