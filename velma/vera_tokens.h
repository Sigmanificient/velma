#ifndef TOKEN_H
    #define TOKEN_H

    #include <stddef.h>

typedef struct {
    int start_line;
    int start_column;
    int end_line;
    int end_column;
    char *name;
} Token;

typedef struct {
    Token *tokens;
    int count;
} TokenVector;

TokenVector get_tokens(
    const char *filename,
    int from_line,
    int from_column,
    int to_line,
    int to_column,
    char const **token_filter,
    int filter_size
);

#endif
