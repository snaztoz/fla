#include <charconv>
#include <expected>
#include <format>
#include <optional>
#include <set>
#include <string_view>
#include <utility>
#include <vector>

#include "ast.hpp"
#include "parser.hpp"
#include "token.hpp"

namespace fla::compiler::parser
{
    namespace function
    {
        using FunctionParameters = std::vector<std::pair<ast::NodeIndex, ast::NodeIndex>>;
        using FunctionParametersParseResult = std::expected<FunctionParameters, Error>;

        const Result parse(Context &ctx);
        const FunctionParametersParseResult parse_parameters(Context &ctx);
    }; // namespace function

    namespace declaration
    {
        const Result parse(Context &ctx);
    }; // namespace declaration

    namespace expression
    {
        const Result parse(Context &ctx);
    }; // namespace expression

    namespace type_notation
    {
        using TypeNotationNodeParseResult = std::expected<ast::NodeIndex, Error>;

        const TypeNotationNodeParseResult parse(Context &ctx);
    }; // namespace type_notation

    using BodyResult = std::expected<std::vector<ast::NodeIndex>, Error>;
    using NameResult = std::expected<ast::NodeIndex, Error>;
    using NestedNamesResult = std::expected<std::vector<ast::NodeIndex>, Error>;

    const Result parse_root(Context &ctx);
    const Result parse_namespace_statement(Context &ctx);
    const Result parse_use_statement(Context &ctx);
    const Result parse_public_scope(Context &ctx);
    const Result parse_class_definition(Context &ctx);
    const Result parse_interface_definition(Context &ctx);
    const NestedNamesResult parse_nested_names(Context &ctx);
    const Result parse_variable_declaration(Context &ctx);
    const Result parse_while_statement(Context &ctx);
    const Result parse_expression_statement(Context &ctx);
    const BodyResult parse_body(Context &ctx, std::set<TokenType> end_delimiters);
    const NameResult parse_name(Context &ctx);

    const std::expected<Token, Error> expect(Context &ctx, const TokenType &expected_tt);

    const Result parse(Context &ctx)
    {
        return parse_root(ctx);
    }

    const Result parse_root(Context &ctx)
    {
        auto body { parse_body(ctx, { TokenType::Eof }) };
        if (!body) {
            return std::unexpected(body.error());
        }

        if (body->size() == 0) {
            return ctx.arena.insert(ast::Root {
                std::move(*body),
                { 0, 0, 0, 0 },
            });
        }

        const auto first { ctx.arena.get_node_metadata(body->at(0)) };
        const auto last { ctx.arena.get_node_metadata(body->at(body->size() - 1)) };

        return ctx.arena.insert(ast::Root {
            std::move(*body),
            {
                first.pos,
                last.pos - first.pos + last.len,
                first.line,
                first.col,
            },
        });
    }

    const Result parse_namespace_statement(Context &ctx)
    {
        const auto kw { ctx.lexer.next() };

        const auto segments { parse_nested_names(ctx) };
        if (!segments) {
            return std::unexpected(segments.error());
        }

        const auto last { segments->at(segments->size() - 1) };
        const auto last_meta { ctx.arena.get_node_metadata(last) };

        return ctx.arena.insert(ast::NamespaceDeclaration {
            std::move(*segments),
            {
                kw.pos,
                last_meta.pos - kw.pos + last_meta.len,
                kw.line,
                kw.col,
            },
        });
    }

    const Result parse_use_statement(Context &ctx)
    {
        const auto kw { ctx.lexer.next() };

        const auto segments { parse_nested_names(ctx) };
        if (!segments) {
            return std::unexpected(segments.error());
        }

        const auto last { ctx.arena.get_node_metadata(segments->at(segments->size() - 1)) };

        return ctx.arena.insert(ast::UseDeclaration {
            std::move(*segments),
            {
                kw.pos,
                last.pos - kw.pos + last.len,
                kw.line,
                kw.col,
            },
        });
    }

    const Result parse_public_scope(Context &ctx)
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

        return ctx.arena.insert(ast::PublicScope {
            std::move(*body),
            {
                kw.pos,
                end->pos - kw.pos + end->len,
                kw.line,
                kw.col,
            },
        });
    }

    const Result parse_class_definition(Context &ctx)
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

        return ctx.arena.insert(ast::ClassDefinition {
            *name,
            std::move(*body),
            {
                kw.pos,
                end->pos - kw.pos + end->len,
                kw.line,
                kw.col,
            },
        });
    }

    const Result parse_interface_definition(Context &ctx)
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

        return ctx.arena.insert(ast::InterfaceDefinition {
            *name,
            std::move(*body),
            {
                kw.pos,
                end->pos - kw.pos + end->len,
                kw.line,
                kw.col,
            },
        });
    }

    const NestedNamesResult parse_nested_names(Context &ctx)
    {
        std::vector<ast::NodeIndex> body;

        const auto first_segment { parse_name(ctx) };
        if (!first_segment) {
            return std::unexpected(first_segment.error());
        }

        body.push_back(*first_segment);

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

            body.push_back(*next_segment);
        }

        return body;
    }

    using BodyStatementRuleTokenPrefixes = std::set<TokenType>;
    using BodyStatementRuleParser = std::function<Result(void)>;

    const BodyResult parse_body(Context &ctx, std::set<TokenType> end_delimiters)
    {
        std::vector<ast::NodeIndex> body;

        const auto rules = {
            std::make_pair<BodyStatementRuleTokenPrefixes, BodyStatementRuleParser>(
                { TokenType::KwClass }, [&ctx] { return parse_class_definition(ctx); }),
            std::make_pair<BodyStatementRuleTokenPrefixes, BodyStatementRuleParser>(
                { TokenType::KwConst, TokenType::KwVar },
                [&ctx] { return parse_variable_declaration(ctx); }),
            std::make_pair<BodyStatementRuleTokenPrefixes, BodyStatementRuleParser>(
                { TokenType::KwDefer }, [&ctx] { return parser::declaration::parse(ctx); }),
            std::make_pair<BodyStatementRuleTokenPrefixes, BodyStatementRuleParser>(
                { TokenType::KwFun }, [&ctx] { return parser::function::parse(ctx); }),
            std::make_pair<BodyStatementRuleTokenPrefixes, BodyStatementRuleParser>(
                { TokenType::KwInterface }, [&ctx] { return parse_interface_definition(ctx); }),
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

                const auto statement { parse_rule() };
                if (!statement) {
                    return std::unexpected(statement.error());
                }

                body.push_back(*statement);
                found = true;

                break;
            }

            if (found) {
                continue;
            }

            const auto statement { parse_expression_statement(ctx) };
            if (!statement) {
                return std::unexpected(statement.error());
            }

            body.push_back(*statement);
        }

        return body;
    }

    const Result parse_variable_declaration(Context &ctx)
    {
        const auto kw { ctx.lexer.next() };

        const auto name { parse_name(ctx) };
        if (!name) {
            return std::unexpected(name.error());
        }

        std::optional<ast::NodeIndex> tn;
        if (ctx.lexer.peek().type != TokenType::OpAssign) {
            const auto tnn { parser::type_notation::parse(ctx) };
            if (!tnn) {
                return std::unexpected(tnn.error());
            }
            tn = *tnn;
        }

        std::optional<ast::NodeIndex> expression;
        if (ctx.lexer.peek().type == TokenType::OpAssign) {
            ctx.lexer.next();

            const auto expr { parser::expression::parse(ctx) };
            if (!expr) {
                return expr;
            }

            expression = *expr;
        }

        if (!tn && !expression) {
            const auto name_metadata { ctx.arena.get_node_metadata(*name) };

            return std::unexpected(Error {
                kw.pos,
                name_metadata.pos - kw.pos + name_metadata.len,
                kw.line,
                kw.col,
                std::format("expecting either type notation or initial value are provided"),
            });
        }

        const auto tail_meta { expression.has_value() ? ctx.arena.get_node_metadata(*expression)
                                                      : ctx.arena.get_node_metadata(*tn) };

        const auto pos { kw.pos };
        const auto len { tail_meta.pos - kw.pos + tail_meta.len };
        const auto line { kw.line };
        const auto col { kw.col };

        return ctx.arena.insert(ast::VariableDeclaration {
            *name,
            tn,
            expression,
            { pos, len, line, col },
        });
    }

    const Result parse_while_statement(Context &ctx)
    {
        const auto kw { ctx.lexer.next() };

        const auto cond_expression { parser::expression::parse(ctx) };
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

        return ctx.arena.insert(ast::WhileLoop {
            *cond_expression,
            std::move(*body),
            {
                kw.pos,
                kw_end->pos - kw.pos + kw_end->len,
                kw.line,
                kw.col,
            },
        });
    }

    const Result parse_expression_statement(Context &ctx)
    {
        return parser::expression::parse(ctx);
    }

    const NameResult parse_name(Context &ctx)
    {
        const auto name_token { expect(ctx, TokenType::Name) };
        if (!name_token) {
            return std::unexpected(name_token.error());
        }

        const std::string name_text { ctx.src.substr(name_token->pos, name_token->len) };

        return ctx.arena.insert(ast::Name {
            name_text,
            {
                name_token->pos,
                name_token->len,
                name_token->line,
                name_token->col,
            },
        });
    }

    const std::expected<Token, Error> expect(Context &ctx, const TokenType &expected_tt)
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
} // namespace fla::compiler::parser

namespace fla::compiler::parser::declaration
{
    const Result parse_class_declaration(Context &ctx, const Token &defer_token);
    const Result parse_function_declaration(Context &ctx, const Token &defer_token);

    const Result parse(Context &ctx)
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
            return parse_class_declaration(ctx, kw);
        } else {
            return parse_function_declaration(ctx, kw);
        }
    }

    const Result parse_class_declaration(Context &ctx, const Token &defer_token)
    {
        const auto name { parse_name(ctx) };
        if (!name) {
            return name;
        }

        const auto name_metadata { ctx.arena.get_node_metadata(*name) };

        return ctx.arena.insert(ast::ClassDeclaration {
            *name,
            {
                defer_token.pos,
                name_metadata.pos - defer_token.pos + name_metadata.len,
                defer_token.line,
                defer_token.col,
            },
        });
    }

    const Result parse_function_declaration(Context &ctx, const Token &defer_token)
    {
        const auto name { parse_name(ctx) };
        if (!name) {
            return name;
        }

        parser::function::FunctionParameters parameters;

        if (ctx.lexer.peek().type == TokenType::SymLParen) {
            ctx.lexer.next();

            auto parsed_parameters { parser::function::parse_parameters(ctx) };
            if (!parsed_parameters) {
                return std::unexpected(parsed_parameters.error());
            }

            if (const auto t { expect(ctx, TokenType::SymRParen) }; !t) {
                return std::unexpected(t.error());
            }

            parameters = std::move(*parsed_parameters);
        }

        const auto return_tn { parser::type_notation::parse(ctx) };
        if (!return_tn) {
            return return_tn;
        }

        const auto return_tn_metadata { ctx.arena.get_node_metadata(*return_tn) };

        return ctx.arena.insert(ast::FunctionDeclaration {
            *name,
            std::move(parameters),
            *return_tn,
            {
                defer_token.pos,
                return_tn_metadata.pos - defer_token.pos + return_tn_metadata.len,
                defer_token.line,
                defer_token.col,
            },
        });
    }
}; // namespace fla::compiler::parser::declaration

namespace fla::compiler::parser::function
{
    const Result parse(Context &ctx)
    {
        const auto kw { ctx.lexer.next() };

        const auto name { parse_name(ctx) };
        if (!name) {
            return std::unexpected(name.error());
        }

        FunctionParameters parameters;

        if (ctx.lexer.peek().type == TokenType::SymLParen) {
            ctx.lexer.next();

            auto parsed_parameters { parse_parameters(ctx) };
            if (!parsed_parameters) {
                return std::unexpected(parsed_parameters.error());
            }

            if (const auto t { expect(ctx, TokenType::SymRParen) }; !t) {
                return std::unexpected(t.error());
            }

            parameters = std::move(*parsed_parameters);
        }

        // Return type is optional
        std::optional<ast::NodeIndex> return_tn;
        if (ctx.lexer.peek().type != TokenType::KwDo) {
            auto tn { parser::type_notation::parse(ctx) };
            if (!tn) {
                return std::unexpected(tn.error());
            }
            return_tn = *tn;
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

        return ctx.arena.insert(ast::FunctionDefinition {
            *name,
            std::move(parameters),
            return_tn,
            std::move(*body),
            {
                kw.pos,
                end->pos - kw.pos + end->len,
                kw.line,
                kw.col,
            },
        });
    }

    const FunctionParametersParseResult parse_parameters(Context &ctx)
    {
        FunctionParameters parameters;

        while (ctx.lexer.peek().type != TokenType::SymRParen) {
            const auto name { parse_name(ctx) };
            if (!name) {
                return std::unexpected(name.error());
            }

            const auto tn { parser::type_notation::parse(ctx) };
            if (!tn) {
                return std::unexpected(tn.error());
            }

            parameters.push_back(std::make_pair(*name, *tn));

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
}; // namespace fla::compiler::parser::function

namespace fla::compiler::parser::expression
{
    const ast::Metadata make_binary_op_metadata(Context &ctx, const ast::NodeIndex lhs,
                                                const ast::NodeIndex rhs)
    {
        const auto lhs_meta { ctx.arena.get_node_metadata(lhs) };
        const auto rhs_meta { ctx.arena.get_node_metadata(rhs) };

        const auto pos { lhs_meta.pos };
        const auto len { rhs_meta.pos - lhs_meta.pos + rhs_meta.len };
        const auto line { lhs_meta.line };
        const auto col { lhs_meta.col };

        return { pos, len, line, col };
    }

    const Result parse_assignment(Context &ctx);
    const Result parse_logical_and_or_expression(Context &ctx);
    const Result parse_equality_expression(Context &ctx);
    const Result parse_comparison_expression(Context &ctx);
    const Result parse_additive_expression(Context &ctx);
    const Result parse_multiplicative_expression(Context &ctx);
    const Result parse_unary_expression(Context &ctx);
    const Result parse_primary_expression(Context &ctx);
    const Result parse_if_expression(Context &ctx);
    const Result parse_else_branch(Context &ctx);

    const Result parse(Context &ctx)
    {
        return parse_assignment(ctx);
    }

    const Result parse_assignment(Context &ctx)
    {
        const auto lhs { parse_logical_and_or_expression(ctx) };
        if (!lhs) {
            return lhs;
        }

        if (const auto t { ctx.lexer.peek() }; t.type != TokenType::OpAssign) {
            return lhs;
        }
        ctx.lexer.next();

        const auto rhs { parse_logical_and_or_expression(ctx) };
        if (!rhs) {
            return rhs;
        }

        const auto meta { make_binary_op_metadata(ctx, *lhs, *rhs) };

        return ctx.arena.insert(ast::Assign { *lhs, *rhs, meta });
    }

    const Result parse_logical_and_or_expression(Context &ctx)
    {
        static const std::set<TokenType> logical_and_or_ops = {
            TokenType::KwAnd,
            TokenType::KwOr,
        };

        auto lhs { parse_equality_expression(ctx) };
        if (!lhs) {
            return lhs;
        }

        while (true) {
            const auto op { ctx.lexer.peek() };
            if (!logical_and_or_ops.contains(op.type)) {
                break;
            }

            ctx.lexer.next();

            const auto rhs { parse_equality_expression(ctx) };
            if (!rhs) {
                return std::unexpected(rhs.error());
            }

            const auto meta { make_binary_op_metadata(ctx, *lhs, *rhs) };

            if (op.type == TokenType::KwAnd) {
                lhs = ctx.arena.insert(ast::And { *lhs, *rhs, meta });
            } else {
                lhs = ctx.arena.insert(ast::Or { *lhs, *rhs, meta });
            }
        }

        return lhs;
    }

    const Result parse_equality_expression(Context &ctx)
    {
        static const std::set<TokenType> equality_ops = {
            TokenType::OpEq,
            TokenType::OpNeq,
        };

        auto lhs { parse_comparison_expression(ctx) };
        if (!lhs) {
            return lhs;
        }

        while (true) {
            const auto op { ctx.lexer.peek() };
            if (!equality_ops.contains(op.type)) {
                break;
            }

            ctx.lexer.next();

            const auto rhs { parse_comparison_expression(ctx) };
            if (!rhs) {
                return std::unexpected(rhs.error());
            }

            const auto meta { make_binary_op_metadata(ctx, *lhs, *rhs) };

            if (op.type == TokenType::OpEq) {
                lhs = ctx.arena.insert(ast::Eq { *lhs, *rhs, meta });
            } else {
                lhs = ctx.arena.insert(ast::Neq { *lhs, *rhs, meta });
            }
        }

        return lhs;
    }

    const Result parse_comparison_expression(Context &ctx)
    {
        static const std::set<TokenType> comparison_ops = {
            TokenType::OpGt,
            TokenType::OpGte,
            TokenType::OpLt,
            TokenType::OpLte,
        };

        auto lhs { parse_additive_expression(ctx) };
        if (!lhs) {
            return lhs;
        }

        while (true) {
            const auto op { ctx.lexer.peek() };
            if (!comparison_ops.contains(op.type)) {
                break;
            }

            ctx.lexer.next();

            const auto rhs { parse_additive_expression(ctx) };
            if (!rhs) {
                return std::unexpected(rhs.error());
            }

            const auto meta { make_binary_op_metadata(ctx, *lhs, *rhs) };

            if (op.type == TokenType::OpGt) {
                lhs = ctx.arena.insert(ast::Gt { *lhs, *rhs, meta });
            } else if (op.type == TokenType::OpGte) {
                lhs = ctx.arena.insert(ast::Gte { *lhs, *rhs, meta });
            } else if (op.type == TokenType::OpLt) {
                lhs = ctx.arena.insert(ast::Lt { *lhs, *rhs, meta });
            } else {
                lhs = ctx.arena.insert(ast::Lte { *lhs, *rhs, meta });
            }
        }

        return lhs;
    }

    const Result parse_additive_expression(Context &ctx)
    {
        static const std::set<TokenType> additive_ops = {
            TokenType::OpAdd,
            TokenType::OpSub,
        };

        auto lhs { parse_multiplicative_expression(ctx) };
        if (!lhs) {
            return lhs;
        }

        while (true) {
            const auto op { ctx.lexer.peek() };
            if (!additive_ops.contains(op.type)) {
                break;
            }

            ctx.lexer.next();

            const auto rhs { parse_multiplicative_expression(ctx) };
            if (!rhs) {
                return std::unexpected(rhs.error());
            }

            const auto meta { make_binary_op_metadata(ctx, *lhs, *rhs) };

            if (op.type == TokenType::OpAdd) {
                lhs = ctx.arena.insert(ast::Add { *lhs, *rhs, meta });
            } else {
                lhs = ctx.arena.insert(ast::Sub { *lhs, *rhs, meta });
            }
        }

        return lhs;
    }

    const Result parse_multiplicative_expression(Context &ctx)
    {
        static const std::set<TokenType> multiplicative_ops = {
            TokenType::OpMul,
            TokenType::OpDiv,
            TokenType::OpMod,
        };

        auto lhs { parse_unary_expression(ctx) };
        if (!lhs) {
            return lhs;
        }

        while (true) {
            const auto op { ctx.lexer.peek() };
            if (!multiplicative_ops.contains(op.type)) {
                break;
            }

            ctx.lexer.next();

            const auto rhs { parse_unary_expression(ctx) };
            if (!rhs) {
                return std::unexpected(rhs.error());
            }

            const auto meta { make_binary_op_metadata(ctx, *lhs, *rhs) };

            if (op.type == TokenType::OpMul) {
                lhs = ctx.arena.insert(ast::Mul { *lhs, *rhs, meta });
            } else if (op.type == TokenType::OpDiv) {
                lhs = ctx.arena.insert(ast::Div { *lhs, *rhs, meta });
            } else {
                lhs = ctx.arena.insert(ast::Mod { *lhs, *rhs, meta });
            }
        }

        return lhs;
    }

    const Result parse_unary_expression(Context &ctx)
    {
        const auto t { ctx.lexer.peek() };

        if (t.type != TokenType::OpSub && t.type != TokenType::KwNot) {
            return parse_primary_expression(ctx);
        }

        ctx.lexer.next();

        const auto sub_expression { parse_unary_expression(ctx) };
        if (!sub_expression) {
            return std::unexpected(sub_expression.error());
        }

        const auto sub_expression_meta { ctx.arena.get_node_metadata(*sub_expression) };
        const auto len { sub_expression_meta.pos - t.pos + sub_expression_meta.len };
        const ast::Metadata meta { t.pos, len, t.line, t.col };

        if (t.type == TokenType::OpSub) {
            return ctx.arena.insert(ast::Neg { *sub_expression, meta });
        } else {
            return ctx.arena.insert(ast::Not { *sub_expression, meta });
        }
    }

    const Result parse_primary_expression(Context &ctx)
    {
        const auto t { ctx.lexer.peek() };

        switch (t.type) {
        case TokenType::SymLParen: {
            ctx.lexer.next();

            const auto sub_expr { parser::expression::parse(ctx) };
            if (!sub_expr) {
                return std::unexpected(sub_expr.error());
            }

            const auto closing_paren { expect(ctx, TokenType::SymRParen) };
            if (!closing_paren) {
                return std::unexpected(closing_paren.error());
            }

            return ctx.arena.insert(ast::ExpressionGroup {
                *sub_expr,
                {
                    t.pos,
                    closing_paren->pos - t.pos + closing_paren->len,
                    t.line,
                    t.col,
                },
            });
        }

        case TokenType::KwIf: {
            return parse_if_expression(ctx);
        }

        case TokenType::True: {
            ctx.lexer.next();
            return ctx.arena.insert(ast::Literal {
                true,
                { t.pos, t.len, t.line, t.col },
            });
        }

        case TokenType::False: {
            ctx.lexer.next();
            return ctx.arena.insert(ast::Literal {
                false,
                { t.pos, t.len, t.line, t.col },
            });
        }

        case TokenType::Null: {
            ctx.lexer.next();

            return ctx.arena.insert(ast::Literal {
                nullptr,
                { t.pos, t.len, t.line, t.col },
            });
        }

        case TokenType::Name: {
            ctx.lexer.next();

            return ctx.arena.insert(ast::Name {
                std::string { ctx.src.substr(t.pos, t.len) },
                { t.pos, t.len, t.line, t.col },
            });
        }

        case TokenType::Number: {
            ctx.lexer.next();

            const auto num_str = ctx.src.substr(t.pos, t.len);
            int num = 0;
            std::from_chars(num_str.data(), num_str.data() + num_str.size(), num);

            return ctx.arena.insert(ast::Literal {
                num,
                { t.pos, t.len, t.line, t.col },
            });
        }

        default:
            return std::unexpected(Error {
                t.pos,
                t.len,
                t.line,
                t.col,
                std::format("expecting an expression, found {} instead", token_type_string(t.type)),
            });
        }
    }

    const Result parse_if_expression(Context &ctx)
    {
        const auto kw { ctx.lexer.next() };

        const auto cond_expression { parser::expression::parse(ctx) };
        if (!cond_expression) {
            return cond_expression;
        }

        if (const auto kw_do { expect(ctx, TokenType::KwDo) }; !kw_do) {
            return std::unexpected(kw_do.error());
        }

        auto body { fla::compiler::parser::parse_body(ctx,
                                                      { TokenType::KwEnd, TokenType::KwElse }) };
        if (!body) {
            return std::unexpected(body.error());
        }

        if (ctx.lexer.peek().type != TokenType::KwElse) {
            const auto end { expect(ctx, TokenType::KwEnd) };
            if (!end) {
                return std::unexpected(end.error());
            }

            return ctx.arena.insert(ast::IfExpression {
                *cond_expression,
                std::move(*body),
                std::nullopt,
                {
                    kw.pos,
                    end->pos - kw.pos + end->len,
                    kw.line,
                    kw.col,
                },
            });
        }

        const auto else_branch { parse_else_branch(ctx) };
        if (!else_branch) {
            return std::unexpected(else_branch.error());
        }

        const auto else_branch_metadata { ctx.arena.get_node_metadata(*else_branch) };

        return ctx.arena.insert(ast::IfExpression {
            *cond_expression,
            std::move(*body),
            *else_branch,
            {
                kw.pos,
                else_branch_metadata.pos - kw.pos + else_branch_metadata.len,
                kw.line,
                kw.col,
            },
        });
    }

    const Result parse_else_branch(Context &ctx)
    {
        const auto kw { ctx.lexer.next() };

        if (ctx.lexer.peek().type == TokenType::KwIf) {
            return parse_if_expression(ctx);
        }

        if (const auto kw_do { expect(ctx, TokenType::KwDo) }; !kw_do) {
            return std::unexpected(kw_do.error());
        }

        auto body { fla::compiler::parser::parse_body(ctx,
                                                      { TokenType::KwEnd, TokenType::KwElse }) };
        if (!body) {
            return std::unexpected(body.error());
        }

        const auto end { expect(ctx, TokenType::KwEnd) };
        if (!end) {
            return std::unexpected(end.error());
        }

        return ctx.arena.insert(ast::ElseBranch {
            std::move(*body),
            {
                kw.pos,
                end->pos - kw.pos + end->len,
                kw.line,
                kw.col,
            },
        });
    }
}; // namespace fla::compiler::parser::expression

namespace fla::compiler::parser::type_notation
{
    using TypeNotationResult = std::expected<ast::NodeIndex, Error>;
    using TypeNotationListResult = std::expected<std::vector<ast::NodeIndex>, Error>;

    const TypeNotationResult parse_type_notation(Context &ctx);
    const TypeNotationResult parse_array_type_notation(Context &ctx);
    const TypeNotationResult parse_function_type_notation(Context &ctx);
    const TypeNotationListResult parse_function_type_notation_parameters(Context &ctx);

    const TypeNotationNodeParseResult parse(Context &ctx)
    {
        const auto tn { parse_type_notation(ctx) };
        if (!tn) {
            return std::unexpected(tn.error());
        }

        const auto meta { ctx.arena.get_node_metadata(*tn) };

        return ctx.arena.insert(ast::TypeNotation { *tn, meta });
    }

    const TypeNotationResult parse_type_notation(Context &ctx)
    {
        const auto t { ctx.lexer.peek() };

        switch (t.type) {
        case TokenType::SymLBrack: {
            return parse_array_type_notation(ctx);
        }

        case TokenType::KwFun: {
            return parse_function_type_notation(ctx);
        }

        default: {
            const auto type_notation { parse_name(ctx) };
            if (!type_notation) {
                return std::unexpected(type_notation.error());
            }

            return type_notation;
        }
        }
    }

    const TypeNotationResult parse_array_type_notation(Context &ctx)
    {
        const auto t { ctx.lexer.next() };

        if (const auto closing { expect(ctx, TokenType::SymRBrack) }; !closing) {
            return std::unexpected(closing.error());
        }

        const auto inner { parse_type_notation(ctx) };
        if (!inner) {
            return std::unexpected(inner.error());
        }

        const auto meta { ctx.arena.get_node_metadata(*inner) };

        return ctx.arena.insert(ast::ArrayTypeNotation {
            *inner,
            {
                t.pos,
                meta.pos - t.pos + meta.len,
                t.line,
                t.col,
            },
        });
    }

    const TypeNotationResult parse_function_type_notation(Context &ctx)
    {
        const auto kw { ctx.lexer.next() };

        if (const auto t { expect(ctx, TokenType::SymLParen) }; !t) {
            return std::unexpected(t.error());
        }

        auto parameters_tns { parse_function_type_notation_parameters(ctx) };
        if (!parameters_tns) {
            return std::unexpected(parameters_tns.error());
        }

        if (const auto t { expect(ctx, TokenType::SymRParen) }; !t) {
            return std::unexpected(t.error());
        }

        const auto return_tn { parse_type_notation(ctx) };
        if (!return_tn) {
            return return_tn;
        }

        const auto return_tn_meta { ctx.arena.get_node_metadata(*return_tn) };

        return ctx.arena.insert(ast::FunctionTypeNotation {
            std::move(*parameters_tns),
            *return_tn,
            {
                kw.pos,
                return_tn_meta.pos - kw.pos + return_tn_meta.len,
                kw.line,
                kw.col,
            },
        });
    }

    const TypeNotationListResult parse_function_type_notation_parameters(Context &ctx)
    {
        std::vector<ast::NodeIndex> parameters_tns;

        while (ctx.lexer.peek().type != TokenType::SymRParen) {
            auto tn { parse_type_notation(ctx) };
            if (!tn) {
                return std::unexpected(tn.error());
            }

            parameters_tns.push_back(*tn);

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

        return parameters_tns;
    }
} // namespace fla::compiler::parser::type_notation
