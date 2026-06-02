#include <cstddef>
#include <format>
#include <iterator>
#include <memory>
#include <set>
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

            const auto res { parse_root(t) };
            if (!res) {
                return res;
            }

            children.push_back(std::move(*res));
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

    ParseResult Parser::parse_root(const Token &t)
    {
        switch (t.type) {
        case TokenType::KwNamespace:
            return parse_namespace_statement();
        case TokenType::KwUse:
            return parse_use_statement();
        case TokenType::KwFun:
            return parse_function_definition();
        default:
            return std::unexpected(Error {
                t.pos,
                t.len,
                t.line,
                t.column,
                std::format("expecting top-level statement(s), found {} instead",
                            token_type_string(t.type)),
            });
        }
    }

    ParseResult Parser::parse_namespace_statement()
    {
        const auto kw { lexer.next() };

        const auto children { parse_nested_names() };
        if (!children) {
            return std::unexpected(children.error());
        }

        const auto last { children->at(children->size() - 1) };

        return ParseResult({
            NodeType::NamespaceDeclaration,
            nullptr,
            *children,
            kw.pos,
            len_between(kw, last),
            kw.line,
            kw.column,
        });
    }

    ParseResult Parser::parse_use_statement()
    {
        const auto kw { lexer.next() };

        const auto children { parse_nested_names() };
        if (!children) {
            return std::unexpected(children.error());
        }

        const auto last { children->at(children->size() - 1) };

        return ParseResult({
            NodeType::UseDeclaration,
            nullptr,
            *children,
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

        if (!parameters->empty()) {
            const auto pos { parameters->at(0).pos };
            const auto len { parameters->at(0).len };
            const auto line { parameters->at(0).line };
            const auto column { parameters->at(0).column };

            children.emplace_back(NodeType::FunctionParameterList, nullptr, std::move(*parameters),
                                  pos, len, line, column);
        }

        if (const auto t { expect(TokenType::SymRParen) }; !t) {
            return std::unexpected(t.error());
        }

        // Return type is optional
        if (lexer.peek().type != TokenType::KwDo) {
            const auto return_type_notation { parse_function_return_type_notation() };
            if (!return_type_notation) {
                return std::unexpected(return_type_notation.error());
            }
            children.push_back(std::move(*return_type_notation));
        }

        if (const auto t { expect(TokenType::KwDo) }; !t) {
            return std::unexpected(t.error());
        }

        const auto body { parse_body() };
        if (!body) {
            return std::unexpected(body.error());
        }
        std::move(body->begin(), body->end(), std::back_inserter(children));

        const auto end { expect(TokenType::KwEnd) };
        if (!end) {
            return std::unexpected(end.error());
        }

        return ParseResult({
            NodeType::FunctionDefinition,
            nullptr,
            std::move(children),
            kw.pos,
            len_between(kw, *end),
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
                return std::unexpected(Error {
                    next.pos,
                    next.len,
                    next.line,
                    next.column,
                    std::format("expecting {}, found {} instead",
                                token_type_string(TokenType::SymRParen),
                                token_type_string(next.type)),
                });
            }

            const std::vector<Node> children = {
                {
                    NodeType::Name,
                    src.substr(name->pos, name->len),
                    name->pos,
                    name->len,
                    name->line,
                    name->column,
                },
                *type_notation,
            };

            parameters.emplace_back(NodeType::FunctionParameter, nullptr, std::move(children),
                                    name->pos, len_between(*name, *type_notation), name->line,
                                    name->column);
        }

        return parameters;
    }

    ParseResult Parser::parse_function_return_type_notation()
    {
        const auto type_notation { parse_type_notation() };
        if (!type_notation) {
            return std::unexpected(type_notation.error());
        }

        const auto pos { type_notation->pos };
        const auto len { type_notation->len };
        const auto line { type_notation->line };
        const auto column { type_notation->column };

        return ParseResult({
            NodeType::FunctionReturnTypeNotation,
            nullptr,
            { std::move(*type_notation) },
            pos,
            len,
            line,
            column,
        });
    }

    ParseChildrenResult Parser::parse_nested_names()
    {
        std::vector<Node> children;

        const auto first_name { expect(TokenType::Name) };
        if (!first_name) {
            return std::unexpected(first_name.error());
        }

        children.emplace_back(NodeType::Name, src.substr(first_name->pos, first_name->len),
                              first_name->pos, first_name->len, first_name->line,
                              first_name->column);

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

            children.emplace_back(NodeType::Name, src.substr(next_name->pos, next_name->len),
                                  next_name->pos, next_name->len, next_name->line,
                                  next_name->column);
        }

        return children;
    }

    using BodyStatementRuleTokenPrefixes = std::set<TokenType>;
    using BodyStatementRuleParser = std::function<ParseResult(void)>;

    ParseChildrenResult Parser::parse_body()
    {
        std::vector<Node> children;

        const auto rules = {
            std::make_pair<BodyStatementRuleTokenPrefixes, BodyStatementRuleParser>(
                { TokenType::KwConst, TokenType::KwVar },
                [this] { return parse_variable_declaration(); }),
        };

        while (true) {
            const auto t { lexer.peek() };
            if (t.type == TokenType::KwEnd) {
                break;
            }

            auto found = false;

            for (const auto &[prefix, parse_rule] : rules) {
                if (!prefix.contains(t.type)) {
                    continue;
                }

                const auto statement { parse_rule() };
                if (!statement) {
                    return std::unexpected(statement.error());
                }

                children.push_back(std::move(*statement));
                found = true;

                break;
            }

            if (found) {
                continue;
            }

            const auto statement { parse_expression_statement() };
            if (!statement) {
                return std::unexpected(statement.error());
            }

            children.push_back(std::move(*statement));
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
            src.substr(type_notation->pos, type_notation->len),
            type_notation->pos,
            type_notation->len,
            type_notation->line,
            type_notation->column,
        });
    }

    ParseResult Parser::parse_variable_declaration()
    {
        std::vector<Node> children;

        const auto kw { lexer.next() };

        const auto name { expect(TokenType::Name) };
        if (!name) {
            return std::unexpected(name.error());
        }
        children.emplace_back(NodeType::Name, src.substr(name->pos, name->len), name->pos,
                              name->len, name->line, name->column);

        if (lexer.peek().type != TokenType::OpAssign) {
            const auto type_notation { parse_type_notation() };
            if (!type_notation) {
                return std::unexpected(type_notation.error());
            }
            children.push_back(std::move(*type_notation));
        }

        if (const auto t { expect(TokenType::OpAssign) }; !t) {
            return std::unexpected(t.error());
        }

        const auto expression { parse_expression() };
        if (!expression) {
            return expression;
        }

        const auto pos { kw.pos };
        const auto len { len_between(kw, *expression) };
        const auto line { kw.line };
        const auto column { kw.column };

        children.push_back(std::move(*expression));

        return ParseResult({
            kw.type == TokenType::KwVar ? NodeType::VariableDeclaration
                                        : NodeType::ConstantDeclaration,
            nullptr,
            std::move(children),
            pos,
            len,
            line,
            column,
        });
    }

    ParseResult Parser::parse_expression_statement()
    {
        return parse_expression();
    }

    std::expected<Token, Error> Parser::expect(const TokenType &expected_tt)
    {
        const auto t { lexer.next() };
        if (t.type != expected_tt) {
            return std::unexpected(Error {
                t.pos,
                t.len,
                t.line,
                t.column,
                std::format("expecting {}, found {} instead", token_type_string(expected_tt),
                            token_type_string(t.type)),
            });
        }
        return t;
    }
} // namespace fla::compiler
