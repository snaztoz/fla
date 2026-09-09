#include <expected>
#include <format>
#include <sstream>
#include <utility>
#include <variant>

#include "ast.hpp"
#include "common.hpp"
#include "error.hpp"
#include "namespace.hpp"
#include "type_check.hpp"

namespace fla::compiler::type_check
{
    using namespace fla::compiler::common;

    using TypeMappingResult = std::expected<ns::TypeMapping, error::Error>;
    using ExternalTypeMappingResult = std::expected<ns::ExternalTypeMapping, error::Error>;

    bool is_missing_namespace(const Context &);
    const std::string read_namespace_string(const Context &);

    TypeMappingResult read_public_deferred_types(const Context &);
    TypeMappingResult read_deferred_types(const Context &,
                                          const std::vector<ast::NodeIndex> &scope_body,
                                          const bool is_public);

    TypeMappingResult read_public_types(const Context &);
    TypeMappingResult read_types(const Context &, const std::vector<ast::NodeIndex> &scope_body,
                                 const bool is_public);

    ExternalTypeMappingResult read_external_types(const Context &);

    const VoidResult resolve_deferred_types(ns::Namespace &, const ast::Arena &);
    constexpr bool should_promote_visibility(const ns::Type &concrete_type,
                                             const ns::Type &deferred_type);

    const Result read_declarations(const Context &ctx)
    {
        ns::Namespace ns;

        if (is_missing_namespace(ctx)) {
            return std::unexpected(error::Error { 0, 0, 0, 0, "missing namespace declaration" });
        }

        ns.name = read_namespace_string(ctx);

        auto uses { read_external_types(ctx) };
        if (!uses) {
            return std::unexpected(uses.error());
        }
        ns.external_types.merge(*uses);

        auto dt { read_deferred_types(ctx, ctx.root.body, false) };
        if (!dt) {
            return std::unexpected(dt.error());
        }
        ns.deferred_types.merge(*dt);

        auto pdt { read_public_deferred_types(ctx) };
        if (!pdt) {
            return std::unexpected(pdt.error());
        }
        ns.deferred_types.merge(*pdt);

        auto t { read_types(ctx, ctx.root.body, false) };
        if (!t) {
            return std::unexpected(t.error());
        }
        ns.types.merge(*t);

        auto pt { read_public_types(ctx) };
        if (!pt) {
            return std::unexpected(pt.error());
        }
        ns.types.merge(*pt);

        if (const auto ok { resolve_deferred_types(ns, ctx.arena) }; !ok) {
            return std::unexpected(ok.error());
        }

        return ns;
    }

    bool is_missing_namespace(const Context &ctx)
    {
        return ctx.root.body.size() == 0 || !std::holds_alternative<ast::NamespaceDeclaration>(
                                                ctx.arena.get(ctx.root.body.at(0)));
    }

    const std::string read_namespace_string(const Context &ctx)
    {
        const auto *ns { std::get_if<ast::NamespaceDeclaration>(
            &ctx.arena.get(ctx.root.body.at(0))) };
        if (!ns) {
            std::unreachable();
        }

        return ns->string(ctx.arena);
    }

    TypeMappingResult read_public_deferred_types(const Context &ctx)
    {
        ns::TypeMapping types;

        for (const auto &n : ctx.root.body) {
            const auto *public_node { std::get_if<ast::PublicScope>(&ctx.arena.get(n)) };
            if (!public_node) {
                continue;
            }

            auto scope_types { read_deferred_types(ctx, public_node->body, true) };
            if (!scope_types) {
                return std::unexpected(scope_types.error());
            }

            types.merge(*scope_types);
        }

        return types;
    }

    TypeMappingResult read_deferred_types(const Context &ctx,
                                          const std::vector<ast::NodeIndex> &scope_body,
                                          const bool is_public)
    {
        ns::TypeMapping types;

        for (const auto &n : scope_body) {
            if (const auto *t { std::get_if<ast::ClassDeclaration>(&ctx.arena.get(n)) }) {
                const auto *name { std::get_if<ast::Name>(&ctx.arena.get(t->name)) };

                if (types.contains(name->name)) {
                    const std::string msg { std::format("`{}` type is already exist", name->name) };
                    return std::unexpected(name->meta.to_error(msg));
                }

                types.insert({
                    name->name,
                    { name->name, ns::TypeVariant::ClassDeclaration, n, is_public },
                });

                continue;
            }
        }

        return types;
    }

    TypeMappingResult read_public_types(const Context &ctx)
    {
        ns::TypeMapping types;

        for (const auto &n : ctx.root.body) {
            const auto *public_node { std::get_if<ast::PublicScope>(&ctx.arena.get(n)) };
            if (!public_node) {
                continue;
            }

            auto scope_types { read_types(ctx, public_node->body, true) };
            if (!scope_types) {
                return std::unexpected(scope_types.error());
            }

            types.merge(*scope_types);
        }

        return types;
    }

    TypeMappingResult read_types(const Context &ctx, const std::vector<ast::NodeIndex> &scope_body,
                                 const bool is_public)
    {
        ns::TypeMapping types;

        for (const auto &n : scope_body) {
            if (const auto *t { std::get_if<ast::ClassDefinition>(&ctx.arena.get(n)) }) {
                const auto *name { std::get_if<ast::Name>(&ctx.arena.get(t->name)) };

                if (types.contains(name->name)) {
                    const std::string msg { std::format("`{}` type is already exist", name->name) };
                    return std::unexpected(name->meta.to_error(msg));
                }

                types.insert({
                    name->name,
                    { name->name, ns::TypeVariant::Class, n, is_public },
                });

                continue;
            }

            if (const auto *t { std::get_if<ast::InterfaceDefinition>(&ctx.arena.get(n)) }) {
                const auto *name { std::get_if<ast::Name>(&ctx.arena.get(t->name)) };

                if (types.contains(name->name)) {
                    const std::string msg { std::format("`{}` type is already exist", name->name) };
                    return std::unexpected(name->meta.to_error(msg));
                }

                types.insert({
                    name->name,
                    { name->name, ns::TypeVariant::Interface, n, is_public },
                });

                continue;
            }
        }

        return types;
    }

    ExternalTypeMappingResult read_external_types(const Context &ctx)
    {
        ns::ExternalTypeMapping types;

        for (const auto &n : ctx.root.body) {
            const auto *use { std::get_if<ast::UseDeclaration>(&ctx.arena.get(n)) };
            if (!use) {
                continue;
            }

            if (use->name_segments.size() < 2) {
                const auto meta { ctx.arena.node_metadata(n) };
                return std::unexpected(meta.to_error("invalid use segments"));
            }

            const auto ns_start { 0 };
            const auto ns_end { use->name_segments.size() - 2 };

            std::ostringstream ns;
            for (std::size_t i { ns_start }; i <= ns_end; i++) {
                const auto *segment { std::get_if<ast::Name>(
                    &ctx.arena.get(use->name_segments.at(i))) };

                ns << segment->name;
                if (i < ns_end) {
                    ns << ".";
                }
            }

            const auto type_i { use->name_segments.size() - 1 };
            const auto type_node { ctx.arena.get(use->name_segments.at(type_i)) };
            const auto *type_name { std::get_if<ast::Name>(&type_node) };

            types.insert({
                ns.str(),
                {
                    ns.str(),
                    type_name->name,
                    ns::TypeVariant::Class,
                },
            });
        }

        return types;
    }

    const VoidResult resolve_deferred_types(ns::Namespace &ns, const ast::Arena &arena)
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

    constexpr bool should_promote_visibility(const ns::Type &concrete_type,
                                             const ns::Type &deferred_type)
    {
        return deferred_type.is_public && !concrete_type.is_public;
    }
}; // namespace fla::compiler::type_check
