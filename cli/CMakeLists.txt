add_executable(fla
    compiler.cpp
    compiler.hpp
    main.cpp
)

if(MSVC)
    target_compile_options(fla PRIVATE /W4)
else()
    target_compile_options(fla PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_include_directories(fla PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(fla PRIVATE ${CMAKE_BINARY_DIR}/include)
target_include_directories(fla SYSTEM PRIVATE ${CMAKE_SOURCE_DIR}/vendor)

target_link_libraries(fla PRIVATE flac)
