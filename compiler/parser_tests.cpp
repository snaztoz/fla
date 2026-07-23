#include <cassert>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <print>
#include <sstream>
#include <string>

#include "parser.hpp"

std::string read_fixture(std::filesystem::path path)
{
    std::ifstream file(path.make_preferred());

    if (!file.is_open()) {
        std::println(stderr, "failed, could not open {}", path.generic_string());
        std::exit(1);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    file.close();

    return buffer.str();
}

#define TEST_PARSE(name, fixture_path)                                                             \
    do {                                                                                           \
        std::print(stderr, "test {} parsing...", name);                                            \
        const auto fixture { read_fixture(fixture_path) };                                         \
        fla::compiler::ParserContext parser_ctx { fla::compiler::Lexer { fixture }, fixture };     \
        if (const auto res = fla::compiler::parse(parser_ctx); !res) {                             \
            std::println(stderr, "failed, {} ({}:{})", res.error().line, res.error().col,          \
                         res.error().msg);                                                         \
            std::exit(1);                                                                          \
        }                                                                                          \
        std::println(stderr, "ok");                                                                \
    } while (0);

int main()
{
    const std::filesystem::path path { "tests/parser" };

    TEST_PARSE("expression", path / "expression.fla");
    TEST_PARSE("function", path / "function.fla");
    TEST_PARSE("if_else", path / "if_else.fla");
    TEST_PARSE("namespace", path / "namespace.fla");
    TEST_PARSE("use", path / "use.fla");
    TEST_PARSE("variable", path / "variable.fla");

    std::println(stderr, "all tests passed!");

    return 0;
}
