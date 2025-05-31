#ifndef TOKEN_H
    #define TOKEN_H

    #include <Python.h>

typedef struct {
    char *file;
    char *name;
    char *raw;
    char *type;
    char *value;
    int column;
    int line;
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

typedef struct {
    PyObject_HEAD
    char *file;
    char *name;
    char *raw;
    char *type;
    char *value;
    int column;
    int line;
} PyTokenObject;

#endif
