#ifndef FLA_COMPILER_CONTEXT_H
#define FLA_COMPILER_CONTEXT_H

#include <string>
#include <unordered_map>
#include <vector>

namespace fla::compiler
{
    enum EntityVariant {
        Class,
        Interface,
    };

    struct Entity {
        const std::string name;
        const EntityVariant variant;
    };

    struct Namespace {
        std::string name;
        std::unordered_map<std::string, Entity> public_entities;
        std::unordered_map<std::string, Entity> private_entities;
        std::vector<std::string> namespace_dependencies;
    };

    struct CompilerContext {
        std::unordered_map<std::string, Namespace> namespaces;
    };
} // namespace fla::compiler

#endif
