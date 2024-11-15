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
#define _PCER                  PC_IER(PC_OS_CMPT_QUEUE_6)
#define _QUEUE_WAKEUP_SENDER   10
#define _QUEUE_WAKEUP_RECEIVER 11

/**
 * @brief Get the queue context based on provided unique id.
 *
 * @param id The timer unique id.
 *
 * @return The pointer of the current unique id timer context.
 */
static queue_context_t *_queue_object_contextGet(os_id_t id)
{
    return (queue_context_t *)(kernel_member_unified_id_toContainerAddress(id));
}

/**
 * @brief Get the init queue list head.
 *
 * @return The value of the init list head.
 */
static list_t *_queue_list_initHeadGet(void)
{
    return (list_t *)kernel_member_list_get(KERNEL_MEMBER_QUEUE, KERNEL_MEMBER_LIST_QUEUE_INIT);
}

/**
 * @brief Get the queue blocking thread list head address.
 *
 * @return The blocking thread list head address.
 */
static list_t *_queue_list_inBlockingHeadGet(os_id_t id)
{
    queue_context_t *pCurQueue = _queue_object_contextGet(id);
    return (list_t *)(&pCurQueue->inBlockingThreadListHead);
}

/**
 * @brief Get the queue blocking thread list head address.
 *
 * @return The blocking thread list head address.
 */
static list_t *_queue_list_OutBlockingHeadGet(os_id_t id)
{
    queue_context_t *pCurQueue = _queue_object_contextGet(id);
    return (list_t *)(&pCurQueue->outBlockingThreadListHead);
}

/**
 * @brief Push one queue context into init list.
 *
 * @param pCurHead The pointer of the queue linker head.
 */
static void _queue_list_transferToInit(linker_head_t *pCurHead)
{
    ENTER_CRITICAL_SECTION();

    list_t *pToInitList = (list_t *)_queue_list_initHeadGet();
    linker_list_transaction_common(&pCurHead->linker, pToInitList, LIST_TAIL);

    EXIT_CRITICAL_SECTION();
}

/**
 * @brief Check if the queue unique id if is's invalid.
 *
 * @param id The provided unique id.
 *
 * @return The true is invalid, otherwise is valid.
 */
static b_t _queue_id_isInvalid(u32_t id)
{
    return kernel_member_unified_id_isInvalid(KERNEL_MEMBER_QUEUE, id);
}

/**
 * @brief Check if the queue object if is's initialized.
 *
 * @param id The provided unique id.
 *
 * @return The true is initialized, otherwise is uninitialized.
 */
static b_t _queue_object_isInit(i32_t id)
{
    queue_context_t *pCurQueue = _queue_object_contextGet(id);

    return ((pCurQueue) ? (((pCurQueue->head.linker.pList) ? (TRUE) : (FALSE))) : FALSE);
}

/**
 * @brief Send a queue message.
 *
 * @param pCurQueue The current queue context.
 * @param pUserBuffer The pointer of user's message buffer.
 * @param userSize The size of user's message buffer.
 */
static void _message_send(queue_context_t *pCurQueue, const u8_t *pUserBuffer, u16_t userSize)
{
    u8_t *pInBuffer = NULL;

    pInBuffer = (u8_t *)((u32_t)((pCurQueue->leftPosition * pCurQueue->elementLength) + (u32_t)pCurQueue->pQueueBufferAddress));
    os_memset((char_t *)pInBuffer, 0x0u, pCurQueue->elementLength);
    os_memcpy((char_t *)pInBuffer, (const char_t *)pUserBuffer, userSize);

    // Calculate the next left position
    // Receive empty: right + 1 == left
    // Send full: left + 1 == right
    pCurQueue->leftPosition = (pCurQueue->leftPosition + 1u) % pCurQueue->elementNumber;
    pCurQueue->cacheSize++;
}

/**
 * @brief Send a queue message to front.
 *
 * @param pCurQueue The current queue context.
 * @param pUserBuffer The pointer of user's message buffer.
 * @param userSize The size of user's message buffer.
 */
static void _message_send_front(queue_context_t *pCurQueue, const u8_t *pUserBuffer, u16_t userSize)
{
    u8_t *pInBuffer = NULL;

    if (pCurQueue->rightPosition) {
        pCurQueue->rightPosition--;
    } else {
        pCurQueue->rightPosition = pCurQueue->elementNumber - 1;
    }
    pCurQueue->cacheSize++;

    pInBuffer = (u8_t *)((u32_t)((pCurQueue->rightPosition * pCurQueue->elementLength) + (u32_t)pCurQueue->pQueueBufferAddress));
    os_memset((char_t *)pInBuffer, 0x0u, pCurQueue->elementLength);
    os_memcpy((char_t *)pInBuffer, (const char_t *)pUserBuffer, userSize);
}

/**
 * @brief Receive a queue message.
 *
 * @param pCurQueue The current queue context.
 * @param pUserBuffer The pointer of user's message buffer.
 * @param userSize The size of user's message buffer.
 */
static void _message_receive(queue_context_t *pCurQueue, const u8_t *pUserBuffer, u16_t userSize)
{
    u8_t *pOutBuffer = NULL;

    pOutBuffer = (u8_t *)((pCurQueue->rightPosition * pCurQueue->elementLength) + (u32_t)pCurQueue->pQueueBufferAddress);
    os_memset((char_t *)pUserBuffer, 0x0u, userSize);
    os_memcpy((char_t *)pUserBuffer, (const char_t *)pOutBuffer, userSize);

    // Calculate the next right position
    // Receive empty: right + 1 == left
    // Send full: left + 1 == right
    pCurQueue->rightPosition = (pCurQueue->rightPosition + 1u) % pCurQueue->elementNumber;
    pCurQueue->cacheSize--;
}

/**
 * @brief Receive a queue message from back.
 *
 * @param pCurQueue The current queue context.
 * @param pUserBuffer The pointer of user's message buffer.
 * @param userSize The size of user's message buffer.
 */
static void _message_receive_behind(queue_context_t *pCurQueue, const u8_t *pUserBuffer, u16_t userSize)
{
    u8_t *pInBuffer = NULL;

    if (pCurQueue->leftPosition) {
        pCurQueue->leftPosition--;
    } else {
        pCurQueue->leftPosition = pCurQueue->elementNumber - 1;
    }
    pCurQueue->cacheSize--;

    pInBuffer = (u8_t *)((u32_t)((pCurQueue->leftPosition * pCurQueue->elementLength) + (u32_t)pCurQueue->pQueueBufferAddress));
    os_memset((char_t *)pInBuffer, 0x0u, pCurQueue->elementLength);
    os_memcpy((char_t *)pInBuffer, (const char_t *)pUserBuffer, userSize);
}

/**
 * @brief The queue schedule routine execute the the pendsv context.
 *
 * @param id The unique id of the entry thread.
 */
static void _queue_schedule(os_id_t id)
{
    thread_context_t *pEntryThread = (thread_context_t *)(kernel_member_unified_id_toContainerAddress(id));
    if (kernel_member_unified_id_toId(pEntryThread->schedule.hold) != KERNEL_MEMBER_QUEUE) {
        pEntryThread->schedule.entry.result = _PCER;
        return;
    }
    timer_stop_for_thread(kernel_member_unified_id_threadToTimer(pEntryThread->head.id));

    queue_context_t *pCurQueue = _queue_object_contextGet(pEntryThread->schedule.hold);
    queue_sch_t *pQue_sche = (queue_sch_t *)pEntryThread->schedule.pPendData;
    if (!pQue_sche) {
        return;
    }
    if (pEntryThread->schedule.entry.result == _QUEUE_WAKEUP_RECEIVER) {
        if (pQue_sche->reverse) {
            _message_receive_behind((queue_context_t *)pCurQueue, pQue_sche->pUsrBuf, pQue_sche->size);
        } else {
            _message_receive((queue_context_t *)pCurQueue, pQue_sche->pUsrBuf, pQue_sche->size);
        }
        pEntryThread->schedule.entry.result = 0;
    } else if (pEntryThread->schedule.entry.result == _QUEUE_WAKEUP_SENDER) {
        if (pQue_sche->reverse) {
            _message_send_front((queue_context_t *)pCurQueue, pQue_sche->pUsrBuf, pQue_sche->size);
        } else {
            _message_send((queue_context_t *)pCurQueue, pQue_sche->pUsrBuf, pQue_sche->size);
        }
        pEntryThread->schedule.entry.result = 0;
    }
}

/**
 * @brief It's sub-routine running at privilege mode.
 *
 * @param pArgs The function argument packages.
 *
 * @return The result of privilege routine.
 */
static u32_t _queue_init_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    const void *pQueueBufferAddr = (const void *)(pArgs[0].ptr_val);
    u16_t elementLen = (u16_t)(pArgs[1].u16_val);
    u16_t elementNum = (u16_t)(pArgs[2].u16_val);
    const char_t *pName = (const char_t *)(pArgs[3].pch_val);
    u32_t endAddr = 0u;
    queue_context_t *pCurQueue = NULL;

    pCurQueue = (queue_context_t *)kernel_member_id_toContainerStartAddress(KERNEL_MEMBER_QUEUE);
    endAddr = (u32_t)kernel_member_id_toContainerEndAddress(KERNEL_MEMBER_QUEUE);
    do {
        os_id_t id = kernel_member_containerAddress_toUnifiedid((u32_t)pCurQueue);
        if (_queue_id_isInvalid(id)) {
            break;
        }

        if (_queue_object_isInit(id)) {
            continue;
        }
        os_memset((char_t *)pCurQueue, 0x0u, sizeof(queue_context_t));
        pCurQueue->head.id = id;
        pCurQueue->head.pName = pName;

        pCurQueue->pQueueBufferAddress = pQueueBufferAddr;
        pCurQueue->elementLength = elementLen;
        pCurQueue->elementNumber = elementNum;
        pCurQueue->leftPosition = 0u;
        pCurQueue->rightPosition = 0u;
        pCurQueue->cacheSize = 0u;

        _queue_list_transferToInit((linker_head_t *)&pCurQueue->head);

        EXIT_CRITICAL_SECTION();
        return id;
    } while ((u32_t)++pCurQueue < endAddr);

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
static i32p_t _queue_send_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    os_id_t id = (os_id_t)pArgs[0].u32_val;
    queue_sch_t *pQue_sch = (queue_sch_t *)pArgs[1].ptr_val;
    u32_t timeout_ms = (u32_t)pArgs[2].u32_val;
    i32p_t postcode = 0;

    queue_context_t *pCurQueue = _queue_object_contextGet(id);
    thread_context_t *pCurThread = kernel_thread_runContextGet();
    if (pQue_sch->size > pCurQueue->elementLength) {
        EXIT_CRITICAL_SECTION();
        return _PCER;
    }

    if (pCurQueue->cacheSize == pCurQueue->elementNumber) {
        if (timeout_ms == OS_TIME_NOWAIT_VAL) {
            EXIT_CRITICAL_SECTION();
            return _PCER;
        }

        pCurThread->schedule.pPendData = (void *)pQue_sch;
        postcode = kernel_thread_exit_trigger(pCurThread, id, _queue_list_inBlockingHeadGet(id), timeout_ms);
        PC_IF(postcode, PC_PASS)
        {
            postcode = PC_OS_WAIT_UNAVAILABLE;
        }
    } else {
        if (pQue_sch->reverse) {
            _message_send_front(pCurQueue, pQue_sch->pUsrBuf, pQue_sch->size);
        } else {
            _message_send(pCurQueue, pQue_sch->pUsrBuf, pQue_sch->size);
        }

        /* Try to wakeup a blocking thread */
        list_iterator_t it = {0u};
        list_iterator_init(&it, _queue_list_OutBlockingHeadGet(id));
        pCurThread = (thread_context_t *)list_iterator_next(&it);
        if (pCurThread) {
            postcode = kernel_thread_entry_trigger(pCurThread, _QUEUE_WAKEUP_RECEIVER, _queue_schedule);
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
static i32p_t _queue_receive_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    os_id_t id = (os_id_t)pArgs[0].u32_val;
    queue_sch_t *pQue_sch = (queue_sch_t *)pArgs[1].ptr_val;
    u32_t timeout_ms = (u32_t)pArgs[2].u32_val;
    i32p_t postcode = 0;

    queue_context_t *pCurQueue = _queue_object_contextGet(id);
    thread_context_t *pCurThread = (thread_context_t *)kernel_thread_runContextGet();
    if (pQue_sch->size > pCurQueue->elementLength) {
        EXIT_CRITICAL_SECTION();
        return _PCER;
    }

    if (!pCurQueue->cacheSize) {
        if (timeout_ms == OS_TIME_NOWAIT_VAL) {
            EXIT_CRITICAL_SECTION();
            return _PCER;
        }
        pCurThread->schedule.pPendData = (void *)pQue_sch;
        postcode = kernel_thread_exit_trigger(pCurThread, id, _queue_list_OutBlockingHeadGet(id), timeout_ms);
        PC_IF(postcode, PC_PASS)
        {
            postcode = PC_OS_WAIT_UNAVAILABLE;
        }
    } else {
        if (pQue_sch->reverse) {
            _message_receive_behind(pCurQueue, (const u8_t *)pQue_sch->pUsrBuf, pQue_sch->size);
        } else {
            _message_receive(pCurQueue, (const u8_t *)pQue_sch->pUsrBuf, pQue_sch->size);
        }

        /* Try to wakeup a blocking thread */
        list_iterator_t it = {0u};
        list_iterator_init(&it, _queue_list_inBlockingHeadGet(id));
        pCurThread = (thread_context_t *)list_iterator_next(&it);
        if (pCurThread) {
            postcode = kernel_thread_entry_trigger(pCurThread, _QUEUE_WAKEUP_SENDER, _queue_schedule);
        }
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
u32_t _impl_queue_os_id_to_number(os_id_t id)
{
    if (_queue_id_isInvalid(id)) {
        return 0u;
    }

    return (u32_t)((id - kernel_member_id_toUnifiedIdStart(KERNEL_MEMBER_QUEUE)) / sizeof(queue_context_t));
}

/**
 * @brief Initialize a new queue.
 *
 * @param pName The queue name.
 * @param pQueueBufferAddr The pointer of the queue buffer.
 * @param elementLen The element size.
 * @param elementNum The element number.
 *
 * @return The queue unique id.
 */
os_id_t _impl_queue_init(const void *pQueueBufferAddr, u16_t elementLen, u16_t elementNum, const char_t *pName)
{
    if (!pQueueBufferAddr) {
        return OS_INVALID_ID_VAL;
    }

    if (!elementLen) {
        return OS_INVALID_ID_VAL;
    }

    if (!elementNum) {
        return OS_INVALID_ID_VAL;
    }

    arguments_t arguments[] = {
        [0] = {.u32_val = (u32_t)pQueueBufferAddr},
        [1] = {.u16_val = (u16_t)elementLen},
        [2] = {.u16_val = (u16_t)elementNum},
        [3] = {.pch_val = (const char_t *)pName},
    };

    return kernel_privilege_invoke((const void *)_queue_init_privilege_routine, arguments);
}

/**
 * @brief Send a queue message.
 *
 * @param id The queue unique id.
 * @param pUserBuffer The pointer of the message buffer address.
 * @param bufferSize The queue buffer size.
 * @param isToFront The direction of the message operation.
 * @param timeout_ms The queue send timeout option.
 *
 * @return The result of the operation.
 */
i32p_t _impl_queue_send(os_id_t id, const u8_t *pUserBuffer, u16_t bufferSize, b_t isToFront, u32_t timeout_ms)
{
    if (_queue_id_isInvalid(id)) {
        return _PCER;
    }

    if (!_queue_object_isInit(id)) {
        return _PCER;
    }

    if (!kernel_isInThreadMode()) {
        if (timeout_ms != OS_TIME_NOWAIT_VAL) {
            return _PCER;
        }
    }

    queue_sch_t que_sch = {.pUsrBuf = pUserBuffer, .size = bufferSize, .reverse = isToFront};
    arguments_t arguments[] = {
        [0] = {.u32_val = (u32_t)id},
        [1] = {.ptr_val = (void *)&que_sch},
        [2] = {.u32_val = (u32_t)timeout_ms},
    };
    i32p_t postcode = kernel_privilege_invoke((const void *)_queue_send_privilege_routine, arguments);

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
 * @brief Receive a queue message.
 *
 * @param id The queue unique id.
 * @param pUserBuffer The pointer of the message buffer address.
 * @param bufferSize The queue buffer size.
 * @param isFromBack The direction of the message operation.
 * @param timeout_ms The queue send timeout option.
 *
 * @return The result of the operation.
 */
i32p_t _impl_queue_receive(os_id_t id, const u8_t *pUserBuffer, u16_t bufferSize, b_t isFromBack, u32_t timeout_ms)
{
    if (_queue_id_isInvalid(id)) {
        return _PCER;
    }

    if (!_queue_object_isInit(id)) {
        return _PCER;
    }

    if (!kernel_isInThreadMode()) {
        if (timeout_ms != OS_TIME_NOWAIT_VAL) {
            return _PCER;
        }
    }

    queue_sch_t que_sch = {.pUsrBuf = pUserBuffer, .size = bufferSize, .reverse = isFromBack};
    arguments_t arguments[] = {
        [0] = {.u32_val = (u32_t)id},
        [1] = {.ptr_val = (void *)&que_sch},
        [2] = {.u32_val = (u32_t)timeout_ms},
    };
    i32p_t postcode = kernel_privilege_invoke((const void *)_queue_receive_privilege_routine, arguments);

    ENTER_CRITICAL_SECTION();

    if (postcode == PC_OS_WAIT_UNAVAILABLE) {
        thread_context_t *pCurThread = (thread_context_t *)kernel_thread_runContextGet();

        postcode = (i32p_t)pCurThread->schedule.entry.result;
        pCurThread->schedule.entry.result = 0u;
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
 * @brief Get queue snapshot informations.
 *
 * @param instance The queue instance number.
 * @param pMsgs The kernel snapshot information pointer.
 *
 * @return TRUE: Operation pass, FALSE: Operation failed.
 */
b_t queue_snapshot(u32_t instance, kernel_snapshot_t *pMsgs)
{
#if defined KTRACE
    queue_context_t *pCurQueue = NULL;
    u32_t offset = 0u;
    os_id_t id = OS_INVALID_ID_VAL;

    ENTER_CRITICAL_SECTION();

    offset = sizeof(queue_context_t) * instance;
    pCurQueue = (queue_context_t *)(kernel_member_id_toContainerStartAddress(KERNEL_MEMBER_QUEUE) + offset);
    id = kernel_member_containerAddress_toUnifiedid((u32_t)pCurQueue);
    os_memset((u8_t *)pMsgs, 0x0u, sizeof(kernel_snapshot_t));

    if (_queue_id_isInvalid(id)) {
        EXIT_CRITICAL_SECTION();
        return FALSE;
    }

    if (pCurQueue->head.linker.pList == _queue_list_initHeadGet()) {
        pMsgs->pState = "init";
    } else if (pCurQueue->head.linker.pList) {
        pMsgs->pState = "*";
    } else {
        pMsgs->pState = "unused";

        EXIT_CRITICAL_SECTION();
        return FALSE;
    }

    pMsgs->id = pCurQueue->head.id;
    pMsgs->pName = pCurQueue->head.pName;

    pMsgs->queue.cacheSize = pCurQueue->cacheSize;
    pMsgs->queue.in_list = pCurQueue->inBlockingThreadListHead;
    pMsgs->queue.out_list = pCurQueue->outBlockingThreadListHead;

    EXIT_CRITICAL_SECTION();
    return TRUE;
#else
    return FALSE;
#endif
}

#ifdef __cplusplus
}
#endif
