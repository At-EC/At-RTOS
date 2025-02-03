add_library(kernel_include INTERFACE)

include_directories(${KERNEL_PATH}/include)
include_directories(${KERNEL_PATH}/include/arch)
include_directories(${KERNEL_PATH}/include/clock)
include_directories(${KERNEL_PATH}/include/port)

target_sources(kernel_include
	PUBLIC
	${KERNEL_PATH}/include/arch/arch.h
	${KERNEL_PATH}/include/clock/clock_tick.h
	${KERNEL_PATH}/include/port/port.h
	
    ${KERNEL_PATH}/include/at_rtos.h
    ${KERNEL_PATH}/include/compiler.h
    ${KERNEL_PATH}/include/configuration.h
    ${KERNEL_PATH}/include/kernel.h
    ${KERNEL_PATH}/include/linker.h
    ${KERNEL_PATH}/include/kstruct.h
    ${KERNEL_PATH}/include/postcode.h
    ${KERNEL_PATH}/include/timer.h
    ${KERNEL_PATH}/include/trace.h
    ${KERNEL_PATH}/include/type_def.h
    ${KERNEL_PATH}/include/ktype.h
	${KERNEL_PATH}/include/init.h
)
