include_directories(${KERNEL_PATH}/include)

target_sources(atos_kernel
	PUBLIC
    ${KERNEL_PATH}/include/port/port.h

    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/port_native_gcc.c
)

target_include_directories(atos_kernel
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)
