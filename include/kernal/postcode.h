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
/* To generating a unique token */
#define _STATIC_MAGIC_3(pre, post) MAGIC(pre, post)
#define _STATIC_MAGIC_2(pre, post) _STATIC_MAGIC_3(pre, post)

#if defined __COUNTER__
#define _STATIC_MAGIC(pre) _STATIC_MAGIC_2(pre, COMPLIER_UNIQUE_NUMBER)
#else
#define _STATIC_MAGIC(pre) _STATIC_MAGIC_2(pre, COMPLIER_UNIQUE_NUMBER)
#endif
#define S_ASSERT(cond, msg) typedef char_t _STATIC_MAGIC(static_assert)[FLAG(cond) * 2 - 1]

#define COMPLIER_LINE_NUMBER __LINE__

#define PC_LINE_NUMBER_MASK (0x3FFFu) /* The maximum line number up to 16383 */
#define PC_LINE_NUMBER_POS  (0u)

#define PC_COMPONENT_NUMBER_MASK (0x1FFFu) /* The maximum component number up to 8,191 */
#define PC_COMPONENT_NUMBER_POS  (14u)

#define PC_PASS_NUMBER_MASK (0x1Fu) /* The maximum pass information number up to 31 */
#define PC_PASS_NUMBER_POS  (27u)

#define PC_SUCCESS(OK_INFO) (((OK_INFO) & PC_PASS_NUMBER_MASK) << PC_PASS_NUMBER_POS)

#define PC_FAILED_IO(COMPONENT, ERR_INFO)                                                                                                  \
    (((((COMPONENT) & PC_COMPONENT_NUMBER_MASK) << PC_COMPONENT_NUMBER_POS)) | ((ERR_INFO) & PC_LINE_NUMBER_MASK))

#define PC_FAILED(COMPONENT) PC_FAILED_IO(COMPONENT, COMPLIER_LINE_NUMBER)

/* The following table defined the At-RTOS component number */
enum {
    PC_CMPT_KERNAL_1 = 1u,
    PC_CMPT_THREAD_2,
    PC_CMPT_SEMAPHORE_3,
    PC_CMPT_MUTEX_4,
    PC_CMPT_QUEUE_5,
    PC_CMPT_EVENT_6,
    PC_CMPT_TIMER_7,

    PC_CMPT_ASSERT_8,

    POSTCODE_COMPONENT_NUMBER,
};

/* This declaration is help to allow users define the customization component number in their application system */
#define POSTCODE_COMPONENT_USER_NUMBER (POSTCODE_COMPONENT_NUMBER)

/* The following table defined the At-RTOS pass information number */
enum {
    PC_SC_SUCCESS = PC_SUCCESS(0u),
    PC_SC_TIMEOUT = PC_SUCCESS(1u),
    PC_SC_NO_MEMERY = PC_SUCCESS(2u),
    PC_SC_AVAILABLE = PC_SUCCESS(3u),
    PC_SC_UNAVAILABLE = PC_SUCCESS(4u),
    PC_SC_A = PC_SUCCESS(5u),
    PC_SC_B = PC_SUCCESS(6u),
    PC_SC_C = PC_SUCCESS(7u),

    POSTCODE_INFORMATION_SUCCESS_NUMBER,
};

#define PC_FAILED_CODE_MASK (0x7FFFFFFu)
#define PC_TO_CMPT_ID(code) (((code) >> PC_COMPONENT_NUMBER_POS) & PC_COMPONENT_NUMBER_MASK)

#define _PC_IOK(code) UNFLAG((code) & PC_FAILED_CODE_MASK)
#define _PC_IER(code) FLAG((code) & PC_FAILED_CODE_MASK)

u32p_t _impl_postcode_cmpt_failed_save(u32p_t postcode);

/**
 * @brief To trace the last failed postcode.
 *
 * @param postcode The postcode number.
 *
 * @return Return the original argument postcode.
 */
static inline u32p_t _impl_trace_postcode_cmpt_last_failed(u32p_t postcode)
{
    return _impl_postcode_cmpt_failed_save(postcode);
}

#define PC_TRACE(code)  _impl_trace_postcode_cmpt_last_failed(code)
#define PC_IOK_TC(code) _PC_IOK(PC_TRACE(code))
#define PC_IER_TC(code) _PC_IER(PC_TRACE(code))

#define PC_IOK(code) PC_IOK_TC(code)
#define PC_IER(code) PC_IER_TC(code)

#define _PC_CMPT_ASSERT_FAILED PC_FAILED(PC_CMPT_ASSERT_8)

#define _CHECK_CONDITION(cond)                                                                                                             \
    do {                                                                                                                                   \
        if (FLAG(!((i32_t)cond))) {                                                                                                        \
            PC_TRACE(_PC_CMPT_ASSERT_FAILED);                                                                                              \
        }                                                                                                                                  \
    } while (0)

#define _CHECK_POINTER(pointer)                                                                                                            \
    do {                                                                                                                                   \
        if (FLAG(!((i32_t)pointer))) {                                                                                                     \
            PC_TRACE(_PC_CMPT_ASSERT_FAILED);                                                                                              \
        }                                                                                                                                  \
    } while (0)

#define _CHECK_INDEX(index, high)                                                                                                          \
    do {                                                                                                                                   \
        if (FLAG(!(((i32_t)index >= 0) && ((i32_t)index < high)))) {                                                                       \
            PC_TRACE(_PC_CMPT_ASSERT_FAILED);                                                                                              \
        }                                                                                                                                  \
    } while (0)

#define _CHECK_BOUND(val, high)                                                                                                            \
    do {                                                                                                                                   \
        if (FLAG(!(((i32_t)val >= 0) && ((i32_t)val <= (i32_t)high)))) {                                                                   \
            PC_TRACE(_PC_CMPT_ASSERT_FAILED);                                                                                              \
        }                                                                                                                                  \
    } while (0)

#define _CHECK_RANGE(val, low, high)                                                                                                       \
    do {                                                                                                                                   \
        if (FLAG(!(((i32_t)val >= (i32_t)low) && ((i32_t)val <= (i32_t)high)))) {                                                          \
            PC_TRACE(_PC_CMPT_ASSERT_FAILED);                                                                                              \
        }                                                                                                                                  \
    } while (0)

#define D_ASSERT(cond)                 _CHECK_CONDITION(cond)
#define D_ASSERT_POINTER(pointer)      _CHECK_POINTER(pointer)
#define D_ASSERT_INDEX(index, high)    _CHECK_INDEX(index, high)
#define D_ASSERT_RANGE(val, low, high) _CHECK_RANGE(val, low, high)
#define D_ASSERT_BOUND(val, high)      _CHECK_BOUND(val, high)

S_ASSERT((POSTCODE_INFORMATION_SUCCESS_NUMBER <= PC_SUCCESS(PC_PASS_NUMBER_MASK)),
         "Warning: PC_SUCCESS_INFO_NUMBER is higher than PC_PASS_NUMBER_MASK");
S_ASSERT((POSTCODE_COMPONENT_NUMBER <= PC_COMPONENT_NUMBER_MASK), "Warning: PC_COMPONENT_NUMBER is higher than PC_COMPONENT_NUMBER_MASK");

#ifdef __cplusplus
}
#endif

#endif /* _POSTCODE_H_ */
