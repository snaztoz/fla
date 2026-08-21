#include <charconv>
#include <format>
#include <memory>
#include <set>
#include <utility>

#include "ast.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "token.hpp"

namespace fla::compiler
{
    Metadata make_binary_op_metadata(const Node &lhs, const Node &rhs)
    {
        const auto lhs_meta { get_node_metadata(lhs) };
        const auto rhs_meta { get_node_metadata(rhs) };

        const auto pos { lhs_meta.pos };
        const auto len { rhs_meta.pos - lhs_meta.pos + rhs_meta.len };
        const auto line { lhs_meta.line };
        const auto col { lhs_meta.col };

        return { pos, len, line, col };
    }

    ParseResult parse_assignment(ParserContext &ctx);
    ParseResult parse_logical_and_or_expression(ParserContext &ctx);
    ParseResult parse_equality_expression(ParserContext &ctx);
    ParseResult parse_comparison_expression(ParserContext &ctx);
    ParseResult parse_additive_expression(ParserContext &ctx);
    ParseResult parse_multiplicative_expression(ParserContext &ctx);
    ParseResult parse_unary_expression(ParserContext &ctx);
    ParseResult parse_primary_expression(ParserContext &ctx);
    ParseResult parse_if_expression(ParserContext &ctx);
    ParseResult parse_else_branch(ParserContext &ctx);

    ParseResult parse_expression(ParserContext &ctx)
    {
        return parse_assignment(ctx);
    }

    ParseResult parse_assignment(ParserContext &ctx)
    {
        auto lhs { parse_logical_and_or_expression(ctx) };
        if (!lhs) {
            return lhs;
        }

        if (const auto t { ctx.lexer.peek() }; t.type != TokenType::OpAssign) {
            return lhs;
        }
        ctx.lexer.next();

        auto rhs { parse_logical_and_or_expression(ctx) };
        if (!rhs) {
            return rhs;
        }

        const auto meta { make_binary_op_metadata(*lhs, *rhs) };

        return std::make_unique<Assign>(std::move(*lhs), std::move(*rhs), meta);
    }

    ParseResult parse_logical_and_or_expression(ParserContext &ctx)
    {
        static const std::set<TokenType> logical_and_or_ops = {
            TokenType::KwAnd,
            TokenType::KwOr,
        };

        auto expr { parse_equality_expression(ctx) };
        if (!expr) {
            return expr;
        }

        Node lhs { std::move(*expr) };

        while (true) {
            const auto op { ctx.lexer.peek() };
            if (!logical_and_or_ops.contains(op.type)) {
                break;
            }

            ctx.lexer.next();

            auto rhs { parse_equality_expression(ctx) };
            if (!rhs) {
                return std::unexpected(rhs.error());
            }

            const auto meta { make_binary_op_metadata(lhs, *rhs) };

            if (op.type == TokenType::KwAnd) {
                lhs.emplace<std::unique_ptr<And>>(
                    std::make_unique<And>(std::move(lhs), std::move(*rhs), meta));
            } else {
                lhs.emplace<std::unique_ptr<Or>>(
                    std::make_unique<Or>(std::move(lhs), std::move(*rhs), meta));
            }
        }

        return lhs;
    }

    ParseResult parse_equality_expression(ParserContext &ctx)
    {
        static const std::set<TokenType> equality_ops = {
            TokenType::OpEq,
            TokenType::OpNeq,
        };

        auto expr { parse_comparison_expression(ctx) };
        if (!expr) {
            return expr;
        }

        Node lhs { std::move(*expr) };

        while (true) {
            const auto op { ctx.lexer.peek() };
            if (!equality_ops.contains(op.type)) {
                break;
            }

            ctx.lexer.next();

            auto rhs { parse_comparison_expression(ctx) };
            if (!rhs) {
                return std::unexpected(rhs.error());
            }

            const auto meta { make_binary_op_metadata(lhs, *rhs) };

            if (op.type == TokenType::OpEq) {
                lhs.emplace<std::unique_ptr<Eq>>(
                    std::make_unique<Eq>(std::move(lhs), std::move(*rhs), meta));
            } else {
                lhs.emplace<std::unique_ptr<Neq>>(
                    std::make_unique<Neq>(std::move(lhs), std::move(*rhs), meta));
            }
        }

        return lhs;
    }

    ParseResult parse_comparison_expression(ParserContext &ctx)
    {
        static const std::set<TokenType> comparison_ops = {
            TokenType::OpGt,
            TokenType::OpGte,
            TokenType::OpLt,
            TokenType::OpLte,
        };

        auto expr { parse_additive_expression(ctx) };
        if (!expr) {
            return expr;
        }

        Node lhs { std::move(*expr) };

        while (true) {
            const auto op { ctx.lexer.peek() };
            if (!comparison_ops.contains(op.type)) {
                break;
            }

            ctx.lexer.next();

            auto rhs { parse_additive_expression(ctx) };
            if (!rhs) {
                return std::unexpected(rhs.error());
            }

            const auto meta { make_binary_op_metadata(lhs, *rhs) };

            if (op.type == TokenType::OpGt) {
                lhs.emplace<std::unique_ptr<Gt>>(
                    std::make_unique<Gt>(std::move(lhs), std::move(*rhs), meta));
            } else if (op.type == TokenType::OpGte) {
                lhs.emplace<std::unique_ptr<Gte>>(
                    std::make_unique<Gte>(std::move(lhs), std::move(*rhs), meta));
            } else if (op.type == TokenType::OpLt) {
                lhs.emplace<std::unique_ptr<Lt>>(
                    std::make_unique<Lt>(std::move(lhs), std::move(*rhs), meta));
            } else {
                lhs.emplace<std::unique_ptr<Lte>>(
                    std::make_unique<Lte>(std::move(lhs), std::move(*rhs), meta));
            }
        }

        return lhs;
    }

    ParseResult parse_additive_expression(ParserContext &ctx)
    {
        static const std::set<TokenType> additive_ops = {
            TokenType::OpAdd,
            TokenType::OpSub,
        };

        auto expr { parse_multiplicative_expression(ctx) };
        if (!expr) {
            return expr;
        }

        Node lhs { std::move(*expr) };

        while (true) {
            const auto op { ctx.lexer.peek() };
            if (!additive_ops.contains(op.type)) {
                break;
            }

            ctx.lexer.next();

            auto rhs { parse_multiplicative_expression(ctx) };
            if (!rhs) {
                return std::unexpected(rhs.error());
            }

            const auto meta { make_binary_op_metadata(lhs, *rhs) };

            if (op.type == TokenType::OpAdd) {
                lhs.emplace<std::unique_ptr<Add>>(
                    std::make_unique<Add>(std::move(lhs), std::move(*rhs), meta));
            } else {
                lhs.emplace<std::unique_ptr<Sub>>(
                    std::make_unique<Sub>(std::move(lhs), std::move(*rhs), meta));
            }
        }

        return lhs;
    }

    ParseResult parse_multiplicative_expression(ParserContext &ctx)
    {
        static const std::set<TokenType> multiplicative_ops = {
            TokenType::OpMul,
            TokenType::OpDiv,
            TokenType::OpMod,
        };

        auto expr { parse_unary_expression(ctx) };
        if (!expr) {
            return expr;
        }

        Node lhs { std::move(*expr) };

        while (true) {
            const auto op { ctx.lexer.peek() };
            if (!multiplicative_ops.contains(op.type)) {
                break;
            }

            ctx.lexer.next();

            auto rhs { parse_unary_expression(ctx) };
            if (!rhs) {
                return std::unexpected(rhs.error());
            }

            const auto meta { make_binary_op_metadata(lhs, *rhs) };

            if (op.type == TokenType::OpMul) {
                lhs.emplace<std::unique_ptr<Mul>>(
                    std::make_unique<Mul>(std::move(lhs), std::move(*rhs), meta));
            } else if (op.type == TokenType::OpDiv) {
                lhs.emplace<std::unique_ptr<Div>>(
                    std::make_unique<Div>(std::move(lhs), std::move(*rhs), meta));
            } else {
                lhs.emplace<std::unique_ptr<Mod>>(
                    std::make_unique<Mod>(std::move(lhs), std::move(*rhs), meta));
            }
        }

        return lhs;
    }

    ParseResult parse_unary_expression(ParserContext &ctx)
    {
        const auto t { ctx.lexer.peek() };

        if (t.type != TokenType::OpSub && t.type != TokenType::KwNot) {
            return parse_primary_expression(ctx);
        }

        ctx.lexer.next();

        auto sub_expression { parse_unary_expression(ctx) };
        if (!sub_expression) {
            return std::unexpected(sub_expression.error());
        }

        const auto sub_expression_meta { get_node_metadata(*sub_expression) };
        const auto len { sub_expression_meta.pos - t.pos + sub_expression_meta.len };
        const Metadata meta { t.pos, len, t.line, t.col };

        if (t.type == TokenType::OpSub) {
            return std::make_unique<Neg>(Neg { std::move(*sub_expression), meta });
        } else {
            return std::make_unique<Not>(Not { std::move(*sub_expression), meta });
        }
    }

    ParseResult parse_primary_expression(ParserContext &ctx)
    {
        const auto t { ctx.lexer.peek() };

        switch (t.type) {
        case TokenType::SymLParen: {
            ctx.lexer.next();

            auto sub_expr { parse_expression(ctx) };
            if (!sub_expr) {
                return std::unexpected(sub_expr.error());
            }

            const auto closing_paren { expect(ctx, TokenType::SymRParen) };
            if (!closing_paren) {
                return std::unexpected(closing_paren.error());
            }

            return std::make_unique<ExpressionGroup>(ExpressionGroup {
                std::move(*sub_expr),
                Metadata {
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

            return Literal { true, Metadata { t.pos, t.len, t.line, t.col } };
        }

        case TokenType::False: {
            ctx.lexer.next();

            return Literal {
                false,
                Metadata { t.pos, t.len, t.line, t.col },
            };
        }

        case TokenType::Null: {
            ctx.lexer.next();

            return Literal {
                nullptr,
                Metadata { t.pos, t.len, t.line, t.col },
            };
        }

        case TokenType::Name: {
            ctx.lexer.next();

            return Name {
                std::string { ctx.src.substr(t.pos, t.len) },
                Metadata { t.pos, t.len, t.line, t.col },
            };
        }

        case TokenType::Number: {
            ctx.lexer.next();

            const auto num_str = ctx.src.substr(t.pos, t.len);
            int num = 0;
            std::from_chars(num_str.data(), num_str.data() + num_str.size(), num);

            return Literal {
                num,
                Metadata { t.pos, t.len, t.line, t.col },
            };
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

    ParseResult parse_if_expression(ParserContext &ctx)
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

        if (ctx.lexer.peek().type != TokenType::KwElse) {
            const auto end { expect(ctx, TokenType::KwEnd) };
            if (!end) {
                return std::unexpected(end.error());
            }

            return std::make_unique<IfExpression>(IfExpression {
                std::move(*cond_expression),
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

        auto else_branch { parse_else_branch(ctx) };
        if (!else_branch) {
            return std::unexpected(else_branch.error());
        }

        const auto else_branch_metadata { get_node_metadata(*else_branch) };

        return std::make_unique<IfExpression>(IfExpression {
            std::move(*cond_expression),
            std::move(*body),
            std::move(*else_branch),
            {
                kw.pos,
                else_branch_metadata.pos - kw.pos + else_branch_metadata.len,
                kw.line,
                kw.col,
            },
        });
    }

    ParseResult parse_else_branch(ParserContext &ctx)
    {
        const auto kw { ctx.lexer.next() };

        if (ctx.lexer.peek().type == TokenType::KwIf) {
            return parse_if_expression(ctx);
        }

        if (const auto kw_do { expect(ctx, TokenType::KwDo) }; !kw_do) {
            return std::unexpected(kw_do.error());
        }

        auto body { parse_body(ctx, { TokenType::KwEnd, TokenType::KwElse }) };
        if (!body) {
            return std::unexpected(body.error());
        }

        const auto end { expect(ctx, TokenType::KwEnd) };
        if (!end) {
            return std::unexpected(end.error());
        }

        return std::make_unique<ElseBranch>(ElseBranch {
            std::move(*body),
            {
                kw.pos,
                end->pos - kw.pos + end->len,
                kw.line,
                kw.col,
            },
        });
    }
} // namespace fla::compiler
