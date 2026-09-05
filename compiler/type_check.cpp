#include <expected>
#include <format>
#include <sstream>
#include <utility>
#include <variant>

#include "ast.hpp"
#include "common.hpp"
#include "context.hpp"
#include "error.hpp"
#include "type_check.hpp"

namespace fla::compiler::type_check
{
    using namespace fla::compiler::common;

    using TypeMappingResult = std::expected<TypeMapping, error::Error>;
    using ExternalTypeMappingResult = std::expected<ExternalTypeMapping, error::Error>;

    bool is_missing_namespace(const ast::Arena &, const ast::NodeList &);
    const std::string read_namespace_string(const ast::Arena &, const ast::Root &);

    TypeMappingResult read_public_deferred_types(const ast::Arena &, const ast::NodeList &);
    TypeMappingResult read_deferred_types(const ast::Arena &, const ast::NodeList &,
                                          const bool is_public);

    TypeMappingResult read_public_types(const ast::Arena &, const ast::NodeList &);
    TypeMappingResult read_types(const ast::Arena &, const ast::NodeList &, const bool is_public);

    ExternalTypeMappingResult read_external_types(const ast::Arena &, const ast::NodeList &);

    const VoidResult resolve_deferred_types(Namespace &, const ast::Arena &);
    constexpr bool should_promote_visibility(const Type &concrete_type, const Type &deferred_type);

    const Result read_declarations(const ast::Arena &arena, const ast::NodeIndex root_ni)
    {
        Namespace ns;

        const auto *r { std::get_if<ast::Root>(&arena.get(root_ni)) };
        if (!r) {
            return std::unexpected(error::Error { 0, 0, 0, 0, "failed to match root" });
        }

        if (is_missing_namespace(arena, r->body)) {
            return std::unexpected(error::Error { 0, 0, 0, 0, "missing namespace declaration" });
        }

        ns.name = read_namespace_string(arena, *r);

        auto uses { read_external_types(arena, r->body) };
        if (!uses) {
            return std::unexpected(uses.error());
        }
        ns.external_types.merge(*uses);

        auto dt { read_deferred_types(arena, r->body, false) };
        if (!dt) {
            return std::unexpected(dt.error());
        }
        ns.deferred_types.merge(*dt);

        auto pdt { read_public_deferred_types(arena, r->body) };
        if (!pdt) {
            return std::unexpected(pdt.error());
        }
        ns.deferred_types.merge(*pdt);

        auto t { read_types(arena, r->body, false) };
        if (!t) {
            return std::unexpected(t.error());
        }
        ns.types.merge(*t);

        auto pt { read_public_types(arena, r->body) };
        if (!pt) {
            return std::unexpected(pt.error());
        }
        ns.types.merge(*pt);

        if (const auto ok { resolve_deferred_types(ns, arena) }; !ok) {
            return std::unexpected(ok.error());
        }

        return ns;
    }

    bool is_missing_namespace(const ast::Arena &arena, const ast::NodeList &body)
    {
        return body.size() == 0 ||
               !std::holds_alternative<ast::NamespaceDeclaration>(arena.get(body.at(0)));
    }

    const std::string read_namespace_string(const ast::Arena &arena, const ast::Root &root)
    {
        const auto *ns { std::get_if<ast::NamespaceDeclaration>(&arena.get(root.body.at(0))) };
        if (!ns) {
            std::unreachable();
        }

        return ns->string(arena);
    }

    TypeMappingResult read_public_deferred_types(const ast::Arena &arena, const ast::NodeList &body)
    {
        TypeMapping types;

        for (const auto &n : body) {
            const auto *public_node { std::get_if<ast::PublicScope>(&arena.get(n)) };
            if (!public_node) {
                continue;
            }

            auto scope_types { read_deferred_types(arena, public_node->body, true) };
            if (!scope_types) {
                return std::unexpected(scope_types.error());
            }

            types.merge(*scope_types);
        }

        return types;
    }

    TypeMappingResult read_deferred_types(const ast::Arena &arena, const ast::NodeList &body,
                                          const bool is_public)
    {
        TypeMapping types;

        for (const auto &n : body) {
            if (const auto *t { std::get_if<ast::ClassDeclaration>(&arena.get(n)) }) {
                const auto *name { std::get_if<ast::Name>(&arena.get(t->name)) };

                if (types.contains(name->name)) {
                    const std::string msg { std::format("`{}` type is already exist", name->name) };
                    return std::unexpected(name->meta.to_error(msg));
                }

                types.insert({
                    name->name,
                    { name->name, TypeVariant::ClassDeclaration, n, is_public },
                });

                continue;
            }
        }

        return types;
    }

    TypeMappingResult read_public_types(const ast::Arena &arena, const ast::NodeList &body)
    {
        TypeMapping types;

        for (const auto &n : body) {
            const auto *public_node { std::get_if<ast::PublicScope>(&arena.get(n)) };
            if (!public_node) {
                continue;
            }

            auto scope_types { read_types(arena, public_node->body, true) };
            if (!scope_types) {
                return std::unexpected(scope_types.error());
            }

            types.merge(*scope_types);
        }

        return types;
    }

    TypeMappingResult read_types(const ast::Arena &arena, const ast::NodeList &body,
                                 const bool is_public)
    {
        TypeMapping types;

        for (const auto &n : body) {
            if (const auto *t { std::get_if<ast::ClassDefinition>(&arena.get(n)) }) {
                const auto *name { std::get_if<ast::Name>(&arena.get(t->name)) };

                if (types.contains(name->name)) {
                    const std::string msg { std::format("`{}` type is already exist", name->name) };
                    return std::unexpected(name->meta.to_error(msg));
                }

                types.insert({
                    name->name,
                    { name->name, TypeVariant::Class, n, is_public },
                });

                continue;
            }

            if (const auto *t { std::get_if<ast::InterfaceDefinition>(&arena.get(n)) }) {
                const auto *name { std::get_if<ast::Name>(&arena.get(t->name)) };

                if (types.contains(name->name)) {
                    const std::string msg { std::format("`{}` type is already exist", name->name) };
                    return std::unexpected(name->meta.to_error(msg));
                }

                types.insert({
                    name->name,
                    { name->name, TypeVariant::Interface, n, is_public },
                });

                continue;
            }
        }

        return types;
    }

    ExternalTypeMappingResult read_external_types(const ast::Arena &arena,
                                                  const ast::NodeList &body)
    {
        ExternalTypeMapping types;

        for (const auto &n : body) {
            const auto *use { std::get_if<ast::UseDeclaration>(&arena.get(n)) };
            if (!use) {
                continue;
            }

            if (use->name_segments.size() < 2) {
                const auto meta { arena.node_metadata(n) };
                return std::unexpected(meta.to_error("invalid use segments"));
            }

            const auto ns_start { 0 };
            const auto ns_end { use->name_segments.size() - 2 };

            std::ostringstream ns;
            for (std::size_t i { ns_start }; i <= ns_end; i++) {
                const auto *segment { std::get_if<ast::Name>(
                    &arena.get(use->name_segments.at(i))) };

                ns << segment->name;
                if (i < ns_end) {
                    ns << ".";
                }
            }

            const auto type_i { use->name_segments.size() - 1 };
            const auto type_node { arena.get(use->name_segments.at(type_i)) };
            const auto *type_name { std::get_if<ast::Name>(&type_node) };

            types.insert({
                ns.str(),
                {
                    ns.str(),
                    type_name->name,
                    TypeVariant::Class,
                },
            });
        }

        return types;
    }

    const VoidResult resolve_deferred_types(Namespace &ns, const ast::Arena &arena)
    {
        for (const auto &[name, t] : ns.deferred_types) {
            if (!ns.types.contains(name)) {
                const auto ni { ns.deferred_types.at(name).ni };
                const auto meta { arena.node_metadata(ni) };
                return std::unexpected(meta.to_error("missing class definition"));
            }

            if (should_promote_visibility(ns.types.at(name), t)) {
                ns.types.at(name).is_public = true;
            }
        }

        return {};
    }

    constexpr bool should_promote_visibility(const Type &concrete_type, const Type &deferred_type)
    {
        return deferred_type.is_public && !concrete_type.is_public;
    }
}; // namespace fla::compiler::type_check
