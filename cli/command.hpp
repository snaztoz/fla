#ifndef FLA_CLI_COMMAND_H
#define FLA_CLI_COMMAND_H

#include "CLI11/CLI11.hpp"

namespace fla::cli::command::compile
{
    void register_command(CLI::App &base);
    bool should_run();
    int run();
} // namespace fla::cli::command::compile

namespace fla::cli::command::version
{
    void register_command(CLI::App &base);
    bool should_run();
    int run();
} // namespace fla::cli::command::version

#endif
