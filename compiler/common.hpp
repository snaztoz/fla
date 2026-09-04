#ifndef FLA_COMPILER_COMMON_H
#define FLA_COMPILER_COMMON_H

#include <expected>

#include "error.hpp"

namespace fla::compiler::common
{
    using VoidResult = std::expected<void, Error>;
} // namespace fla::compiler::common

#endif
