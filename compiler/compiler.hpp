#ifndef FLA_COMPILER_COMPILER_H
#define FLA_COMPILER_COMPILER_H

#include <filesystem>

#include "common.hpp"

namespace fla::compiler
{
    using namespace fla::compiler::common;

    const VoidResult compile(const std::filesystem::path entrypoint);
} // namespace fla::compiler

#endif
