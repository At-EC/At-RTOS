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
#include "arch/arch.h"

#ifdef __cplusplus
extern "C" {
#endif

#define _PC_CMPT_FAILED       PC_FAILED(PC_CMPT_THREAD)

static void  _thread_callback_fromTimeOut(os_id_t id);
static os_id_t _thread_init_privilege_routine(arguments_t* pArgs);
static u32_t _thread_resume_privilege_routine(arguments_t* pArgs);
static u32_t _thread_suspend_privilege_routine(arguments_t* pArgs);
static u32_t _thread_yield_privilege_routine(arguments_t* pArgs);
static u32_t _thread_sleep_privilege_routine(arguments_t* pArgs);
static u32_t _thread_delete_privilege_routine(arguments_t* pArgs);

/**
 * @brief Get the thread context based on provided unique id.
 *
 * Get the thread context based on provided unique id, and then return the thread context pointer.
 *
 * @param id Thread unique id.
 *
 * @retval VALUE The thread context.
 */
static thread_context_t* _thread_object_contextGet(os_id_t id)
{
    return (thread_context_t*)kernal_member_unified_id_toContainerAddress(id);
}

/**
 * @brief Get the current runtime thread id.
 *
 * Get the current runtime thread id.
 *
 * @param NONE.
 *
 * @retval VALUE The id of current thread.
 */
static os_id_t _thread_id_runtime_get(void)
{
    return (os_id_t)kernal_thread_runIdGet();
}

/**
 * @brief Get the current runtime thread object context.
 *
 * Get the current runtime thread object context.
 *
 * @param NONE.
 *
 * @retval VALUE The id of current thread object context.
 */
static thread_context_t* _thread_object_runtime_get(void)
{
    return (thread_context_t*)_thread_object_contextGet(kernal_thread_runIdGet());
}

/**
 * @brief Get the thread waiting list head address.
 *
 * Get the thread waiting list head address.
 *
 * @param NONE.
 *
 * @retval VALUE The waiting list head address.
 */
static list_t* _thread_list_waitingHeadGet(void)
{
    return (list_t*)kernal_member_list_get(KERNAL_MEMBER_THREAD, KERNAL_MEMBER_LIST_THREAD_WAIT);
}

/**
 * @brief Get the thread pending list head address.
 *
 * Get the thread pending list head address.
 *
 * @param NONE.
 *
 * @retval VALUE The pending list head address.
 */
static list_t* _thread_list_pendingHeadGet(void)
{
    return (list_t*)kernal_list_pendingHeadGet();
}

/**
 * @brief Push one thread context into uninitialized status.
 *
 * Push one thread context into uninitialized status.
 *
 * @param pCurHead The pointer of the thread linker head.
 *
 * @retval NONE .
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
 * Push one thread context into waiting list.
 *
 * @param pCurHead The pointer of the thread linker head.
 *
 * @retval NONE .
 */
static void _thread_list_transfer_toWait(linker_head_t *pCurHead)
{
    ENTER_CRITICAL_SECTION();

    list_t *pToWaitList = (list_t *)_thread_list_waitingHeadGet();
    linker_list_transaction_common(&pCurHead->linker, pToWaitList, LIST_TAIL);

    EXIT_CRITICAL_SECTION();
}

/**
 * @brief Push one thread context into Wakeup list.
 *
 * Push one thread context into Wakeup list.
 *
 * @param pCurHead The pointer of the thread linker head.
 *
 * @retval NONE .
 */
static void _thread_list_transfer_toEntry(linker_head_t *pCurHead)
{
    ENTER_CRITICAL_SECTION();

    kernal_thread_list_transfer_toEntry(pCurHead);

    EXIT_CRITICAL_SECTION();
}

/**
 * @brief Push one thread context into pending list.
 *
 * Push one thread context into pending list.
 *
 * @param pCurHead The pointer of the thread linker head.
 *
 * @retval NONE .
 */
static void _thread_list_transfer_toPend(linker_head_t *pCurHead)
{
    ENTER_CRITICAL_SECTION();

    kernal_thread_list_transfer_toPend(pCurHead);

    EXIT_CRITICAL_SECTION();
}

/**
 * @brief Pick up a highest priority thread from the pending list.
 *
 * Pick up a highest priority thread from the pending list.
 *
 * @param NONE
 *
 * @retval VALUE The highest thread head.
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
 * Pick up the next priority thread from the pending list.
 *
 * @param NONE
 *
 * @retval VALUE The next thread head.
 */
static linker_head_t* _thread_linker_Head_next_fromPending(void)
{
	linker_head_t* pHead = _thread_linker_head_fromPending();

    return (linker_head_t*)((pHead) ? (pHead->linker.node.pNext) : (NULL));
}

/**
 * @brief Get the current thread PSP stack address.
 *
 * Get the current thread PSP stack address.
 *
 * @param id The thread unique id.
 *
 * @retval VALUE The PSP stacke address.
 */
u32_t* thread_psp_addressGet(os_id_t id)
{
    thread_context_t* pCurThread = (thread_context_t*)_thread_object_contextGet(id);

    return (u32_t*)&pCurThread->PSPStartAddr;
}

/**
 * @brief Check if the thread unique id if is's invalid.
 *
 * Check if the thread unique id if is's invalid.
 *
 * @param id The provided unique id.
 *
 * @retval TRUE The id is invalid
 *         FALSE The id is valid
 */
static b_t _thread_id_isInvalid(os_id_t id)
{
    return kernal_member_unified_id_isInvalid(KERNAL_MEMBER_THREAD, id);
}

/**
 * @brief Check if the thread object if is's initialized.
 *
 * Check if the thread unique id if is's initialization.
 *
 * @param id The provided unique id.
 *
 * @retval TRUE The id is initialized.
 *         FALSE The id isn't initialized.
 */
static b_t thread_object_isInit(os_id_t id)
{
    thread_context_t *pCurThread = (thread_context_t *)_thread_object_contextGet(id);

    if (pCurThread)
    {
        return ((pCurThread->head.linker.pList) ? (TRUE) : (FALSE));
    }
    else
    {
        return FALSE;
    }
}

/**
 * @brief The thread timeout callback fucntion.
 *
 * The thread timeout callback fucntion.
 *
 * @param id The thread unique id.
 *
 * @retval NONE.
 */
static void _thread_callback_fromTimeOut(os_id_t id)
{
    timer_context_t *pCurTimer = (timer_context_t *)kernal_member_unified_id_toContainerAddress(id);
    kernal_thread_entry_trigger(kernal_member_unified_id_timerToThread(id), id, PC_SC_TIMEOUT, NULL);
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
u32_t _impl_thread_os_id_to_number(os_id_t id)
{
    return (u32_t)(_thread_id_isInvalid(id) ? (0u) : (id - kernal_member_id_toUnifiedIdStart(KERNAL_MEMBER_THREAD)) / sizeof(thread_context_t));
}

/**
 * @brief Initialize a new thread.
 *
 * Initialize a new thread.
 *
 * @param pEntryFun The thread entry function pointer.
 * @param pAddress The thread stack address.
 * @param size The thread stack size.
 * @param priority The thread priority.
 * @param pName The thread name.
 *
 * @retval VALUE The thread unique id.
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

    if (_memcmp((char_t*)KERNAL_THREAD_NAME_STRING, (char_t*)pName, sizeof(KERNAL_THREAD_NAME_STRING)))
    {
        if ((priority == OS_PRIORITY_KERNAL_THREAD_IDLE_LEVEL) || (priority == OS_PRIORITY_KERNAL_THREAD_SCHEDULE_LEVEL))
        {
            return OS_INVALID_ID;
        }
    }

    arguments_t arguments[] =
    {
        [0] = {(u32_t)pEntryFun},
        [1] = {(u32_t)pAddress},
        [2] = {(u32_t)size},
        [3] = {(u32_t)priority},
        [4] = {(u32_t)pName},
    };

    return kernal_privilege_invoke(_thread_init_privilege_routine, arguments);
}

/**
 * @brief Resume a thread to run.
 *
 * Resume a thread to run.
 *
 * @param id The thread unique id.
 *
 * @retval VALUE The result of thread resume.
 */
u32p_t _impl_thread_resume(os_id_t id)
{
    if (_thread_id_isInvalid(id))
    {
        return _PC_CMPT_FAILED;
    }

    if (!thread_object_isInit(id))
    {
        return _PC_CMPT_FAILED;
    }

    arguments_t arguments[] =
    {
        [0] = {(u32_t)id},
    };

    return kernal_privilege_invoke(_thread_resume_privilege_routine, arguments);
}

/**
 * @brief Suspend a thread to allow other thread to run.
 *
 * Suspend a thread to allow other thread to run.
 *
 * @param id The thread unique id.
 *
 * @retval VALUE The result of thread Suspend.
 */
u32p_t _impl_thread_suspend(os_id_t id)
{
    if (_thread_id_isInvalid(id))
    {
        return _PC_CMPT_FAILED;
    }

    if (!thread_object_isInit(id))
    {
        return _PC_CMPT_FAILED;
    }

    arguments_t arguments[] =
    {
        [0] = {(u32_t)id},
    };

    return kernal_privilege_invoke(_thread_suspend_privilege_routine, arguments);
}

/**
 * @brief Yield current thread to allow other thread to run.
 *
 * Yield a thread to allow other thread to run.
 *
 * @param id The thread unique id.
 *
 * @retval VALUE The result of thread yield.
 */
u32p_t _impl_thread_yield(void)
{
    if (!kernal_isInThreadMode())
    {
        return _PC_CMPT_FAILED;
    }

    return kernal_privilege_invoke(_thread_yield_privilege_routine, NULL);
}

/**
 * @brief Delete a not current running thread, and the stack data will be erased but stack memory cann't be free.
 *
 * Delete a not current running thread, and the stack data will be erased but stack memory cann't be free.
 *
 * @param id The thread unique id.
 *
 * @retval VALUE The result of thread delete.
 */
u32p_t _impl_thread_delete(os_id_t id)
{
    if (_thread_id_isInvalid(id))
    {
        return _PC_CMPT_FAILED;
    }

    if (!thread_object_isInit(id))
    {
        return _PC_CMPT_FAILED;
    }

    arguments_t arguments[] =
    {
        [0] = {(u32_t)id},
    };

    return kernal_privilege_invoke(_thread_delete_privilege_routine, arguments);
}

/**
 * @brief Set thread into sleep mode with timeout function.
 *
 * Set thread into sleep mode with timeout function.
 *
 * @param timeout_ms The time user defined.
 *
 * @retval VALUE The result of thread sleep.
 */
u32p_t _impl_thread_sleep(u32_t timeout_ms)
{
    if (!timeout_ms)
    {
        return _PC_CMPT_FAILED;
    }

    if (!kernal_isInThreadMode())
    {
        return _PC_CMPT_FAILED;
    }

    arguments_t arguments[] =
    {
        [0] = {(u32_t)timeout_ms},
    };

    return kernal_privilege_invoke(_thread_sleep_privilege_routine, arguments);
}

/**
 * @brief Check the thread stack if the data overflow.
 *
 * Check the thread stack if the data overflow.
 *
 * @param NONE.
 *
 * @retval NONE.
 */
void thread_stack_check(void)
{
    thread_context_t *pCurThread = (thread_context_t *)kernal_member_id_toContainerStartAddress(KERNAL_MEMBER_THREAD);
    os_id_t id = kernal_member_id_toUnifiedIdStart(KERNAL_MEMBER_THREAD);

    while (((u32_t)pCurThread < (u32_t)kernal_member_id_toContainerEndAddress(KERNAL_MEMBER_THREAD)) && (pCurThread) && thread_object_isInit(id))
    {
        if (*pCurThread->pStackAddr != STACT_UNUSED_FRAME_MARK)
        {
            /* Restore it so further checks don't trigger this same error */
            *pCurThread->pStackAddr = STACT_UNUSED_FRAME_MARK;
            while(1){};
        }
        pCurThread++;
        id++;
    }
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
static os_id_t _thread_init_privilege_routine(arguments_t* pArgs)
{
    ENTER_CRITICAL_SECTION();

    pThread_entryFunc_t pEntryFun = (pThread_entryFunc_t)pArgs[0].u32_val;
    u32_t *pAddress = (u32_t *)pArgs[1].u32_val;
    u32_t size = (u32_t)pArgs[2].u32_val;
    u8_t priority = (u8_t)pArgs[3].u32_val;
    const char_t *pName = (const char_t *)pArgs[4].u32_val;

    thread_context_t *pCurThread = (thread_context_t *)kernal_member_id_toContainerStartAddress(KERNAL_MEMBER_THREAD);
    os_id_t id = kernal_member_id_toUnifiedIdStart(KERNAL_MEMBER_THREAD);

    do {
        if (!thread_object_isInit(id))
        {
            _memset((char_t*)pCurThread, 0x0u, sizeof(thread_context_t));

            pCurThread->head.id = id;
            pCurThread->head.pName = pName;

            pCurThread->priority.level = priority;
            pCurThread->pEntryFunc = pEntryFun;
            pCurThread->pStackAddr = pAddress;
            pCurThread->stackSize = size;
            _memset((char_t*)pCurThread->pStackAddr, STACT_UNUSED_DATA, size);
            pCurThread->PSPStartAddr = (u32_t)kernal_stack_frame_init(pEntryFun, pCurThread->pStackAddr, pCurThread->stackSize);
            _impl_thread_timer_init(kernal_member_unified_id_threadToTimer(id));
            _thread_list_transfer_toPend((linker_head_t*)&pCurThread->head);
            break;
        }

        pCurThread++;
        id = kernal_member_containerAddress_toUnifiedid((u32_t)pCurThread);
    } while ((u32_t)pCurThread < (u32_t)kernal_member_id_toContainerEndAddress(KERNAL_MEMBER_THREAD));

    id = ((!_thread_id_isInvalid(id)) ? (id) : (OS_INVALID_ID_VAL));

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
static u32p_t _thread_resume_privilege_routine(arguments_t* pArgs)
{
    ENTER_CRITICAL_SECTION();
    os_id_t id = (os_id_t)pArgs[0].u32_val;
    u32p_t postcode = PC_SC_SUCCESS;

    if (_thread_id_runtime_get() != id)
    {
        thread_context_t *pCurThread = (thread_context_t *)_thread_object_contextGet(id);
        _thread_list_transfer_toEntry((linker_head_t*)&pCurThread->head);
        postcode = kernal_thread_schedule_request();
    }

    EXIT_CRITICAL_SECTION();
    return postcode;
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
static u32p_t _thread_suspend_privilege_routine(arguments_t* pArgs)
{
    ENTER_CRITICAL_SECTION();
    os_id_t id = (os_id_t)pArgs[0].u32_val;
    u32p_t postcode = PC_SC_SUCCESS;

    thread_context_t *pCurThread = (thread_context_t *)_thread_object_contextGet(id);
    if (_thread_linker_Head_next_fromPending())
    {
        _thread_list_transfer_toWait((linker_head_t*)&pCurThread->head);
        postcode = kernal_thread_schedule_request();
    }
    else
    {
        postcode = _PC_CMPT_FAILED;
    }

    EXIT_CRITICAL_SECTION();

    return postcode;
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
static u32p_t _thread_yield_privilege_routine(arguments_t* pArgs)
{
    ENTER_CRITICAL_SECTION();
    UNUSED_MSG(pArgs);

    u32p_t postcode = PC_SC_SUCCESS;
    thread_context_t *pCurThread = (thread_context_t *)_thread_object_runtime_get();
    if (_thread_linker_Head_next_fromPending())
    {
        _thread_list_transfer_toWait((linker_head_t*)&pCurThread->head);
        postcode = kernal_thread_schedule_request();
    }
    else
    {
        postcode = _PC_CMPT_FAILED;
    }

    EXIT_CRITICAL_SECTION();

    return postcode;
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
            postcode = kernal_thread_schedule_request();
        }
    }

    EXIT_CRITICAL_SECTION();

    return postcode;
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
static u32p_t _thread_sleep_privilege_routine(arguments_t* pArgs)
{
    ENTER_CRITICAL_SECTION();
    u32_t timeout_ms = (u32_t)pArgs[0].u32_val;

    thread_context_t *pCurThread = (thread_context_t *)kernal_thread_runContextGet();
    u32p_t postcode = kernal_thread_exit_trigger(pCurThread->head.id, OS_INVALID_ID, _thread_list_waitingHeadGet(), timeout_ms, _thread_callback_fromTimeOut);

    EXIT_CRITICAL_SECTION();

    return postcode;
}

#ifdef __cplusplus
}
#endif
