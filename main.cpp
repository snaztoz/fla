#include "fla/compiler.h"

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

int main()
{
    fla_compile(src);

    return 0;
}
