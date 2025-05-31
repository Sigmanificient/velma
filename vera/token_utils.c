#include <stdbool.h>

#include <Python.h>

#include "vera_tokens.h"

#define BASE_CAPACITY 64

TokenVector get_tokens(
    const char *filename,
    int from_line,
    int from_column,
    int to_line,
    int to_column,
    char const **token_filter,
    int filter_size
) {
    TokenVector out = {
        .tokens = calloc(BASE_CAPACITY, sizeof *out.tokens),
        .count = 0
    };

    if (out.tokens == NULL) {
        out.count = 0;
        return out;
    }

    out.tokens[0].value = "plop";
    out.tokens[0].name = "test token";
    out.tokens[0].file = filename;
    out.count++;

    return out;
}
