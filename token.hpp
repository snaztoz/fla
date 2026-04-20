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
    KwImport,
    KwPackage,
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
    const TokenType type;
    const std::size_t pos;
    const std::size_t len;
    const std::size_t line;
    const std::size_t column;
  };
} // namespace orchid::compiler

#endif
