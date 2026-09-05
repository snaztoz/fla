#ifndef FLA_COMPILER_TYPE_CHECK_H
#define FLA_COMPILER_TYPE_CHECK_H

#include <expected>

#include "ast.hpp"
#include "error.hpp"
#include "namespace.hpp"

namespace fla::compiler::type_check
{
    using Result = std::expected<ns::Namespace, error::Error>;

    const Result read_declarations(const ast::Arena &, const ast::NodeIndex);
} // namespace fla::compiler::type_check

#endif
