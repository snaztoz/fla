add_library(flac
    ../include/fla/compiler.h
    ast.cpp
    ast.hpp
    compiler.cpp
    compiler.hpp
    context.hpp
    error.hpp
    lexer.cpp
    lexer.hpp
    parser.cpp
    parser.hpp
    token.hpp
    type_check.cpp
    type_check.hpp
    util.cpp
    util.hpp
)

if(MSVC)
    target_compile_options(flac PRIVATE /W4)
else()
    target_compile_options(flac PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_include_directories(flac PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(flac PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})


# Tests

add_executable(flac-tests
    ${CMAKE_SOURCE_DIR}/vendor/Catch2/catch_amalgamated.cpp
    parser_tests.cpp
)

target_compile_definitions(flac-tests PRIVATE BUILD_TYPE="$<CONFIG>")

target_link_libraries(flac-tests PRIVATE flac)

target_include_directories(flac-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(flac-tests SYSTEM PRIVATE ${CMAKE_SOURCE_DIR}/vendor)

add_test(NAME CompilerTests COMMAND flac-tests)
