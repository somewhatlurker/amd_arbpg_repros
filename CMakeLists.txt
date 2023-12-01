cmake_minimum_required(VERSION 3.12)
project(amd_arbpg_repros C)
set(CMAKE_C_STANDARD 11)

if(MSVC)
        add_compile_options(/W4 /experimental:external /external:anglebrackets /external:W3)
else()
        add_compile_options(-Wall)
endif()

add_subdirectory(lib)
add_subdirectory(texunit_limit)