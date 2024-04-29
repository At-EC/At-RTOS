/**
 * Copyright (c) Riven Zheng (zhengheiot@gmail.com).
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 **/

#include "at_rtos.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Local defined the kernel thread stack and error postcode */
#define _PC_CMPT_FAILED                 PC_FAILED(PC_CMPT_KERNEL)
#define SAMPLE_THREAD_STACK_SIZE        (1024u)

OS_THREAD_DEFINE(sample_thread, SAMPLE_THREAD_STACK_SIZE, 5);

static os_thread_id_t g_sample_thread_id;

/*
 * @brief The kernel sample entry function.
 **/
static void sample_entry_thread(void)
{
    while(1) {
        /* Put the current thread into sleep state */
        os.thread_sleep(1000);
    }
}

int main(void)
{
    g_sample_thread_id = os.thread_init(sample_thread, sample_entry_thread);

    if (os.id_isInvalid(g_sample_thread_id)) {
       /* return _PC_CMPT_FAILED; */
    }

    /* At_RTOS kernel running starts */
    os.schedule_run();
    D_ASSERT(0);

    while(1) {};
}

#ifdef __cplusplus
}
#endif
