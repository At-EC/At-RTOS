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
 * @brief Take firmare snapshot information.
 */
void _impl_trace_firmware_snapshot_take(void)
{
    printf(">> At-RTOS Version: v%d.%d.%d snaphost!!!\n", ATOS_VERSION_MAJOR_NUMBER, ATOS_VERSION_MINOR_NUMBER, ATOS_VERSION_PATCH_NUMBER);
    printf(">> %-16s %-40s\n", "Build Time", "Commit ID");
    printf("   %-16s %-40s\n", ATOS_BUILD_TIME, ATOS_COMMIT_HEAD_ID);
}

/**
 * @brief Take postcode snapshot information.
 */
void _impl_trace_postcode_snapshot_take(void)
{
    extern vu32_t g_postcode_cmpt_failed_container[POSTCODE_COMPONENT_NUMBER];

    b_t is_failed = FALSE;

    for (u32_t i = 0u; i < POSTCODE_COMPONENT_NUMBER; i++) {
        if (g_postcode_cmpt_failed_container[i]) {
            is_failed = TRUE;
            printf(">> Postcode CMPT %d is failed at %d\n", i, (g_postcode_cmpt_failed_container[i] & PC_LINE_NUMBER_MASK));
        }
    }

    if (!is_failed) {
        printf(">> Postcode CMPT succeeded\n");
    }
}

/**
 * @brief Take kernal snapshot information.
 */
void _impl_trace_kernal_snapshot_take(void)
{
    u32_t unused[7] = {0u};
    kernal_snapshot_t snapshot_data;

    _impl_trace_firmware_snapshot_take();
    _impl_trace_postcode_snapshot_take();

    printf(">> %-6s %-15s %-5s %-7s %-3s %-10s %-6s\n", "Thread", "Name", "ID", "STATE", "PRI", "PSP_ADDR", "USE(%)");
    for (u32_t i = 0u; i < THREAD_INSTANCE_SUPPORTED_NUMBER; i++) {
        if (_impl_trace_thread_snapshot(i, &snapshot_data)) {
            printf("   %-6d %-15s %-5d %-7s %-3d 0x%-8x %-3d\n", (i + 1u), snapshot_data.pName, snapshot_data.id, snapshot_data.pState,
                   snapshot_data.thread.priority, snapshot_data.thread.current_psp, snapshot_data.thread.usage);
        } else {
            unused[0]++;
        }
    }

    printf(">> %-6s %-15s %-5s %-7s %-5s %-5s %-1s %-11s %-5s\n", "Sem", "Name", "ID", "STATE", "Init", "Limit", "P", "Timeout(ms)",
           "Block(ID)");
    for (u32_t i = 0u; i < SEMAPHORE_INSTANCE_SUPPORTED_NUMBER; i++) {
        if (_impl_trace_semaphore_snapshot(i, &snapshot_data)) {
            printf("   %-6d %-15s %-5d %-7s %-5d %-5d %-1d %-11d", (i + 1u), snapshot_data.pName, snapshot_data.id, snapshot_data.pState,
                   snapshot_data.semaphore.initial_count, snapshot_data.semaphore.limit_count, snapshot_data.semaphore.permit,
                   snapshot_data.semaphore.timeout_ms);

            list_t *pList = (list_t *)&snapshot_data.semaphore.wait_list;
            while (pList->pHead) {
                linker_head_t *pHead = (linker_head_t *)pList->pHead;
                printf(" %-5d", pHead->id);

                pList->pHead = pList->pHead->pNext;
            }
            printf("\n");

        } else {
            unused[1]++;
        }
    }

    printf(">> %-6s %-15s %-5s %-7s %-5s %-3s %-5s\n", "Mutex", "Name", "ID", "STATE", "Hold", "Ori", "Block(ID)");
    for (u32_t i = 0u; i < MUTEX_INSTANCE_SUPPORTED_NUMBER; i++) {
        if (_impl_trace_mutex_snapshot(i, &snapshot_data)) {
            printf("   %-6d %-15s %-5d %-7s %-5d %-3d", (i + 1u), snapshot_data.pName, snapshot_data.id, snapshot_data.pState,
                   snapshot_data.mutex.holdThreadId, snapshot_data.mutex.originalPriority);

            list_t *pList = (list_t *)&snapshot_data.mutex.wait_list;
            while (pList->pHead) {
                linker_head_t *pHead = (linker_head_t *)pList->pHead;
                printf(" %-5d", pHead->id);

                pList->pHead = pList->pHead->pNext;
            }
            printf("\n");

        } else {
            unused[2]++;
        }
    }

    printf(">> %-6s %-15s %-5s %-7s %-10s %-10s %-5s\n", "Event", "Name", "ID", "State", "Set", "Edge", "Block(ID)");
    for (u32_t i = 0u; i < EVENT_INSTANCE_SUPPORTED_NUMBER; i++) {
        if (_impl_trace_event_snapshot(i, &snapshot_data)) {
            printf("   %-6d %-15s %-5d %-7s 0x%-8x 0x%-8x", (i + 1u), snapshot_data.pName, snapshot_data.id, snapshot_data.pState,
                   snapshot_data.event.set, snapshot_data.event.edge);

            list_t *pList = (list_t *)&snapshot_data.event.wait_list;
            while (pList->pHead) {
                linker_head_t *pHead = (linker_head_t *)pList->pHead;
                printf(" %-5d", pHead->id);

                pList->pHead = pList->pHead->pNext;
            }
            printf("\n");

        } else {
            unused[3]++;
        }
    }

    printf(">> %-6s %-15s %-5s %-7s %-5s %-20s\n", "Queue", "Name", "ID", "State", "Has", "Block(ID)");
    for (u32_t i = 0u; i < QUEUE_INSTANCE_SUPPORTED_NUMBER; i++) {
        if (_impl_trace_queue_snapshot(i, &snapshot_data)) {
            printf("   %-6d %-15s %-5d %-7s %-5d", (i + 1u), snapshot_data.pName, snapshot_data.id, snapshot_data.pState,
                   snapshot_data.queue.cacheSize);

            list_t *pList = (list_t *)&snapshot_data.queue.in_list;
            while (pList->pHead) {
                linker_head_t *pHead = (linker_head_t *)pList->pHead;
                printf(" i%-5d", pHead->id);

                pList->pHead = pList->pHead->pNext;
            }

            pList = (list_t *)&snapshot_data.queue.out_list;
            while (pList->pHead) {
                linker_head_t *pHead = (linker_head_t *)pList->pHead;
                printf(" o%-5d", pHead->id);

                pList->pHead = pList->pHead->pNext;
            }
            printf("\n");

        } else {
            unused[4]++;
        }
    }

    printf(">> %-6s %-15s %-5s %-7s %-1s %-11s\n", "Timer", "Name", "ID", "State", "C", "Timeout(ms)");
    for (u32_t i = 0u; i < TIMER_INSTANCE_SUPPORTED_NUMBER; i++) {
        if (_impl_trace_timer_snapshot(i, &snapshot_data)) {
            printf("   %-6d %-15s %-5d %-7s %-1d %-11d\n", (i + 1u), snapshot_data.pName, snapshot_data.id, snapshot_data.pState,
                   snapshot_data.timer.is_cycle, snapshot_data.timer.timeout_ms);
        } else {
            unused[5]++;
        }
    }
    printf(">> %-9s %-6s %-6s\n", "Statistic", "Totals", "Remain");
    printf("   %-9s %-6d %-6d\n", "Thread", THREAD_INSTANCE_SUPPORTED_NUMBER, unused[0]);
    printf("   %-9s %-6d %-6d\n", "Semaphore", SEMAPHORE_INSTANCE_SUPPORTED_NUMBER, unused[1]);
    printf("   %-9s %-6d %-6d\n", "Mutex", MUTEX_INSTANCE_SUPPORTED_NUMBER, unused[2]);
    printf("   %-9s %-6d %-6d\n", "Event", EVENT_INSTANCE_SUPPORTED_NUMBER, unused[3]);
    printf("   %-9s %-6d %-6d\n", "Queue", QUEUE_INSTANCE_SUPPORTED_NUMBER, unused[4]);
    printf("   %-9s %-6d %-6d\n", "Timer", TIMER_INSTANCE_SUPPORTED_NUMBER, unused[5]);
}

#ifdef __cplusplus
}
#endif
