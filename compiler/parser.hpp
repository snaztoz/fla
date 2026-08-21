#ifndef FLA_COMPILER_PARSER_H
#define FLA_COMPILER_PARSER_H

#include <expected>
#include <set>
#include <string_view>

#include "ast.hpp"
#include "error.hpp"
#include "lexer.hpp"
#include "token.hpp"

namespace fla::compiler
{
    using BodyParseResult = std::expected<std::vector<Node>, Error>;
    using NameParseResult = std::expected<Name, Error>;
    using ParseResult = std::expected<Node, Error>;
    using TypeNotationNodeParseResult = std::expected<TypeNotationNode, Error>;

    struct ParserContext {
        Lexer lexer;
        const std::string_view src;

        ParserContext(const std::string_view s) : lexer(s), src(s)
        {
        }
    };

    ParseResult parse(std::string_view src);
    BodyParseResult parse_body(ParserContext &ctx, std::set<TokenType> end_delimiters);
    TypeNotationNodeParseResult parse_type_notation_node(ParserContext &ctx);
    ParseResult parse_expression(ParserContext &ctx);
    NameParseResult parse_name(ParserContext &ctx);
    std::expected<Token, Error> expect(ParserContext &ctx, const TokenType &expected_tt);
} // namespace fla::compiler

#endif
