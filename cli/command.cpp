#include <expected>
#include <format>
#include <fstream>
#include <print>
#include <sstream>
#include <string>

#include "CLI11/CLI11.hpp"

#include "command.hpp"
#include "fla/compiler.h"
#include "fla/version.h"

namespace fla::cli
{
    std::expected<std::string, std::string> read_content(const std::string &path)
    {
        std::ifstream file(path);
        if (!file.is_open()) {
            return std::unexpected(std::format("unable to open {}", path));
        }

        std::stringstream buffer;
        buffer << file.rdbuf();

        return buffer.str();
    }

    CompileCommand::CompileCommand(CLI::App &base)
    {
        command = base.add_subcommand("compile", "Run the compiler.");
        command->add_option("path", src_path, "Source code file path")->required();
    }

    int CompileCommand::run() const
    {
        const auto src { read_content(src_path) };
        if (!src) {
            std::println("error: {}", src.error());
            return 1;
        }

        FlaCompilerError err {};

        const auto res { fla_compile(src->c_str(), &err) };
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
