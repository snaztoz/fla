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
        FunctionDefinition,
        FunctionParameter,
        FunctionParameterList,
        Name,
        NamespaceDeclaration,
        Root,
        TypeNotation,
        UseDeclaration,
    };

    constexpr std::string_view node_type_string(const NodeType &nt) noexcept
    {
        switch (nt) {
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

        Node(const NodeType t, const NodeValue v)
            : type(t), value(v), children({})
        {
        }

        Node(const NodeType t, const NodeValue v, const std::vector<Node> c)
            : type(t), value(v), children(c)
        {
        }
    };
} // namespace fla::compiler

#endif
