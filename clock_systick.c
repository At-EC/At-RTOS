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

#define _CONVERT_MICROSENCOND_TO_COUNT(us) 		      (((u64_t)(us) * (PORTAL_SYSTEM_CORE_CLOCK_HZ / 1000000u)) - 1u)
#define _CONVERT_COUNT_TO_MICROSENCOND(count) 	      (((u64_t)(count) + 1u) / (PORTAL_SYSTEM_CORE_CLOCK_HZ / 1000000u))

enum
{
    _CLOCK_INTERVAL_MAX_US = (_CONVERT_COUNT_TO_MICROSENCOND(SysTick_LOAD_RELOAD_Msk)),
    _CLOCK_INTERVAL_MIN_US = (PORTAL_SYSTEM_CLOCK_INTERVAL_MIN_US),
};

static volatile u32_t _s_clock_last_load_count = 0u;
static volatile u32_t _s_clock_total_count = 0u;
static volatile u32_t _s_clock_total_reported_count = 0u;
static volatile u32_t _s_clock_wrap_count = 0u;
static time_report_handler_t _s_pClock_time_report_func = NULL;

b_t _clock_isWrap(void)
{
    if (SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk)
    {
        _s_clock_total_count += _s_clock_last_load_count;
        return TRUE;
    }
    return FALSE;
}

static u32_t _clock_elapsed(void)
{
    u32_t expired = 0u;
    b_t previous, next = _clock_isWrap();

    do {
        previous = next;
        expired = _s_clock_last_load_count - SysTick->VAL;
        next = _clock_isWrap();
    } while (previous || next);

    return expired;
}

static void _clock_time_elapsed_report(u32_t us)
{
    if (_s_pClock_time_report_func)
    {
        _s_pClock_time_report_func(us);
    }
}

void _impl_clock_isr(void)
{
    /* maintain purpose */
    u32_t total_count = _clock_elapsed();
    total_count += _s_clock_total_count;

    u32_t elapsed_interval_us = _CONVERT_COUNT_TO_MICROSENCOND(total_count - _s_clock_total_reported_count);
    _s_clock_total_reported_count += _CONVERT_MICROSENCOND_TO_COUNT(elapsed_interval_us);

    _clock_time_elapsed_report(elapsed_interval_us);
}

void _impl_clock_time_interval_set(u32_t interval_us)
{
    if (interval_us == 0xFFFFFFFFu)
    {
        SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
        return;
    }

    ENTER_CRITICAL_SECTION();

    if (interval_us > _CLOCK_INTERVAL_MAX_US)
    {
        interval_us = _CLOCK_INTERVAL_MAX_US;
    }
    u32_t set_count = _CONVERT_MICROSENCOND_TO_COUNT(interval_us);

    u32_t elapsed = _clock_elapsed();

    _s_clock_total_count += elapsed;

    u32_t before = SysTick->VAL;

    u32_t unreported = _s_clock_total_count - _s_clock_total_reported_count;

    if ((i32_t)unreported < 0)
    {
        _s_clock_last_load_count = _CONVERT_MICROSENCOND_TO_COUNT(100u);
    }
    else
    {
        set_count -= unreported;
        //interval_us = _CONVERT_COUNT_TO_MICROSENCOND(set_count);
        //set_count = _CONVERT_MICROSENCOND_TO_COUNT(interval_us) - unreported;
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

        _s_clock_last_load_count = _CONVERT_MICROSENCOND_TO_COUNT(interval_us);
    }

    u32_t after = SysTick->VAL;

	SysTick->LOAD = _s_clock_last_load_count;
	SysTick->VAL = 0;

	if (before < after)
	{
	    (void)SysTick->CTRL;
        _s_clock_total_count += (before + (_s_clock_last_load_count - after));
	}
	else
	{
        _s_clock_total_count += (before - after);
	}

    EXIT_CRITICAL_SECTION();
}

u32_t _impl_clock_time_elapsed_get(void)
{
    ENTER_CRITICAL_SECTION();

    u32_t us = _CONVERT_COUNT_TO_MICROSENCOND(_clock_elapsed() + _s_clock_total_count - _s_clock_total_reported_count);

    EXIT_CRITICAL_SECTION();

    return us;
}

u32_t _impl_clock_time_get(void)
{
    ENTER_CRITICAL_SECTION();

    u32_t us = _CONVERT_COUNT_TO_MICROSENCOND(_s_clock_total_count + _clock_elapsed());

    EXIT_CRITICAL_SECTION();

    return us;
}

void _impl_clock_time_enable(void)
{
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
}

void _impl_clock_time_disable(void)
{
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}

void _impl_clock_time_init(time_report_handler_t pTime_function)
{
    _s_pClock_time_report_func = pTime_function;

    NVIC_SetPriority(SysTick_IRQn, 0xFFu);
    _s_clock_last_load_count = SysTick_LOAD_RELOAD_Msk;

    SysTick->CTRL = 0x0u;
    SysTick->LOAD = _s_clock_last_load_count;
    SysTick->VAL  = 0x0u;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                    SysTick_CTRL_TICKINT_Msk   |
                    SysTick_CTRL_ENABLE_Msk;
}

#ifdef __cplusplus
}
#endif
