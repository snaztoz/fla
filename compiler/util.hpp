#ifndef FLA_COMPILER_UTIL_H
#define FLA_COMPILER_UTIL_H

#include <expected>
#include <filesystem>

namespace fla::compiler::util
{
    std::expected<std::string, std::string> read_file(std::filesystem::path path);
}

#endif
