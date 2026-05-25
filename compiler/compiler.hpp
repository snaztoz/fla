#ifndef _FLA_COMPILER_H
#define _FLA_COMPILER_H

#include <string_view>

namespace fla::compiler
{
    int compile(const std::string_view src);
}

#endif
