target_sources(atos_kernal
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/basic.c
    ${CMAKE_CURRENT_LIST_DIR}/event.c
    ${CMAKE_CURRENT_LIST_DIR}/kernal.c
    ${CMAKE_CURRENT_LIST_DIR}/linker.c
    ${CMAKE_CURRENT_LIST_DIR}/list.c
    ${CMAKE_CURRENT_LIST_DIR}/mutex.c
    ${CMAKE_CURRENT_LIST_DIR}/queue.c
    ${CMAKE_CURRENT_LIST_DIR}/semaphore.c
    ${CMAKE_CURRENT_LIST_DIR}/thread.c
    ${CMAKE_CURRENT_LIST_DIR}/timer.c   
    ${CMAKE_CURRENT_LIST_DIR}/trace.c
    ${CMAKE_CURRENT_LIST_DIR}/kthread.c
    ${CMAKE_CURRENT_LIST_DIR}/postcode.c
)

target_include_directories(atos_kernal
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(atos_kernal kernal_include)
