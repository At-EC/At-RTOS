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

/* Local defined the kernal thread stack and error postcode */
#define _PC_CMPT_FAILED                 PC_FAILED(PC_CMPT_KERNAL)
#define SAMPLE_THREAD_STACK_SIZE        (1024u)

ATOS_STACK_DEFINE(g_sample_thread_stack, SAMPLE_THREAD_STACK_SIZE);
static os_thread_id_t g_sample_thread_id = {0u};

/*
 * @brief The kernal sample entry function.
 **/
static void sample_entry_thread(void)
{
    while(1)
    {
        /* Put the current thread into sleep state */
        AtOS.thread_sleep(1000);
    }
}

int main(void)
{
    g_sample_thread_id = AtOS.thread_init(sample_entry_thread,       /* The thread entry function  */
                                          g_sample_thread_stack,     /* The address of thread stack  */
                                          SAMPLE_THREAD_STACK_SIZE,  /* The size of thread stack */
                                          0xFEu,                     /* The priority of the thread: [FE, 1] */
                                          "sample");                 /* The name of the thread */

    if (AtOS.os_id_is_invalid(g_sample_thread_id))
    {
       return _PC_CMPT_FAILED;
    }
    
    /* At_RTOS kernal running starts */
    AtOS.kernal_atos_run();
    D_ASSERT(0);
    
    while(1) {};
}

#ifdef __cplusplus
}
#endif
