#ifndef FLA_COMPILER_H
#define FLA_COMPILER_H

#include <string_view>

namespace fla::compiler
{
    int compile(std::string_view src);
}

#endif
