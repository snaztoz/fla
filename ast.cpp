#include "ast.hpp"

namespace orchid::compiler
{
    std::string_view node_type_sv(NodeType &nt)
    {
        switch (nt)
        {
        case NodeType::Root:
            return "root";
        case NodeType::NamespaceDeclaration:
            return "namespace declaration";
        case NodeType::Name:
            return "name";
        default:
            std::unreachable();
        }
    }
} // namespace orchid::compiler
