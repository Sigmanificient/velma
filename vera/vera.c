#include <Python.h>

#include "vera_tokens.h"

static
PyObject* py_gettokens(PyObject* self, PyObject* args)
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
PyObject* py_getsourcefilenames(PyObject* self, PyObject* args)
{
    PyObject *py_result = PyList_New(0);
    char *buff;
    size_t buffsize;

    while (getline(&buff, &buffsize, stdin) > 0) {
        buff[strcspn(buff, "\n")] = '\0';
        PyList_Append(py_result, PyUnicode_FromString(buff));
    }
    return py_result;
}

static
PyMethodDef VERA_METHODS[] = {
    {"getTokens", py_gettokens, METH_VARARGS, "Get tokens from a file."},
    {"getSourceFileNames", py_getsourcefilenames, METH_VARARGS, "."},
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
