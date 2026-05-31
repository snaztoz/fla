#include <print>

#include "CLI11/CLI11.hpp"

#include "command.hpp"

int main(int argc, char **argv)
{
    CLI::App app { "Fla is a programming language that focuses on developer experience, "
                   " performance, and user-safeness."
                   "\n\n"
                   "Fla is released under MIT License. Complete documentation can be found at"
                   " https://github.com/snaztoz/fla" };

    const fla::cli::CompileCommand compile { app };
    const fla::cli::VersionCommand version { app };

    CLI11_PARSE(app, argc, argv);

    if (compile.should_run()) {
        return compile.run();
    } else if (version.should_run()) {
        return version.run();
    }

    return 0;
}
