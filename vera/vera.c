#include <fcntl.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/stat.h>

#include <Python.h>
#include <structmember.h>

#include "internal.h"
#include "vera_tokens.h"

static PyObject *FILE_TABLE = NULL;
static Py_ssize_t FILE_TABLE_SIZE = 0;
static bool NO_DUPLICATION = 0;

#define FILE_TABLE_MIN_SIZE 128
#define UNUSED __attribute__((unused))

static PyMemberDef PY_TOKEN_MEMBERS[] = {
    {"file", T_STRING, offsetof(PyTokenObject, file), 0, "file"},
    {"name", T_STRING, offsetof(PyTokenObject, name), 0, "name"},
    {"raw", T_STRING, offsetof(PyTokenObject, raw), 0, "raw"},
    {"type", T_STRING, offsetof(PyTokenObject, name), 0, "type"},
    {"value", T_STRING, offsetof(PyTokenObject, value), 0, "value"},
    {"column", T_INT, offsetof(PyTokenObject, column), 0, "column"},
    {"line", T_INT, offsetof(PyTokenObject, line), 0, "line"},
    {NULL}
};

static PyTypeObject PY_TOKEN_TYPE = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "vera.Token",
    .tp_basicsize = sizeof(PyTokenObject),
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = "Token object",
    .tp_new = PyType_GenericNew,
    .tp_members = PY_TOKEN_MEMBERS
};

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

    if (result.count < 0)
        return NULL;

    free(c_token_filter);
    PyObject *py_result = PyList_New(result.count);

    for (int i = 0; i < result.count; i++) {
        PyTokenObject *token = PyObject_New(PyTokenObject, &PY_TOKEN_TYPE);
        Token *src = &result.tokens[i];

        token->column = src->column;
        token->value = src->value;
        token->file = src->file;
        token->line = src->line;
        token->name = src->name;
        token->raw = src->raw;
        token->type = src->type;
        PyList_SetItem(py_result, i, (PyObject *)token);
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
    free(content);
    Py_RETURN_FALSE;
}

static
PyObject *py_set_duplication_policy(PyObject *self UNUSED, PyObject *args)
{
    if (!PyArg_ParseTuple(args, "b", &NO_DUPLICATION))
        return NULL;
    Py_RETURN_NONE;
}

static
PyObject *py_report(PyObject *self UNUSED, PyObject *args)
{
    int line;
    char *filename;
    char *violation;
    static PyObject *reported_set = NULL;

    if (!PyArg_ParseTuple(args, "sis", &filename, &line, &violation))
        return NULL;

    if (!NO_DUPLICATION)
        goto print_warning;

    if (reported_set == NULL) {
        reported_set = PySet_New(NULL);
        if (reported_set == NULL)
            return NULL;
    }

    PyObject *key = Py_BuildValue("sis", filename, line, violation);
    if (key == NULL)
        return NULL;

    int contains = PySet_Contains(reported_set, key);
    if (contains < 0) {
        Py_DECREF(key);
        return NULL;
    }

    if (contains == 1) {
        Py_DECREF(key);
        Py_RETURN_NONE;
    }

    if (PySet_Add(reported_set, key) < 0) {
        Py_DECREF(key);
        return NULL;
    }

    Py_DECREF(key);
print_warning:
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
PyObject *py_get_parameter(PyObject *self UNUSED, PyObject *args UNUSED)
{
    PyErr_SetString(
        PyExc_NotImplementedError,
        "getParameter will not be implemented.");
    return NULL;
}

static
PyMethodDef VERA_METHODS[] = {
    {"report", py_report, METH_VARARGS, "."},
    {"isBinary", py_is_binary, METH_VARARGS, "."},
    {"getTokens", py_get_tokens, METH_VARARGS, "."},
    {"getLineCount", py_get_line_count, METH_VARARGS, "."},
    {"getParameter", py_get_parameter, METH_VARARGS, "."},
    {"_register_sources", py_register_sources, METH_VARARGS, "."},
    {"_set_duplication_policy", py_set_duplication_policy, METH_VARARGS, "."},
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
    PyObject *mod = PyModule_Create(&VERA_MODULE);

    if (PyType_Ready(&PY_TOKEN_TYPE) < 0)
        return NULL;

    Py_INCREF(&PY_TOKEN_TYPE);
    PyModule_AddObject(mod, "Token", (PyObject *) &PY_TOKEN_TYPE);
    return mod;
}
