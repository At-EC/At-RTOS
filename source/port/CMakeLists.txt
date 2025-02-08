include_directories(${KERNEL_PATH}/include)

target_sources(atos_kernel
	PUBLIC
    ${KERNEL_PATH}/include/port/k_port.h

    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/k_port_native_gcc.c
)

target_include_directories(atos_kernel
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)
