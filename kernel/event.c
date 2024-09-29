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
#define _PCER PC_IER(PC_OS_CMPT_EVENT_7)

/**
 * @brief Get the event context based on provided unique id.
 *
 * @param id The event unique id.
 *
 * @return The pointer of the current unique id event context.
 */
static event_context_t *_event_object_contextGet(os_id_t id)
{
    return (event_context_t *)(kernel_member_unified_id_toContainerAddress(id));
}

/**
 * @brief Get the init event list head.
 *
 * @return The value of the init list head.
 */
static list_t *_event_list_initHeadGet(void)
{
    return (list_t *)kernel_member_list_get(KERNEL_MEMBER_EVENT, KERNEL_MEMBER_LIST_EVENT_INIT);
}

/**
 * @brief Pick up a highest priority thread that blocking by the event pending list.
 *
 * @param The event unique id.
 *
 * @return The highest blocking thread head.
 */
static list_t *_event_list_blockingHeadGet(os_id_t id)
{
    event_context_t *pCurEvent = _event_object_contextGet(id);

    return (list_t *)((pCurEvent) ? (&pCurEvent->blockingThreadHead) : (NULL));
}

/**
 * @brief Push one event context into init list.
 *
 * @param pCurHead The pointer of the timer linker head.
 */
static void _event_list_transfer_toInit(linker_head_t *pCurHead)
{
    ENTER_CRITICAL_SECTION();

    list_t *pToInitList = (list_t *)_event_list_initHeadGet();
    linker_list_transaction_common(&pCurHead->linker, pToInitList, LIST_TAIL);

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
    return kernel_member_unified_id_isInvalid(KERNEL_MEMBER_EVENT, id);
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
    event_context_t *pCurEvent = _event_object_contextGet(id);

    return ((pCurEvent) ? (((pCurEvent->head.linker.pList) ? (TRUE) : (FALSE))) : FALSE);
}

/**
 * @brief The event schedule routine execute the the pendsv context.
 *
 * @param id The unique id of the entry thread.
 */
static void _event_schedule(os_id_t id)
{
    thread_context_t *pEntryThread = (thread_context_t *)(kernel_member_unified_id_toContainerAddress(id));

    if (kernel_member_unified_id_toId(pEntryThread->schedule.hold) != KERNEL_MEMBER_EVENT) {
        pEntryThread->schedule.entry.result = _PCER;
        return;
    }
    timer_stop_for_thread(kernel_member_unified_id_threadToTimer(pEntryThread->head.id));

    u32_t changed, any, edge, level, trigger = 0u;
    event_context_t *pCurEvent = _event_object_contextGet(pEntryThread->schedule.hold);
    os_evt_val_t *pEvtVal = pEntryThread->event.pEvtVal;
    changed = pEvtVal->value ^ pCurEvent->value;
    if (changed) {
        // Any position
        any = pCurEvent->anyMask;

        // Changings trigger.
        trigger = any & changed;

        // Edge position
        edge = pCurEvent->modeMask;
        edge &= ~pCurEvent->anyMask;

        // Edge rise trigger.
        trigger |= edge & pCurEvent->value & pCurEvent->dirMask & changed;

        // Edge fall trigger.
        trigger |= edge & ~pCurEvent->value & ~pCurEvent->dirMask & changed;

        // Level position
        level = ~pCurEvent->modeMask;
        level &= ~pCurEvent->anyMask;

        // Level high trigger.
        trigger |= level & pCurEvent->value & pCurEvent->dirMask & changed;

        // Level low trigger.
        trigger |= level & ~pCurEvent->value & ~pCurEvent->dirMask & changed;
    }
    // Triggered bits
    trigger |= pCurEvent->triggered;

    pEvtVal->value = pCurEvent->value;
    u32_t report = trigger & pEntryThread->event.listen;
    if (report) {
        pEvtVal->trigger = trigger;
        pCurEvent->triggered &= ~report;
    }

    /* Auto clear user configuration */
    pEntryThread->event.listen = 0u;
    pEntryThread->event.pEvtVal = NULL;

    pEntryThread->schedule.entry.result = 0;
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

    u32_t anyMask = (u32_t)(pArgs[0].u32_val);
    u32_t modeMask = (u32_t)(pArgs[1].u32_val);
    u32_t dirMask = (u32_t)(pArgs[2].u32_val);
    u32_t init = (u32_t)(pArgs[3].u32_val);
    const char_t *pName = (const char_t *)(pArgs[4].pch_val);
    u32_t endAddr = 0u;
    event_context_t *pCurEvent = NULL;

    pCurEvent = (event_context_t *)kernel_member_id_toContainerStartAddress(KERNEL_MEMBER_EVENT);
    endAddr = (u32_t)kernel_member_id_toContainerEndAddress(KERNEL_MEMBER_EVENT);
    do {
        os_id_t id = kernel_member_containerAddress_toUnifiedid((u32_t)pCurEvent);
        if (_event_id_isInvalid(id)) {
            break;
        }

        if (_event_object_isInit(id)) {
            continue;
        }

        os_memset((char_t *)pCurEvent, 0x0u, sizeof(event_context_t));
        pCurEvent->head.id = id;
        pCurEvent->head.pName = pName;

        pCurEvent->value = init;
        pCurEvent->triggered = 0u;
        pCurEvent->anyMask = anyMask;
        pCurEvent->modeMask = modeMask;
        pCurEvent->dirMask = dirMask;
        pCurEvent->call.pCallbackFunc = NULL;

        _event_list_transfer_toInit((linker_head_t *)&pCurEvent->head);

        EXIT_CRITICAL_SECTION();
        return id;

    } while ((u32_t)++pCurEvent < endAddr);

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
static i32p_t _event_value_get_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    os_id_t id = (os_id_t)pArgs[0].u32_val;
    u32_t *pValue = (u32_t *)pArgs[1].pv_val;
    event_context_t *pCurEvent = _event_object_contextGet(id);
    *pValue = pCurEvent->value;

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
static i32p_t _event_set_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    os_id_t id = (os_id_t)pArgs[0].u32_val;
    u32_t set = (u32_t)pArgs[1].u32_val;
    u32_t clear = (u32_t)pArgs[2].u32_val;
    u32_t toggle = (u32_t)pArgs[3].u32_val;

    event_context_t *pCurEvent = _event_object_contextGet(id);
    u32_t val = pCurEvent->value;
    u32_t changed, any, edge, level, trigger = 0u;
    i32p_t postcode = 0;

    /// Clear bits
    val &= ~clear;
    // Set bits
    val |= set;
    // Toggle bits
    val ^= toggle;

    changed = val ^ pCurEvent->value;

    // Any position
    any = pCurEvent->anyMask;

    // Changings trigger.
    trigger = any & changed;

    // Edge position
    edge = pCurEvent->modeMask;
    edge &= ~pCurEvent->anyMask;

    // Edge rise trigger.
    trigger |= edge & val & pCurEvent->dirMask & changed;

    // Edge fall trigger.
    trigger |= edge & ~val & ~pCurEvent->dirMask & changed;

    // Level position
    level = ~pCurEvent->modeMask;
    level &= ~pCurEvent->anyMask;

    // Level high trigger.
    trigger |= level & val & pCurEvent->dirMask & changed;

    // Level low trigger.
    trigger |= level & ~val & ~pCurEvent->dirMask & changed;

    // Triggered bits
    trigger |= pCurEvent->triggered;

    u32_t report, reported = 0u;
    list_iterator_t it = {0u};
    list_iterator_init(&it, _event_list_blockingHeadGet(id));
    thread_context_t *pCurThread = (thread_context_t *)list_iterator_next(&it);
    while (pCurThread) {
        report = trigger & pCurThread->event.listen;

        if (report) {
            reported |= report;
            pCurThread->event.pEvtVal->trigger = trigger;
            pCurThread->event.pEvtVal->value = val;

            postcode = kernel_thread_entry_trigger(pCurThread, 0, _event_schedule);
            PC_IF(postcode, PC_ERROR)
            {
                break;
            }
        }
        pCurThread = (thread_context_t *)list_iterator_next(&it);
    }
    pCurEvent->triggered = (~reported) & trigger;
    pCurEvent->value = val;

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
static i32p_t _event_wait_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    os_id_t id = (os_id_t)pArgs[0].u32_val;
    os_evt_val_t *pEvtData = (os_evt_val_t *)pArgs[1].pv_val;
    u32_t listen = (u32_t)pArgs[2].u32_val;
    u32_t timeout_ms = (u32_t)pArgs[3].u32_val;
    i32p_t postcode = 0;

    thread_context_t *pCurThread = kernel_thread_runContextGet();
    event_context_t *pCurEvent = _event_object_contextGet(id);
    pCurThread->event.listen = listen;
    pCurThread->event.pEvtVal = pEvtData;
    u32_t val = pEvtData->value;
    u32_t changed, any, edge, level, trigger = 0u;

    changed = val ^ pCurEvent->value;
    if (changed) {
        // Any position
        any = pCurEvent->anyMask;

        // Changings trigger.
        trigger = any & changed;

        // Edge position
        edge = pCurEvent->modeMask;
        edge &= ~pCurEvent->anyMask;

        // Edge rise trigger.
        trigger |= edge & pCurEvent->value & pCurEvent->dirMask & changed;

        // Edge fall trigger.
        trigger |= edge & ~pCurEvent->value & ~pCurEvent->dirMask & changed;

        // Level position
        level = ~pCurEvent->modeMask;
        level &= ~pCurEvent->anyMask;

        // Level high trigger.
        trigger |= level & pCurEvent->value & pCurEvent->dirMask & changed;

        // Level low trigger.
        trigger |= level & ~pCurEvent->value & ~pCurEvent->dirMask & changed;
    }

    // Triggered bits
    trigger |= pCurEvent->triggered;

    pEvtData->value = val;
    u32_t report = trigger & pCurThread->event.listen;
    if (report) {
        pEvtData->trigger = trigger;
        pCurEvent->triggered &= ~report;

        EXIT_CRITICAL_SECTION();
        return postcode;
    }

    postcode = kernel_thread_exit_trigger(pCurThread, id, _event_list_blockingHeadGet(id), timeout_ms);
    PC_IF(postcode, PC_PASS)
    {
        postcode = PC_OS_WAIT_UNAVAILABLE;
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
u32_t _impl_event_os_id_to_number(os_id_t id)
{
    if (_event_id_isInvalid(id)) {
        return 0u;
    }

    return (u32_t)((id - kernel_member_id_toUnifiedIdStart(KERNEL_MEMBER_EVENT)) / sizeof(event_context_t));
}

/**
 * @brief Initialize a new event.
 *
 * @param anyMask: Changed bits always trigger = 1. otherwise, see dirMask below = 0.
 * @param modeMask: Level trigger = 0, Edge trigger = 1.
 * @param dirMask: Fall or Low trigger = 0, Rise or high trigger = 1.
 * @param init: The init signal value.
 * @param pName: The event name.
 *
 * @return The event unique id.
 */
os_id_t _impl_event_init(u32_t anyMask, u32_t modeMask, u32_t dirMask, u32_t init, const char_t *pName)
{
    arguments_t arguments[] = {
        [0] = {.u32_val = (u32_t)anyMask}, [1] = {.u32_val = (u32_t)modeMask},       [2] = {.u32_val = (u32_t)dirMask},
        [3] = {.u32_val = (u32_t)init},    [4] = {.pch_val = (const char_t *)pName},
    };

    return kernel_privilege_invoke((const void *)_event_init_privilege_routine, arguments);
}

i32p_t _impl_event_wait_callfunc_register(pEvent_callbackFunc_t pCallFun)
{
    return 0u;
}

/**
 * @brief Read or write the event signal value.
 *
 * @param id: Event unique id.
 * @param pValue: The pointer of the private event value.
 *
 * @return The result of the operation.
 */
i32p_t _impl_event_value_get(os_id_t id, u32_t *pValue)
{
    if (_event_id_isInvalid(id)) {
        return _PCER;
    }

    if (!_event_object_isInit(id)) {
        return _PCER;
    }

    arguments_t arguments[] = {
        [0] = {.u32_val = (u32_t)id},
        [1] = {.pv_val = (void *)pValue},
    };

    return kernel_privilege_invoke((const void *)_event_value_get_privilege_routine, arguments);
}

/**
 * @brief Set/clear/toggle event signal bits.
 *
 * @param id: Event unique id.
 * @param set: Event value bits set.
 * @param clear: Event value bits clear.
 * @param toggle: Event value bits toggle.
 *
 * @return The result of the operation.
 */
i32p_t _impl_event_set(os_id_t id, u32_t set, u32_t clear, u32_t toggle)
{
    if (_event_id_isInvalid(id)) {
        return _PCER;
    }

    if (!_event_object_isInit(id)) {
        return _PCER;
    }

    arguments_t arguments[] = {
        [0] = {.u32_val = (u32_t)id},
        [1] = {.u32_val = (u32_t)set},
        [2] = {.u32_val = (u32_t)clear},
        [3] = {.u32_val = (u32_t)toggle},
    };

    return kernel_privilege_invoke((const void *)_event_set_privilege_routine, arguments);
}

/**
 * @brief Wait a trigger event.
 *
 * @param id The event unique id.
 * @param pEvtData The pointer of event value.
 * @param listen_mask Current thread listen which bits in the event.
 * @param timeout_ms The event wait timeout setting.
 *
 * @return The result of the operation.
 */
i32p_t _impl_event_wait(os_id_t id, os_evt_val_t *pEvtData, u32_t listen_mask, u32_t timeout_ms)
{
    if (_event_id_isInvalid(id)) {
        return _PCER;
    }

    if (!_event_object_isInit(id)) {
        return _PCER;
    }

    if (!pEvtData) {
        return _PCER;
    }

    if (!timeout_ms) {
        return _PCER;
    }

    if (!kernel_isInThreadMode()) {
        return _PCER;
    }

    arguments_t arguments[] = {
        [0] = {.u32_val = (u32_t)id},
        [1] = {.pv_val = (void *)pEvtData},
        [2] = {.u32_val = (u32_t)listen_mask},
        [3] = {.u32_val = (u32_t)timeout_ms},
    };

    i32p_t postcode = kernel_privilege_invoke((const void *)_event_wait_privilege_routine, arguments);

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
 * @brief Get event snapshot informations.
 *
 * @param instance The event instance number.
 * @param pMsgs The kernel snapshot information pointer.
 *
 * @return TRUE: Operation pass, FALSE: Operation failed.
 */
b_t event_snapshot(u32_t instance, kernel_snapshot_t *pMsgs)
{
#if defined KTRACE
    event_context_t *pCurEvent = NULL;
    u32_t offset = 0u;
    os_id_t id = OS_INVALID_ID_VAL;

    ENTER_CRITICAL_SECTION();

    offset = sizeof(event_context_t) * instance;
    pCurEvent = (event_context_t *)(kernel_member_id_toContainerStartAddress(KERNEL_MEMBER_EVENT) + offset);
    id = kernel_member_containerAddress_toUnifiedid((u32_t)pCurEvent);
    os_memset((u8_t *)pMsgs, 0x0u, sizeof(kernel_snapshot_t));

    if (_event_id_isInvalid(id)) {
        EXIT_CRITICAL_SECTION();
        return FALSE;
    }

    if (pCurEvent->head.linker.pList == _event_list_initHeadGet()) {
        pMsgs->pState = "init";
    } else if (pCurEvent->head.linker.pList) {
        pMsgs->pState = "*";
    } else {
        pMsgs->pState = "unused";

        EXIT_CRITICAL_SECTION();
        return FALSE;
    }

    pMsgs->id = pCurEvent->head.id;
    pMsgs->pName = pCurEvent->head.pName;

    pMsgs->event.set = pCurEvent->value;
    pMsgs->event.edge = pCurEvent->modeMask;
    pMsgs->event.wait_list = pCurEvent->blockingThreadHead;

    EXIT_CRITICAL_SECTION();
    return TRUE;
#else
    return FALSE;
#endif
}

#ifdef __cplusplus
}
#endif
