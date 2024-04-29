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
#define _PC_CMPT_FAILED PC_FAILED(PC_CMPT_POOL_8)

/**
 * @brief Get the pool context based on provided unique id.
 *
 * @param id The timer unique id.
 *
 * @return The pointer of the current unique id timer context.
 */
static pool_context_t *_pool_object_contextGet(os_id_t id)
{
    return (pool_context_t *)(kernel_member_unified_id_toContainerAddress(id));
}

/**
 * @brief Get the init pool list head.
 *
 * @return The value of the init list head.
 */
static list_t *_pool_list_initHeadGet(void)
{
    return (list_t *)kernel_member_list_get(KERNEL_MEMBER_POOL, KERNEL_MEMBER_LIST_POOL_INIT);
}

/**
 * @brief Pick up a highest priority thread that blocking by the pool pending list.
 *
 * @param The pool unique id.
 *
 * @return The highest blocking thread head.
 */
static list_t *_pool_list_blockingHeadGet(os_id_t id)
{
    pool_context_t *pCurPool = _pool_object_contextGet(id);

    return (list_t *)((pCurPool) ? (&pCurPool->blockingThreadHead) : (NULL));
}

/**
 * @brief Push one pool context into init list.
 *
 * @param pCurHead The pointer of the pool linker head.
 */
static void _pool_list_transferToInit(linker_head_t *pCurHead)
{
    ENTER_CRITICAL_SECTION();

    list_t *pToInitList = (list_t *)_pool_list_initHeadGet();
    linker_list_transaction_common(&pCurHead->linker, pToInitList, LIST_TAIL);

    EXIT_CRITICAL_SECTION();
}

/**
 * @brief Check if the pool unique id if is's invalid.
 *
 * @param id The provided unique id.
 *
 * @return The true is invalid, otherwise is valid.
 */
static b_t _pool_id_isInvalid(u32_t id)
{
    return kernel_member_unified_id_isInvalid(KERNEL_MEMBER_POOL, id);
}

/**
 * @brief Check if the pool object if is's initialized.
 *
 * @param id The provided unique id.
 *
 * @return The true is initialized, otherwise is uninitialized.
 */
static b_t _pool_object_isInit(i32_t id)
{
    pool_context_t *pCurPool = _pool_object_contextGet(id);

    return ((pCurPool) ? (((pCurPool->head.linker.pList) ? (TRUE) : (FALSE))) : FALSE);
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
    u32_t free = pCurPool->elementFreeBits;
    u32_t num = pCurPool->elementNumber;
    void *pMemTake = NULL;
    u8_t i = 0u;

    do {
        if (!(free & SBIT(i))) {
            continue;
        }

        pMemTake = (void *)((u32_t)((i * pCurPool->elementLength) + (u32_t)pCurPool->pMemAddress));
        _memset((char_t *)pMemTake, 0x0u, pCurPool->elementLength);
        pCurPool->elementFreeBits &= ~SBIT(i);
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
    u32_t free = pCurPool->elementFreeBits;
    u32_t num = pCurPool->elementNumber;
    void *pMemTake = NULL;
    u8_t i = 0u;

    do {
        if (free & SBIT(i)) {
            continue;
        }

        pMemTake = (void *)((u32_t)((i * pCurPool->elementLength) + (u32_t)pCurPool->pMemAddress));
        if (pMemTake == pUserMem) {
            _memset((char_t *)pMemTake, 0x0u, pCurPool->elementLength);
            pCurPool->elementFreeBits |= SBIT(i);
            break;
        }
    } while (i++ < num);

    return (pMemTake == pUserMem) ? (TRUE) : (FALSE);
}

/**
 * @brief The pool schedule routine execute the the pendsv context.
 *
 * @param id The unique id of the entry thread.
 */
static void _pool_schedule(os_id_t id)
{
    thread_context_t *pEntryThread = (thread_context_t *)(kernel_member_unified_id_toContainerAddress(id));
    thread_entry_t *pEntry = NULL;
    pool_context_t *pCurPool = NULL;
    b_t isAvail = FALSE;

    pCurPool = _pool_object_contextGet(pEntryThread->schedule.hold);
    if (kernel_member_unified_id_toId(pEntryThread->schedule.hold) != KERNEL_MEMBER_POOL) {
        pEntryThread->schedule.entry.result = _PC_CMPT_FAILED;
        return;
    }

    if ((pEntryThread->schedule.entry.result != PC_SC_SUCCESS) && (pEntryThread->schedule.entry.result != PC_SC_TIMEOUT)) {
        return;
    }

    pEntry = &pEntryThread->schedule.entry;
    if (!timer_busy(kernel_member_unified_id_threadToTimer(pEntryThread->head.id))) {
        if (kernel_member_unified_id_toId(pEntry->release) == KERNEL_MEMBER_TIMER_INTERNAL) {
            pEntry->result = PC_SC_TIMEOUT;
        } else {
            isAvail = true;
        }
    } else if (kernel_member_unified_id_toId(pEntry->release) == KERNEL_MEMBER_POOL) {
        timer_stop_for_thread(kernel_member_unified_id_threadToTimer(pEntryThread->head.id));
        isAvail = true;
    } else {
        pEntry->result = _PC_CMPT_FAILED;
    }

    if (isAvail) {
        *pEntryThread->pool.ppUserMemAddress = _mem_take(pCurPool);
        if (!*pEntryThread->pool.ppUserMemAddress) {
            pEntry->result = _PC_CMPT_FAILED;
        } else {
            pEntry->result = PC_SC_SUCCESS;
        }
    }
}

/**
 * @brief The pool timeout callback fucntion.
 *
 * @param id The pool unique id.
 */
static void _pool_callback_fromTimeOut(os_id_t id)
{
    kernel_thread_entry_trigger(kernel_member_unified_id_timerToThread(id), id, PC_SC_TIMEOUT, _pool_schedule);
}

/**
 * @brief It's sub-routine running at privilege mode.
 *
 * @param pArgs The function argument packages.
 *
 * @return The result of privilege routine.
 */
static u32_t _pool_init_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    const void *pMemAddr = (const void *)(pArgs[0].ptr_val);
    u16_t elementLen = (u16_t)(pArgs[1].u16_val);
    u16_t elementNum = (u16_t)(pArgs[2].u16_val);
    const char_t *pName = (const char_t *)(pArgs[3].pch_val);
    u32_t endAddr = 0u;
    pool_context_t *pCurPool = NULL;

    pCurPool = (pool_context_t *)kernel_member_id_toContainerStartAddress(KERNEL_MEMBER_POOL);
    endAddr = (u32_t)kernel_member_id_toContainerEndAddress(KERNEL_MEMBER_POOL);
    do {
        os_id_t id = kernel_member_containerAddress_toUnifiedid((u32_t)pCurPool);
        if (_pool_id_isInvalid(id)) {
            break;
        }

        if (_pool_object_isInit(id)) {
            continue;
        }
        _memset((char_t *)pCurPool, 0x0u, sizeof(pool_context_t));
        pCurPool->head.id = id;
        pCurPool->head.pName = pName;

        pCurPool->pMemAddress = pMemAddr;
        pCurPool->elementLength = elementLen;
        pCurPool->elementNumber = elementNum;
        pCurPool->elementFreeBits = SBITS(0u, (elementNum - 1u));

        _pool_list_transferToInit((linker_head_t *)&pCurPool->head);

        EXIT_CRITICAL_SECTION();
        return id;
    } while ((u32_t)++pCurPool < endAddr);

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
static u32_t _pool_take_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    os_id_t id = (os_id_t)pArgs[0].u32_val;
    void **ppUserBuffer = (void **)pArgs[1].pv_val;
    u16_t bufferSize = (u16_t)pArgs[2].u16_val;
    u32_t timeout_ms = (u32_t)pArgs[3].u32_val;
    pool_context_t *pCurPool = NULL;
    thread_context_t *pCurThread = NULL;
    u32p_t postcode = PC_SC_SUCCESS;

    pCurPool = _pool_object_contextGet(id);
    pCurThread = kernel_thread_runContextGet();

    if (bufferSize > pCurPool->elementLength) {
        EXIT_CRITICAL_SECTION();
        return _PC_CMPT_FAILED;
    }

    if (!pCurPool->elementFreeBits) {
        if ((timeout_ms == OS_TIME_NOWAIT_VAL) && (!kernel_isInThreadMode())) {
            EXIT_CRITICAL_SECTION();
            return _PC_CMPT_FAILED;
        }

        _memset((u8_t *)&pCurThread->pool, 0x0u, sizeof(action_pool_t));

        pCurThread->pool.ppUserMemAddress = ppUserBuffer;
        postcode =
            kernel_thread_exit_trigger(pCurThread->head.id, id, _pool_list_blockingHeadGet(id), timeout_ms, _pool_callback_fromTimeOut);

        if (PC_IOK(postcode)) {
            postcode = PC_SC_UNAVAILABLE;
        }
    } else {
        *ppUserBuffer = _mem_take(pCurPool);

        if (!*ppUserBuffer) {
            postcode = _PC_CMPT_FAILED;
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
static u32_t _pool_release_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    os_id_t id = (os_id_t)pArgs[0].u32_val;
    void **ppUserBuffer = (void **)pArgs[1].ptr_val;
    pool_context_t *pCurPool = NULL;
    thread_context_t *pCurThread = NULL;
    u32p_t postcode = PC_SC_SUCCESS;

    pCurPool = _pool_object_contextGet(id);
    pCurThread = (thread_context_t *)kernel_thread_runContextGet();

    if (!_mem_release(pCurPool, *ppUserBuffer)) {
        EXIT_CRITICAL_SECTION();
        return _PC_CMPT_FAILED;
    }

    *ppUserBuffer = NULL;

    /* Try to wakeup a blocking thread */
    list_iterator_t it = {0u};
    list_iterator_init(&it, _pool_list_blockingHeadGet(id));
    pCurThread = (thread_context_t *)list_iterator_next(&it);
    if (pCurThread) {
        postcode = kernel_thread_entry_trigger(pCurThread->head.id, id, PC_SC_SUCCESS, _pool_schedule);
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
u32_t _impl_pool_os_id_to_number(os_id_t id)
{
    if (_pool_id_isInvalid(id)) {
        return 0u;
    }

    return (u32_t)((id - kernel_member_id_toUnifiedIdStart(KERNEL_MEMBER_POOL)) / sizeof(pool_context_t));
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
os_id_t _impl_pool_init(const void *pMemAddr, u16_t elementLen, u16_t elementNum, const char_t *pName)
{
    if (!pMemAddr) {
        return OS_INVALID_ID;
    }

    if (!elementLen) {
        return OS_INVALID_ID;
    }

    if (!elementNum) {
        return OS_INVALID_ID;
    }

    if (elementNum > U32_B) {
        return OS_INVALID_ID;
    }

    arguments_t arguments[] = {
        [0] = {.ptr_val = (const void *)pMemAddr},
        [1] = {.u16_val = (u16_t)elementLen},
        [2] = {.u16_val = (u16_t)elementNum},
        [3] = {.pch_val = (const char_t *)pName},
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
u32p_t _impl_pool_take(os_id_t id, void **ppUserBuffer, u16_t bufferSize, u32_t timeout_ms)
{
    if (_pool_id_isInvalid(id)) {
        return _PC_CMPT_FAILED;
    }

    if (!_pool_object_isInit(id)) {
        return _PC_CMPT_FAILED;
    }

    if (!kernel_isInThreadMode()) {
        if (timeout_ms != OS_TIME_NOWAIT_VAL) {
            return _PC_CMPT_FAILED;
        }
    }

    arguments_t arguments[] = {
        [0] = {.u32_val = (u32_t)id},
        [1] = {.pv_val = (void **)ppUserBuffer},
        [2] = {.u16_val = (u16_t)bufferSize},
        [3] = {.u32_val = (u32_t)timeout_ms},
    };

    u32p_t postcode = kernel_privilege_invoke((const void *)_pool_take_privilege_routine, arguments);

    ENTER_CRITICAL_SECTION();
    if (postcode == PC_SC_UNAVAILABLE) {
        thread_context_t *pCurThread = (thread_context_t *)kernel_thread_runContextGet();
        postcode = (u32p_t)kernel_schedule_entry_result_take((action_schedule_t *)&pCurThread->schedule);
    }

    if (PC_IOK(postcode) && (postcode != PC_SC_TIMEOUT)) {
        postcode = PC_SC_SUCCESS;
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
u32p_t _impl_pool_release(os_id_t id, void **ppUserBuffer)
{
    if (_pool_id_isInvalid(id)) {
        return _PC_CMPT_FAILED;
    }

    if (!_pool_object_isInit(id)) {
        return _PC_CMPT_FAILED;
    }

    if (*ppUserBuffer == NULL) {
        return _PC_CMPT_FAILED;
    }

    arguments_t arguments[] = {
        [0] = {.u32_val = (u32_t)id},
        [1] = {.pv_val = (void **)ppUserBuffer},
    };

    return kernel_privilege_invoke((const void *)_pool_release_privilege_routine, arguments);
}

/**
 * @brief Get pool snapshot informations.
 *
 * @param instance The pool instance number.
 * @param pMsgs The kernel snapshot information pointer.
 *
 * @return TRUE: Operation pass, FALSE: Operation failed.
 */
b_t pool_snapshot(u32_t instance, kernel_snapshot_t *pMsgs)
{
#if defined KTRACE
    pool_context_t *pCurPool = NULL;
    u32_t offset = 0u;
    os_id_t id = OS_INVALID_ID;

    ENTER_CRITICAL_SECTION();

    offset = sizeof(pool_context_t) * instance;
    pCurPool = (pool_context_t *)(kernel_member_id_toContainerStartAddress(KERNEL_MEMBER_POOL) + offset);
    id = kernel_member_containerAddress_toUnifiedid((u32_t)pCurPool);
    _memset((u8_t *)pMsgs, 0x0u, sizeof(kernel_snapshot_t));

    if (_pool_id_isInvalid(id)) {
        EXIT_CRITICAL_SECTION();
        return FALSE;
    }

    if (pCurPool->head.linker.pList == _pool_list_initHeadGet()) {
        pMsgs->pState = "init";
    } else if (pCurPool->head.linker.pList) {
        pMsgs->pState = "*";
    } else {
        pMsgs->pState = "unused";

        EXIT_CRITICAL_SECTION();
        return FALSE;
    }

    pMsgs->id = pCurPool->head.id;
    pMsgs->pName = pCurPool->head.pName;

    pMsgs->pool.free = pCurPool->elementFreeBits;
    pMsgs->pool.wait_list = pCurPool->blockingThreadHead;

    EXIT_CRITICAL_SECTION();
    return TRUE;
#else
    return FALSE;
#endif
}

#ifdef __cplusplus
}
#endif
