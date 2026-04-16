cmake_minimum_required(VERSION 3.18...3.24)

project(Orchid
    VERSION 1.0
    DESCRIPTION "A (yet another) programming language"
    LANGUAGES C
)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_library(orchidc
    compiler.c
    compiler.h
    lexer.c
    lexer.h
    token.h
)

add_executable(orchid main.c)
target_link_libraries(orchid PRIVATE orchidc)
