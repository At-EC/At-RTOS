target_sources(atos_kernel
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/sync_event.c
    ${CMAKE_CURRENT_LIST_DIR}/sync_mutex.c
    ${CMAKE_CURRENT_LIST_DIR}/sync_semaphore.c
    ${CMAKE_CURRENT_LIST_DIR}/msg_queue.c
    ${CMAKE_CURRENT_LIST_DIR}/msg_subscribe.c
    ${CMAKE_CURRENT_LIST_DIR}/mem_pool.c
    ${CMAKE_CURRENT_LIST_DIR}/sched_kernel.c
    ${CMAKE_CURRENT_LIST_DIR}/sched_thread.c
    ${CMAKE_CURRENT_LIST_DIR}/sched_timer.c
    ${CMAKE_CURRENT_LIST_DIR}/k_linker.c
    ${CMAKE_CURRENT_LIST_DIR}/k_trace.c
    ${CMAKE_CURRENT_LIST_DIR}/k_thread.c
    ${CMAKE_CURRENT_LIST_DIR}/static_init.c
)

target_include_directories(atos_kernel
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(atos_kernel kernel_include)
