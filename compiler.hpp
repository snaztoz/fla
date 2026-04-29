#ifndef ORCHID_COMPILER_H
#define ORCHID_COMPILER_H

#include <string_view>

namespace orchid::compiler
{
    int compile(std::string_view src);
}

#endif
