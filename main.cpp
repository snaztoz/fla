#include <string_view>

#include "compiler.hpp"

const std::string_view src = R"(
namespace foo.bar

use std.math

fun main(argc int,) do
end
)";

int main()
{
    fla::compiler::compile(src);

    return 0;
}
