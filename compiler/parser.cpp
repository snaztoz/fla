#include <expected>
#include <format>
#include <memory>
#include <optional>
#include <string_view>
#include <utility>
#include <vector>

#include "ast.hpp"
#include "parser.hpp"
#include "token.hpp"

namespace fla::compiler
{
    using ParseNameResult = std::expected<Name, Error>;
    using ParseNestedNamesResult = std::expected<std::vector<Name>, Error>;
    using ParseTypeNotationResult = std::expected<TypeNotation, Error>;
    using ParseFunctionParametersResult =
        std::expected<std::vector<std::pair<Name, TypeNotation>>, Error>;

    ParseResult parse_root(ParserContext &ctx, const Token &t);
    ParseResult parse_namespace_statement(ParserContext &ctx);
    ParseResult parse_use_statement(ParserContext &ctx);
    ParseResult parse_function_definition(ParserContext &ctx);
    ParseFunctionParametersResult parse_function_parameters(ParserContext &ctx);
    ParseNestedNamesResult parse_nested_names(ParserContext &ctx);
    ParseTypeNotationResult parse_type_notation(ParserContext &ctx);
    ParseResult parse_variable_declaration(ParserContext &ctx);
    ParseResult parse_expression_statement(ParserContext &ctx);
    ParseNameResult parse_name(ParserContext &ctx);

    ParseResult parse(ParserContext &ctx)
    {
        std::vector<Node> body;

        while (true) {
            const auto t { ctx.lexer.peek() };
            if (t.is_eof()) {
                break;
            }

            auto res { parse_root(ctx, t) };
            if (!res) {
                return res;
            }

            body.push_back(std::move(*res));
        }

        const auto first { get_node_metadata(body.at(0)) };
        const auto last { get_node_metadata(body.at(body.size() - 1)) };

        return std::make_unique<Root>(Root { std::move(body),
                                             {
                                                 first.pos,
                                                 last.pos - first.pos + last.len,
                                                 first.line,
                                                 first.col,
                                             } });
    }

    ParseResult parse_root(ParserContext &ctx, const Token &t)
    {
        switch (t.type) {
        case TokenType::KwNamespace:
            return parse_namespace_statement(ctx);
        case TokenType::KwUse:
            return parse_use_statement(ctx);
        case TokenType::KwFun:
            return parse_function_definition(ctx);
        default:
            return std::unexpected(Error {
                t.pos,
                t.len,
                t.line,
                t.col,
                std::format("expecting top-level statement(s), found {} instead",
                            token_type_string(t.type)),
            });
        }
    }

    ParseResult parse_namespace_statement(ParserContext &ctx)
    {
        const auto kw { ctx.lexer.next() };

        const auto segments { parse_nested_names(ctx) };
        if (!segments) {
            return std::unexpected(segments.error());
        }

        const auto last { segments->at(segments->size() - 1) };
        const auto last_meta { get_node_metadata(last) };

        return std::make_unique<NamespaceDeclaration>(
            NamespaceDeclaration { std::move(*segments),
                                   {
                                       kw.pos,
                                       last_meta.pos - kw.pos + last_meta.len,
                                       kw.line,
                                       kw.col,
                                   } });
    }

    ParseResult parse_use_statement(ParserContext &ctx)
    {
        const auto kw { ctx.lexer.next() };

        const auto segments { parse_nested_names(ctx) };
        if (!segments) {
            return std::unexpected(segments.error());
        }

        const auto last { get_node_metadata(segments->at(segments->size() - 1)) };

        return std::make_unique<UseDeclaration>(UseDeclaration { std::move(*segments),
                                                                 {
                                                                     kw.pos,
                                                                     last.pos - kw.pos + last.len,
                                                                     kw.line,
                                                                     kw.col,
                                                                 } });
    }

    ParseResult parse_function_definition(ParserContext &ctx)
    {
        const auto kw { ctx.lexer.next() };

        const auto name { parse_name(ctx) };
        if (!name) {
            return std::unexpected(name.error());
        }

        if (const auto t { expect(ctx, TokenType::SymLParen) }; !t) {
            return std::unexpected(t.error());
        }

        const auto parameters { parse_function_parameters(ctx) };
        if (!parameters) {
            return std::unexpected(parameters.error());
        }

        if (const auto t { expect(ctx, TokenType::SymRParen) }; !t) {
            return std::unexpected(t.error());
        }

        // Return type is optional
        std::optional<TypeNotation> return_type_notation;
        if (ctx.lexer.peek().type != TokenType::KwDo) {
            const auto tn { parse_type_notation(ctx) };
            if (!tn) {
                return std::unexpected(tn.error());
            }
            return_type_notation.emplace(std::move(*tn));
        }

        if (const auto t { expect(ctx, TokenType::KwDo) }; !t) {
            return std::unexpected(t.error());
        }

        auto body { parse_body(ctx, { TokenType::KwEnd }) };
        if (!body) {
            return std::unexpected(body.error());
        }

        const auto end { expect(ctx, TokenType::KwEnd) };
        if (!end) {
            return std::unexpected(end.error());
        }

        return std::make_unique<FunctionDefinition>(
            FunctionDefinition { std::move(*name),
                                 std::move(*parameters),
                                 std::move(return_type_notation),
                                 std::move(*body),
                                 {
                                     kw.pos,
                                     end->pos - kw.pos + end->len,
                                     kw.line,
                                     kw.col,
                                 } });
    }

    ParseFunctionParametersResult parse_function_parameters(ParserContext &ctx)
    {
        std::vector<std::pair<Name, TypeNotation>> parameters;

        while (ctx.lexer.peek().type != TokenType::SymRParen) {
            auto name { parse_name(ctx) };
            if (!name) {
                return std::unexpected(name.error());
            }

            auto type_notation { parse_type_notation(ctx) };
            if (!type_notation) {
                return std::unexpected(type_notation.error());
            }

            parameters.push_back(std::make_pair(*name, *type_notation));

            const auto next { ctx.lexer.peek() };

            switch (next.type) {
            case TokenType::SymComma:
                ctx.lexer.next();
                break;
            case TokenType::SymRParen:
                continue;
            default:
                return std::unexpected(Error {
                    next.pos,
                    next.len,
                    next.line,
                    next.col,
                    std::format("expecting {}, found {} instead",
                                token_type_string(TokenType::SymRParen),
                                token_type_string(next.type)),
                });
            }
        }

        return parameters;
    }

    ParseNestedNamesResult parse_nested_names(ParserContext &ctx)
    {
        std::vector<Name> body;

        const auto first_segment { parse_name(ctx) };
        if (!first_segment) {
            return std::unexpected(first_segment.error());
        }

        body.push_back(std::move(*first_segment));

        while (true) {
            const auto t { ctx.lexer.peek() };
            if (t.type != TokenType::OpDot) {
                break;
            }

            ctx.lexer.next();

            const auto next_segment { parse_name(ctx) };
            if (!next_segment) {
                return std::unexpected(next_segment.error());
            }

            body.push_back(std::move(*next_segment));
        }

        return body;
    }

    using BodyStatementRuleTokenPrefixes = std::set<TokenType>;
    using BodyStatementRuleParser = std::function<ParseResult(void)>;

    ParseBodyResult parse_body(ParserContext &ctx, std::set<TokenType> end_delimiters)
    {
        std::vector<Node> body;

        const auto rules = {
            std::make_pair<BodyStatementRuleTokenPrefixes, BodyStatementRuleParser>(
                { TokenType::KwConst, TokenType::KwVar },
                [&ctx] { return parse_variable_declaration(ctx); }),
        };

        while (true) {
            const auto t { ctx.lexer.peek() };
            if (end_delimiters.contains(t.type)) {
                break;
            }

            auto found = false;

            for (const auto &[prefix, parse_rule] : rules) {
                if (!prefix.contains(t.type)) {
                    continue;
                }

                auto statement { parse_rule() };
                if (!statement) {
                    return std::unexpected(statement.error());
                }

                body.push_back(std::move(*statement));
                found = true;

                break;
            }

            if (found) {
                continue;
            }

            auto statement { parse_expression_statement(ctx) };
            if (!statement) {
                return std::unexpected(statement.error());
            }

            body.push_back(std::move(*statement));
        }

        return body;
    }

    ParseTypeNotationResult parse_type_notation(ParserContext &ctx)
    {
        const auto type_notation { parse_name(ctx) };
        if (!type_notation) {
            return std::unexpected(type_notation.error());
        }

        const auto meta { get_node_metadata(*type_notation) };

        return TypeNotation { std::move(*type_notation), meta };
    }

    ParseResult parse_variable_declaration(ParserContext &ctx)
    {
        const auto kw { ctx.lexer.next() };

        const auto name { parse_name(ctx) };
        if (!name) {
            return std::unexpected(name.error());
        }

        std::optional<TypeNotation> type_notation;
        if (ctx.lexer.peek().type != TokenType::OpAssign) {
            const auto tn { parse_type_notation(ctx) };
            if (!tn) {
                return std::unexpected(tn.error());
            }
            type_notation.emplace(std::move(*tn));
        }

        if (const auto t { expect(ctx, TokenType::OpAssign) }; !t) {
            return std::unexpected(t.error());
        }

        auto expression { parse_expression(ctx) };
        if (!expression) {
            return expression;
        }
        const auto expression_meta { get_node_metadata(*expression) };

        const auto pos { kw.pos };
        const auto len { expression_meta.pos - kw.pos + expression_meta.len };
        const auto line { kw.line };
        const auto col { kw.col };

        Metadata meta { pos, len, line, col };

        return std::make_unique<VariableDeclaration>(VariableDeclaration {
            std::move(*name),
            type_notation,
            std::move(*expression),
            meta,
        });
    }

    ParseResult parse_expression_statement(ParserContext &ctx)
    {
        return parse_expression(ctx);
    }

    ParseNameResult parse_name(ParserContext &ctx)
    {
        const auto name_token { expect(ctx, TokenType::Name) };
        if (!name_token) {
            return std::unexpected(name_token.error());
        }

        const std::string name_text { ctx.src.substr(name_token->pos, name_token->len) };
        const Name name {
            name_text,
            { name_token->pos, name_token->len, name_token->line, name_token->col },
        };

        return name;
    }

    std::expected<Token, Error> expect(ParserContext &ctx, const TokenType &expected_tt)
    {
        const auto t { ctx.lexer.next() };
        if (t.type != expected_tt) {
            return std::unexpected(Error {
                t.pos,
                t.len,
                t.line,
                t.col,
                std::format("expecting {}, found {} instead", token_type_string(expected_tt),
                            token_type_string(t.type)),
            });
        }
        return t;
    }
} // namespace fla::compiler
