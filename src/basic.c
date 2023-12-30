/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "basic.h"

#ifdef __cplusplus
extern "C" {
#endif

void _memcpy(char_t *dst, const char_t *src, u32_t cnt)
{
    char_t *d = (char_t *)dst;
    const char_t *s = (const char_t *)src;
    while (cnt--)
    {
        *d++ = *s++;
    }
}

void _memset(char_t *dst, u8_t val, u32_t cnt)
{
    char_t *d = (char_t *)dst;
    while (cnt--)
    {
        *d++ = (u8_t)val;
    }
}

i32_t _memcmp(const char_t *dst, const char_t *src, u32_t cnt)
{
    const char_t *d = (const char_t *)dst, *s = (const char_t *)src;
    int r = 0;
    while (cnt-- && (r = *d++ - *s++) == 0);
    return r;
}

i32_t _memstr(const char_t *str, u32_t chr)
{
    while (*str && *str != chr)
    {
        str++;
    }
    return *str;
}

u32_t _strlen(const char_t *str)
{
    u32_t len = 0u;
    while (*str++ != '\0')
    {
        len++;
    }
    return len;
}

#ifdef __cplusplus
}
#endif
