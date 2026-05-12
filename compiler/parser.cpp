#include <cstddef>
#include <memory>
#include <optional>
#include <string_view>
#include <utility>
#include <vector>

#include "ast.hpp"
#include "parser.hpp"

#define EXPECT_NEXT(lexer, expected_type)                                      \
    do {                                                                       \
        auto t { lexer.next() };                                               \
        if (t.type != expected_type) {                                         \
            return std::unexpected("unexpected token");                        \
        }                                                                      \
    } while (0)

#define EXPECT_NEXT_AND_TAKE(lexer, expected_type, t)                          \
    do {                                                                       \
        t = lexer.next();                                                      \
        if (t.type != expected_type) {                                         \
            return std::unexpected("unexpected token");                        \
        }                                                                      \
    } while (0)

namespace fla::compiler
{
    Parser::Parser(std::string_view src) : ast({}), lexer(src), src(src)
    {
    }

    ParseResult Parser::parse()
    {
        auto root { parse_root() };
        if (!root) {
            return root;
        }

        ast.root = root.value();
        return root;
    }

    ParseResult Parser::parse_root()
    {
        std::vector<std::size_t> children;

        for (auto t { lexer.peek() }; !t.is_eof(); t = lexer.peek()) {
            ParseResult res;

            switch (t.type) {
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

            if (!res) {
                return res;
            }

            children.push_back(res.value());
        }

        return push_node({ NodeType::Root, nullptr, std::move(children) });
    }

    ParseResult Parser::parse_namespace_statement()
    {
        lexer.next();

        auto children { parse_nested_names() };
        if (!children) {
            return std::unexpected(children.error());
        }

        return push_node({ NodeType::NamespaceDeclaration, nullptr,
                                std::move(children.value()) });
    }

    ParseResult Parser::parse_use_statement()
    {
        lexer.next();

        auto children { parse_nested_names() };
        if (!children) {
            return std::unexpected(children.error());
        }

        return push_node({ NodeType::UseDeclaration, nullptr,
                                std::move(children.value()) });
    }

    ParseResult Parser::parse_function_definition()
    {
        lexer.next();

        EXPECT_NEXT(lexer, TokenType::Name);
        EXPECT_NEXT(lexer, TokenType::OpLParen);
        EXPECT_NEXT(lexer, TokenType::OpRParen);

        return std::unexpected("TODO");
    }

    ParseChildrenResult Parser::parse_nested_names()
    {
        std::vector<std::size_t> children;

        Token t;
        EXPECT_NEXT_AND_TAKE(lexer, TokenType::Name, t);

        children.push_back(push_node({
            NodeType::Name,
            src.substr(t.pos, t.len),
        }));

        for (t = lexer.peek(); t.type == TokenType::OpDot; t = lexer.peek()) {
            lexer.next();

            EXPECT_NEXT_AND_TAKE(lexer, TokenType::Name, t);

            children.push_back(push_node({
                NodeType::Name,
                src.substr(t.pos, t.len),
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
} // namespace fla::compiler

#undef EXPECT_NEXT
#undef EXPECT_NEXT_AND_TAKE
