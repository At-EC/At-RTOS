include_directories(atos_kernal PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include)

target_sources(atos_kernal
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/port_sample.c
)

target_include_directories(atos_kernal
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)
