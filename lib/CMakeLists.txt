# https://www.glfw.org/docs/3.3/build_guide.html#build_link_cmake_source
# https://github.com/juliettef/GLFW-CMake-starter/blob/main/CMakeLists.txt
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

add_subdirectory(glad)
add_subdirectory(glfw)