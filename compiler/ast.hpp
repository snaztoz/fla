#ifndef FLA_AST_H
#define FLA_AST_H

#include <cstddef>
#include <memory>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace fla::compiler
{
    enum class NodeType {
        Add,
        And,
        Assign,
        Bool,
        Div,
        Eq,
        ExpressionGroup,
        FunctionDefinition,
        FunctionParameter,
        FunctionParameterList,
        Gt,
        Gte,
        Lt,
        Lte,
        Mod,
        Mul,
        Name,
        NamespaceDeclaration,
        Neq,
        Not,
        Null,
        Number,
        Or,
        Root,
        Sub,
        TypeNotation,
        UseDeclaration,
    };

    constexpr std::string_view node_type_string(const NodeType &nt) noexcept
    {
        switch (nt) {
        case NodeType::Add:
            return "addition";
        case NodeType::And:
            return "and";
        case NodeType::Assign:
            return "assign";
        case NodeType::Bool:
            return "bool";
        case NodeType::Div:
            return "division";
        case NodeType::Eq:
            return "equal";
        case NodeType::ExpressionGroup:
            return "expression group";
        case NodeType::FunctionDefinition:
            return "function definition";
        case NodeType::FunctionParameter:
            return "function parameter";
        case NodeType::FunctionParameterList:
            return "function parameter list";
        case NodeType::Gt:
            return "greater than";
        case NodeType::Gte:
            return "greater than or equal";
        case NodeType::Lt:
            return "less than";
        case NodeType::Lte:
            return "less than or equal";
        case NodeType::Mod:
            return "modulo";
        case NodeType::Mul:
            return "multiplication";
        case NodeType::Name:
            return "name";
        case NodeType::NamespaceDeclaration:
            return "namespace declaration";
        case NodeType::Neq:
            return "not equal";
        case NodeType::Not:
            return "not";
        case NodeType::Null:
            return "null";
        case NodeType::Number:
            return "number";
        case NodeType::Or:
            return "or";
        case NodeType::Root:
            return "root";
        case NodeType::Sub:
            return "subtraction";
        case NodeType::TypeNotation:
            return "type notation";
        case NodeType::UseDeclaration:
            return "use declaration";
        default:
            std::unreachable();
        }
    }

    using NodeValue = std::variant<std::nullptr_t, std::string_view, int, bool>;

    struct Node {
        NodeType type;
        NodeValue value;
        std::vector<Node> children;
        std::size_t pos;
        std::size_t len;
        std::size_t line;
        std::size_t column;

        Node(const NodeType t, const NodeValue v, const std::size_t pos, const std::size_t len,
             const std::size_t line, const std::size_t column)
            : type(t), value(v), children({}), pos(pos), len(len), line(line), column(column)
        {
        }

        Node(const NodeType t, const NodeValue v, const std::vector<Node> c, const std::size_t pos,
             const std::size_t len, const std::size_t line, const std::size_t column)
            : type(t), value(v), children(c), pos(pos), len(len), line(line), column(column)
        {
        }
    };
} // namespace fla::compiler

#endif
