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
#define CMAME_TEST_STACK_SIZE       (1024u)
ATOS_STACK_DEFINE(g_cmake_test_stack, CMAME_TEST_STACK_SIZE);

/*
 * @brief It's cmake pure test thread entry fucntion.
 **/
void cmake_test_thread(void)
{
    while (1)
    {
		/* Nothing to do */
        AtOS.thread_sleep(1000);
    }
}

int main(void)
{
    os_thread_it_t cmake_thread_id = {OS_INVALID_ID};
	
	cmake_thread_id = AtOS.thread_init(cmake_test_thread,
                                       g_cmake_test_stack,
                                       CMAME_TEST_STACK_SIZE,
                                       5u,
                                       "cmake_thread");

    if (AtOS.os_id_is_invalid(cmake_thread_id))
    {
        D_ASSERT(0);
    }

    AtOS.kernal_atos_run();
    while (1)
	{
		
	}
}

#ifdef __cplusplus
}
#endif
