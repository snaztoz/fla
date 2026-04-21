cmake_minimum_required(VERSION 3.18...3.24)

project(Orchid
    VERSION 1.0
    DESCRIPTION "A (yet another) programming language"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(orchidc
    ast.hpp
    compiler.cpp
    compiler.hpp
    lexer.cpp
    lexer.hpp
    parser.cpp
    parser.hpp
    token.hpp
)

add_executable(orchid main.cpp)
target_link_libraries(orchid PRIVATE orchidc)
