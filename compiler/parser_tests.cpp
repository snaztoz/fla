#include "Catch2/catch_amalgamated.hpp"

#include "parser_tests.hpp"

TEST_CASE("parse-public", "[parser]")
{
    SECTION("simple")
    {
        REQUIRE(is_parseable(R"(
            public do
            end
        )"));
    }

    SECTION("with-statements")
    {
        REQUIRE(is_parseable(R"(
            public do
                class Person do
                end

                fun greet() do
                end
            end
        )"));
    }

    SECTION("nested")
    {
        REQUIRE(is_parseable(R"(
            class Person do
                public do
                    fun greet() do
                    end
                end
            end
        )"));
    }
}

TEST_CASE("parse-forward-declaration", "[parser]")
{
    SECTION("class")
    {
        REQUIRE(is_parseable(R"(
            declare class Person
        )"));
    }

    SECTION("function")
    {
        REQUIRE(is_parseable(R"(
            declare fun greet() string
        )"));
    }

    SECTION("function-with-skipped-parameters")
    {
        REQUIRE(is_parseable(R"(
            declare fun greet string
        )"));
    }

    SECTION("nested-inside-scope")
    {
        REQUIRE(is_parseable(R"(
            public do
                declare class Person
                declare fun greet(name string) string
            end
        )"));
    }
}

TEST_CASE("parse-class", "[parser]")
{
    SECTION("simple")
    {
        REQUIRE(is_parseable(R"(
            class Person do
            end
        )"));
    }

    SECTION("variable-member")
    {
        REQUIRE(is_parseable(R"(
            class Person do
                var name string
            end
        )"));
    }

    SECTION("method-member")
    {
        REQUIRE(is_parseable(R"(
            class Person do
                fun greet() do
                end
            end
        )"));
    }
}

TEST_CASE("parse-function", "[parser]")
{
    SECTION("empty")
    {
        REQUIRE(is_parseable(R"(
            fun main() do end
        )"));
    }

    SECTION("skip-parentheses-on-empty-parameter")
    {
        REQUIRE(is_parseable(R"(
            fun main do end
        )"));
    }

    SECTION("single-parameter")
    {
        REQUIRE(is_parseable(R"(
            fun main(s string) do
            end
        )"));
    }

    SECTION("multiple-parameters")
    {
        REQUIRE(is_parseable(R"(
            fun main(x int, y int, z int,) do
            end
        )"));
    }

    SECTION("return-type")
    {
        REQUIRE(is_parseable(R"(
            fun main() string do
            end
        )"));
    }

    SECTION("body")
    {
        REQUIRE(is_parseable(R"(
            fun main() do
                foo
            end
        )"));
    }
}

TEST_CASE("parse-if-else", "[parser]")
{
    SECTION("basic")
    {
        REQUIRE(is_parseable(R"(
            fun main() do
                if 1 + 1 == 5 do
                    5
                end
            end
        )"));
    }

    SECTION("if-else")
    {
        REQUIRE(is_parseable(R"(
            fun main() do
                if foo do
                else do
                end
            end
        )"));
    }

    SECTION("multiple-branches")
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

    SECTION("expression")
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

TEST_CASE("parse-interface", "[parser]")
{
    SECTION("simple")
    {
        REQUIRE(is_parseable(R"(
            interface Person do
            end
        )"));
    }

    SECTION("method-member")
    {
        REQUIRE(is_parseable(R"(
            class Person do
                declare fun greet() string
            end
        )"));
    }
}

TEST_CASE("parse-while-loop", "[parser]")
{
    SECTION("basic")
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

TEST_CASE("parse-namespace", "[parser]")
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

TEST_CASE("parse-use", "[parser]")
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

TEST_CASE("parse-variable", "[parser]")
{
    SECTION("variable-assignment")
    {
        REQUIRE(is_parseable(R"(
            fun main() do
                var foo = 10 + 5
            end
        )"));
    }

    SECTION("constant-assignment")
    {
        REQUIRE(is_parseable(R"(
            fun main() do
                const foo = 10 + 5
            end
        )"));
    }

    SECTION("type-notation")
    {
        REQUIRE(is_parseable(R"(
            fun main() do
                var abc int = 100
                const def int = 100
            end
        )"));
    }
}
