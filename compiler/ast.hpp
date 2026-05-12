#ifndef FLA_AST_H
#define FLA_AST_H

#include <cstddef>
#include <memory>
#include <optional>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace fla::compiler
{
    enum class NodeType
    {
        Root,
        NamespaceDeclaration,
        UseDeclaration,
        Name,
    };

    constexpr std::string_view node_type_string(const NodeType &nt) noexcept
    {
        switch (nt)
        {
        case NodeType::Root:
            return "root";
        case NodeType::NamespaceDeclaration:
            return "namespace declaration";
        case NodeType::UseDeclaration:
            return "use declaration";
        case NodeType::Name:
            return "name";
        default:
            std::unreachable();
        }
    }

    using NodeValue = std::variant<std::nullptr_t, std::string_view, int>;
    using NodeChildren = std::optional<std::vector<std::size_t>>;

    struct Node
    {
        NodeType type;
        NodeValue value;
        NodeChildren children;

        Node(NodeType t, NodeValue v)
            : type(t), value(v), children(std::nullopt)
        {
        }

        Node(NodeType t, NodeValue v, NodeChildren c)
            : type(t), value(v), children(c)
        {
        }
    };

    struct Ast
    {
        std::vector<Node> arena;
        std::size_t root;
    };
} // namespace fla::compiler

#endif
