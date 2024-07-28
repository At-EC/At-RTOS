/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 **/

#ifndef _TYPEDEF_H_
#define _TYPEDEF_H_

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef char char_t;
typedef unsigned char uchar_t;
typedef unsigned char u8_t;
typedef unsigned short u16_t;
typedef unsigned int u32_t;
typedef unsigned long long u64_t;
typedef signed char i8_t;
typedef signed short i16_t;
typedef signed int i32_t;
typedef signed long long i64_t;
typedef volatile char vchar_t;
typedef volatile unsigned char vuchar_t;
typedef volatile unsigned char vu8_t;
typedef volatile unsigned short vu16_t;
typedef volatile unsigned int vu32_t;
typedef volatile unsigned long long vu64_t;
typedef volatile signed char vi8_t;
typedef volatile signed short vi16_t;
typedef volatile signed int vi32_t;
typedef volatile signed long long vi64_t;
typedef bool b_t;
typedef u32_t u32p_t;

#ifndef FALSE
#define FALSE false
#endif

#ifndef TRUE
#define TRUE true
#endif

#define U8_B  (8u)
#define U16_B (16u)
#define U32_B (32u)
#define U8_V  (0xFFu)
#define U16_V (0xFFFFu)
#define U32_V (0xFFFFFFFFu)

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

#define VREG32(addr) (*(volatile u32_t *)(u32_t)(addr))
#define VREG16(addr) (*(volatile u16_t *)(u32_t)(addr))
#define VREG8(addr)  (*(volatile u8_t *)(u32_t)(addr))

#define SET_BIT(x)                    (u32_t)((u32_t)1u << (x))
#define MASK_BIT(x)                   (u32_t)(SET_BIT(x) - 1u)
#define SET_BITS(start, end)          (u32_t)((0xFFFFFFFFul << (start)) & (0xFFFFFFFFul >> (31u - (u32_t)(end))))
#define DUMP_BITS(regval, start, end) (u32_t)(((regval) & SET_BITS((start), (end))) >> (start))

#define BV_C(v, m, p)    v &= ~((MASK_BIT(m)) << ((p) * (m)))
#define BV_S(v, m, p, n) v |= ((n) << ((p) * (m)))
#define BV_CS(v, m, p, n)                                                                                                                  \
    BV_C(v, m, p);                                                                                                                         \
    BV_S(v, m, p, n)

#define DEQUALIFY(s, v)      ((s)(u32_t)(const volatile void *)(v))
#define OFFSETOF(s, m)       ((u32_t)(&((s *)0)->m))
#define CONTAINEROF(p, s, m) (DEQUALIFY(s *, ((const vu8_t *)(p)-OFFSETOF(s, m))))
#define SIZEOF(arr)          (sizeof(arr))
#define DIMOF(arr)           (SIZEOF(arr) / SIZEOF(arr[0]))

#define MINI_AB(a, b) (((a) < (b)) ? (a) : (b))
#define MAX_AB(a, b)  (((a) > (b)) ? (a) : (b))

#define ROUND_UP(size, align)   (((u32_t)(size) + (align - 1u)) & (~(align - 1)))
#define ROUND_DOWN(size, align) (((u32_t)(size)) & (~(align - 1)))
#define RANGE_ADDRESS_CONDITION(address, pool)                                                                                             \
    (((u32_t)(address) >= (u32_t)(pool)) && ((u32_t)(address) < ((u32_t)(pool) + (u16_t)SIZEOF(pool))))

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

#define CBITS           .bits.
#define CV_3(pre, post) MAGIC(pre, post)
#define CV_2(pre, post) CV_3(pre, post)
#define CV(m)           CV_2(CBITS, m)
#define CB(c, b)        CV_2(c, CV(b))

#define CMV              CM_V
#define CMV_3(pre, post) MAGIC(pre, post)
#define CMV_2(pre, post) CMV_3(pre, post)
#define CM(m)            CMV_2(CMV, m)

#define CM_V1(...)                                                                                                                         \
    {                                                                                                                                      \
        CV(ARGS_N(1, __VA_ARGS__)) = ARGS_N(2, __VA_ARGS__)                                                                                \
    }
#define CM_V2(...)                                                                                                                         \
    {                                                                                                                                      \
        CV(ARGS_N(1, __VA_ARGS__)) = ARGS_N(3, __VA_ARGS__), CV(ARGS_N(2, __VA_ARGS__)) = ARGS_N(4, __VA_ARGS__)                           \
    }
#define CM_V3(...)                                                                                                                         \
    {                                                                                                                                      \
        CV(ARGS_N(1, __VA_ARGS__)) = ARGS_N(4, __VA_ARGS__), CV(ARGS_N(2, __VA_ARGS__)) = ARGS_N(5, __VA_ARGS__),                          \
                     CV(ARGS_N(3, __VA_ARGS__)) = ARGS_N(6, __VA_ARGS__)                                                                   \
    }
#define CM_V4(...)                                                                                                                         \
    {                                                                                                                                      \
        CV(ARGS_N(1, __VA_ARGS__)) = ARGS_N(5, __VA_ARGS__), CV(ARGS_N(2, __VA_ARGS__)) = ARGS_N(6, __VA_ARGS__),                          \
                     CV(ARGS_N(3, __VA_ARGS__)) = ARGS_N(7, __VA_ARGS__), CV(ARGS_N(4, __VA_ARGS__)) = ARGS_N(8, __VA_ARGS__)              \
    }
#define CM_V5(...)                                                                                                                         \
    {                                                                                                                                      \
        CV(ARGS_N(1, __VA_ARGS__)) = ARGS_N(6, __VA_ARGS__), CV(ARGS_N(2, __VA_ARGS__)) = ARGS_N(7, __VA_ARGS__),                          \
                     CV(ARGS_N(3, __VA_ARGS__)) = ARGS_N(8, __VA_ARGS__), CV(ARGS_N(4, __VA_ARGS__)) = ARGS_N(9, __VA_ARGS__),             \
                     CV(ARGS_N(5, __VA_ARGS__)) = ARGS_N(10, __VA_ARGS__)                                                                  \
    }
#define CM_V6(...)                                                                                                                         \
    {                                                                                                                                      \
        CV(ARGS_N(1, __VA_ARGS__)) = ARGS_N(7, __VA_ARGS__), CV(ARGS_N(2, __VA_ARGS__)) = ARGS_N(8, __VA_ARGS__),                          \
                     CV(ARGS_N(3, __VA_ARGS__)) = ARGS_N(9, __VA_ARGS__), CV(ARGS_N(4, __VA_ARGS__)) = ARGS_N(10, __VA_ARGS__),            \
                     CV(ARGS_N(5, __VA_ARGS__)) = ARGS_N(11, __VA_ARGS__), CV(ARGS_N(6, __VA_ARGS__)) = ARGS_N(12, __VA_ARGS__)            \
    }
#define CM_V7(...)                                                                                                                         \
    {                                                                                                                                      \
        CV(ARGS_N(1, __VA_ARGS__)) = ARGS_N(8, __VA_ARGS__), CV(ARGS_N(2, __VA_ARGS__)) = ARGS_N(9, __VA_ARGS__),                          \
                     CV(ARGS_N(3, __VA_ARGS__)) = ARGS_N(10, __VA_ARGS__), CV(ARGS_N(4, __VA_ARGS__)) = ARGS_N(11, __VA_ARGS__),           \
                     CV(ARGS_N(5, __VA_ARGS__)) = ARGS_N(12, __VA_ARGS__), CV(ARGS_N(6, __VA_ARGS__)) = ARGS_N(13, __VA_ARGS__),           \
                     CV(ARGS_N(7, __VA_ARGS__)) = ARGS_N(14, __VA_ARGS__)                                                                  \
    }
#define CM_V8(...)                                                                                                                         \
    {                                                                                                                                      \
        CV(ARGS_N(1, __VA_ARGS__)) = ARGS_N(9, __VA_ARGS__), CV(ARGS_N(2, __VA_ARGS__)) = ARGS_N(10, __VA_ARGS__),                         \
                     CV(ARGS_N(3, __VA_ARGS__)) = ARGS_N(11, __VA_ARGS__), CV(ARGS_N(4, __VA_ARGS__)) = ARGS_N(12, __VA_ARGS__),           \
                     CV(ARGS_N(5, __VA_ARGS__)) = ARGS_N(13, __VA_ARGS__), CV(ARGS_N(6, __VA_ARGS__)) = ARGS_N(14, __VA_ARGS__),           \
                     CV(ARGS_N(7, __VA_ARGS__)) = ARGS_N(15, __VA_ARGS__), CV(ARGS_N(8, __VA_ARGS__)) = ARGS_N(16, __VA_ARGS__)            \
    }
#define CM_V9(...)                                                                                                                         \
    {                                                                                                                                      \
        CV(ARGS_N(1, __VA_ARGS__)) = ARGS_N(10, __VA_ARGS__), CV(ARGS_N(2, __VA_ARGS__)) = ARGS_N(11, __VA_ARGS__),                        \
                     CV(ARGS_N(3, __VA_ARGS__)) = ARGS_N(12, __VA_ARGS__), CV(ARGS_N(4, __VA_ARGS__)) = ARGS_N(13, __VA_ARGS__),           \
                     CV(ARGS_N(5, __VA_ARGS__)) = ARGS_N(14, __VA_ARGS__), CV(ARGS_N(6, __VA_ARGS__)) = ARGS_N(15, __VA_ARGS__),           \
                     CV(ARGS_N(7, __VA_ARGS__)) = ARGS_N(16, __VA_ARGS__), CV(ARGS_N(8, __VA_ARGS__)) = ARGS_N(17, __VA_ARGS__),           \
                     CV(ARGS_N(9, __VA_ARGS__)) = ARGS_N(18, __VA_ARGS__)                                                                  \
    }
#define CM_V10(...)                                                                                                                        \
    {                                                                                                                                      \
        CV(ARGS_N(1, __VA_ARGS__)) = ARGS_N(11, __VA_ARGS__), CV(ARGS_N(2, __VA_ARGS__)) = ARGS_N(12, __VA_ARGS__),                        \
                     CV(ARGS_N(3, __VA_ARGS__)) = ARGS_N(13, __VA_ARGS__), CV(ARGS_N(4, __VA_ARGS__)) = ARGS_N(14, __VA_ARGS__),           \
                     CV(ARGS_N(5, __VA_ARGS__)) = ARGS_N(15, __VA_ARGS__), CV(ARGS_N(6, __VA_ARGS__)) = ARGS_N(16, __VA_ARGS__),           \
                     CV(ARGS_N(7, __VA_ARGS__)) = ARGS_N(17, __VA_ARGS__), CV(ARGS_N(8, __VA_ARGS__)) = ARGS_N(18, __VA_ARGS__),           \
                     CV(ARGS_N(9, __VA_ARGS__)) = ARGS_N(19, __VA_ARGS__), CV(ARGS_N(10, __VA_ARGS__)) = ARGS_N(20, __VA_ARGS__)           \
    }
#define CM_V11(...)                                                                                                                        \
    {                                                                                                                                      \
        CV(ARGS_N(1, __VA_ARGS__)) = ARGS_N(12, __VA_ARGS__), CV(ARGS_N(2, __VA_ARGS__)) = ARGS_N(13, __VA_ARGS__),                        \
                     CV(ARGS_N(3, __VA_ARGS__)) = ARGS_N(14, __VA_ARGS__), CV(ARGS_N(4, __VA_ARGS__)) = ARGS_N(15, __VA_ARGS__),           \
                     CV(ARGS_N(5, __VA_ARGS__)) = ARGS_N(16, __VA_ARGS__), CV(ARGS_N(6, __VA_ARGS__)) = ARGS_N(17, __VA_ARGS__),           \
                     CV(ARGS_N(7, __VA_ARGS__)) = ARGS_N(18, __VA_ARGS__), CV(ARGS_N(8, __VA_ARGS__)) = ARGS_N(19, __VA_ARGS__),           \
                     CV(ARGS_N(9, __VA_ARGS__)) = ARGS_N(20, __VA_ARGS__), CV(ARGS_N(10, __VA_ARGS__)) = ARGS_N(21, __VA_ARGS__),          \
                     CV(ARGS_N(11, __VA_ARGS__)) = ARGS_N(22, __VA_ARGS__)                                                                 \
    }
#define CM_V12(...)                                                                                                                        \
    {                                                                                                                                      \
        CV(ARGS_N(1, __VA_ARGS__)) = ARGS_N(13, __VA_ARGS__), CV(ARGS_N(2, __VA_ARGS__)) = ARGS_N(14, __VA_ARGS__),                        \
                     CV(ARGS_N(3, __VA_ARGS__)) = ARGS_N(15, __VA_ARGS__), CV(ARGS_N(4, __VA_ARGS__)) = ARGS_N(16, __VA_ARGS__),           \
                     CV(ARGS_N(5, __VA_ARGS__)) = ARGS_N(17, __VA_ARGS__), CV(ARGS_N(6, __VA_ARGS__)) = ARGS_N(18, __VA_ARGS__),           \
                     CV(ARGS_N(7, __VA_ARGS__)) = ARGS_N(19, __VA_ARGS__), CV(ARGS_N(8, __VA_ARGS__)) = ARGS_N(20, __VA_ARGS__),           \
                     CV(ARGS_N(9, __VA_ARGS__)) = ARGS_N(21, __VA_ARGS__), CV(ARGS_N(10, __VA_ARGS__)) = ARGS_N(22, __VA_ARGS__),          \
                     CV(ARGS_N(11, __VA_ARGS__)) = ARGS_N(23, __VA_ARGS__), CV(ARGS_N(12, __VA_ARGS__)) = ARGS_N(24, __VA_ARGS__)          \
    }
#define CM_V13(...)                                                                                                                        \
    {                                                                                                                                      \
        CV(ARGS_N(1, __VA_ARGS__)) = ARGS_N(14, __VA_ARGS__), CV(ARGS_N(2, __VA_ARGS__)) = ARGS_N(15, __VA_ARGS__),                        \
                     CV(ARGS_N(3, __VA_ARGS__)) = ARGS_N(16, __VA_ARGS__), CV(ARGS_N(4, __VA_ARGS__)) = ARGS_N(17, __VA_ARGS__),           \
                     CV(ARGS_N(5, __VA_ARGS__)) = ARGS_N(18, __VA_ARGS__), CV(ARGS_N(6, __VA_ARGS__)) = ARGS_N(19, __VA_ARGS__),           \
                     CV(ARGS_N(7, __VA_ARGS__)) = ARGS_N(20, __VA_ARGS__), CV(ARGS_N(8, __VA_ARGS__)) = ARGS_N(21, __VA_ARGS__),           \
                     CV(ARGS_N(9, __VA_ARGS__)) = ARGS_N(22, __VA_ARGS__), CV(ARGS_N(10, __VA_ARGS__)) = ARGS_N(23, __VA_ARGS__),          \
                     CV(ARGS_N(11, __VA_ARGS__)) = ARGS_N(24, __VA_ARGS__), CV(ARGS_N(12, __VA_ARGS__)) = ARGS_N(25, __VA_ARGS__),         \
                     CV(ARGS_N(13, __VA_ARGS__)) = ARGS_N(26, __VA_ARGS__),                                                                \
    }
#define CM_V14(...)                                                                                                                        \
    {                                                                                                                                      \
        CV(ARGS_N(1, __VA_ARGS__)) = ARGS_N(15, __VA_ARGS__), CV(ARGS_N(2, __VA_ARGS__)) = ARGS_N(16, __VA_ARGS__),                        \
                     CV(ARGS_N(3, __VA_ARGS__)) = ARGS_N(17, __VA_ARGS__), CV(ARGS_N(4, __VA_ARGS__)) = ARGS_N(18, __VA_ARGS__),           \
                     CV(ARGS_N(5, __VA_ARGS__)) = ARGS_N(19, __VA_ARGS__), CV(ARGS_N(6, __VA_ARGS__)) = ARGS_N(20, __VA_ARGS__),           \
                     CV(ARGS_N(7, __VA_ARGS__)) = ARGS_N(21, __VA_ARGS__), CV(ARGS_N(8, __VA_ARGS__)) = ARGS_N(22, __VA_ARGS__),           \
                     CV(ARGS_N(9, __VA_ARGS__)) = ARGS_N(23, __VA_ARGS__), CV(ARGS_N(10, __VA_ARGS__)) = ARGS_N(24, __VA_ARGS__),          \
                     CV(ARGS_N(11, __VA_ARGS__)) = ARGS_N(25, __VA_ARGS__), CV(ARGS_N(12, __VA_ARGS__)) = ARGS_N(26, __VA_ARGS__),         \
                     CV(ARGS_N(13, __VA_ARGS__)) = ARGS_N(27, __VA_ARGS__), CV(ARGS_N(14, __VA_ARGS__)) = ARGS_N(28, __VA_ARGS__)          \
    }
#define CM_V15(...)                                                                                                                        \
    {                                                                                                                                      \
        CV(ARGS_N(1, __VA_ARGS__)) = ARGS_N(16, __VA_ARGS__), CV(ARGS_N(2, __VA_ARGS__)) = ARGS_N(17, __VA_ARGS__),                        \
                     CV(ARGS_N(3, __VA_ARGS__)) = ARGS_N(18, __VA_ARGS__), CV(ARGS_N(4, __VA_ARGS__)) = ARGS_N(19, __VA_ARGS__),           \
                     CV(ARGS_N(5, __VA_ARGS__)) = ARGS_N(20, __VA_ARGS__), CV(ARGS_N(6, __VA_ARGS__)) = ARGS_N(21, __VA_ARGS__),           \
                     CV(ARGS_N(7, __VA_ARGS__)) = ARGS_N(22, __VA_ARGS__), CV(ARGS_N(8, __VA_ARGS__)) = ARGS_N(23, __VA_ARGS__),           \
                     CV(ARGS_N(9, __VA_ARGS__)) = ARGS_N(24, __VA_ARGS__), CV(ARGS_N(10, __VA_ARGS__)) = ARGS_N(25, __VA_ARGS__),          \
                     CV(ARGS_N(11, __VA_ARGS__)) = ARGS_N(26, __VA_ARGS__), CV(ARGS_N(12, __VA_ARGS__)) = ARGS_N(27, __VA_ARGS__),         \
                     CV(ARGS_N(13, __VA_ARGS__)) = ARGS_N(28, __VA_ARGS__), CV(ARGS_N(14, __VA_ARGS__)) = ARGS_N(29, __VA_ARGS__),         \
                     CV(ARGS_N(15, __VA_ARGS__)) = ARGS_N(30, __VA_ARGS__),                                                                \
    }

#define BS_MISMATCH (0xFFFFFFFFu)

#define BS_V2(c, ...) (c == ARGS_N(1, __VA_ARGS__)) ? (ARGS_N(2, __VA_ARGS__)) : BS_MISMATCH
#define BS_V4(c, ...)                                                                                                                      \
    (c == ARGS_N(1, __VA_ARGS__)) ? (ARGS_N(2, __VA_ARGS__)) : ((c == ARGS_N(3, __VA_ARGS__)) ? (ARGS_N(4, __VA_ARGS__)) : BS_MISMATCH)

#define BS_V6(c, ...)                                                                                                                      \
    (c == ARGS_N(1, __VA_ARGS__))                                                                                                          \
        ? (ARGS_N(2, __VA_ARGS__))                                                                                                         \
        : ((c == ARGS_N(3, __VA_ARGS__)) ? (ARGS_N(4, __VA_ARGS__))                                                                        \
                                         : ((c == ARGS_N(5, __VA_ARGS__)) ? (ARGS_N(6, __VA_ARGS__)) : BS_MISMATCH))
#define BS_V8(c, ...)                                                                                                                      \
    (c == ARGS_N(1, __VA_ARGS__))                                                                                                          \
        ? (ARGS_N(2, __VA_ARGS__))                                                                                                         \
        : ((c == ARGS_N(3, __VA_ARGS__))                                                                                                   \
               ? (ARGS_N(4, __VA_ARGS__))                                                                                                  \
               : ((c == ARGS_N(5, __VA_ARGS__)) ? (ARGS_N(6, __VA_ARGS__))                                                                 \
                                                : ((c == ARGS_N(7, __VA_ARGS__)) ? (ARGS_N(8, __VA_ARGS__)) : BS_MISMATCH)))
#define BS_V10(c, ...)                                                                                                                     \
    (c == ARGS_N(1, __VA_ARGS__))                                                                                                          \
        ? (ARGS_N(2, __VA_ARGS__))                                                                                                         \
        : ((c == ARGS_N(3, __VA_ARGS__))                                                                                                   \
               ? (ARGS_N(4, __VA_ARGS__))                                                                                                  \
               : ((c == ARGS_N(5, __VA_ARGS__))                                                                                            \
                      ? (ARGS_N(6, __VA_ARGS__))                                                                                           \
                      : ((c == ARGS_N(7, __VA_ARGS__)) ? (ARGS_N(8, __VA_ARGS__))                                                          \
                                                       : ((c == ARGS_N(9, __VA_ARGS__)) ? (ARGS_N(10, __VA_ARGS__)) : BS_MISMATCH))))

#define BS_V12(c, ...)                                                                                                                     \
    (c == ARGS_N(1, __VA_ARGS__))                                                                                                          \
        ? (ARGS_N(2, __VA_ARGS__))                                                                                                         \
        : ((c == ARGS_N(3, __VA_ARGS__))                                                                                                   \
               ? (ARGS_N(4, __VA_ARGS__))                                                                                                  \
               : ((c == ARGS_N(5, __VA_ARGS__))                                                                                            \
                      ? (ARGS_N(6, __VA_ARGS__))                                                                                           \
                      : ((c == ARGS_N(7, __VA_ARGS__))                                                                                     \
                             ? (ARGS_N(8, __VA_ARGS__))                                                                                    \
                             : ((c == ARGS_N(9, __VA_ARGS__))                                                                              \
                                    ? (ARGS_N(10, __VA_ARGS__))                                                                            \
                                    : ((c == ARGS_N(11, __VA_ARGS__)) ? (ARGS_N(12, __VA_ARGS__)) : BS_MISMATCH)))))

#define BS_V14(c, ...)                                                                                                                     \
    (c == ARGS_N(1, __VA_ARGS__))                                                                                                          \
        ? (ARGS_N(2, __VA_ARGS__))                                                                                                         \
        : ((c == ARGS_N(3, __VA_ARGS__))                                                                                                   \
               ? (ARGS_N(4, __VA_ARGS__))                                                                                                  \
               : ((c == ARGS_N(5, __VA_ARGS__))                                                                                            \
                      ? (ARGS_N(6, __VA_ARGS__))                                                                                           \
                      : ((c == ARGS_N(7, __VA_ARGS__))                                                                                     \
                             ? (ARGS_N(8, __VA_ARGS__))                                                                                    \
                             : ((c == ARGS_N(9, __VA_ARGS__))                                                                              \
                                    ? (ARGS_N(10, __VA_ARGS__))                                                                            \
                                    : ((c == ARGS_N(11, __VA_ARGS__))                                                                      \
                                           ? (ARGS_N(12, __VA_ARGS__))                                                                     \
                                           : ((c == ARGS_N(13, __VA_ARGS__)) ? (ARGS_N(14, __VA_ARGS__)) : BS_MISMATCH))))))

#define BS_V16(c, ...)                                                                                                                     \
    (c == ARGS_N(1, __VA_ARGS__))                                                                                                          \
        ? (ARGS_N(2, __VA_ARGS__))                                                                                                         \
        : ((c == ARGS_N(3, __VA_ARGS__))                                                                                                   \
               ? (ARGS_N(4, __VA_ARGS__))                                                                                                  \
               : ((c == ARGS_N(5, __VA_ARGS__))                                                                                            \
                      ? (ARGS_N(6, __VA_ARGS__))                                                                                           \
                      : ((c == ARGS_N(7, __VA_ARGS__))                                                                                     \
                             ? (ARGS_N(8, __VA_ARGS__))                                                                                    \
                             : ((c == ARGS_N(9, __VA_ARGS__))                                                                              \
                                    ? (ARGS_N(10, __VA_ARGS__))                                                                            \
                                    : ((c == ARGS_N(11, __VA_ARGS__))                                                                      \
                                           ? (ARGS_N(12, __VA_ARGS__))                                                                     \
                                           : ((c == ARGS_N(13, __VA_ARGS__))                                                               \
                                                  ? (ARGS_N(14, __VA_ARGS__))                                                              \
                                                  : ((c == ARGS_N(15, __VA_ARGS__)) ? (ARGS_N(16, __VA_ARGS__)) : BS_MISMATCH)))))))

#define BS_V18(c, ...)                                                                                                                     \
    (c == ARGS_N(1, __VA_ARGS__))                                                                                                          \
        ? (ARGS_N(2, __VA_ARGS__))                                                                                                         \
        : ((c == ARGS_N(3, __VA_ARGS__))                                                                                                   \
               ? (ARGS_N(4, __VA_ARGS__))                                                                                                  \
               : ((c == ARGS_N(5, __VA_ARGS__))                                                                                            \
                      ? (ARGS_N(6, __VA_ARGS__))                                                                                           \
                      : ((c == ARGS_N(7, __VA_ARGS__))                                                                                     \
                             ? (ARGS_N(8, __VA_ARGS__))                                                                                    \
                             : ((c == ARGS_N(9, __VA_ARGS__))                                                                              \
                                    ? (ARGS_N(10, __VA_ARGS__))                                                                            \
                                    : ((c == ARGS_N(11, __VA_ARGS__))                                                                      \
                                           ? (ARGS_N(12, __VA_ARGS__))                                                                     \
                                           : ((c == ARGS_N(13, __VA_ARGS__))                                                               \
                                                  ? (ARGS_N(14, __VA_ARGS__))                                                              \
                                                  : ((c == ARGS_N(15, __VA_ARGS__))                                                        \
                                                         ? (ARGS_N(16, __VA_ARGS__))                                                       \
                                                         : ((c == ARGS_N(17, __VA_ARGS__)) ? (ARGS_N(18, __VA_ARGS__))                     \
                                                                                           : BS_MISMATCH))))))))
#define BS_V20(c, ...)                                                                                                                     \
    (c == ARGS_N(1, __VA_ARGS__))                                                                                                          \
        ? (ARGS_N(2, __VA_ARGS__))                                                                                                         \
        : ((c == ARGS_N(3, __VA_ARGS__))                                                                                                   \
               ? (ARGS_N(4, __VA_ARGS__))                                                                                                  \
               : ((c == ARGS_N(5, __VA_ARGS__))                                                                                            \
                      ? (ARGS_N(6, __VA_ARGS__))                                                                                           \
                      : ((c == ARGS_N(7, __VA_ARGS__))                                                                                     \
                             ? (ARGS_N(8, __VA_ARGS__))                                                                                    \
                             : ((c == ARGS_N(9, __VA_ARGS__))                                                                              \
                                    ? (ARGS_N(10, __VA_ARGS__))                                                                            \
                                    : ((c == ARGS_N(11, __VA_ARGS__))                                                                      \
                                           ? (ARGS_N(12, __VA_ARGS__))                                                                     \
                                           : ((c == ARGS_N(13, __VA_ARGS__))                                                               \
                                                  ? (ARGS_N(14, __VA_ARGS__))                                                              \
                                                  : ((c == ARGS_N(15, __VA_ARGS__))                                                        \
                                                         ? (ARGS_N(16, __VA_ARGS__))                                                       \
                                                         : ((c == ARGS_N(17, __VA_ARGS__))                                                 \
                                                                ? (ARGS_N(18, __VA_ARGS__))                                                \
                                                                : ((c == ARGS_N(19, __VA_ARGS__)) ? (ARGS_N(20, __VA_ARGS__))              \
                                                                                                  : BS_MISMATCH)))))))))

#define BS_V22(c, ...)                                                                                                                     \
    (c == ARGS_N(1, __VA_ARGS__))                                                                                                          \
        ? (ARGS_N(2, __VA_ARGS__))                                                                                                         \
        : ((c == ARGS_N(3, __VA_ARGS__))                                                                                                   \
               ? (ARGS_N(4, __VA_ARGS__))                                                                                                  \
               : ((c == ARGS_N(5, __VA_ARGS__))                                                                                            \
                      ? (ARGS_N(6, __VA_ARGS__))                                                                                           \
                      : ((c == ARGS_N(7, __VA_ARGS__))                                                                                     \
                             ? (ARGS_N(8, __VA_ARGS__))                                                                                    \
                             : ((c == ARGS_N(9, __VA_ARGS__))                                                                              \
                                    ? (ARGS_N(10, __VA_ARGS__))                                                                            \
                                    : ((c == ARGS_N(11, __VA_ARGS__))                                                                      \
                                           ? (ARGS_N(12, __VA_ARGS__))                                                                     \
                                           : ((c == ARGS_N(13, __VA_ARGS__))                                                               \
                                                  ? (ARGS_N(14, __VA_ARGS__))                                                              \
                                                  : ((c == ARGS_N(15, __VA_ARGS__))                                                        \
                                                         ? (ARGS_N(16, __VA_ARGS__))                                                       \
                                                         : ((c == ARGS_N(17, __VA_ARGS__))                                                 \
                                                                ? (ARGS_N(18, __VA_ARGS__))                                                \
                                                                : ((c == ARGS_N(19, __VA_ARGS__))                                          \
                                                                       ? (ARGS_N(20, __VA_ARGS__))                                         \
                                                                       : ((c == ARGS_N(21, __VA_ARGS__)) ? (ARGS_N(22, __VA_ARGS__))       \
                                                                                                         : BS_MISMATCH))))))))))

#define BS_V24(c, ...)                                                                                                                     \
    (c == ARGS_N(1, __VA_ARGS__))                                                                                                          \
        ? (ARGS_N(2, __VA_ARGS__))                                                                                                         \
        : ((c == ARGS_N(3, __VA_ARGS__))                                                                                                   \
               ? (ARGS_N(4, __VA_ARGS__))                                                                                                  \
               : ((c == ARGS_N(5, __VA_ARGS__))                                                                                            \
                      ? (ARGS_N(6, __VA_ARGS__))                                                                                           \
                      : ((c == ARGS_N(7, __VA_ARGS__))                                                                                     \
                             ? (ARGS_N(8, __VA_ARGS__))                                                                                    \
                             : ((c == ARGS_N(9, __VA_ARGS__)) ? (ARGS_N(10, __VA_ARGS__))                                                  \
                                                              : ((c == ARGS_N(11, __VA_ARGS__))                                            \
                                                                     ? (ARGS_N(12, __VA_ARGS__))                                           \
                                                                     : ((c == ARGS_N(13, __VA_ARGS__))                                     \
                                                                            ? (ARGS_N(14, __VA_ARGS__))                                    \
                                                                            : ((c == ARGS_N(15, __VA_ARGS__))                              \
                                                                                   ? (ARGS_N(16, __VA_ARGS__))                             \
                                                                                   : ((c == ARGS_N(17, __VA_ARGS__))                       \
                                                                                          ? (ARGS_N(18, __VA_ARGS__))                      \
                                                                                          : ((c == ARGS_N(19, __VA_ARGS__))                \
                                                                                                 ? (ARGS_N(20, __VA_ARGS__))               \
                                                                                                 : ((c == ARGS_N(21, __VA_ARGS__))         \
                                                                                                        ? (ARGS_N(22, __VA_ARGS__))        \
                                                                                                        : ((c == ARGS_N(23, __VA_ARGS__))  \
                                                                                                               ? (ARGS_N(24, __VA_ARGS__)) \
                                                                                                               : BS_MISMATCH)))))))))))

#define BS_V26(c, ...)                                                                                                                     \
    (c == ARGS_N(1, __VA_ARGS__))                                                                                                          \
        ? (ARGS_N(2, __VA_ARGS__))                                                                                                         \
        : ((c == ARGS_N(3, __VA_ARGS__))                                                                                                   \
               ? (ARGS_N(4, __VA_ARGS__))                                                                                                  \
               : ((c == ARGS_N(5, __VA_ARGS__))                                                                                            \
                      ? (ARGS_N(6, __VA_ARGS__))                                                                                           \
                      : ((c == ARGS_N(7, __VA_ARGS__))                                                                                     \
                             ? (ARGS_N(8, __VA_ARGS__))                                                                                    \
                             : ((c == ARGS_N(9, __VA_ARGS__))                                                                              \
                                    ? (ARGS_N(10, __VA_ARGS__))                                                                            \
                                    : ((c == ARGS_N(11, __VA_ARGS__))                                                                      \
                                           ? (ARGS_N(12, __VA_ARGS__))                                                                     \
                                           : ((c == ARGS_N(13, __VA_ARGS__))                                                               \
                                                  ? (ARGS_N(14, __VA_ARGS__))                                                              \
                                                  : ((c == ARGS_N(15, __VA_ARGS__))                                                        \
                                                         ? (ARGS_N(16, __VA_ARGS__))                                                       \
                                                         : ((c == ARGS_N(17, __VA_ARGS__))                                                 \
                                                                ? (ARGS_N(18, __VA_ARGS__))                                                \
                                                                : ((c == ARGS_N(19, __VA_ARGS__))                                          \
                                                                       ? (ARGS_N(20, __VA_ARGS__))                                         \
                                                                       : ((c == ARGS_N(21, __VA_ARGS__))                                   \
                                                                              ? (ARGS_N(22, __VA_ARGS__))                                  \
                                                                              : ((c == ARGS_N(23, __VA_ARGS__))                            \
                                                                                     ? (ARGS_N(24, __VA_ARGS__))                           \
                                                                                     : ((c == ARGS_N(25, __VA_ARGS__))                     \
                                                                                            ? (ARGS_N(26, __VA_ARGS__))                    \
                                                                                            : BS_MISMATCH))))))))))))

#define BS_V28(c, ...)                                                                                                                     \
    (c == ARGS_N(1, __VA_ARGS__))                                                                                                          \
        ? (ARGS_N(2, __VA_ARGS__))                                                                                                         \
        : ((c == ARGS_N(3, __VA_ARGS__))                                                                                                   \
               ? (ARGS_N(4, __VA_ARGS__))                                                                                                  \
               : ((c == ARGS_N(5, __VA_ARGS__))                                                                                            \
                      ? (ARGS_N(6, __VA_ARGS__))                                                                                           \
                      : ((c == ARGS_N(7, __VA_ARGS__))                                                                                     \
                             ? (ARGS_N(8, __VA_ARGS__))                                                                                    \
                             : ((c == ARGS_N(9, __VA_ARGS__))                                                                              \
                                    ? (ARGS_N(10, __VA_ARGS__))                                                                            \
                                    : ((c == ARGS_N(11, __VA_ARGS__))                                                                      \
                                           ? (ARGS_N(12, __VA_ARGS__))                                                                     \
                                           : ((c == ARGS_N(13, __VA_ARGS__))                                                               \
                                                  ? (ARGS_N(14, __VA_ARGS__))                                                              \
                                                  : ((c == ARGS_N(15, __VA_ARGS__))                                                        \
                                                         ? (ARGS_N(16, __VA_ARGS__))                                                       \
                                                         : ((c == ARGS_N(17, __VA_ARGS__))                                                 \
                                                                ? (ARGS_N(18, __VA_ARGS__))                                                \
                                                                : ((c == ARGS_N(19, __VA_ARGS__))                                          \
                                                                       ? (ARGS_N(20, __VA_ARGS__))                                         \
                                                                       : ((c == ARGS_N(21, __VA_ARGS__))                                   \
                                                                              ? (ARGS_N(22, __VA_ARGS__))                                  \
                                                                              : ((c == ARGS_N(23, __VA_ARGS__))                            \
                                                                                     ? (ARGS_N(24, __VA_ARGS__))                           \
                                                                                     : ((c == ARGS_N(25, __VA_ARGS__))                     \
                                                                                            ? (ARGS_N(26, __VA_ARGS__))                    \
                                                                                            : ((c == ARGS_N(27, __VA_ARGS__))              \
                                                                                                   ? (ARGS_N(28, __VA_ARGS__))             \
                                                                                                   : BS_MISMATCH)))))))))))))

#define BS_V30(c, ...)                                                                                                                     \
    (c == ARGS_N(1, __VA_ARGS__))                                                                                                          \
        ? (ARGS_N(2, __VA_ARGS__))                                                                                                         \
        : ((c == ARGS_N(3, __VA_ARGS__))                                                                                                   \
               ? (ARGS_N(4, __VA_ARGS__))                                                                                                  \
               : ((c == ARGS_N(5, __VA_ARGS__))                                                                                            \
                      ? (ARGS_N(6, __VA_ARGS__))                                                                                           \
                      : ((c == ARGS_N(7, __VA_ARGS__))                                                                                     \
                             ? (ARGS_N(8, __VA_ARGS__))                                                                                    \
                             : ((c == ARGS_N(9, __VA_ARGS__))                                                                              \
                                    ? (ARGS_N(10, __VA_ARGS__))                                                                            \
                                    : ((c == ARGS_N(11, __VA_ARGS__))                                                                      \
                                           ? (ARGS_N(12, __VA_ARGS__))                                                                     \
                                           : ((c == ARGS_N(13, __VA_ARGS__))                                                               \
                                                  ? (ARGS_N(14, __VA_ARGS__))                                                              \
                                                  : ((c == ARGS_N(15, __VA_ARGS__))                                                        \
                                                         ? (ARGS_N(16, __VA_ARGS__))                                                       \
                                                         : ((c == ARGS_N(17, __VA_ARGS__))                                                 \
                                                                ? (ARGS_N(18, __VA_ARGS__))                                                \
                                                                : ((c == ARGS_N(19, __VA_ARGS__))                                          \
                                                                       ? (ARGS_N(20, __VA_ARGS__))                                         \
                                                                       : ((c == ARGS_N(21, __VA_ARGS__))                                   \
                                                                              ? (ARGS_N(22, __VA_ARGS__))                                  \
                                                                              : ((c == ARGS_N(23, __VA_ARGS__))                            \
                                                                                     ? (ARGS_N(24, __VA_ARGS__))                           \
                                                                                     : ((c == ARGS_N(25, __VA_ARGS__))                     \
                                                                                            ? (ARGS_N(26, __VA_ARGS__))                    \
                                                                                            : ((c == ARGS_N(27, __VA_ARGS__))              \
                                                                                                   ? (ARGS_N(28, __VA_ARGS__))             \
                                                                                                   : ((c == ARGS_N(29, __VA_ARGS__))       \
                                                                                                          ? (ARGS_N(30, __VA_ARGS__))      \
                                                                                                          : BS_MISMATCH))))))))))))))

#define BS_V32(c, ...)                                                                                                                     \
    (c == ARGS_N(1, __VA_ARGS__))                                                                                                          \
        ? (ARGS_N(2, __VA_ARGS__))                                                                                                         \
        : ((c == ARGS_N(3, __VA_ARGS__))                                                                                                   \
               ? (ARGS_N(4, __VA_ARGS__))                                                                                                  \
               : ((c == ARGS_N(5, __VA_ARGS__))                                                                                            \
                      ? (ARGS_N(6, __VA_ARGS__))                                                                                           \
                      : ((c == ARGS_N(7, __VA_ARGS__))                                                                                     \
                             ? (ARGS_N(8, __VA_ARGS__))                                                                                    \
                             : ((c == ARGS_N(9, __VA_ARGS__))                                                                              \
                                    ? (ARGS_N(10, __VA_ARGS__))                                                                            \
                                    : ((c == ARGS_N(11, __VA_ARGS__))                                                                      \
                                           ? (ARGS_N(12, __VA_ARGS__))                                                                     \
                                           : ((c == ARGS_N(13, __VA_ARGS__))                                                               \
                                                  ? (ARGS_N(14, __VA_ARGS__))                                                              \
                                                  : ((c == ARGS_N(15, __VA_ARGS__))                                                        \
                                                         ? (ARGS_N(16, __VA_ARGS__))                                                       \
                                                         : ((c == ARGS_N(17, __VA_ARGS__))                                                 \
                                                                ? (ARGS_N(18, __VA_ARGS__))                                                \
                                                                : ((c == ARGS_N(19, __VA_ARGS__))                                          \
                                                                       ? (ARGS_N(20, __VA_ARGS__))                                         \
                                                                       : ((c == ARGS_N(21, __VA_ARGS__))                                   \
                                                                              ? (ARGS_N(22, __VA_ARGS__))                                  \
                                                                              : ((c == ARGS_N(23, __VA_ARGS__))                            \
                                                                                     ? (ARGS_N(24, __VA_ARGS__))                           \
                                                                                     : ((c == ARGS_N(25, __VA_ARGS__))                     \
                                                                                            ? (ARGS_N(26, __VA_ARGS__))                    \
                                                                                            : ((c == ARGS_N(27, __VA_ARGS__))              \
                                                                                                   ? (ARGS_N(28, __VA_ARGS__))             \
                                                                                                   : ((c == ARGS_N(29, __VA_ARGS__))       \
                                                                                                          ? (ARGS_N(30, __VA_ARGS__))      \
                                                                                                          : ((c ==                         \
                                                                                                              ARGS_N(31, __VA_ARGS__))     \
                                                                                                                 ? (ARGS_N(32,             \
                                                                                                                           __VA_ARGS__))   \
                                                                                                                 : BS_MISMATCH)))))))))))))))

#define BSV              BS_V
#define BSV_3(pre, post) MAGIC(pre, post)
#define BSV_2(pre, post) CMV_3(pre, post)
#define BS(m)            CMV_2(BSV, m)
#define BS_MAP(c, ...)   BS(ARGS_NUM(__VA_ARGS__))(c, __VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /* _TYPEDEF_H_ */
