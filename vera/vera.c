#include <Python.h>

#include "vera_tokens.h"

static char **FILE_TABLE = NULL;
static size_t FILE_TABLE_SIZE = 0;

#define FILE_TABLE_MIN_SIZE 128
#define UNUSED __attribute__((unused))

static
PyObject* py_get_tokens(PyObject* self, PyObject* args)
{
    const char *filename;
    int from_line;
    int from_column;
    int to_line;
    int to_column;
    PyObject *token_filter;

    if (!PyArg_ParseTuple(
        args,
        "siiiiO",
        &filename,
        &from_line,
        &from_column,
        &to_line,
        &to_column,
        &token_filter
    ))
        return NULL;

    Py_ssize_t filter_size = PyList_Size(token_filter);
    char const **c_token_filter = malloc(filter_size * sizeof(char *));

    for (Py_ssize_t i = 0; i < filter_size; i++) {
        PyObject *item = PyList_GetItem(token_filter, i);

        if (PyUnicode_Check(item)) {
            c_token_filter[i] = PyUnicode_AsUTF8(item);
        } else {
            free(c_token_filter);
            PyErr_SetString(PyExc_TypeError, "token_filter must be a list of strings");
            return NULL;
        }
    }

    TokenVector result = get_tokens(
        filename,
        from_line,
        from_column,
        to_line,
        to_column,
        c_token_filter,
        filter_size
    );

    free(c_token_filter);
    PyObject *py_result = PyList_New(result.count);

    for (int i = 0; i < result.count; i++) {
        PyObject *token = Py_BuildValue(
            "{s:i, s:i, s:i, s:i, s:s}",
            "start_line", result.tokens[i].start_line,
            "start_column", result.tokens[i].start_column,
            "end_line", result.tokens[i].end_line,
            "end_column", result.tokens[i].end_column,
            "name", result.tokens[i].name
        );
        PyList_SetItem(py_result, i, token);
    }
    return py_result;
}

static
PyObject* py_get_source_filenames(PyObject* self UNUSED, PyObject* args UNUSED)
{
    static char *buff = NULL;
    static size_t buffsize = 0;
    ssize_t len;
    size_t i;
    PyObject *filenames;

    if (FILE_TABLE != NULL)
        goto send_list;

    FILE_TABLE = malloc(FILE_TABLE_MIN_SIZE * sizeof *FILE_TABLE);
    if (FILE_TABLE == NULL)
        return NULL;

    FILE_TABLE_SIZE = FILE_TABLE_MIN_SIZE;
    for (i = 0; (len = getline(&buff, &buffsize, stdin)) > 1; i++) {
        FILE_TABLE[i] = strndup(buff, len - 1);
        if (FILE_TABLE[i] == NULL)
            goto failed_alloc;

        if (i == FILE_TABLE_SIZE) {
            FILE_TABLE_SIZE <<= 1;
            FILE_TABLE = reallocarray(
                FILE_TABLE, FILE_TABLE_SIZE, sizeof *FILE_TABLE
            );
        }

        if (errno == ENOMEM)
            goto failed_alloc;
    }

    FILE_TABLE_SIZE = i;

send_list:
    filenames = PyList_New(FILE_TABLE_SIZE);
    for (i = 0; i < FILE_TABLE_SIZE; i++)
        PyList_SetItem(filenames, i, PyUnicode_FromString(FILE_TABLE[i]));

    return filenames;

failed_alloc:
    for (; --i >= 0; )
        free(FILE_TABLE[i]);
    free(FILE_TABLE);
    return NULL;
}

static
PyMethodDef VERA_METHODS[] = {
    // {"getTokens", py_get_tokens, METH_VARARGS, "."},
    {"getSourceFileNames", py_get_source_filenames, METH_VARARGS, "."},
    {NULL, NULL, 0, NULL}
};

static
struct PyModuleDef VERA_MODULE = {
    PyModuleDef_HEAD_INIT,
    "vera",
    NULL,
    -1,
    VERA_METHODS
};

PyMODINIT_FUNC PyInit_vera(void)
{
    return PyModule_Create(&VERA_MODULE);
}
