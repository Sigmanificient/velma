#include <stdbool.h>
#include <stdio.h>

#include <fcntl.h>
#include <sys/stat.h>

#include <Python.h>

#include "internal.h"
#include "vera_tokens.h"

static PyObject *FILE_TABLE = NULL;
static Py_ssize_t FILE_TABLE_SIZE = 0;

#define FILE_TABLE_MIN_SIZE 128
#define UNUSED __attribute__((unused))

static
PyObject *py_get_tokens(PyObject *self UNUSED, PyObject *args)
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
PyObject *py_register_sources(PyObject *self UNUSED, PyObject *args)
{
    PyObject *input_list;
    Py_ssize_t list_size;

    if (!PyArg_ParseTuple(args, "O", &input_list))
        return NULL;

    list_size = PyList_Size(input_list);
    if (FILE_TABLE != NULL)
        Py_DECREF(FILE_TABLE);

    FILE_TABLE = input_list;
    FILE_TABLE_SIZE = list_size;
    Py_INCREF(FILE_TABLE);

    Py_RETURN_TRUE;
}

static
PyObject *py_get_source_filenames(PyObject *self UNUSED, PyObject *args UNUSED)
{
    if (FILE_TABLE == NULL)
        exit(EXIT_FAILURE);
    Py_INCREF(FILE_TABLE);
    return FILE_TABLE;
}

static
PyObject *py_is_binary(PyObject *self UNUSED, PyObject *args UNUSED)
{
    char const *filepath;
    char *content;
    ssize_t filesize;

    if (!PyArg_ParseTuple(args, "s", &filepath))
        return NULL;
    filesize = file_read(filepath, &content);
    if (filesize < 0)
        return NULL;
    if (memchr(content, '\0', filesize) != &content[filesize])
        Py_RETURN_TRUE;
    Py_RETURN_FALSE;
}

static
PyObject *py_report(PyObject *self UNUSED, PyObject *args)
{
    int line;
    char *filename;
    char *violation;

    if (!PyArg_ParseTuple(args, "sis", &filename, &line, &violation))
        return NULL;
    fprintf(stderr, "%s:%d: %s\n", filename, line, violation);
    Py_RETURN_NONE;
}

static
PyObject *py_get_line_count(PyObject *self UNUSED, PyObject *args)
{
    char const *filepath;
    char *content;
    ssize_t filesize;
    size_t linecount = 0;

    if (!PyArg_ParseTuple(args, "s", &filepath))
        return NULL;
    filesize = file_read(filepath, &content);
    for (ssize_t i = 0; i < filesize; i++)
        if (content[i] == '\n')
            linecount++;
    return Py_BuildValue("i", linecount + 1);
}

static
PyMethodDef VERA_METHODS[] = {
    {"report", py_report, METH_VARARGS, "."},
    {"isBinary", py_is_binary, METH_VARARGS, "."},
    {"getTokens", py_get_tokens, METH_VARARGS, "."},
    {"getLineCount", py_get_line_count, METH_VARARGS, "."},
    {"_register_sources", py_register_sources, METH_VARARGS, "."},
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
