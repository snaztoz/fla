#ifndef FLA_COMPILER_PARSER_H
#define FLA_COMPILER_PARSER_H

#include <expected>
#include <string_view>

#include "ast.hpp"
#include "error.hpp"
#include "lexer.hpp"

namespace fla::compiler
{
    using ParseResult = std::expected<Node, Error>;

    struct ParserContext {
        Lexer lexer;
        const std::string_view src;

        ParserContext(const std::string_view s) : lexer(s), src(s)
        {
        }
    };

    ParseResult parse(std::string_view src);
} // namespace fla::compiler

#endif
