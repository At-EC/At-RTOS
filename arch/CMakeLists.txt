include_directories(atos_kernal PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_sources(atos_kernal
	PUBLIC
	${CMAKE_CURRENT_LIST_DIR}/arch.h
)

target_include_directories(atos_kernal
    INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)
