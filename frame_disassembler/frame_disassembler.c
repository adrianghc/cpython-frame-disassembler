#define PY_SSIZE_T_CLEAN
#include "Python.h"

static void disassemble_frame_impl(const char *marker)
{
    PyObject *dismodule = PyImport_ImportModule("dis");
    PyObject *disco = PyObject_GetAttrString(dismodule, "disco");
    PyObject *const _f_lasti = PyUnicode_FromString("f_lasti");
    PyObject *const _f_lineno = PyUnicode_FromString("f_lineno");
    PyObject *const _f_code = PyUnicode_FromString("f_code");
    PyObject *const _co_filename = PyUnicode_FromString("co_filename");
    PyObject *frame = (PyObject *)PyEval_GetFrame();
    if (frame == NULL)
    {
        fprintf(stdout, "\n%s BEGIN no frame END\n\n", marker);
    }
    else
    {
        PyObject *f_lasti = PyObject_GetAttr(frame, _f_lasti);
        PyObject *f_lineno = PyObject_GetAttr(frame, _f_lineno);
        PyObject *f_code = PyObject_GetAttr(frame, _f_code);
        PyObject *co_filename = PyObject_GetAttr(f_code, _co_filename);
        long line = PyLong_AsLong(f_lineno);
        const char *fname = PyUnicode_AsUTF8(co_filename);
        fprintf(stdout, "\n%s BEGIN line=%ld %s\n", marker, line, fname);
        PyObject_CallFunctionObjArgs(disco, f_code, f_lasti, NULL);
        fprintf(stdout, "%s END line=%ld %s\n\n", marker, line, fname);
        Py_XDECREF(f_lasti);
        Py_XDECREF(f_lineno);
        Py_XDECREF(f_code);
        Py_XDECREF(co_filename);
    }
    if (PyErr_Occurred())
    {
        PyErr_Print();
    }
    PyObject *stdout_file = PySys_GetObject("stdout");
    PyObject_CallMethod(stdout_file, "flush", NULL);
}

static PyObject *py_disassemble_frame(PyObject *self, PyObject *args, PyObject *kwargs)
{
    const char *marker = "MARKER";
    static const char *kwlist[] = {"marker", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|s", (char **)kwlist, &marker))
    {
        return NULL;
    }
    disassemble_frame_impl(marker);
    Py_RETURN_NONE;
}

static PyMethodDef frame_disassembler_methods[] = {
    {"disassemble_frame", (PyCFunction) py_disassemble_frame,
     METH_VARARGS | METH_KEYWORDS, "Disassemble the current Python frame"},
    {NULL, NULL, 0, NULL}};

static struct PyModuleDef frame_disassembler_module = {
    PyModuleDef_HEAD_INIT,
    "cpython_frame_disassembler",
    "Disassemble the current Python frame using C",
    -1,
    frame_disassembler_methods};

PyMODINIT_FUNC PyInit_frame_disassembler(void)
{
    return PyModule_Create(&frame_disassembler_module);
}
