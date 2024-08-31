/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 **/

#ifndef _POSTCODE_H_
#define _POSTCODE_H_

#include "typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

/* The following table defined the At-RTOS component number */
enum {
    PC_OS_CMPT_KERNEL_2 = PC_CMPT_NUMBER_USER,
    PC_OS_CMPT_THREAD_3,
    PC_OS_CMPT_SEMAPHORE_4,
    PC_OS_CMPT_MUTEX_5,
    PC_OS_CMPT_QUEUE_6,
    PC_OS_CMPT_EVENT_7,
    PC_OS_CMPT_TIMER_8,
    PC_OS_CMPT_POOL_9,
    PC_OS_CMPT_PUBLISH_10,

    PC_OS_COMPONENT_NUMBER,
};

/**
 * @brief To trace the last failed postcode.
 *
 * @param postcode The postcode number.
 *
 * @return Return the original argument postcode.
 */
static inline i32p_t _impl_postcode_cmpt_failed(i32p_t postcode)
{
    extern u32_t g_postcode_os_cmpt_failed_container[PC_OS_COMPONENT_NUMBER];

    if (postcode < 0) {
        u32_t code = PC_NEGATIVE(postcode);
        u32_t component = (((u32_t)code >> PC_COMPONENT_NUMBER_POS) & PC_COMPONENT_NUMBER_MSK);
        if (component < PC_OS_COMPONENT_NUMBER) {
            g_postcode_os_cmpt_failed_container[component] = code;
        }

        while (1) {};
    }
    return postcode;
}

#define PC_ERROR     <
#define PC_PASS      ==
#define PC_PASS_INFO >=

#define PCST(code)      _impl_postcode_cmpt_failed(code)
#define PC_IF(expr, op) if (PCST(expr) op 0)

#define _CHECK_COND(c, n)                                                                                                                  \
    if (UNFLAG(c)) {                                                                                                                       \
        PCST(PC_NER(PC_CMPT_ASSERT, n));                                                                                                   \
    }
#define _CHECK_CONDITION(cond)          _CHECK_COND((i32_t)cond, 1)
#define _CHECK_POINTER(pointer)         _CHECK_COND((i32_t)pointer, 2)
#define _CHECK_INDEX(index, high)       _CHECK_COND(((i32_t)index >= 0) && ((i32_t)index < high), 3)
#define _CHECK_BOUND(val, high)         _CHECK_COND(((i32_t)val >= 0) && ((i32_t)val <= (i32_t)high), 4)
#define _CHECK_RANGE(val, low, high)    _CHECK_COND(((i32_t)val >= (i32_t)low) && ((i32_t)val <= (i32_t)high), 5)
#define PC_ASSERT(cond)                 _CHECK_CONDITION(cond)
#define PC_ASSERT_EQ(a, b)              _CHECK_CONDITION((a) == (b))
#define PC_ASSERT_POINTER(pointer)      _CHECK_POINTER(pointer)
#define PC_ASSERT_INDEX(index, high)    _CHECK_INDEX(index, high)
#define PC_ASSERT_RANGE(val, low, high) _CHECK_RANGE(val, low, high)
#define PC_ASSERT_BOUND(val, high)      _CHECK_BOUND(val, high)

#ifdef __cplusplus
}
#endif

#endif /* _POSTCODE_H_ */
