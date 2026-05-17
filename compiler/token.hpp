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

    constexpr std::string_view token_type_string(const TokenType &tt) noexcept
    {
        switch (tt) {
        case TokenType::Unknown:
            return "unknown token";
        case TokenType::Eof:
            return "EOF";
        case TokenType::Name:
            return "name";
        case TokenType::Number:
            return "number";
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
        case TokenType::KwUse:
            return "`use` keyword";
        case TokenType::KwVar:
            return "`var` keyword";
        case TokenType::KwWhile:
            return "`while` keyword";
        case TokenType::OpAdd:
            return "`+`";
        case TokenType::OpAnd:
            return "`&&`";
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
        case TokenType::OpNot:
            return "`!`";
        case TokenType::OpOr:
            return "`||`";
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
