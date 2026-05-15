#include <cctype>
#include <cstddef>
#include <functional>
#include <optional>
#include <vector>

#include "lexer.hpp"
#include "token.hpp"

namespace fla::compiler
{
    Lexer::Lexer(std::string_view s)
        : src(s), cursor(0), curr_line(1), curr_column(1),
          rules({
              // EOF checking should be the first to avoid out of range access
              [this] { return try_match_eof(); },

              [this] { return try_match("class", TokenType::KwClass); },
              [this] { return try_match("do", TokenType::KwDo); },
              [this] { return try_match("else", TokenType::KwElse); },
              [this] { return try_match("end", TokenType::KwEnd); },
              [this] { return try_match("fun", TokenType::KwFun); },
              [this] { return try_match("if", TokenType::KwIf); },
              [this] { return try_match("namespace", TokenType::KwNamespace); },
              [this] { return try_match("use", TokenType::KwUse); },
              [this] { return try_match("var", TokenType::KwVar); },
              [this] { return try_match("while", TokenType::KwWhile); },

              // Operators with more characters should have higher priority
              [this] { return try_match("&&", TokenType::OpAnd); },
              [this] { return try_match("==", TokenType::OpEq); },
              [this] { return try_match(">=", TokenType::OpGte); },
              [this] { return try_match("<=", TokenType::OpLte); },
              [this] { return try_match("!=", TokenType::OpNeq); },
              [this] { return try_match("||", TokenType::OpOr); },
              [this] { return try_match("+", TokenType::OpAdd); },
              [this] { return try_match("=", TokenType::OpAssign); },
              [this] { return try_match(":", TokenType::OpColon); },
              [this] { return try_match("/", TokenType::OpDiv); },
              [this] { return try_match(".", TokenType::OpDot); },
              [this] { return try_match(">", TokenType::OpGt); },
              [this] { return try_match("<", TokenType::OpLt); },
              [this] { return try_match("%", TokenType::OpMod); },
              [this] { return try_match("*", TokenType::OpMul); },
              [this] { return try_match("!", TokenType::OpNot); },
              [this] { return try_match("-", TokenType::OpSub); },

              [this] { return try_match(",", TokenType::SymComma); },
              [this] { return try_match("{", TokenType::SymLBrace); },
              [this] { return try_match("[", TokenType::SymLBrack); },
              [this] { return try_match("(", TokenType::SymLParen); },
              [this] { return try_match("}", TokenType::SymRBrace); },
              [this] { return try_match("]", TokenType::SymRBrack); },
              [this] { return try_match(")", TokenType::SymRParen); },

              [this] { return try_match_name(); },
              [this] { return try_match_number(); },
          })
    {
    }

    Token Lexer::next()
    {
        skip_whitespaces();

        for (const auto &rule : rules) {
            if (auto match = rule(); match) {
                return match.value();
            }
        }

        return {
            .type = TokenType::Unknown,
            .pos = cursor,
            .len = 0,
            .line = curr_line,
            .column = curr_column,
        };
    }

    Token Lexer::peek()
    {
        skip_whitespaces();

        auto real_cursor = cursor;
        auto real_curr_line = curr_line;
        auto real_curr_column = curr_column;

        for (const auto &rule : rules) {
            if (auto match = rule(); match) {
                // Restore states
                cursor = real_cursor;
                curr_line = real_curr_line;
                curr_column = real_curr_column;

                return match.value();
            }
        }

        return {
            .type = TokenType::Unknown,
            .pos = cursor,
            .len = 0,
            .line = curr_line,
            .column = curr_column,
        };
    }

    void Lexer::skip_whitespaces()
    {
        while (cursor < src.length() && std::isspace(current())) {
            if (current() == '\n') {
                curr_line += 1;
                curr_column = 0;
            }
            curr_column += 1;
            cursor += 1;
        }
    }

    std::optional<Token> Lexer::try_match(const std::string_view text,
                                          const TokenType type_if_matches)
    {
        if (src.substr(cursor, text.length()) != text) {
            return std::nullopt;
        }

        const Token t = {
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
        if (!is_current_valid_name_start()) {
            return std::nullopt;
        }

        const std::size_t pos { cursor };
        cursor += 1;

        std::size_t len { 1 };
        while (is_current_valid_name_tail()) {
            len += 1;
            cursor += 1;
        }

        const Token t {
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
        if (!std::isdigit(current())) {
            return std::nullopt;
        }

        const std::size_t pos { cursor };
        cursor += 1;

        std::size_t len { 1 };
        while (std::isdigit(current())) {
            len += 1;
            cursor += 1;
        }

        const Token t {
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
        if (cursor < src.length()) {
            return std::nullopt;
        }

        return Token {
            .type = TokenType::Eof,
            .pos = cursor,
            .len = 0,
            .line = curr_line,
            .column = curr_column,
        };
    }

    constexpr char Lexer::current()
    {
        return src[cursor];
    }

    bool Lexer::is_current_valid_name_start()
    {
        return std::isalpha(current()) || current() == '_';
    }

    bool Lexer::is_current_valid_name_tail()
    {
        return std::isalnum(current()) || current() == '_';
    }
}; // namespace fla::compiler
