/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef _POSTCODE_H_
#define _POSTCODE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"
                                              /* To generating a unique token */
#define _STATIC_MAGIC_3(pre, post) 	    	MAGIC(pre, post)
#define _STATIC_MAGIC_2(pre, post) 	   		_STATIC_MAGIC_3(pre, post)

#if defined __COUNTER__
    #define _STATIC_MAGIC(pre)  	    	_STATIC_MAGIC_2(pre, COMPLIER_UNIQUE_NUMBER)
#else
    #define _STATIC_MAGIC(pre)  	    	_STATIC_MAGIC_2(pre, COMPLIER_UNIQUE_NUMBER)
#endif
#define S_ASSERT(cond, msg)    		        typedef char_t _STATIC_MAGIC(static_assert)[FLAG(cond) * 2 - 1];


#define COMPLIER_LINE_NUMBER      	    	__LINE__

#define PC_SUCCESS(OK_INFO)             	(((OK_INFO) & 0x0Fu) << 28u)

#define PC_FAILED_IO(COMPONENT, ERR_INFO)   (((((COMPONENT) & 0x3FFFu) << 14u)) | ((ERR_INFO) & 0x3FFFu))

#define PC_FAILED(COMPONENT)           		PC_FAILED_IO(COMPONENT, COMPLIER_LINE_NUMBER)

enum
{
    PC_CMPT_KERNAL = 1u,         /* 1 */
    PC_CMPT_THREAD,              /* 2 */
    PC_CMPT_SEMAPHORE,           /* 3 */
    PC_CMPT_MUTEX,               /* 4 */
    PC_CMPT_QUEUE,               /* 5 */
    PC_CMPT_EVENT,               /* 6 */
    PC_CMPT_TIMER,               /* 7 */

    PC_CMPT_ASSERT,              /* 8 */

    POSTCODE_COMPONENT_NUMBER,
};

enum
{
    PC_SC_SUCCESS        = PC_SUCCESS(0u),
    PC_SC_TIMEOUT        = PC_SUCCESS(1u),
    PC_SC_AVAILABLE      = PC_SUCCESS(2u),
    PC_SC_UNAVAILABLE    = PC_SUCCESS(3u),
    PC_SC_A              = PC_SUCCESS(4u),
    PC_SC_B              = PC_SUCCESS(5u),
    PC_SC_C              = PC_SUCCESS(6u),
    PC_SC_D              = PC_SUCCESS(7u),

    POSTCODE_INFORMATION_SUCCESS_NUMBER,
};

u32p_t _impl_postcode_trace_cmpt_last_failed(u32_t cmpt_instance, u32p_t postcode);

#define PC_FAILED_CODE_MASK                 (0xFFFFFFFu)

#define PC_IOK(code)                   		UNFLAG((code) & PC_FAILED_CODE_MASK)
#define PC_IER(code)                   		FLAG((code) & PC_FAILED_CODE_MASK)

#define PC_TRACE(code)                      _impl_postcode_trace_cmpt_last_failed((((code) >> 14u) & 0x3FFFu), code)
#define PC_IOK_TC(code)                     PC_IOK(PC_TRACE(code))
#define PC_IER_TC(code)                     PC_IER(PC_TRACE(code))

#define _CHECK_CONDITION(cond)                                                         								    \
        do {                                                                                                            \
            if (FLAG(!((i32_t)cond))) {                                                                                 \
				PC_TRACE(PC_FAILED(PC_CMPT_ASSERT));                                                                    \
			}                                                                                                           \
        } while(0)

#define _CHECK_POINTER(pointer)   		        		                               								    \
        do {                                                                                                            \
            if (FLAG(!((i32_t)pointer))) {                                                                              \
                PC_TRACE(PC_FAILED(PC_CMPT_ASSERT));                                                                    \
            }                                                                                                           \
        } while(0)

#define _CHECK_INDEX(index, high)                                                           						    \
        do {                                                                                                            \
            if (FLAG(!(((i32_t)index >= 0) && ((i32_t)index < high)))) {                                                \
                PC_TRACE(PC_FAILED(PC_CMPT_ASSERT));                                                                    \
            }                                                                                                           \
        } while(0)

#define _CHECK_BOUND(val, high)                                                        								    \
        do {                                                                                                            \
            if (FLAG(!(((i32_t)val >= 0) && ((i32_t)val <= (i32_t)high)))) {                                            \
                PC_TRACE(PC_FAILED(PC_CMPT_ASSERT));                                                                    \
            }                                                                                                           \
        } while(0)

#define _CHECK_RANGE(val, low, high)                                                        						    \
        do {                                                                                                            \
            if (FLAG(!(((i32_t)val >= (i32_t)low) && ((i32_t)val <= (i32_t)high)))) {                                   \
                PC_TRACE(PC_FAILED(PC_CMPT_ASSERT));                                                                    \
            }                                                                                                           \
        } while(0)

#define D_ASSERT(cond) 							_CHECK_CONDITION(cond)
#define D_ASSERT_POINTER(pointer) 				_CHECK_POINTER(pointer)
#define D_ASSERT_INDEX(index, high) 			_CHECK_INDEX(index, high)
#define D_ASSERT_RANGE(val, low, high) 			_CHECK_RANGE(val, low, high)
#define D_ASSERT_BOUND(val, high) 				_CHECK_BOUND(val, high)

S_ASSERT((POSTCODE_INFORMATION_SUCCESS_NUMBER <= PC_SUCCESS(0x0Fu)), "Warning: PC_SUCCESS_INFO_NUMBER is higher than 0x0Fu");
S_ASSERT((POSTCODE_COMPONENT_NUMBER <= 0x3FFFu), "Warning: PC_COMPONENT_NUMBER is higher than 0x3FFFu");

#ifdef __cplusplus
}
#endif

#endif /* _POSTCODE_H_ */
