/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 **/
#ifndef _TYPE_DEF_H_
#define _TYPE_DEF_H_

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef char _char_t;
typedef unsigned char _uchar_t;
typedef unsigned char _u8_t;
typedef unsigned short _u16_t;
typedef unsigned int _u32_t;
typedef unsigned long long _u64_t;
typedef signed char _i8_t;
typedef signed short _i16_t;
typedef signed int _i32_t;
typedef signed long long _i64_t;
typedef bool _b_t;
typedef _i32_t _i32p_t;

#ifndef FALSE
#define FALSE false
#endif

#ifndef TRUE
#define TRUE true
#endif

#define U8_B    (8u)
#define U16_B   (16u)
#define U32_B   (32u)
#define U8_MAX  ((_u8_t)-1)
#define U16_MAX ((_u16_t)-1)
#define U32_MAX ((_u32_t)-1)

#define UNUSED_MSG(x) (void)(x)
#define UNUSED_ARG    (0u)

#define FLAG(x)   (!!(x))
#define UNFLAG(x) (!!!(x))

#define MAGIC(pre, post) pre##post

#if defined __COUNTER__
#define COMPLIER_UNIQUE_NUMBER __COUNTER__
#else
#define COMPLIER_UNIQUE_NUMBER __LINE__
#endif
#define COMPLIER_LINE_NUMBER __LINE__

/* To generating a unique token */
#define _STATIC_MAGIC_3(pre, post) MAGIC(pre, post)
#define _STATIC_MAGIC_2(pre, post) _STATIC_MAGIC_3(pre, post)

#if defined __COUNTER__
#define _STATIC_MAGIC(pre) _STATIC_MAGIC_2(pre, COMPLIER_UNIQUE_NUMBER)
#else
#define _STATIC_MAGIC(pre) _STATIC_MAGIC_2(pre, COMPLIER_UNIQUE_NUMBER)
#endif
#define BUILD_ASSERT(cond, msg) typedef _char_t _STATIC_MAGIC(static_assert)[FLAG(cond) * 2 - 1]

#define RUN_ASSERT(c)                                                                                                                      \
    if (UNFLAG(c)) {                                                                                                                       \
        while (1) {};                                                                                                                      \
    }

#define RUN_UNREACHABLE() RUN_ASSERT(0)

#define VREG32(addr) (*(volatile _u32_t *)(_u32_t)(addr))
#define VREG16(addr) (*(volatile _u16_t *)(_u32_t)(addr))
#define VREG8(addr)  (*(volatile _u8_t *)(_u32_t)(addr))

#define B(x)                        (_u32_t)((_u32_t)1u << (x))
#define MSK_B(x)                    (_u32_t)(B(x) - 1u)
#define Bs(start, end)              (_u32_t)((0xFFFFFFFFul << (start)) & (0xFFFFFFFFul >> (31u - (_u32_t)(end))))
#define DUMP_Bs(regval, start, end) (_u32_t)(((regval) & Bs((start), (end))) >> (start))

#define BV_GET(v, m, p)    ((v >> ((p) * (m))) & (MSK_B(m)))
#define BV_CLR(v, m, p)    (v &= ~((MSK_B(m)) << ((p) * (m))))
#define BV_SET(v, m, p, n) (v |= ((n) << ((p) * (m))))
#define BV_CS(v, m, p, n)                                                                                                                  \
    BV_CLR(v, m, p);                                                                                                                       \
    BV_SET(v, m, p, n)

#define DEQUALIFY(s, v)      ((s)(_u32_t)(const volatile void *)(v))
#define OFFSETOF(s, m)       ((_u32_t)(&((s *)0)->m))
#define CONTAINEROF(p, s, m) (DEQUALIFY(s *, ((const volatile _u8_t *)(p)-OFFSETOF(s, m))))
#define SIZEOF(arr)          (sizeof(arr))
#define DIMOF(arr)           (SIZEOF(arr) / SIZEOF(arr[0]))

#define MINI_AB(a, b) (((a) < (b)) ? (a) : (b))
#define MAX_AB(a, b)  (((a) > (b)) ? (a) : (b))

#define ROUND_UP(size, align)   (((_u32_t)(size) + (align - 1u)) & (~(align - 1)))
#define ROUND_DOWN(size, align) (((_u32_t)(size)) & (~(align - 1)))
#define RANGE_ADDRESS_CONDITION(address, pool)                                                                                             \
    (((_u32_t)(address) >= (_u32_t)(pool)) && ((_u32_t)(address) < ((_u32_t)(pool) + (_u16_t)SIZEOF(pool))))

#define PC_COMMON_NUMBER_POS    (0u)
#define PC_COMMON_NUMBER_MSK    (MSK_B(8)) /* The maximum pass information number up to 31 */
#define PC_LINE_NUMBER_POS      (8u)
#define PC_LINE_NUMBER_MSK      (MSK_B(13)) /* The maximum line number up to 8191 */
#define PC_COMPONENT_NUMBER_POS (21u)
#define PC_COMPONENT_NUMBER_MSK (MSK_B(10)) /* The maximum component number up to 1024 */

#define _PC_COMMON_VALUE(n)    (((n) & PC_COMMON_NUMBER_MSK) << PC_COMMON_NUMBER_POS)
#define _PC_LINE_VALUE(e)      (((e) & PC_LINE_NUMBER_MSK) << PC_LINE_NUMBER_POS)
#define _PC_COMPONENT_VALUE(c) (((c) & PC_COMPONENT_NUMBER_MSK) << PC_COMPONENT_NUMBER_POS)
#define _PC_VALUE(c, n)        (_PC_COMPONENT_VALUE(c) | _PC_LINE_VALUE(COMPLIER_LINE_NUMBER) | _PC_COMMON_VALUE(n))
#define PC_NEGATIVE(v)         (_i32_t)(U32_MAX - (_i32_t)(v))
#define PC_IER(c)              (PC_NEGATIVE(_PC_VALUE(c, 0)))
#define PC_NER(c, n)           (PC_NEGATIVE(_PC_VALUE(c, n)))

#define PC_CMPT_NUMBER_INIT (1u)
#define PC_CMPT_ASSERT      (PC_CMPT_NUMBER_INIT)
#define PC_CMPT_NUMBER_USER (PC_CMPT_NUMBER_INIT + 1u)

#define REVERSE_NUMBER_32                                                                                                                  \
    32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0

#define ARGS_INTERGRATION_32(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23,     \
                             _24, _25, _26, _27, _28, _29, _30, _31, _32, N, ...)                                                          \
    N
#define ARGS_SHIFT_32(...) ARGS_INTERGRATION_32(__VA_ARGS__)
#define ARGS_NUM_32(...)   ARGS_SHIFT_32(__VA_ARGS__, REVERSE_NUMBER_32)
#define ARGS_NUM(...)      ARGS_NUM_32(__VA_ARGS__)

#define ARGS_1(_1, ...)                                                                                                                  _1
#define ARGS_2(_1, _2, ...)                                                                                                              _2
#define ARGS_3(_1, _2, _3, ...)                                                                                                          _3
#define ARGS_4(_1, _2, _3, _4, ...)                                                                                                      _4
#define ARGS_5(_1, _2, _3, _4, _5, ...)                                                                                                  _5
#define ARGS_6(_1, _2, _3, _4, _5, _6, ...)                                                                                              _6
#define ARGS_7(_1, _2, _3, _4, _5, _6, _7, ...)                                                                                          _7
#define ARGS_8(_1, _2, _3, _4, _5, _6, _7, _8, ...)                                                                                      _8
#define ARGS_9(_1, _2, _3, _4, _5, _6, _7, _8, _9, ...)                                                                                  _9
#define ARGS_10(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, ...)                                                                            _10
#define ARGS_11(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, ...)                                                                       _11
#define ARGS_12(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, ...)                                                                  _12
#define ARGS_13(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, ...)                                                             _13
#define ARGS_14(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, ...)                                                        _14
#define ARGS_15(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, ...)                                                   _15
#define ARGS_16(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, ...)                                              _16
#define ARGS_17(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, ...)                                         _17
#define ARGS_18(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, ...)                                    _18
#define ARGS_19(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, ...)                               _19
#define ARGS_20(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, ...)                          _20
#define ARGS_21(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, ...)                     _21
#define ARGS_22(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, ...)                _22
#define ARGS_23(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, ...)           _23
#define ARGS_24(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, ...)      _24
#define ARGS_25(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, ...) _25
#define ARGS_26(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26,   \
                ...)                                                                                                                       \
    _26
#define ARGS_27(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26,   \
                _27, ...)                                                                                                                  \
    _27
#define ARGS_28(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26,   \
                _27, _28, ...)                                                                                                             \
    _28
#define ARGS_29(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26,   \
                _27, _28, _29, ...)                                                                                                        \
    _29
#define ARGS_30(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26,   \
                _27, _28, _29, _30, ...)                                                                                                   \
    _30
#define ARGS_31(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26,   \
                _27, _28, _29, _30, _31, ...)                                                                                              \
    _31
#define ARGS_32(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26,   \
                _27, _28, _29, _30, _31, _32, ...)                                                                                         \
    _32

#define H_ARGS          ARGS_
#define AG_3(pre, post) MAGIC(pre, post)
#define AG_2(pre, post) AG_3(pre, post)
#define AG(m)           AG_2(H_ARGS, m)
#define ARGS_N(N, ...)  AG(N)(__VA_ARGS__)

#endif /* _TYPE_DEF_H_ */
