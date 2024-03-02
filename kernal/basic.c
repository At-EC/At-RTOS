/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 **/

#include "typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Copy the character from src to dst.
 *
 * @param dst The pointer of the destination.
 * @param src The pointer of the source.
 * @param cnt The opereation specific length.
 */
void _memcpy(void *dst, const void *src, u32_t cnt)
{
    uchar_t *d = (uchar_t *)dst;
    const uchar_t *s = (const uchar_t *)src;
    while (cnt--) {
        *d++ = *s++;
    }
}

/**
 * @brief Set the character val to dst.
 *
 * @param dst The pointer of the destination.
 * @param val The character value.
 * @param cnt The opereation specific length.
 */
void _memset(void *dst, u8_t val, u32_t cnt)
{
    uchar_t *d = (uchar_t *)dst;
    while (cnt--) {
        *d++ = (u8_t)val;
    }
}

/**
 * @brief Compare the two character.
 *
 * @param dst The pointer of the destination.
 * @param src The pointer of the source.
 * @param cnt The opereation specific length.
 *
 * @return The value 0 indicates the both strings are same, otherwise is different
 */
i32_t _memcmp(const void *dst, const void *src, u32_t cnt)
{
    const uchar_t *d = (const uchar_t *)dst, *s = (const uchar_t *)src;
    int r = 0;
    while (cnt-- && (r = *d++ - *s++) == 0)
        ;
    return r;
}

/**
 * @brief Calaculate the string length.
 *
 * @param str The pointer of the string.
 *
 * @return The value of the string length
 */
u32_t _strlen(const uchar_t *str)
{
    u32_t len = 0u;
    while (*str++ != '\0') {
        len++;
    }
    return len;
}

#ifdef __cplusplus
}
#endif
