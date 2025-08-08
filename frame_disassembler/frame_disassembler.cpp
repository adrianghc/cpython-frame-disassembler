// SPDX-License-Identifier: LGPL-3.0-only

#include "Python.h"
#include <cstdio>

/* Minimal Shiboken compatibility layer */
namespace Shiboken {

    // Simple GIL State manager
    class GilState {
        PyGILState_STATE m_state;
        bool m_valid;
    public:
        GilState() : m_valid(true) {
#ifdef Py_GIL_DISABLED
            m_valid = false;
#else
            // Runtime check for free-threaded Python (3.13t)
            static bool isNoGil = false;
            static bool checked = false;
            if (!checked) {
                PyObject *flags = PySys_GetObject("flags");
                isNoGil = flags && PyObject_HasAttrString(flags, "nogil");
                checked = true;
            }
            if (isNoGil) {
                m_valid = false;
            } else {
                m_state = PyGILState_Ensure();
            }
#endif
        }

        ~GilState() { 
            if (m_valid) {
                PyGILState_Release(m_state);
            }
        }

        bool isValid() const { return m_valid; }

        // Disable copy
        GilState(const GilState&) = delete;
        GilState& operator=(const GilState&) = delete;
    };

    class AutoDecRef {
        PyObject* m_ptr;
    public:
        explicit AutoDecRef(PyObject* p = nullptr) : m_ptr(p) {}
        ~AutoDecRef() { Py_XDECREF(m_ptr); }

        void reset(PyObject* p = nullptr) {
            Py_XDECREF(m_ptr);
            m_ptr = p;
        }

        PyObject* object() const { return m_ptr; }
        operator PyObject*() const { return m_ptr; }

        // Disable copy
        AutoDecRef(const AutoDecRef&) = delete;
        AutoDecRef& operator=(const AutoDecRef&) = delete;
    };
}

static void disassembleFrame(const char *marker)
{
    using Shiboken::AutoDecRef;
    Shiboken::GilState gil;  // Ensure we have the GIL

    // Check if we're running on no-GIL Python
    if (!gil.isValid()) {
        fprintf(stderr, "\n%s ERROR: frame_disassembler does not support free-threaded Python (3.13t).\n", marker);
        fprintf(stderr, "%s       VSCode debugging is unreliable with --disable-gil.\n\n", marker);
        return;
    }

    PyObject *error_type, *error_value, *error_traceback;
    PyErr_Fetch(&error_type, &error_value, &error_traceback);

    static PyObject *dismodule = PyImport_ImportModule("dis");
    if (!dismodule) {
        PyErr_Restore(error_type, error_value, error_traceback);
        return;
    }

    static PyObject *disco = PyObject_GetAttrString(dismodule, "disco");
    if (!disco) {
        PyErr_Restore(error_type, error_value, error_traceback);
        return;
    }

    static PyObject *const _f_lasti = PyUnicode_FromString("f_lasti");
    static PyObject *const _f_lineno = PyUnicode_FromString("f_lineno");
    static PyObject *const _f_code = PyUnicode_FromString("f_code");
    static PyObject *const _co_filename = PyUnicode_FromString("co_filename");

    AutoDecRef ignore{};
    auto *frame = reinterpret_cast<PyObject *>(PyEval_GetFrame());

    if (frame == nullptr) {
        fprintf(stdout, "\n%s BEGIN no frame END\n\n", marker);
    } else {
        AutoDecRef f_lasti(PyObject_GetAttr(frame, _f_lasti));
        AutoDecRef f_lineno(PyObject_GetAttr(frame, _f_lineno));
        AutoDecRef f_code(PyObject_GetAttr(frame, _f_code));
        AutoDecRef co_filename(PyObject_GetAttr(f_code, _co_filename));

        long line = PyLong_AsLong(f_lineno);
        const char *fname = PyUnicode_AsUTF8(co_filename);

        fprintf(stdout, "\n%s BEGIN line=%ld %s\n", marker, line, fname);
        ignore.reset(PyObject_CallFunctionObjArgs(disco, f_code.object(), f_lasti.object(), nullptr));
        fprintf(stdout, "%s END line=%ld %s\n\n", marker, line, fname);
    }

    static PyObject *stdout_file = PySys_GetObject("stdout");
    ignore.reset(PyObject_CallMethod(stdout_file, "flush", nullptr));

    PyErr_Restore(error_type, error_value, error_traceback);
}

extern "C" {

static PyObject *py_disassemble_frame(PyObject *self, PyObject *args, PyObject *kwargs)
{
    const char *marker = "MARKER";
    static const char *kwlist[] = {"marker", nullptr};
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|s", const_cast<char**>(kwlist), &marker))
    {
        return nullptr;
    }
    disassembleFrame(marker);
    Py_RETURN_NONE;
}

static PyMethodDef frame_disassembler_methods[] = {
    {"disassemble_frame", reinterpret_cast<PyCFunction>(py_disassemble_frame),
     METH_VARARGS | METH_KEYWORDS, "Disassemble the current Python frame"},
    {nullptr, nullptr, 0, nullptr}
};

static struct PyModuleDef frame_disassembler_module = {
    PyModuleDef_HEAD_INIT,
    "frame_disassembler",
    "Disassemble the current Python frame using C++",
    -1,
    frame_disassembler_methods
};

PyMODINIT_FUNC PyInit_frame_disassembler(void)
{
    return PyModule_Create(&frame_disassembler_module);
}

} // extern "C"
