#ifndef FLA_COMPILER_LEXER_H
#define FLA_COMPILER_LEXER_H

#include <cstddef>
#include <functional>
#include <optional>
#include <string_view>
#include <vector>

#include "token.hpp"

namespace fla::compiler
{
    using LexerRules = std::vector<std::function<std::optional<Token>(void)>>;

    class Lexer
    {
    public:
        explicit Lexer(const std::string_view src);
        Token next();
        Token peek();

    private:
        const std::string_view src;
        std::size_t cursor;
        std::size_t curr_line;
        std::size_t curr_column;
        const LexerRules rules;

        std::optional<Token> try_match(const std::string_view text,
                                       const TokenType type_if_matches);
        std::optional<Token> try_match_name();
        std::optional<Token> try_match_number();
        std::optional<Token> try_match_eof() const;
        void skip_whitespaces();
        constexpr char current() const;
        bool is_current_valid_name_start() const;
        bool is_current_valid_name_tail() const;
    };
} // namespace fla::compiler

#endif
