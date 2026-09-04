#ifndef FLA_COMPILER_LEXER_H
#define FLA_COMPILER_LEXER_H

#include <cstddef>
#include <functional>
#include <optional>
#include <string_view>
#include <vector>

#include "token.hpp"

namespace fla::compiler::lexer
{
    using MaybeToken = std::optional<token::Token>;
    using Rules = std::vector<std::function<MaybeToken(void)>>;

    class Lexer
    {
    public:
        explicit Lexer(const std::string_view src);
        const token::Token next();
        const token::Token peek();

    private:
        const std::string_view src;
        std::size_t cursor;
        std::size_t curr_line;
        std::size_t curr_col;
        const Rules rules;

        const MaybeToken try_match();
        const MaybeToken try_match_number();
        const MaybeToken try_match_sym(const std::string_view, const token::Variant v_if_matches);
        const MaybeToken try_match_eof() const;
        void skip_whitespaces();
        constexpr char current() const;
        constexpr bool is_current_valid_name_start() const;
        constexpr bool is_current_valid_name_tail() const;
    };
} // namespace fla::compiler::lexer

#endif
