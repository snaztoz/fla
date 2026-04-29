#ifndef ORCHID_PARSER_H
#define ORCHID_PARSER_H

#include <cstddef>
#include <expected>
#include <memory>
#include <string>
#include <string_view>

#include "ast.hpp"
#include "lexer.hpp"

namespace orchid::compiler
{
    using ParseResult = std::expected<std::size_t, std::string>;
    using ParseChildrenResult = std::expected<std::vector<std::size_t>, std::string>;

    class Parser
    {
    public:
        Ast ast;

        Parser(std::string_view src);

        void parse();

    private:
        Lexer lexer;
        std::string_view src;

        ParseResult parse_root();
        ParseResult parse_namespace_statement();
        ParseResult parse_use_statement();

        ParseChildrenResult parse_nested_names(const std::size_t min_column);

        std::size_t push_node(Node node);
    };
} // namespace orchid::compiler

#endif
