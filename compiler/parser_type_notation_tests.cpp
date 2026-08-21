#include "Catch2/catch_amalgamated.hpp"

#include "parser_tests.hpp"

TEST_CASE("parse-type-notation", "[parser]")
{
    SECTION("array-type")
    {
        REQUIRE(is_parseable(R"(
            fun foo(x []int) do end
        )"));
    }

    SECTION("function-type")
    {
        REQUIRE(is_parseable(R"(
            fun foo(x fun(int) void) do end
        )"));
    }
}
