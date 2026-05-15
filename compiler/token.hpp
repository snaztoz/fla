#ifndef FLA_TOKEN_H
#define FLA_TOKEN_H

#include <cstddef>

namespace fla::compiler
{
    enum class TokenType {
        Unknown,
        Eof,
        Name,
        Number,
        KwClass,
        KwDo,
        KwElse,
        KwEnd,
        KwFun,
        KwIf,
        KwNamespace,
        KwUse,
        KwVar,
        KwWhile,
        OpAdd,
        OpAnd,
        OpAssign,
        OpColon,
        OpDiv,
        OpDot,
        OpEq,
        OpGt,
        OpGte,
        OpLt,
        OpLte,
        OpMod,
        OpMul,
        OpNeq,
        OpNot,
        OpOr,
        OpSub,
        SymComma,
        SymLBrace,
        SymLBrack,
        SymLParen,
        SymRBrace,
        SymRBrack,
        SymRParen,
    };

    struct Token {
        TokenType type;
        std::size_t pos;
        std::size_t len;
        std::size_t line;
        std::size_t column;

        constexpr bool is_eof() noexcept
        {
            return type == TokenType::Eof;
        }
    };
} // namespace fla::compiler

#endif
