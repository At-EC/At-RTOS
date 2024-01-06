cmake_minimum_required(VERSION 3.20)

add_library(at_rtos_kernel STATIC)

target_sources(at_rtos_kernal 
	PUBLIC
	${CMKAE_CURRENT_LIST_DIR}/at_rtos.h
	)

include(${CMAKE_CURRENT_LIST_DIR}/kernal/CMakeLists.txt)
