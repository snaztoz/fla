#include <format>
#include <print>
#include <string>

#include "CLI11/CLI11.hpp"

#include "command.hpp"
#include "fla/compiler.h"
#include "fla/version.h"

namespace fla::cli
{
    CompileCommand::CompileCommand(CLI::App &base)
    {
        command = base.add_subcommand("compile", "Run the compiler.");
        command->add_option("path", entrypoint, "Source code file path")->required();
    }

    int CompileCommand::run() const
    {
        FlaCompilerError err {};

        const auto res { fla_compile(entrypoint.c_str(), &err) };
        if (res != 0) {
            std::println("error: {}", err.msg);
        }

        fla_free_compiler_error(&err);

        return res;
    }

    VersionCommand::VersionCommand(CLI::App &base)
    {
        command = base.add_subcommand("version", "Display version.");
    }

    int VersionCommand::run() const
    {
        std::println("Fla v{}.{}.{}", FLA_VERSION_MAJOR, FLA_VERSION_MINOR, FLA_VERSION_PATCH);
        return 0;
    }
} // namespace fla::cli
