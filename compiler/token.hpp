#ifndef FLA_COMPILER_TOKEN_H
#define FLA_COMPILER_TOKEN_H

#include <cstddef>
#include <string>
#include <string_view>
#include <utility>

#include "error.hpp"

namespace fla::compiler::token
{
    enum class Variant {
        Unknown,
        Eof,
        False,
        Name,
        Null,
        Number,
        True,
        KwAnd,
        KwClass,
        KwConst,
        KwDefer,
        KwDo,
        KwElse,
        KwEnd,
        KwFun,
        KwIf,
        KwInterface,
        KwNamespace,
        KwNot,
        KwOr,
        KwPublic,
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

    constexpr std::string_view type_string(const Variant &tt) noexcept
    {
        switch (tt) {
        case Variant::Unknown:
            return "unknown token";
        case Variant::Eof:
            return "EOF";
        case Variant::False:
            return "false";
        case Variant::Name:
            return "name";
        case Variant::Null:
            return "null";
        case Variant::Number:
            return "number";
        case Variant::True:
            return "true";
        case Variant::KwAnd:
            return "`and` keyword";
        case Variant::KwClass:
            return "`class` keyword";
        case Variant::KwConst:
            return "`const` keyword";
        case Variant::KwDefer:
            return "`defer` keyword";
        case Variant::KwDo:
            return "`do` keyword";
        case Variant::KwElse:
            return "`else` keyword";
        case Variant::KwEnd:
            return "`end` keyword";
        case Variant::KwFun:
            return "`fun` keyword";
        case Variant::KwIf:
            return "`if` keyword";
        case Variant::KwInterface:
            return "`interface` keyword";
        case Variant::KwNamespace:
            return "`namespace` keyword";
        case Variant::KwNot:
            return "`not` keyword";
        case Variant::KwOr:
            return "`or` keyword";
        case Variant::KwPublic:
            return "`public` keyword";
        case Variant::KwUse:
            return "`use` keyword";
        case Variant::KwVar:
            return "`var` keyword";
        case Variant::KwWhile:
            return "`while` keyword";
        case Variant::OpAdd:
            return "`+`";
        case Variant::OpAssign:
            return "`=`";
        case Variant::OpDiv:
            return "`/`";
        case Variant::OpDot:
            return "`.`";
        case Variant::OpEq:
            return "`==`";
        case Variant::OpGt:
            return "`>`";
        case Variant::OpGte:
            return "`>=`";
        case Variant::OpLt:
            return "`<`";
        case Variant::OpLte:
            return "`<=`";
        case Variant::OpMod:
            return "`%`";
        case Variant::OpMul:
            return "`*`";
        case Variant::OpNeq:
            return "`!=`";
        case Variant::OpSub:
            return "`-`";
        case Variant::SymComma:
            return "`,` symbol";
        case Variant::SymLBrace:
            return "`{` symbol";
        case Variant::SymLBrack:
            return "`[` symbol";
        case Variant::SymLParen:
            return "`(` symbol";
        case Variant::SymRBrace:
            return "`}` symbol";
        case Variant::SymRBrack:
            return "`]` symbol";
        case Variant::SymRParen:
            return "`)` symbol";
        default:
            std::unreachable();
        }
    }

    struct Token {
        const Variant variant;
        const std::size_t pos;
        const std::size_t len;
        const std::size_t line;
        const std::size_t col;

        constexpr bool is_eof() const noexcept
        {
            return variant == Variant::Eof;
        }

        inline const error::Error to_error(const std::string msg) const
        {
            return error::Error { pos, len, line, col, msg };
        }
    };
} // namespace fla::compiler::token

#endif
