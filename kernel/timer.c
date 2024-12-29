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
#define PC_EOR PC_IER(PC_OS_CMPT_TIMER_8)

/**
 * Data structure for location timer
 */
typedef struct {
    /* The last load count value */
    u64_t system_us;

    /* The clock time total count value */
    u32_t remaining_us;

    list_t tt_wait_list;

    list_t tt_pend_list;

    list_t tt_idle_list;

    list_t callback_list;
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
static timer_context_t *_timer_context_get(os_id_t id)
{
    return (timer_context_t *)(kernel_member_unified_id_toContainerAddress(id));
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
static b_t _timer_id_isInit(u32_t id)
{
    timer_context_t *pCurTimer = _timer_context_get(id);

    return ((pCurTimer) ? (((pCurTimer->head.cs) ? (TRUE) : (FALSE))) : FALSE);
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
static b_t _timeout_node_Order_compare_condition(list_node_t *pCurNode, list_node_t *pExtractNode)
{
    struct expired_time *pCurTime = (struct expired_time *)pCurNode;
    struct expired_time *pExtractTime = (struct expired_time *)pExtractNode;

    if ((!pCurTime) || (!pExtractTime)) {
        /* no available timer */
        return false;
    }

    if (pCurTime->duration_us >= pExtractTime->duration_us) {
        pCurTime->duration_us -= pExtractTime->duration_us;
        return true;
    } else {
        pExtractTime->duration_us -= pCurTime->duration_us;
        return false;
    }
}

/**
 * @brief Push one timer context into waiting list,
 *        but it has to compared the existed duration time, and calculate the divided time.
 *
 * @param pCurHead The pointer of the timer linker head.
 */
static void _timeout_transfer_toWaitList(linker_t *pLinker)
{
    ENTER_CRITICAL_SECTION();

    list_t *pToTimeoutList = (list_t *)&g_timer_resource.tt_wait_list;
    linker_list_transaction_specific(pLinker, pToTimeoutList, _timeout_node_Order_compare_condition);

    EXIT_CRITICAL_SECTION();
}

/**
 * @brief Push one timer context into waiting list,
 *        but it has to compared the existed duration time, and calculate the divided time.
 *
 * @param pCurHead The pointer of the timer linker head.
 */
static void _timeout_transfer_toPendList(linker_t *pLinker)
{
    ENTER_CRITICAL_SECTION();

    list_t *pToTimeoutList = (list_t *)&g_timer_resource.tt_pend_list;
    linker_list_transaction_common(pLinker, pToTimeoutList, LIST_TAIL);

    EXIT_CRITICAL_SECTION();
}

/**
 * @brief Push one timer context into waiting list,
 *        but it has to compared the existed duration time, and calculate the divided time.
 *
 * @param pCurHead The pointer of the timer linker head.
 */
static void _timeout_transfer_toIdleList(linker_t *pLinker)
{
    ENTER_CRITICAL_SECTION();

    list_t *pToTimeoutList = (list_t *)&g_timer_resource.tt_idle_list;
    linker_list_transaction_common(pLinker, pToTimeoutList, LIST_TAIL);

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
static void _timeout_transfer_toNoInitList(linker_t *pLinker)
{
    ENTER_CRITICAL_SECTION();

    linker_list_transaction_common(pLinker, NULL, LIST_TAIL);

    EXIT_CRITICAL_SECTION();
}

/**
 * @brief Remove a list node from the waiting list and push the duration time into the next node.
 *
 * @param The node linker head pointer.
 */
static void _timeout_remove_fromWaitList(linker_t *pLinker)
{
    ENTER_CRITICAL_SECTION();

    struct expired_time *pCurTime = (struct expired_time *)&pLinker->node;
    struct expired_time *pNext = (struct expired_time *)pCurTime->linker.node.pNext;

    if (pNext) {
        pNext->duration_us += pCurTime->duration_us;
    }
    pCurTime->duration_us = 0u;

    EXIT_CRITICAL_SECTION();
}

static void _timeout_schedule(void)
{
    ENTER_CRITICAL_SECTION();

    struct expired_time *pCurExpired = (struct expired_time *)g_timer_resource.tt_wait_list.pHead;
    if (pCurExpired) {
        clock_time_interval_set(pCurExpired->duration_us);
    } else {
        clock_time_interval_set(OS_TIME_FOREVER_VAL);
    }

    EXIT_CRITICAL_SECTION();
}

static void _timer_callback_fromTimeOut(void *pLinker)
{
    timer_context_t* pCurTimer = (timer_context_t*)CONTAINEROF(pLinker, timer_context_t, expire);
    struct expired_time *pExpired = (struct expired_time *)&pCurTimer->expire;

    if (pCurTimer->control == TIMER_CTRL_CYCLE_VAL) {
        u64_t timeout_us = pCurTimer->timeout_ms * 1000u;
        u64_t elapsed_us = g_timer_resource.system_us - pExpired->duration_us;

        while (elapsed_us >= timeout_us) {
            elapsed_us -= timeout_us;
        }
        pExpired->duration_us = timeout_us - elapsed_us;
        _timeout_transfer_toWaitList((linker_t *)&pExpired->linker);
    } else if (pCurTimer->control == TIMER_CTRL_ONCE_VAL) {
        _timeout_transfer_toIdleList((linker_t *)&pExpired->linker);
    } else if (pCurTimer->control == TIMER_CTRL_TEMPORARY_VAL) {
        _timeout_transfer_toNoInitList((linker_t *)&pExpired->linker);
        os_memset((u8_t *)pCurTimer, 0u, sizeof(timer_context_t));
    }

    list_t *pCallback_list = (list_t *)&g_timer_resource.callback_list;
    if (!list_node_isExisted(pCallback_list, &pCurTimer->call.node)) {
        list_node_push((list_t *)&g_timer_resource.callback_list, &pCurTimer->call.node, LIST_HEAD);
    }
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

    _timeout_schedule();

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
    const char_t *pName = (const char_t *)pArgs[1].pch_val;
    u32_t endAddr = 0u;
    timer_context_t *pCurTimer = NULL;

    pCurTimer = (timer_context_t *)kernel_member_id_toContainerStartAddress(KERNEL_MEMBER_TIMER);
    endAddr = (u32_t)kernel_member_id_toContainerEndAddress(KERNEL_MEMBER_TIMER);
    do {
        os_id_t id = kernel_member_containerAddress_toUnifiedid((u32_t)pCurTimer);
        if (_timer_id_isInvalid(id)) {
            break;
        }

        if (_timer_id_isInit(id)) {
            continue;
        }

        os_memset((char_t *)pCurTimer, 0x0u, sizeof(timer_context_t));
        pCurTimer->head.cs = CS_INITED;
        pCurTimer->head.pName = pName;
        pCurTimer->call.pTimerCallEntry = pCallFun;
        timeout_init(&pCurTimer->expire, _timer_callback_fromTimeOut);

        EXIT_CRITICAL_SECTION();
        return id;
    }
    while ((u32_t)++pCurTimer < endAddr);

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
static u32_t _timer_automatic_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    pTimer_callbackFunc_t pCallFun = (pTimer_callbackFunc_t)(pArgs[0].ptr_val);
    const char_t *pName = (const char_t *)pArgs[1].pch_val;
    u32_t endAddr = 0u;
    timer_context_t *pCurTimer = NULL;

    pCurTimer = (timer_context_t *)kernel_member_id_toContainerStartAddress(KERNEL_MEMBER_TIMER);
    endAddr = (u32_t)kernel_member_id_toContainerEndAddress(KERNEL_MEMBER_TIMER);
    do {
        os_id_t id = kernel_member_containerAddress_toUnifiedid((u32_t)pCurTimer);
        if (_timer_id_isInvalid(id)) {
            break;
        }

        if (_timer_id_isInit(id)) {
            continue;
        }

        os_memset((char_t *)pCurTimer, 0x0u, sizeof(timer_context_t));
        pCurTimer->head.cs = CS_INITED;
        pCurTimer->head.pName = pName;
        pCurTimer->control = TIMER_CTRL_TEMPORARY_VAL;
        pCurTimer->call.pTimerCallEntry = pCallFun;
        timeout_init(&pCurTimer->expire, _timer_callback_fromTimeOut);

        EXIT_CRITICAL_SECTION();
        return id;
    }
    while ((u32_t)++pCurTimer < endAddr);

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
    u8_t ctrl = (u8_t)pArgs[1].u8_val;
    u32_t timeout_ms = (u32_t)pArgs[2].u32_val;

    timer_context_t *pCurTimer = _timer_context_get(id);
    pCurTimer->timeout_ms = timeout_ms;
    pCurTimer->control = ctrl;
    timeout_set(&pCurTimer->expire, timeout_ms, true);

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

    timer_context_t *pCurTimer = _timer_context_get(id);
    timeout_remove(&pCurTimer->expire, true);

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
 * @brief Initialize a new timer, or allocate a temporary timer to run.
 *
 * @param pCallFun The timer entry function pointer.
 * @param pName The timer's name, it supported NULL pointer.
 *
 * @return The value of the timer unique id.
 */
os_id_t _impl_timer_init(pTimer_callbackFunc_t pCallFun, const char_t *pName)
{
    arguments_t arguments[] = {
        [0] = {.ptr_val = (const void *)pCallFun},
        [1] = {.pch_val = (const void *)pName},
    };

    return kernel_privilege_invoke((const void *)_timer_init_privilege_routine, arguments);
}

/**
 * @brief Allocate a temporary timer to run.
 *
 * @param pCallFun The timer entry function pointer.
 * @param pName The timer's name, it supported NULL pointer.
 *
 * @return The value of the timer unique id.
 */
os_id_t _impl_timer_automatic(pTimer_callbackFunc_t pCallFun, const char_t *pName)
{
    arguments_t arguments[] = {
        [0] = {.ptr_val = (const void *)pCallFun},
        [1] = {.pch_val = (const void *)pName},
    };

    return kernel_privilege_invoke((const void *)_timer_automatic_privilege_routine, arguments);
}


/**
 * @brief Timer starts operation, be careful if the timer's last time isn't expired or be handled,
 *        the newer start will override it.
 *
 * @param id The timer unique id.
 * @param control It defines the timer running mode.
 * @param timeout_ms The timer expired time.
 *
 * @return The result of timer start operation.
 */
i32p_t _impl_timer_start(os_id_t id, u8_t control, u32_t timeout_ms)
{
    if (_timer_id_isInvalid(id)) {
        return PC_EOR;
    }

    if (!_timer_id_isInit(id)) {
        return PC_EOR;
    }

    if (!timeout_ms) {
        return PC_EOR;
    }

    if ((control != TIMER_CTRL_ONCE_VAL) && (control != TIMER_CTRL_CYCLE_VAL)) {
        return PC_EOR;
    }

    arguments_t arguments[] = {
        [0] = {.u32_val = (u32_t)id},
        [1] = {.u8_val = (u8_t)control},
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
        return PC_EOR;
    }

    if (!_timer_id_isInit(id)) {
        return PC_EOR;
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

    if (!_timer_id_isInit(id)) {
        return FALSE;
    }

    ENTER_CRITICAL_SECTION();
    timer_context_t* pCurTimer = _timer_context_get(id);
    b_t isBusy = (pCurTimer->expire.linker.pList == (list_t *)&g_timer_resource.tt_wait_list) ? TRUE : FALSE;

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

/**
 * @brief Timer callback function handle in the kernel thread.
 */
void timer_reamining_elapsed_handler(void)
{
    list_t *pListRunning = (list_t *)&g_timer_resource.callback_list;

    ENTER_CRITICAL_SECTION();
    struct timer_callback *pCallFunEntry = (struct timer_callback *)list_node_pop(pListRunning, LIST_TAIL);
    EXIT_CRITICAL_SECTION();

    while (pCallFunEntry) {
        if (pCallFunEntry->pTimerCallEntry) {
            pCallFunEntry->pTimerCallEntry();
        }

        ENTER_CRITICAL_SECTION();
        pCallFunEntry = (struct timer_callback *)list_node_pop(pListRunning, LIST_TAIL);
        EXIT_CRITICAL_SECTION();
    }
}

void timeout_init(struct expired_time *pExpire, pTimeout_callbackFunc_t fun)
{
    pExpire->duration_us = 0u;
    pExpire->fn = fun;
    _timeout_transfer_toIdleList((linker_t *)pExpire);
}

void timeout_set(struct expired_time *pExpire, u32_t timeout_ms, b_t immediately)
{
    ENTER_CRITICAL_SECTION();
    b_t need = false;
    if (pExpire->linker.pList == &g_timer_resource.tt_wait_list) {
        _timeout_remove_fromWaitList((linker_t *)&pExpire->linker);
        need = true;
    }

    if (timeout_ms == OS_TIME_FOREVER_VAL) {
        _timeout_transfer_toIdleList((linker_t *)&pExpire->linker);
    } else {
        pExpire->duration_us = timeout_ms * 1000u;
        _timeout_transfer_toWaitList((linker_t *)&pExpire->linker);
        need = true;
    }

    if (need && immediately) {
        _timer_schedule();
    }
    EXIT_CRITICAL_SECTION();
}

void timeout_remove(struct expired_time *pExpire, b_t immediately)
{
    ENTER_CRITICAL_SECTION();

    b_t need = false;
    if (pExpire->linker.pList == &g_timer_resource.tt_wait_list) {
        _timeout_remove_fromWaitList((linker_t *)&pExpire->linker);
        need = true;
    }
    _timeout_transfer_toIdleList((linker_t *)&pExpire->linker);

    if (need && immediately) {
        _timer_schedule();
    }

    EXIT_CRITICAL_SECTION();
}

/**
 * @brief kernel RTOS handle the clock time.
 *
 * @param elapsed_us Clock time reported elapsed time.
 */
void timeout_handler(u32_t elapsed_us)
{
    ENTER_CRITICAL_SECTION();

    struct expired_time *pCurExpired = NULL;
    g_timer_resource.remaining_us = elapsed_us;

    list_iterator_t it = {0u};
    list_t *pListWaiting = (list_t *)&g_timer_resource.tt_wait_list;
    list_iterator_init(&it, pListWaiting);
    while (list_iterator_next_condition(&it, (void *)&pCurExpired)) {
        if (g_timer_resource.remaining_us >= pCurExpired->duration_us) {
            g_timer_resource.remaining_us -= pCurExpired->duration_us;
            g_timer_resource.system_us += pCurExpired->duration_us;
            pCurExpired->duration_us = 0u;

            if (pCurExpired->fn != _timer_callback_fromTimeOut) {
                pCurExpired->fn((void *)pCurExpired);

                _timeout_transfer_toIdleList((linker_t *)&pCurExpired->linker);
            } else {
                pCurExpired->duration_us = g_timer_resource.system_us;
                _timeout_transfer_toPendList((linker_t *)&pCurExpired->linker);
            }
        } else {
            pCurExpired->duration_us -= g_timer_resource.remaining_us;
            break;
        }
    }

    g_timer_resource.system_us += g_timer_resource.remaining_us;
    g_timer_resource.remaining_us = 0u;

    b_t need = FALSE;
    list_t *pListPending = (list_t *)&g_timer_resource.tt_pend_list;
    list_iterator_init(&it, pListPending);
    while (list_iterator_next_condition(&it, (void *)&pCurExpired)) {
        if (pCurExpired->fn != NULL) {
            pCurExpired->fn((void *)pCurExpired);
            need = TRUE;
        }
    }

    if (need) {
        kernel_message_notification();
    }
    _timer_schedule();

    EXIT_CRITICAL_SECTION();
}

#ifdef __cplusplus
}
#endif
