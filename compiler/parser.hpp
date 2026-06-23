#ifndef FLA_COMPILER_PARSER_H
#define FLA_COMPILER_PARSER_H

#include <expected>
#include <string_view>

#include "ast.hpp"
#include "error.hpp"
#include "lexer.hpp"
#include "token.hpp"

namespace fla::compiler
{
    using ParseResult = std::expected<Node, Error>;

    struct ParserContext {
        Lexer lexer;
        const std::string_view src;
    };

    ParseResult parse(ParserContext &ctx);
    ParseResult parse_expression(ParserContext &ctx);
    std::expected<Token, Error> expect(ParserContext &ctx, const TokenType &expected_tt);
} // namespace fla::compiler

#endif
