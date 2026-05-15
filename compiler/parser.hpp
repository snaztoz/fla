#ifndef FLA_PARSER_H
#define FLA_PARSER_H

#include <cstddef>
#include <expected>
#include <memory>
#include <string>
#include <string_view>

#include "ast.hpp"
#include "lexer.hpp"

namespace fla::compiler
{
    using ParseResult = std::expected<Node, std::string>;
    using ParseChildrenResult = std::expected<std::vector<Node>, std::string>;

    class Parser
    {
    public:
        Parser(const std::string_view src);

        ParseResult parse();

    private:
        Lexer lexer;
        const std::string_view src;

        ParseResult parse_root(const TokenType &type);
        ParseResult parse_namespace_statement();
        ParseResult parse_use_statement();
        ParseResult parse_function_definition();

        ParseChildrenResult parse_nested_names();
    };
} // namespace fla::compiler

#endif
