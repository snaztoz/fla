#include <print>

#include "CLI11/CLI11.hpp"

#include "fla/compiler.h"
#include "fla/version.h"

const char *src = R"(namespace foo.bar

use std.math

fun helper() string do
end

fun main(argc int,) do
  1045
  bar
  (((baz)))
  foo = bar = 5 + 123 * (10 - 4 / -5) * 2 <= 0 == true != not not false and true
end
)";

int run_compiler()
{
    FlaCompilerError err {};

    const auto res { fla_compile(src, &err) };
    if (res != 0) {
        std::println("{}", err.msg);
    }

    fla_free_compiler_error(&err);

    return res;
}

int run_version()
{
    std::println("Fla v{}.{}.{}", FLA_VERSION_MAJOR, FLA_VERSION_MINOR, FLA_VERSION_PATCH);
    return 0;
}

int main(int argc, char **argv)
{
    CLI::App app {
        "Fla is a programming language that focuses on developer experience, "
        " performance, and safety."
        "\n\n"
        "Fla is released under MIT License. Complete documentation can be found at ..."
    };

    const auto compiler = app.add_subcommand("compile", "Run the compiler");
    const auto version = app.add_subcommand("version", "Display version");

    CLI11_PARSE(app, argc, argv);

    if (*compiler) {
        return run_compiler();
    } else if (*version) {
        return run_version();
    }

    return 0;
}
