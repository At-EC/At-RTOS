add_library(kernal_include INTERFACE)

include_directories(${KERNAL_PATH}/include)
include_directories(${KERNAL_PATH}/include/kernal)

target_sources(kernal_include
	PUBLIC
	${KERNAL_PATH}/include/arch.h
	
    ${KERNAL_PATH}/include/kernal/at_rtos.h
    ${KERNAL_PATH}/include/kernal/compiler.h
    ${KERNAL_PATH}/include/kernal/configuration.h
    ${KERNAL_PATH}/include/kernal/event.h
    ${KERNAL_PATH}/include/kernal/kernal.h
    ${KERNAL_PATH}/include/kernal/linker.h
    ${KERNAL_PATH}/include/kernal/list.h
    ${KERNAL_PATH}/include/kernal/kstruct.h
    ${KERNAL_PATH}/include/kernal/mutex.h
    ${KERNAL_PATH}/include/kernal/postcode.h
    ${KERNAL_PATH}/include/kernal/queue.h
    ${KERNAL_PATH}/include/kernal/semaphore.h
    ${KERNAL_PATH}/include/kernal/thread.h
    ${KERNAL_PATH}/include/kernal/timer.h
    ${KERNAL_PATH}/include/kernal/trace.h
    ${KERNAL_PATH}/include/kernal/typedef.h
    ${KERNAL_PATH}/include/kernal/unique.h
    ${KERNAL_PATH}/include/kernal/kthread.h
    ${KERNAL_PATH}/include/kernal/pool.h
)
