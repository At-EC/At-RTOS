/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 **/

#include "kernal.h"
#include "clock_tick.h"
#include "timer.h"
#include "postcode.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Local unique postcode.
 */
#define _PC_CMPT_FAILED PC_FAILED(PC_CMPT_TIMER)

/**
 * Data structure for location timer
 */
typedef struct {
    /* The last load count value */
    u64_t system_us;

    /* The clock time total count value */
    u32_t remaining_us;
} _timer_resource_t;

/**
 * Local timer resource
 */
_timer_resource_t g_timer_resource = {0u};

/**
 * The local function lists for current file internal use.
 */
static u32_t _timer_init_privilege_routine(arguments_t *pArgs);
static u32_t _timer_start_privilege_routine(arguments_t *pArgs);
static u32_t _timer_stop_privilege_routine(arguments_t *pArgs);
static u32_t _timer_total_system_get_privilege_routine(arguments_t *pArgs);
static u32_t _kernal_timer_schedule_request_privilege_routine(arguments_t *pArgs);

/**
 * @brief Get the timer context based on provided unique id.
 *
 * @param id The timer unique id.
 *
 * @return The pointer of the current unique id timer context.
 */
static timer_context_t *_timer_object_contextGet(os_id_t id)
{
    return (timer_context_t *)(_impl_kernal_member_unified_id_toContainerAddress(id));
}

/**
 * @brief Get the stoping timer list head.
 *
 * @return The value of the stoping list head.
 */
static list_t *_timer_list_stopingHeadGet(void)
{
    return (list_t *)_impl_kernal_member_list_get(KERNAL_MEMBER_TIMER, KERNAL_MEMBER_LIST_TIMER_STOP);
}

/**
 * @brief Get the waiting timer list head.
 *
 * @return The value of the waiting list head.
 */
static list_t *_timer_list_waitingHeadGet(void)
{
    return (list_t *)_impl_kernal_member_list_get(KERNAL_MEMBER_TIMER, KERNAL_MEMBER_LIST_TIMER_WAIT);
}

/**
 * @brief Get the ending timer list head.
 *
 * @return The value of the ending list head.
 */
static list_t *_timer_list_endingHeadGet(void)
{
    return (list_t *)_impl_kernal_member_list_get(KERNAL_MEMBER_TIMER, KERNAL_MEMBER_LIST_TIMER_END);
}

/**
 * @brief Get the pending timer list head.
 *
 * @return The value of the pending list head.
 */
static list_t *_timer_list_pendingHeadGet(void)
{
    return (list_t *)_impl_kernal_member_list_get(KERNAL_MEMBER_TIMER, KERNAL_MEMBER_LIST_TIMER_PEND);
}

/**
 * @brief Get the running timer list head.
 *
 * @return The value of the running list head.
 */
static list_t *_timer_list_runningHeadGet(void)
{
    return (list_t *)_impl_kernal_member_list_get(KERNAL_MEMBER_TIMER, KERNAL_MEMBER_LIST_TIMER_RUN);
}

/**
 * @brief Remove a list node from the waiting list and push the duration time into the next node.
 *
 * @param The node linker head pointer.
 */
static void _timer_list_remove_fromWaitList(linker_head_t *pCurHead)
{
    ENTER_CRITICAL_SECTION();

    timer_context_t *pCurTimer = (timer_context_t *)&pCurHead->linker.node;
    timer_context_t *pNext = (timer_context_t *)pCurTimer->head.linker.node.pNext;

    if (pNext) {
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
 * @param pCurHead The pointer of the timer linker head.
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
 * @param pCurHead The pointer of the timer linker head.
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
 * @param pCurHead The pointer of the timer linker head.
 */
static void _timer_list_transfer_toPendList(linker_head_t *pCurHead)
{
    ENTER_CRITICAL_SECTION();

    list_t *pToPendList = (list_t *)_timer_list_pendingHeadGet();
    linker_list_transaction_common(&pCurHead->linker, pToPendList, LIST_TAIL);

    EXIT_CRITICAL_SECTION();
}

/**
 * @brief Compare the customization value between the current and extract thread.
 *
 * @param pCurNode The pointer of the current thread node.
 * @param pExtractNode The pointer of the extract thread node.
 *
 * @return The false value indicates it is right position and it can kiil the loop routine,
 *         otherwise is not a correct position.
 */
static b_t _timer_node_Order_compare_condition(list_node_t *pCurNode, list_node_t *pExtractNode)
{
    timer_context_t *pCurTimer = (timer_context_t *)pCurNode;
    timer_context_t *pExtractTimer = (timer_context_t *)pExtractNode;

    if ((!pCurTimer) || (!pExtractTimer)) {
        /* no available timer */
        return FALSE;
    }

    if (pCurTimer->duration_us > pExtractTimer->duration_us) {
        pCurTimer->duration_us -= pExtractTimer->duration_us;
        return TRUE;
    } else {
        pExtractTimer->duration_us -= pCurTimer->duration_us;
        return FALSE;
    }
}

/**
 * @brief Push one timer context into waiting list,
 *        but it has to compared the existed duration time, and calculate the divided time.
 *
 * @param pCurHead The pointer of the timer linker head.
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
 * @return The value of waiting linker head pointer.
 */
static linker_head_t *_timer_linker_head_fromWaiting(void)
{
    ENTER_CRITICAL_SECTION();
    list_t *pListWaiting = (list_t *)_timer_list_waitingHeadGet();
    EXIT_CRITICAL_SECTION();

    return (linker_head_t *)(pListWaiting->pHead);
}

/**
 * @brief Check if the timer unique id if is's invalid.
 *
 * @param id The provided unique id.
 *
 * @return The true is invalid, otherwise is valid.
 */
static b_t _timer_id_isInvalid(u32_t id)
{
    if (!_impl_kernal_member_unified_id_isInvalid(KERNAL_MEMBER_TIMER_INTERNAL, id)) {
        return FALSE;
    }

    if (!_impl_kernal_member_unified_id_isInvalid(KERNAL_MEMBER_TIMER, id)) {
        return FALSE;
    }

    return TRUE;
}

/**
 * @brief Check if the timer object if is's initialized.
 *
 * @param id The provided unique id.
 *
 * @return The true is initialized, otherwise is uninitialized.
 */
static b_t _timer_object_isInit(u32_t id)
{
    timer_context_t *pCurTimer = _timer_object_contextGet(id);

    return ((pCurTimer) ? (((pCurTimer->head.linker.pList) ? (TRUE) : (FALSE))) : FALSE);
}

/**
 * @brief Convert the internal os id to kernal member number.
 *
 * @param id The provided unique id.
 *
 * @return The value of member number.
 */
u32_t _impl_timer_os_id_to_number(u32_t id)
{
    if (!_impl_kernal_member_unified_id_isInvalid(KERNAL_MEMBER_TIMER_INTERNAL, id)) {
        return (u32_t)((id - _impl_kernal_member_id_toUnifiedIdStart(KERNAL_MEMBER_TIMER_INTERNAL)) / sizeof(timer_context_t));
    }

    if (!_impl_kernal_member_unified_id_isInvalid(KERNAL_MEMBER_TIMER, id)) {
        return (u32_t)((id - _impl_kernal_member_id_toUnifiedIdStart(KERNAL_MEMBER_TIMER)) / sizeof(timer_context_t));
    }

    return 0u;
}

/**
 * @brief Initialize a new timer.
 *
 * @param pCallFun The timer entry function pointer.
 * @param isCycle It indicates the timer if it's cycle repeat.
 * @param timeout_ms The expired time.
 * @param pName The timer's name, it supported NULL pointer.
 *
 * @return The value of the timer unique id.
 */
os_id_t _impl_timer_init(pTimer_callbackFunc_t pCallFun, b_t isCycle, u32_t timeout_ms, const char_t *pName)
{
    arguments_t arguments[] = {
        [0] = {.ptr_val = (const void *)pCallFun},
        [1] = {.b_val = (b_t)isCycle},
        [2] = {.u32_val = (u32_t)timeout_ms},
        [3] = {.pch_val = (const void *)pName},
    };

    return _impl_kernal_privilege_invoke((const void *)_timer_init_privilege_routine, arguments);
}

/**
 * @brief Initialize a timer for internal thread context use.
 *
 * @param id The thread unique id is same as timer.
 */
void _impl_thread_timer_init(os_id_t id)
{
    ENTER_CRITICAL_SECTION();

    timer_context_t *pCurTimer = _timer_object_contextGet(id);

    _memset((char_t *)pCurTimer, 0x0u, sizeof(timer_context_t));
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
 * @brief Timer start for internal thread context use.
 *
 * @param id The thread unique id is same as timer.
 * @param timeout_ms The thread timer timeout time.
 * @param pCallback The thread timeout callback function.
 */
void _impl_thread_timer_start(os_id_t id, u32_t timeout_ms, void (*pCallback)(os_id_t))
{
    ENTER_CRITICAL_SECTION();

    timer_context_t *pCurTimer = _timer_object_contextGet(id); // Only for internal thread use

    pCurTimer->call.pThread = pCallback;
    pCurTimer->timeout_ms = OS_TIME_FOREVER_VAL;
    pCurTimer->isCycle = FALSE;

    if (pCurTimer->head.linker.pList == _timer_list_waitingHeadGet()) {
        _timer_list_remove_fromWaitList((linker_head_t *)&pCurTimer->head);
    }

    if (timeout_ms == OS_TIME_FOREVER_VAL) {
        _timer_list_transfer_toEndList((linker_head_t *)&pCurTimer->head);
    } else {
        pCurTimer->duration_us = (timeout_ms * 1000u);
        _timer_list_transfer_toWaitList((linker_head_t *)&pCurTimer->head);
    }

    EXIT_CRITICAL_SECTION();
}

/**
 * @brief Timer starts operation, be careful if the timer's last time isn't expired or be handled,
 *        the newer start will override it.
 *
 * @param id The timer unique id.
 * @param isCycle It indicates the timer if it's cycle repeat.
 * @param timeout_ms The timer expired time.
 *
 * @return The result of timer start operation.
 */
u32p_t _impl_timer_start(os_id_t id, b_t isCycle, u32_t timeout_ms)
{
    if (_timer_id_isInvalid(id)) {
        return _PC_CMPT_FAILED;
    }

    if (!_timer_object_isInit(id)) {
        return _PC_CMPT_FAILED;
    }

    if (!timeout_ms) {
        return _PC_CMPT_FAILED;
    }

    arguments_t arguments[] = {
        [0] = {.u32_val = (u32_t)id},
        [1] = {.b_val = (u32_t)isCycle},
        [2] = {.u32_val = (u32_t)timeout_ms},
    };

    return _impl_kernal_privilege_invoke((const void *)_timer_start_privilege_routine, arguments);
}

/**
 * @brief timer stops operation.
 *
 * @param id The timer unique id.
 *
 * @return The result of timer stop operation.
 */
u32p_t _impl_timer_stop(os_id_t id)
{
    if (_timer_id_isInvalid(id)) {
        return _PC_CMPT_FAILED;
    }

    if (!_timer_object_isInit(id)) {
        return _PC_CMPT_FAILED;
    }

    arguments_t arguments[] = {
        [0] = {.u32_val = (u32_t)id},
    };

    return _impl_kernal_privilege_invoke((const void *)_timer_stop_privilege_routine, arguments);
}

/**
 * @brief Check the timer to confirm if it's already scheduled in the waiting list.
 *
 * @param id The timer unique id.
 *
 * @return The true result indicates time busy, otherwise is free status.
 */
b_t _impl_timer_status_isBusy(os_id_t id)
{
    if (_timer_id_isInvalid(id)) {
        return FALSE;
    }

    if (!_timer_object_isInit(id)) {
        return FALSE;
    }

    ENTER_CRITICAL_SECTION();

    linker_head_t *pCurHead = (linker_head_t *)_timer_object_contextGet(id);
    b_t isBusy = (pCurHead->linker.pList == (list_t *)_timer_list_waitingHeadGet());
    isBusy |= (pCurHead->linker.pList == (list_t *)_timer_list_endingHeadGet());

    EXIT_CRITICAL_SECTION();
    return isBusy;
}

/**
 * @brief Get the kernal RTOS system time (ms).
 *
 * @return The value of the total system time (ms).
 */
u32_t _impl_timer_total_system_get(void)
{
    return _impl_kernal_privilege_invoke((const void *)_timer_total_system_get_privilege_routine, NULL);
}

/**
 * @brief Kernal RTOS request to update new schedule.
 *
 * @return The result of timer schedule request.
 */
u32p_t _impl_kernal_timer_schedule_request(void)
{
    return _impl_kernal_privilege_invoke((const void *)_kernal_timer_schedule_request_privilege_routine, NULL);
}

/**
 * @brief It's sub-routine running at privilege mode.
 *
 * @param pArgs The function argument packages.
 *
 * @return The result of privilege routine.
 */
static u32_t _timer_init_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    pTimer_callbackFunc_t pCallFun = (pTimer_callbackFunc_t)(pArgs[0].ptr_val);
    b_t isCycle = (b_t)(pArgs[1].b_val);
    u32_t timeout_ms = (u32_t)(pArgs[2].u32_val);
    const char_t *pName = (const char_t *)pArgs[3].pch_val;
    u32_t endAddr = 0u;
    timer_context_t *pCurTimer = NULL;

    pCurTimer = (timer_context_t *)_impl_kernal_member_id_toContainerStartAddress(KERNAL_MEMBER_TIMER);
    endAddr = (u32_t)_impl_kernal_member_id_toContainerEndAddress(KERNAL_MEMBER_TIMER);
    do {
        os_id_t id = _impl_kernal_member_containerAddress_toUnifiedid((u32_t)pCurTimer);
        if (_timer_id_isInvalid(id)) {
            break;
        }

        if (_timer_object_isInit(id)) {
            continue;
        }

        _memset((char_t *)pCurTimer, 0x0u, sizeof(timer_context_t));
        pCurTimer->head.id = id;
        pCurTimer->head.pName = pName;

        pCurTimer->isCycle = isCycle;
        pCurTimer->timeout_ms = timeout_ms;
        pCurTimer->duration_us = 0u;
        pCurTimer->call.pTimer = pCallFun;

        _timer_list_transfer_toStopList((linker_head_t *)&pCurTimer->head);

        EXIT_CRITICAL_SECTION();
        return id;
    } while ((u32_t)++pCurTimer < endAddr);

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
static u32_t _timer_start_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    os_id_t id = (os_id_t)pArgs[0].u32_val;
    b_t isCycle = (b_t)pArgs[1].b_val;
    u32_t timeout_ms = (u32_t)pArgs[2].u32_val;
    timer_context_t *pCurTimer = NULL;

    pCurTimer = _timer_object_contextGet(id);
    pCurTimer->timeout_ms = timeout_ms;
    pCurTimer->isCycle = isCycle;

    if (pCurTimer->head.linker.pList == _timer_list_waitingHeadGet()) {
        _timer_list_remove_fromWaitList((linker_head_t *)&pCurTimer->head);
    }

    if (pCurTimer->timeout_ms == OS_TIME_FOREVER_VAL) {
        _timer_list_transfer_toEndList((linker_head_t *)&pCurTimer->head);
    } else {
        pCurTimer->duration_us = timeout_ms * 1000u;
        _timer_list_transfer_toWaitList((linker_head_t *)&pCurTimer->head);
    }
    _impl_kernal_timer_schedule_request();

    EXIT_CRITICAL_SECTION();
    return PC_SC_SUCCESS;
}

/**
 * @brief It's sub-routine running at privilege mode.
 *
 * @param pArgs The function argument packages.
 *
 * @return The result of privilege routine.
 */
static u32_t _timer_stop_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    os_id_t id = (os_id_t)pArgs[0].u32_val;

    timer_context_t *pCurTimer = NULL;

    pCurTimer = _timer_object_contextGet(id);
    if (pCurTimer->head.linker.pList == _timer_list_waitingHeadGet()) {
        _timer_list_remove_fromWaitList((linker_head_t *)&pCurTimer->head);
    }
    _timer_list_transfer_toStopList((linker_head_t *)&pCurTimer->head);

    _impl_kernal_timer_schedule_request();

    EXIT_CRITICAL_SECTION();
    return PC_SC_SUCCESS;
}

/**
 * @brief It's sub-routine running at privilege mode.
 *
 * @param pArgs The function argument packages.
 *
 * @return The result of privilege routine.
 */
static u32_t _timer_total_system_get_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    UNUSED_MSG(pArgs);

    u64_t us = (!g_timer_resource.remaining_us) ? (_impl_clock_time_elapsed_get()) : (0u);

    us += g_timer_resource.system_us;

    u32_t ms = ((us / 1000u) & 0xFFFFFFFFu);

    EXIT_CRITICAL_SECTION();
    return ms;
}

/**
 * @brief It's sub-routine running at privilege mode.
 *
 * @param pArgs The function argument packages.
 *
 * @return The result of privilege routine.
 */
static u32_t _kernal_timer_schedule_request_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    timer_context_t *pCurTimer = (timer_context_t *)_timer_linker_head_fromWaiting();
    if (pCurTimer) {
        _impl_clock_time_interval_set(pCurTimer->duration_us);
    } else {
        _impl_clock_time_interval_set(OS_TIME_FOREVER_VAL);
    }

    EXIT_CRITICAL_SECTION();
    return PC_SC_SUCCESS;
}

/**
 * @brief Timer callback function handle in the kernal thread.
 */
void _impl_timer_reamining_elapsed_handler(void)
{
    list_t *pListRunning = (list_t *)_timer_list_runningHeadGet();

    ENTER_CRITICAL_SECTION();
    struct callFunc *pCallFun = (struct callFunc *)list_node_pop(pListRunning, LIST_TAIL);
    EXIT_CRITICAL_SECTION();

    while (pCallFun) {
        if (pCallFun->pTimer) {
            pCallFun->pTimer();
        }

        ENTER_CRITICAL_SECTION();
        pCallFun = (struct callFunc *)list_node_pop(pListRunning, LIST_TAIL);
        EXIT_CRITICAL_SECTION();
    }
}

/**
 * @brief Kernal RTOS handle the clock time.
 *
 * @param elapsed_us Clock time reported elapsed time.
 */
void _impl_timer_elapsed_handler(u32_t elapsed_us)
{
    ENTER_CRITICAL_SECTION();

    timer_context_t *pCurTimer = NULL;
    g_timer_resource.remaining_us = elapsed_us;

    list_iterator_t it = {0u};
    list_t *pListWaiting = (list_t *)_timer_list_waitingHeadGet();
    list_iterator_init(&it, pListWaiting);
    while (list_iterator_next_condition(&it, (void *)&pCurTimer)) {
        if (g_timer_resource.remaining_us >= pCurTimer->duration_us) {
            g_timer_resource.remaining_us -= pCurTimer->duration_us;
            g_timer_resource.system_us += pCurTimer->duration_us;
            pCurTimer->duration_us = 0u;

            if (_impl_kernal_member_unified_id_toId(pCurTimer->head.id) == KERNAL_MEMBER_TIMER_INTERNAL) {
                _timer_list_transfer_toStopList((linker_head_t *)&pCurTimer->head);

                if (pCurTimer->call.pThread) {
                    pCurTimer->call.pThread(pCurTimer->head.id);
                }
                pCurTimer->call.pThread = NULL;
            } else {
                pCurTimer->duration_us = g_timer_resource.system_us;
                _timer_list_transfer_toPendList((linker_head_t *)&pCurTimer->head);
            }
        } else {
            pCurTimer->duration_us -= g_timer_resource.remaining_us;
            break;
        }
    }

    g_timer_resource.system_us += g_timer_resource.remaining_us;
    g_timer_resource.remaining_us = 0u;

    b_t request = FALSE;
    list_t *pListPending = (list_t *)_timer_list_pendingHeadGet();
    list_iterator_init(&it, pListPending);
    while (list_iterator_next_condition(&it, (void *)&pCurTimer)) {
        if (pCurTimer->isCycle) {
            u64_t timeout_us = pCurTimer->timeout_ms * 1000u;
            u64_t elapsed_us = g_timer_resource.system_us - pCurTimer->duration_us;

            while (elapsed_us >= timeout_us) {
                elapsed_us -= timeout_us;
            }
            pCurTimer->duration_us = timeout_us - elapsed_us;
            _timer_list_transfer_toWaitList((linker_head_t *)&pCurTimer->head);
        } else {
            pCurTimer->duration_us = 0u;
            _timer_list_transfer_toStopList((linker_head_t *)&pCurTimer->head);
        }
        list_t *pListRunning = (list_t *)_timer_list_runningHeadGet();
        if (!list_node_isExisted(pListRunning, &pCurTimer->call.node)) {
            list_node_push(_timer_list_runningHeadGet(), &pCurTimer->call.node, LIST_HEAD);
        }
        request = TRUE;
    }

    if (request) {
        _impl_kernal_message_notification();
    }
    _impl_kernal_timer_schedule_request();

    EXIT_CRITICAL_SECTION();
}

#ifdef __cplusplus
}
#endif
