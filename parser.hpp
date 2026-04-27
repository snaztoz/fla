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
    class Parser
    {
    public:
        Ast ast;

        Parser(std::string_view src);

        void parse();

    private:
        Lexer lexer;
        std::string_view src;

        std::expected<std::size_t, std::string> parse_root();
        std::expected<std::size_t, std::string> parse_namespace_statement();

        std::expected<std::vector<std::size_t>, std::string>
        parse_nested_names(const std::size_t min_column);

        std::size_t push_node(Node node);
    };
} // namespace orchid::compiler

#endif
