#ifndef ORCHID_TOKEN_H
#define ORCHID_TOKEN_H

#include <cstddef>

namespace orchid::compiler
{
    enum class TokenType
    {
        Unknown,
        Eof,
        Name,
        Number,
        KwClass,
        KwElse,
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
        OpComma,
        OpDiv,
        OpDot,
        OpEq,
        OpGt,
        OpGte,
        OpLBrace,
        OpLBrack,
        OpLParen,
        OpLt,
        OpLte,
        OpMod,
        OpMul,
        OpNeq,
        OpNot,
        OpOr,
        OpRBrace,
        OpRBrack,
        OpRParen,
        OpSub,
    };

    struct Token
    {
        TokenType type;
        std::size_t pos;
        std::size_t len;
        std::size_t line;
        std::size_t column;
    };
} // namespace orchid::compiler

#endif
