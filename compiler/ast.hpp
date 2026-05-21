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
        ExpressionGroup,
        FunctionDefinition,
        FunctionParameter,
        FunctionParameterList,
        Name,
        NamespaceDeclaration,
        Number,
        Root,
        TypeNotation,
        UseDeclaration,
    };

    constexpr std::string_view node_type_string(const NodeType &nt) noexcept
    {
        switch (nt) {
        case NodeType::ExpressionGroup:
            return "expression group";
        case NodeType::FunctionDefinition:
            return "function definition";
        case NodeType::FunctionParameter:
            return "function parameter";
        case NodeType::FunctionParameterList:
            return "function parameter list";
        case NodeType::Name:
            return "name";
        case NodeType::NamespaceDeclaration:
            return "namespace declaration";
        case NodeType::Number:
            return "number";
        case NodeType::Root:
            return "root";
        case NodeType::TypeNotation:
            return "type notation";
        case NodeType::UseDeclaration:
            return "use declaration";
        default:
            std::unreachable();
        }
    }

    using NodeValue = std::variant<std::nullptr_t, std::string_view, int>;

    struct Node {
        const NodeType type;
        const NodeValue value;
        const std::vector<Node> children;
        const std::size_t pos;
        const std::size_t len;
        const std::size_t line;
        const std::size_t column;

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
