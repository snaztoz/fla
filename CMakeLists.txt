cmake_minimum_required(VERSION 3.28)

project(Fla
    VERSION 0.1.0
    DESCRIPTION "A (yet another) programming language"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

configure_file(
    "${PROJECT_SOURCE_DIR}/include/fla/version.h.in"
    "${PROJECT_BINARY_DIR}/include/fla/version.h"
)

enable_testing()

add_subdirectory(cli)
add_subdirectory(compiler)
