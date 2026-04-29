#include <cstddef>
#include <memory>
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

    ParseResult Parser::parse_root()
    {
        auto idx { push_node(Node { NodeType::Root, nullptr }) };

        auto t { lexer.peek() };
        while (t.type != TokenType::Eof)
        {
            if (t.type == TokenType::KwNamespace)
            {
                parse_namespace_statement();
            }
            else if (t.type == TokenType::KwUse)
            {
                parse_use_statement();
            }
            else
            {
                return std::unexpected("unexpected token");
            }

            t = lexer.peek();
        }

        return idx;
    }

    ParseResult Parser::parse_namespace_statement()
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

    ParseResult Parser::parse_use_statement()
    {
        auto kw { lexer.next() };

        auto children { parse_nested_names(kw.column) };
        if (!children)
        {
            return std::unexpected(children.error());
        }

        return push_node(Node {
            NodeType::UseDeclaration,
            nullptr,
            std::move(children.value())
        });
    }

    ParseChildrenResult Parser::parse_nested_names(const std::size_t min_column)
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
