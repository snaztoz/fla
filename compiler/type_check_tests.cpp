#include <string_view>

#include "Catch2/catch_amalgamated.hpp"

#include "parser.hpp"
#include "type_check.hpp"

bool is_passing_type_check(const std::string_view src)
{
    fla::compiler::parser::Context parser_ctx { src };

    const auto root { fla::compiler::parser::parse(parser_ctx) };
    if (!root) {
        std::unreachable();
    }

    return !!fla::compiler::type_check::read_declarations(parser_ctx.arena, *root);
}

TEST_CASE("check-namespace", "[type-check]")
{
    SECTION("namespace-declaration")
    {
        REQUIRE(is_passing_type_check(R"(
            namespace std.kernel.type
        )"));
    }

    SECTION("no-namespace-declaration")
    {
        REQUIRE(!is_passing_type_check(""));
    }

    SECTION("namespace-declaration-not-as-first-statement")
    {
        REQUIRE(!is_passing_type_check(R"(
            use std.kernel.type

            namespace std.kernel.io
        )"));
    }
}

TEST_CASE("check-declarations", "[type-check]")
{
    SECTION("normal-declarations")
    {
        REQUIRE(is_passing_type_check(R"(
            namespace std.kernel.type

            class Person do
            end

            interface Greeter do
            end
        )"));
    }

    SECTION("deferred-declarations")
    {
        REQUIRE(is_passing_type_check(R"(
            namespace std.kernel.type

            defer class Person

            class Person do
            end
        )"));
    }

    SECTION("deferred-declarations-in-public-scope")
    {
        REQUIRE(is_passing_type_check(R"(
            namespace std.kernel.type

            public do
                defer class Person
            end

            class Person do
            end
        )"));
    }

    SECTION("missing-definition-for-deferred-type")
    {
        REQUIRE(!is_passing_type_check(R"(
            namespace std.kernel.type

            defer class Person
        )"));
    }

    SECTION("define-before-declare")
    {
        REQUIRE(is_passing_type_check(R"(
            namespace std.kernel.type

            class Person do
            end

            public do
                defer class Person
            end
        )"));
    }

    SECTION("declare-name-multiple-times")
    {
        REQUIRE(!is_passing_type_check(R"(
            namespace std.kernel.type

            defer class Person

            defer class Person
        )"));
    }

    SECTION("define-name-multiple-times")
    {
        REQUIRE(!is_passing_type_check(R"(
            namespace std.kernel.type

            interface Person do
            end

            class Person do
            end
        )"));
    }
}
