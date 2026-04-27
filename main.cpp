#include "compiler.hpp"

int main(int argc, char const *argv[])
{
    orchid::compiler::compile("namespace foo.bar");

    return 0;
}
