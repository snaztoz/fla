cmake_minimum_required(VERSION 3.28)

project(Fla
    VERSION 1.0
    DESCRIPTION "A (yet another) programming language"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

enable_testing()

add_subdirectory(compiler)

add_executable(fla main.cpp)

if(MSVC)
    target_compile_options(fla PRIVATE /W4)
else()
    target_compile_options(fla PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_link_libraries(fla PRIVATE flac)
