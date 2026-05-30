#ifndef FLA_COMPILER_PARSER_H
#define FLA_COMPILER_PARSER_H

#include <cstddef>
#include <expected>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>

#include "ast.hpp"
#include "error.hpp"
#include "lexer.hpp"
#include "token.hpp"

namespace fla::compiler
{
    using ParseResult = std::expected<Node, Error>;
    using ParseChildrenResult = std::expected<std::vector<Node>, Error>;

    class Parser
    {
    public:
        Parser(const std::string_view src);

        ParseResult parse();

    private:
        Lexer lexer;
        const std::string_view src;

        ParseResult parse_root(const Token &t);
        ParseResult parse_namespace_statement();
        ParseResult parse_use_statement();
        ParseResult parse_function_definition();
        ParseChildrenResult parse_function_parameters();
        ParseResult parse_function_return_type_notation();
        ParseChildrenResult parse_nested_names();
        ParseChildrenResult parse_body();
        ParseResult parse_type_notation();
        ParseResult parse_expression_statement();
        ParseResult parse_expression();
        ParseResult parse_assignment();
        ParseResult parse_logical_and_or_expression();
        ParseResult parse_equality_expression();
        ParseResult parse_comparison_expression();
        ParseResult parse_additive_expression();
        ParseResult parse_multiplicative_expression();
        ParseResult parse_unary_expression();
        ParseResult parse_primary_expression();
        std::expected<Token, Error> expect(const TokenType &expected_tt);
    };

    template <typename T>
    concept Positionable = std::same_as<T, Node> || std::same_as<T, Token>;

    template <Positionable P1, Positionable P2>
    constexpr std::size_t len_between(const P1 &a, const P2 &b) noexcept
    {
        return b.pos - a.pos + b.len;
    }
} // namespace fla::compiler

#endif
