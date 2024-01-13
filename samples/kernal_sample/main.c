/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "at_rtos.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Local defined the kernal thread stack */
#define _PC_CMPT_FAILED                 PC_FAILED(PC_CMPT_KERNAL)
#define SAMPLE_THREAD_STACK_SIZE        (1024u)

ATOS_STACK_DEFINE(g_sample_thread_stack, SAMPLE_THREAD_STACK_SIZE);
static os_thread_id_t g_sample_thread_id = {0u};

/*
 * @brief It's cmake pure test thread entry fucntion.
 **/
void sample_entry_thread(void)
{
    while (1)
    {
        /* Nothing to do */
        AtOS.thread_sleep(1000);
    }
}

int main(void)
{
    g_sample_thread_id = AtOS.thread_init(sample_entry_thread,
                                          g_sample_thread_stack,
                                          SAMPLE_THREAD_STACK_SIZE,
                                          0xFFu,
                                          "sample");

    if (AtOS.os_id_is_invalid(g_sample_thread_id))
    {
       return _PC_CMPT_FAILED;
    }
    
    AtOS.kernal_atos_run();
    D_ASSERT(0);
    
    while(1) {};
}

#ifdef __cplusplus
}
#endif
