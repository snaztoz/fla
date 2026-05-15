#include <cstddef>
#include <memory>
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
    Parser::Parser(const std::string_view src) : lexer(src), src(src)
    {
    }

    ParseResult Parser::parse()
    {
        std::vector<Node> children;

        for (auto t { lexer.peek() }; !t.is_eof(); t = lexer.peek()) {
            const auto res { parse_root(t.type) };
            if (!res) {
                return res;
            }

            children.push_back(res.value());
        }

        return ParseResult({ NodeType::Root, nullptr, std::move(children) });
    }

    ParseResult Parser::parse_root(const TokenType &type)
    {
        switch (type) {
        case TokenType::KwNamespace:
            return parse_namespace_statement();
        case TokenType::KwUse:
            return parse_use_statement();
        case TokenType::KwFun:
            return parse_function_definition();
        default:
            return std::unexpected("unexpected token");
        }
    }

    ParseResult Parser::parse_namespace_statement()
    {
        lexer.next();

        auto children { parse_nested_names() };
        if (!children) {
            return std::unexpected(children.error());
        }

        return ParseResult({ NodeType::NamespaceDeclaration, nullptr,
                             std::move(children.value()) });
    }

    ParseResult Parser::parse_use_statement()
    {
        lexer.next();

        auto children { parse_nested_names() };
        if (!children) {
            return std::unexpected(children.error());
        }

        return ParseResult(
            { NodeType::UseDeclaration, nullptr, std::move(children.value()) });
    }

    ParseResult Parser::parse_function_definition()
    {
        lexer.next();

        std::vector<Node> children;

        EXPECT_NEXT(lexer, TokenType::Name);
        EXPECT_NEXT(lexer, TokenType::SymLParen);

        const auto parameters { parse_function_parameters() };
        if (!parameters) {
            return std::unexpected(parameters.error());
        }

        EXPECT_NEXT(lexer, TokenType::SymRParen);
        EXPECT_NEXT(lexer, TokenType::KwDo);

        // TODO: parse body

        EXPECT_NEXT(lexer, TokenType::KwEnd);

        children.push_back(
            { NodeType::FunctionParameterList, nullptr, parameters.value() });

        return ParseResult(
            { NodeType::FunctionDefinition, nullptr, std::move(children) });
    }

    ParseChildrenResult Parser::parse_function_parameters()
    {
        std::vector<Node> parameters;

        while (lexer.peek().type != TokenType::SymRParen) {
            Token name;
            EXPECT_NEXT_AND_TAKE(lexer, TokenType::Name, name);

            Token type_notation;
            EXPECT_NEXT_AND_TAKE(lexer, TokenType::Name, type_notation);

            switch (lexer.peek().type) {
            case TokenType::SymComma:
                lexer.next();
                break;
            case TokenType::SymRParen:
                continue;
            default:
                return std::unexpected("unknown token in function parameter");
            }

            parameters.push_back(
                { NodeType::FunctionParameter,
                  nullptr,
                  { { NodeType::Name, src.substr(name.pos, name.len) },
                    { NodeType::TypeNotation,
                      src.substr(type_notation.pos, type_notation.len) } } });
        }

        return parameters;
    }

    ParseChildrenResult Parser::parse_nested_names()
    {
        std::vector<Node> children;

        Token t;
        EXPECT_NEXT_AND_TAKE(lexer, TokenType::Name, t);

        children.push_back({
            NodeType::Name,
            src.substr(t.pos, t.len),
        });

        for (t = lexer.peek(); t.type == TokenType::OpDot; t = lexer.peek()) {
            lexer.next();

            EXPECT_NEXT_AND_TAKE(lexer, TokenType::Name, t);

            children.push_back({
                NodeType::Name,
                src.substr(t.pos, t.len),
            });
        }

        return children;
    }
} // namespace fla::compiler

#undef EXPECT_NEXT
#undef EXPECT_NEXT_AND_TAKE
