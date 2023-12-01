add_library(glad INTERFACE)

target_sources(glad INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/src/glad.c
)

target_include_directories(glad INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/include
)