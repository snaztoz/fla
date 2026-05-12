add_library(flac
    ast.hpp
    compiler.cpp
    compiler.hpp
    lexer.cpp
    lexer.hpp
    parser.cpp
    parser.hpp
    token.hpp
)

if(MSVC)
    target_compile_options(flac PRIVATE /W4)
else()
    target_compile_options(flac PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_include_directories(flac PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Tests

add_executable(flac_parser_tests parser_tests.cpp)

target_compile_definitions(flac_parser_tests PRIVATE BUILD_TYPE="$<CONFIG>")

target_link_libraries(flac_parser_tests PRIVATE flac)

# Copy tests/ into build/ directory
add_custom_command(TARGET flac_parser_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/tests"
            "$<TARGET_FILE_DIR:flac_parser_tests>/tests"
)

add_test(NAME ParserTests COMMAND flac_parser_tests)
