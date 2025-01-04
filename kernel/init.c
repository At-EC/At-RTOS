/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 **/
#include "init.h"

void init_func_level(u8_t level)
{
    INIT_SECTION_FOREACH(INIT_SECTION_FUNC, init_func_t, ifun)
    {
        if (ifun->level == level && ifun) {
            ifun->func();
        }
    }
}

void init_func_list(void)
{
    for (u8_t lv = INIT_LEVEL_0; lv < INIT_LEVEL_NUM; lv++) {
        init_func_level(lv);
    }
}

void init_static_thread_list(void)
{
    extern void _impl_thread_static_init(thread_context_t * pCurThread);

    INIT_SECTION_FOREACH(INIT_SECTION_OS_THREAD_STATIC, thread_context_init_t, pInit)
    {
        if (pInit->pThread) {
            _impl_thread_static_init(pInit->pThread);
        }
    }
}
