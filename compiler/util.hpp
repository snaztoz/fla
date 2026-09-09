#ifndef FLA_COMPILER_UTIL_H
#define FLA_COMPILER_UTIL_H

#include <expected>
#include <filesystem>

#include "error.hpp"

namespace fla::compiler::util
{
    template <class... Ts> struct overloaded : Ts... {
        using Ts::operator()...;
    };

    const std::expected<std::string, error::Error> read_file(const std::filesystem::path &path);
} // namespace fla::compiler::util

#endif
