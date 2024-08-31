/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 **/

#include "trace.h"
#include "configuration.h"
#include "postcode.h"
#include "linker.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Local trace postcode contrainer
 */
u32_t g_postcode_os_cmpt_failed_container[PC_OS_COMPONENT_NUMBER] = {0u};

/**
 * @brief Take firmare snapshot information.
 */
void _impl_trace_firmware_snapshot_print(void)
{
#if defined KTRACE
    KTRACE(">> At-RTOS Version: v%d.%d.%d snaphost!!!\n", ATOS_VERSION_MAJOR_NUMBER, ATOS_VERSION_MINOR_NUMBER, ATOS_VERSION_PATCH_NUMBER);
    KTRACE(">> %-16s %-40s\n", "Build Time", "Commit ID");
    KTRACE("   %-16s %-40s\n", ATOS_BUILD_TIME, ATOS_COMMIT_HEAD_ID);
#endif
}

/**
 * @brief Take postcode snapshot information.
 */
void _impl_trace_postcode_snapshot_print(void)
{
    b_t is_failed = FALSE;

    for (u32_t i = 0u; i < PC_OS_COMPONENT_NUMBER; i++) {
        if (g_postcode_os_cmpt_failed_container[i]) {
            is_failed = TRUE;
#if defined KTRACE
            KTRACE(">> Postcode CMPT %d indicates it is failed at %d\n", i,
                   (((g_postcode_os_cmpt_failed_container[i] >> PC_LINE_NUMBER_POS) & PC_LINE_NUMBER_MSK)));
#endif
        }
    }

    if (!is_failed) {
#if defined KTRACE
        KTRACE(">> Postcode CMPT succeeded\n");
#endif
    }
}

/**
 * @brief Take kernel snapshot information.
 */
void _impl_trace_kernel_snapshot_print(void)
{
#if defined KTRACE
    u32_t unused[8] = {0u};
    kernel_snapshot_t snapshot_data;

    _impl_trace_firmware_snapshot_print();
    _impl_trace_postcode_snapshot_print();

    KTRACE(">> %-6s %-15s %-5s %-7s %-3s %-10s %-7s %-9s %-10s\n", "Thread", "Name", "ID", "STATE", "PRI", "PSP_ADDR", "RAM(1%)",
           "CPU(0.1%)", "W/P/R(ms)");
    for (u32_t i = 0u; i < THREAD_INSTANCE_SUPPORTED_NUMBER; i++) {
        if (thread_snapshot(i, &snapshot_data)) {
            KTRACE("   %-6d %-15s %-5d %-7s %-3d 0x%-8x %-7d %-9d %-10d\n", (i + 1u), snapshot_data.pName, snapshot_data.id,
                   snapshot_data.pState, snapshot_data.thread.priority, snapshot_data.thread.current_psp, snapshot_data.thread.ram,
                   snapshot_data.thread.cpu, snapshot_data.thread.delay);
        } else {
            unused[0]++;
        }
    }

    KTRACE(">> %-6s %-15s %-5s %-7s %-5s %-5s %-11s %-5s\n", "Sem", "Name", "ID", "STATE", "Init", "Limit", "Timeout(ms)", "Block(ID)");
    for (u32_t i = 0u; i < SEMAPHORE_INSTANCE_SUPPORTED_NUMBER; i++) {
        if (semaphore_snapshot(i, &snapshot_data)) {
            KTRACE("   %-6d %-15s %-5d %-7s %-5d %-5d %-11d", (i + 1u), snapshot_data.pName, snapshot_data.id, snapshot_data.pState,
                   snapshot_data.semaphore.initial_count, snapshot_data.semaphore.limit_count, snapshot_data.semaphore.timeout_ms);

            list_t *pList = (list_t *)&snapshot_data.semaphore.wait_list;
            while (pList->pHead) {
                KTRACE(" %-5d", ((linker_head_t *)pList->pHead)->id);

                pList->pHead = pList->pHead->pNext;
            }
            KTRACE("\n");

        } else {
            unused[1]++;
        }
    }

    KTRACE(">> %-6s %-15s %-5s %-7s %-5s %-3s %-5s\n", "Mutex", "Name", "ID", "STATE", "Hold", "Ori", "Block(ID)");
    for (u32_t i = 0u; i < MUTEX_INSTANCE_SUPPORTED_NUMBER; i++) {
        if (mutex_snapshot(i, &snapshot_data)) {
            KTRACE("   %-6d %-15s %-5d %-7s %-5d %-3d", (i + 1u), snapshot_data.pName, snapshot_data.id, snapshot_data.pState,
                   snapshot_data.mutex.holdThreadId, snapshot_data.mutex.originalPriority);

            list_t *pList = (list_t *)&snapshot_data.mutex.wait_list;
            while (pList->pHead) {
                KTRACE(" %-5d", ((linker_head_t *)pList->pHead)->id);

                pList->pHead = pList->pHead->pNext;
            }
            KTRACE("\n");

        } else {
            unused[2]++;
        }
    }

    KTRACE(">> %-6s %-15s %-5s %-7s %-10s %-10s %-5s\n", "Event", "Name", "ID", "State", "Set", "Edge", "Block(ID)");
    for (u32_t i = 0u; i < EVENT_INSTANCE_SUPPORTED_NUMBER; i++) {
        if (event_snapshot(i, &snapshot_data)) {
            KTRACE("   %-6d %-15s %-5d %-7s 0x%-8x 0x%-8x", (i + 1u), snapshot_data.pName, snapshot_data.id, snapshot_data.pState,
                   snapshot_data.event.set, snapshot_data.event.edge);

            list_t *pList = (list_t *)&snapshot_data.event.wait_list;
            while (pList->pHead) {
                KTRACE(" %-5d", ((linker_head_t *)pList->pHead)->id);

                pList->pHead = pList->pHead->pNext;
            }
            KTRACE("\n");

        } else {
            unused[3]++;
        }
    }

    KTRACE(">> %-6s %-15s %-5s %-7s %-5s %-20s\n", "Queue", "Name", "ID", "State", "Has", "Block(ID)");
    for (u32_t i = 0u; i < QUEUE_INSTANCE_SUPPORTED_NUMBER; i++) {
        if (queue_snapshot(i, &snapshot_data)) {
            KTRACE("   %-6d %-15s %-5d %-7s %-5d", (i + 1u), snapshot_data.pName, snapshot_data.id, snapshot_data.pState,
                   snapshot_data.queue.cacheSize);

            list_t *pList = (list_t *)&snapshot_data.queue.in_list;
            while (pList->pHead) {
                KTRACE(" i%-5d", ((linker_head_t *)pList->pHead)->id);

                pList->pHead = pList->pHead->pNext;
            }

            pList = (list_t *)&snapshot_data.queue.out_list;
            while (pList->pHead) {
                KTRACE(" o%-5d", ((linker_head_t *)pList->pHead)->id);

                pList->pHead = pList->pHead->pNext;
            }
            KTRACE("\n");

        } else {
            unused[4]++;
        }
    }

    KTRACE(">> %-6s %-15s %-5s %-7s %-1s %-11s\n", "Timer", "Name", "ID", "State", "C", "Timeout(ms)");
    for (u32_t i = 0u; i < TIMER_INSTANCE_SUPPORTED_NUMBER; i++) {
        if (timer_snapshot(i, &snapshot_data)) {
            KTRACE("   %-6d %-15s %-5d %-7s %-1d %-11d\n", (i + 1u), snapshot_data.pName, snapshot_data.id, snapshot_data.pState,
                   snapshot_data.timer.is_cycle, snapshot_data.timer.timeout_ms);
        } else {
            unused[5]++;
        }
    }

    KTRACE(">> %-6s %-15s %-5s %-7s %-7s\n", "Publish", "Name", "ID", "State", "refresh");
    for (u32_t i = 0u; i < PUBLISH_INSTANCE_SUPPORTED_NUMBER; i++) {
        if (publish_snapshot(i, &snapshot_data)) {
            KTRACE("   %-6d %-15s %-5d %-7s %-7d\n", (i + 1u), snapshot_data.pName, snapshot_data.id, snapshot_data.pState,
                   snapshot_data.publish.refresh_count);
        } else {
            unused[6]++;
        }
    }

    KTRACE(">> %-6s %-15s %-5s %-7s %-13s %-5s\n", "Pool", "Name", "ID", "State", "FreeBits", "Block(ID)");
    for (u32_t i = 0u; i < POOL_INSTANCE_SUPPORTED_NUMBER; i++) {
        if (pool_snapshot(i, &snapshot_data)) {
            KTRACE("   %-6d %-15s %-5d %-7s 0x%-11x", (i + 1u), snapshot_data.pName, snapshot_data.id, snapshot_data.pState,
                   snapshot_data.pool.free);

            list_t *pList = (list_t *)&snapshot_data.pool.wait_list;
            while (pList->pHead) {
                KTRACE(" i%-5d", ((linker_head_t *)pList->pHead)->id);

                pList->pHead = pList->pHead->pNext;
            }
            KTRACE("\n");
        } else {
            unused[7]++;
        }
    }

    KTRACE(">> %-9s %-6s %-6s\n", "Statistic", "Totals", "Remain");
    KTRACE("   %-9s %-6d %-6d\n", "Thread", THREAD_INSTANCE_SUPPORTED_NUMBER, unused[0]);
    KTRACE("   %-9s %-6d %-6d\n", "Semaphore", SEMAPHORE_INSTANCE_SUPPORTED_NUMBER, unused[1]);
    KTRACE("   %-9s %-6d %-6d\n", "Mutex", MUTEX_INSTANCE_SUPPORTED_NUMBER, unused[2]);
    KTRACE("   %-9s %-6d %-6d\n", "Event", EVENT_INSTANCE_SUPPORTED_NUMBER, unused[3]);
    KTRACE("   %-9s %-6d %-6d\n", "Queue", QUEUE_INSTANCE_SUPPORTED_NUMBER, unused[4]);
    KTRACE("   %-9s %-6d %-6d\n", "Timer", TIMER_INSTANCE_SUPPORTED_NUMBER, unused[5]);
    KTRACE("   %-9s %-6d %-6d\n", "Timer", PUBLISH_INSTANCE_SUPPORTED_NUMBER, unused[6]);
    KTRACE("   %-9s %-6d %-6d\n", "Pool", POOL_INSTANCE_SUPPORTED_NUMBER, unused[7]);
#endif
}

#ifdef __cplusplus
}
#endif
