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
