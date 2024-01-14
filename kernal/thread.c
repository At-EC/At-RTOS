/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "basic.h"
#include "kernal.h"
#include "timer.h"
#include "thread.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Local unique postcode.
 */
#define _PC_CMPT_FAILED       PC_FAILED(PC_CMPT_THREAD)

/**
 * The local function lists for current file internal use.
 */
static os_id_t _thread_init_privilege_routine(arguments_t* pArgs);
static u32_t   _thread_resume_privilege_routine(arguments_t* pArgs);
static u32_t   _thread_suspend_privilege_routine(arguments_t* pArgs);
static u32_t   _thread_yield_privilege_routine(arguments_t* pArgs);
static u32_t   _thread_sleep_privilege_routine(arguments_t* pArgs);
static u32_t   _thread_delete_privilege_routine(arguments_t* pArgs);

/**
 * @brief Get the thread context based on provided unique id.
 *
 * @param id Thread unique id.
 *
 * @return The pointer of the current unique id thread context.
 */
static thread_context_t* _thread_object_contextGet(os_id_t id)
{
    return (thread_context_t*)_impl_kernal_member_unified_id_toContainerAddress(id);
}

/**
 * @brief Get the current runtime thread id.
 *
 * @return The id of current running thread.
 */
static os_id_t _thread_id_runtime_get(void)
{
    return (os_id_t)_impl_kernal_thread_runIdGet();
}

/**
 * @brief Get the current runtime thread object context.
 *
 * @return The pointer of current running thread.
 */
static thread_context_t* _thread_object_runtime_get(void)
{
    return (thread_context_t*)_thread_object_contextGet(_impl_kernal_thread_runIdGet());
}

/**
 * @brief Get the waiting thread list head.
 *
 * @return The value of the waiting list head.
 */
static list_t* _thread_list_waitingHeadGet(void)
{
    return (list_t*)_impl_kernal_member_list_get(KERNAL_MEMBER_THREAD, KERNAL_MEMBER_LIST_THREAD_WAIT);
}

/**
 * @brief Get the pending thread list head.
 *
 * @return The value of the pending list head.
 */
static list_t* _thread_list_pendingHeadGet(void)
{
    return (list_t*)_impl_kernal_list_pendingHeadGet();
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

    _impl_kernal_thread_list_transfer_toEntry(pCurHead);

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

    _impl_kernal_thread_list_transfer_toPend(pCurHead);

    EXIT_CRITICAL_SECTION();
}

/**
 * @brief Pick up a highest priority thread from the pending list.
 *
 * @return The pointer of the pending thread head.
 */
static linker_head_t* _thread_linker_head_fromPending(void)
{
    ENTER_CRITICAL_SECTION();
    list_t *pListPending = (list_t *)_thread_list_pendingHeadGet();
    EXIT_CRITICAL_SECTION();

    return (linker_head_t*)(pListPending->pHead);
}

/**
 * @brief Pick up the next priority thread from the pending list.
 *
 * @return The pointer of the next pending thread head.
 */
static linker_head_t* _thread_linker_Head_next_fromPending(void)
{
    linker_head_t* pHead = _thread_linker_head_fromPending();

    return (linker_head_t*)((pHead) ? (pHead->linker.node.pNext) : (NULL));
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
    return _impl_kernal_member_unified_id_isInvalid(KERNAL_MEMBER_THREAD, id);
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
    thread_context_t *pCurThread = (thread_context_t *)_thread_object_contextGet(id);

    return ((pCurThread) ? (((pCurThread->head.linker.pList) ? (TRUE) : (FALSE))) : FALSE);
}

/**
 * @brief The thread timeout callback fucntion.
 *
 * @param id The thread unique id.
 */
static void _thread_callback_fromTimeOut(os_id_t id)
{
    _impl_kernal_thread_entry_trigger(_impl_kernal_member_unified_id_timerToThread(id), id, PC_SC_TIMEOUT, NULL);
}

/**
 * @brief Convert the internal os id to kernal member number.
 *
 * @return The Member number.
 */
u32_t _impl_thread_os_id_to_number(os_id_t id)
{
    return (u32_t)(_thread_id_isInvalid(id) ? (0u) : (id - _impl_kernal_member_id_toUnifiedIdStart(KERNAL_MEMBER_THREAD)) / sizeof(thread_context_t));
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
os_id_t _impl_thread_init(pThread_entryFunc_t pEntryFun, u32_t *pAddress, u32_t size, u8_t priority, const char_t *pName)
{
    if (!pEntryFun)
    {
        return OS_INVALID_ID;
    }

    if (!pAddress)
    {
        return OS_INVALID_ID;
    }

    if ((size < STACK_SIZE_MINIMUM) || (size > STACK_SIZE_MAXIMUM))
    {
        return OS_INVALID_ID;
    }

    if (priority > 0xFFu)
    {
        return OS_INVALID_ID;
    }

    if ((priority == OS_PRIORITY_KERNAL_THREAD_IDLE_LEVEL) || (priority == OS_PRIORITY_KERNAL_THREAD_SCHEDULE_LEVEL))
    {
        return OS_INVALID_ID;
    }

    arguments_t arguments[] =
    {
        [0] = {(u32_t)pEntryFun},
        [1] = {(u32_t)pAddress},
        [2] = {(u32_t)size},
        [3] = {(u32_t)priority},
        [4] = {(u32_t)pName},
    };

    return _impl_kernal_privilege_invoke(_thread_init_privilege_routine, arguments);
}

/**
 * @brief Resume a thread to run.
 *
 * @param id The thread unique id.
 *
 * @return The result of thread resume operation.
 */
u32p_t _impl_thread_resume(os_id_t id)
{
    if (_thread_id_isInvalid(id))
    {
        return _PC_CMPT_FAILED;
    }

    if (!_thread_object_isInit(id))
    {
        return _PC_CMPT_FAILED;
    }

    arguments_t arguments[] =
    {
        [0] = {(u32_t)id},
    };

    return _impl_kernal_privilege_invoke(_thread_resume_privilege_routine, arguments);
}

/**
 * @brief Suspend a thread to permit another to run.
 *
 * @param id The thread unique id.
 *
 * @return The result of thread suspend operation.
 */
u32p_t _impl_thread_suspend(os_id_t id)
{
    if (_thread_id_isInvalid(id))
    {
        return _PC_CMPT_FAILED;
    }

    if (!_thread_object_isInit(id))
    {
        return _PC_CMPT_FAILED;
    }

    arguments_t arguments[] =
    {
        [0] = {(u32_t)id},
    };

    return _impl_kernal_privilege_invoke(_thread_suspend_privilege_routine, arguments);
}

/**
 * @brief Yield current thread to allow other thread to run.
 *
 * @param id The thread unique id.
 *
 * @return The result of thread yield operation.
 */
u32p_t _impl_thread_yield(void)
{
    if (!_impl_kernal_isInThreadMode())
    {
        return _PC_CMPT_FAILED;
    }

    return _impl_kernal_privilege_invoke(_thread_yield_privilege_routine, NULL);
}

/**
 * @brief Delete a idle thread, and erase the stack data.
 *
 * @param id The thread unique id.
 *
 * @return The result of thread delete operation.
 */
u32p_t _impl_thread_delete(os_id_t id)
{
    if (_thread_id_isInvalid(id))
    {
        return _PC_CMPT_FAILED;
    }

    if (!_thread_object_isInit(id))
    {
        return _PC_CMPT_FAILED;
    }

    arguments_t arguments[] =
    {
        [0] = {(u32_t)id},
    };

    return _impl_kernal_privilege_invoke(_thread_delete_privilege_routine, arguments);
}

/**
 * @brief Put the current running thread into sleep mode with timeout condition.
 *
 * @param timeout_ms The time user defined.
 *
 * @return The result of thread sleep operation.
 */
u32p_t _impl_thread_sleep(u32_t timeout_ms)
{
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
        [0] = {(u32_t)timeout_ms},
    };

    return _impl_kernal_privilege_invoke(_thread_sleep_privilege_routine, arguments);
}

/**
 * @brief It's sub-routine running at privilege mode.
 *
 * @param pArgs The function argument packages.
 *
 * @return The result of privilege routine.
 */
static os_id_t _thread_init_privilege_routine(arguments_t* pArgs)
{
    ENTER_CRITICAL_SECTION();

    pThread_entryFunc_t pEntryFun = (pThread_entryFunc_t)pArgs[0].u32_val;
    u32_t *pAddress = (u32_t *)pArgs[1].u32_val;
    u32_t size = (u32_t)pArgs[2].u32_val;
    u8_t priority = (u8_t)pArgs[3].u32_val;
    const char_t *pName = (const char_t *)pArgs[4].u32_val;

    u32_t offset = (sizeof(thread_context_t)*KERNAL_APPLICATION_THREAD_INSTANCE);
    thread_context_t *pCurThread = (thread_context_t *)(_impl_kernal_member_id_toContainerStartAddress(KERNAL_MEMBER_THREAD) + offset);
    os_id_t id = _impl_kernal_member_id_toUnifiedIdStart(KERNAL_MEMBER_THREAD) + offset;

    do {
        if (!_thread_object_isInit(id))
        {
            _memset((char_t*)pCurThread, 0x0u, sizeof(thread_context_t));

            pCurThread->head.id = id;
            pCurThread->head.pName = pName;

            pCurThread->priority.level = priority;
            pCurThread->pEntryFunc = pEntryFun;
            pCurThread->pStackAddr = pAddress;
            pCurThread->stackSize = size;
            _memset((char_t*)pCurThread->pStackAddr, STACT_UNUSED_DATA, size);
            pCurThread->PSPStartAddr = (u32_t)_impl_kernal_stack_frame_init(pEntryFun, pCurThread->pStackAddr, pCurThread->stackSize);
            _impl_thread_timer_init(_impl_kernal_member_unified_id_threadToTimer(id));
            _thread_list_transfer_toPend((linker_head_t*)&pCurThread->head);
            break;
        }

        pCurThread++;
        id = _impl_kernal_member_containerAddress_toUnifiedid((u32_t)pCurThread);
    } while ((u32_t)pCurThread < (u32_t)_impl_kernal_member_id_toContainerEndAddress(KERNAL_MEMBER_THREAD));

    id = ((!_thread_id_isInvalid(id)) ? (id) : (OS_INVALID_ID_VAL));

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
static u32p_t _thread_resume_privilege_routine(arguments_t* pArgs)
{
    ENTER_CRITICAL_SECTION();
    os_id_t id = (os_id_t)pArgs[0].u32_val;
    u32p_t postcode = PC_SC_SUCCESS;

    if (_thread_id_runtime_get() != id)
    {
        thread_context_t *pCurThread = (thread_context_t *)_thread_object_contextGet(id);
        _thread_list_transfer_toEntry((linker_head_t*)&pCurThread->head);
        postcode = _impl_kernal_thread_schedule_request();
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
static u32p_t _thread_suspend_privilege_routine(arguments_t* pArgs)
{
    ENTER_CRITICAL_SECTION();
    os_id_t id = (os_id_t)pArgs[0].u32_val;
    u32p_t postcode = PC_SC_SUCCESS;

    thread_context_t *pCurThread = (thread_context_t *)_thread_object_contextGet(id);
    if (_thread_linker_Head_next_fromPending())
    {
        _thread_list_transfer_toWait((linker_head_t*)&pCurThread->head);
        postcode = _impl_kernal_thread_schedule_request();
    }
    else
    {
        postcode = _PC_CMPT_FAILED;
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
static u32p_t _thread_yield_privilege_routine(arguments_t* pArgs)
{
    ENTER_CRITICAL_SECTION();
    UNUSED_MSG(pArgs);

    u32p_t postcode = PC_SC_SUCCESS;
    thread_context_t *pCurThread = (thread_context_t *)_thread_object_runtime_get();
    if (_thread_linker_Head_next_fromPending())
    {
        _thread_list_transfer_toWait((linker_head_t*)&pCurThread->head);
        postcode = _impl_kernal_thread_schedule_request();
    }
    else
    {
        postcode = _PC_CMPT_FAILED;
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
static u32p_t _thread_delete_privilege_routine(arguments_t* pArgs)
{
    ENTER_CRITICAL_SECTION();
    os_id_t id = (os_id_t)pArgs[0].u32_val;
    u32p_t postcode = PC_SC_SUCCESS;

    if (id != _thread_id_runtime_get())
    {
        thread_context_t *pCurThread = (thread_context_t *)_thread_object_contextGet(id);
        if (_thread_linker_Head_next_fromPending())
        {
            _thread_list_transfer_toUninitialized((linker_head_t*)&pCurThread->head);
            _memset((char_t*)pCurThread->pStackAddr, STACT_UNUSED_DATA, pCurThread->stackSize);
            _memset((char_t*)pCurThread, 0x0u, sizeof(thread_context_t));
            postcode = _impl_kernal_thread_schedule_request();
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
static u32p_t _thread_sleep_privilege_routine(arguments_t* pArgs)
{
    ENTER_CRITICAL_SECTION();
    u32_t timeout_ms = (u32_t)pArgs[0].u32_val;

    thread_context_t *pCurThread = (thread_context_t *)_impl_kernal_thread_runContextGet();
    u32p_t postcode = _impl_kernal_thread_exit_trigger(pCurThread->head.id, OS_INVALID_ID, _thread_list_waitingHeadGet(), timeout_ms, _thread_callback_fromTimeOut);

    EXIT_CRITICAL_SECTION();

    return postcode;
}

#ifdef __cplusplus
}
#endif
