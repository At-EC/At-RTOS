cmake_minimum_required(VERSION 3.20)

add_library(atos_kernal STATIC)

set(KERNAL_PATH ${CMAKE_CURRENT_LIST_DIR})

include_directories(${CMAKE_CURRENT_LIST_DIR})

target_sources(atos_kernal
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/build_version.h
)

include(${CMAKE_CURRENT_LIST_DIR}/include/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/kernal/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/clock/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/port/CMakeLists.txt)

if(NOT TARGET atos_config)
    message(FATAL_ERROR " atos_config target not specified. Please specify a cmake target that defines the include path for atos_configuration.h\n")
endif()

target_link_libraries(atos_kernal atos_config)

