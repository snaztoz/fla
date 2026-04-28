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
        auto root { parse_root() };
        if (!root)
        {
            // todo
        }

        ast.root_idx = root.value();
    }

    std::expected<std::size_t, std::string> Parser::parse_root()
    {
        auto idx { push_node(Node { NodeType::Root, nullptr }) };

        auto t { lexer.peek() };
        while (t.type != TokenType::Eof)
        {
            if (t.type == TokenType::KwNamespace)
            {
                parse_namespace_statement();
            }

            t = lexer.peek();
        }

        return idx;
    }

    std::expected<std::size_t, std::string> Parser::parse_namespace_statement()
    {
        auto kw { lexer.next() };

        auto children { parse_nested_names(kw.column) };
        if (!children)
        {
            return std::unexpected(children.error());
        }

        return push_node(Node { NodeType::NamespaceDeclaration, nullptr,
                                std::move(children.value()) });
    }

    std::expected<std::vector<std::size_t>, std::string>
    Parser::parse_nested_names(const std::size_t min_column)
    {
        std::vector<std::size_t> children;

        auto t { lexer.next() };
        if (t.type != TokenType::Name || t.column <= min_column)
        {
            return std::unexpected("unexpected token");
        }

        children.push_back(push_node(Node {
            NodeType::Name,
            src.substr(t.pos, t.len),
        }));

        t = lexer.peek();
        while (t.type == TokenType::OpDot && t.column > min_column)
        {
            lexer.next();

            t = lexer.next();
            if (t.type != TokenType::Name || t.column <= min_column)
            {
                return std::unexpected("unexpected token");
            }

            children.push_back(push_node(Node {
                NodeType::Name,
                src.substr(t.pos, t.len),
            }));

            t = lexer.peek();
        }

        return children;
    }

    std::size_t Parser::push_node(Node node)
    {
        auto idx { ast.arena.size() };
        ast.arena.push_back(node);
        return idx;
    }
} // namespace orchid::compiler
