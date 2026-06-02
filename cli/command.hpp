#ifndef FLA_CLI_COMMAND_H
#define FLA_CLI_COMMAND_H

#include <string>

#include "CLI11/CLI11.hpp"

namespace fla::cli
{
    class Command
    {
    public:
        bool should_run() const
        {
            return command->parsed();
        }

        virtual int run() const = 0;

    protected:
        CLI::App *command;
    };

    class CompileCommand : public Command
    {
    public:
        explicit CompileCommand(CLI::App &base);

        int run() const override;

    private:
        std::string src_path;
    };

    class VersionCommand : public Command
    {
    public:
        explicit VersionCommand(CLI::App &base);

        int run() const override;
    };
} // namespace fla::cli

#endif
