# Copyright (c) Riven Zheng (zhengheiot@gmail.com).
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

cmake_minimum_required(VERSION 3.20)

project(pure_cmake_test)

set(AT_RTOS_KERNEL_PATH "../../")
# Adding the FreeRTOS-Kernel subdirectory
add_subdirectory(${AT_RTOS_KERNEL_PATH} at_rtos_kernel)

# Add the at_rtos_configuration for at_rtos_kernel
add_library(at_rtos_configuration INTERFACE)
target_include_directories(at_rtos_configuration INTERFACE "../../include")

aux_source_directory(. DIR_SRCS)
add_executable(${PROJECT_NAME} ${DIR_SRCS})

target_link_libraries(${PROJECT_NAME} at_rtos_kernel at_rtos_configuration)
