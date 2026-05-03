#include <cstddef>
#include <memory>
#include <string_view>
#include <utility>
#include <vector>

#include "ast.hpp"
#include "parser.hpp"

#define EXPECT_TOKEN(t, expected_type)                                         \
    if (t.type != expected_type)                                               \
    {                                                                          \
        return std::unexpected("unexpected token");                            \
    }

namespace orchid::compiler
{
    Parser::Parser(std::string_view src) : ast({}), lexer(src), src(src)
    {
    }

    ParseResult Parser::parse()
    {
        auto root { parse_root() };
        if (!root)
        {
            return root;
        }

        ast.root = root.value();
        return root;
    }

    ParseResult Parser::parse_root()
    {
        std::vector<std::size_t> children;

        for (auto t { lexer.peek() }; !t.is_eof(); t = lexer.peek())
        {
            ParseResult res;

            switch (t.type)
            {
            case TokenType::KwNamespace:
                res = parse_namespace_statement();
                break;
            case TokenType::KwUse:
                res = parse_use_statement();
                break;
            case TokenType::KwFun:
                res = parse_function_definition();
                break;
            default:
                return std::unexpected("unexpected token");
            }

            if (!res)
            {
                return res;
            }

            children.push_back(res.value());
        }

        return push_node(Node { NodeType::Root, nullptr, std::move(children) });
    }

    ParseResult Parser::parse_namespace_statement()
    {
        lexer.next();

        auto children { parse_nested_names() };
        if (!children)
        {
            return std::unexpected(children.error());
        }

        return push_node(Node { NodeType::NamespaceDeclaration, nullptr,
                                std::move(children.value()) });
    }

    ParseResult Parser::parse_use_statement()
    {
        lexer.next();

        auto children { parse_nested_names() };
        if (!children)
        {
            return std::unexpected(children.error());
        }

        return push_node(Node { NodeType::UseDeclaration, nullptr,
                                std::move(children.value()) });
    }

    ParseResult Parser::parse_function_definition()
    {
        lexer.next();

        auto t { lexer.next() };
        EXPECT_TOKEN(t, TokenType::Name);

        t = lexer.next();
        EXPECT_TOKEN(t, TokenType::OpLParen);

        t = lexer.next();
        EXPECT_TOKEN(t, TokenType::OpRParen);

        return std::unexpected("TODO");
    }

    ParseChildrenResult Parser::parse_nested_names()
    {
        std::vector<std::size_t> children;

        auto t { lexer.next() };
        EXPECT_TOKEN(t, TokenType::Name);

        children.push_back(push_node(Node {
            NodeType::Name,
            src.substr(t.pos, t.len),
            std::nullopt,
        }));

        for (t = lexer.peek(); t.type == TokenType::OpDot; t = lexer.peek())
        {
            lexer.next();

            t = lexer.next();
            EXPECT_TOKEN(t, TokenType::Name);

            children.push_back(push_node(Node {
                NodeType::Name,
                src.substr(t.pos, t.len),
                std::nullopt,
            }));
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

#undef EXPECT_TOKEN
