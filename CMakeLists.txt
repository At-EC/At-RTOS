cmake_minimum_required(VERSION 3.20)

add_library(rtos_kernel STATIC)

add_subdirectory(kernal)

target_sources(rtos_kernel 
	PRIVATE
	${CMAKE_CURRENT_LIST_DIR}/clock_systick.c
	
	PUBLIC
	${CMAKE_CURRENT_LIST_DIR}/at_rtos.h
	${CMAKE_CURRENT_LIST_DIR}/include/rtos_configuration.h
)

target_include_directories(rtos_kernel
    INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
	${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(rtos_kernel
    PUBLIC
    kernel_impl
)
