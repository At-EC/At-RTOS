include_directories(${KERNAL_PATH}/include)

target_sources(atos_kernal
	PUBLIC
    ${KERNAL_PATH}/include/port.h

    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/port_native_gcc.c
)

target_include_directories(atos_kernal
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)
