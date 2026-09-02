#ifndef FLA_COMPILER_CONTEXT_H
#define FLA_COMPILER_CONTEXT_H

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "ast.hpp"

namespace fla::compiler
{
    enum class TypeVariant {
        Class,
        ClassDeclaration,
        Interface,
    };

    inline std::string_view string(const TypeVariant &tv)
    {
        switch (tv) {
        case TypeVariant::Class:
            return "class";
        case TypeVariant::ClassDeclaration:
            return "class declaration";
        case TypeVariant::Interface:
            return "interface";
        default:
            std::unreachable();
        }
    }

    struct Type {
        const std::string name;
        const TypeVariant variant;
        const ast::NodeIndex ni;
        bool is_public;
    };

    using TypeMapping = std::unordered_map<std::string, Type>;

    struct Namespace {
        std::string name;
        TypeMapping types;
        TypeMapping deferred_types;
        std::vector<std::string> namespace_dependencies;
    };

    struct CompilerContext {
        std::unordered_map<std::string, Namespace> namespaces;
    };
} // namespace fla::compiler

#endif
