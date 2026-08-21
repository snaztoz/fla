#include <expected>
#include <format>
#include <memory>

#include "ast.hpp"
#include "error.hpp"
#include "parser.hpp"
#include "token.hpp"

namespace fla::compiler
{
    using TypeNotationParseResult = std::expected<TypeNotation, Error>;
    using TypeNotationListParseResult = std::expected<std::vector<TypeNotation>, Error>;

    TypeNotationParseResult parse_type_notation(ParserContext &ctx);
    TypeNotationParseResult parse_array_type_notation(ParserContext &ctx);
    TypeNotationParseResult parse_function_type_notation(ParserContext &ctx);
    TypeNotationListParseResult parse_function_type_notation_parameters(ParserContext &ctx);

    TypeNotationNodeParseResult parse_type_notation_node(ParserContext &ctx)
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
} // namespace fla::compiler
