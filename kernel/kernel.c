/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 **/

#include "at_rtos.h"
#include "kernel.h"
#include "timer.h"
#include "compiler.h"
#include "clock_tick.h"
#include "ktype.h"
#include "postcode.h"
#include "trace.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Local unique postcode.
 */
#define _PCER PC_IER(PC_OS_CMPT_KERNEL_2)

/**
 * Local kernel member setting container.
 */
static kernel_member_setting_t g_kernel_member_setting[KERNEL_MEMBER_NUMBER] = {
    [KERNEL_MEMBER_THREAD] = {KERNEL_MEMBER_MAP_1, (SET_BITS(KERNEL_MEMBER_LIST_THREAD_WAIT, KERNEL_MEMBER_LIST_THREAD_EXIT))},
    [KERNEL_MEMBER_TIMER_INTERNAL] = {KERNEL_MEMBER_MAP_2, (SET_BITS(KERNEL_MEMBER_LIST_TIMER_STOP, KERNEL_MEMBER_LIST_TIMER_RUN))},
    [KERNEL_MEMBER_TIMER] = {KERNEL_MEMBER_MAP_3, (SET_BITS(KERNEL_MEMBER_LIST_TIMER_STOP, KERNEL_MEMBER_LIST_TIMER_RUN))},
    [KERNEL_MEMBER_SEMAPHORE] = {KERNEL_MEMBER_MAP_4, (SET_BIT(KERNEL_MEMBER_LIST_SEMAPHORE_INIT))},
    [KERNEL_MEMBER_MUTEX] = {KERNEL_MEMBER_MAP_5, (SET_BITS(KERNEL_MEMBER_LIST_MUTEX_LOCK, KERNEL_MEMBER_LIST_MUTEX_UNLOCK))},
    [KERNEL_MEMBER_EVENT] = {KERNEL_MEMBER_MAP_6, (SET_BIT(KERNEL_MEMBER_LIST_EVENT_INIT))},
    [KERNEL_MEMBER_QUEUE] = {KERNEL_MEMBER_MAP_7, (SET_BIT(KERNEL_MEMBER_LIST_QUEUE_INIT))},
    [KERNEL_MEMBER_POOL] = {KERNEL_MEMBER_MAP_8, (SET_BIT(KERNEL_MEMBER_LIST_POOL_INIT))},
    [KERNEL_MEMBER_PUBLISH] = {KERNEL_MEMBER_MAP_9, (SET_BITS(KERNEL_MEMBER_LIST_PUBLISH_INIT, KERNEL_MEMBER_LIST_PUBLISH_PEND))},
    [KERNEL_MEMBER_SUBSCRIBE] = {KERNEL_MEMBER_MAP_10, (SET_BIT(KERNEL_MEMBER_LIST_SUBSCRIBE_INIT))},
};

/**
 * Local kernel member container.
 */
static u8_t g_kernel_member_container[KERNEL_MEMBER_MAP_NUMBER] = {0u};

/**
 * Local kernel supported member list container.
 */
static list_t g_kernel_member_list[KERNEL_MEMBER_LIST_NUMBER] = {LIST_NULL};

/**
 * Local kernel resource
 */
static kernel_context_t g_kernel_resource = {
    .current = 0u,
    .list = LIST_NULL,
    .run = FALSE,
    .pendsv_ms = 0u,
    .member =
        {
            .pListContainer = (list_t *)&g_kernel_member_list[0],
            .pMemoryContainer = (u8_t *)&g_kernel_member_container[0],
            .pSetting = (kernel_member_setting_t *)&g_kernel_member_setting[0],
        },
};

/**
 * The local function lists for current file internal use.
 */
static i32p_t _kernel_start_privilege_routine(arguments_t *pArgs);

/**
 * @brief To set a PendSV.
 */
static void _kernel_setPendSV(void)
{
    port_setPendSV();
}

/**
 * @brief Check if kernel is in privilege mode.
 *
 * @return The true indicates the kernel is in privilege mode.
 */
static b_t _kernel_isInPrivilegeMode(void)
{
    return port_isInInterruptContent();
}

/**
 * @brief Update pendsv executed time.
 */
static void _kernel_pendsv_time_update(void)
{
    g_kernel_resource.pendsv_ms = timer_total_system_ms_get();
}

/**
 * @brief Get pendsv executed time.
 *
 * @return The value of pendsv executed time.
 */
static u32_t _kernel_pendsv_time_get(void)
{
    return g_kernel_resource.pendsv_ms;
}

/**
 * @brief kernel schedule exit time analyze.
 */
static void _kernel_schedule_exit_time_analyze(os_id_t id)
{
    /* Nothing to do */
}

/**
 * @brief kernel schedule entry time analyze.
 */
static void _kernel_schedule_entry_time_analyze(os_id_t id)
{
#if defined KTRACE
    thread_context_t *pEntryThread = (thread_context_t *)kernel_member_unified_id_toContainerAddress(id);
    pEntryThread->schedule.analyze.pend_ms = _kernel_pendsv_time_get();
#endif
}

/**
 * @brief kernel schedule run time analyze.
 */
static void _kernel_schedule_run_time_analyze(os_id_t from, os_id_t to)
{
#if defined KTRACE
    thread_context_t *pFromThread = (thread_context_t *)kernel_member_unified_id_toContainerAddress(from);
    thread_context_t *pToThread = (thread_context_t *)kernel_member_unified_id_toContainerAddress(to);
    u32_t sv_ms = _kernel_pendsv_time_get();

    pFromThread->schedule.analyze.active_ms += (u32_t)(sv_ms - pFromThread->schedule.analyze.run_ms);

    pFromThread->schedule.analyze.exit_ms = sv_ms;
    pToThread->schedule.analyze.run_ms = sv_ms;
#endif
}

/**
 * @brief Get the thread PSP stack address.
 *
 * @param id The thread unique id.
 *
 * @return The PSP stacke address.
 */
static u32_t *_kernel_thread_PSP_Get(os_id_t id)
{
    thread_context_t *pCurThread = (thread_context_t *)kernel_member_unified_id_toContainerAddress(id);

    return (u32_t *)&pCurThread->PSPStartAddr;
}

/**
 * @brief Get the next thread id.
 *
 * @return The next thread id.
 */
static os_id_t _kernel_thread_nextIdGet(void)
{
    ENTER_CRITICAL_SECTION();
    list_t *pListPending = (list_t *)kernel_list_pendingHeadGet();
    linker_head_t *pHead = (linker_head_t *)(pListPending->pHead);
    EXIT_CRITICAL_SECTION();

    return pHead->id;
}

/**
 * @brief Compare the priority between the current and extract thread.
 *
 * @param pCurNode The pointer of the current thread node.
 * @param pExtractNode The pointer of the extract thread node.
 *
 * @return The false indicates it's a right position and it can kill the loop calling.
 */
static b_t _kernel_thread_node_Order_compare_condition(list_node_t *pCurNode, list_node_t *pExtractNode)
{
    thread_context_t *pCurThread = (thread_context_t *)pCurNode;
    thread_context_t *pExtractThread = (thread_context_t *)pExtractNode;

    if ((!pCurThread) || (!pExtractThread)) {
        /* no available thread */
        return FALSE;
    }

    if (pCurThread->priority.level <= pExtractThread->priority.level) {
        /* Find a right position and doesn't has to do schedule */
        return FALSE;
    }

    return TRUE;
}

/**
 * @brief Push one thread context into target list.
 *
 * @param pCurHead The pointer of the thread linker head.
 */
static void _kernel_thread_list_transfer_toTargetBlocking(linker_head_t *pCurHead, list_t *pToBlockingList)
{
    ENTER_CRITICAL_SECTION();

    if (pToBlockingList) {
        linker_list_transaction_specific(&pCurHead->linker, pToBlockingList, _kernel_thread_node_Order_compare_condition);
    }

    EXIT_CRITICAL_SECTION();
}

/**
 * @brief kernel thread try to do entry schedule in the PendSV routine.
 */
static void _kernel_thread_entry_schedule(void)
{
    thread_context_t *pCurThread = NULL;
    list_iterator_t it = ITERATION_NULL;
    thread_entry_t *pEntry = NULL;

    list_t *pList = (list_t *)kernel_member_list_get(KERNEL_MEMBER_THREAD, KERNEL_MEMBER_LIST_THREAD_ENTRY);
    list_iterator_init(&it, pList);
    while (list_iterator_next_condition(&it, (void *)&pCurThread)) {
        pEntry = &pCurThread->schedule.entry;

        if (pEntry->pEntryCallFun) {
            pEntry->pEntryCallFun(pCurThread->head.id);
            pEntry->pEntryCallFun = NULL;
            pEntry->release = OS_INVALID_ID_VAL;
            pCurThread->schedule.hold = OS_INVALID_ID_VAL;
        }

        _kernel_schedule_entry_time_analyze(pCurThread->head.id);
        kernel_thread_list_transfer_toPend((linker_head_t *)&pCurThread->head);
    }
}

/**
 * @brief kernel thread try to do exit schedule in the PendSV routine.
 *
 * @return The true indicates the kernel request a timer schedule.
 */
static b_t _kernel_thread_exit_schedule(void)
{
    list_iterator_t it = ITERATION_NULL;
    thread_context_t *pCurThread = NULL;
    thread_exit_t *pExit = NULL;
    b_t request = FALSE;

    /* The thread block */
    list_t *pList = (list_t *)kernel_member_list_get(KERNEL_MEMBER_THREAD, KERNEL_MEMBER_LIST_THREAD_EXIT);
    list_iterator_init(&it, pList);
    while (list_iterator_next_condition(&it, (void *)&pCurThread)) {
        pExit = &pCurThread->schedule.exit;

        if (pExit->timeout_us) {
            timer_start_for_thread(kernel_member_unified_id_threadToTimer(pCurThread->head.id), pExit->timeout_us, pExit->pTimeoutCallFun);

            if (pExit->timeout_us != OS_TIME_FOREVER_VAL) {
                request = TRUE;
            }
        }

        _kernel_schedule_exit_time_analyze(pCurThread->head.id);
        _kernel_thread_list_transfer_toTargetBlocking((linker_head_t *)&pCurThread->head, (list_t *)pExit->pToList);
    }

    return request;
}

/**
 * @brief It's sub-routine running at privilege mode.
 *
 * @param pArgs The function argument packages.
 *
 * @return The result of privilege routine.
 */
static i32p_t _kernel_start_privilege_routine(arguments_t *pArgs)
{
    UNUSED_MSG(pArgs);

    ENTER_CRITICAL_SECTION();

    kthread_init();
    port_interrupt_init();
    clock_time_init(timer_elapsed_handler);

    g_kernel_resource.current = _kernel_thread_nextIdGet();
    g_kernel_resource.run = TRUE;

    EXIT_CRITICAL_SECTION();

    port_run_theFirstThread(*(_kernel_thread_PSP_Get(g_kernel_resource.current)));

    // nothing arrive
    return _PCER;
}

/**
 * @brief Get the kernel member ending unified id according to the member id.
 *
 * @param member_id The kernel member id.
 *
 * @return The value of the kernel member ending unified id.
 */
static u32_t _kernel_member_id_toUnifiedIdEnd(u8_t member_id)
{
    if (member_id >= KERNEL_MEMBER_NUMBER) {
        return OS_INVALID_ID_VAL;
    }

    return (u32_t)g_kernel_resource.member.pSetting[member_id].mem;
}

/**
 * @brief Get the kernel member unified id range according to the member id.
 *
 * @param member_id The kernel member id.
 *
 * @return The value of the kernel member unified id range.
 */
static u32_t _kernel_member_id_toUnifiedIdRange(u8_t member_id)
{
    if (member_id >= KERNEL_MEMBER_NUMBER) {
        return 0u;
    }

    return (u32_t)(_kernel_member_id_toUnifiedIdEnd(member_id) - kernel_member_id_toUnifiedIdStart(member_id));
}

/**
 * @brief To check if the kernel message arrived.
 */
static i32p_t _kernel_message_arrived(void)
{
    return kthread_message_arrived();
}

/**
 * @brief Get pendsv executed time.
 *
 * @return The value of pendsv executed time.
 */
u32_t kernel_schedule_time_get(void)
{
    return _kernel_pendsv_time_get();
}

/**
 * @brief Push one semaphore context into lock list.
 *
 * @param pCurHead The pointer of the semaphore linker head.
 */
void kernel_semaphore_list_transfer_toInit(linker_head_t *pCurHead)
{
    ENTER_CRITICAL_SECTION();

    list_t *pToLockList = (list_t *)(list_t *)kernel_member_list_get(KERNEL_MEMBER_SEMAPHORE, KERNEL_MEMBER_LIST_SEMAPHORE_INIT);
    linker_list_transaction_common(&pCurHead->linker, pToLockList, LIST_TAIL);

    EXIT_CRITICAL_SECTION();
}

/**
 * @brief kernel thread use percent value take.
 *
 * @return The value of thread use percent.
 */
u32_t kernel_thread_use_percent_take(os_id_t id)
{
#if defined KTRACE
    thread_context_t *pCurThread = (thread_context_t *)kernel_member_unified_id_toContainerAddress(id);

    pCurThread->schedule.analyze.percent =
        (pCurThread->schedule.analyze.active_ms * 1000u) / (_kernel_pendsv_time_get() - pCurThread->schedule.analyze.cycle_ms);
    pCurThread->schedule.analyze.active_ms = 0u;
    pCurThread->schedule.analyze.cycle_ms = _kernel_pendsv_time_get();

    return pCurThread->schedule.analyze.percent;
#else
    return 0u;
#endif
}

/**
 * @brief kernel schedule in PendSV interrupt content.
 *
 * @param ppCurThreadPsp The current thread psp address.
 * @param ppNextThreadPSP The next thread psp address.
 */
void kernel_scheduler_inPendSV_c(u32_t **ppCurPsp, u32_t **ppNextPSP)
{
    _kernel_pendsv_time_update();

    if (_kernel_thread_exit_schedule()) {
        timer_schedule();
    }
    _kernel_thread_entry_schedule();

    os_id_t next = _kernel_thread_nextIdGet();

    *ppCurPsp = (u32_t *)_kernel_thread_PSP_Get(g_kernel_resource.current);
    *ppNextPSP = (u32_t *)_kernel_thread_PSP_Get(next);

    _kernel_schedule_run_time_analyze(g_kernel_resource.current, next);
    g_kernel_resource.current = next;
}

/**
 * @brief Get the kernel member list according to the member id and list id.
 *
 * @param member_id The kernel member unique id.
 * @param list_id The list unique id.
 *
 * @return The pointer of the list pointer.
 */
list_t *kernel_member_list_get(u8_t member_id, u8_t list_id)
{
    if (member_id >= KERNEL_MEMBER_NUMBER) {
        return NULL;
    }

    if (list_id >= KERNEL_MEMBER_LIST_NUMBER) {
        return NULL;
    }

    if (!(g_kernel_resource.member.pSetting[member_id].list & SET_BIT(list_id))) {
        return NULL;
    }

    return (list_t *)&g_kernel_resource.member.pListContainer[list_id];
}

/**
 * @brief Get the kernel member address according to the unified id.
 *
 * @param unified_id The kernel member unified id.
 *
 * @return The pointer of the memeber address.
 */
u8_t *kernel_member_unified_id_toContainerAddress(u32_t unified_id)
{
    if (unified_id >= KERNEL_MEMBER_MAP_NUMBER) {
        return NULL;
    }

    return (u8_t *)(&g_kernel_resource.member.pMemoryContainer[unified_id]);
}

/**
 * @brief Get the kernel unified id according to the member address.
 *
 * @param container_address The kernel member address.
 *
 * @return The value of the kernel unified id.
 */
u32_t kernel_member_containerAddress_toUnifiedid(u32_t container_address)
{
    u32_t start = (u32_t)(u8_t *)&g_kernel_resource.member.pMemoryContainer[0];

    if (container_address < start) {
        return OS_INVALID_ID_VAL;
    }

    if (container_address >= (start + KERNEL_MEMBER_MAP_NUMBER)) {
        return OS_INVALID_ID_VAL;
    }

    return (u32_t)(container_address - start);
}

/**
 * @brief Get the kernel member start unified id according to the member id.
 *
 * @param member_id The kernel member id.
 *
 * @return The value of the kernel member unified id.
 */
u32_t kernel_member_id_toUnifiedIdStart(u8_t member_id)
{
    if (member_id >= KERNEL_MEMBER_NUMBER) {
        return OS_INVALID_ID_VAL;
    }

    if (!member_id) {
        return 0u;
    }

    return (u32_t)(g_kernel_resource.member.pSetting[member_id - 1].mem);
}

/**
 * @brief Get the kernel member start address according to the member unique id.
 *
 * @param member_id The kernel member id.
 *
 * @return The value of the kernel member address range.
 */
u8_t *kernel_member_id_toContainerStartAddress(u32_t member_id)
{
    if (member_id >= KERNEL_MEMBER_NUMBER) {
        return NULL;
    }

    return (u8_t *)kernel_member_unified_id_toContainerAddress(kernel_member_id_toUnifiedIdStart(member_id));
}

/**
 * @brief Get the kernel member ending address according to the member unique id.
 *
 * @param member_id The kernel member id.
 *
 * @return The value of the kernel member ending address.
 */
u8_t *kernel_member_id_toContainerEndAddress(u32_t member_id)
{
    if (member_id >= KERNEL_MEMBER_NUMBER) {
        return NULL;
    }

    return (u8_t *)kernel_member_unified_id_toContainerAddress(_kernel_member_id_toUnifiedIdEnd(member_id));
}

/**
 * @brief Get the kernel member number according to the member id and unified id.
 *
 * @param member_id The kernel member id.
 * @param unified_id The kernel member unified id.
 *
 * @return The value of the kernel member number.
 */
u32_t kernel_member_id_unifiedConvert(u8_t member_id, u32_t unified_id)
{
    if (member_id >= KERNEL_MEMBER_NUMBER) {
        return 0u;
    }

    u32_t diff = kernel_member_id_toUnifiedIdStart(member_id);
    if (unified_id >= diff) {
        diff = unified_id - diff;
    } else {
        diff = 0u;
    }

    return (u32_t)(diff / _kernel_member_id_toUnifiedIdRange(member_id));
}

/**
 * @brief Check if the kernel member unique id if is's invalid.
 *
 * @param member_id The kernel member unique id.
 * @param unified_id The kernel member unified id.
 *
 * @return The value of true is invalid, otherwise is valid.
 */
b_t kernel_member_unified_id_isInvalid(u32_t member_id, u32_t unified_id)
{
    if (member_id >= KERNEL_MEMBER_NUMBER) {
        return TRUE;
    }

    if (unified_id == OS_INVALID_ID_VAL) {
        return TRUE;
    }

    if (unified_id < kernel_member_id_toUnifiedIdStart(member_id)) {
        return TRUE;
    }

    if (unified_id >= _kernel_member_id_toUnifiedIdEnd(member_id)) {
        return TRUE;
    }

    return FALSE;
}

/**
 * @brief Check if the thread unique id if is's invalid.
 *
 * @param id The provided unique id.
 *
 * @return The value of true is invalid, otherwise is valid.
 */
b_t kernel_os_id_is_invalid(struct os_id id)
{
    if (id.val == OS_INVALID_ID_VAL) {
        return TRUE;
    }

    if (id.val == KERNEL_MEMBER_MAP_NUMBER) {
        return TRUE;
    }

    return FALSE;
}

/**
 * @brief Convert the thread id into timer id.
 *
 * @param unified_id The thread unified id.
 *
 * @return The value of timer unified id.
 */
u32_t kernel_member_unified_id_threadToTimer(u32_t unified_id)
{
    u32_t uid = OS_INVALID_ID_VAL;

    if (kernel_member_unified_id_isInvalid(KERNEL_MEMBER_THREAD, unified_id)) {
        return OS_INVALID_ID_VAL;
    }

    uid = (unified_id - kernel_member_id_toUnifiedIdStart(KERNEL_MEMBER_THREAD)) / sizeof(thread_context_t);

    return (u32_t)((uid * sizeof(timer_context_t)) + kernel_member_id_toUnifiedIdStart(KERNEL_MEMBER_TIMER_INTERNAL));
}

/**
 * @brief Convert the timer id into thread id.
 *
 * @param unified_id The timer unified id.
 *
 * @return The value of thread unified id.
 */
u32_t kernel_member_unified_id_timerToThread(u32_t unified_id)
{
    u32_t uid = OS_INVALID_ID_VAL;

    if (kernel_member_unified_id_isInvalid(KERNEL_MEMBER_TIMER_INTERNAL, unified_id)) {
        return OS_INVALID_ID_VAL;
    }

    uid = (unified_id - kernel_member_id_toUnifiedIdStart(KERNEL_MEMBER_TIMER_INTERNAL)) / sizeof(timer_context_t);
    return (u32_t)((uid * sizeof(thread_context_t)) + kernel_member_id_toUnifiedIdStart(KERNEL_MEMBER_THREAD));
}

/**
 * @brief Get the kernel member id.
 *
 * @param unified_id The provided unified id.
 *
 * @return The value of the kernel member id.
 */
u8_t kernel_member_unified_id_toId(u32_t unified_id)
{
    u8_t member_id = KERNEL_MEMBER_THREAD;

    while ((member_id < KERNEL_MEMBER_NUMBER) &&
           ((unified_id < kernel_member_id_toUnifiedIdStart(member_id)) || (unified_id >= _kernel_member_id_toUnifiedIdEnd(member_id)))) {

        member_id++;
    }

    return (u8_t)member_id;
}

/**
 * @brief Initialize a thread stack frame.
 *
 * @param pEntryFunction The entry function pointer.
 * @param pAddress The stack address.
 * @param size The stack size.
 *
 * @return The PSP stack address.
 */
u32_t kernel_stack_frame_init(void (*pEntryFunction)(void), u32_t *pAddress, u32_t size)
{
    return (u32_t)port_stack_frame_init(pEntryFunction, pAddress, size);
}

/**
 * @brief Get the kernel thread pending list head.
 *
 * @return The pending list head.
 */
list_t *kernel_list_pendingHeadGet(void)
{
    return (list_t *)(&g_kernel_resource.list);
}

/**
 * @brief Get the current running thread id.
 *
 * @return The id of current running thread.
 */
os_id_t kernel_thread_runIdGet(void)
{
    return (os_id_t)g_kernel_resource.current;
}

/**
 * @brief Get the current running thread context.
 *
 * @return The context pointer of current running thread.
 */
thread_context_t *kernel_thread_runContextGet(void)
{
    return (void *)kernel_member_unified_id_toContainerAddress(kernel_thread_runIdGet());
}

/**
 * @brief Push one thread context into exit list.
 *
 * @param pCurHead The pointer of the thread linker head.
 */
static void kernel_thread_list_transfer_toExit(linker_head_t *pCurHead)
{
    ENTER_CRITICAL_SECTION();

    list_t *pToExitList = (list_t *)kernel_member_list_get(KERNEL_MEMBER_THREAD, KERNEL_MEMBER_LIST_THREAD_EXIT);
    linker_list_transaction_common(&pCurHead->linker, pToExitList, LIST_TAIL);

    EXIT_CRITICAL_SECTION();
}

/**
 * @brief Push one thread context into entry list.
 *
 * @param pCurHead The pointer of the thread linker head.
 */
void kernel_thread_list_transfer_toEntry(linker_head_t *pCurHead)
{
    ENTER_CRITICAL_SECTION();

    list_t *pToEntryList = (list_t *)kernel_member_list_get(KERNEL_MEMBER_THREAD, KERNEL_MEMBER_LIST_THREAD_ENTRY);
    linker_list_transaction_common(&pCurHead->linker, pToEntryList, LIST_TAIL);

    EXIT_CRITICAL_SECTION();
}

/**
 * @brief Push one thread context into pending list.
 *
 * @param pCurHead The pointer of the thread linker head.
 */
void kernel_thread_list_transfer_toPend(linker_head_t *pCurHead)
{
    ENTER_CRITICAL_SECTION();

    list_t *pToPendList = (list_t *)kernel_list_pendingHeadGet();
    linker_list_transaction_specific(&pCurHead->linker, pToPendList, _kernel_thread_node_Order_compare_condition);

    EXIT_CRITICAL_SECTION();
}

/**
 * @brief The thread is trying to exit into suspend.
 *
 * @param id The thread unique id.
 * @param hold The member unique id hold on the thread.
 * @param pToList The blocking list.
 * @param timeout If the thread has sleep time setting.
 * @param pCallback The timeout callback function pointer.
 *
 * @return The result of exit operation.
 */
i32p_t kernel_thread_exit_trigger(os_id_t id, os_id_t hold, list_t *pToList, u32_t timeout_us, void (*pCallback)(os_id_t))
{
    thread_context_t *pCurThread = (thread_context_t *)(kernel_member_unified_id_toContainerAddress(id));

    pCurThread->schedule.hold = hold;
    pCurThread->schedule.exit.pToList = pToList;
    pCurThread->schedule.exit.timeout_us = timeout_us;
    pCurThread->schedule.exit.pTimeoutCallFun = pCallback;

    kernel_thread_list_transfer_toExit((linker_head_t *)&pCurThread->head);
    return kernel_thread_schedule_request();
}

/**
 * @brief Try to trigger one thread active.
 *
 * @param id The thread unique id.
 * @param release The thread release id.
 * @param result The thread entry result.
 * @param pCallback The timeout callback function pointer.
 *
 * @return The result of entry operation.
 */
i32p_t kernel_thread_entry_trigger(os_id_t id, os_id_t release, u32_t result, void (*pCallback)(os_id_t))
{
    thread_context_t *pCurThread = (thread_context_t *)(kernel_member_unified_id_toContainerAddress(id));

    pCurThread->schedule.entry.release = release;
    pCurThread->schedule.entry.result = result;
    pCurThread->schedule.entry.pEntryCallFun = pCallback;

    kernel_thread_list_transfer_toEntry((linker_head_t *)&pCurThread->head);
    return kernel_thread_schedule_request();
}

/**
 * @brief Read and clean the schedule entry result.
 *
 * @param pSchedule The pointer of thread action schedule.
 *
 * @return The result of entry action schedule.
 */
u32_t kernel_schedule_entry_result_take(action_schedule_t *pSchedule)
{
    if (!pSchedule) {
        return 0u;
    }

    u32_t ret = (i32p_t)pSchedule->entry.result;
    pSchedule->entry.result = 0u;

    return (u32_t)ret;
}

/**
 * @brief Check if kernel is in thread mode.
 *
 * @return The true indicates the kernel is in thread mode.
 */
b_t kernel_isInThreadMode(void)
{
    return port_isInThreadMode();
}

/**
 * @brief Request the kernel do thread schedule.
 *
 * @return The result of the request operation.
 */
i32p_t kernel_thread_schedule_request(void)
{
    if (!_kernel_isInPrivilegeMode()) {
        return _PCER;
    }

    _kernel_setPendSV();
    return 0;
}

/**
 * @brief To issue a kernel message notification.
 */
void kernel_message_notification(void)
{
    kthread_message_notification();
}

/**
 * @brief The kernel thread only serve for RTOS with highest priority.
 */
void kernel_schedule_thread(void)
{
    while (1) {
        PC_IF(_kernel_message_arrived(), PC_PASS)
        {
            timer_reamining_elapsed_handler();

            extern void _impl_publish_pending_handler(void);
            _impl_publish_pending_handler();
        }
    }
}

/**
 * @brief The idle thread entry function.
 */
void kernel_idle_thread(void)
{
    while (1) {
        /* TODO: Power Management */
    }
}

/**
 * @brief kernel call privilege function in SVC interrupt content.
 *
 * @param svc_args The function arguments.
 */
void kernel_privilege_call_inSVC_c(u32_t *svc_args)
{
    /*
     * Stack contains:
     * r0, r1, r2, r3, r12, r14, the return address and xPSR
     * First argument (r0) is svc_args[0]
     * Put the result into R0
     */
    u8_t svc_number = ((u8_t *)svc_args[6])[-2];

    if (svc_number == SVC_KERNEL_INVOKE_NUMBER) {
        pPrivilege_callFunc_t pCall = (pPrivilege_callFunc_t)svc_args[0];

        svc_args[0] = (u32_t)pCall((arguments_t *)svc_args[1]);
    }
}

/**
 * @brief kernel privilege function invoke interface.
 *
 * @param pCallFun The privilege function entry pointer.
 * @param pArgs The arguments list pool.
 *
 * @return The result of the privilege function.
 */
i32p_t kernel_privilege_invoke(const void *pCallFun, arguments_t *pArgs)
{
    if (!pCallFun) {
        return _PCER;
    }

    if (!_kernel_isInPrivilegeMode()) {
        return (i32p_t)kernel_svc_call((u32_t)pCallFun, (u32_t)pArgs, 0u, 0u);
    }

    ENTER_CRITICAL_SECTION();
    pPrivilege_callFunc_t pCall = (pPrivilege_callFunc_t)pCallFun;
    i32p_t ret = (i32p_t)pCall((arguments_t *)pArgs);
    EXIT_CRITICAL_SECTION();

    return ret;
}

/**
 * @brief To check if the kernel OS is running.
 *
 * return The true indicates the kernel OS is running.
 */
b_t _impl_kernel_rtos_isRun(void)
{
    return (b_t)((g_kernel_resource.run) ? (TRUE) : (FALSE));
}

/**
 * @brief The kernel OS start to run.
 */
i32p_t _impl_kernel_at_rtos_run(void)
{
    if (_impl_kernel_rtos_isRun()) {
        return 0;
    }

    return kernel_privilege_invoke((const void *)_kernel_start_privilege_routine, NULL);
}

#ifdef __cplusplus
}
#endif
