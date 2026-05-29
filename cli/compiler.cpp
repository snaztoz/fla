#include <expected>
#include <format>
#include <fstream>
#include <print>
#include <sstream>
#include <string>

#include "compiler.hpp"
#include "fla/compiler.h"

namespace fla::cli
{
    std::expected<std::string, std::string> read_content()
    {
        const auto path { "./temp/main.fla" };

        std::ifstream file(path);
        if (!file.is_open()) {
            return std::unexpected(std::format("unable to open {}", path));
        }

        std::stringstream buffer;
        buffer << file.rdbuf();

        return buffer.str();
    }

    int run_compiler()
    {
        const auto src { read_content() };
        if (!src) {
            std::println("error: {}", src.error());
            return 1;
        }

        FlaCompilerError err {};

        const auto res { fla_compile(src->c_str(), &err) };
        if (res != 0) {
            std::println("{}", err.msg);
        }

        fla_free_compiler_error(&err);

        return res;
    }
} // namespace fla::cli
