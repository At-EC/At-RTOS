include_directories(${KERNEL_PATH}/include)

target_sources(atos_kernel
	PUBLIC
    ${KERNEL_PATH}/include/clock/k_clock_tick.h

	PRIVATE
	${CMAKE_CURRENT_LIST_DIR}/k_clock_native_gcc.c
)

target_include_directories(atos_kernel
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)


