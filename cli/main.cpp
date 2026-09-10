#include "CLI11/CLI11.hpp"

#include "command.hpp"

int main(int argc, char **argv)
{
    CLI::App app { "Fla is a programming language that focuses on developer experience, "
                   " performance, and user-safeness."
                   "\n\n"
                   "Fla is released under MIT License. Complete documentation can be found at"
                   " https://github.com/snaztoz/fla" };

    using namespace fla::cli::command;

    compile::register_command(app);
    version::register_command(app);

    CLI11_PARSE(app, argc, argv);

    if (compile::should_run()) {
        return compile::run();
    }
    if (version::should_run()) {
        return version::run();
    }

    return 0;
}
