#ifndef FLA_COMPILER_PARSER_H
#define FLA_COMPILER_PARSER_H

#include <expected>
#include <string_view>

#include "ast.hpp"
#include "error.hpp"
#include "lexer.hpp"

namespace fla::compiler::parser
{
    using Result = std::expected<ast::NodeIndex, Error>;

    struct Context {
        Lexer lexer;
        ast::Arena arena;
        const std::string_view src;

        Context(const std::string_view s) : lexer(s), arena(), src(s)
        {
        }
    };

    const Result parse(Context &ctx);
} // namespace fla::compiler::parser

#endif
