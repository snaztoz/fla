#include <cctype>
#include <cstddef>
#include <functional>
#include <optional>
#include <vector>

#include "lexer.hpp"
#include "token.hpp"

namespace orchid::compiler
{
  Lexer::Lexer(std::string_view s)
      : src(s), cursor(0), curr_line(1), curr_column(1)
  {
    rules = {
        [this]() { return try_match("class", TokenType::KwClass); },
        [this]() { return try_match("else", TokenType::KwElse); },
        [this]() { return try_match("fun", TokenType::KwFun); },
        [this]() { return try_match("if", TokenType::KwIf); },
        [this]() { return try_match("import", TokenType::KwImport); },
        [this]() { return try_match("package", TokenType::KwPackage); },
        [this]() { return try_match("var", TokenType::KwVar); },
        [this]() { return try_match("while", TokenType::KwWhile); },

        // Operators with more characters should have higher priority
        [this]() { return try_match("&&", TokenType::OpAnd); },
        [this]() { return try_match("==", TokenType::OpEq); },
        [this]() { return try_match(">=", TokenType::OpGte); },
        [this]() { return try_match("<=", TokenType::OpLte); },
        [this]() { return try_match("!=", TokenType::OpNeq); },
        [this]() { return try_match("||", TokenType::OpOr); },
        [this]() { return try_match("+", TokenType::OpAdd); },
        [this]() { return try_match("=", TokenType::OpAssign); },
        [this]() { return try_match(":", TokenType::OpColon); },
        [this]() { return try_match(",", TokenType::OpComma); },
        [this]() { return try_match("/", TokenType::OpDiv); },
        [this]() { return try_match(".", TokenType::OpDot); },
        [this]() { return try_match(">", TokenType::OpGt); },
        [this]() { return try_match("{", TokenType::OpLBrace); },
        [this]() { return try_match("[", TokenType::OpLBrack); },
        [this]() { return try_match("(", TokenType::OpLParen); },
        [this]() { return try_match("<", TokenType::OpLt); },
        [this]() { return try_match("%", TokenType::OpMod); },
        [this]() { return try_match("*", TokenType::OpMul); },
        [this]() { return try_match("!", TokenType::OpNot); },
        [this]() { return try_match("}", TokenType::OpRBrace); },
        [this]() { return try_match("]", TokenType::OpRBrack); },
        [this]() { return try_match(")", TokenType::OpRParen); },
        [this]() { return try_match("-", TokenType::OpSub); },

        [this]() { return try_match_name(); },
        [this]() { return try_match_number(); },
        [this]() { return try_match_eof(); },
    };
  }

  Token Lexer::next_token()
  {
    skip_whitespaces();

    for (const auto &rule : rules)
    {
      if (auto match = rule(); match)
      {
        return match.value();
      }
    }

    return Token{
        .type = TokenType::Unknown,
        .pos = cursor,
        .len = 0,
        .line = curr_line,
        .column = curr_column,
    };
  }

  char Lexer::current() { return src[cursor]; }

  void Lexer::skip_whitespaces()
  {
    while (cursor < src.length() && std::isspace(current()))
    {
      if (current() == '\n')
      {
        curr_line += 1;
        curr_column = 0;
      }
      curr_column += 1;
      cursor += 1;
    }
  }

  std::optional<Token> Lexer::try_match(std::string_view text,
                                        TokenType type_if_matches)
  {
    if (src.substr(cursor, text.length()) != text)
    {
      return std::nullopt;
    }

    Token t = {
        .type = type_if_matches,
        .pos = cursor,
        .len = text.length(),
        .line = curr_line,
        .column = curr_column,
    };

    cursor += text.length();
    curr_column += text.length();

    return t;
  }

  std::optional<Token> Lexer::try_match_name()
  {
    if (!is_current_valid_name_start())
    {
      return std::nullopt;
    }

    std::size_t pos{cursor};
    cursor += 1;

    std::size_t len{1};
    while (is_current_valid_name_tail())
    {
      len += 1;
      cursor += 1;
    }

    Token t{
        .type = TokenType::Name,
        .pos = pos,
        .len = len,
        .line = curr_line,
        .column = curr_column,
    };

    curr_column += len;

    return t;
  }

  std::optional<Token> Lexer::try_match_number()
  {
    if (!std::isdigit(current()))
    {
      return std::nullopt;
    }

    std::size_t pos{cursor};
    cursor += 1;

    std::size_t len{1};
    while (std::isdigit(current()))
    {
      len += 1;
      cursor += 1;
    }

    Token t{
        .type = TokenType::Number,
        .pos = pos,
        .len = len,
        .line = curr_line,
        .column = curr_column,
    };

    curr_column += len;

    return t;
  }

  std::optional<Token> Lexer::try_match_eof()
  {
    if (cursor < src.length())
    {
      return std::nullopt;
    }

    return Token{
        .type = TokenType::Eof,
        .pos = cursor,
        .len = 0,
        .line = curr_line,
        .column = curr_column,
    };
  }

  bool Lexer::is_current_valid_name_start()
  {
    return std::isalpha(current()) || current() == '_';
  }

  bool Lexer::is_current_valid_name_tail()
  {
    return std::isalnum(current()) || current() == '_';
  }
}; // namespace orchid::compiler
