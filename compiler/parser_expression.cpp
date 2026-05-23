#include <array>
#include <charconv>
#include <functional>
#include <optional>
#include <span>
#include <utility>

#include "ast.hpp"
#include "lexer.hpp"
#include "parser.hpp"

namespace fla::compiler
{
    ParseResult
    parse_binary_operation(const std::span<const std::pair<TokenType, NodeType>> op_mapping,
                           const std::function<ParseResult()> operand_op, Lexer &lexer);

    ParseResult Parser::parse_expression()
    {
        return parse_assignment();
    }

    ParseResult Parser::parse_assignment()
    {
        constexpr std::array op_mapping {
            std::pair { TokenType::OpAssign, NodeType::Assign },
        };

        return parse_binary_operation(
            op_mapping, [this] { return parse_logical_and_or_expression(); }, lexer);
    }

    ParseResult Parser::parse_logical_and_or_expression()
    {
        constexpr std::array op_mapping {
            std::pair { TokenType::KwAnd, NodeType::And },
            std::pair { TokenType::KwOr, NodeType::Or },
        };

        return parse_binary_operation(
            op_mapping, [this] { return parse_equality_expression(); }, lexer);
    }

    ParseResult Parser::parse_equality_expression()
    {
        constexpr std::array op_mapping {
            std::pair { TokenType::OpEq, NodeType::Eq },
            std::pair { TokenType::OpNeq, NodeType::Neq },
        };

        return parse_binary_operation(
            op_mapping, [this] { return parse_comparison_expression(); }, lexer);
    }

    ParseResult Parser::parse_comparison_expression()
    {
        constexpr std::array op_mapping {
            std::pair { TokenType::OpGt, NodeType::Gt },
            std::pair { TokenType::OpGte, NodeType::Gte },
            std::pair { TokenType::OpLt, NodeType::Lt },
            std::pair { TokenType::OpLte, NodeType::Lte },
        };

        return parse_binary_operation(
            op_mapping, [this] { return parse_additive_expression(); }, lexer);
    }

    ParseResult Parser::parse_additive_expression()
    {
        constexpr std::array op_mapping {
            std::pair { TokenType::OpAdd, NodeType::Add },
            std::pair { TokenType::OpSub, NodeType::Sub },
        };

        return parse_binary_operation(
            op_mapping, [this] { return parse_multiplicative_expression(); }, lexer);
    }

    ParseResult Parser::parse_multiplicative_expression()
    {
        constexpr std::array op_mapping {
            std::pair { TokenType::OpMul, NodeType::Mul },
            std::pair { TokenType::OpDiv, NodeType::Div },
            std::pair { TokenType::OpMod, NodeType::Mod },
        };

        return parse_binary_operation(
            op_mapping, [this] { return parse_primary_expression(); }, lexer);
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
                len_between(t, closing_paren.value()),
                t.line,
                t.column,
            });
        }

        case TokenType::True: {
            lexer.next();
            return ParseResult({
                NodeType::Bool,
                true,
                t.pos,
                t.len,
                t.line,
                t.column,
            });
        }

        case TokenType::False: {
            lexer.next();
            return ParseResult({
                NodeType::Bool,
                false,
                t.pos,
                t.len,
                t.line,
                t.column,
            });
        }

        case TokenType::Null: {
            lexer.next();
            return ParseResult({
                NodeType::Null,
                nullptr,
                t.pos,
                t.len,
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

    ParseResult
    parse_binary_operation(const std::span<const std::pair<TokenType, NodeType>> op_mapping,
                           const std::function<ParseResult()> sub_expr, Lexer &lexer)
    {
        const auto expr { sub_expr() };
        if (!expr) {
            return expr;
        }

        auto lhs { expr.value() };

        while (true) {
            std::optional<NodeType> node_type { std::nullopt };

            const auto op { lexer.peek() };
            for (const auto &[mapping_op, mapping_node] : op_mapping) {
                if (op.type == mapping_op) {
                    node_type = mapping_node;
                    break;
                }
            }

            if (!node_type.has_value()) {
                break;
            }

            lexer.next();

            const auto rhs { sub_expr() };
            if (!rhs) {
                return rhs;
            }

            const auto pos { lhs.pos };
            const auto len { len_between(lhs, rhs.value()) };
            const auto line { lhs.line };
            const auto column { lhs.column };
            const std::vector<Node> children = { std::move(lhs), rhs.value() };

            lhs = { node_type.value(), nullptr, children, pos, len, line, column };
        }

        return lhs;
    }
} // namespace fla::compiler
