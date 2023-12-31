/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "kernal.h"
#include "compiler.h"
#include "clock_systick.h"
#include "at_rtos.h"

#ifdef __cplusplus
extern "C" {
#endif

const static at_rtos_api_t at_rtos_init =
{
    .thread_init = thread_init,
    .thread_sleep = thread_sleep,
    .thread_resume = thread_resume,
    .thread_suspend = thread_suspend,
    .thread_yield = thread_yield,
    .thread_delete = thread_delete,

    .timer_init = timer_init,
    .timer_start = timer_start,
    .timer_stop = timer_stop,
    .timer_isBusy = timer_isBusy,
    .timer_system_total_ms = timer_system_total_ms,

    .semaphore_init = semaphore_init,
    .semaphore_take = semaphore_take,
    .semaphore_give = semaphore_give,
    .semaphore_flush = semaphore_flush,

    .mutex_init = mutex_init,
    .mutex_lock = mutex_lock,
    .mutex_unlock = mutex_unlock,

    .event_init = event_init,
    .event_set = event_set,
    .event_wait = event_wait,

    .queue_init = queue_init,
    .queue_send = queue_send,
    .queue_receive = queue_receive,

    .os_id_is_invalid = os_id_is_invalid,
    .kernal_run = kernal_rtos_run,
};

at_rtos_api_t *atos = (at_rtos_api_t*)&at_rtos_init;

/* The gloable kernal symbol */
ATOS_STACK_DEFINE(g_stack_kernalThread, KERNAL_THREAD_STACK_SIZE);

static u8_t g_kernal_object_container[KERNAL_MEMBER_MAP_NUMBER] = {0u};
static list_t g_kernal_member_list[KERNAL_MEMBER_LIST_NUMBER] = {LIST_NULL};
static kernal_member_setting_t g_kernal_member_setting[KERNAL_MEMBER_NUMBER] =
{
    [KERNAL_MEMBER_THREAD]         = {KERNAL_MEMBER_MAP_1, (SET_BITS(KERNAL_MEMBER_LIST_THREAD_WAIT, KERNAL_MEMBER_LIST_THREAD_EXIT))},
    [KERNAL_MEMBER_TIMER_INTERNAL] = {KERNAL_MEMBER_MAP_2, (SET_BITS(KERNAL_MEMBER_LIST_TIMER_STOP, KERNAL_MEMBER_LIST_TIMER_RUN))},
    [KERNAL_MEMBER_TIMER]          = {KERNAL_MEMBER_MAP_3, (SET_BITS(KERNAL_MEMBER_LIST_TIMER_STOP, KERNAL_MEMBER_LIST_TIMER_RUN))},
    [KERNAL_MEMBER_SEMAPHORE]      = {KERNAL_MEMBER_MAP_4, (SET_BITS(KERNAL_MEMBER_LIST_SEMAPHORE_LOCK, KERNAL_MEMBER_LIST_SEMAPHORE_UNLOCK))},
    [KERNAL_MEMBER_MUTEX]          = {KERNAL_MEMBER_MAP_5, (SET_BITS(KERNAL_MEMBER_LIST_MUTEX_LOCK, KERNAL_MEMBER_LIST_MUTEX_UNLOCK))},
    [KERNAL_MEMBER_EVENT]          = {KERNAL_MEMBER_MAP_6, (SET_BITS(KERNAL_MEMBER_LIST_EVENT_INACTIVE, KERNAL_MEMBER_LIST_EVENT_ACTIVE))},
    [KERNAL_MEMBER_QUEUE]          = {KERNAL_MEMBER_MAP_7, (SET_BIT(KERNAL_MEMBER_LIST_QUEUE_INIT))},
};

static kernal_context_t g_kernal_object =
{
    .current = 0u,
    .list = LIST_NULL,
    .run = FALSE,
    .thread =
    {
        .thId.val = OS_INVALID_ID,
        .semId.val = OS_INVALID_ID,
    },
    .member =
    {
        .pListContainer = (list_t*)&g_kernal_member_list[0],
        .pMemoryContainer = (u8_t*)&g_kernal_object_container[0],
        .pSetting = (kernal_member_setting_t*)&g_kernal_member_setting[0],
    },
};

__ASM void kernal_run_theFirstThread(u32_t sp);
static u32_t _kernal_start_privilege_routine(arguments_t *pArgs);

list_t *kernal_member_list_get(u8_t member_id, u8_t list_id)
{
    return (list_t*)(((member_id < KERNAL_MEMBER_NUMBER) && (list_id < KERNAL_MEMBER_LIST_NUMBER)) ?
                     ((g_kernal_object.member.pSetting[member_id].list & SET_BIT(list_id)) ?
                     (&g_kernal_object.member.pListContainer[list_id]):(NULL)) : (NULL));
}

u8_t* kernal_member_unified_id_toContainerAddress(u32_t unified_id)
{
    return (u8_t*)((unified_id < KERNAL_MEMBER_MAP_NUMBER) ? (&g_kernal_object.member.pMemoryContainer[unified_id]) : (NULL));
}

u32_t kernal_member_containerAddress_toUnifiedid(u32_t container_address)
{
    u32_t start = (u32_t)(u8_t*)&g_kernal_object.member.pMemoryContainer[0];
    return (u32_t)(((container_address >= start) && (container_address < (start + KERNAL_MEMBER_MAP_NUMBER))) ? (container_address - start) : (OS_INVALID_ID));
}

u32_t kernal_member_id_toUnifiedIdStart(u8_t member_id)
{
    return ((member_id < KERNAL_MEMBER_NUMBER) ? (((member_id) ? (g_kernal_object.member.pSetting[member_id-1].mem): (0u))) : (OS_INVALID_ID));
}

u32_t kernal_member_id_toUnifiedIdEnd(u8_t member_id)
{
    return ((member_id < KERNAL_MEMBER_NUMBER) ? (g_kernal_object.member.pSetting[member_id].mem) : (OS_INVALID_ID));
}

u32_t kernal_member_id_toUnifiedIdRange(u8_t member_id)
{
    return ((member_id < KERNAL_MEMBER_NUMBER) ? (kernal_member_id_toUnifiedIdEnd(member_id) - kernal_member_id_toUnifiedIdStart(member_id)) : (0u));
}

u8_t* kernal_member_id_toContainerStartAddress(u32_t member_id)
{
    return (u8_t*)((member_id < KERNAL_MEMBER_NUMBER) ? (kernal_member_unified_id_toContainerAddress(kernal_member_id_toUnifiedIdStart(member_id))) : (NULL));
}

u8_t* kernal_member_id_toContainerEndAddress(u32_t member_id)
{
    return (u8_t*)((member_id < KERNAL_MEMBER_NUMBER) ? (kernal_member_unified_id_toContainerAddress(kernal_member_id_toUnifiedIdEnd(member_id))) : (NULL));
}

u32_t kernal_member_id_unifiedConvert(u8_t member_id, u32_t unified_id)
{
    return (u32_t)((member_id < KERNAL_MEMBER_NUMBER) ?
            (((unified_id >= kernal_member_id_toUnifiedIdStart(member_id)) ?
                (unified_id - kernal_member_id_toUnifiedIdStart(member_id)) : (0U))
                / kernal_member_id_toUnifiedIdRange(member_id)) : (0U));
}


/**
 * @brief Check if the kernal member unique id if is's invalid.
 *
 * Check if the kernal member unique id if is's invalid.
 *
 * @param id The provided unique id.
 *
 * @retval TRUE The id is invalid
 *         FALSE The id is valid
 */
b_t kernal_member_unified_id_isInvalid(u32_t member_id, u32_t unified_id)
{
    return ((member_id >= KERNAL_MEMBER_NUMBER) ||
            (unified_id == OS_INVALID_ID) ||
            (unified_id < kernal_member_id_toUnifiedIdStart(member_id)) ||
            (unified_id >= kernal_member_id_toUnifiedIdEnd(member_id)));
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
b_t _impl_kernal_os_id_is_invalid(struct os_id id)
{
    return ((id.val == OS_INVALID_ID) || (id.val >= KERNAL_MEMBER_MAP_NUMBER));
}

u32_t kernal_member_unified_id_threadToTimer(u32_t unified_id)
{
    u32_t uid = OS_INVALID_ID;

    if (!kernal_member_unified_id_isInvalid(KERNAL_MEMBER_THREAD, unified_id))
    {
        uid = (unified_id - kernal_member_id_toUnifiedIdStart(KERNAL_MEMBER_THREAD)) / sizeof(thread_context_t);
        uid = (uid * sizeof(timer_context_t)) + kernal_member_id_toUnifiedIdStart(KERNAL_MEMBER_TIMER_INTERNAL);
    }
    return uid;
}

u32_t kernal_member_unified_id_timerToThread(u32_t unified_id)
{
    u32_t uid = OS_INVALID_ID;

    if (!kernal_member_unified_id_isInvalid(KERNAL_MEMBER_TIMER_INTERNAL, unified_id))
    {
        uid = (unified_id - kernal_member_id_toUnifiedIdStart(KERNAL_MEMBER_TIMER_INTERNAL)) / sizeof(timer_context_t);
        uid = (uid * sizeof(thread_context_t)) + kernal_member_id_toUnifiedIdStart(KERNAL_MEMBER_THREAD);
    }
    return uid;
}

u8_t kernal_member_unified_id_toId(u32_t unified_id)
{
    u8_t member_id = KERNAL_MEMBER_THREAD;

    while((member_id < KERNAL_MEMBER_NUMBER) &&
           (unified_id < kernal_member_id_toUnifiedIdStart(member_id)) ||
           (unified_id >= kernal_member_id_toUnifiedIdEnd(member_id)))
    {
        member_id++;
    }

    return (u8_t)member_id;
}

/**
 * @brief Get the kernal thread pending list head address.
 *
 * Get the kernal thread pending list head address.
 *
 * @param NONE.
 *
 * @retval VALUE The pending list head address.
 */
list_t* kernal_list_pendingHeadGet(void)
{
    return (list_t*)(&g_kernal_object.list);
}

/**
 * @brief Get the next thread id.
 *
 * Get the next thread id.
 *
 * @param NONE.
 *
 * @retval VALUE The next thread id.
 */
os_id_t kernal_thread_nextIdGet(void)
{
    ENTER_CRITICAL_SECTION();
    list_t *pListPending = (list_t *)kernal_list_pendingHeadGet();
    linker_head_t* pHead = (linker_head_t*)(pListPending->pHead);
    EXIT_CRITICAL_SECTION();

    return pHead->id;
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
os_id_t kernal_thread_runIdGet(void)
{
    return (os_id_t)g_kernal_object.current;
}

thread_context_t* kernal_thread_runContextGet(void)
{
    return (thread_context_t*)kernal_member_unified_id_toContainerAddress(kernal_thread_runIdGet());
}

/**
 * @brief Initialize a stack frame.
 *
 * Initialize a stack frame.
 *
 * @param pEntryFunction The entry function pointer.
 * @param pAddress The stack address.
 * @param size The stack size.
 *
 * @retval VALUE The PSP stack address.
 */
u32_t kernal_stack_frame_init(void (*pEntryFunction)(void), u32_t *pAddress, u32_t size)
{
    u32_t psp_frame = (u32_t)pAddress + size - sizeof(stack_snapshot_t);

    psp_frame = STACK_ADDRESS_DOWN(psp_frame);

    ((stack_snapshot_t *)psp_frame)->xPSR   = SET_BIT(24);              /* xPSR */
    ((stack_snapshot_t *)psp_frame)->R15_PC = (u32_t)pEntryFunction;    /* PC   */
    ((stack_snapshot_t *)psp_frame)->R14_LR = 0xFFFFFFFDu;              /* LR   */

    ((stack_snapshot_t *)psp_frame)->R12    = 0x12121212u;              /* R12  */
    ((stack_snapshot_t *)psp_frame)->R3     = 0x03030303u;              /* R3   */
    ((stack_snapshot_t *)psp_frame)->R2     = 0x02020202u;              /* R2   */
    ((stack_snapshot_t *)psp_frame)->R1     = 0x01010101u;              /* R1   */
    ((stack_snapshot_t *)psp_frame)->R0     = 0x0B0B0B0Bu;              /* R0   */

#if defined ARCH_CORTEX_M0
    ((stack_snapshot_t *)psp_frame)->cm0.R11    = 0x11111111u;              /* R11  */
    ((stack_snapshot_t *)psp_frame)->cm0.R10    = 0x10101010u;              /* R10  */
    ((stack_snapshot_t *)psp_frame)->cm0.R9     = 0x09090909u;              /* R9   */
    ((stack_snapshot_t *)psp_frame)->cm0.R8     = 0x08080808u;              /* R8   */
    ((stack_snapshot_t *)psp_frame)->cm0.R7     = 0x07070707u;              /* R7   */
    ((stack_snapshot_t *)psp_frame)->cm0.R6     = 0x06060606u;              /* R6   */
    ((stack_snapshot_t *)psp_frame)->cm0.R5     = 0x05050505u;              /* R5   */
    ((stack_snapshot_t *)psp_frame)->cm0.R4     = 0x04040404u;              /* R4   */
#else
    ((stack_snapshot_t *)psp_frame)->cmx.R11    = 0x11111111u;              /* R11  */
    ((stack_snapshot_t *)psp_frame)->cmx.R10    = 0x10101010u;              /* R10  */
    ((stack_snapshot_t *)psp_frame)->cmx.R9     = 0x09090909u;              /* R9   */
    ((stack_snapshot_t *)psp_frame)->cmx.R8     = 0x08080808u;              /* R8   */
    ((stack_snapshot_t *)psp_frame)->cmx.R7     = 0x07070707u;              /* R7   */
    ((stack_snapshot_t *)psp_frame)->cmx.R6     = 0x06060606u;              /* R6   */
    ((stack_snapshot_t *)psp_frame)->cmx.R5     = 0x05050505u;              /* R5   */
    ((stack_snapshot_t *)psp_frame)->cmx.R4     = 0x04040404u;              /* R4   */
#endif

#if defined FPU_ENABLED
    #if (THREAD_PSP_WITH_PRIVILEGED)
        ((stack_snapshot_t *)psp_frame)->CONTROL = BIT1;                 /* PSP with privileged */
    #else
        ((stack_snapshot_t *)psp_frame)->CONTROL = BIT(1) & BIT(0);      /* PSP with Unprivileged */
    #endif
    ((stack_snapshot_t *)psp_frame)->EXC_RETURN = 0xFFFFFFFDu;          /* EXC_RETURN */
#endif

    return (u32_t)psp_frame;
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
static b_t kernal_thread_node_Order_compare_condition(list_node_t *pCurNode, list_node_t *pExtractNode)
{
    thread_context_t *pCurThread = (thread_context_t *)pCurNode;
    thread_context_t *pExtractThread = (thread_context_t *)pExtractNode;

    if ((!pCurThread) || (!pExtractThread))
    {
        /* no available thread */
        return FALSE;
    }

    if (pCurThread->priority.level < pExtractThread->priority.level)
    {
        /* Find a right position and don't need to do schedule */
        return FALSE;
    }

    return TRUE;
}

/**
 * @brief Push one thread context into exit list.
 *
 * Push one thread context into exit list.
 *
 * @param pCurHead The pointer of the thread linker head.
 *
 * @retval NONE .
 */
void kernal_thread_list_transfer_toExit(linker_head_t *pCurHead)
{
    ENTER_CRITICAL_SECTION();

    list_t *pToExitList = (list_t*)kernal_member_list_get(KERNAL_MEMBER_THREAD, KERNAL_MEMBER_LIST_THREAD_EXIT);
    linker_list_transaction_common(&pCurHead->linker, pToExitList, LIST_TAIL);

    EXIT_CRITICAL_SECTION();
}

/**
 * @brief Push one thread context into entry list.
 *
 * Push one thread context into entry list.
 *
 * @param pCurHead The pointer of the thread linker head.
 *
 * @retval NONE .
 */
void kernal_thread_list_transfer_toEntry(linker_head_t *pCurHead)
{
    ENTER_CRITICAL_SECTION();

    list_t *pToEntryList = (list_t*)kernal_member_list_get(KERNAL_MEMBER_THREAD, KERNAL_MEMBER_LIST_THREAD_ENTRY);
    linker_list_transaction_common(&pCurHead->linker, pToEntryList, LIST_TAIL);

    EXIT_CRITICAL_SECTION();
}

/**
 * @brief Push one thread context into target list.
 *
 * Push one thread context into target list.
 *
 * @param pCurHead The pointer of the thread linker head.
 *
 * @retval NONE .
 */
void kernal_thread_list_transfer_toTargetBlocking(linker_head_t *pCurHead, list_t *pToBlockingList)
{
    ENTER_CRITICAL_SECTION();

    if (pToBlockingList)
    {
        linker_list_transaction_specific(&pCurHead->linker, pToBlockingList, kernal_thread_node_Order_compare_condition);
    }

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
void kernal_thread_list_transfer_toPend(linker_head_t *pCurHead)
{
    ENTER_CRITICAL_SECTION();

    list_t *pToPendList = (list_t *)kernal_list_pendingHeadGet();
    linker_list_transaction_specific(&pCurHead->linker, pToPendList, kernal_thread_node_Order_compare_condition);

    EXIT_CRITICAL_SECTION();
}


/**
 * @brief The thread is trying to sleep.
 *
 * The thread is trying to sleep.
 *
 * @param id The thread unique id.
 * @param pToList The blocking list.
 * @param pSleepObject Which object casue this.
 * @param reason The sleep reason.
 * @param timeout If the thread has sleep time setting.
 * @param pCallback The timeout callback function pointer.
 *
 * @retval VALUE The result of operation.
 */
u32p_t kernal_thread_exit_trigger(os_id_t id, os_id_t hold, list_t *pToList, u32_t timeout_us, void (*pCallback)(os_id_t))
{
    thread_context_t *pCurThread = (thread_context_t*)(kernal_member_unified_id_toContainerAddress(id));

    pCurThread->schedule.hold = hold;
    pCurThread->schedule.exit.pToList = pToList;
    pCurThread->schedule.exit.timeout_us = timeout_us;
    pCurThread->schedule.exit.pTimeoutCallFun = pCallback;

    kernal_thread_list_transfer_toExit((linker_head_t*)&pCurThread->head);
    return kernal_thread_schedule_request();
}

/**
 * @brief Try to trigger one thread wakeup.
 *
 * Try to trigger one thread wakeup.
 *
 * @param id The thread unique id.
 * @param release The thread release id.
 * @param result The thread entry result.
 *
 * @retval VALUE The result of operation.
 */
u32p_t kernal_thread_entry_trigger(os_id_t id, os_id_t release, u32_t result, void (*pCallback)(os_id_t))
{
    thread_context_t *pCurThread = (thread_context_t*)(kernal_member_unified_id_toContainerAddress(id));

    pCurThread->schedule.entry.release = release;
    pCurThread->schedule.entry.result = result;
    pCurThread->schedule.entry.pEntryCallFun = pCallback;

    kernal_thread_list_transfer_toEntry((linker_head_t*)&pCurThread->head);
    return kernal_thread_schedule_request();
}

b_t kernal_thread_exit_schedule(void)
{
    list_iterator_t it = ITERATION_NULL;
    thread_context_t *pCurThread = NULL;
    thread_exit_t *pExit = NULL;
    b_t request = FALSE;

    /* The thread block */
    list_t *pList = (list_t *)kernal_member_list_get(KERNAL_MEMBER_THREAD, KERNAL_MEMBER_LIST_THREAD_EXIT);
    list_iterator_init(&it, pList);
    while (list_iterator_next_condition(&it, (void *)&pCurThread))
    {
    	pExit = &pCurThread->schedule.exit;

        if (pExit->timeout_us)
        {
            _impl_thread_timer_start(kernal_member_unified_id_threadToTimer(pCurThread->head.id), pExit->timeout_us, pExit->pTimeoutCallFun);
            if (pExit->timeout_us != WAIT_FOREVER)
            {
                request = TRUE;
            }
        }

        kernal_thread_list_transfer_toTargetBlocking((linker_head_t*)&pCurThread->head, (list_t*)pExit->pToList);
    }

    return request;
}

void kernal_thread_entry_schedule(void)
{
    thread_context_t *pCurThread = NULL;
    list_iterator_t it = ITERATION_NULL;
    thread_entry_t *pEntry = NULL;

    list_t *pList = (list_t *)kernal_member_list_get(KERNAL_MEMBER_THREAD, KERNAL_MEMBER_LIST_THREAD_ENTRY);
    list_iterator_init(&it, pList);
    while (list_iterator_next_condition(&it, (void *)&pCurThread))
    {
        pEntry = &pCurThread->schedule.entry;

        if (pEntry->pEntryCallFun)
        {
            pEntry->pEntryCallFun(pCurThread->head.id);
            pEntry->pEntryCallFun = NULL;
            pEntry->release = OS_INVALID_ID;
            pCurThread->schedule.hold = OS_INVALID_ID;
        }

        kernal_thread_list_transfer_toPend((linker_head_t *)&pCurThread->head);
    }
}

/**
 * @brief kernal schedule in pendsv interrupt content.
 *
 * kernal schedule in pendsv interrupt content.
 *
 * @param ppCurThreadPsp The current thread psp address.
 * @param ppNextThreadPSP The next thread psp address.
 *
 * @retval NONE.
 */
void kernal_scheduler_inPendSV_c(u32_t **ppCurPsp, u32_t **ppNextPSP)
{
    if (kernal_thread_exit_schedule())
    {
        _impl_kernal_timer_schedule_request();
    }

    kernal_thread_entry_schedule();

    os_id_t next = kernal_thread_nextIdGet();

    *ppCurPsp = (u32_t*)thread_psp_addressGet(g_kernal_object.current);
    *ppNextPSP = (u32_t*)thread_psp_addressGet(next);

    g_kernal_object.current = next;
}

/**
 * @brief To set PendSV.
 *
 * To set PendSV.
 *
 * @param NONE.
 *
 * @retval NONE.
 */
void kernal_setPendSV(void)
{
    SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

/**
 * @brief Check if kernal is in privilege mode.
 *
 * Check if kernal is in privilege mode.
 *
 * @param NONE.
 *
 * @retval TRUE The kernal is in privilege mode.
 *         FALSE The kernal is not in privilege mode.
 */
b_t kernal_isInPrivilegeMode(void)
{
    if (__get_IPSR())
    {
        return TRUE;
    }

    if (__get_PRIMASK() == SET_BIT(0))
    {
        return TRUE;
    }

    return FALSE;  //The kernal is not in privilege mode
}

/**
 * @brief Check if kernal is in thread mode.
 *
 * Check if kernal is in thread mode.
 *
 * @param NONE.
 *
 * @retval TRUE The kernal is in thread mode.
 *         FALSE The kernal is not in thread mode.
 */
b_t kernal_isInThreadMode(void)
{
    if (__get_IPSR())
    {
        return FALSE;
    }
    return TRUE;  //The kernal is not in Thread mode
}

/**
 * @brief Request the kernal do thread schedule.
 *
 * Request the kernal do thread schedule.
 *
 * @param NONE.
 *
 * @retval NONE.
 */
u32p_t kernal_thread_schedule_request(void)
{
    if (kernal_isInPrivilegeMode())
    {
        kernal_setPendSV();
        return PC_SC_SUCCESS;
    }
    else
    {
        return PC_FAILED(PC_CMPT_KERNAL);
    }
}

/**
 * @brief To check if the kernal OS is running.
 *
 * To check if the kernal OS is running.
 *
 * @param NONE.
 *
 * @retval TRUE The kernal OS is running.
 *         FALSE The kernal OS is inactive.
 */
b_t kernal_rtos_isRun(void)
{
    return (b_t)((g_kernal_object.run) ? (TRUE) : (FALSE));
}

void kernal_message_notification(void)
{
    semaphore_give(g_kernal_object.thread.semId);
}

u32_t Kernal_message_arrived(void)
{
    return semaphore_take(g_kernal_object.thread.semId, SEMAPHORE_WAIT_FOREVER);
}

/**
 * @brief The kernal thread only serve for RTOS.
 *
 * The kernal thread only serve for RTOS.
 *
 * @param NONE.
 *
 * @retval NONE.
 */
void kernal_rtos_thread(void)
{
    while (1)
    {
        if (PC_IOK(Kernal_message_arrived()))
        {
            _impl_timer_reamining_elapsed_handler();
        }
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
static u32_t _kernal_start_privilege_routine(arguments_t *pArgs)
{
    UNUSED_MSG(pArgs);

    ENTER_CRITICAL_SECTION();

    g_kernal_object.thread.thId = thread_init(kernal_rtos_thread,
                                              g_stack_kernalThread,
                                              KERNAL_THREAD_STACK_SIZE,
                                              PRIORITY_KERNAL_SCHDULE,
                                              KERNAL_THREAD_NAME_STRING);

    if (os_id_is_invalid(g_kernal_object.thread.thId))
    {
       return PC_FAILED(PC_CMPT_KERNAL);
    }

    g_kernal_object.thread.semId = semaphore_init(0u, SEMAPHORE_LIMITATION_BINARY_COUNT, KERNAL_THREAD_NAME_STRING);
    if (os_id_is_invalid(g_kernal_object.thread.semId))
    {
        return PC_FAILED(PC_CMPT_KERNAL);
    }

    NVIC_SetPriority(PendSV_IRQn, 0xFF); // Set PendSV to lowest possible priority
    NVIC_SetPriority(SVCall_IRQn, 0xFF); // Set SV to lowest possible priority
    NVIC_SetPriority(SysTick_IRQn, 0xFFu);
    _impl_clock_time_init(_impl_timer_elapsed_handler);

    g_kernal_object.current = kernal_thread_nextIdGet();
    g_kernal_object.run = TRUE;

    EXIT_CRITICAL_SECTION();

    kernal_run_theFirstThread(*thread_psp_addressGet(g_kernal_object.current));

    // nothing arrive
    return PC_FAILED(PC_CMPT_KERNAL);
}

/**
 * @brief The kernal OS start to run.
 *
 * The kernal OS start to run.
 *
 * @param NONE.
 *
 * @retval NONE.
 */
u32p_t kernal_rtos_run(void)
{
    u32p_t status = PC_SC_SUCCESS;

    if (!kernal_rtos_isRun())
    {
        return kernal_privilege_invoke(_kernal_start_privilege_routine, NULL);
    }

   return status;
}

/**
 * @brief kernal call privilege function in SVC interrupt content.
 *
 * kernal call privilege function in SVC interrupt content.
 *
 * @param svc_args The function arguments.
 *
 * @retval NONE.
 */
void kernal_privilege_call_inSVC_c(u32_t *svc_args)
{
    /*
    * Stack contains:
    * r0, r1, r2, r3, r12, r14, the return address and xPSR
    * First argument (r0) is svc_args[0]
    * Put the result into R0
    */
    u8_t svc_number = ((u8_t *)svc_args[6])[-2];

    if (svc_number == SVC_KERNAL_INVOKE_NUMBER)
    {
        pPrivilege_callFunc_t pCall = (pPrivilege_callFunc_t)svc_args[0];

		svc_args[0] = (u32_t)pCall((arguments_t*)svc_args[1]);
    }
}

u32_t _impl_kernal_privilege_invoke(const void* pCallFun, arguments_t* pArgs)
{
    if (!pCallFun)
    {
        return PC_FAILED(PC_CMPT_KERNAL);
    }

    u32_t ret = 0u;

    if (kernal_isInPrivilegeMode())
    {
        ENTER_CRITICAL_SECTION();
        pPrivilege_callFunc_t pCall = (pPrivilege_callFunc_t)pCallFun;
        ret = (u32_t)pCall((arguments_t *)pArgs);
        EXIT_CRITICAL_SECTION();
    }
    else
    {
        ret = kernal_svc_call((u32_t)pCallFun, (u32_t)pArgs, 0u, 0u);
    }

    return ret;
}

#ifdef __cplusplus
}
#endif
