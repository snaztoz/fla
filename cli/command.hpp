#ifndef FLA_CLI_COMMAND_H
#define FLA_CLI_COMMAND_H

#include <string>

#include "CLI11/CLI11.hpp"

namespace fla::cli
{
    class CompileCommand
    {
    public:
        CLI::App *command;

        CompileCommand(CLI::App &base);

        int run() const;

    private:
        std::string src_path;
    };

    class VersionCommand
    {
    public:
        CLI::App *command;

        VersionCommand(CLI::App &base);

        int run() const;
    };
} // namespace fla::cli

#endif
