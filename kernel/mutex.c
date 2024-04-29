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
#define _PC_CMPT_FAILED PC_FAILED(PC_CMPT_MUTEX_4)

/**
 * @brief Get the mutex context based on provided unique id.
 *
 * @param id The mutex unique id.
 *
 * @return The pointer of the current unique id timer context.
 */
static mutex_context_t *_mutex_object_contextGet(os_id_t id)
{
    return (mutex_context_t *)(kernel_member_unified_id_toContainerAddress(id));
}

/**
 * @brief Get the locking mutex list head.
 *
 * @return The value of the locking list head.
 */
static list_t *_mutex_list_lockingHeadGet(void)
{
    return (list_t *)kernel_member_list_get(KERNEL_MEMBER_MUTEX, KERNEL_MEMBER_LIST_MUTEX_LOCK);
}

/**
 * @brief Get the unlocking mutex list head.
 *
 * @return The value of the unlocking list head.
 */
static list_t *_mutex_list_unlockingHeadGet(void)
{
    return (list_t *)kernel_member_list_get(KERNEL_MEMBER_MUTEX, KERNEL_MEMBER_LIST_MUTEX_UNLOCK);
}

/**
 * @brief Get the mutex blocking thread list head address.
 *
 * @return The blocking thread list head address.
 */
static list_t *_mutex_list_blockingHeadGet(os_id_t id)
{
    mutex_context_t *pCurMutex = _mutex_object_contextGet(id);

    return (list_t *)((pCurMutex) ? (&pCurMutex->blockingThreadHead) : (NULL));
}

/**
 * @brief Push one mutex context into lock list.
 *
 * @param pCurHead The pointer of the mutex linker head.
 */
static void _mutex_list_transfer_toLock(linker_head_t *pCurHead)
{
    ENTER_CRITICAL_SECTION();

    list_t *pToLockingList = (list_t *)_mutex_list_lockingHeadGet();
    linker_list_transaction_common(&pCurHead->linker, pToLockingList, LIST_TAIL);

    EXIT_CRITICAL_SECTION();
}

/**
 * @brief Push one mutex context into unlock list.
 *
 * @param pCurHead The pointer of the mutex linker head.
 */
static void _mutex_list_transfer_toUnlock(linker_head_t *pCurHead)
{
    ENTER_CRITICAL_SECTION();

    list_t *pToUnlockingList = (list_t *)_mutex_list_unlockingHeadGet();
    linker_list_transaction_common(&pCurHead->linker, pToUnlockingList, LIST_TAIL);

    EXIT_CRITICAL_SECTION();
}

/**
 * @brief Pick up a highest priority thread from the mutex blocking list.
 *
 * @param id The mutex context id
 *
 * @return The highest thread head.
 */
static linker_head_t *_mutex_linker_head_fromBlocking(os_id_t id)
{
    ENTER_CRITICAL_SECTION();

    list_t *pListBlocking = (list_t *)_mutex_list_blockingHeadGet(id);

    EXIT_CRITICAL_SECTION();
    return (linker_head_t *)(pListBlocking->pHead);
}

/**
 * @brief Check if the mutex unique id if is's invalid.
 *
 * @param id The provided unique id.
 *
 * @return The true is invalid, otherwise is valid.
 */
static b_t _mutex_id_isInvalid(u32_t id)
{
    return kernel_member_unified_id_isInvalid(KERNEL_MEMBER_MUTEX, id);
}

/**
 * @brief Check if the mutex object if is's initialized.
 *
 * @param id The provided unique id.
 *
 * @return The true is initialized, otherwise is uninitialized.
 */
static b_t _mutex_object_isInit(i32_t id)
{
    mutex_context_t *pCurMutex = _mutex_object_contextGet(id);

    return ((pCurMutex) ? (((pCurMutex->head.linker.pList) ? (TRUE) : (FALSE))) : FALSE);
}

/**
 * @brief It's sub-routine running at privilege mode.
 *
 * @param pArgs The function argument packages.
 *
 * @return The result of privilege routine.
 */
static u32_t _mutex_init_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    const char_t *pName = (const char_t *)(pArgs[0].pch_val);
    u32_t endAddr = 0u;
    mutex_context_t *pCurMutex = NULL;

    pCurMutex = (mutex_context_t *)kernel_member_id_toContainerStartAddress(KERNEL_MEMBER_MUTEX);
    endAddr = (u32_t)kernel_member_id_toContainerEndAddress(KERNEL_MEMBER_MUTEX);
    do {
        os_id_t id = kernel_member_containerAddress_toUnifiedid((u32_t)pCurMutex);
        if (_mutex_id_isInvalid(id)) {
            break;
        }

        if (_mutex_object_isInit(id)) {
            continue;
        }

        _memset((char_t *)pCurMutex, 0x0u, sizeof(mutex_context_t));
        pCurMutex->head.id = id;
        pCurMutex->head.pName = pName;

        pCurMutex->holdThreadId = OS_INVALID_ID;
        pCurMutex->originalPriority.level = OS_PRIORITY_INVALID;

        _mutex_list_transfer_toUnlock((linker_head_t *)&pCurMutex->head);

        EXIT_CRITICAL_SECTION();
        return id;
    } while ((u32_t)++pCurMutex < endAddr);

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
static u32_t _mutex_lock_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    os_id_t id = (os_id_t)pArgs[0].u32_val;
    mutex_context_t *pCurMutex = NULL;
    thread_context_t *pCurThread = NULL;
    thread_context_t *pLockThread = NULL;
    u32p_t postcode = PC_SC_SUCCESS;

    pCurMutex = _mutex_object_contextGet(id);
    pCurThread = kernel_thread_runContextGet();
    if (pCurMutex->head.linker.pList == _mutex_list_lockingHeadGet()) {
        pLockThread = (thread_context_t *)kernel_member_unified_id_toContainerAddress(pCurMutex->holdThreadId);

        if (pCurThread->priority.level < pLockThread->priority.level) {
            pLockThread->priority = pCurThread->priority;
        }
        postcode = kernel_thread_exit_trigger(pCurThread->head.id, id, _mutex_list_blockingHeadGet(id), 0u, NULL);

        EXIT_CRITICAL_SECTION();
        return postcode;
    }

    /* Highest priority inheritance */
    pCurMutex->holdThreadId = pCurThread->head.id;
    pCurMutex->originalPriority = pCurThread->priority;
    _mutex_list_transfer_toLock((linker_head_t *)&pCurMutex->head);

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
static u32_t _mutex_unlock_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    os_id_t id = (os_id_t)pArgs[0].u32_val;
    mutex_context_t *pCurMutex = NULL;
    thread_context_t *pMutexHighestBlockingThread = NULL;
    thread_context_t *pLockThread = NULL;
    u32p_t postcode = PC_SC_SUCCESS;

    pCurMutex = _mutex_object_contextGet(id);
    pMutexHighestBlockingThread = (thread_context_t *)_mutex_linker_head_fromBlocking(id);
    pLockThread = (thread_context_t *)kernel_member_unified_id_toContainerAddress(pCurMutex->holdThreadId);
    /* priority recovery */
    pLockThread->priority = pCurMutex->originalPriority;
    if (!pMutexHighestBlockingThread) {
        // no blocking thread
        pCurMutex->originalPriority.level = OS_PRIORITY_INVALID;
        pCurMutex->holdThreadId = OS_INVALID_ID;

        _mutex_list_transfer_toUnlock((linker_head_t *)&pCurMutex->head);
    } else {
        /* The next thread take the ticket */
        pCurMutex->holdThreadId = pMutexHighestBlockingThread->head.id;
        pCurMutex->originalPriority = pMutexHighestBlockingThread->priority;
        postcode = kernel_thread_entry_trigger(pMutexHighestBlockingThread->head.id, id, PC_SC_SUCCESS, NULL);
    }

    EXIT_CRITICAL_SECTION();
    return postcode;
}

/**
 * @brief Convert the internal os id to kernel member number.
 *
 * @param id The provided unique id.
 *
 * @return The value of member number.
 */
u32_t _impl_mutex_os_id_to_number(os_id_t id)
{
    if (_mutex_id_isInvalid(id)) {
        return 0u;
    }

    return (u32_t)((id - kernel_member_id_toUnifiedIdStart(KERNEL_MEMBER_MUTEX)) / sizeof(mutex_context_t));
}

/**
 * @brief Initialize a new mutex.
 *
 * @param pName The mutex name.
 *
 * @return The mutex unique id.
 */
os_id_t _impl_mutex_init(const char_t *pName)
{
    arguments_t arguments[] = {
        [0] = {.pch_val = (const char_t *)pName},
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
u32p_t _impl_mutex_lock(os_id_t id)
{
    if (_mutex_id_isInvalid(id)) {
        return _PC_CMPT_FAILED;
    }

    if (!_mutex_object_isInit(id)) {
        return _PC_CMPT_FAILED;
    }

    if (!kernel_isInThreadMode()) {
        return _PC_CMPT_FAILED;
    }

    arguments_t arguments[] = {
        [0] = {.u32_val = (u32_t)id},
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
u32p_t _impl_mutex_unlock(os_id_t id)
{
    if (_mutex_id_isInvalid(id)) {
        return _PC_CMPT_FAILED;
    }

    if (!_mutex_object_isInit(id)) {
        return _PC_CMPT_FAILED;
    }

    arguments_t arguments[] = {
        [0] = {.u32_val = (u32_t)id},
    };

    return kernel_privilege_invoke((const void *)_mutex_unlock_privilege_routine, arguments);
}

/**
 * @brief Get mutex snapshot informations.
 *
 * @param instance The mutex instance number.
 * @param pMsgs The kernel snapshot information pointer.
 *
 * @return TRUE: Operation pass, FALSE: Operation failed.
 */
b_t mutex_snapshot(u32_t instance, kernel_snapshot_t *pMsgs)
{
#if defined KTRACE
    mutex_context_t *pCurMutex = NULL;
    u32_t offset = 0u;
    os_id_t id = OS_INVALID_ID;

    ENTER_CRITICAL_SECTION();

    offset = sizeof(mutex_context_t) * instance;
    pCurMutex = (mutex_context_t *)(kernel_member_id_toContainerStartAddress(KERNEL_MEMBER_MUTEX) + offset);
    id = kernel_member_containerAddress_toUnifiedid((u32_t)pCurMutex);
    _memset((u8_t *)pMsgs, 0x0u, sizeof(kernel_snapshot_t));

    if (_mutex_id_isInvalid(id)) {
        EXIT_CRITICAL_SECTION();
        return FALSE;
    }

    if (pCurMutex->head.linker.pList == _mutex_list_lockingHeadGet()) {
        pMsgs->pState = "lock";
    } else if (pCurMutex->head.linker.pList == _mutex_list_unlockingHeadGet()) {
        pMsgs->pState = "unlock";
    } else if (pCurMutex->head.linker.pList) {
        pMsgs->pState = "*";
    } else {
        pMsgs->pState = "unused";

        EXIT_CRITICAL_SECTION();
        return FALSE;
    }

    pMsgs->id = pCurMutex->head.id;
    pMsgs->pName = pCurMutex->head.pName;

    pMsgs->mutex.holdThreadId = pCurMutex->holdThreadId;
    pMsgs->mutex.originalPriority = pCurMutex->originalPriority.level;
    pMsgs->mutex.wait_list = pCurMutex->blockingThreadHead;

    EXIT_CRITICAL_SECTION();
    return TRUE;
#else
    return FALSE;
#endif
}

#ifdef __cplusplus
}
#endif
