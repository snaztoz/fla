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

if(MSVC)
    target_compile_options(orchidc PRIVATE /W4)
else()
    target_compile_options(orchidc PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_include_directories(orchidc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


# Tests

add_executable(orchidc_parser_tests parser_tests.cpp)

target_compile_definitions(orchidc_parser_tests PRIVATE BUILD_TYPE="$<CONFIG>")

target_link_libraries(orchidc_parser_tests PRIVATE orchidc)

# Copy tests/ into build/ directory
add_custom_command(TARGET orchidc_parser_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/tests"
            "$<TARGET_FILE_DIR:orchidc_parser_tests>/tests"
)

add_test(NAME ParserTests COMMAND orchidc_parser_tests)
