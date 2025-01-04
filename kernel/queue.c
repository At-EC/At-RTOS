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
#include "init.h"

/**
 * Local unique postcode.
 */
#define PC_EOR                 PC_IER(PC_OS_CMPT_QUEUE_6)
#define _QUEUE_WAKEUP_SENDER   (10u)
#define _QUEUE_WAKEUP_RECEIVER (11u)

/**
 * @brief Check if the queue unique id if is's invalid.
 *
 * @param id The provided unique id.
 *
 * @return The true is invalid, otherwise is valid.
 */
static b_t _queue_context_isInvalid(queue_context_t *pCurQue)
{
    u32_t start, end;
    INIT_SECTION_FIRST(INIT_SECTION_OS_QUEUE_LIST, start);
    INIT_SECTION_LAST(INIT_SECTION_OS_QUEUE_LIST, end);

    return ((u32_t)pCurQue < start || (u32_t)pCurQue >= end) ? true : false;
}

/**
 * @brief Check if the queue object if is's initialized.
 *
 * @param id The provided unique id.
 *
 * @return The true is initialized, otherwise is uninitialized.
 */
static b_t _queue_context_isInit(queue_context_t *pCurQue)
{
    return ((pCurQue) ? (((pCurQue->head.cs) ? (true) : (false))) : false);
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
static void _queue_schedule(void *pTask)
{
    struct schedule_task *pCurTask = (struct schedule_task *)pTask;
    struct call_entry *pEntry = &pCurTask->exec.entry;

    timeout_remove(&pCurTask->expire, true);

    queue_context_t *pCurQueue = (queue_context_t *)pCurTask->pPendCtx;
    queue_sch_t *pQue_sche = (queue_sch_t *)pCurTask->pPendData;
    if (!pQue_sche) {
        return;
    }
    if (pEntry->result == _QUEUE_WAKEUP_RECEIVER) {
        if (pQue_sche->reverse) {
            _message_receive_behind((queue_context_t *)pCurQueue, pQue_sche->pUsrBuf, pQue_sche->size);
        } else {
            _message_receive((queue_context_t *)pCurQueue, pQue_sche->pUsrBuf, pQue_sche->size);
        }
        pEntry->result = 0;
    } else if (pEntry->result == _QUEUE_WAKEUP_SENDER) {
        if (pQue_sche->reverse) {
            _message_send_front((queue_context_t *)pCurQueue, pQue_sche->pUsrBuf, pQue_sche->size);
        } else {
            _message_send((queue_context_t *)pCurQueue, pQue_sche->pUsrBuf, pQue_sche->size);
        }
        pEntry->result = 0;
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

    INIT_SECTION_FOREACH(INIT_SECTION_OS_QUEUE_LIST, queue_context_t, pCurQueue)
    {
        if (_queue_context_isInvalid(pCurQueue)) {
            break;
        }

        if (_queue_context_isInit(pCurQueue)) {
            continue;
        }

        os_memset((char_t *)pCurQueue, 0x0u, sizeof(queue_context_t));
        pCurQueue->head.cs = CS_INITED;
        pCurQueue->head.pName = pName;

        pCurQueue->pQueueBufferAddress = pQueueBufferAddr;
        pCurQueue->elementLength = elementLen;
        pCurQueue->elementNumber = elementNum;
        pCurQueue->leftPosition = 0u;
        pCurQueue->rightPosition = 0u;
        pCurQueue->cacheSize = 0u;

        EXIT_CRITICAL_SECTION();
        return (u32_t)pCurQueue;
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
static i32p_t _queue_send_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    queue_context_t *pCurQueue = (queue_context_t *)pArgs[0].u32_val;
    queue_sch_t *pQue_sch = (queue_sch_t *)pArgs[1].ptr_val;
    u32_t timeout_ms = (u32_t)pArgs[2].u32_val;
    i32p_t postcode = 0;

    thread_context_t *pCurThread = kernel_thread_runContextGet();
    if (pQue_sch->size > pCurQueue->elementLength) {
        EXIT_CRITICAL_SECTION();
        return PC_EOR;
    }

    if (pCurQueue->cacheSize == pCurQueue->elementNumber) {
        if (timeout_ms == OS_TIME_NOWAIT_VAL) {
            EXIT_CRITICAL_SECTION();
            return PC_EOR;
        }
        postcode = schedule_exit_trigger(&pCurThread->task, pCurQueue, pQue_sch, &pCurQueue->in_QList, timeout_ms, true);
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
        list_t *plist = (list_t *)&pCurQueue->out_QList;
        list_iterator_init(&it, plist);
        struct schedule_task *pCurTask = (struct schedule_task *)list_iterator_next(&it);
        if (pCurTask) {
            postcode = schedule_entry_trigger(pCurTask, _queue_schedule, _QUEUE_WAKEUP_RECEIVER);
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

    queue_context_t *pCurQueue = (queue_context_t *)pArgs[0].u32_val;
    queue_sch_t *pQue_sch = (queue_sch_t *)pArgs[1].ptr_val;
    u32_t timeout_ms = (u32_t)pArgs[2].u32_val;
    i32p_t postcode = 0;

    thread_context_t *pCurThread = (thread_context_t *)kernel_thread_runContextGet();
    if (pQue_sch->size > pCurQueue->elementLength) {
        EXIT_CRITICAL_SECTION();
        return PC_EOR;
    }

    if (!pCurQueue->cacheSize) {
        if (timeout_ms == OS_TIME_NOWAIT_VAL) {
            EXIT_CRITICAL_SECTION();
            return PC_EOR;
        }
        postcode = schedule_exit_trigger(&pCurThread->task, pCurQueue, pQue_sch, &pCurQueue->out_QList, timeout_ms, true);
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

        /* Try to wakeup a blocking task */
        list_iterator_t it = {0u};
        list_t *plist = (list_t *)&pCurQueue->in_QList;
        list_iterator_init(&it, plist);
        struct schedule_task *pCurTask = (struct schedule_task *)list_iterator_next(&it);
        if (pCurTask) {
            postcode = schedule_entry_trigger(pCurTask, _queue_schedule, _QUEUE_WAKEUP_SENDER);
        }
    }

    EXIT_CRITICAL_SECTION();
    return postcode;
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
u32_t _impl_queue_init(const void *pQueueBufferAddr, u16_t elementLen, u16_t elementNum, const char_t *pName)
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
 * @param ctx The queue unique id.
 * @param pUserBuffer The pointer of the message buffer address.
 * @param bufferSize The queue buffer size.
 * @param isToFront The direction of the message operation.
 * @param timeout_ms The queue send timeout option.
 *
 * @return The result of the operation.
 */
i32p_t _impl_queue_send(u32_t ctx, const u8_t *pUserBuffer, u16_t bufferSize, b_t isToFront, u32_t timeout_ms)
{
    queue_context_t *pCtx = (queue_context_t *)ctx;
    if (_queue_context_isInvalid(pCtx)) {
        return PC_EOR;
    }

    if (!_queue_context_isInit(pCtx)) {
        return PC_EOR;
    }

    if (!kernel_isInThreadMode()) {
        if (timeout_ms != OS_TIME_NOWAIT_VAL) {
            return PC_EOR;
        }
    }

    queue_sch_t que_sch = {.pUsrBuf = pUserBuffer, .size = bufferSize, .reverse = isToFront};
    arguments_t arguments[] = {
        [0] = {.u32_val = (u32_t)ctx},
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
i32p_t _impl_queue_receive(u32_t ctx, const u8_t *pUserBuffer, u16_t bufferSize, b_t isFromBack, u32_t timeout_ms)
{
    queue_context_t *pCtx = (queue_context_t *)ctx;
    if (_queue_context_isInvalid(pCtx)) {
        return PC_EOR;
    }

    if (!_queue_context_isInit(pCtx)) {
        return PC_EOR;
    }

    if (!kernel_isInThreadMode()) {
        if (timeout_ms != OS_TIME_NOWAIT_VAL) {
            return PC_EOR;
        }
    }

    queue_sch_t que_sch = {.pUsrBuf = pUserBuffer, .size = bufferSize, .reverse = isFromBack};
    arguments_t arguments[] = {
        [0] = {.u32_val = (u32_t)ctx},
        [1] = {.ptr_val = (void *)&que_sch},
        [2] = {.u32_val = (u32_t)timeout_ms},
    };
    i32p_t postcode = kernel_privilege_invoke((const void *)_queue_receive_privilege_routine, arguments);

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
