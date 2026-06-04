add_library(flac
    ../include/fla/compiler.h
    ast.hpp
    compiler.cpp
    compiler.hpp
    error.hpp
    lexer.cpp
    lexer.hpp
    parser_expression.cpp
    parser.cpp
    parser.hpp
    token.hpp
)

if(MSVC)
    target_compile_options(flac PRIVATE /W4)
else()
    target_compile_options(flac PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_include_directories(flac PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(flac PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Tests

add_executable(flac_parser_tests parser_tests.cpp)

target_compile_definitions(flac_parser_tests PRIVATE BUILD_TYPE="$<CONFIG>")

target_link_libraries(flac_parser_tests PRIVATE flac)

target_include_directories(flac_parser_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Copy tests/ into build/ directory
add_custom_command(TARGET flac_parser_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/tests"
            "${CMAKE_CURRENT_BINARY_DIR}/tests"
)

add_test(NAME ParserTests COMMAND flac_parser_tests)
