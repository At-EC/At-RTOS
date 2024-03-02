/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
**/

#include "kernal.h"
#include "timer.h"
#include "semaphore.h"
#include "postcode.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Local unique postcode.
 */
#define _PC_CMPT_FAILED                    PC_FAILED(PC_CMPT_SEMAPHORE)
#define _SEMAPHORE_AVAILABLE_COUNT_MAXIMUM (0xFEu)

/**
 * The local function lists for current file internal use.
 */
static u32_t _semaphore_init_privilege_routine(arguments_t *pArgs);
static u32_t _semaphore_take_privilege_routine(arguments_t *pArgs);
static u32_t _semaphore_give_privilege_routine(arguments_t *pArgs);
static u32_t _semaphore_flush_privilege_routine(arguments_t *pArgs);

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
    return (semaphore_context_t *)(_impl_kernal_member_unified_id_toContainerAddress(id));
}

/**
 * @brief Get the locking semaphore list head.
 *
 * @return The value of the locking list head.
 */
static list_t *_semaphore_list_lockingHeadGet(void)
{
    return (list_t *)_impl_kernal_member_list_get(KERNAL_MEMBER_SEMAPHORE, KERNAL_MEMBER_LIST_SEMAPHORE_LOCK);
}

/**
 * @brief Get the unlocking semaphore list head.
 *
 * @return The value of the unlocking list head.
 */
static list_t *_semaphore_list_unlockingHeadGet(void)
{
    return (list_t *)_impl_kernal_member_list_get(KERNAL_MEMBER_SEMAPHORE, KERNAL_MEMBER_LIST_SEMAPHORE_UNLOCK);
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
 * @brief Push one semaphore context into locking list.
 *
 * @param pCurHead The pointer of the semaphore linker head.
 */
static void _semaphore_list_transfer_toLock(linker_head_t *pCurHead)
{
    ENTER_CRITICAL_SECTION();

    list_t *pToLockingList = (list_t *)_semaphore_list_lockingHeadGet();
    linker_list_transaction_common(&pCurHead->linker, pToLockingList, LIST_TAIL);

    EXIT_CRITICAL_SECTION();
}

/**
 * @brief Push one semaphore context into unlocking list.
 *
 * @param pCurHead The pointer of the semaphore linker head.
 */
static void _semaphore_list_transfer_toUnlock(linker_head_t *pCurHead)
{
    ENTER_CRITICAL_SECTION();

    list_t *pToUnlockList = (list_t *)_semaphore_list_unlockingHeadGet();
    linker_list_transaction_common(&pCurHead->linker, pToUnlockList, LIST_TAIL);

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
    return _impl_kernal_member_unified_id_isInvalid(KERNAL_MEMBER_SEMAPHORE, id);
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
    _impl_kernal_thread_entry_trigger(_impl_kernal_member_unified_id_timerToThread(id), id, PC_SC_TIMEOUT, _semaphore_schedule);
}

/**
 * @brief Convert the internal os id to kernal member number.
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

    return (u32_t)((id - _impl_kernal_member_id_toUnifiedIdStart(KERNAL_MEMBER_SEMAPHORE)) / sizeof(semaphore_context_t));
}

/**
 * @brief Initialize a new semaphore.
 *
 * @param initial The initial count that allows the system take.
 * @param limit The maximum count that it's the semaphore's limitation.
 * @param permit It permits that all sem_give counts save until sem_take flush, even if the counts number higher than limitation setting
 * count.
 * @param pName The semaphore name.
 *
 * @return The semaphore unique id.
 */
os_id_t _impl_semaphore_init(u8_t initialCount, u8_t limitCount, b_t permit, const char_t *pName)
{
    if (!limitCount) {
        return OS_INVALID_ID;
    }

    arguments_t arguments[] = {
        [0] = {.u8_val = (u8_t)initialCount},
        [1] = {.u8_val = (u8_t)limitCount},
        [2] = {.b_val = (b_t)permit},
        [3] = {.pch_val = (const char_t *)pName},
    };

    return _impl_kernal_privilege_invoke((const void *)_semaphore_init_privilege_routine, arguments);
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

    if (!_impl_kernal_isInThreadMode()) {
        return _PC_CMPT_FAILED;
    }

    arguments_t arguments[] = {
        [0] = {.u32_val = (u32_t)id},
        [1] = {.u32_val = (u32_t)timeout_ms},
    };

    u32p_t postcode = _impl_kernal_privilege_invoke((const void *)_semaphore_take_privilege_routine, arguments);

    ENTER_CRITICAL_SECTION();

    if (postcode == PC_SC_UNAVAILABLE) {
        thread_context_t *pCurThread = _impl_kernal_thread_runContextGet();
        postcode = (u32p_t)_impl_kernal_schedule_entry_result_read_clean((action_schedule_t *)&pCurThread->schedule);
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

    return _impl_kernal_privilege_invoke((const void *)_semaphore_give_privilege_routine, arguments);
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

    return _impl_kernal_privilege_invoke((const void *)_semaphore_flush_privilege_routine, arguments);
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
    b_t permit = (b_t)(pArgs[2].b_val);
    const char_t *pName = (const char_t *)(pArgs[3].pch_val);
    u32_t internal = 0u;
    u32_t endAddr = 0u;
    semaphore_context_t *pCurSemaphore = NULL;

    internal = (sizeof(semaphore_context_t) * KERNAL_APPLICATION_SEMAPHORE_INSTANCE);
    pCurSemaphore = (semaphore_context_t *)(_impl_kernal_member_id_toContainerStartAddress(KERNAL_MEMBER_SEMAPHORE) + internal);
    endAddr = (u32_t)_impl_kernal_member_id_toContainerEndAddress(KERNAL_MEMBER_SEMAPHORE);

    do {
        os_id_t id = _impl_kernal_member_containerAddress_toUnifiedid((u32_t)pCurSemaphore);
        if (_semaphore_id_isInvalid(id)) {
            break;
        }

        if (_semaphore_object_isInit(id)) {
            continue;
        }

        _memset((char_t *)pCurSemaphore, 0x0u, sizeof(semaphore_context_t));
        pCurSemaphore->head.id = id;
        pCurSemaphore->head.pName = pName;

        pCurSemaphore->initialCount = initialCount;
        pCurSemaphore->limitCount = limitCount;
        pCurSemaphore->isPermit = permit;

        if (pCurSemaphore->initialCount < pCurSemaphore->limitCount) {
            _semaphore_list_transfer_toLock((linker_head_t *)&pCurSemaphore->head);
        } else {
            _semaphore_list_transfer_toUnlock((linker_head_t *)&pCurSemaphore->head);
        }

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
    u32p_t postcode = PC_SC_SUCCESS;

    pCurThread = _impl_kernal_thread_runContextGet();
    pCurSemaphore = _semaphore_object_contextGet(id);
    if (pCurSemaphore->head.linker.pList == _semaphore_list_lockingHeadGet()) {
        /* No availabe count */
        postcode = _impl_kernal_thread_exit_trigger(pCurThread->head.id, id, _semaphore_list_blockingHeadGet(id), timeout_ms,
                                                    _semaphore_callback_fromTimeOut);

        if (PC_IOK(postcode)) {
            postcode = PC_SC_UNAVAILABLE;
        }

        EXIT_CRITICAL_SECTION();
        return postcode;
    }

    /* The semaphore has available count */
    pCurSemaphore->initialCount--;

    /* Check if we can take the next acquired thread into locking */
    if (pCurSemaphore->initialCount < pCurSemaphore->limitCount) {
        _semaphore_list_transfer_toLock((linker_head_t *)&pCurSemaphore->head);
    }
    postcode = PC_SC_AVAILABLE;

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
    semaphore_context_t *pCurSemaphore = NULL;
    thread_context_t *pSemaphoreHighestBlockingThread = NULL;
    u32p_t postcode = PC_SC_SUCCESS;

    pCurSemaphore = _semaphore_object_contextGet(id);
    pSemaphoreHighestBlockingThread = (thread_context_t *)_semaphore_linker_head_fromBlocking(id);
    if (pCurSemaphore->isPermit) {
        /* permit to save all released counts */
        pCurSemaphore->initialCount++;

        if (pCurSemaphore->initialCount >= _SEMAPHORE_AVAILABLE_COUNT_MAXIMUM) {
            postcode = _PC_CMPT_FAILED;
        }
    } else if (pCurSemaphore->initialCount < pCurSemaphore->limitCount) {
        pCurSemaphore->initialCount++;
    }

    if (pSemaphoreHighestBlockingThread) {
        postcode = _impl_kernal_thread_entry_trigger(pSemaphoreHighestBlockingThread->head.id, id, PC_SC_SUCCESS, _semaphore_schedule);
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
        pCurSemaphore->initialCount++;
        postcode = _impl_kernal_thread_entry_trigger(pCurThread->head.id, id, PC_SC_SUCCESS, _semaphore_schedule);
        if (PC_IER(postcode)) {
            break;
        }
        pCurThread = (thread_context_t *)list_iterator_next(&it);
    }

    EXIT_CRITICAL_SECTION();
    return postcode;
}

/**
 * @brief The semaphore schedule routine execute the the pendsv context.
 *
 * @param id The unique id of the entry thread.
 */
static void _semaphore_schedule(os_id_t id)
{
    thread_context_t *pEntryThread = (thread_context_t *)(_impl_kernal_member_unified_id_toContainerAddress(id));
    semaphore_context_t *pCurSemaphore = NULL;
    thread_entry_t *pEntry = NULL;
    b_t isAvail = FALSE;

    if (_impl_kernal_member_unified_id_toId(pEntryThread->schedule.hold) != KERNAL_MEMBER_SEMAPHORE) {
        pEntryThread->schedule.entry.result = _PC_CMPT_FAILED;
        return;
    }

    if ((pEntryThread->schedule.entry.result != PC_SC_SUCCESS) && (pEntryThread->schedule.entry.result != PC_SC_TIMEOUT)) {
        return;
    }

    // Release function doesn't kill the timer node from waiting list
    pEntry = &pEntryThread->schedule.entry;
    pCurSemaphore = _semaphore_object_contextGet(pEntryThread->schedule.hold);
    if (!_impl_timer_status_isBusy(_impl_kernal_member_unified_id_threadToTimer(pEntryThread->head.id))) {
        if (_impl_kernal_member_unified_id_toId(pEntry->release) == KERNAL_MEMBER_TIMER_INTERNAL) {
            pEntry->result = PC_SC_TIMEOUT;
        } else {
            isAvail = true;
        }
    } else if (_impl_kernal_member_unified_id_toId(pEntry->release) == KERNAL_MEMBER_SEMAPHORE) {
        _impl_timer_stop(_impl_kernal_member_unified_id_threadToTimer(pEntryThread->head.id));
        isAvail = true;
    } else {
        pEntry->result = _PC_CMPT_FAILED;
    }

    if (isAvail) {
        pEntry->result = PC_SC_SUCCESS;
        /* If the PC arrive, the semaphore will be available and can be acquired */
        pCurSemaphore->initialCount--; // The semaphore has available count
    }

    /* Check if we can take the next acquired thread into locking */
    if (pCurSemaphore->initialCount < pCurSemaphore->limitCount) {
        _semaphore_list_transfer_toLock((linker_head_t *)&pCurSemaphore->head);
    } else {
        _semaphore_list_transfer_toUnlock((linker_head_t *)&pCurSemaphore->head);
    }
}

#ifdef __cplusplus
}
#endif
