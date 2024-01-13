/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "basic.h"
#include "kernal.h"
#include "timer.h"
#include "queue.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Local unique postcode.
 */
#define _PC_CMPT_FAILED              PC_FAILED(PC_CMPT_QUEUE)
#define _QUEUE_WAKEUP_SENDER         PC_SC_B
#define _QUEUE_WAKEUP_RECEIVER       PC_SC_A

/**
 * The local function lists for current file internal use.
 */
static u32_t _queue_init_privilege_routine(arguments_t *pArgs);
static u32_t _queue_send_privilege_routine(arguments_t *pArgs);
static u32_t _queue_receive_privilege_routine(arguments_t *pArgs);

static void _queue_schedule(os_id_t id);

/**
 * @brief Get the queue context based on provided unique id.
 *
 * @param id The timer unique id.
 *
 * @return The pointer of the current unique id timer context.
 */
static queue_context_t* _queue_object_contextGet(os_id_t id)
{
    return (queue_context_t*)(_impl_kernal_member_unified_id_toContainerAddress(id));
}

/**
 * @brief Get the init queue list head.
 *
 * @return The value of the init list head.
 */
static list_t* _queue_list_initHeadGet(void)
{
    return (list_t*)_impl_kernal_member_list_get(KERNAL_MEMBER_QUEUE, KERNAL_MEMBER_LIST_QUEUE_INIT);
}

/**
 * @brief Get the queue blocking thread list head address.
 *
 * @return The blocking thread list head address.
 */
static list_t* _queue_list_inBlockingHeadGet(os_id_t id)
{
    queue_context_t *pCurQueue = (queue_context_t *)_queue_object_contextGet(id);
    return (list_t*)(&pCurQueue->inBlockingThreadListHead);
}

/**
 * @brief Get the queue blocking thread list head address.
 *
 * @return The blocking thread list head address.
 */
static list_t* _queue_list_OutBlockingHeadGet(os_id_t id)
{
    queue_context_t *pCurQueue = (queue_context_t *)_queue_object_contextGet(id);
    return (list_t*)(&pCurQueue->outBlockingThreadListHead);
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
    return _impl_kernal_member_unified_id_isInvalid(KERNAL_MEMBER_QUEUE, id);
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
    queue_context_t *pCurQueue = (queue_context_t *)_queue_object_contextGet(id);

    return ((pCurQueue) ? (((pCurQueue->head.linker.pList) ? (TRUE) : (FALSE))) : FALSE);
}

/**
 * @brief The queue timeout callback fucntion.
 *
 * @param id The queue unique id.
 */
static void _queue_callback_fromTimeOut(os_id_t id)
{
    _impl_kernal_thread_entry_trigger(_impl_kernal_member_unified_id_timerToThread(id), id, PC_SC_TIMEOUT, _queue_schedule);
}

/**
 * @brief Convert the internal os id to kernal member number.
 *
 * @param id The provided unique id.
 *
 * @return The value of member number.
 */
u32_t _impl_queue_os_id_to_number(os_id_t id)
{
    return (u32_t)(_queue_id_isInvalid(id) ? (0u) : (id - _impl_kernal_member_id_toUnifiedIdStart(KERNAL_MEMBER_QUEUE)) / sizeof(queue_context_t));
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
    if (!pQueueBufferAddr)
    {
        return OS_INVALID_ID;
    }

    if (!elementLen)
    {
        return OS_INVALID_ID;
    }

    if (!elementNum)
    {
        return OS_INVALID_ID;
    }

    arguments_t arguments[] =
    {
        [0] = {(u32_t)pQueueBufferAddr},
        [1] = {(u32_t)elementLen},
        [2] = {(u32_t)elementNum},
        [3] = {(u32_t)pName},
    };

    return _impl_kernal_privilege_invoke(_queue_init_privilege_routine, arguments);
}

/**
 * @brief Send a queue message.
 *
 * @param pCurQueue The current queue context.
 * @param pUserBuffer The pointer of user's message buffer.
 * @param userSize The size of user's message buffer.
 */
static void _queue_message_send(queue_context_t *pCurQueue, const u8_t *pUserBuffer, u16_t userSize)
{
    u8_t *pInBuffer = (u8_t*)((u32_t)((pCurQueue->leftPosition * pCurQueue->elementLength) + (u32_t)pCurQueue->pQueueBufferAddress));
    _memset((char_t*)pInBuffer, 0x0u, pCurQueue->elementLength);
    _memcpy((char_t*)pInBuffer, (const char_t*)pUserBuffer, userSize);

    // Calculate the next left position
    // Receive empty: right + 1 == left
    // Send full: left + 1 == right
    pCurQueue->leftPosition = (pCurQueue->elementNumber % (pCurQueue->leftPosition + 1u));
    pCurQueue->cacheSize++;
}

/**
 * @brief Receive a queue message.
 *
 * @param pCurQueue The current queue context.
 * @param pUserBuffer The pointer of user's message buffer.
 * @param userSize The size of user's message buffer.
 */
static void _queue_message_receive(queue_context_t *pCurQueue, const u8_t *pUserBuffer, u16_t userSize)
{
    u8_t *pOutBuffer = (u8_t*)((pCurQueue->rightPosition * pCurQueue->elementLength) + (u32_t)pCurQueue->pQueueBufferAddress);
    _memset((char_t*)pUserBuffer, 0x0u, userSize);
    _memcpy((char_t*)pUserBuffer, (const char_t*)pOutBuffer, userSize);

    // Calculate the next right position
    // Receive empty: right + 1 == left
    // Send full: left + 1 == right
    pCurQueue->rightPosition = (pCurQueue->elementNumber % (pCurQueue->rightPosition + 1u));
    pCurQueue->cacheSize--;
}

/**
 * @brief Send a queue message.
 *
 * @param id The queue unique id.
 * @param pUserBuffer The pointer of the message buffer address.
 * @param bufferSize The queue buffer size.
 * @param timeout_ms The queue send timeout option.
 *
 * @return The result of the operation.
 */
u32p_t _impl_queue_send(os_id_t id, const u8_t *pUserBuffer, u16_t bufferSize, u32_t timeout_ms)
{
    if (_queue_id_isInvalid(id))
    {
        return _PC_CMPT_FAILED;
    }

    if (!_queue_object_isInit(id))
    {
        return _PC_CMPT_FAILED;
    }

    if (!_impl_kernal_isInThreadMode())
    {
        if (timeout_ms != OS_TIME_NOWAIT_VAL)
        {
            return _PC_CMPT_FAILED;
        }
    }

    arguments_t arguments[] =
    {
        [0] = {(u32_t)id},
        [1] = {(u32_t)pUserBuffer},
        [2] = {(u32_t)bufferSize},
        [3] = {(u32_t)timeout_ms},
    };

    u32p_t postcode = _impl_kernal_privilege_invoke(_queue_send_privilege_routine, arguments);

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
 * @brief Receive a queue message.
 *
 * @param id The queue unique id.
 * @param pUserBuffer The pointer of the message buffer address.
 * @param bufferSize The queue buffer size.
 * @param timeout_ms The queue send timeout option.
 *
 * @return The result of the operation.
 */
u32p_t _impl_queue_receive(os_id_t id, const u8_t *pUserBuffer, u16_t bufferSize, u32_t timeout_ms)
{
    queue_context_t *pCurQueue = (queue_context_t *)_queue_object_contextGet(id);
    thread_context_t *pCurThread = (thread_context_t *)_impl_kernal_thread_runContextGet();

    if (_queue_id_isInvalid(id))
    {
        return _PC_CMPT_FAILED;
    }

    if (!_queue_object_isInit(id))
    {
        return _PC_CMPT_FAILED;
    }

    if (!_impl_kernal_isInThreadMode())
    {
        if (timeout_ms != OS_TIME_NOWAIT_VAL)
        {
            return _PC_CMPT_FAILED;
        }
    }

    arguments_t arguments[] =
    {
        [0] = {(u32_t)id},
        [1] = {(u32_t)pUserBuffer},
        [2] = {(u32_t)bufferSize},
        [3] = {(u32_t)timeout_ms},
    };

    u32p_t postcode = _impl_kernal_privilege_invoke(_queue_receive_privilege_routine, arguments);

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
 * @brief It's sub-routine running at privilege mode.
 *
 * @param pArgs The function argument packages.
 *
 * @return The result of privilege routine.
 */
static u32_t _queue_init_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    const void *pQueueBufferAddr = (const void *)(pArgs[0].u32_val);
    u16_t elementLen = (u16_t)(pArgs[1].u32_val);
    u16_t elementNum = (u16_t)(pArgs[2].u32_val);
    const char_t *pName = (const char_t *)(pArgs[3].u32_val);

    queue_context_t *pCurQueue = (queue_context_t *)_impl_kernal_member_id_toContainerStartAddress(KERNAL_MEMBER_QUEUE);
    os_id_t id = _impl_kernal_member_id_toUnifiedIdStart(KERNAL_MEMBER_QUEUE);

    do {
        if (!_queue_object_isInit(id))
        {
            _memset((char_t*)pCurQueue, 0x0u, sizeof(queue_context_t));

            pCurQueue->head.id = id;
            pCurQueue->head.pName = pName;

            pCurQueue->pQueueBufferAddress = pQueueBufferAddr;
            pCurQueue->elementLength = elementLen;
            pCurQueue->elementNumber = elementNum;
            pCurQueue->leftPosition = 0u;
            pCurQueue->rightPosition = 0u;
            pCurQueue->cacheSize = 0u;

            _queue_list_transferToInit((linker_head_t*)&pCurQueue->head);

            break;
        }

        pCurQueue++;
        id = _impl_kernal_member_containerAddress_toUnifiedid((u32_t)pCurQueue);
    } while ((u32_t)pCurQueue < (u32_t)_impl_kernal_member_id_toContainerEndAddress(KERNAL_MEMBER_QUEUE));

    id = ((!_queue_id_isInvalid(id)) ? (id) : (OS_INVALID_ID));

    EXIT_CRITICAL_SECTION();

    return id;
}

/**
 * @brief It's sub-routine running at privilege mode.
 *
 * @param pArgs The function argument packages.
 *
 * @return The result of privilege routine.
 */
static u32_t _queue_send_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    os_id_t id = (os_id_t)pArgs[0].u32_val;
    const u8_t *pUserBuffer = (const void *)pArgs[1].u32_val;
    u16_t bufferSize = (u16_t)pArgs[2].u32_val;
    u32_t timeout_ms = (u32_t)pArgs[3].u32_val;

    u32p_t postcode = PC_SC_SUCCESS;
    queue_context_t *pCurQueue = (queue_context_t *)_queue_object_contextGet(id);
    thread_context_t *pCurThread = (thread_context_t *)_impl_kernal_thread_runContextGet();

    if (bufferSize > pCurQueue->elementLength)
    {
        EXIT_CRITICAL_SECTION();
        return _PC_CMPT_FAILED;
    }

    if (pCurQueue->cacheSize == pCurQueue->elementNumber)
    {
        if (timeout_ms == OS_TIME_NOWAIT_VAL)
        {
            EXIT_CRITICAL_SECTION();
            return _PC_CMPT_FAILED;
        }
        else
        {
            pCurThread->queue.pUserBufferAddress = pUserBuffer;
            pCurThread->queue.userBufferSize = bufferSize;

            postcode = _impl_kernal_thread_exit_trigger(pCurThread->head.id, id, _queue_list_inBlockingHeadGet(id), timeout_ms, _queue_callback_fromTimeOut);

            if(PC_IOK(postcode))
            {
                postcode = PC_SC_UNAVAILABLE;
            }
        }
    }
    else
    {
        _queue_message_send(pCurQueue, pUserBuffer, bufferSize);

        /* Try to wakeup a blocking thread */
        list_iterator_t it = {0u};
        list_iterator_init(&it, _queue_list_OutBlockingHeadGet(id));
        pCurThread = (thread_context_t *)list_iterator_next(&it);
        if (pCurThread)
        {
            postcode = _impl_kernal_thread_entry_trigger(pCurThread->head.id, id, _QUEUE_WAKEUP_RECEIVER, _queue_schedule);
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
static u32_t _queue_receive_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    os_id_t id = (os_id_t)pArgs[0].u32_val;
    const u8_t *pUserBuffer = (const u8_t *)pArgs[1].u32_val;
    u16_t bufferSize = (u16_t)pArgs[2].u32_val;
    u32_t timeout_ms = (u32_t)pArgs[3].u32_val;

    u32p_t postcode = PC_SC_SUCCESS;
    queue_context_t *pCurQueue = (queue_context_t *)_queue_object_contextGet(id);
    thread_context_t *pCurThread = (thread_context_t *)_impl_kernal_thread_runContextGet();

    if (bufferSize > pCurQueue->elementLength)
    {
        EXIT_CRITICAL_SECTION();
        return _PC_CMPT_FAILED;
    }

    if (!pCurQueue->cacheSize)
    {
        if (timeout_ms == OS_TIME_NOWAIT_VAL)
        {
            EXIT_CRITICAL_SECTION();
            return _PC_CMPT_FAILED;
        }
        else
        {
            pCurThread->queue.pUserBufferAddress = pUserBuffer;
            pCurThread->queue.userBufferSize = bufferSize;

            postcode = _impl_kernal_thread_exit_trigger(pCurThread->head.id, id, _queue_list_OutBlockingHeadGet(id), timeout_ms, _queue_callback_fromTimeOut);

            if(PC_IOK(postcode))
            {
                postcode = PC_SC_UNAVAILABLE;
            }
        }
    }
    else
    {
        _queue_message_receive(pCurQueue, pUserBuffer, bufferSize);

        /* Try to wakeup a blocking thread */
        list_iterator_t it = {0u};
        list_iterator_init(&it, _queue_list_inBlockingHeadGet(id));
        pCurThread = (thread_context_t *)list_iterator_next(&it);
        if (pCurThread)
        {
            postcode = _impl_kernal_thread_entry_trigger(pCurThread->head.id, id, _QUEUE_WAKEUP_SENDER, _queue_schedule);
        }
    }

    EXIT_CRITICAL_SECTION();

    return postcode;
}

/**
 * @brief The queue schedule routine execute the the pendsv context.
 *
 * @param id The unique id of the entry thread.
 */
static void _queue_schedule(os_id_t id)
{
    thread_context_t *pEntryThread = (thread_context_t*)(_impl_kernal_member_unified_id_toContainerAddress(id));

    if (_impl_kernal_member_unified_id_toId(pEntryThread->schedule.hold) == KERNAL_MEMBER_QUEUE)
    {
        queue_context_t *pCurQueue = (queue_context_t *)_queue_object_contextGet(pEntryThread->schedule.hold);
        thread_entry_t *pEntry = &pEntryThread->schedule.entry;

        b_t isTxAvail = FALSE;
        b_t isRxAvail = FALSE;

        if (pEntry->result == PC_SC_TIMEOUT)
        {
            pEntry->result = PC_SC_TIMEOUT;
        }
        else if (pEntry->result == _QUEUE_WAKEUP_RECEIVER)
        {
            // Release function doesn't kill the timer node from waiting list
            if (!_impl_timer_status_isBusy(_impl_kernal_member_unified_id_threadToTimer(pEntryThread->head.id)))
            {
                if (_impl_kernal_member_unified_id_toId(pEntry->release) == KERNAL_MEMBER_TIMER_INTERNAL)
                {
                    pEntry->result = PC_SC_TIMEOUT;
                }
                else
                {
                    isRxAvail = true;
                }
            }
            else if (_impl_kernal_member_unified_id_toId(pEntry->release) == KERNAL_MEMBER_QUEUE)
            {
                _impl_timer_stop(_impl_kernal_member_unified_id_threadToTimer(pEntryThread->head.id));
                isRxAvail = true;
            }
            else
            {
                pEntry->result = _PC_CMPT_FAILED;
            }
        }
        else if (pEntry->result == _QUEUE_WAKEUP_SENDER)
        {
            // Release function doesn't kill the timer node from waiting list
            if (!_impl_timer_status_isBusy(_impl_kernal_member_unified_id_threadToTimer(pEntryThread->head.id)))
            {
                if (_impl_kernal_member_unified_id_toId(pEntry->release) == KERNAL_MEMBER_TIMER_INTERNAL)
                {
                    pEntry->result = PC_SC_TIMEOUT;
                }
                else
                {
                    isTxAvail = true;
                }
            }
            else if (_impl_kernal_member_unified_id_toId(pEntry->release) == KERNAL_MEMBER_QUEUE)
            {
                _impl_timer_stop(_impl_kernal_member_unified_id_threadToTimer(pEntryThread->head.id));
                isTxAvail = true;
            }
            else
            {
                pEntry->result = _PC_CMPT_FAILED;
            }
        }
        else
        {
            pEntry->result = _PC_CMPT_FAILED;
        }

        if ((isRxAvail) || (isTxAvail))
        {
            if (isRxAvail)
            {
                _queue_message_receive((queue_context_t*)pCurQueue, pEntryThread->queue.pUserBufferAddress, pEntryThread->queue.userBufferSize);
            }
            else if (isTxAvail)
            {
                _queue_message_send((queue_context_t*)pCurQueue, pEntryThread->queue.pUserBufferAddress, pEntryThread->queue.userBufferSize);
            }
            pEntry->result = PC_SC_SUCCESS;
        }
    }
    else
    {
        pEntryThread->schedule.entry.result = PC_FAILED(PC_CMPT_EVENT);
    }
}

#ifdef __cplusplus
}
#endif
