#ifndef FLA_COMPILER_COMPILER_H
#define FLA_COMPILER_COMPILER_H

#include <filesystem>
#include <unordered_map>

#include "common.hpp"
#include "namespace.hpp"

namespace fla::compiler
{
    using namespace fla::compiler::common;

    struct CompilerContext {
        std::unordered_map<std::string, ns::Namespace> namespaces;
    };

    const VoidResult compile(const std::filesystem::path entrypoint);
} // namespace fla::compiler

#endif
