include_directories(atos_kernal PRIVATE ${CMAKE_CURRENT_LIST_DIR}/kernal_sample)

target_sources(atos_kernal
	PUBLIC
	${CMAKE_CURRENT_LIST_DIR}/kernal_sample/atos_configuration.h
)

target_include_directories(atos_kernal
    INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)
