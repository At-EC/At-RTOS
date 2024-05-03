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
#define _PC_CMPT_FAILED                    PC_FAILED(PC_CMPT_SEMAPHORE_3)
#define _SEMAPHORE_AVAILABLE_COUNT_MAXIMUM (0xFEu)

static void _semaphore_schedule(os_id_t id);

/**
 * @brief Get the semaphore context based on provided unique id.
 *
 * @param id The timer unique id.
 *
 * @return The pointer of the current unique id timer context.
 */

static semaphore_context_t *_semaphore_object_contextGet(os_id_t id)
{
    return (semaphore_context_t *)(kernel_member_unified_id_toContainerAddress(id));
}

/**
 * @brief Get the init semaphore list head.
 *
 * @return The value of the init list head.
 */
static list_t *_semaphore_list_initHeadGet(void)
{
    return (list_t *)kernel_member_list_get(KERNEL_MEMBER_SEMAPHORE, KERNEL_MEMBER_LIST_SEMAPHORE_INIT);
}

/**
 * @brief Get the semaphore blocking thread list head address.
 *
 * @return The blocking thread list head address.
 */
static list_t *_semaphore_list_blockingHeadGet(os_id_t id)
{
    semaphore_context_t *pCurSemaphore = _semaphore_object_contextGet(id);

    return (list_t *)((pCurSemaphore) ? (&pCurSemaphore->blockingThreadHead) : (NULL));
}

/**
 * @brief Push one semaphore context into init list.
 *
 * @param pCurHead The pointer of the semaphore linker head.
 */
static void _semaphore_list_transfer_toInit(linker_head_t *pCurHead)
{
    ENTER_CRITICAL_SECTION();

    list_t *pToLockingList = (list_t *)_semaphore_list_initHeadGet();
    linker_list_transaction_common(&pCurHead->linker, pToLockingList, LIST_TAIL);

    EXIT_CRITICAL_SECTION();
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
static b_t _semaphore_object_isInit(i32_t id)
{
    semaphore_context_t *pCurSemaphore = _semaphore_object_contextGet(id);

    return ((pCurSemaphore) ? (((pCurSemaphore->head.linker.pList) ? (TRUE) : (FALSE))) : FALSE);
}

/**
 * @brief The semaphore timeout callback fucntion.
 *
 * @param id The semaphore unique id.
 */
static void _semaphore_callback_fromTimeOut(os_id_t id)
{
    kernel_thread_entry_trigger(kernel_member_unified_id_timerToThread(id), id, PC_SC_TIMEOUT, _semaphore_schedule);
}

/**
 * @brief The semaphore schedule routine execute the the pendsv context.
 *
 * @param id The unique id of the entry thread.
 */
static void _semaphore_schedule(os_id_t id)
{
    thread_context_t *pEntryThread = (thread_context_t *)(kernel_member_unified_id_toContainerAddress(id));
    semaphore_context_t *pCurSemaphore = NULL;
    thread_entry_t *pEntry = NULL;
    b_t isAvail = FALSE;

    if (kernel_member_unified_id_toId(pEntryThread->schedule.hold) != KERNEL_MEMBER_SEMAPHORE) {
        pEntryThread->schedule.entry.result = _PC_CMPT_FAILED;
        return;
    }

    if ((pEntryThread->schedule.entry.result != PC_SC_SUCCESS) && (pEntryThread->schedule.entry.result != PC_SC_TIMEOUT)) {
        return;
    }

    // Release function doesn't kill the timer node from waiting list
    pEntry = &pEntryThread->schedule.entry;
    pCurSemaphore = _semaphore_object_contextGet(pEntryThread->schedule.hold);
    if (!timer_busy(kernel_member_unified_id_threadToTimer(pEntryThread->head.id))) {
        if (kernel_member_unified_id_toId(pEntry->release) == KERNEL_MEMBER_TIMER_INTERNAL) {
            pEntry->result = PC_SC_TIMEOUT;
        } else {
            isAvail = true;
        }
    } else if (kernel_member_unified_id_toId(pEntry->release) == KERNEL_MEMBER_SEMAPHORE) {
        timer_stop_for_thread(kernel_member_unified_id_threadToTimer(pEntryThread->head.id));
        isAvail = true;
    } else {
        pEntry->result = _PC_CMPT_FAILED;
    }

    if (isAvail) {
        /* If the PC arrive, the semaphore will be available and can be acquired */
        pCurSemaphore->remains--; // The semaphore has available count

        pEntry->result = PC_SC_SUCCESS;
    }
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

        if (_semaphore_object_isInit(id)) {
            continue;
        }

        os_memset((char_t *)pCurSemaphore, 0x0u, sizeof(semaphore_context_t));
        pCurSemaphore->head.id = id;
        pCurSemaphore->head.pName = pName;
        pCurSemaphore->remains = initialCount;
        pCurSemaphore->limits = limitCount;
        _semaphore_list_transfer_toInit((linker_head_t *)&pCurSemaphore->head);

        EXIT_CRITICAL_SECTION();
        return id;
    } while ((u32_t)++pCurSemaphore < endAddr);

    EXIT_CRITICAL_SECTION();
    return OS_INVALID_ID;
}

/**
 * @brief It's sub-routine running at privilege mode.
 *
 * @param pArgs The function argument packages.
 *
 * @return The result of privilege routine.
 */
static u32_t _semaphore_take_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    os_id_t id = (os_id_t)pArgs[0].u32_val;
    u32_t timeout_ms = (u32_t)pArgs[1].u32_val;
    semaphore_context_t *pCurSemaphore = NULL;
    thread_context_t *pCurThread = NULL;
    u32p_t postcode = PC_SC_AVAILABLE;

    pCurThread = kernel_thread_runContextGet();
    pCurSemaphore = _semaphore_object_contextGet(id);
    if (!pCurSemaphore->remains) {
        /* No availabe count */
        postcode = kernel_thread_exit_trigger(pCurThread->head.id, id, _semaphore_list_blockingHeadGet(id), timeout_ms,
                                              _semaphore_callback_fromTimeOut);

        if (PC_IOK(postcode)) {
            postcode = PC_SC_UNAVAILABLE;
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
static u32_t _semaphore_give_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    os_id_t id = (os_id_t)pArgs[0].u32_val;
    u32p_t postcode = PC_SC_SUCCESS;

    semaphore_context_t *pCurSemaphore = _semaphore_object_contextGet(id);
    if (pCurSemaphore->remains < pCurSemaphore->limits) {
        pCurSemaphore->remains++;

        thread_context_t *pSemaphoreHighestBlockingThread = (thread_context_t *)_semaphore_linker_head_fromBlocking(id);
        if (pSemaphoreHighestBlockingThread) {
            postcode = kernel_thread_entry_trigger(pSemaphoreHighestBlockingThread->head.id, id, PC_SC_SUCCESS, _semaphore_schedule);
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
static u32_t _semaphore_flush_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    semaphore_context_t *pCurSemaphore = NULL;
    thread_context_t *pCurThread = NULL;
    os_id_t id = (os_id_t)pArgs[0].u32_val;
    u32p_t postcode = PC_SC_SUCCESS;

    pCurSemaphore = _semaphore_object_contextGet(id);
    list_iterator_t it = {0u};
    list_iterator_init(&it, _semaphore_list_blockingHeadGet(id));
    pCurThread = (thread_context_t *)list_iterator_next(&it);
    while (pCurThread) {
        pCurSemaphore->remains++;
        postcode = kernel_thread_entry_trigger(pCurThread->head.id, id, PC_SC_SUCCESS, _semaphore_schedule);
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
        return OS_INVALID_ID;
    }

    if (remainCount > limitCount) {
        return OS_INVALID_ID;
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
u32p_t _impl_semaphore_take(os_id_t id, u32_t timeout_ms)
{
    if (_semaphore_id_isInvalid(id)) {
        return _PC_CMPT_FAILED;
    }

    if (!_semaphore_object_isInit(id)) {
        return _PC_CMPT_FAILED;
    }

    if (!timeout_ms) {
        return _PC_CMPT_FAILED;
    }

    if (!kernel_isInThreadMode()) {
        return _PC_CMPT_FAILED;
    }

    arguments_t arguments[] = {
        [0] = {.u32_val = (u32_t)id},
        [1] = {.u32_val = (u32_t)timeout_ms},
    };

    u32p_t postcode = kernel_privilege_invoke((const void *)_semaphore_take_privilege_routine, arguments);

    ENTER_CRITICAL_SECTION();

    if (postcode == PC_SC_UNAVAILABLE) {
        thread_context_t *pCurThread = kernel_thread_runContextGet();
        postcode = (u32p_t)kernel_schedule_entry_result_take((action_schedule_t *)&pCurThread->schedule);
    }

    if (PC_IOK(postcode) && (postcode != PC_SC_TIMEOUT)) {
        postcode = PC_SC_SUCCESS;
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
u32_t _impl_semaphore_give(os_id_t id)
{
    if (_semaphore_id_isInvalid(id)) {
        return _PC_CMPT_FAILED;
    }

    if (!_semaphore_object_isInit(id)) {
        return _PC_CMPT_FAILED;
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
u32_t _impl_semaphore_flush(os_id_t id)
{
    if (_semaphore_id_isInvalid(id)) {
        return _PC_CMPT_FAILED;
    }

    if (!_semaphore_object_isInit(id)) {
        return _PC_CMPT_FAILED;
    }

    arguments_t arguments[] = {
        [0] = {.u32_val = (u32_t)id},
    };

    return kernel_privilege_invoke((const void *)_semaphore_flush_privilege_routine, arguments);
}

/**
 * @brief Get semaphore snapshot informations.
 *
 * @param instance The semaphore instance number.
 * @param pMsgs The kernel snapshot information pointer.
 *
 * @return TRUE: Operation pass, FALSE: Operation failed.
 */
b_t semaphore_snapshot(u32_t instance, kernel_snapshot_t *pMsgs)
{
#if defined KTRACE
    semaphore_context_t *pCurSemaphore = NULL;
    u32_t offset = 0u;
    os_id_t id = OS_INVALID_ID;

    ENTER_CRITICAL_SECTION();

    offset = sizeof(semaphore_context_t) * instance;
    pCurSemaphore = (semaphore_context_t *)(kernel_member_id_toContainerStartAddress(KERNEL_MEMBER_SEMAPHORE) + offset);
    id = kernel_member_containerAddress_toUnifiedid((u32_t)pCurSemaphore);
    os_memset((u8_t *)pMsgs, 0x0u, sizeof(kernel_snapshot_t));

    if (_semaphore_id_isInvalid(id)) {
        EXIT_CRITICAL_SECTION();
        return FALSE;
    }

    if (pCurSemaphore->head.linker.pList == _semaphore_list_initHeadGet()) {
        pMsgs->pState = "init";
    } else if (pCurSemaphore->head.linker.pList) {
        pMsgs->pState = "*";
    } else {
        pMsgs->pState = "unused";

        EXIT_CRITICAL_SECTION();
        return FALSE;
    }

    pMsgs->id = pCurSemaphore->head.id;
    pMsgs->pName = pCurSemaphore->head.pName;

    pMsgs->semaphore.initial_count = pCurSemaphore->remains;
    pMsgs->semaphore.limit_count = pCurSemaphore->limits;
    pMsgs->semaphore.timeout_ms = pCurSemaphore->timeout_ms;
    pMsgs->semaphore.wait_list = pCurSemaphore->blockingThreadHead;

    EXIT_CRITICAL_SECTION();
    return TRUE;
#else
    return FALSE;
#endif
}

#ifdef __cplusplus
}
#endif
