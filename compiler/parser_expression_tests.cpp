#include "Catch2/catch_amalgamated.hpp"

#include "parser_tests.hpp"

TEST_CASE("parse-expression", "[parser]")
{
    SECTION("simple")
    {
        REQUIRE(is_parseable(R"(
            fun main() do
                5
            end
        )"));
    };

    SECTION("identifier")
    {
        REQUIRE(is_parseable(R"(
            fun main() do
                foo
            end
        )"));
    };

    SECTION("grouped")
    {
        REQUIRE(is_parseable(R"(
            fun main() do
                (((bar)))
            end
        )"));
    };

    SECTION("complex")
    {
        REQUIRE(is_parseable(R"(
            fun main() do
                foo = 3 + 10 * (abc - 7 / -2) * 5 >= 0 == true != not not false or false and 1 > 5
            end
        )"));
    };
}
