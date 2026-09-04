#include <cctype>
#include <cstddef>
#include <functional>
#include <map>
#include <optional>

#include "lexer.hpp"
#include "token.hpp"

namespace fla::compiler::lexer
{
    const std::map<std::string_view, token::Variant> keywords {
        { "and", token::Variant::KwAnd },
        { "class", token::Variant::KwClass },
        { "const", token::Variant::KwConst },
        { "defer", token::Variant::KwDefer },
        { "do", token::Variant::KwDo },
        { "else", token::Variant::KwElse },
        { "end", token::Variant::KwEnd },
        { "fun", token::Variant::KwFun },
        { "if", token::Variant::KwIf },
        { "interface", token::Variant::KwInterface },
        { "namespace", token::Variant::KwNamespace },
        { "not", token::Variant::KwNot },
        { "or", token::Variant::KwOr },
        { "public", token::Variant::KwPublic },
        { "use", token::Variant::KwUse },
        { "var", token::Variant::KwVar },
        { "while", token::Variant::KwWhile },

        { "false", token::Variant::False },
        { "null", token::Variant::Null },
        { "true", token::Variant::True },
    };

    Lexer::Lexer(std::string_view s)
        : src(s), cursor(0), curr_line(1), curr_col(1),
          rules({
              // EOF checking should be the first to avoid out of range access
              [this] { return try_match_eof(); },

              [this] { return try_match(); },
              [this] { return try_match_number(); },

              // Operators with more characters should have higher priority
              [this] { return try_match_sym("==", token::Variant::OpEq); },
              [this] { return try_match_sym(">=", token::Variant::OpGte); },
              [this] { return try_match_sym("<=", token::Variant::OpLte); },
              [this] { return try_match_sym("!=", token::Variant::OpNeq); },
              [this] { return try_match_sym("+", token::Variant::OpAdd); },
              [this] { return try_match_sym("=", token::Variant::OpAssign); },
              [this] { return try_match_sym("/", token::Variant::OpDiv); },
              [this] { return try_match_sym(".", token::Variant::OpDot); },
              [this] { return try_match_sym(">", token::Variant::OpGt); },
              [this] { return try_match_sym("<", token::Variant::OpLt); },
              [this] { return try_match_sym("%", token::Variant::OpMod); },
              [this] { return try_match_sym("*", token::Variant::OpMul); },
              [this] { return try_match_sym("-", token::Variant::OpSub); },

              [this] { return try_match_sym(",", token::Variant::SymComma); },
              [this] { return try_match_sym("{", token::Variant::SymLBrace); },
              [this] { return try_match_sym("[", token::Variant::SymLBrack); },
              [this] { return try_match_sym("(", token::Variant::SymLParen); },
              [this] { return try_match_sym("}", token::Variant::SymRBrace); },
              [this] { return try_match_sym("]", token::Variant::SymRBrack); },
              [this] { return try_match_sym(")", token::Variant::SymRParen); },
          })
    {
    }

    const token::Token Lexer::next()
    {
        skip_whitespaces();

        for (const auto &rule : rules) {
            if (const auto match { rule() }; match) {
                return *match;
            }
        }

        return {
            .variant = token::Variant::Unknown,
            .pos = cursor,
            .len = 0,
            .line = curr_line,
            .col = curr_col,
        };
    }

    const token::Token Lexer::peek()
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
            .variant = token::Variant::Unknown,
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

    const MaybeToken Lexer::try_match_sym(const std::string_view text,
                                          const token::Variant v_if_matches)
    {
        if (cursor + text.length() > src.length()) {
            return std::nullopt;
        }

        if (src.substr(cursor, text.length()) != text) {
            return std::nullopt;
        }

        const token::Token t = {
            .variant = v_if_matches,
            .pos = cursor,
            .len = text.length(),
            .line = curr_line,
            .col = curr_col,
        };

        cursor += text.length();
        curr_col += text.length();

        return t;
    }

    const MaybeToken Lexer::try_match()
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

        const auto it { keywords.find(src.substr(pos, len)) };
        const auto variant { (it != keywords.end()) ? it->second : token::Variant::Name };

        const token::Token t {
            .variant = variant,
            .pos = pos,
            .len = len,
            .line = curr_line,
            .col = curr_col,
        };

        curr_col += len;

        return t;
    }

    const MaybeToken Lexer::try_match_number()
    {
        if (cursor >= src.length() || !std::isdigit(current())) {
            return std::nullopt;
        }

        const auto pos { cursor };
        cursor += 1;

        std::size_t len { 1 };
        while (cursor < src.length() && std::isdigit(current())) {
            len += 1;
            cursor += 1;
        }

        const token::Token t {
            .variant = token::Variant::Number,
            .pos = pos,
            .len = len,
            .line = curr_line,
            .col = curr_col,
        };

        curr_col += len;

        return t;
    }

    const MaybeToken Lexer::try_match_eof() const
    {
        if (cursor < src.length()) {
            return std::nullopt;
        }

        return token::Token {
            .variant = token::Variant::Eof,
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

    constexpr bool Lexer::is_current_valid_name_start() const
    {
        return cursor < src.length() && (std::isalpha(current()) || current() == '_');
    }

    constexpr bool Lexer::is_current_valid_name_tail() const
    {
        return cursor < src.length() && (std::isalnum(current()) || current() == '_');
    }
}; // namespace fla::compiler::lexer
