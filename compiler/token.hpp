#ifndef FLA_COMPILER_TOKEN_H
#define FLA_COMPILER_TOKEN_H

#include <cstddef>
#include <utility>

namespace fla::compiler
{
    enum class TokenType {
        Unknown,
        Eof,
        False,
        Name,
        Null,
        Number,
        True,
        KwAnd,
        KwClass,
        KwDo,
        KwElse,
        KwEnd,
        KwFun,
        KwIf,
        KwNamespace,
        KwNot,
        KwOr,
        KwUse,
        KwVar,
        KwWhile,
        OpAdd,
        OpAssign,
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
        OpSub,
        SymComma,
        SymLBrace,
        SymLBrack,
        SymLParen,
        SymRBrace,
        SymRBrack,
        SymRParen,
    };

    constexpr std::string_view token_type_string(const TokenType &tt) noexcept
    {
        switch (tt) {
        case TokenType::Unknown:
            return "unknown token";
        case TokenType::Eof:
            return "EOF";
        case TokenType::False:
            return "false";
        case TokenType::Name:
            return "name";
        case TokenType::Null:
            return "null";
        case TokenType::Number:
            return "number";
        case TokenType::True:
            return "true";
        case TokenType::KwAnd:
            return "`and` keyword";
        case TokenType::KwClass:
            return "`class` keyword";
        case TokenType::KwDo:
            return "`do` keyword";
        case TokenType::KwElse:
            return "`else` keyword";
        case TokenType::KwEnd:
            return "`end` keyword";
        case TokenType::KwFun:
            return "`fun` keyword";
        case TokenType::KwIf:
            return "`if` keyword";
        case TokenType::KwNamespace:
            return "`namespace` keyword";
        case TokenType::KwNot:
            return "`not` keyword";
        case TokenType::KwOr:
            return "`or` keyword";
        case TokenType::KwUse:
            return "`use` keyword";
        case TokenType::KwVar:
            return "`var` keyword";
        case TokenType::KwWhile:
            return "`while` keyword";
        case TokenType::OpAdd:
            return "`+`";
        case TokenType::OpAssign:
            return "`=`";
        case TokenType::OpDiv:
            return "`/`";
        case TokenType::OpDot:
            return "`.`";
        case TokenType::OpEq:
            return "`==`";
        case TokenType::OpGt:
            return "`>`";
        case TokenType::OpGte:
            return "`>=`";
        case TokenType::OpLt:
            return "`<`";
        case TokenType::OpLte:
            return "`<=`";
        case TokenType::OpMod:
            return "`%`";
        case TokenType::OpMul:
            return "`*`";
        case TokenType::OpNeq:
            return "`!=`";
        case TokenType::OpSub:
            return "`-`";
        case TokenType::SymComma:
            return "`,` symbol";
        case TokenType::SymLBrace:
            return "`{` symbol";
        case TokenType::SymLBrack:
            return "`[` symbol";
        case TokenType::SymLParen:
            return "`(` symbol";
        case TokenType::SymRBrace:
            return "`}` symbol";
        case TokenType::SymRBrack:
            return "`]` symbol";
        case TokenType::SymRParen:
            return "`)` symbol";
        default:
            std::unreachable();
        }
    }

    struct Token {
        const TokenType type;
        const std::size_t pos;
        const std::size_t len;
        const std::size_t line;
        const std::size_t column;

        constexpr bool is_eof() const noexcept
        {
            return type == TokenType::Eof;
        }
    };
} // namespace fla::compiler

#endif
