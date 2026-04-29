#include <string_view>

#include "compiler.hpp"

const std::string_view src = R"(
namespace foo.bar

use std.math
)";

int main()
{
    orchid::compiler::compile(src);

    return 0;
}
