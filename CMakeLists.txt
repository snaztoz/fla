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

add_subdirectory(compiler)

add_executable(fla main.cpp)

if(MSVC)
    target_compile_options(fla PRIVATE /W4)
else()
    target_compile_options(fla PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_include_directories(fla PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/include)
target_include_directories(fla PRIVATE ${CMAKE_SOURCE_DIR}/vendor)

target_link_libraries(fla PRIVATE flac)
