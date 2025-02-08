add_library(kernel_include INTERFACE)

include_directories(${KERNEL_PATH}/include)
include_directories(${KERNEL_PATH}/include/arch)
include_directories(${KERNEL_PATH}/include/clock)
include_directories(${KERNEL_PATH}/include/port)

target_sources(kernel_include
	PUBLIC
	${KERNEL_PATH}/include/arch/k_arch.h
	${KERNEL_PATH}/include/clock/k_clock_tick.h
	${KERNEL_PATH}/include/port/k_port.h
	
    ${KERNEL_PATH}/include/at_rtos.h
    ${KERNEL_PATH}/include/k_config.h
    ${KERNEL_PATH}/include/sched_kernel.h
    ${KERNEL_PATH}/include/k_linker.h
    ${KERNEL_PATH}/include/k_struct.h
    ${KERNEL_PATH}/include/postcode.h
    ${KERNEL_PATH}/include/sched_timer.h
    ${KERNEL_PATH}/include/k_trace.h
    ${KERNEL_PATH}/include/type_def.h
    ${KERNEL_PATH}/include/k_type.h
	${KERNEL_PATH}/include/static_init.h
)
