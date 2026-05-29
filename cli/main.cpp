#include <print>

#include "CLI11/CLI11.hpp"

#include "compiler.hpp"
#include "fla/version.h"

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

    const auto compiler = app.add_subcommand("compile", "Run the compiler.");

    std::string compiler_path;
    compiler->add_option("path", compiler_path, "Source code file path")->required();

    const auto version = app.add_subcommand("version", "Display version.");

    CLI11_PARSE(app, argc, argv);

    if (*compiler) {
        return fla::cli::run_compiler(compiler_path);
    } else if (*version) {
        return run_version();
    }

    return 0;
}
