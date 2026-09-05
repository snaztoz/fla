#ifndef FLA_COMPILER_CONTEXT_H
#define FLA_COMPILER_CONTEXT_H

#include <string>
#include <string_view>
#include <unordered_map>

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

    struct ExternalType {
        const std::string ns;
        const std::string name;
        const TypeVariant variant;
    };

    using TypeMapping = std::unordered_map<std::string, Type>;
    using ExternalTypeMapping = std::unordered_map<std::string, ExternalType>;

    struct Namespace {
        std::string name;
        TypeMapping types;
        TypeMapping deferred_types;
        ExternalTypeMapping external_types;
    };

    struct CompilerContext {
        std::unordered_map<std::string, Namespace> namespaces;
    };
} // namespace fla::compiler

#endif
