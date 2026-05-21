#include <cstddef>
#include <format>
#include <iterator>
#include <memory>
#include <string_view>
#include <utility>
#include <vector>

#include "ast.hpp"
#include "parser.hpp"

namespace fla::compiler
{
    Parser::Parser(const std::string_view src) : lexer(src), src(src)
    {
    }

    ParseResult Parser::parse()
    {
        std::vector<Node> children;

        while (true) {
            const auto t { lexer.peek() };
            if (t.is_eof()) {
                break;
            }

            const auto res { parse_root(t.type) };
            if (!res) {
                return res;
            }

            children.push_back(res.value());
        }

        const auto first { children.at(0) };
        const auto last { children.at(children.size() - 1) };

        return ParseResult({
            NodeType::Root,
            nullptr,
            std::move(children),
            first.pos,
            len_between(first, last),
            first.line,
            first.column,
        });
    }

    ParseResult Parser::parse_root(const TokenType &tt)
    {
        switch (tt) {
        case TokenType::KwNamespace:
            return parse_namespace_statement();
        case TokenType::KwUse:
            return parse_use_statement();
        case TokenType::KwFun:
            return parse_function_definition();
        default:
            return std::unexpected(std::format("expecting top-level statement(s), found {} instead",
                                               token_type_string(tt)));
        }
    }

    ParseResult Parser::parse_namespace_statement()
    {
        const auto kw { lexer.next() };

        auto children { parse_nested_names() };
        if (!children) {
            return std::unexpected(children.error());
        }

        const auto last { children.value().at(children.value().size() - 1) };

        return ParseResult({
            NodeType::NamespaceDeclaration,
            nullptr,
            std::move(children.value()),
            kw.pos,
            len_between(kw, last),
            kw.line,
            kw.column,
        });
    }

    ParseResult Parser::parse_use_statement()
    {
        const auto kw { lexer.next() };

        auto children { parse_nested_names() };
        if (!children) {
            return std::unexpected(children.error());
        }

        const auto last { children.value().at(children.value().size() - 1) };

        return ParseResult({
            NodeType::UseDeclaration,
            nullptr,
            std::move(children.value()),
            kw.pos,
            len_between(kw, last),
            kw.line,
            kw.column,
        });
    }

    ParseResult Parser::parse_function_definition()
    {
        const auto kw { lexer.next() };

        std::vector<Node> children;

        if (const auto t { expect(TokenType::Name) }; !t) {
            return std::unexpected(t.error());
        }

        if (const auto t { expect(TokenType::SymLParen) }; !t) {
            return std::unexpected(t.error());
        }

        const auto parameters { parse_function_parameters() };
        if (!parameters) {
            return std::unexpected(parameters.error());
        }

        if (!parameters.value().empty()) {
            children.push_back({
                NodeType::FunctionParameterList,
                nullptr,
                parameters.value(),
                parameters.value()[0].pos,
                parameters.value()[0].len,
                parameters.value()[0].line,
                parameters.value()[0].column,
            });
        }

        if (const auto t { expect(TokenType::SymRParen) }; !t) {
            return std::unexpected(t.error());
        }

        if (const auto t { expect(TokenType::KwDo) }; !t) {
            return std::unexpected(t.error());
        }

        const auto body { parse_body() };
        if (!body) {
            return std::unexpected(body.error());
        }
        std::move(body.value().begin(), body.value().end(), std::back_inserter(children));

        const auto end { expect(TokenType::KwEnd) };
        if (!end) {
            return std::unexpected(end.error());
        }

        return ParseResult({
            NodeType::FunctionDefinition,
            nullptr,
            std::move(children),
            kw.pos,
            len_between(kw, end.value()),
            kw.line,
            kw.column,
        });
    }

    ParseChildrenResult Parser::parse_function_parameters()
    {
        std::vector<Node> parameters;

        while (lexer.peek().type != TokenType::SymRParen) {
            const auto name { expect(TokenType::Name) };
            if (!name) {
                return std::unexpected(name.error());
            }

            const auto type_notation { parse_type_notation() };
            if (!type_notation) {
                return std::unexpected(type_notation.error());
            }

            const auto next { lexer.peek() };

            switch (next.type) {
            case TokenType::SymComma:
                lexer.next();
                break;
            case TokenType::SymRParen:
                continue;
            default:
                return std::unexpected(std::format("expecting {}, found {} instead",
                                                   token_type_string(TokenType::SymRParen),
                                                   token_type_string(next.type)));
            }

            parameters.push_back({
                NodeType::FunctionParameter,
                nullptr,
                {
                    {
                        NodeType::Name,
                        src.substr(name.value().pos, name.value().len),
                        name.value().pos,
                        name.value().len,
                        name.value().line,
                        name.value().column,
                    },
                    type_notation.value(),
                },
                name.value().pos,
                len_between(name.value(), type_notation.value()),
                name.value().line,
                name.value().column,
            });
        }

        return parameters;
    }

    ParseChildrenResult Parser::parse_nested_names()
    {
        std::vector<Node> children;

        const auto first_name { expect(TokenType::Name) };
        if (!first_name) {
            return std::unexpected(first_name.error());
        }

        children.push_back({
            NodeType::Name,
            src.substr(first_name.value().pos, first_name.value().len),
            first_name.value().pos,
            first_name.value().len,
            first_name.value().line,
            first_name.value().column,
        });

        while (true) {
            const auto t { lexer.peek() };
            if (t.type != TokenType::OpDot) {
                break;
            }

            lexer.next();

            const auto next_name = expect(TokenType::Name);
            if (!next_name) {
                return std::unexpected(next_name.error());
            }

            children.push_back({
                NodeType::Name,
                src.substr(next_name.value().pos, next_name.value().len),
                next_name.value().pos,
                next_name.value().len,
                next_name.value().line,
                next_name.value().column,
            });
        }

        return children;
    }

    ParseChildrenResult Parser::parse_body()
    {
        std::vector<Node> children;

        while (lexer.peek().type != TokenType::KwEnd) {
            const auto expression { parse_expression_statement() };
            if (!expression) {
                return std::unexpected(expression.error());
            }
            children.push_back(expression.value());
        }

        return children;
    }

    ParseResult Parser::parse_type_notation()
    {
        const auto type_notation { expect(TokenType::Name) };
        if (!type_notation) {
            return std::unexpected(type_notation.error());
        }

        return ParseResult({
            NodeType::TypeNotation,
            src.substr(type_notation.value().pos, type_notation.value().len),
            type_notation.value().pos,
            type_notation.value().len,
            type_notation.value().line,
            type_notation.value().column,
        });
    }

    ParseResult Parser::parse_expression_statement()
    {
        return parse_expression();
    }

    std::expected<Token, std::string> Parser::expect(const TokenType &expected_tt)
    {
        const auto t { lexer.next() };
        if (t.type != expected_tt) {
            return std::unexpected(std::format("expecting {}, found {} instead",
                                               token_type_string(expected_tt),
                                               token_type_string(t.type)));
        }
        return t;
    }
} // namespace fla::compiler
