cmake_minimum_required(3.14.0)
project(libsgl_proj)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
add_executable(libsgl, src/graphics.c)
