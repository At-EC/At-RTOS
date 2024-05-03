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
#define _PC_CMPT_FAILED PC_FAILED(PC_CMPT_EVENT_6)

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
 * @brief Get the active event list head.
 *
 * @return The value of the active list head.
 */
static list_t *_event_list_activeHeadGet(void)
{
    return (list_t *)kernel_member_list_get(KERNEL_MEMBER_EVENT, KERNEL_MEMBER_LIST_EVENT_ACTIVE);
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
    thread_entry_t *pEntry = NULL;
    b_t isAvail = FALSE;

    if (kernel_member_unified_id_toId(pEntryThread->schedule.hold) != KERNEL_MEMBER_EVENT) {
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
    } else if (kernel_member_unified_id_toId(pEntry->release) == KERNEL_MEMBER_EVENT) {
        timer_stop_for_thread(kernel_member_unified_id_threadToTimer(pEntryThread->head.id));
        isAvail = true;
    } else {
        pEntry->result = _PC_CMPT_FAILED;
    }

    /* Auto clear user configuration */
    pEntryThread->event.listen = 0u;
    pEntryThread->event.trigger = 0u;
    pEntryThread->event.group = 0u;
    pEntryThread->event.pEvtVal = NULL;

    if (isAvail) {
        pEntry->result = PC_SC_SUCCESS;
    }
}

/**
 * @brief The event timeout callback fucntion.
 *
 * @param id The event unique id.
 */
static void _event_callback_fromTimeOut(os_id_t id)
{
    kernel_thread_entry_trigger(kernel_member_unified_id_timerToThread(id), id, PC_SC_TIMEOUT, _event_schedule);
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

    u32_t edgeMask = (u32_t)(pArgs[0].u32_val);
    u32_t clrDisMask = (u32_t)(pArgs[1].u32_val);
    const char_t *pName = (const char_t *)(pArgs[2].pch_val);
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

        _memset((char_t *)pCurEvent, 0x0u, sizeof(event_context_t));
        pCurEvent->head.id = id;
        pCurEvent->head.pName = pName;

        _memset((char_t *)pCurEvent->value, 0x0u, OS_EVENT_POOL_DEPTH);
        pCurEvent->edgeMask = edgeMask;
        pCurEvent->clearMask = ~clrDisMask;
        pCurEvent->call.pCallbackFunc = NULL;

        _event_list_transfer_toInit((linker_head_t *)&pCurEvent->head);

        EXIT_CRITICAL_SECTION();
        return id;

    } while ((u32_t)++pCurEvent < endAddr);

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
static u32_t _event_set_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    os_id_t id = (os_id_t)pArgs[0].u32_val;
    u32_t set = (u32_t)pArgs[1].u32_val;
    u32_t clear = (u32_t)pArgs[2].u32_val;
    u32_t toggle = (u32_t)pArgs[3].u32_val;

    u32p_t postcode = PC_SC_SUCCESS;
    event_context_t *pCurEvent = _event_object_contextGet(id);
    u32_t idx = pCurEvent->index;
    u32_t val = pCurEvent->value[idx];

    /* clear bits */
    val &= ~clear;
    /* set bits */
    val |= set;
    /* toggle bits */
    val ^= toggle;

    /* Edge trigger */
    u32_t trigger = (val ^ pCurEvent->value[idx]) & pCurEvent->edgeMask;

    /* Level trigger */
    trigger |= val & (~pCurEvent->edgeMask);

    /* Order new index and value*/
    idx = (idx + 1) % OS_EVENT_POOL_DEPTH;
    pCurEvent->value[idx] = val;
    pCurEvent->index = idx;

    list_iterator_t it = {0u};
    list_iterator_init(&it, _event_list_blockingHeadGet(id));
    thread_context_t *pCurThread = (thread_context_t *)list_iterator_next(&it);
    while (pCurThread) {
        /* set = desired-trigger bits & listening bits */
        u32_t set = ~(trigger ^ pCurThread->event.trigger) & pCurThread->event.listen;

        if (set) {
            pCurThread->event.pEvtVal->value |= set;

            if (pCurThread->event.group) {
                if (pCurThread->event.group == (pCurThread->event.pEvtVal->value & pCurThread->event.group)) {
                    /* group */
                    pCurThread->event.pEvtVal->depth.location = pCurEvent->index;
                    postcode = kernel_thread_entry_trigger(pCurThread->head.id, id, PC_SC_SUCCESS, _event_schedule);
                }
            } else {
                if (pCurThread->event.pEvtVal->value) {
                    /* single */
                    pCurThread->event.pEvtVal->depth.location = pCurEvent->index;
                    postcode = kernel_thread_entry_trigger(pCurThread->head.id, id, PC_SC_SUCCESS, _event_schedule);
                }
            }

            if (PC_IER(postcode)) {
                break;
            }
            pCurThread = (thread_context_t *)list_iterator_next(&it);
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
static u32_t _event_wait_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    os_id_t id = (os_id_t)pArgs[0].u32_val;
    os_evt_val_t *pEvtData = (os_evt_val_t *)pArgs[1].pv_val;
    u32_t trigger = (u32_t)pArgs[2].u32_val;
    u32_t listen = (u32_t)pArgs[3].u32_val;
    u32_t group = (u32_t)pArgs[4].u32_val;
    u32_t timeout_ms = (u32_t)pArgs[5].u32_val;
    u32p_t postcode = PC_SC_SUCCESS;

    thread_context_t *pCurThread = kernel_thread_runContextGet();
    pCurThread->event.listen = listen;
    pCurThread->event.trigger = trigger;
    pCurThread->event.group = group;
    pCurThread->event.pEvtVal = pEvtData;

    event_context_t *pCurEvent = _event_object_contextGet(id);
    u32_t record = 0u;

    /* reset event value */
    pEvtData->value = 0u;

    if (!pEvtData->depth.active) {
        pEvtData->depth.active = TRUE;
        pEvtData->depth.location = (pCurEvent->index + 1u) % OS_EVENT_POOL_DEPTH;
    }

    if (!pEvtData->depth.enable) {
        record = pCurEvent->value[pCurEvent->index];
    } else {
        u8_t i = 0u;
        u8_t idx = pEvtData->depth.location;
        while ((idx != pCurEvent->index) && (i < OS_EVENT_POOL_DEPTH)) {
            idx = (pEvtData->depth.location + i) % OS_EVENT_POOL_DEPTH;
            record |= pCurEvent->value[idx];
            i++;
        };
    }
    /* It'll be updated again when event trigger successful */
    pEvtData->depth.location = pCurEvent->index;

    /* set = desired-trigger bits & listening bits */
    u32_t set = ~(record ^ pCurThread->event.trigger) & pCurThread->event.listen;

    if (set) {
        pCurThread->event.pEvtVal->value |= set;

        if (pCurThread->event.group) {
            if (pCurThread->event.group == (pCurThread->event.pEvtVal->value & pCurThread->event.group)) {
                /* group */
                EXIT_CRITICAL_SECTION();
                return postcode;
            }
        } else {
            if (pCurThread->event.pEvtVal->value) {
                /* single */
                EXIT_CRITICAL_SECTION();
                return postcode;
            }
        }
    }

    postcode =
        kernel_thread_exit_trigger(pCurThread->head.id, id, _event_list_blockingHeadGet(id), timeout_ms, _event_callback_fromTimeOut);

    if (PC_IOK(postcode)) {
        postcode = PC_SC_UNAVAILABLE;
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
 * @param edgeMask specific the event desired condition of edge or level.
 * @param clearMask automatically clear the set events.
 * @param pName The event name.
 *
 * @return The event unique id.
 */
os_id_t _impl_event_init(u32_t edgeMask, u32_t clrDisMask, const char_t *pName)
{
    arguments_t arguments[] = {
        [0] = {.u32_val = (u32_t)edgeMask},
        [1] = {.u32_val = (u32_t)clrDisMask},
        [2] = {.pch_val = (const char_t *)pName},
    };

    return kernel_privilege_invoke((const void *)_event_init_privilege_routine, arguments);
}

u32p_t _impl_event_wait_callfunc_register(pEvent_callbackFunc_t pCallFun)
{
    return 0u;
}

/**
 * @brief Set/clear/toggle a event bits.
 *
 * @param id The event unique id.
 * @param set The event value bits set.
 * @param clear The event value bits clear.
 * @param toggle The event value bits toggle.
 *
 * @return The result of the operation.
 */
u32p_t _impl_event_set(os_id_t id, u32_t set, u32_t clear, u32_t toggle)
{
    if (_event_id_isInvalid(id)) {
        return _PC_CMPT_FAILED;
    }

    if (!_event_object_isInit(id)) {
        return _PC_CMPT_FAILED;
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
 * @brief Wait a target event.
 *
 * @param id The event unique id.
 * @param pEvtData The pointer of event value.
 * @param trigger If the trigger is not zero, All changed bits seen can wake up the thread to handle event.
 * @param listen_mask Current thread listen which bits in the event.
 * @param group_mask To define a group event.
 * @param timeout_ms The event wait timeout setting.
 *
 * @return The result of the operation.
 */
u32p_t _impl_event_wait(os_id_t id, os_evt_val_t *pEvtData, u32_t trigger, u32_t listen_mask, u32_t group_mask, u32_t timeout_ms)
{
    if (_event_id_isInvalid(id)) {
        return _PC_CMPT_FAILED;
    }

    if (!_event_object_isInit(id)) {
        return _PC_CMPT_FAILED;
    }

    if (!pEvtData) {
        return _PC_CMPT_FAILED;
    }

    if (!timeout_ms) {
        return _PC_CMPT_FAILED;
    }

    if (!kernel_isInThreadMode()) {
        return _PC_CMPT_FAILED;
    }

    arguments_t arguments[] = {
        [0] = {.u32_val = (u32_t)id},          [1] = {.pv_val = (void *)pEvtData},   [2] = {.u32_val = (u32_t)trigger},
        [3] = {.u32_val = (u32_t)listen_mask}, [4] = {.u32_val = (u32_t)group_mask}, [5] = {.u32_val = (u32_t)timeout_ms},
    };

    u32p_t postcode = kernel_privilege_invoke((const void *)_event_wait_privilege_routine, arguments);

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
    os_id_t id = OS_INVALID_ID;

    ENTER_CRITICAL_SECTION();

    offset = sizeof(event_context_t) * instance;
    pCurEvent = (event_context_t *)(kernel_member_id_toContainerStartAddress(KERNEL_MEMBER_EVENT) + offset);
    id = kernel_member_containerAddress_toUnifiedid((u32_t)pCurEvent);
    _memset((u8_t *)pMsgs, 0x0u, sizeof(kernel_snapshot_t));

    if (_event_id_isInvalid(id)) {
        EXIT_CRITICAL_SECTION();
        return FALSE;
    }

    if (pCurEvent->head.linker.pList == _event_list_activeHeadGet()) {
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

    pMsgs->event.set = pCurEvent->value[pCurEvent->index];
    pMsgs->event.edge = pCurEvent->edgeMask;
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
