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
#define _PCER PC_IER(PC_OS_CMPT_THREAD_3)

/**
 * @brief Get the thread context based on provided unique id.
 *
 * @param id Thread unique id.
 *
 * @return The pointer of the current unique id thread context.
 */
static thread_context_t *_thread_object_contextGet(os_id_t id)
{
    return (thread_context_t *)kernel_member_unified_id_toContainerAddress(id);
}

/**
 * @brief Get the current runtime thread id.
 *
 * @return The id of current running thread.
 */
static os_id_t _thread_id_runtime_get(void)
{
    return (os_id_t)kernel_thread_runIdGet();
}

/**
 * @brief Get the current runtime thread object context.
 *
 * @return The pointer of current running thread.
 */
static thread_context_t *_thread_object_runtime_get(void)
{
    return _thread_object_contextGet(kernel_thread_runIdGet());
}

/**
 * @brief Get the waiting thread list head.
 *
 * @return The value of the waiting list head.
 */
static list_t *_thread_list_waitingHeadGet(void)
{
    return (list_t *)kernel_member_list_get(KERNEL_MEMBER_THREAD, KERNEL_MEMBER_LIST_THREAD_WAIT);
}

/**
 * @brief Get the pending thread list head.
 *
 * @return The value of the pending list head.
 */
static list_t *_thread_list_pendingHeadGet(void)
{
    return (list_t *)kernel_list_pendingHeadGet();
}

/**
 * @brief Push one thread into uninitialized status.
 *
 * @param pCurHead The pointer of the thread linker head.
 */
static void _thread_list_transfer_toUninitialized(linker_head_t *pCurHead)
{
    ENTER_CRITICAL_SECTION();

    linker_list_transaction_common(&pCurHead->linker, NULL, LIST_TAIL);

    EXIT_CRITICAL_SECTION();
}

/**
 * @brief Push one thread context into waiting list.
 *
 * @param pCurHead The pointer of the thread linker head.
 */
static void _thread_list_transfer_toWait(linker_head_t *pCurHead)
{
    ENTER_CRITICAL_SECTION();

    list_t *pToWaitList = (list_t *)_thread_list_waitingHeadGet();
    linker_list_transaction_common(&pCurHead->linker, pToWaitList, LIST_TAIL);

    EXIT_CRITICAL_SECTION();
}

/**
 * @brief Push one thread context into entry list.
 *
 * @param pCurHead The pointer of the thread linker head.
 */
static void _thread_list_transfer_toEntry(linker_head_t *pCurHead)
{
    ENTER_CRITICAL_SECTION();

    kernel_thread_list_transfer_toEntry(pCurHead);

    EXIT_CRITICAL_SECTION();
}

/**
 * @brief Push one thread context into pending list.
 *
 * @param pCurHead The pointer of the thread linker head.
 */
static void _thread_list_transfer_toPend(linker_head_t *pCurHead)
{
    ENTER_CRITICAL_SECTION();

    kernel_thread_list_transfer_toPend(pCurHead);

    EXIT_CRITICAL_SECTION();
}

/**
 * @brief Pick up a highest priority thread from the pending list.
 *
 * @return The pointer of the pending thread head.
 */
static linker_head_t *_thread_linker_head_fromPending(void)
{
    ENTER_CRITICAL_SECTION();

    list_t *pListPending = (list_t *)_thread_list_pendingHeadGet();

    EXIT_CRITICAL_SECTION();
    return (linker_head_t *)(pListPending->pHead);
}

/**
 * @brief Pick up the next priority thread from the pending list.
 *
 * @return The pointer of the next pending thread head.
 */
static linker_head_t *_thread_linker_Head_next_fromPending(void)
{
    linker_head_t *pHead = _thread_linker_head_fromPending();

    return (linker_head_t *)((pHead) ? (pHead->linker.node.pNext) : (NULL));
}

/**
 * @brief Check if the thread unique id if is's invalid.
 *
 * @param id The provided unique id.
 *
 * @return The true indicates the id is invalid, otherwise it valid.
 */
static b_t _thread_id_isInvalid(u32_t id)
{
    return kernel_member_unified_id_isInvalid(KERNEL_MEMBER_THREAD, id);
}

/**
 * @brief Check if the timer object if is's initialized.
 *
 * @param id The provided unique id.
 *
 * @return The true is initialized, otherwise is uninitialized.
 */
static b_t _thread_object_isInit(os_id_t id)
{
    thread_context_t *pCurThread = _thread_object_contextGet(id);

    return ((pCurThread) ? (((pCurThread->head.linker.pList) ? (TRUE) : (FALSE))) : FALSE);
}

/**
 * @brief The thread timeout callback fucntion.
 *
 * @param id The thread unique id.
 */
static void _thread_callback_fromTimeOut(os_id_t id)
{
    kernel_thread_entry_trigger(kernel_member_unified_id_timerToThread(id), id, PC_OS_WAIT_TIMEOUT, NULL);
}

/**
 * @brief It's sub-routine running at privilege mode.
 *
 * @param pArgs The function argument packages.
 *
 * @return The result of privilege routine.
 */
static os_id_t _thread_init_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();

    pThread_entryFunc_t pEntryFun = (pThread_entryFunc_t)pArgs[0].ptr_val;
    u32_t *pAddress = (u32_t *)pArgs[1].u32_val;
    u32_t size = (u32_t)pArgs[2].u32_val;
    u8_t priority = (u8_t)pArgs[3].u8_val;
    const char_t *pName = (const char_t *)pArgs[4].pch_val;
    u32_t internal = 0u;
    u32_t endAddr = 0u;
    thread_context_t *pCurThread = NULL;

    internal = sizeof(thread_context_t) * KERNEL_APPLICATION_THREAD_INSTANCE;
    pCurThread = (thread_context_t *)(kernel_member_id_toContainerStartAddress(KERNEL_MEMBER_THREAD) + internal);
    endAddr = (u32_t)kernel_member_id_toContainerEndAddress(KERNEL_MEMBER_THREAD);
    do {
        os_id_t id = kernel_member_containerAddress_toUnifiedid((u32_t)pCurThread);
        if (_thread_id_isInvalid(id)) {
            break;
        }

        if (_thread_object_isInit(id)) {
            continue;
        }

        os_memset((char_t *)pCurThread, 0x0u, sizeof(thread_context_t));
        pCurThread->head.id = id;
        pCurThread->head.pName = pName;

        pCurThread->priority.level = priority;
        pCurThread->pEntryFunc = pEntryFun;
        pCurThread->pStackAddr = pAddress;
        pCurThread->stackSize = size;

        pCurThread->PSPStartAddr = (u32_t)kernel_stack_frame_init(pEntryFun, pCurThread->pStackAddr, pCurThread->stackSize);
        timer_init_for_thread(kernel_member_unified_id_threadToTimer(id));

        _thread_list_transfer_toPend((linker_head_t *)&pCurThread->head);

        EXIT_CRITICAL_SECTION();
        return id;
    } while ((u32_t)++pCurThread < endAddr);

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
static i32p_t _thread_resume_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();
    os_id_t id = (os_id_t)pArgs[0].u32_val;
    thread_context_t *pCurThread = NULL;
    i32p_t postcode = 0;

    if (_thread_id_runtime_get() == id) {
        EXIT_CRITICAL_SECTION();
        return postcode;
    }

    pCurThread = _thread_object_contextGet(id);
    _thread_list_transfer_toEntry((linker_head_t *)&pCurThread->head);
    postcode = kernel_thread_schedule_request();

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
static i32p_t _thread_suspend_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();
    os_id_t id = (os_id_t)pArgs[0].u32_val;
    thread_context_t *pCurThread = NULL;
    i32p_t postcode = _PCER;

    pCurThread = _thread_object_contextGet(id);
    if (!_thread_linker_Head_next_fromPending()) {
        EXIT_CRITICAL_SECTION();
        return postcode;
    }
    _thread_list_transfer_toWait((linker_head_t *)&pCurThread->head);
    postcode = kernel_thread_schedule_request();

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
static i32p_t _thread_yield_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();
    UNUSED_MSG(pArgs);
    thread_context_t *pCurThread = NULL;
    i32p_t postcode = 0;

    pCurThread = (thread_context_t *)_thread_object_runtime_get();
    if (!_thread_linker_Head_next_fromPending()) {
        EXIT_CRITICAL_SECTION();
        return postcode;
    }

    _thread_list_transfer_toWait((linker_head_t *)&pCurThread->head);
    postcode = kernel_thread_schedule_request();

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
static i32p_t _thread_delete_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();
    os_id_t id = (os_id_t)pArgs[0].u32_val;
    thread_context_t *pCurThread = NULL;
    i32p_t postcode = _PCER;

    pCurThread = _thread_object_contextGet(id);
    if (id == _thread_id_runtime_get()) {
        EXIT_CRITICAL_SECTION();
        return postcode;
    }

    if (!_thread_linker_Head_next_fromPending()) {
        EXIT_CRITICAL_SECTION();
        return postcode;
    }

    _thread_list_transfer_toUninitialized((linker_head_t *)&pCurThread->head);
    os_memset((char_t *)pCurThread->pStackAddr, STACT_UNUSED_DATA, pCurThread->stackSize);
    os_memset((char_t *)pCurThread, 0x0u, sizeof(thread_context_t));
    postcode = kernel_thread_schedule_request();

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
static i32p_t _thread_sleep_privilege_routine(arguments_t *pArgs)
{
    ENTER_CRITICAL_SECTION();
    u32_t timeout_ms = (u32_t)pArgs[0].u32_val;
    thread_context_t *pCurThread = NULL;
    i32p_t postcode = _PCER;

    pCurThread = kernel_thread_runContextGet();
    postcode = kernel_thread_exit_trigger(pCurThread->head.id, OS_INVALID_ID_VAL, _thread_list_waitingHeadGet(), timeout_ms,
                                          _thread_callback_fromTimeOut);

    EXIT_CRITICAL_SECTION();
    return postcode;
}

/**
 * @brief Convert the internal os id to kernel member number.
 *
 * @return The Member number.
 */
u32_t _impl_thread_os_id_to_number(os_id_t id)
{
    if (_thread_id_isInvalid(id)) {
        return 0u;
    }

    return (u32_t)((id - kernel_member_id_toUnifiedIdStart(KERNEL_MEMBER_THREAD)) / sizeof(thread_context_t));
}

/**
 * @brief Get the thread name based on provided unique id.
 *
 * @param id Thread unique id.
 *
 * @return The name of current thread.
 */
const char_t *_impl_thread_name_get(os_id_t id)
{
    if (_thread_id_isInvalid(id)) {
        return NULL;
    }

    if (!_thread_object_isInit(id)) {
        return NULL;
    }

    thread_context_t *pCurThread = _thread_object_contextGet(id);

    return (const char_t *)((pCurThread) ? (pCurThread->head.pName) : (NULL));
}

/**
 * @brief Initialize a new thread.
 *
 * @param pEntryFun The thread entry function pointer.
 * @param pAddress The thread stack address.
 * @param size The thread stack size.
 * @param priority The thread priority.
 * @param pName The thread name.
 *
 * @return The value of thread unique id.
 */
os_id_t _impl_thread_init(pThread_entryFunc_t pEntryFun, u32_t *pAddress, u32_t size, u16_t priority, const char_t *pName)
{
    if (!pEntryFun) {
        return OS_INVALID_ID_VAL;
    }

    if (!pAddress) {
        return OS_INVALID_ID_VAL;
    }

    if ((size < STACK_SIZE_MINIMUM) || (size > STACK_SIZE_MAXIMUM)) {
        return OS_INVALID_ID_VAL;
    }

    if (priority > 0xFFu) {
        return OS_INVALID_ID_VAL;
    }

    if ((priority == OS_PRIOTITY_LOWEST_LEVEL) || (priority == OS_PRIOTITY_HIGHEST_LEVEL)) {
        return OS_INVALID_ID_VAL;
    }

    arguments_t arguments[] = {
        [0] = {.ptr_val = (void *)pEntryFun}, [1] = {.u32_val = (u32_t)pAddress},     [2] = {.u32_val = (u32_t)size},
        [3] = {.u8_val = (u8_t)priority},     [4] = {.pch_val = (const void *)pName},
    };

    return kernel_privilege_invoke((const void *)_thread_init_privilege_routine, arguments);
}

/**
 * @brief Resume a thread to run.
 *
 * @param id The thread unique id.
 *
 * @return The result of thread resume operation.
 */
i32p_t _impl_thread_resume(os_id_t id)
{
    if (_thread_id_isInvalid(id)) {
        return _PCER;
    }

    if (!_thread_object_isInit(id)) {
        return _PCER;
    }

    arguments_t arguments[] = {
        [0] = {.u32_val = (u32_t)id},
    };

    return kernel_privilege_invoke((const void *)_thread_resume_privilege_routine, arguments);
}

/**
 * @brief Suspend a thread to permit another to run.
 *
 * @param id The thread unique id.
 *
 * @return The result of thread suspend operation.
 */
i32p_t _impl_thread_suspend(os_id_t id)
{
    if (_thread_id_isInvalid(id)) {
        return _PCER;
    }

    if (!_thread_object_isInit(id)) {
        return _PCER;
    }

    arguments_t arguments[] = {
        [0] = {.u32_val = (u32_t)id},
    };

    return kernel_privilege_invoke((const void *)_thread_suspend_privilege_routine, arguments);
}

/**
 * @brief Yield current thread to allow other thread to run.
 *
 * @param id The thread unique id.
 *
 * @return The result of thread yield operation.
 */
i32p_t _impl_thread_yield(void)
{
    if (!kernel_isInThreadMode()) {
        return _PCER;
    }

    return kernel_privilege_invoke((const void *)_thread_yield_privilege_routine, NULL);
}

/**
 * @brief Delete a idle thread, and erase the stack data.
 *
 * @param id The thread unique id.
 *
 * @return The result of thread delete operation.
 */
i32p_t _impl_thread_delete(os_id_t id)
{
    if (_thread_id_isInvalid(id)) {
        return _PCER;
    }

    if (!_thread_object_isInit(id)) {
        return _PCER;
    }

    arguments_t arguments[] = {
        [0] = {.u32_val = (u32_t)id},
    };

    return kernel_privilege_invoke((const void *)_thread_delete_privilege_routine, arguments);
}

/**
 * @brief Put the current running thread into sleep mode with timeout condition.
 *
 * @param timeout_ms The time user defined.
 *
 * @return The result of thread sleep operation.
 */
i32p_t _impl_thread_sleep(u32_t timeout_ms)
{
    if (!timeout_ms) {
        return _PCER;
    }

    if (!kernel_isInThreadMode()) {
        return _PCER;
    }

    arguments_t arguments[] = {
        [0] = {.u32_val = (u32_t)timeout_ms},
    };

    return kernel_privilege_invoke((const void *)_thread_sleep_privilege_routine, arguments);
}

/**
 * @brief Get thread snapshot informations.
 *
 * @param instance The thread instance number.
 * @param pMsgs The kernel snapshot information pointer.
 *
 * @return TRUE: Operation pass, FALSE: Operation failed.
 */
b_t thread_snapshot(u32_t instance, kernel_snapshot_t *pMsgs)
{
#if defined KTRACE
    thread_context_t *pCurThread = NULL;
    u32_t offset = 0u;
    os_id_t id = OS_INVALID_ID_VAL;
    u32_t sv_ms = 0u;

    ENTER_CRITICAL_SECTION();

    offset = sizeof(thread_context_t) * instance;
    pCurThread = (thread_context_t *)(kernel_member_id_toContainerStartAddress(KERNEL_MEMBER_THREAD) + offset);
    id = kernel_member_containerAddress_toUnifiedid((u32_t)pCurThread);
    os_memset((u8_t *)pMsgs, 0x0u, sizeof(kernel_snapshot_t));

    if (_thread_id_isInvalid(id)) {
        EXIT_CRITICAL_SECTION();
        return FALSE;
    }

    sv_ms = kernel_schedule_time_get();
    if (pCurThread->head.linker.pList == _thread_list_waitingHeadGet()) {
        pMsgs->pState = "wait";
        pMsgs->thread.delay = sv_ms - pCurThread->schedule.analyze.exit_ms;
    } else if (pCurThread->head.linker.pList == _thread_list_pendingHeadGet()) {
        if (id == _thread_id_runtime_get()) {
            pMsgs->pState = "run";
            pMsgs->thread.delay = sv_ms - pCurThread->schedule.analyze.run_ms;
        } else {
            pMsgs->pState = "pend";
            pMsgs->thread.delay = sv_ms - pCurThread->schedule.analyze.pend_ms;
        }
    } else if (pCurThread->head.linker.pList) {
        pMsgs->pState = "wait";
        pMsgs->thread.delay = sv_ms - pCurThread->schedule.analyze.exit_ms;
    } else {
        pMsgs->pState = "unused";

        EXIT_CRITICAL_SECTION();
        return FALSE;
    }

    pMsgs->id = pCurThread->head.id;
    pMsgs->pName = pCurThread->head.pName;

    pMsgs->thread.priority = pCurThread->priority.level;
    pMsgs->thread.current_psp = (u32_t)pCurThread->PSPStartAddr;

    u32_t *pData_u32 = (u32_t *)pCurThread->pStackAddr;
    u32_t unused = 0u;

    while ((*pData_u32 == STACT_UNUSED_FRAME_MARK) && (unused < pCurThread->stackSize)) {
        pData_u32++;
        unused++;
    }
    unused *= sizeof(u32_t);
    pMsgs->thread.ram = ((pCurThread->stackSize - unused) * 100u) / pCurThread->stackSize;
    pMsgs->thread.cpu = kernel_thread_use_percent_take(pCurThread->head.id);

    EXIT_CRITICAL_SECTION();
    return TRUE;
#else
    return FALSE;
#endif
}

#ifdef __cplusplus
}
#endif
