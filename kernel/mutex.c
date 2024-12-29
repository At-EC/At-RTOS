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
#define PC_EOR PC_IER(PC_OS_CMPT_MUTEX_5)

/**
 * @brief Get the mutex context based on provided unique id.
 *
 * @param id The mutex unique id.
 *
 * @return The pointer of the current unique id timer context.
 */
static mutex_context_t *_mutex_context_get(os_id_t id)
{
    return (mutex_context_t *)(kernel_member_unified_id_toContainerAddress(id));
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
static b_t _mutex_id_isInit(i32_t id)
{
    mutex_context_t *pCurMutex = _mutex_context_get(id);

    return ((pCurMutex) ? (((pCurMutex->head.cs) ? (TRUE) : (FALSE))) : FALSE);
}

/**
 * @brief Get the mutex blocking thread list head address.
 *
 * @return The blocking thread list head address.
 */
static list_t *_mutex_list_blockingHeadGet(os_id_t id)
{
    mutex_context_t *pCurMutex = _mutex_context_get(id);

    return (list_t *)((pCurMutex) ? (&pCurMutex->blockingThreadHead) : (NULL));
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

        if (_mutex_id_isInit(id)) {
            continue;
        }

        os_memset((char_t *)pCurMutex, 0x0u, sizeof(mutex_context_t));
        pCurMutex->head.cs = CS_INITED;
        pCurMutex->head.pName = pName;

        pCurMutex->locked = false;
        pCurMutex->pHoldThread = NULL;
        pCurMutex->originalPriority.level = OS_PRIOTITY_INVALID_LEVEL;

        EXIT_CRITICAL_SECTION();
        return id;
    } while ((u32_t)++pCurMutex < endAddr);

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
static i32p_t _mutex_lock_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    os_id_t id = (os_id_t)pArgs[0].u32_val;
    mutex_context_t *pCurMutex = NULL;
    thread_context_t *pCurThread = NULL;
    i32p_t postcode = 0;

    pCurMutex = _mutex_context_get(id);
    pCurThread = kernel_thread_runContextGet();
    if (pCurMutex->locked == true) {
        thread_context_t *pLockThread = pCurMutex->pHoldThread;
        if (pCurThread->priority.level < pLockThread->priority.level) {
            pLockThread->priority = pCurThread->priority;
        }
        postcode = kernel_thread_exit_trigger(pCurThread, pCurMutex, _mutex_list_blockingHeadGet(id), 0u);

        EXIT_CRITICAL_SECTION();
        return postcode;
    }

    /* Highest priority inheritance */
    pCurMutex->pHoldThread = pCurThread;
    pCurMutex->originalPriority = pCurThread->priority;
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
static i32p_t _mutex_unlock_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    os_id_t id = (os_id_t)pArgs[0].u32_val;
    mutex_context_t *pCurMutex = NULL;
    thread_context_t *pMutexHighestBlockingThread = NULL;
    i32p_t postcode = 0;

    pCurMutex = _mutex_context_get(id);
    pMutexHighestBlockingThread = (thread_context_t *)_mutex_linker_head_fromBlocking(id);
    thread_context_t *pLockThread = pCurMutex->pHoldThread;
    /* priority recovery */
    pLockThread->priority = pCurMutex->originalPriority;
    if (!pMutexHighestBlockingThread) {
        // no blocking thread
        pCurMutex->originalPriority.level = OS_PRIOTITY_INVALID_LEVEL;
        pCurMutex->pHoldThread = NULL;
        pCurMutex->locked = false;
    } else {
        /* The next thread take the ticket */
        pCurMutex->pHoldThread = pMutexHighestBlockingThread;
        pCurMutex->originalPriority = pMutexHighestBlockingThread->priority;
        postcode = kernel_thread_entry_trigger(pMutexHighestBlockingThread, 0, NULL);
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
i32p_t _impl_mutex_lock(os_id_t id)
{
    if (_mutex_id_isInvalid(id)) {
        return PC_EOR;
    }

    if (!_mutex_id_isInit(id)) {
        return PC_EOR;
    }

    if (!kernel_isInThreadMode()) {
        return PC_EOR;
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
i32p_t _impl_mutex_unlock(os_id_t id)
{
    if (_mutex_id_isInvalid(id)) {
        return PC_EOR;
    }

    if (!_mutex_id_isInit(id)) {
        return PC_EOR;
    }

    arguments_t arguments[] = {
        [0] = {.u32_val = (u32_t)id},
    };

    return kernel_privilege_invoke((const void *)_mutex_unlock_privilege_routine, arguments);
}

#ifdef __cplusplus
}
#endif
