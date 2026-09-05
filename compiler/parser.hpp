#ifndef FLA_COMPILER_PARSER_H
#define FLA_COMPILER_PARSER_H

#include <chrono>
#include <expected>
#include <string_view>

#include "ast.hpp"
#include "error.hpp"
#include "lexer.hpp"

namespace fla::compiler::parser
{
    using Result = std::expected<ast::NodeIndex, error::Error>;

    struct Context {
        lexer::Lexer lexer;
        ast::Arena arena;
        const std::string_view src;

        std::chrono::time_point<std::chrono::steady_clock> start;
        std::chrono::time_point<std::chrono::steady_clock> end;

        Context(const std::string_view s) : lexer(s), arena(), src(s)
        {
        }
    };

    const Result parse(Context &ctx);
} // namespace fla::compiler::parser

#endif
