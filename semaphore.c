/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "basic.h"
#include "kernal.h"
#include "timer.h"
#include "semaphore.h"

#ifdef __cplusplus
extern "C" {
#endif

#define _PC_CMPT_FAILED                             PC_FAILED(PC_CMPT_SEMAPHORE)
#define _SEMAPHORE_AVAILABLE_COUNT_MAXIMUM          (0xFFu)

static void  _semaphore_callback_fromTimeOut(os_id_t id);
static u32_t _semaphore_init_privilege_routine(arguments_t *pArgs);
static u32_t _semaphore_take_privilege_routine(arguments_t *pArgs);
static u32_t _semaphore_give_privilege_routine(arguments_t *pArgs);
static u32_t _semaphore_flush_privilege_routine(arguments_t *pArgs);

static void _semaphore_schedule(os_id_t id);

/**
 * @brief Get the semaphore context based on provided unique id.
 *
 * Get the semaphore context based on provided unique id, and then return the semaphore context pointer.
 *
 * @param id The semaphore unique id.
 *
 * @retval VALUE The semaphore context.
 */
static semaphore_context_t* _semaphore_object_contextGet(os_id_t id)
{
    return (semaphore_context_t*)(_impl_kernal_member_unified_id_toContainerAddress(id));
}

/**
 * @brief Get the semaphore locking list head address.
 *
 * Get the semaphore locking list head address.
 *
 * @param NONE.
 *
 * @retval VALUE The locking list head address.
 */
static list_t* _semaphore_list_lockingHeadGet(void)
{
    return (list_t*)_impl_kernal_member_list_get(KERNAL_MEMBER_SEMAPHORE, KERNAL_MEMBER_LIST_SEMAPHORE_LOCK);
}

/**
 * @brief Get the semaphore unlocking list head address.
 *
 * Get the semaphore unlocking list head address.
 *
 * @param NONE.
 *
 * @retval VALUE The unlocking list head address.
 */
static list_t* _semaphore_list_unlockingHeadGet(void)
{
    return (list_t*)_impl_kernal_member_list_get(KERNAL_MEMBER_SEMAPHORE, KERNAL_MEMBER_LIST_SEMAPHORE_UNLOCK);
}

/**
 * @brief Get the semaphore blocking thread list head address.
 *
 * Get the blocking thread list head address.
 *
 * @param NONE.
 *
 * @retval VALUE the blocking thread list head address.
 */
static list_t* _semaphore_list_blockingHeadGet(os_id_t id)
{
    semaphore_context_t *pCurSemaphore = (semaphore_context_t *)_semaphore_object_contextGet(id);

    return (list_t*)((pCurSemaphore) ? (&pCurSemaphore->blockingThreadHead) : (NULL));
}

/**
 * @brief Push one semaphore context into locking list.
 *
 * Push one semaphore context into locking list.
 *
 * @param pCurHead The pointer of the semaphore linker head.
 *
 * @retval NONE .
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
 * Push one semaphore context into unlocking list.
 *
 * @param pCurHead The pointer of the semaphore linker head.
 *
 * @retval NONE .
 */
static void _semaphore_list_transfer_toUnlock(linker_head_t *pCurHead)
{
    ENTER_CRITICAL_SECTION();

    list_t *pToUnlockList = (list_t *)_semaphore_list_unlockingHeadGet();
    linker_list_transaction_common(&pCurHead->linker, pToUnlockList, LIST_TAIL);

    EXIT_CRITICAL_SECTION();
}

/**
 * @brief Pick up a highest priority thread from the pending list.
 *
 * Pick up a highest priority thread from the pending list.
 *
 * @param NONE
 *
 * @retval VALUE The highest thread head.
 */
static linker_head_t* _semaphore_linker_head_fromBlocking(os_id_t id)
{
    ENTER_CRITICAL_SECTION();
    list_t *pListPending = (list_t *)_semaphore_list_blockingHeadGet(id);
    EXIT_CRITICAL_SECTION();

    return (linker_head_t*)(pListPending->pHead);
}

/**
 * @brief Check if the semaphore unique id if is's invalid.
 *
 * Check if the semaphore unique id if is's invalid.
 *
 * @param id The provided unique id.
 *
 * @retval TRUE The id is invalid
 *         FALSE The id is valid
 */
static b_t _semaphore_id_isInvalid(i32_t id)
{
    return _impl_kernal_member_unified_id_isInvalid(KERNAL_MEMBER_SEMAPHORE, id);
}

/**
 * @brief Check if the semaphore object if is's initialized.
 *
 * Check if the semaphore unique id if is's initialization.
 *
 * @param id The provided unique id.
 *
 * @retval TRUE The id is initialized.
 *         FALSE The id isn't initialized.
 */
static b_t _semaphore_object_isInit(i32_t id)
{
    semaphore_context_t *pCurSemaphore = (semaphore_context_t *)_semaphore_object_contextGet(id);

    if (pCurSemaphore)
    {
        return ((pCurSemaphore->head.linker.pList) ? (TRUE) : (FALSE));
    }
    else
    {
        return FALSE;
    }
}

/**
 * @brief The semaphore timeout callback fucntion.
 *
 * The semaphore timeout callback fucntion.
 *
 * @param id The semaphore unique id.
 *
 * @retval NONE.
 */
static void _semaphore_callback_fromTimeOut(os_id_t id)
{
    timer_context_t *pCurTimer = (timer_context_t *)_impl_kernal_member_unified_id_toContainerAddress(id);
    _impl_kernal_thread_entry_trigger(_impl_kernal_member_unified_id_timerToThread(id), id, PC_SC_TIMEOUT, _semaphore_schedule);
}

/**
 * @brief Convert the internal os id to kernal member number.
 *
 * Convert the internal os id to kernal member number.
 *
 * @param id The provided unique id.
 *
 * @retval VALUE Member number.
 */
u32_t _impl_semaphore_os_id_to_number(os_id_t id)
{
    return (u32_t)(_semaphore_id_isInvalid(id) ? (0u) : (id - _impl_kernal_member_id_toUnifiedIdStart(KERNAL_MEMBER_SEMAPHORE)) / sizeof(semaphore_context_t));
}

/**
 * @brief Initialize a new semaphore.
 *
 * Initialize a new semaphore.
 *
 * @param pName The semaphore name.
 * @param availableCount The available count that allows the system take.
 * @param limitationCount The maximum count that it's the semaphore's limitation.
 *
 * @retval VALUE The semaphore unique id.
 */
os_id_t _impl_semaphore_init(u8_t availableCount, u8_t limitationCount, const char_t *pName)
{
    arguments_t arguments[] =
    {
        [0] = {(u32_t)availableCount},
        [1] = {(u32_t)limitationCount},
        [2] = {(u32_t)pName},
    };

    return _impl_kernal_privilege_invoke(_semaphore_init_privilege_routine, arguments);
}

/**
 * @brief Take the semaphore away with timeout option.
 *
 * Take the semaphore away wiht timeout option.
 *
 * @param id The semaphore unique id.
 *
 * @retval VALUE The result of the operation.
 */
u32p_t _impl_semaphore_take(os_id_t id, u32_t timeout_ms)
{
    if (_semaphore_id_isInvalid(id))
    {
        return _PC_CMPT_FAILED;
    }

    if (!_semaphore_object_isInit(id))
    {
        return _PC_CMPT_FAILED;
    }

    if (!timeout_ms)
    {
        return _PC_CMPT_FAILED;
    }

    if (!_impl_kernal_isInThreadMode())
    {
        return _PC_CMPT_FAILED;
    }

    arguments_t arguments[] =
    {
        [0] = {(u32_t)id},
        [1] = {(u32_t)timeout_ms},
    };

    u32p_t postcode = _impl_kernal_privilege_invoke(_semaphore_take_privilege_routine, arguments);

    ENTER_CRITICAL_SECTION();

    if (postcode == PC_SC_UNAVAILABLE)
    {
        thread_context_t *pCurThread = (thread_context_t *)_impl_kernal_thread_runContextGet();
        postcode = (u32p_t)pCurThread->schedule.entry.result;
        pCurThread->schedule.entry.result = 0u;
    }

    if (PC_IOK(postcode) && (postcode != PC_SC_TIMEOUT))
    {
        postcode = PC_SC_SUCCESS;
    }
    EXIT_CRITICAL_SECTION();

   return postcode;
}

/**
 * @brief Give the semaphore to release the avaliable count.
 *
 * Give the semaphore to release the avaliable count.
 *
 * @param id The semaphore unique id.
 *
 * @retval VALUE The result of the operation.
 */
u32_t _impl_semaphore_give(os_id_t id)
{
    if (_semaphore_id_isInvalid(id))
    {
        return _PC_CMPT_FAILED;
    }

    if (!_semaphore_object_isInit(id))
    {
        return _PC_CMPT_FAILED;
    }

    arguments_t arguments[] =
    {
        [0] = {(u32_t)id},
    };

    return _impl_kernal_privilege_invoke(_semaphore_give_privilege_routine, arguments);
}

/**
 * @brief Flush the semaphore to release all the avaliable count.
 *
 * Flush the semaphore to release all the avaliable count.
 *
 * @param id The semaphore unique id.
 *
 * @retval POSTCODE_RTOS_SEMAPHORE_FLUSH_SUCCESS Semaphore flush successful.
 *         POSTCODE_RTOS_SEMAPHORE_FLUSH_FAILED Semaphore flush failed.
 */
u32_t _impl_semaphore_flush(os_id_t id)
{
    if (_semaphore_id_isInvalid(id))
    {
        return _PC_CMPT_FAILED;
    }

    if (!_semaphore_object_isInit(id))
    {
        return _PC_CMPT_FAILED;
    }

    arguments_t arguments[] =
    {
        [0] = {(u32_t)id},
    };

    return _impl_kernal_privilege_invoke(_semaphore_flush_privilege_routine, arguments);
}

/**
 * @brief The privilege routine running at handle mode.
 *
 * The privilege routine running at handle mode.
 *
 * @param args_0 The function argument 1.
 * @param args_1 The function argument 2.
 * @param args_2 The function argument 3.
 * @param args_3 The function argument 4.
 *
 * @retval VALUE The result of privilege routine.
 */
static u32_t _semaphore_init_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    u8_t availableCount = (u8_t)(pArgs[0].u32_val);
    u8_t limitationCount = (u8_t)(pArgs[1].u32_val);
    const char_t *pName = (const char_t *)(pArgs[2].u32_val);

    semaphore_context_t *pCurSemaphore = (semaphore_context_t *)_impl_kernal_member_id_toContainerStartAddress(KERNAL_MEMBER_SEMAPHORE);
    os_id_t id = _impl_kernal_member_id_toUnifiedIdStart(KERNAL_MEMBER_SEMAPHORE);

    do {
        if (!_semaphore_object_isInit(id))
        {
            _memset((char_t*)pCurSemaphore, 0x0u, sizeof(semaphore_context_t));

            pCurSemaphore->head.id = id;
            pCurSemaphore->head.pName = pName;

            pCurSemaphore->availableCount = availableCount;
            pCurSemaphore->limitationCount = limitationCount;

            if (pCurSemaphore->availableCount >= pCurSemaphore->limitationCount)
            {
                _semaphore_list_transfer_toUnlock((linker_head_t*)&pCurSemaphore->head);
            }
            else
            {
                _semaphore_list_transfer_toLock((linker_head_t*)&pCurSemaphore->head);
            }

            break;
        }

        pCurSemaphore++;
        id = _impl_kernal_member_containerAddress_toUnifiedid((u32_t)pCurSemaphore);
    } while ((u32_t)pCurSemaphore < (u32_t)_impl_kernal_member_id_toContainerEndAddress(KERNAL_MEMBER_SEMAPHORE));

    id = ((!_semaphore_id_isInvalid(id)) ? (id) : (OS_INVALID_ID));

    EXIT_CRITICAL_SECTION();

    return id;
}

/**
 * @brief The privilege routine running at handle mode.
 *
 * The privilege routine running at handle mode.
 *
 * @param args_0 The function argument 1.
 * @param args_1 The function argument 2.
 * @param args_2 The function argument 3.
 * @param args_3 The function argument 4.
 *
 * @retval VALUE The result of privilege routine.
 */
static u32_t _semaphore_take_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    os_id_t id = (os_id_t)pArgs[0].u32_val;
    u32_t timeout_ms = (u32_t)pArgs[1].u32_val;

    u32p_t postcode = PC_SC_SUCCESS;
    thread_context_t *pCurThread = (thread_context_t *)_impl_kernal_thread_runContextGet();
    semaphore_context_t *pCurSemaphore = (semaphore_context_t *)_semaphore_object_contextGet(id);

    if (pCurSemaphore->head.linker.pList == _semaphore_list_lockingHeadGet())
    {
        /* No availabe count */
        postcode = _impl_kernal_thread_exit_trigger(pCurThread->head.id, id, _semaphore_list_blockingHeadGet(id), timeout_ms, _semaphore_callback_fromTimeOut);

        if (PC_IOK(postcode))
        {
            postcode = PC_SC_UNAVAILABLE;
        }
    }
    else
    {
        /* The semaphore has available count */
        pCurSemaphore->availableCount--;

        /* Check if we can take the next acquired thread into locking */
        if (pCurSemaphore->availableCount < pCurSemaphore->limitationCount)
        {
            _semaphore_list_transfer_toLock((linker_head_t*)&pCurSemaphore->head);
        }
        postcode = PC_SC_AVAILABLE;
    }

    EXIT_CRITICAL_SECTION();

    return postcode;
}

/**
 * @brief The privilege routine running at handle mode.
 *
 * The privilege routine running at handle mode.
 *
 * @param args_0 The function argument 1.
 * @param args_1 The function argument 2.
 * @param args_2 The function argument 3.
 * @param args_3 The function argument 4.
 *
 * @retval VALUE The result of privilege routine.
 */
static u32_t _semaphore_give_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    os_id_t id = (os_id_t)pArgs[0].u32_val;

    u32p_t postcode = PC_SC_SUCCESS;
    semaphore_context_t *pCurSemaphore = (semaphore_context_t *)_semaphore_object_contextGet(id);
    thread_context_t *pSemaphoreHighestBlockingThread = (thread_context_t *)_semaphore_linker_head_fromBlocking(id);

    pCurSemaphore->availableCount++;

    if (pCurSemaphore->availableCount >= _SEMAPHORE_AVAILABLE_COUNT_MAXIMUM)
    {
        postcode = _PC_CMPT_FAILED;
    }

    if (pSemaphoreHighestBlockingThread)
    {
        postcode = _impl_kernal_thread_entry_trigger(pSemaphoreHighestBlockingThread->head.id, id, PC_SC_SUCCESS, _semaphore_schedule);
    }

    EXIT_CRITICAL_SECTION();

    return postcode;
}

/**
 * @brief The privilege routine running at handle mode.
 *
 * The privilege routine running at handle mode.
 *
 * @param args_0 The function argument 1.
 * @param args_1 The function argument 2.
 * @param args_2 The function argument 3.
 * @param args_3 The function argument 4.
 *
 * @retval VALUE The result of privilege routine.
 */
static u32_t _semaphore_flush_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    os_id_t id = (os_id_t)pArgs[0].u32_val;

    u32p_t postcode = PC_SC_SUCCESS;
    semaphore_context_t *pCurSemaphore = (semaphore_context_t *)_semaphore_object_contextGet(id);
    list_iterator_t it = {0u};
    list_iterator_init(&it, _semaphore_list_blockingHeadGet(id));
    thread_context_t *pCurThread = (thread_context_t *)list_iterator_next(&it);
    while (pCurThread)
    {
        pCurSemaphore->availableCount++;
        postcode = _impl_kernal_thread_entry_trigger(pCurThread->head.id, id, PC_SC_SUCCESS, _semaphore_schedule);
        if (PC_IER(postcode))
        {
            break;
        }
        pCurThread = (thread_context_t *)list_iterator_next(&it);
    }

    EXIT_CRITICAL_SECTION();

    return postcode;
}


/**
 * @brief Semaphore invoke back handle routine.
 *
 * Semaphore wakeup handle routine.
 *
 * @param pWakeupThread The thread pointer.
 *
 * @retval TRUE It's for semaphore object.
 * @retval FALSE It's not for semaphore object.
 */
static void _semaphore_schedule(os_id_t id)
{
    thread_context_t *pEntryThread = (thread_context_t*)(_impl_kernal_member_unified_id_toContainerAddress(id));

    if (_impl_kernal_member_unified_id_toId(pEntryThread->schedule.hold) == KERNAL_MEMBER_SEMAPHORE)
    {
        semaphore_context_t *pCurSemaphore = (semaphore_context_t *)_semaphore_object_contextGet(pEntryThread->schedule.hold);

        thread_entry_t *pEntry = &pEntryThread->schedule.entry;

        if ((pEntry->result == PC_SC_SUCCESS) || (pEntry->result == PC_SC_TIMEOUT))
        {
            b_t isAvail = FALSE;

            // Release function doesn't kill the timer node from waiting list
            if (!_impl_timer_status_isBusy(_impl_kernal_member_unified_id_threadToTimer(pEntryThread->head.id)))
            {
                if (_impl_kernal_member_unified_id_toId(pEntry->release) == KERNAL_MEMBER_TIMER_INTERNAL)
                {
                    pEntry->result = PC_SC_TIMEOUT;
                }
                else
                {
                    isAvail = true;
                }
            }
            else if (_impl_kernal_member_unified_id_toId(pEntry->release) == KERNAL_MEMBER_SEMAPHORE)
            {
                _impl_timer_stop(_impl_kernal_member_unified_id_threadToTimer(pEntryThread->head.id));
                isAvail = true;
            }
            else
            {
                pEntry->result = _PC_CMPT_FAILED;
            }

            if (isAvail)
            {
                pEntry->result  = PC_SC_SUCCESS;
                /* If the PC arrive, the semaphore will be available and can be acquired */
                pCurSemaphore->availableCount--; // The semaphore has available count
            }

            /* Check if we can take the next acquired thread into locking */
            if (pCurSemaphore->availableCount < pCurSemaphore->limitationCount)
            {
                _semaphore_list_transfer_toLock((linker_head_t*)&pCurSemaphore->head);
            }
            else
            {
                _semaphore_list_transfer_toUnlock((linker_head_t*)&pCurSemaphore->head);
            }
        }
    }
    else
    {
        pEntryThread->schedule.entry.result = _PC_CMPT_FAILED;
    }
}

#ifdef __cplusplus
}
#endif
