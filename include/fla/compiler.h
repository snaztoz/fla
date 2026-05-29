#ifndef FLA_COMPILER_H
#define FLA_COMPILER_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct FlaCompilerError {
    size_t pos;
    size_t len;
    size_t line;
    size_t col;
    char *msg;
};

int fla_compile(const char *src, struct FlaCompilerError *err);
int fla_free_compiler_error(struct FlaCompilerError *err);

#ifdef __cplusplus
}
#endif

#endif
