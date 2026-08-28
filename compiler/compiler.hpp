#ifndef FLA_COMPILER_COMPILER_H
#define FLA_COMPILER_COMPILER_H

#include <expected>
#include <filesystem>

#include "error.hpp"

namespace fla::compiler
{
    std::expected<void, Error> compile(const std::filesystem::path entrypoint);
}

#endif
