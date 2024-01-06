target_sources(at_rtos_kernal 
	PRIVATE 
	${CMKAE_CURRENT_LIST_DIR}/port/port_gcc.c
	${CMKAE_CURRENT_LIST_DIR}/kernal.c
	${CMKAE_CURRENT_LIST_DIR}/trace.c
	${CMKAE_CURRENT_LIST_DIR}/timer.c
	${CMKAE_CURRENT_LIST_DIR}/thread.c	
	${CMKAE_CURRENT_LIST_DIR}/semaphore.c
	${CMKAE_CURRENT_LIST_DIR}/queue.c
	${CMKAE_CURRENT_LIST_DIR}/mutex.c
	${CMKAE_CURRENT_LIST_DIR}/list.c
	${CMKAE_CURRENT_LIST_DIR}/linker.c
	${CMKAE_CURRENT_LIST_DIR}/event.c
	${CMKAE_CURRENT_LIST_DIR}/clock_systick.c
	${CMKAE_CURRENT_LIST_DIR}/basic.c
	
	PUBLIC
	${CMKAE_CURRENT_LIST_DIR}/include/arch32/arch.h
	${CMKAE_CURRENT_LIST_DIR}/include/basic.h
	${CMKAE_CURRENT_LIST_DIR}/include/clock_systick.h
	${CMKAE_CURRENT_LIST_DIR}/include/compiler.h
	${CMKAE_CURRENT_LIST_DIR}/include/event.h
	${CMKAE_CURRENT_LIST_DIR}/include/kernal.h
	${CMKAE_CURRENT_LIST_DIR}/include/linker.h
	${CMKAE_CURRENT_LIST_DIR}/include/list.h
	${CMKAE_CURRENT_LIST_DIR}/include/member_struct.h
	${CMKAE_CURRENT_LIST_DIR}/include/mutex.h
	${CMKAE_CURRENT_LIST_DIR}/include/postcode.h
	${CMKAE_CURRENT_LIST_DIR}/include/queue.h
	${CMKAE_CURRENT_LIST_DIR}/include/semaphore.h
	${CMKAE_CURRENT_LIST_DIR}/include/svc_call.h
	${CMKAE_CURRENT_LIST_DIR}/include/thread.h
	${CMKAE_CURRENT_LIST_DIR}/include/timer.h
	${CMKAE_CURRENT_LIST_DIR}/include/trace.h
	${CMKAE_CURRENT_LIST_DIR}/include/type.h
	)

include_directories(at_rtos_kernal, PRIVATE ${CMKAE_CURRENT_SOURCE_DIR}/include)
include_directories(at_rtos_kernal, PRIVATE ${CMKAE_CURRENT_SOURCE_DIR}/include/arch32)
include_directories(at_rtos_kernal, PRIVATE ${CMKAE_CURRENT_SOURCE_DIR}/port)

target_include_directories(at_rtos_kernal PRIVATE ${CMKAE_CURRENT_LIST_DIR})
