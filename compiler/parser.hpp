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
    using ParseBodyResult = std::expected<std::vector<Node>, Error>;
    using ParseNameResult = std::expected<Name, Error>;
    using ParseResult = std::expected<Node, Error>;
    using ParseTypeNotationResult = std::expected<TypeNotationNode, Error>;

    struct ParserContext {
        Lexer lexer;
        const std::string_view src;
    };

    ParseResult parse(ParserContext &ctx);
    ParseBodyResult parse_body(ParserContext &ctx, std::set<TokenType> end_delimiters);
    ParseTypeNotationResult parse_type_notation_node(ParserContext &ctx);
    ParseResult parse_expression(ParserContext &ctx);
    ParseNameResult parse_name(ParserContext &ctx);
    std::expected<Token, Error> expect(ParserContext &ctx, const TokenType &expected_tt);
} // namespace fla::compiler

#endif
