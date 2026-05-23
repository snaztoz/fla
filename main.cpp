#include <string_view>

#include "compiler.hpp"

const std::string_view src = R"(namespace foo.bar

use std.math

fun main(argc int,) do
  1045
  bar
  (((baz)))
  foo = bar = 5 + 123 * (10 - 4 / 5) * 2 <= 0 == true != false and true
end
)";

int main()
{
    fla::compiler::compile(src);

    return 0;
}
