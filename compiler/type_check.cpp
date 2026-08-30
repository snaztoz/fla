#include <expected>
#include <memory>
#include <utility>
#include <variant>

#include "ast.hpp"
#include "context.hpp"
#include "error.hpp"
#include "type_check.hpp"

namespace fla::compiler::type_check
{
    bool is_missing_namespace(std::vector<Node> &body);
    std::string read_namespace_string(const Node &ns_node);
    std::unordered_map<std::string, Entity> read_public_entities(const Node &root);

    Result read_declarations(const Node &root)
    {
        Namespace ns;

        const auto *r { std::get_if<std::unique_ptr<Root>>(&root) };
        if (!r) {
            return std::unexpected(Error { 0, 0, 0, 0, "failed to match root" });
        }

        if (is_missing_namespace(r->get()->body)) {
            return std::unexpected(Error { 0, 0, 0, 0, "missing namespace declaration" });
        }

        ns.name = read_namespace_string(r->get()->body.at(0));

        return ns;
    }

    bool is_missing_namespace(std::vector<Node> &body)
    {
        return body.size() == 0 ||
               !std::holds_alternative<std::unique_ptr<NamespaceDeclaration>>(body.at(0));
    }

    std::string read_namespace_string(const Node &ns_node)
    {
        const auto *ns { std::get_if<std::unique_ptr<NamespaceDeclaration>>(&ns_node) };
        if (!ns) {
            std::unreachable();
        }

        return ns->get()->string();
    }
}; // namespace fla::compiler::type_check
