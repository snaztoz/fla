#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

#include "ast.hpp"
#include "parser.hpp"

namespace orchid::compiler
{
    Parser::Parser(std::string_view src) : lexer(src), ast({}), src(src)
    {
    }

    void Parser::parse()
    {
        auto root = parse_root();
        if (!root)
        {
            // todo
        }

        ast.root_idx = root.value();
    }

    std::expected<std::size_t, std::string> Parser::parse_root()
    {
        auto idx = push_node(Node{NodeType::Root, nullptr});

        auto token = lexer.peek_token();
        while (token.type != TokenType::Eof)
        {
            if (token.type == TokenType::KwNamespace)
            {
                parse_namespace_statement();
            }

            token = lexer.peek_token();
        }

        return idx;
    }

    std::expected<std::size_t, std::string> Parser::parse_namespace_statement()
    {
        auto kw = lexer.next_token();
        auto column = kw.column;

        auto children_idx = parse_nested_names(column);
        if (!children_idx)
        {
            return std::unexpected(children_idx.error());
        }

        return push_node(Node{NodeType::NamespaceDeclaration, nullptr,
                              std::move(children_idx.value())});
    }

    std::expected<std::vector<std::size_t>, std::string>
    Parser::parse_nested_names(const std::size_t min_column)
    {
        std::vector<std::size_t> children_idx;

        auto t{lexer.next_token()};
        if (t.type != TokenType::Name || t.column <= min_column)
        {
            return std::unexpected("unexpected token");
        }

        children_idx.push_back(push_node(Node{
            NodeType::Name,
            src.substr(t.pos, t.len),
        }));

        t = lexer.peek_token();
        while (t.type == TokenType::OpDot && t.column > min_column)
        {
            lexer.next_token();

            t = lexer.next_token();
            if (t.type != TokenType::Name || t.column <= min_column)
            {
                return std::unexpected("unexpected token");
            }

            children_idx.push_back(push_node(Node{
                NodeType::Name,
                src.substr(t.pos, t.len),
            }));

            t = lexer.peek_token();
        }

        return children_idx;
    }

    std::size_t Parser::push_node(Node node)
    {
        auto idx = ast.arena.size();
        ast.arena.push_back(node);
        return idx;
    }
} // namespace orchid::compiler
