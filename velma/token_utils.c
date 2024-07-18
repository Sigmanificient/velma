#include <Python.h>

#include "vera_tokens.h"

TokenVector get_tokens(
    const char *filename,
    int from_line,
    int from_column,
    int to_line,
    int to_column,
    char const **token_filter,
    int filter_size
) {
    TokenVector token_vector = {
        .count = 0,
        .tokens = NULL
    };

    return token_vector;
}
