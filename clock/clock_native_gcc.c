/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 **/
#include "clock_tick.h"
#include "configuration.h"

/**
 * Data structure for location time clock
 */
typedef struct {
    /* The last load count value */
    u32_t last_load;

    /* The clock time total count value */
    u32_t total;

    /* The clock time has reported count value */
    u32_t reported;

    /* The hook function interface for clock time data pushing */
    time_report_handler_t pCallFunc;

    /* The flag indicates the clock ctrl register enabled status */
    b_t ctrl_enabled;
} _clock_resource_t;

/**
 * Local clock systick resource
 */
static _clock_resource_t g_clock_resource = {0u};

/**
 * @brief Detecting the wrap flag has to add the last load count into the clock total counter.
 *
 * @return True if the clock wrap, otherwise return false
 */
static b_t _clock_isWrap(void)
{
    /* Nothing need to do for kernel cmake sample build. */
    return FALSE;
}

/**
 * @brief Calculate the elapsed time.
 *
 * @return Value of the elapsed time.
 */
static u32_t _clock_elapsed(void)
{
    /* Nothing need to do for kernel cmake sample build. */
    return 0u;
}

/**
 * @brief Report the elasped time for rtos kernel timer using, through the hook interface.
 *
 * @param Value of the elapsed time.
 */
static void _clock_time_elapsed_report(u32_t us)
{
    if (g_clock_resource.pCallFunc) {
        g_clock_resource.pCallFunc(us);
    }
}

/**
 * @brief It's invoked in the SysTick_Handler to maintain the clock system.
 */
void clock_isr(void)
{
    /* Nothing need to do for kernel cmake sample build. */
}

/**
 * @brief The interface for kernel rtos set the next timeout.
 *
 * @param Value of the next timeout.
 */
void clock_time_interval_set(u32_t interval_us)
{
    /* Nothing need to do for kernel cmake sample build. */
}

/**
 * @brief Get the clock time unreported elapse.
 *
 * @return Value of the unreported elapse time.
 */
u32_t clock_time_elapsed_get(void)
{
    /* Nothing need to do for kernel cmake sample build. */
    return 0u;
}

/**
 * @brief Get the current clock time.
 *
 * @return Value of the current clock time.
 */
u32_t clock_time_get(void)
{
    /* Nothing need to do for kernel cmake sample build. */
    return 0u;
}

/**
 * @brief Enable the time clock.
 */
void clock_time_enable(void)
{
    /* Nothing need to do for kernel cmake sample build. */
}

/**
 * @brief Disable the time clock.
 */
void clock_time_disable(void)
{
    /* Nothing need to do for kernel cmake sample build. */
}

/**
 * @brief Init the time clock.
 */
void clock_time_init(time_report_handler_t pTime_function)
{
    g_clock_resource.pCallFunc = pTime_function;

    /* Nothing need to do for kernel cmake sample build. */
}
