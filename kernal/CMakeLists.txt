add_library(kernel_impl INTERFACE)

target_sources(kernel_impl 
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
	
	${CMAKE_CURRENT_LIST_DIR}/port/port_gcc.c
	
	PUBLIC
	${CMAKE_CURRENT_LIST_DIR}/include/basic.h
	${CMAKE_CURRENT_LIST_DIR}/include/clock_systick.h
	${CMAKE_CURRENT_LIST_DIR}/include/compiler.h
	${CMAKE_CURRENT_LIST_DIR}/include/event.h
	${CMAKE_CURRENT_LIST_DIR}/include/kernal.h
	${CMAKE_CURRENT_LIST_DIR}/include/linker.h
	${CMAKE_CURRENT_LIST_DIR}/include/list.h
	${CMAKE_CURRENT_LIST_DIR}/include/member_struct.h
	${CMAKE_CURRENT_LIST_DIR}/include/mutex.h
	${CMAKE_CURRENT_LIST_DIR}/include/postcode.h
	${CMAKE_CURRENT_LIST_DIR}/include/queue.h
	${CMAKE_CURRENT_LIST_DIR}/include/semaphore.h
	${CMAKE_CURRENT_LIST_DIR}/include/svc_call.h
	${CMAKE_CURRENT_LIST_DIR}/include/thread.h
	${CMAKE_CURRENT_LIST_DIR}/include/timer.h
	${CMAKE_CURRENT_LIST_DIR}/include/trace.h
	${CMAKE_CURRENT_LIST_DIR}/include/type.h
	
	${CMAKE_CURRENT_LIST_DIR}/include/arch32/arch.h
)

target_include_directories(kernel_impl
    INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
	${CMAKE_CURRENT_LIST_DIR}/include
	${CMAKE_CURRENT_LIST_DIR}/include/arch32
	${CMAKE_CURRENT_LIST_DIR}/port
)

target_link_libraries(kernel_impl
    INTERFACE
    $<$<TARGET_EXISTS:atos_config>:atos_config>
)

