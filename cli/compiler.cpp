#include <print>

#include "compiler.hpp"
#include "fla/compiler.h"

namespace fla::cli
{
    const char *src = R"(namespace foo.bar

    use std.math

    fun helper() string do
    end

    fun main(argc int,) do
    1045
    bar
    (((baz)))
    foo = bar = 5 + 123 * (10 - 4 / -5) * 2 <= 0 == true != not not false and true
    end
    )";

    int run_compiler()
    {
        FlaCompilerError err {};

        const auto res { fla_compile(src, &err) };
        if (res != 0) {
            std::println("{}", err.msg);
        }

        fla_free_compiler_error(&err);

        return res;
    }
} // namespace fla::cli
