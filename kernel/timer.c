/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 **/

#include "kernel.h"
#include "clock_tick.h"
#include "timer.h"
#include "postcode.h"
#include "trace.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Local unique postcode.
 */
#define _PCER PC_IER(PC_OS_CMPT_TIMER_8)

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
 * @brief Get the timer context based on provided unique id.
 *
 * @param id The timer unique id.
 *
 * @return The pointer of the current unique id timer context.
 */
static timer_context_t *_timer_object_contextGet(os_id_t id)
{
    return (timer_context_t *)(kernel_member_unified_id_toContainerAddress(id));
}

/**
 * @brief Get the stoping timer list head.
 *
 * @return The value of the stoping list head.
 */
static list_t *_timer_list_stopingHeadGet(void)
{
    return (list_t *)kernel_member_list_get(KERNEL_MEMBER_TIMER, KERNEL_MEMBER_LIST_TIMER_STOP);
}

/**
 * @brief Get the waiting timer list head.
 *
 * @return The value of the waiting list head.
 */
static list_t *_timer_list_waitingHeadGet(void)
{
    return (list_t *)kernel_member_list_get(KERNEL_MEMBER_TIMER, KERNEL_MEMBER_LIST_TIMER_WAIT);
}

/**
 * @brief Get the ending timer list head.
 *
 * @return The value of the ending list head.
 */
static list_t *_timer_list_endingHeadGet(void)
{
    return (list_t *)kernel_member_list_get(KERNEL_MEMBER_TIMER, KERNEL_MEMBER_LIST_TIMER_END);
}

/**
 * @brief Get the pending timer list head.
 *
 * @return The value of the pending list head.
 */
static list_t *_timer_list_pendingHeadGet(void)
{
    return (list_t *)kernel_member_list_get(KERNEL_MEMBER_TIMER, KERNEL_MEMBER_LIST_TIMER_PEND);
}

/**
 * @brief Get the running timer list head.
 *
 * @return The value of the running list head.
 */
static list_t *_timer_list_runningHeadGet(void)
{
    return (list_t *)kernel_member_list_get(KERNEL_MEMBER_TIMER, KERNEL_MEMBER_LIST_TIMER_RUN);
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
    if (!kernel_member_unified_id_isInvalid(KERNEL_MEMBER_TIMER_INTERNAL, id)) {
        return FALSE;
    }

    if (!kernel_member_unified_id_isInvalid(KERNEL_MEMBER_TIMER, id)) {
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
 * @brief It's sub-routine running at privilege mode.
 *
 * @param pArgs The function argument packages.
 *
 * @return The result of privilege routine.
 */
static i32p_t _timer_schedule_request_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    timer_context_t *pCurTimer = (timer_context_t *)_timer_linker_head_fromWaiting();
    if (pCurTimer) {
        clock_time_interval_set(pCurTimer->duration_us);
    } else {
        clock_time_interval_set(OS_TIME_FOREVER_VAL);
    }

    EXIT_CRITICAL_SECTION();
    return 0;
}

/**
 * @brief kernel RTOS request to update new schedule.
 *
 * @return The result of timer schedule request.
 */
static i32p_t _timer_schedule(void)
{
    return kernel_privilege_invoke((const void *)_timer_schedule_request_privilege_routine, NULL);
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

    pCurTimer = (timer_context_t *)kernel_member_id_toContainerStartAddress(KERNEL_MEMBER_TIMER);
    endAddr = (u32_t)kernel_member_id_toContainerEndAddress(KERNEL_MEMBER_TIMER);
    do {
        os_id_t id = kernel_member_containerAddress_toUnifiedid((u32_t)pCurTimer);
        if (_timer_id_isInvalid(id)) {
            break;
        }

        if (_timer_object_isInit(id)) {
            continue;
        }

        os_memset((char_t *)pCurTimer, 0x0u, sizeof(timer_context_t));
        pCurTimer->head.id = id;
        pCurTimer->head.pName = pName;

        pCurTimer->isCycle = isCycle;
        pCurTimer->timeout_ms = timeout_ms;
        pCurTimer->duration_us = 0u;
        pCurTimer->callEntry.pTimerCallEntry = pCallFun;

        _timer_list_transfer_toStopList((linker_head_t *)&pCurTimer->head);

        EXIT_CRITICAL_SECTION();
        return id;
    } while ((u32_t)++pCurTimer < endAddr);

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
    _timer_schedule();

    EXIT_CRITICAL_SECTION();
    return 0;
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

    _timer_schedule();

    EXIT_CRITICAL_SECTION();
    return 0;
}

/**
 * @brief It's sub-routine running at privilege mode.
 *
 * @param pArgs The function argument packages.
 *
 * @return The result of privilege routine.
 */
static u32_t _timer_total_system_ms_get_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    UNUSED_MSG(pArgs);

    u64_t us = (!g_timer_resource.remaining_us) ? (clock_time_elapsed_get()) : (0u);

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
static u32_t _timer_total_system_us_get_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    UNUSED_MSG(pArgs);

    u32_t us = (u32_t)((!g_timer_resource.remaining_us) ? (clock_time_elapsed_get()) : (0u));

    us += g_timer_resource.system_us;

    EXIT_CRITICAL_SECTION();
    return us;
}

/**
 * @brief Convert the internal os id to kernel member number.
 *
 * @param id The provided unique id.
 *
 * @return The value of member number.
 */
u32_t _impl_timer_os_id_to_number(u32_t id)
{
    if (!kernel_member_unified_id_isInvalid(KERNEL_MEMBER_TIMER_INTERNAL, id)) {
        return (u32_t)((id - kernel_member_id_toUnifiedIdStart(KERNEL_MEMBER_TIMER_INTERNAL)) / sizeof(timer_context_t));
    }

    if (!kernel_member_unified_id_isInvalid(KERNEL_MEMBER_TIMER, id)) {
        return (u32_t)((id - kernel_member_id_toUnifiedIdStart(KERNEL_MEMBER_TIMER)) / sizeof(timer_context_t));
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

    return kernel_privilege_invoke((const void *)_timer_init_privilege_routine, arguments);
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
i32p_t _impl_timer_start(os_id_t id, b_t isCycle, u32_t timeout_ms)
{
    if (_timer_id_isInvalid(id)) {
        return _PCER;
    }

    if (!_timer_object_isInit(id)) {
        return _PCER;
    }

    if (!timeout_ms) {
        return _PCER;
    }

    arguments_t arguments[] = {
        [0] = {.u32_val = (u32_t)id},
        [1] = {.b_val = (u32_t)isCycle},
        [2] = {.u32_val = (u32_t)timeout_ms},
    };

    return kernel_privilege_invoke((const void *)_timer_start_privilege_routine, arguments);
}

/**
 * @brief timer stops operation.
 *
 * @param id The timer unique id.
 *
 * @return The result of timer stop operation.
 */
i32p_t _impl_timer_stop(os_id_t id)
{
    if (_timer_id_isInvalid(id)) {
        return _PCER;
    }

    if (!_timer_object_isInit(id)) {
        return _PCER;
    }

    arguments_t arguments[] = {
        [0] = {.u32_val = (u32_t)id},
    };

    return kernel_privilege_invoke((const void *)_timer_stop_privilege_routine, arguments);
}

/**
 * @brief Check the timer to confirm if it's already scheduled in the waiting list.
 *
 * @param id The timer unique id.
 *
 * @return The true result indicates time busy, otherwise is free status.
 */
b_t _impl_timer_busy(os_id_t id)
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
 * @brief Get the kernel RTOS system time (ms).
 *
 * @return The value of the total system time (ms).
 */
u32_t _impl_timer_total_system_ms_get(void)
{
    return kernel_privilege_invoke((const void *)_timer_total_system_ms_get_privilege_routine, NULL);
}

/**
 * @brief Get the kernel RTOS system time (us).
 *
 * @return The value of the total system time (us).
 */
u32_t _impl_timer_total_system_us_get(void)
{
    return kernel_privilege_invoke((const void *)_timer_total_system_us_get_privilege_routine, NULL);
}

/**
 * @brief Initialize a timer for internal thread context use.
 *
 * @param id The thread unique id is same as timer.
 */
void timer_init_for_thread(os_id_t id)
{
    ENTER_CRITICAL_SECTION();

    timer_context_t *pCurTimer = _timer_object_contextGet(id);

    os_memset((char_t *)pCurTimer, 0x0u, sizeof(timer_context_t));
    pCurTimer->head.id = id;
    pCurTimer->head.pName = "TH";

    pCurTimer->isCycle = FALSE;
    pCurTimer->timeout_ms = 0u;
    pCurTimer->duration_us = 0u;
    pCurTimer->callEntry.pThreadCallEntry = NULL;

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
void timer_start_for_thread(os_id_t id, u32_t timeout_ms, void (*pCallback)(os_id_t))
{
    ENTER_CRITICAL_SECTION();

    timer_context_t *pCurTimer = _timer_object_contextGet(id); // Only for internal thread use

    pCurTimer->callEntry.pThreadCallEntry = pCallback;
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
 * @brief Timer callback function handle in the kernel thread.
 */
void timer_reamining_elapsed_handler(void)
{
    list_t *pListRunning = (list_t *)_timer_list_runningHeadGet();

    ENTER_CRITICAL_SECTION();
    struct callTimerEntry *pCallFunEntry = (struct callTimerEntry *)list_node_pop(pListRunning, LIST_TAIL);
    EXIT_CRITICAL_SECTION();

    while (pCallFunEntry) {
        if (pCallFunEntry->pTimerCallEntry) {
            pCallFunEntry->pTimerCallEntry();
        }

        ENTER_CRITICAL_SECTION();
        pCallFunEntry = (struct callTimerEntry *)list_node_pop(pListRunning, LIST_TAIL);
        EXIT_CRITICAL_SECTION();
    }
}

/**
 * @brief kernel RTOS handle the clock time.
 *
 * @param elapsed_us Clock time reported elapsed time.
 */
void timer_elapsed_handler(u32_t elapsed_us)
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

            if (kernel_member_unified_id_toId(pCurTimer->head.id) == KERNEL_MEMBER_TIMER_INTERNAL) {
                _timer_list_transfer_toStopList((linker_head_t *)&pCurTimer->head);

                if (pCurTimer->callEntry.pThreadCallEntry) {
                    pCurTimer->callEntry.pThreadCallEntry(pCurTimer->head.id);
                }
                pCurTimer->callEntry.pThreadCallEntry = NULL;
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
        if (!list_node_isExisted(pListRunning, &pCurTimer->callEntry.node)) {
            list_node_push(_timer_list_runningHeadGet(), &pCurTimer->callEntry.node, LIST_HEAD);
        }
        request = TRUE;
    }

    if (request) {
        kernel_message_notification();
    }
    _timer_schedule();

    EXIT_CRITICAL_SECTION();
}

/**
 * @brief Get timer snapshot informations.
 *
 * @param instance The timer instance number.
 * @param pMsgs The kernel snapshot information pointer.
 *
 * @return TRUE: Operation pass, FALSE: Operation failed.
 */
b_t timer_snapshot(u32_t instance, kernel_snapshot_t *pMsgs)
{
#if defined KTRACE
    timer_context_t *pCurTimer = NULL;
    u32_t offset = 0u;
    os_id_t id = OS_INVALID_ID_VAL;

    ENTER_CRITICAL_SECTION();

    offset = sizeof(timer_context_t) * instance;
    pCurTimer = (timer_context_t *)(kernel_member_id_toContainerStartAddress(KERNEL_MEMBER_TIMER) + offset);
    id = kernel_member_containerAddress_toUnifiedid((u32_t)pCurTimer);
    os_memset((u8_t *)pMsgs, 0x0u, sizeof(kernel_snapshot_t));

    if (_timer_id_isInvalid(id)) {
        EXIT_CRITICAL_SECTION();
        return FALSE;
    }

    if (pCurTimer->head.linker.pList == _timer_list_stopingHeadGet()) {
        pMsgs->pState = "stop";
    } else if (pCurTimer->head.linker.pList == _timer_list_waitingHeadGet()) {
        pMsgs->pState = "wait";
    } else if (pCurTimer->head.linker.pList == _timer_list_endingHeadGet()) {
        pMsgs->pState = "end";
    } else if (pCurTimer->head.linker.pList == _timer_list_pendingHeadGet()) {
        pMsgs->pState = "pend";
    } else if (pCurTimer->head.linker.pList == _timer_list_runningHeadGet()) {
        pMsgs->pState = "run";
    } else if (pCurTimer->head.linker.pList) {
        pMsgs->pState = "*";
    } else {
        pMsgs->pState = "unused";

        EXIT_CRITICAL_SECTION();
        return FALSE;
    }

    pMsgs->id = pCurTimer->head.id;
    pMsgs->pName = pCurTimer->head.pName;

    pMsgs->timer.is_cycle = pCurTimer->isCycle;
    pMsgs->timer.timeout_ms = pCurTimer->timeout_ms;

    EXIT_CRITICAL_SECTION();
    return TRUE;
#else
    return FALSE;
#endif
}

/**
 * @brief timer stops operation.
 *
 * @param id The timer unique id.
 *
 * @return The result of timer stop operation.
 */
i32p_t timer_stop_for_thread(os_id_t id)
{
    if (kernel_member_unified_id_toId(id) != KERNEL_MEMBER_TIMER_INTERNAL) {
        return _PCER;
    }

    return _impl_timer_stop(id);
}

/**
 * @brief Check the timer to confirm if it's already scheduled in the waiting list.
 *
 * @param id The timer unique id.
 *
 * @return The true result indicates time busy, otherwise is free status.
 */
b_t timer_busy(os_id_t id)
{
    return _impl_timer_busy(id);
}

/**
 * @brief Get the kernel RTOS system time (ms).
 *
 * @return The value of the total system time (ms).
 */
u32_t timer_total_system_ms_get(void)
{
    return _impl_timer_total_system_ms_get();
}

/**
 * @brief Get the kernel RTOS system time (us).
 *
 * @return The value of the total system time (us).
 */
u32_t timer_total_system_us_get(void)
{
    return _impl_timer_total_system_us_get();
}

/**
 * @brief kernel RTOS request to update new schedule.
 *
 * @return The result of timer schedule request.
 */
i32p_t timer_schedule(void)
{
    return _timer_schedule();
}

#ifdef __cplusplus
}
#endif
