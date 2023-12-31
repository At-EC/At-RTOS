/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "kernal.h"
#include "clock_systick.h"
#include "os_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Convert the microsecond to clock count */
#define _CONVERT_MICROSENCOND_TO_COUNT(us)      (((u64_t)(us) * (PORTAL_SYSTEM_CORE_CLOCK_HZ / 1000000u)) - 1u)

/* Convert the clock count to microsecond */
#define _CONVERT_COUNT_TO_MICROSENCOND(count)   (((u64_t)(count) + 1u) / (PORTAL_SYSTEM_CORE_CLOCK_HZ / 1000000u))

enum
{
    /* The maximum timeout setting value */
    _CLOCK_INTERVAL_MAX_US = (_CONVERT_COUNT_TO_MICROSENCOND(SysTick_LOAD_RELOAD_Msk)),

    /* The minimum timeout setting value is in order to avoid the clock dead looping call */
    _CLOCK_INTERVAL_MIN_US = (PORTAL_SYSTEM_CLOCK_INTERVAL_MIN_US),
};

/**
 * Data structure for location time clock
 */
typedef struct
{
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
}_clock_resource_t;

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
    if (SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk)
    {
        g_clock_resource.total += g_clock_resource.last_load;
        return TRUE;
    }
    return FALSE;
}

/**
 * @brief Calculate the elapsed time.
 *
 * @return Value of the elapsed time.
 */
static u32_t _clock_elapsed(void)
{
    u32_t expired = 0u;

    /**
     * The elasped time has to calculate when no wrap occur.
     */
    b_t previous, next = _clock_isWrap();
    do
    {
        previous = next;
        expired = g_clock_resource.last_load - SysTick->VAL;
        next = _clock_isWrap();
    } while (previous || next);

    return expired;
}

/**
 * @brief Report the elasped time for rtos kernal timer using, through the hook interface.
 *
 * @param Value of the elapsed time.
 */
static void _clock_time_elapsed_report(u32_t us)
{
    if (g_clock_resource.pCallFunc)
    {
        g_clock_resource.pCallFunc(us);
    }
}

/**
 * @brief It's invoked in the SysTick_Handler to maintain the clock system.
 */
void _impl_clock_isr(void)
{
    /**
     * For maintain purpose.
     */
    u32_t total_count = _clock_elapsed();
    total_count += g_clock_resource.total;

    /**
     * Avoid the count lost.
     */
    u32_t elapsed_interval_us = _CONVERT_COUNT_TO_MICROSENCOND(total_count - g_clock_resource.reported);
    g_clock_resource.reported += _CONVERT_MICROSENCOND_TO_COUNT(elapsed_interval_us);

    _clock_time_elapsed_report(elapsed_interval_us);
}

/**
 * @brief The interface for kernal rtos set the next timeout.
 *
 * @param Value of the next timeout.
 */
void _impl_clock_time_interval_set(u32_t interval_us)
{
    if (interval_us == WAIT_FOREVER)
    {
        SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
         g_clock_resource.ctrl_enabled = FALSE;
        return;
    }
    else if (!g_clock_resource.ctrl_enabled)
    {
        SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
    }

    ENTER_CRITICAL_SECTION();

    if (interval_us > _CLOCK_INTERVAL_MAX_US)
    {
        interval_us = _CLOCK_INTERVAL_MAX_US;
    }
    u32_t set_count = _CONVERT_MICROSENCOND_TO_COUNT(interval_us);

    u32_t elapsed = _clock_elapsed();

    g_clock_resource.total += elapsed;

    /**
     * The redundance elasped times occur.
     * The elapsed time lost after seting the load count.
     * The following code helps to reduce the redundance time,
     * but it can't be fixed entirily
     */
    u32_t before = SysTick->VAL;

    u32_t unreported = g_clock_resource.total - g_clock_resource.reported;

    if ((i32_t)unreported < 0)
    {
        g_clock_resource.last_load = _CONVERT_MICROSENCOND_TO_COUNT(100u);
    }
    else
    {
        set_count -= unreported;
        interval_us = _CONVERT_COUNT_TO_MICROSENCOND(set_count);
        set_count = _CONVERT_MICROSENCOND_TO_COUNT(interval_us) - unreported;
        interval_us = _CONVERT_COUNT_TO_MICROSENCOND(set_count);

        if (interval_us > _CLOCK_INTERVAL_MAX_US)
        {
            interval_us = _CLOCK_INTERVAL_MAX_US;
        }
        else if (interval_us < _CLOCK_INTERVAL_MIN_US)
        {
            interval_us = _CLOCK_INTERVAL_MIN_US;
        }
        else
        {
            set_count -= unreported;
            interval_us = _CONVERT_COUNT_TO_MICROSENCOND(set_count);
        }

        g_clock_resource.last_load = _CONVERT_MICROSENCOND_TO_COUNT(interval_us);
    }

    u32_t after = SysTick->VAL;

	SysTick->LOAD = g_clock_resource.last_load;
	SysTick->VAL = 0;

	if (before < after)
	{
	    (void)SysTick->CTRL;
        g_clock_resource.total += (before + (g_clock_resource.last_load - after));
	}
	else
	{
        g_clock_resource.total += (before - after);
	}

    EXIT_CRITICAL_SECTION();
}

/**
 * @brief Get the clock time unreported elapse.
 *
 * @return Value of the unreported elapse time.
 */
u32_t _impl_clock_time_elapsed_get(void)
{
    ENTER_CRITICAL_SECTION();

    u32_t us = _CONVERT_COUNT_TO_MICROSENCOND(_clock_elapsed() + g_clock_resource.total - g_clock_resource.reported);

    EXIT_CRITICAL_SECTION();

    return us;
}

/**
 * @brief Get the current clock time.
 *
 * @return Value of the current clock time.
 */
u32_t _impl_clock_time_get(void)
{
    ENTER_CRITICAL_SECTION();

    u32_t us = _CONVERT_COUNT_TO_MICROSENCOND(g_clock_resource.total + _clock_elapsed());

    EXIT_CRITICAL_SECTION();

    return us;
}

/**
 * @brief Enable the time clock.
 */
void _impl_clock_time_enable(void)
{
    if (!g_clock_resource.ctrl_enabled)
    {
        g_clock_resource.ctrl_enabled = TRUE;
        SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
    }
}

/**
 * @brief Disable the time clock.
 */
void _impl_clock_time_disable(void)
{
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}

/**
 * @brief Init the time clock.
 */
void _impl_clock_time_init(time_report_handler_t pTime_function)
{
    g_clock_resource.pCallFunc = pTime_function;

    NVIC_SetPriority(SysTick_IRQn, 0xFFu);
    g_clock_resource.last_load = SysTick_LOAD_RELOAD_Msk;
    g_clock_resource.ctrl_enabled = TRUE;

    SysTick->CTRL = 0x0u;
    SysTick->LOAD = g_clock_resource.last_load;
    SysTick->VAL  = 0x0u;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                    SysTick_CTRL_TICKINT_Msk   |
                    SysTick_CTRL_ENABLE_Msk;
}

#ifdef __cplusplus
}
#endif
