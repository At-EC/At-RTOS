target_sources(atos_kernel
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/event.c
    ${CMAKE_CURRENT_LIST_DIR}/kernel.c
    ${CMAKE_CURRENT_LIST_DIR}/linker.c
    ${CMAKE_CURRENT_LIST_DIR}/mutex.c
    ${CMAKE_CURRENT_LIST_DIR}/queue.c
    ${CMAKE_CURRENT_LIST_DIR}/semaphore.c
    ${CMAKE_CURRENT_LIST_DIR}/thread.c
    ${CMAKE_CURRENT_LIST_DIR}/timer.c   
    ${CMAKE_CURRENT_LIST_DIR}/trace.c
    ${CMAKE_CURRENT_LIST_DIR}/kthread.c
    ${CMAKE_CURRENT_LIST_DIR}/postcode.c
    ${CMAKE_CURRENT_LIST_DIR}/pool.c
)

target_include_directories(atos_kernel
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(atos_kernel kernel_include)
