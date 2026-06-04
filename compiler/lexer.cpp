#include <cctype>
#include <cstddef>
#include <functional>
#include <map>
#include <optional>

#include "lexer.hpp"
#include "token.hpp"

namespace fla::compiler
{
    const std::map<std::string_view, TokenType> keywords {
        { "and", TokenType::KwAnd },
        { "class", TokenType::KwClass },
        { "const", TokenType::KwConst },
        { "do", TokenType::KwDo },
        { "else", TokenType::KwElse },
        { "end", TokenType::KwEnd },
        { "fun", TokenType::KwFun },
        { "if", TokenType::KwIf },
        { "namespace", TokenType::KwNamespace },
        { "not", TokenType::KwNot },
        { "or", TokenType::KwOr },
        { "use", TokenType::KwUse },
        { "var", TokenType::KwVar },
        { "while", TokenType::KwWhile },

        { "false", TokenType::False },
        { "null", TokenType::Null },
        { "true", TokenType::True },
    };

    Lexer::Lexer(std::string_view s)
        : src(s), cursor(0), curr_line(1), curr_col(1),
          rules({
              // EOF checking should be the first to avoid out of range access
              [this] { return try_match_eof(); },

              [this] { return try_match(); },
              [this] { return try_match_number(); },

              // Operators with more characters should have higher priority
              [this] { return try_match_sym("==", TokenType::OpEq); },
              [this] { return try_match_sym(">=", TokenType::OpGte); },
              [this] { return try_match_sym("<=", TokenType::OpLte); },
              [this] { return try_match_sym("!=", TokenType::OpNeq); },
              [this] { return try_match_sym("+", TokenType::OpAdd); },
              [this] { return try_match_sym("=", TokenType::OpAssign); },
              [this] { return try_match_sym("/", TokenType::OpDiv); },
              [this] { return try_match_sym(".", TokenType::OpDot); },
              [this] { return try_match_sym(">", TokenType::OpGt); },
              [this] { return try_match_sym("<", TokenType::OpLt); },
              [this] { return try_match_sym("%", TokenType::OpMod); },
              [this] { return try_match_sym("*", TokenType::OpMul); },
              [this] { return try_match_sym("-", TokenType::OpSub); },

              [this] { return try_match_sym(",", TokenType::SymComma); },
              [this] { return try_match_sym("{", TokenType::SymLBrace); },
              [this] { return try_match_sym("[", TokenType::SymLBrack); },
              [this] { return try_match_sym("(", TokenType::SymLParen); },
              [this] { return try_match_sym("}", TokenType::SymRBrace); },
              [this] { return try_match_sym("]", TokenType::SymRBrack); },
              [this] { return try_match_sym(")", TokenType::SymRParen); },
          })
    {
    }

    Token Lexer::next()
    {
        skip_whitespaces();

        for (const auto &rule : rules) {
            if (const auto match { rule() }; match) {
                return *match;
            }
        }

        return {
            .type = TokenType::Unknown,
            .pos = cursor,
            .len = 0,
            .line = curr_line,
            .col = curr_col,
        };
    }

    Token Lexer::peek()
    {
        skip_whitespaces();

        const auto real_cursor = cursor;
        const auto real_curr_line = curr_line;
        const auto real_curr_col = curr_col;

        for (const auto &rule : rules) {
            if (const auto match { rule() }; match) {
                // Restore states
                cursor = real_cursor;
                curr_line = real_curr_line;
                curr_col = real_curr_col;

                return *match;
            }
        }

        return {
            .type = TokenType::Unknown,
            .pos = cursor,
            .len = 0,
            .line = curr_line,
            .col = curr_col,
        };
    }

    void Lexer::skip_whitespaces()
    {
        while (cursor < src.length() && std::isspace(current())) {
            if (current() == '\n') {
                curr_line += 1;
                curr_col = 0;
            }
            curr_col += 1;
            cursor += 1;
        }
    }

    std::optional<Token> Lexer::try_match_sym(const std::string_view text,
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
            .col = curr_col,
        };

        cursor += text.length();
        curr_col += text.length();

        return t;
    }

    std::optional<Token> Lexer::try_match()
    {
        if (!is_current_valid_name_start()) {
            return std::nullopt;
        }

        const auto pos { cursor };
        cursor += 1;

        std::size_t len { 1 };
        while (is_current_valid_name_tail()) {
            len += 1;
            cursor += 1;
        }

        auto it = keywords.find(src.substr(pos, len));
        const auto tt { (it != keywords.end()) ? it->second : TokenType::Name };

        const Token t {
            .type = tt,
            .pos = pos,
            .len = len,
            .line = curr_line,
            .col = curr_col,
        };

        curr_col += len;

        return t;
    }

    std::optional<Token> Lexer::try_match_number()
    {
        if (!std::isdigit(current())) {
            return std::nullopt;
        }

        const auto pos { cursor };
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
            .col = curr_col,
        };

        curr_col += len;

        return t;
    }

    std::optional<Token> Lexer::try_match_eof() const
    {
        if (cursor < src.length()) {
            return std::nullopt;
        }

        return Token {
            .type = TokenType::Eof,
            .pos = cursor,
            .len = 0,
            .line = curr_line,
            .col = curr_col,
        };
    }

    constexpr char Lexer::current() const
    {
        return src[cursor];
    }

    bool Lexer::is_current_valid_name_start() const
    {
        return std::isalpha(current()) || current() == '_';
    }

    bool Lexer::is_current_valid_name_tail() const
    {
        return std::isalnum(current()) || current() == '_';
    }
}; // namespace fla::compiler
