#include <string_view>

#include "Catch2/catch_amalgamated.hpp"

#include "parser.hpp"

bool is_parseable(std::string_view src)
{
    fla::compiler::ParserContext parser_ctx { fla::compiler::Lexer { src }, src };
    return !!fla::compiler::parse(parser_ctx);
}

TEST_CASE("parsing expression", "parse-expression")
{
    SECTION("simple expression")
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

    SECTION("grouped expression")
    {
        REQUIRE(is_parseable(R"(
            fun main() do
                (((bar)))
            end
        )"));
    };

    SECTION("complex expression")
    {
        REQUIRE(is_parseable(R"(
            fun main() do
                foo = 3 + 10 * (abc - 7 / -2) * 5 >= 0 == true != not not false or false and 1 > 5
            end
        )"));
    };
}

TEST_CASE("parsing function", "parse-function")
{
    SECTION("empty function")
    {
        REQUIRE(is_parseable(R"(
            fun main() do end
        )"));
    }

    SECTION("function with single parameter")
    {
        REQUIRE(is_parseable(R"(
            fun main(s string) do
            end
        )"));
    }

    SECTION("function with multiple parameters")
    {
        REQUIRE(is_parseable(R"(
            fun main(x int, y int, z int,) do
            end
        )"));
    }

    SECTION("function with return type")
    {
        REQUIRE(is_parseable(R"(
            fun main() string do
            end
        )"));
    }

    SECTION("function with body")
    {
        REQUIRE(is_parseable(R"(
            fun main() do
                foo
            end
        )"));
    }
}

TEST_CASE("parsing if-else", "parse-if-else")
{
    SECTION("basic if")
    {
        REQUIRE(is_parseable(R"(
            fun main() do
                if 1 + 1 == 5 do
                    5
                end
            end
        )"));
    }

    SECTION("basic if else")
    {
        REQUIRE(is_parseable(R"(
            fun main() do
                if foo do
                else do
                end
            end
        )"));
    }

    SECTION("multiple if else branches")
    {
        REQUIRE(is_parseable(R"(
            fun main() do
                if foo do
                    50
                else if 2 > 5 and bar do
                    100
                else do
                    150
                end
            end
        )"));
    }

    SECTION("if else expression")
    {
        REQUIRE(is_parseable(R"(
            fun main() do
                var dynamic = if a == 5 do
                    true
                else do
                    false
                end
            end
        )"));
    }
}

TEST_CASE("parsing while-loop", "parse-while-loop")
{
    SECTION("basic while-loop")
    {
        REQUIRE(is_parseable(R"(
            fun main() do
                while not true do
                    5
                end
            end
        )"));
    }
}

TEST_CASE("parsing namespace", "parse-namespace")
{
    SECTION("namespace statement")
    {
        REQUIRE(is_parseable(R"(
            namespace std.util.math

            namespace std
                .util
                .math

            namespace
            std
            .util
            .math

            namespace   std
                . util
            . math
        )"));
    }
}

TEST_CASE("parsing use", "parse-use")
{
    SECTION("use statement")
    {
        REQUIRE(is_parseable(R"(
            use std.util.math

            use std
                .util
                .math

            use
            std
            .util
            .math

            use   std
                . util
            . math
        )"));
    }
}

TEST_CASE("parsing variable", "parse-variable")
{
    SECTION("variable assignment")
    {
        REQUIRE(is_parseable(R"(
            fun main() do
                var foo = 10 + 5
            end
        )"));
    }

    SECTION("constant assignment")
    {
        REQUIRE(is_parseable(R"(
            fun main() do
                const foo = 10 + 5
            end
        )"));
    }

    SECTION("variable & constant assignment with type notation")
    {
        REQUIRE(is_parseable(R"(
            fun main() do
                var abc int = 100
                const def int = 100
            end
        )"));
    }
}
