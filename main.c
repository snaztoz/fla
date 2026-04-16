#include <stdio.h>

#include "compiler.h"

int main(int argc, char const *argv[])
{
    printf("Hello, World!\n");
    orchid_compile("  package orchid. std");
    return 0;
}
