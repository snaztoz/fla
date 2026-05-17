#include <cassert>
#include <cstdio>
#include <filesystem>

#if defined(_WIN32) && defined(_MSC_VER)

#include <format>

// Avoid editor displaying error
#ifndef BUILD_TYPE
#define BUILD_TYPE ""
#endif

#endif

#include <fstream>
#include <print>
#include <sstream>
#include <string>

#include "parser.hpp"

std::string read_fixture(std::filesystem::path path)
{
    std::ifstream file(path.make_preferred());

    if (!file.is_open()) {
        std::println(stderr, "failed (could not open {})",
                     path.generic_string());
        std::exit(1);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    file.close();

    return buffer.str();
}

#define TEST_PARSE(name, fixture_path)                                         \
    do {                                                                       \
        std::print(stderr, "test {} parsing...", name);                        \
        auto fixture { read_fixture(fixture_path) };                           \
        fla::compiler::Parser parser { fixture };                              \
        if (auto res = parser.parse(); !res) {                                 \
            std::println(stderr, "failed ({})", res.error());                  \
            std::exit(1);                                                      \
        }                                                                      \
        std::println(stderr, "ok");                                            \
    } while (0);

int main()
{
#if defined(_WIN32) && defined(_MSC_VER)
    std::filesystem::path path { std::format("{}/tests/parser", BUILD_TYPE) };
#else
    std::filesystem::path path { "tests/parser" };
#endif

    TEST_PARSE("function", path / "function.fla");
    TEST_PARSE("namespace", path / "namespace.fla");
    TEST_PARSE("use", path / "use.fla");

    std::println(stderr, "all tests passed!");

    return 0;
}
