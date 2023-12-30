/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "basic.h"
#include "kernal.h"
#include "clock_systick.h"
#include "timer.h"

#ifdef __cplusplus
extern "C" {
#endif

#define _PC_CMPT_FAILED       PC_FAILED(PC_CMPT_TIMER)

static u64_t _s_system_time_us = 0u;
static u32_t _s_remaining_elapsed_us = 0u;

static u32_t _timer_init_privilege_routine(arguments_t *pArgs);
static u32_t _timer_start_privilege_routine(arguments_t *pArgs);
static u32_t _timer_stop_privilege_routine(arguments_t *pArgs);
static u64_t _impl_timer_total_system_get_privilege_routine(arguments_t *pArgs);
static u32_t _impl_kernal_timer_schedule_request_privilege_routine(arguments_t *pArgs);

/**
 * @brief Get the timer context based on provided unique id.
 *
 * Get the timer context based on provided unique id, and then return the timer context pointer.
 *
 * @param id Timer unique id.
 *
 * @retval VALUE The timer context.
 */
static timer_context_t* _timer_object_contextGet(os_id_t id)
{
    return (timer_context_t*)(kernal_member_unified_id_toContainerAddress(id));
}

/**
 * @brief Get the stoping timer list address.
 *
 * Get the stoping timer list address.
 *
 * @param NONE.
 *
 * @retval VALUE The stoping list head.
 */
static list_t* _timer_list_stopingHeadGet(void)
{
    return (list_t*)kernal_member_list_get(KERNAL_MEMBER_TIMER, KERNAL_MEMBER_LIST_TIMER_STOP);
}

/**
 * @brief Get the waiting timer list address.
 *
 * Get the waiting timer list address.
 *
 * @param NONE.
 *
 * @retval VALUE The waiting list head.
 */
static list_t* _timer_list_waitingHeadGet(void)
{
    return (list_t*)kernal_member_list_get(KERNAL_MEMBER_TIMER, KERNAL_MEMBER_LIST_TIMER_WAIT);
}

/**
 * @brief Get the ending timer list address.
 *
 * Get the ending timer list address.
 *
 * @param NONE.
 *
 * @retval VALUE The ending list head.
 */
static list_t* _timer_list_endingHeadGet(void)
{
    return (list_t*)kernal_member_list_get(KERNAL_MEMBER_TIMER, KERNAL_MEMBER_LIST_TIMER_END);
}

/**
 * @brief Get the pending timer list address.
 *
 * Get the pending timer list address.
 *
 * @param NONE.
 *
 * @retval VALUE The pending list head.
 */
static list_t* _timer_list_pendingHeadGet(void)
{
    return (list_t*)kernal_member_list_get(KERNAL_MEMBER_TIMER, KERNAL_MEMBER_LIST_TIMER_PEND);
}

/**
 * @brief Get the running timer list address.
 *
 * Get the running timer list address.
 *
 * @param NONE.
 *
 * @retval VALUE The running list head.
 */
static list_t* _timer_list_runningHeadGet(void)
{
    return (list_t*)kernal_member_list_get(KERNAL_MEMBER_TIMER, KERNAL_MEMBER_LIST_TIMER_RUN);
}

static void _timer_list_remove_fromWaitList(linker_head_t *pCurHead)
{
    ENTER_CRITICAL_SECTION();

    timer_context_t* pCurTimer = (timer_context_t*)&pCurHead->linker.node;
    timer_context_t* pNext = (timer_context_t*)pCurTimer->head.linker.node.pNext;

    if (pNext)
    {
        pNext->duration_us += pCurTimer->duration_us;
    }

    pCurTimer->duration_us = 0u;

    EXIT_CRITICAL_SECTION();
}

/**
 * @brief Push one timer context into uninitialized status.
 *
 * Push one timer context into uninitialized status.
 *
 * @param pCurHead The pointer of the timer linker head.
 *
 * @retval NONE .
 */
/* TODO
static void _timer_list_transfer_toUninitialized(linker_head_t *pCurHead)
{
    ENTER_CRITICAL_SECTION();

    linker_list_transaction_common(&pCurHead->linker, NULL, LIST_TAIL);

    EXIT_CRITICAL_SECTION();
}
*/

/**
 * @brief Push one timer context into stoping list.
 *
 * Push one thread context into stoping list.
 *
 * @param pCurHead The pointer of the timer linker head.
 *
 * @retval NONE .
 */
static void _timer_list_transfer_toStopList(linker_head_t *pCurHead)
{
    ENTER_CRITICAL_SECTION();

    list_t *pToStopList = (list_t *)_timer_list_stopingHeadGet();
    linker_list_transaction_common(&pCurHead->linker, pToStopList, LIST_TAIL);

    EXIT_CRITICAL_SECTION();
}

/**
 * @brief Push one timer context into ending list.
 *
 * Push one thread context into ending list.
 *
 * @param pCurHead The pointer of the timer linker head.
 *
 * @retval NONE .
 */
static void _timer_list_transfer_toEndList(linker_head_t *pCurHead)
{
    ENTER_CRITICAL_SECTION();

    list_t *pToEndList = (list_t *)_timer_list_endingHeadGet();
    linker_list_transaction_common(&pCurHead->linker, pToEndList, LIST_TAIL);

    EXIT_CRITICAL_SECTION();
}

/**
 * @brief Push one timer context into pending list.
 *
 * Push one thread context into pending list.
 *
 * @param pCurHead The pointer of the timer linker head.
 *
 * @retval NONE .
 */
static void _timer_list_transfer_toPendList(linker_head_t *pCurHead)
{
    ENTER_CRITICAL_SECTION();

    list_t *pToPendList = (list_t *)_timer_list_pendingHeadGet();
    linker_list_transaction_common(&pCurHead->linker, pToPendList, LIST_TAIL);

    EXIT_CRITICAL_SECTION();
}

/**
 * @brief Compare the priority between the current and extract thread.
 *
 * Compare the priority between the current and extract thread.
 *
 * @param pCurNode The pointer of the current thread node.
 * @param pExtractNode The pointer of the extract thread node.
 *
 * @retval TRUE Not a correct position.
 * @retval FALSE find a right position and it can kill the loop routine.
 */
static b_t _timer_node_Order_compare_condition(list_node_t *pCurNode, list_node_t *pExtractNode)
{
    timer_context_t *pCurTimer = (timer_context_t *)pCurNode;
    timer_context_t *pExtractTimer = (timer_context_t *)pExtractNode;

    if ((!pCurTimer) || (!pExtractTimer))
    {
        /* no available timer */
        return FALSE;
    }

    if (pCurTimer->duration_us > pExtractTimer->duration_us)
    {
        pCurTimer->duration_us -= pExtractTimer->duration_us;
        return TRUE;
    }
    else
    {
        pExtractTimer->duration_us -= pCurTimer->duration_us;
        return FALSE;
    }
}

/**
 * @brief Push one timer context into waiting list.
 *
 * Push one thread context into waiting list.
 *
 * @param pCurHead The pointer of the timer linker head.
 *
 * @retval NONE .
 */
static void _timer_list_transfer_toWaitList(linker_head_t *pCurHead)
{
    ENTER_CRITICAL_SECTION();

    list_t *pToWaitList = (list_t *)_timer_list_waitingHeadGet();
    linker_list_transaction_specific(&pCurHead->linker, pToWaitList, _timer_node_Order_compare_condition);

    EXIT_CRITICAL_SECTION();
}

/**
 * @brief Pick up a shortest remaining time from the waiting list.
 *
 * Pick up a shortest remaining time from the waiting list.
 *
 * @param NONE
 *
 * @retval VALUE The shortest time head.
 */
static linker_head_t* _timer_linker_head_fromWaiting(void)
{
    ENTER_CRITICAL_SECTION();
    list_t *pListWaiting = (list_t *)_timer_list_waitingHeadGet();
    EXIT_CRITICAL_SECTION();

    return (linker_head_t*)(pListWaiting->pHead);
}

/**
 * @brief Check if the timer unique id if is's invalid.
 *
 * Check if the timer unique id if is's invalid.
 *
 * @param id The provided unique id.
 *
 * @retval TRUE The id is invalid
 *         FALSE The id is valid
 */
static b_t _timer_id_isInvalid(i32_t id)
{
    return ((kernal_member_unified_id_isInvalid(KERNAL_MEMBER_TIMER_INTERNAL, id)) &&
            (kernal_member_unified_id_isInvalid(KERNAL_MEMBER_TIMER, id)));
}

/**
 * @brief Check if the timer object if is's initialized.
 *
 * Check if the timer unique id if is's initialization.
 *
 * @param id The provided unique id.
 *
 * @retval TRUE The id is initialized.
 *         FALSE The id isn't initialized.
 */
static b_t _timer_object_isInit(i32_t id)
{
    timer_context_t *pCurTimer = (timer_context_t *)_timer_object_contextGet(id);

    if (pCurTimer)
    {
        return ((pCurTimer->head.linker.pList) ? (TRUE) : (FALSE));
    }
    else
    {
        return FALSE;
    }
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
u32_t _impl_timer_os_id_to_number(os_id_t id)
{
    if (!kernal_member_unified_id_isInvalid(KERNAL_MEMBER_TIMER_INTERNAL, id))
    {
        return (u32_t)((id - kernal_member_id_toUnifiedIdStart(KERNAL_MEMBER_TIMER_INTERNAL)) / sizeof(timer_context_t));
    }
    else if (!kernal_member_unified_id_isInvalid(KERNAL_MEMBER_TIMER, id))
    {
        return (u32_t)((id - kernal_member_id_toUnifiedIdStart(KERNAL_MEMBER_TIMER)) / sizeof(timer_context_t));
    }
    else
    {
        return 0u;
    }
}

/**
 * @brief Initialize a new timer.
 *
 * Initialize a new timer.
 *
 * @param pCallFun The thread entry function pointer.
 * @param timeout The expired time.
 * @param pName The timer's name.
 *
 * @retval VALUE The timer unique id.
 */
os_id_t _impl_timer_init(pTimer_callbackFunc_t pCallFun, b_t isCycle, u32_t timeout_ms, const char_t *pName)
{
    arguments_t arguments[] =
    {
        [0] = {(u32_t)pCallFun},
        [1] = {(u32_t)isCycle},
        [2] = {(u32_t)timeout_ms},
        [3] = {(u32_t)pName},
    };

    return kernal_privilege_invoke(_timer_init_privilege_routine, arguments);
}

/**
 * @brief Init a timer for thread object to use.
 *
 * Init a timer for thread object to use.
 *
 * @param id The thread unique id is same as timer.
 *
 * @retval NONE .
 */
void _impl_thread_timer_init(os_id_t id)
{
    ENTER_CRITICAL_SECTION();

    timer_context_t* pCurTimer = _timer_object_contextGet(id); // Only for internal thread use

    _memset((char_t*)pCurTimer, 0x0u, sizeof(timer_context_t));

    pCurTimer->head.id = id;
    pCurTimer->head.pName = "TH";

    pCurTimer->isCycle = FALSE;
    pCurTimer->timeout_ms = 0u;
    pCurTimer->duration_us = 0u;
    pCurTimer->call.pThread = NULL;

    _timer_list_transfer_toStopList((linker_head_t *)&pCurTimer->head);

    EXIT_CRITICAL_SECTION();
}

/**
 * @brief Start a thread timer.
 *
 * Start a thread timer.
 *
 * @param id The thread unique id is same as timer.
 * @param start_us The thread timer start time.
 * @param timeout_ms he thread timer timeout time.
 * @param pCallback The thread timeout callback function.
 *
 * @retval NONE.
 */
void _impl_thread_timer_start(os_id_t id, u32_t timeout_ms, void (*pCallback)(os_id_t))
{
    ENTER_CRITICAL_SECTION();

    timer_context_t* pCurTimer = _timer_object_contextGet(id); // Only for internal thread use
    pCurTimer->call.pThread = pCallback;

    pCurTimer->timeout_ms = WAIT_FOREVER;
    pCurTimer->isCycle = FALSE;

    if (pCurTimer->head.linker.pList == _timer_list_waitingHeadGet())
    {
        _timer_list_remove_fromWaitList((linker_head_t*)&pCurTimer->head);
    }

    if (timeout_ms == WAIT_FOREVER)
    {
        _timer_list_transfer_toEndList((linker_head_t *)&pCurTimer->head);
    }
    else
    {
        pCurTimer->duration_us = (timeout_ms * 1000u);
        _timer_list_transfer_toWaitList((linker_head_t*)&pCurTimer->head);
    }

	EXIT_CRITICAL_SECTION();
}

/**
 * @brief Start a timer, Be careful if the timer's last ticket isn't expired, the new ticket will override it.
 *
 * Start a timer, Be careful if the timer's last ticket isn't expired, the new ticket will override it.
 *
 * @param id The timer unique id.
 * @param timeout_ms The timer expired time.
 *
 * @retval VALUE The result of timer start.
 */
u32p_t _impl_timer_start(os_id_t id, b_t isCycle, u32_t timeout_ms)
{
    if (_timer_id_isInvalid(id))
    {
        return _PC_CMPT_FAILED;
    }

    if (!_timer_object_isInit(id))
    {
        return _PC_CMPT_FAILED;
    }

    if (!timeout_ms)
    {
        return _PC_CMPT_FAILED;
    }

    arguments_t arguments[] =
    {
        [0] = {(u32_t)id},
        [1] = {(u32_t)isCycle},
        [2] = {(u32_t)timeout_ms},
    };

    return kernal_privilege_invoke(_timer_start_privilege_routine, arguments);
}

/**
 * @brief Stop a schduled timer.
 *
 * Stop a schduled timer.
 *
 * @param id The timer unique id.
 *
 * @retval VALUE The result of timer stop.
 */
u32p_t _impl_timer_stop(os_id_t id)
{
    if (_timer_id_isInvalid(id))
    {
        return _PC_CMPT_FAILED;
    }

    if (!_timer_object_isInit(id))
    {
        return _PC_CMPT_FAILED;
    }

    arguments_t arguments[] =
    {
        [0] = {(u32_t)id},
    };

    return kernal_privilege_invoke(_timer_stop_privilege_routine, arguments);
}

/**
 * @brief Check the timer to confirm if it's already scheduled in the waiting list.
 *
 * Check the timer to confirm if it's already scheduled in the waiting list.
 *
 * @param id The timer unique id.
 *
 * @retval VALUE The result of timer status.
 */
b_t _impl_timer_status_isBusy(os_id_t id)
{
    if (_timer_id_isInvalid(id))
    {
        return FALSE;
    }

    if (!_timer_object_isInit(id))
    {
        return FALSE;
    }

    ENTER_CRITICAL_SECTION();
    linker_head_t *pCurHead = (linker_head_t *)_timer_object_contextGet(id);
    b_t isBusy = FALSE;

    isBusy = (pCurHead->linker.pList == (list_t*)_timer_list_waitingHeadGet());
    isBusy |= (pCurHead->linker.pList == (list_t*)_timer_list_endingHeadGet());
    EXIT_CRITICAL_SECTION();

    return isBusy;
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
static u32_t _timer_init_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    pTimer_callbackFunc_t pCallFun = (pTimer_callbackFunc_t)(pArgs[0].u32_val);
    b_t isCycle = (u32_t)(pArgs[1].u32_val);
    u32_t timeout_ms = (u32_t)(pArgs[2].u32_val);
    const char_t *pName = (const char_t *)pArgs[3].u32_val;

    timer_context_t *pCurTimer = (timer_context_t *)kernal_member_id_toContainerStartAddress(KERNAL_MEMBER_TIMER);
    os_id_t id = kernal_member_id_toUnifiedIdStart(KERNAL_MEMBER_TIMER);

    do {
        if (!_timer_object_isInit(id))
        {
            _memset((char_t*)pCurTimer, 0x0u, sizeof(timer_context_t));

            pCurTimer->head.id = id;
            pCurTimer->head.pName = pName;

            pCurTimer->isCycle = isCycle;
            pCurTimer->timeout_ms = timeout_ms;
            pCurTimer->duration_us = 0u;
            pCurTimer->call.pTimer = pCallFun;

            _timer_list_transfer_toStopList((linker_head_t *)&pCurTimer->head);
            break;
        }

            pCurTimer++;
            id = kernal_member_containerAddress_toUnifiedid((u32_t)pCurTimer);
    } while ((u32_t)pCurTimer < (u32_t)kernal_member_id_toContainerEndAddress(KERNAL_MEMBER_TIMER));

    id = ((!_timer_id_isInvalid(id)) ? (id) : (OS_INVALID_ID));

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
static u32_t _timer_start_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    os_id_t id = (os_id_t)pArgs[0].u32_val;
    b_t isCycle = (b_t)pArgs[1].u32_val;
    u32_t timeout_ms = (u32_t)pArgs[2].u32_val;

    timer_context_t *pCurTimer = (timer_context_t *)_timer_object_contextGet(id);

    pCurTimer->timeout_ms = timeout_ms;
    pCurTimer->isCycle = isCycle;

    if (pCurTimer->head.linker.pList == _timer_list_waitingHeadGet())
    {
        _timer_list_remove_fromWaitList((linker_head_t*)&pCurTimer->head);
    }

    if (pCurTimer->timeout_ms == WAIT_FOREVER)
    {
        _timer_list_transfer_toEndList((linker_head_t *)&pCurTimer->head);
    }
    else
    {
        pCurTimer->duration_us = timeout_ms * 1000u;
        _timer_list_transfer_toWaitList((linker_head_t*)&pCurTimer->head);
    }
    _impl_kernal_timer_schedule_request();

    EXIT_CRITICAL_SECTION();

    return PC_SC_SUCCESS;
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
static u32_t _timer_stop_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    os_id_t id = (os_id_t)pArgs[0].u32_val;

    timer_context_t *pCurTimer = (timer_context_t *)_timer_object_contextGet(id);

    if (pCurTimer->head.linker.pList == _timer_list_waitingHeadGet())
    {
        _timer_list_remove_fromWaitList((linker_head_t*)&pCurTimer->head);
    }
    _timer_list_transfer_toStopList((linker_head_t *)&pCurTimer->head);

    _impl_kernal_timer_schedule_request();

    EXIT_CRITICAL_SECTION();

    return PC_SC_SUCCESS;
}

u32p_t _impl_kernal_timer_schedule_request(void)
{
    return kernal_privilege_invoke(_impl_kernal_timer_schedule_request_privilege_routine, NULL);
}

static u32_t _impl_kernal_timer_schedule_request_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    timer_context_t *pCurTimer = (timer_context_t *)_timer_linker_head_fromWaiting();
    if (pCurTimer)
    {
        _impl_clock_time_interval_set(pCurTimer->duration_us);
    }
    else
    {
        _impl_clock_time_interval_set(0xFFFFFFFEu);
    }

    EXIT_CRITICAL_SECTION();

    return PC_SC_SUCCESS;
}

void _impl_timer_reamining_elapsed_handler(void)
{
    list_t *pListRunning = (list_t *)_timer_list_runningHeadGet();

    ENTER_CRITICAL_SECTION();
    struct callFunc *pCallFun = (struct callFunc *)list_node_pop(pListRunning, LIST_TAIL);
    EXIT_CRITICAL_SECTION();

    while (pCallFun)
    {
        if (pCallFun->pTimer)
        {
            pCallFun->pTimer();
        }

        ENTER_CRITICAL_SECTION();
        pCallFun = (struct callFunc *)list_node_pop(pListRunning, LIST_TAIL);
        EXIT_CRITICAL_SECTION();
    }
}

u64_t _impl_timer_total_system_get(void)
{
    return kernal_privilege_invoke(_impl_timer_total_system_get_privilege_routine, NULL);
}

static u64_t _impl_timer_total_system_get_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    UNUSED_MSG(pArgs);

    u64_t us = (!_s_remaining_elapsed_us) ? (_impl_clock_time_elapsed_get()) : (0u);

    us += _s_system_time_us;

    EXIT_CRITICAL_SECTION();

    return us;
}

void _impl_timer_elapsed_handler(u32_t elapsed_us)
{
    ENTER_CRITICAL_SECTION();

    timer_context_t *pCurTimer = NULL;
    _s_remaining_elapsed_us = elapsed_us;

    list_iterator_t it = {0u};
    list_t *pListWaiting = (list_t *)_timer_list_waitingHeadGet();
    list_iterator_init(&it, pListWaiting);
    while (list_iterator_next_condition(&it, (void *)&pCurTimer))
    {
        if (_s_remaining_elapsed_us >= pCurTimer->duration_us)
        {
            _s_remaining_elapsed_us -= pCurTimer->duration_us;
            _s_system_time_us += pCurTimer->duration_us;
            pCurTimer->duration_us = 0u;

            if (kernal_member_unified_id_toId(pCurTimer->head.id) == KERNAL_MEMBER_TIMER_INTERNAL)
            {
                _timer_list_transfer_toStopList((linker_head_t *)&pCurTimer->head);

                if (pCurTimer->call.pThread)
                {
                    pCurTimer->call.pThread(pCurTimer->head.id);
                }
                pCurTimer->call.pThread = NULL;
            }
            else
            {
                pCurTimer->duration_us = _s_system_time_us;
                _timer_list_transfer_toPendList((linker_head_t *)&pCurTimer->head);
            }
        }
        else
        {
            pCurTimer->duration_us -= _s_remaining_elapsed_us;
            break;
        }
    }

    _s_system_time_us += _s_remaining_elapsed_us;
    _s_remaining_elapsed_us = 0u;

    list_t *pListPending = (list_t *)_timer_list_pendingHeadGet();
    list_iterator_init(&it, pListPending);
    while (list_iterator_next_condition(&it, (void *)&pCurTimer))
    {
        if (pCurTimer->isCycle)
        {
            u64_t timeout_us = pCurTimer->timeout_ms * 1000u;
            u64_t elapsed_us = _s_system_time_us - pCurTimer->duration_us;

            while (elapsed_us >= timeout_us)
            {
                elapsed_us -= timeout_us;
            }
            pCurTimer->duration_us = timeout_us - elapsed_us;
            _timer_list_transfer_toWaitList((linker_head_t *)&pCurTimer->head);
        }
        else
        {
            pCurTimer->duration_us = 0u;
            _timer_list_transfer_toStopList((linker_head_t *)&pCurTimer->head);
        }
        list_t *pListRunning = (list_t *)_timer_list_runningHeadGet();
        if (!list_node_isExisted(pListRunning, &pCurTimer->call.node))
        {
            list_node_push(_timer_list_runningHeadGet(), &pCurTimer->call.node, LIST_HEAD);
            kernal_message_notification();
        }
    }
    _impl_kernal_timer_schedule_request();

    EXIT_CRITICAL_SECTION();
}

#ifdef __cplusplus
}
#endif
