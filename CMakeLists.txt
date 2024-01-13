cmake_minimum_required(VERSION 3.20)

add_library(atos_kernal STATIC)

include_directories(atos_kernal PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_sources(atos_kernal
	PRIVATE
	${CMAKE_CURRENT_LIST_DIR}/idle.c
)

include(${CMAKE_CURRENT_LIST_DIR}/kernal/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/clock/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/port/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/arch/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/include/CMakeLists.txt)

target_include_directories(atos_kernal
    PRIVATE
	${CMAKE_CURRENT_LIST_DIR}
)
