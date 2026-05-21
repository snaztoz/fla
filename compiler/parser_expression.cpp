#include <charconv>
#include <utility>

#include "ast.hpp"
#include "lexer.hpp"
#include "parser.hpp"

namespace fla::compiler
{
    ParseResult Parser::parse_expression()
    {
        return parse_primary_expression();
    }

    ParseResult Parser::parse_primary_expression()
    {
        const auto t { lexer.peek() };

        switch (t.type) {
        case TokenType::SymLParen: {
            lexer.next();

            const auto sub_expression { parse_expression() };
            if (!sub_expression) {
                return std::unexpected(sub_expression.error());
            }

            const auto closing_paren { expect(TokenType::SymRParen) };
            if (!closing_paren) {
                return std::unexpected(closing_paren.error());
            }

            return ParseResult({
                NodeType::ExpressionGroup,
                nullptr,
                { sub_expression.value() },
                t.pos,
                closing_paren.value().pos - t.pos,
                t.line,
                t.column,
            });
        }

        case TokenType::Name: {
            lexer.next();
            return ParseResult({
                NodeType::Name,
                src.substr(t.pos, t.len),
                t.pos,
                t.len,
                t.line,
                t.column,
            });
        }

        case TokenType::Number: {
            lexer.next();

            const auto num_str = src.substr(t.pos, t.len);
            int num = 0;
            std::from_chars(num_str.data(), num_str.data() + num_str.size(), num);

            return ParseResult({
                NodeType::Number,
                num,
                t.pos,
                t.len,
                t.line,
                t.column,
            });
        }

        default:
            std::unreachable();
        }
    }
} // namespace fla::compiler
