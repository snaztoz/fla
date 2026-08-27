#include <expected>
#include <format>
#include <memory>
#include <optional>
#include <set>
#include <string_view>
#include <utility>
#include <vector>

#include "ast.hpp"
#include "parser.hpp"
#include "token.hpp"

namespace fla::compiler
{
    namespace function_parser
    {
        using FunctionParameters = std::vector<std::pair<Name, Node>>;
        using FunctionParametersParseResult = std::expected<FunctionParameters, Error>;

        ParseResult parse(ParserContext &ctx);
        FunctionParametersParseResult parse_parameters(ParserContext &ctx);
    }; // namespace function_parser

    namespace forward_declaration_parser
    {
        ParseResult parse(ParserContext &ctx);
    }; // namespace forward_declaration_parser

    namespace expression_parser
    {
        ParseResult parse(ParserContext &ctx);
    }; // namespace expression_parser

    namespace type_notation_parser
    {
        using TypeNotationNodeParseResult = std::expected<TypeNotationNode, Error>;

        TypeNotationNodeParseResult parse(ParserContext &ctx);
    }; // namespace type_notation_parser

    using BodyParseResult = std::expected<std::vector<Node>, Error>;
    using NameParseResult = std::expected<Name, Error>;
    using NestedNamesParseResult = std::expected<std::vector<Name>, Error>;

    ParseResult parse_root(ParserContext &ctx);
    ParseResult parse_namespace_statement(ParserContext &ctx);
    ParseResult parse_use_statement(ParserContext &ctx);
    ParseResult parse_public_scope(ParserContext &ctx);
    ParseResult parse_class_definition(ParserContext &ctx);
    ParseResult parse_interface_definition(ParserContext &ctx);
    NestedNamesParseResult parse_nested_names(ParserContext &ctx);
    ParseResult parse_variable_declaration(ParserContext &ctx);
    ParseResult parse_while_statement(ParserContext &ctx);
    ParseResult parse_expression_statement(ParserContext &ctx);
    BodyParseResult parse_body(ParserContext &ctx, std::set<TokenType> end_delimiters);
    NameParseResult parse_name(ParserContext &ctx);

    std::expected<Token, Error> expect(ParserContext &ctx, const TokenType &expected_tt);

    ParseResult parse(const std::string_view src)
    {
        ParserContext parser_ctx { src };
        return parse_root(parser_ctx);
    }

    ParseResult parse_root(ParserContext &ctx)
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

        return std::make_unique<Root>(Root {
            std::move(*body),
            {
                first.pos,
                last.pos - first.pos + last.len,
                first.line,
                first.col,
            },
        });
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

        return std::make_unique<NamespaceDeclaration>(NamespaceDeclaration {
            std::move(*segments),
            {
                kw.pos,
                last_meta.pos - kw.pos + last_meta.len,
                kw.line,
                kw.col,
            },
        });
    }

    ParseResult parse_use_statement(ParserContext &ctx)
    {
        const auto kw { ctx.lexer.next() };

        const auto segments { parse_nested_names(ctx) };
        if (!segments) {
            return std::unexpected(segments.error());
        }

        const auto last { get_node_metadata(segments->at(segments->size() - 1)) };

        return std::make_unique<UseDeclaration>(UseDeclaration {
            std::move(*segments),
            {
                kw.pos,
                last.pos - kw.pos + last.len,
                kw.line,
                kw.col,
            },
        });
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

        return std::make_unique<PublicScope>(PublicScope {
            std::move(*body),
            {
                kw.pos,
                end->pos - kw.pos + end->len,
                kw.line,
                kw.col,
            },
        });
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

        return std::make_unique<ClassDefinition>(ClassDefinition {
            std::move(*name),
            std::move(*body),
            {
                kw.pos,
                end->pos - kw.pos + end->len,
                kw.line,
                kw.col,
            },
        });
    }

    ParseResult parse_interface_definition(ParserContext &ctx)
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

        return std::make_unique<InterfaceDefinition>(InterfaceDefinition {
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

    NestedNamesParseResult parse_nested_names(ParserContext &ctx)
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

    BodyParseResult parse_body(ParserContext &ctx, std::set<TokenType> end_delimiters)
    {
        std::vector<Node> body;

        const auto rules = {
            std::make_pair<BodyStatementRuleTokenPrefixes, BodyStatementRuleParser>(
                { TokenType::KwClass }, [&ctx] { return parse_class_definition(ctx); }),
            std::make_pair<BodyStatementRuleTokenPrefixes, BodyStatementRuleParser>(
                { TokenType::KwConst, TokenType::KwVar },
                [&ctx] { return parse_variable_declaration(ctx); }),
            std::make_pair<BodyStatementRuleTokenPrefixes, BodyStatementRuleParser>(
                { TokenType::KwDeclare },
                [&ctx] { return forward_declaration_parser::parse(ctx); }),
            std::make_pair<BodyStatementRuleTokenPrefixes, BodyStatementRuleParser>(
                { TokenType::KwFun }, [&ctx] { return function_parser::parse(ctx); }),
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
            auto tnn { type_notation_parser::parse(ctx) };
            if (!tnn) {
                return std::unexpected(tnn.error());
            }
            tn.emplace(std::move(*tnn));
        }

        std::optional<Node> expression;
        if (ctx.lexer.peek().type == TokenType::OpAssign) {
            ctx.lexer.next();

            auto expr { expression_parser::parse(ctx) };
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

        const Metadata meta { pos, len, line, col };

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

        auto cond_expression { expression_parser::parse(ctx) };
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

        return std::make_unique<WhileLoop>(WhileLoop {
            std::move(*cond_expression),
            std::move(*body),
            {
                kw.pos,
                kw_end->pos - kw.pos + kw_end->len,
                kw.line,
                kw.col,
            },
        });
    }

    ParseResult parse_expression_statement(ParserContext &ctx)
    {
        return expression_parser::parse(ctx);
    }

    NameParseResult parse_name(ParserContext &ctx)
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

namespace fla::compiler::forward_declaration_parser
{
    ParseResult parse_class_forward_declaration(ParserContext &ctx, const Token &declare_t);
    ParseResult parse_function_forward_declaration(ParserContext &ctx, const Token &declare_t);

    ParseResult parse(ParserContext &ctx)
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

    ParseResult parse_class_forward_declaration(ParserContext &ctx, const Token &declare_t)
    {
        auto name { parse_name(ctx) };
        if (!name) {
            return name;
        }

        const auto name_metadata { get_node_metadata(*name) };

        return std::make_unique<ClassForwardDeclaration>(ClassForwardDeclaration {
            std::move(*name),
            {
                declare_t.pos,
                name_metadata.pos - declare_t.pos + name_metadata.len,
                declare_t.line,
                declare_t.col,
            },
        });
    }

    ParseResult parse_function_forward_declaration(ParserContext &ctx, const Token &declare_t)
    {
        auto name { parse_name(ctx) };
        if (!name) {
            return name;
        }

        function_parser::FunctionParameters parameters;

        if (ctx.lexer.peek().type == TokenType::SymLParen) {
            ctx.lexer.next();

            auto parsed_parameters { function_parser::parse_parameters(ctx) };
            if (!parsed_parameters) {
                return std::unexpected(parsed_parameters.error());
            }

            if (const auto t { expect(ctx, TokenType::SymRParen) }; !t) {
                return std::unexpected(t.error());
            }

            parameters = std::move(*parsed_parameters);
        }

        auto return_tn { type_notation_parser::parse(ctx) };
        if (!return_tn) {
            return return_tn;
        }

        const auto return_tn_metadata { get_type_notation_metadata(return_tn->tn) };

        return std::make_unique<FunctionForwardDeclaration>(FunctionForwardDeclaration {
            std::move(*name),
            std::move(parameters),
            std::move(*return_tn),
            {
                declare_t.pos,
                return_tn_metadata.pos - declare_t.pos + return_tn_metadata.len,
                declare_t.line,
                declare_t.col,
            },
        });
    }
}; // namespace fla::compiler::forward_declaration_parser

namespace fla::compiler::function_parser
{
    ParseResult parse(ParserContext &ctx)
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
        std::optional<TypeNotationNode> return_tn;
        if (ctx.lexer.peek().type != TokenType::KwDo) {
            auto tn { type_notation_parser::parse(ctx) };
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

        return std::make_unique<FunctionDefinition>(FunctionDefinition {
            std::move(*name),
            std::move(parameters),
            std::move(return_tn),
            std::move(*body),
            {
                kw.pos,
                end->pos - kw.pos + end->len,
                kw.line,
                kw.col,
            },
        });
    }

    FunctionParametersParseResult parse_parameters(ParserContext &ctx)
    {
        FunctionParameters parameters;

        while (ctx.lexer.peek().type != TokenType::SymRParen) {
            const auto name { parse_name(ctx) };
            if (!name) {
                return std::unexpected(name.error());
            }

            auto tn { type_notation_parser::parse(ctx) };
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
}; // namespace fla::compiler::function_parser

namespace fla::compiler::expression_parser
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

    ParseResult parse(ParserContext &ctx)
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

            auto sub_expr { parse(ctx) };
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

        auto cond_expression { parse(ctx) };
        if (!cond_expression) {
            return cond_expression;
        }

        if (const auto kw_do { expect(ctx, TokenType::KwDo) }; !kw_do) {
            return std::unexpected(kw_do.error());
        }

        auto body { fla::compiler::parse_body(ctx, { TokenType::KwEnd, TokenType::KwElse }) };
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

        auto body { fla::compiler::parse_body(ctx, { TokenType::KwEnd, TokenType::KwElse }) };
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
}; // namespace fla::compiler::expression_parser

namespace fla::compiler::type_notation_parser
{
    using TypeNotationParseResult = std::expected<TypeNotation, Error>;
    using TypeNotationListParseResult = std::expected<std::vector<TypeNotation>, Error>;

    TypeNotationParseResult parse_type_notation(ParserContext &ctx);
    TypeNotationParseResult parse_array_type_notation(ParserContext &ctx);
    TypeNotationParseResult parse_function_type_notation(ParserContext &ctx);
    TypeNotationListParseResult parse_function_type_notation_parameters(ParserContext &ctx);

    TypeNotationNodeParseResult parse(ParserContext &ctx)
    {
        auto tn { parse_type_notation(ctx) };
        if (!tn) {
            return std::unexpected(tn.error());
        }

        const auto meta { get_type_notation_metadata(*tn) };

        return TypeNotationNode { std::move(*tn), meta };
    }

    TypeNotationParseResult parse_type_notation(ParserContext &ctx)
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

    TypeNotationParseResult parse_array_type_notation(ParserContext &ctx)
    {
        const auto t { ctx.lexer.next() };

        if (const auto closing { expect(ctx, TokenType::SymRBrack) }; !closing) {
            return std::unexpected(closing.error());
        }

        auto inner { parse_type_notation(ctx) };
        if (!inner) {
            return std::unexpected(inner.error());
        }

        const auto meta { get_type_notation_metadata(*inner) };

        return std::make_unique<ArrayTypeNotation>(ArrayTypeNotation {
            std::move(*inner),
            {
                t.pos,
                meta.pos - t.pos + meta.len,
                t.line,
                t.col,
            },
        });
    }

    TypeNotationParseResult parse_function_type_notation(ParserContext &ctx)
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

        auto return_tn { parse_type_notation(ctx) };
        if (!return_tn) {
            return return_tn;
        }

        const auto return_tn_meta { get_type_notation_metadata(*return_tn) };

        return std::make_unique<FunctionTypeNotation>(FunctionTypeNotation {
            std::move(*parameters_tns),
            std::move(*return_tn),
            {
                kw.pos,
                return_tn_meta.pos - kw.pos + return_tn_meta.len,
                kw.line,
                kw.col,
            },
        });
    }

    TypeNotationListParseResult parse_function_type_notation_parameters(ParserContext &ctx)
    {
        std::vector<TypeNotation> parameters_tns;

        while (ctx.lexer.peek().type != TokenType::SymRParen) {
            auto tn { parse_type_notation(ctx) };
            if (!tn) {
                return std::unexpected(tn.error());
            }

            parameters_tns.push_back(std::move(*tn));

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
} // namespace fla::compiler::type_notation_parser
