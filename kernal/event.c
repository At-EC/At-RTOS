/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "basic.h"
#include "kernal.h"
#include "timer.h"
#include "event.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Local unique postcode.
 */
#define _PC_CMPT_FAILED       PC_FAILED(PC_CMPT_EVENT)

/**
 * The local function lists for current file internal use.
 */
static u32_t _event_init_privilege_routine(arguments_t *pArgs);
static u32_t _event_set_privilege_routine(arguments_t *pArgs);
static u32_t _event_wait_privilege_routine(arguments_t *pArgs);

static void _event_schedule(os_id_t id);

/**
 * @brief Get the event context based on provided unique id.
 *
 * @param id The event unique id.
 *
 * @return The pointer of the current unique id event context.
 */
static event_context_t* _event_object_contextGet(os_id_t id)
{
    return (event_context_t*)(_impl_kernal_member_unified_id_toContainerAddress(id));
}

/**
 * @brief Get the active event list head.
 *
 * @return The value of the active list head.
 */
static list_t* _event_list_activeHeadGet(void)
{
    return (list_t*)_impl_kernal_member_list_get(KERNAL_MEMBER_EVENT, KERNAL_MEMBER_LIST_EVENT_ACTIVE);
}

/**
 * @brief Pick up a highest priority thread that blocking by the event pending list.
 *
 * @param The semaphore unique id.
 *
 * @return The highest blocking thread head.
 */
static list_t* _event_list_blockingHeadGet(os_id_t id)
{
    event_context_t *pCurEvent = (event_context_t *)_event_object_contextGet(id);

    return (list_t*)((pCurEvent) ? (&pCurEvent->blockingThreadHead) : (NULL));
}

/**
 * @brief Push one event context into active list.
 *
 * @param pCurHead The pointer of the timer linker head.
 */
static void _event_list_transfer_toActive(linker_head_t *pCurHead)
{
    ENTER_CRITICAL_SECTION();

    list_t *pToActiveList = (list_t *)_event_list_activeHeadGet();
    linker_list_transaction_common(&pCurHead->linker, pToActiveList, LIST_TAIL);

    EXIT_CRITICAL_SECTION();
}

/**
 * @brief Check if the event unique id if is's invalid.
 *
 * @param id The provided unique id.
 *
 * @return The true is invalid, otherwise is valid.
 */
static b_t _event_id_isInvalid(u32_t id)
{
    return _impl_kernal_member_unified_id_isInvalid(KERNAL_MEMBER_EVENT, id);
}

/**
 * @brief Check if the event object if is's initialized.
 *
 * @param id The provided unique id.
 *
 * @return The true is initialized, otherwise is uninitialized.
 */
static b_t _event_object_isInit(u32_t id)
{
    event_context_t *pCurEvent = (event_context_t *)_event_object_contextGet(id);

    return ((pCurEvent) ? (((pCurEvent->head.linker.pList) ? (TRUE) : (FALSE))) : FALSE);
}

/**
 * @brief The event timeout callback fucntion.
 *
 * @param id The event unique id.
 */
static void _event_callback_fromTimeOut(os_id_t id)
{
    _impl_kernal_thread_entry_trigger(_impl_kernal_member_unified_id_timerToThread(id), id, PC_SC_TIMEOUT, _event_schedule);
}

/**
 * @brief Convert the internal os id to kernal member number.
 *
 * @param id The provided unique id.
 *
 * @return The value of member number.
 */
u32_t _impl_event_os_id_to_number(os_id_t id)
{
    return (u32_t)(_event_id_isInvalid(id) ?
                   (0u) : (id - _impl_kernal_member_id_toUnifiedIdStart(KERNAL_MEMBER_EVENT)) / sizeof(event_context_t));
}

/**
 * @brief Initialize a new event.
 *
 * @param pName The event name.
 * @param edge Callback function trigger edge condition.
 *
 * @return The event unique id.
 */
os_id_t _impl_event_init(u32_t edge, pEvent_callbackFunc_t pCallFun, const char_t *pName)
{
    arguments_t arguments[] =
    {
        [0] = {.u32_val = (u32_t)edge},
        [1] = {.ptr_val = (const void*)pCallFun},
        [2] = {.pch_val = (const char_t *)pName},
    };

    return _impl_kernal_privilege_invoke((const void*)_event_init_privilege_routine, arguments);
}

/**
 * @brief Set a event value.
 *
 * @param id The event unique id.
 * @param id The event value.
 *
 * @return The result of the operation.
 */
u32p_t _impl_event_set(os_id_t id, u32_t event)
{
    if (_event_id_isInvalid(id))
    {
        return _PC_CMPT_FAILED;
    }

    if (!_event_object_isInit(id))
    {
        return _PC_CMPT_FAILED;
    }

    arguments_t arguments[] =
    {
        [0] = {.u32_val = (u32_t)id},
        [1] = {.u32_val = (u32_t)event},
    };

    return _impl_kernal_privilege_invoke((const void*)_event_set_privilege_routine, arguments);
}

/**
 * @brief Wait a target event.
 *
 * @param id The event unique id.
 * @param pEvent The pointer of event value.
 * @param trigger If the trigger is not zero, All changed bits seen can wake up the thread to handle event.
 * @param listen Current thread listen which bits in the event.
 * @param timeout_ms The event wait timeout setting.
 *
 * @return The result of the operation.
 */
u32p_t _impl_event_wait(os_id_t id, u32_t *pEvent, u32_t trigger, u32_t listen, u32_t timeout_ms)
{
    if (_event_id_isInvalid(id))
    {
        return _PC_CMPT_FAILED;
    }

    if (!_event_object_isInit(id))
    {
        return _PC_CMPT_FAILED;
    }

    if (!timeout_ms)
    {
        return _PC_CMPT_FAILED;
    }

    if (!_impl_kernal_isInThreadMode())
    {
        return _PC_CMPT_FAILED;
    }

    arguments_t arguments[] =
    {
        [0] = {.u32_val = (u32_t)id},
        [1] = {.u32_val = (u32_t)pEvent},
        [2] = {.u32_val = (u32_t)trigger},
        [3] = {.u32_val = (u32_t)listen},
        [4] = {.u32_val = (u32_t)timeout_ms},
    };

    u32p_t postcode = _impl_kernal_privilege_invoke((const void*)_event_wait_privilege_routine, arguments);

    ENTER_CRITICAL_SECTION();

    if (PC_IOK(postcode))
    {
        thread_context_t *pCurThread = (thread_context_t *)_impl_kernal_thread_runContextGet();
        postcode = (u32p_t)_impl_kernal_schedule_entry_result_read_clean((action_schedule_t*)&pCurThread->schedule);
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
static u32_t _event_init_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    u32_t edge = (u32_t)(pArgs[0].u32_val);
    pEvent_callbackFunc_t pCallFun = (pEvent_callbackFunc_t)(pArgs[1].ptr_val);
    const char_t *pName = (const char_t *)(pArgs[2].pch_val);

    event_context_t *pCurEvent = (event_context_t *)_impl_kernal_member_id_toContainerStartAddress(KERNAL_MEMBER_EVENT);
    os_id_t id = _impl_kernal_member_id_toUnifiedIdStart(KERNAL_MEMBER_EVENT);

    do {
        if (!_event_object_isInit(id))
        {
            _memset((char_t*)pCurEvent, 0x0u, sizeof(event_context_t));

            pCurEvent->head.id = id;
            pCurEvent->head.pName = pName;

            pCurEvent->set = 0u;
            pCurEvent->edge = edge;
            pCurEvent->pCallbackFunc = pCallFun;

            _event_list_transfer_toActive((linker_head_t*)&pCurEvent->head);

            break;
        }

        pCurEvent++;
        id = _impl_kernal_member_containerAddress_toUnifiedid((u32_t)pCurEvent);
    } while ((u32_t)pCurEvent < (u32_t)_impl_kernal_member_id_toContainerEndAddress(KERNAL_MEMBER_EVENT));

    id = ((!_event_id_isInvalid(id)) ? (id) : (OS_INVALID_ID));

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
static u32_t _event_set_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    os_id_t id = (os_id_t)pArgs[0].u32_val;
    u32_t event = (u32_t)pArgs[1].u32_val;
    u32p_t postcode = PC_SC_SUCCESS;
    event_context_t *pCurEvent = (event_context_t *)_event_object_contextGet(id);
    pCurEvent->set = event;

    list_iterator_t it = {0u};
    list_iterator_init(&it, _event_list_blockingHeadGet(id));
    thread_context_t *pCurThread = (thread_context_t *)list_iterator_next(&it);
    while (pCurThread)
    {
        *pCurThread->event.pStore |= ((pCurThread->event.listen) & (pCurEvent->set));

        if ((pCurThread->event.trigger) && (pCurThread->event.trigger == (*pCurThread->event.pStore & pCurThread->event.trigger)))
        {
            /* Group event */
            postcode = _impl_kernal_thread_entry_trigger(pCurThread->head.id, id, PC_SC_SUCCESS, _event_schedule);
        }
        else if ((!pCurThread->event.trigger) && (*pCurThread->event.pStore))
        {
            /* General event */
            postcode = _impl_kernal_thread_entry_trigger(pCurThread->head.id, id, PC_SC_SUCCESS, _event_schedule);
        }

        if (PC_IER(postcode))
        {
            break;
        }
        pCurThread = (thread_context_t *)list_iterator_next(&it);
    }

    pCurEvent->set = 0u;

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
static u32_t _event_wait_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    os_id_t id = (os_id_t)pArgs[0].u32_val;
    u32_t* pEvent = (u32_t*)pArgs[1].u32_val;
    u32_t trigger = (u32_t)pArgs[2].u32_val;
    u32_t listen = (u32_t)pArgs[3].u32_val;
    u32_t timeout_ms = (u32_t)pArgs[4].u32_val;

    thread_context_t *pCurThread = (thread_context_t *)_impl_kernal_thread_runContextGet();

    pCurThread->event.listen = listen;
    pCurThread->event.trigger = trigger;
    pCurThread->event.pStore = pEvent;
    *pCurThread->event.pStore = 0u;

    u32p_t postcode = _impl_kernal_thread_exit_trigger(pCurThread->head.id, id, _event_list_blockingHeadGet(id), timeout_ms, _event_callback_fromTimeOut);

    EXIT_CRITICAL_SECTION();
    return postcode;
}

/**
 * @brief The event schedule routine execute the the pendsv context.
 *
 * @param id The unique id of the entry thread.
 */
static void _event_schedule(os_id_t id)
{
    thread_context_t *pEntryThread = (thread_context_t*)(_impl_kernal_member_unified_id_toContainerAddress(id));
    thread_entry_t *pEntry = NULL;
    b_t isAvail = FALSE;

    if (_impl_kernal_member_unified_id_toId(pEntryThread->schedule.hold) != KERNAL_MEMBER_EVENT)
    {
        pEntryThread->schedule.entry.result = _PC_CMPT_FAILED;
        return ;
    }

    if ((pEntryThread->schedule.entry.result != PC_SC_SUCCESS) && (pEntryThread->schedule.entry.result != PC_SC_TIMEOUT))
    {
        return ;
    }

    pEntry = &pEntryThread->schedule.entry;
    if (!_impl_timer_status_isBusy(_impl_kernal_member_unified_id_threadToTimer(pEntryThread->head.id)))
    {
        if (_impl_kernal_member_unified_id_toId(pEntry->release) == KERNAL_MEMBER_TIMER_INTERNAL)
        {
            pEntry->result = PC_SC_TIMEOUT;
        }
        else
        {
            isAvail = true;
        }
    }
    else if (_impl_kernal_member_unified_id_toId(pEntry->release) == KERNAL_MEMBER_EVENT)
    {
        _impl_timer_stop(_impl_kernal_member_unified_id_threadToTimer(pEntryThread->head.id));
        isAvail = true;
    }
    else
    {
        pEntry->result = _PC_CMPT_FAILED;
    }

    pEntryThread->event.listen = 0u;
    pEntryThread->event.trigger = 0u;

    if (isAvail)
    {
        pEntry->result  = PC_SC_SUCCESS;
    }
}

#ifdef __cplusplus
}
#endif
