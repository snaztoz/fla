#include <format>
#include <print>
#include <string>

#include "CLI11/CLI11.hpp"

#include "command.hpp"
#include "fla/compiler.h"
#include "fla/version.h"

namespace fla::cli::command::compile
{
    CLI::App *command;

    std::string std_path;
    std::string pkg_name;
    std::string entrypoint;
    std::vector<std::string> src_dirs;

    void setup_src_dirs();

    void register_command(CLI::App &base)
    {
        std_path = "./std";

        command = base.add_subcommand("compile", "Run the compiler.");
        command->add_option("path", entrypoint, "Source code file path")->required();
        command->add_flag("--std", std_path, "Path to std package");
        command->add_flag("--pkg", pkg_name, "Package name");
        command->add_flag("--src", src_dirs, "Compilation source directories");
    }

    bool should_run()
    {
        return command->parsed();
    }

    int run()
    {
        setup_src_dirs();

        FlaCompilerError err {};

        const auto res { fla_compile(entrypoint.c_str(), std_path.c_str(), &err) };
        if (res != 0) {
            std::println("error: {}", err.msg);
        }

        fla_free_compiler_error(&err);

        return res;
    }

    void setup_src_dirs()
    {
        if (src_dirs.empty()) {
            src_dirs.push_back(".");
        }
    }
} // namespace fla::cli::command::compile

namespace fla::cli::command::version
{
    CLI::App *command;

    void register_command(CLI::App &base)
    {
        command = base.add_subcommand("version", "Display version.");
    }

    bool should_run()
    {
        return command->parsed();
    }

    int run()
    {
        std::println("Fla v{}.{}.{}, built at {}", FLA_VERSION_MAJOR, FLA_VERSION_MINOR,
                     FLA_VERSION_PATCH, BUILD_TIMESTAMP);
        return 0;
    }
} // namespace fla::cli::command::version
