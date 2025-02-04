cmake_minimum_required(VERSION 3.20)

project(build_native_gcc LANGUAGES C)

set(At_RTOS_PATH "../../../")
set(ATOS_CONFIG_FILE_PATH "${At_RTOS_PATH}/include/template/native_gcc")

add_library(atos_config INTERFACE)

target_include_directories(atos_config
    INTERFACE
    ${ATOS_CONFIG_FILE_PATH}
)

target_sources(atos_config
	PUBLIC
    ${ATOS_CONFIG_FILE_PATH}/os_config.h
)

include(${At_RTOS_PATH}/CMakeLists.txt)

aux_source_directory(. DIR_SRCS)

add_executable(${PROJECT_NAME} ${DIR_SRCS})

target_compile_options(${PROJECT_NAME} PRIVATE
    ### Gnu/Clang C Options
    $<$<COMPILE_LANG_AND_ID:C,GNU>:-fdiagnostics-color=always>
    $<$<COMPILE_LANG_AND_ID:C,Clang>:-fcolor-diagnostics>

    $<$<COMPILE_LANG_AND_ID:C,Clang,GNU>:-Wall>
    $<$<COMPILE_LANG_AND_ID:C,Clang,GNU>:-Wextra>
    $<$<COMPILE_LANG_AND_ID:C,Clang,GNU>:-Wpedantic>
    $<$<COMPILE_LANG_AND_ID:C,Clang,GNU>:-Werror>
    # $<$<COMPILE_LANG_AND_ID:C,Clang,GNU>:-Wconversion>
    $<$<COMPILE_LANG_AND_ID:C,Clang>:-Weverything>
    # $<$<COMPILE_LANG_AND_ID:C,Clang>:-Wsign-conversion>
	
    # Suppressions required to build clean with clang.
    $<$<COMPILE_LANG_AND_ID:C,Clang>:-Wno-unused-macros>
    $<$<COMPILE_LANG_AND_ID:C,Clang>:-Wno-padded>
    $<$<COMPILE_LANG_AND_ID:C,Clang>:-Wno-missing-variable-declarations>
    $<$<COMPILE_LANG_AND_ID:C,Clang>:-Wno-covered-switch-default>
    $<$<COMPILE_LANG_AND_ID:C,Clang>:-Wno-format-truncation>
    $<$<COMPILE_LANG_AND_ID:C,Clang>:-Wno-int-to-pointer-cast>
    $<$<COMPILE_LANG_AND_ID:C,Clang>:-Wno-pointer-to-int-cast>
    $<$<COMPILE_LANG_AND_ID:C,Clang>:-Wno-sign-conversion>
    $<$<COMPILE_LANG_AND_ID:C,Clang>:-Wno-cast-align> )

target_link_libraries(${PROJECT_NAME} atos_kernel)
