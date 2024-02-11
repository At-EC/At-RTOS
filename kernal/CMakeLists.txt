include_directories(atos_kernal PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include)

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
    ${CMAKE_CURRENT_LIST_DIR}/kernal_thread.c
	
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include/at_rtos.h
    ${CMAKE_CURRENT_LIST_DIR}/include/basic.h
    ${CMAKE_CURRENT_LIST_DIR}/include/compiler.h
    ${CMAKE_CURRENT_LIST_DIR}/include/configuration.h
    ${CMAKE_CURRENT_LIST_DIR}/include/event.h
    ${CMAKE_CURRENT_LIST_DIR}/include/kernal.h
    ${CMAKE_CURRENT_LIST_DIR}/include/linker.h
    ${CMAKE_CURRENT_LIST_DIR}/include/list.h
    ${CMAKE_CURRENT_LIST_DIR}/include/member_struct.h
    ${CMAKE_CURRENT_LIST_DIR}/include/mutex.h
    ${CMAKE_CURRENT_LIST_DIR}/include/postcode.h
    ${CMAKE_CURRENT_LIST_DIR}/include/queue.h
    ${CMAKE_CURRENT_LIST_DIR}/include/semaphore.h
    ${CMAKE_CURRENT_LIST_DIR}/include/thread.h
    ${CMAKE_CURRENT_LIST_DIR}/include/timer.h
    ${CMAKE_CURRENT_LIST_DIR}/include/trace.h
    ${CMAKE_CURRENT_LIST_DIR}/include/type.h
    ${CMAKE_CURRENT_LIST_DIR}/include/unique.h
    ${CMAKE_CURRENT_LIST_DIR}/include/kernal_thread.h
)

target_include_directories(atos_kernal
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

