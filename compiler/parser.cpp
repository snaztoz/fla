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
    using ParseNestedNamesResult = std::expected<std::vector<Name>, Error>;
    using ParseFunctionParametersResult = std::expected<std::vector<std::pair<Name, Node>>, Error>;

    ParseResult parse_namespace_statement(ParserContext &ctx);
    ParseResult parse_use_statement(ParserContext &ctx);
    ParseResult parse_public_scope(ParserContext &ctx);
    ParseResult parse_class_definition(ParserContext &ctx);
    ParseResult parse_function_definition(ParserContext &ctx);
    ParseResult parse_forward_declaration(ParserContext &ctx);
    ParseFunctionParametersResult parse_function_parameters(ParserContext &ctx);
    ParseNestedNamesResult parse_nested_names(ParserContext &ctx);
    ParseResult parse_variable_declaration(ParserContext &ctx);
    ParseResult parse_while_statement(ParserContext &ctx);
    ParseResult parse_expression_statement(ParserContext &ctx);

    ParseResult parse(ParserContext &ctx)
    {
        auto body { parse_body(ctx, { TokenType::Eof }) };
        if (!body) {
            return std::unexpected(body.error());
        }

        if (body->size() == 0) {
            return std::make_unique<Root>(Root { std::move(*body), { 0, 0, 0, 0 } });
        }

        const auto first { get_node_metadata(body->at(0)) };
        const auto last { get_node_metadata(body->at(body->size() - 1)) };

        return std::make_unique<Root>(Root { std::move(*body),
                                             {
                                                 first.pos,
                                                 last.pos - first.pos + last.len,
                                                 first.line,
                                                 first.col,
                                             } });
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

    ParseResult parse_public_scope(ParserContext &ctx)
    {
        const auto kw { ctx.lexer.next() };

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

        return std::make_unique<PublicScope>(PublicScope { std::move(*body),
                                                           {
                                                               kw.pos,
                                                               end->pos - kw.pos + end->len,
                                                               kw.line,
                                                               kw.col,
                                                           } });
    }

    ParseResult parse_class_definition(ParserContext &ctx)
    {
        const auto kw { ctx.lexer.next() };

        const auto name { parse_name(ctx) };
        if (!name) {
            return std::unexpected(name.error());
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

        return std::make_unique<ClassDefinition>(ClassDefinition { std::move(*name),
                                                                   std::move(*body),
                                                                   {
                                                                       kw.pos,
                                                                       end->pos - kw.pos + end->len,
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

        auto parameters { parse_function_parameters(ctx) };
        if (!parameters) {
            return std::unexpected(parameters.error());
        }

        if (const auto t { expect(ctx, TokenType::SymRParen) }; !t) {
            return std::unexpected(t.error());
        }

        // Return type is optional
        std::optional<TypeNotationNode> return_tn;
        if (ctx.lexer.peek().type != TokenType::KwDo) {
            auto tn { parse_type_notation_node(ctx) };
            if (!tn) {
                return std::unexpected(tn.error());
            }
            return_tn.emplace(std::move(*tn));
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
                                 std::move(return_tn),
                                 std::move(*body),
                                 {
                                     kw.pos,
                                     end->pos - kw.pos + end->len,
                                     kw.line,
                                     kw.col,
                                 } });
    }

    ParseResult parse_class_forward_declaration(ParserContext &ctx, const Token &declare_t)
    {
        auto name { parse_name(ctx) };
        if (!name) {
            return name;
        }

        const auto name_metadata { get_node_metadata(*name) };

        return std::make_unique<ClassForwardDeclaration>(
            ClassForwardDeclaration { std::move(*name),
                                      {
                                          declare_t.pos,
                                          name_metadata.pos - declare_t.pos + name_metadata.len,
                                          declare_t.line,
                                          declare_t.col,
                                      } });
    }

    ParseResult parse_function_forward_declaration(ParserContext &ctx, const Token &declare_t)
    {
        auto name { parse_name(ctx) };
        if (!name) {
            return name;
        }

        if (const auto t { expect(ctx, TokenType::SymLParen) }; !t) {
            return std::unexpected(t.error());
        }

        if (const auto t { expect(ctx, TokenType::SymRParen) }; !t) {
            return std::unexpected(t.error());
        }

        auto return_tn { parse_type_notation_node(ctx) };
        if (!return_tn) {
            return return_tn;
        }

        const auto return_tn_metadata { get_type_notation_metadata(return_tn->tn) };

        return std::make_unique<FunctionForwardDeclaration>(FunctionForwardDeclaration {
            std::move(*name),
            {},
            std::move(*return_tn),
            {
                declare_t.pos,
                return_tn_metadata.pos - declare_t.pos + return_tn_metadata.len,
                declare_t.line,
                declare_t.col,
            } });
    }

    ParseResult parse_forward_declaration(ParserContext &ctx)
    {
        const auto kw { ctx.lexer.next() };

        const auto entity { ctx.lexer.next() };
        if (entity.type != TokenType::KwClass && entity.type != TokenType::KwFun) {
            return std::unexpected(Error {
                entity.pos,
                entity.len,
                entity.line,
                entity.col,
                std::format("expecting `class` or `fun` keywords, found {} instead",
                            token_type_string(entity.type)),
            });
        }

        if (entity.type == TokenType::KwClass) {
            return parse_class_forward_declaration(ctx, kw);
        } else {
            return parse_function_forward_declaration(ctx, kw);
        }
    }

    ParseFunctionParametersResult parse_function_parameters(ParserContext &ctx)
    {
        std::vector<std::pair<Name, Node>> parameters;

        while (ctx.lexer.peek().type != TokenType::SymRParen) {
            auto name { parse_name(ctx) };
            if (!name) {
                return std::unexpected(name.error());
            }

            auto tn { parse_type_notation_node(ctx) };
            if (!tn) {
                return std::unexpected(tn.error());
            }

            parameters.push_back(std::make_pair(*name, std::move(*tn)));

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
                { TokenType::KwClass }, [&ctx] { return parse_class_definition(ctx); }),
            std::make_pair<BodyStatementRuleTokenPrefixes, BodyStatementRuleParser>(
                { TokenType::KwConst, TokenType::KwVar },
                [&ctx] { return parse_variable_declaration(ctx); }),
            std::make_pair<BodyStatementRuleTokenPrefixes, BodyStatementRuleParser>(
                { TokenType::KwDeclare }, [&ctx] { return parse_forward_declaration(ctx); }),
            std::make_pair<BodyStatementRuleTokenPrefixes, BodyStatementRuleParser>(
                { TokenType::KwFun }, [&ctx] { return parse_function_definition(ctx); }),
            std::make_pair<BodyStatementRuleTokenPrefixes, BodyStatementRuleParser>(
                { TokenType::KwNamespace }, [&ctx] { return parse_namespace_statement(ctx); }),
            std::make_pair<BodyStatementRuleTokenPrefixes, BodyStatementRuleParser>(
                { TokenType::KwPublic }, [&ctx] { return parse_public_scope(ctx); }),
            std::make_pair<BodyStatementRuleTokenPrefixes, BodyStatementRuleParser>(
                { TokenType::KwUse }, [&ctx] { return parse_use_statement(ctx); }),
            std::make_pair<BodyStatementRuleTokenPrefixes, BodyStatementRuleParser>(
                { TokenType::KwWhile }, [&ctx] { return parse_while_statement(ctx); }),
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

    ParseResult parse_variable_declaration(ParserContext &ctx)
    {
        const auto kw { ctx.lexer.next() };

        const auto name { parse_name(ctx) };
        if (!name) {
            return std::unexpected(name.error());
        }

        std::optional<TypeNotationNode> tn;
        if (ctx.lexer.peek().type != TokenType::OpAssign) {
            auto tnn { parse_type_notation_node(ctx) };
            if (!tnn) {
                return std::unexpected(tnn.error());
            }
            tn.emplace(std::move(*tnn));
        }

        std::optional<Node> expression;
        if (ctx.lexer.peek().type == TokenType::OpAssign) {
            ctx.lexer.next();

            auto expr { parse_expression(ctx) };
            if (!expr) {
                return expr;
            }

            expression.emplace(std::move(*expr));
        }

        if (!tn && !expression) {
            const auto name_metadata { get_node_metadata(*name) };

            return std::unexpected(Error {
                kw.pos,
                name_metadata.pos - kw.pos + name_metadata.len,
                kw.line,
                kw.col,
                std::format("expecting either type notation or initial value are provided"),
            });
        }

        const auto tail_meta { expression.has_value() ? get_node_metadata(*expression)
                                                      : get_type_notation_metadata(tn->tn) };

        const auto pos { kw.pos };
        const auto len { tail_meta.pos - kw.pos + tail_meta.len };
        const auto line { kw.line };
        const auto col { kw.col };

        Metadata meta { pos, len, line, col };

        return std::make_unique<VariableDeclaration>(VariableDeclaration {
            std::move(*name),
            std::move(tn),
            std::move(expression),
            meta,
        });
    }

    ParseResult parse_while_statement(ParserContext &ctx)
    {
        const auto kw { ctx.lexer.next() };

        auto cond_expression { parse_expression(ctx) };
        if (!cond_expression) {
            return cond_expression;
        }

        if (const auto kw_do { expect(ctx, TokenType::KwDo) }; !kw_do) {
            return std::unexpected(kw_do.error());
        }

        auto body { parse_body(ctx, { TokenType::KwEnd, TokenType::KwElse }) };
        if (!body) {
            return std::unexpected(body.error());
        }

        const auto kw_end { expect(ctx, TokenType::KwEnd) };
        if (!kw_end) {
            return std::unexpected(kw_end.error());
        }

        return std::make_unique<WhileLoop>(WhileLoop { std::move(*cond_expression),
                                                       std::move(*body),
                                                       {
                                                           kw.pos,
                                                           kw_end->pos - kw.pos + kw_end->len,
                                                           kw.line,
                                                           kw.col,
                                                       } });
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
