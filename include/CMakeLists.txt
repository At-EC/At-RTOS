add_library(kernel_include INTERFACE)

include_directories(${KERNEL_PATH}/include)
include_directories(${KERNEL_PATH}/include/kernel)

target_sources(kernel_include
	PUBLIC
	${KERNEL_PATH}/include/arch.h
	
    ${KERNEL_PATH}/include/kernel/at_rtos.h
    ${KERNEL_PATH}/include/kernel/compiler.h
    ${KERNEL_PATH}/include/kernel/configuration.h
    ${KERNEL_PATH}/include/kernel/kernel.h
    ${KERNEL_PATH}/include/kernel/linker.h
    ${KERNEL_PATH}/include/kernel/kstruct.h
    ${KERNEL_PATH}/include/kernel/postcode.h
    ${KERNEL_PATH}/include/kernel/timer.h
    ${KERNEL_PATH}/include/kernel/trace.h
    ${KERNEL_PATH}/include/kernel/typedef.h
    ${KERNEL_PATH}/include/kernel/ktype.h
)
