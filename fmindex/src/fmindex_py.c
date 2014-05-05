#include    <Python.h>
#include "structmember.h"

#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include    <numpy/arrayobject.h>

#include    <stdio.h>
#include    <string.h> /* for NULL pointers */
#include    <inttypes.h>

#include "fm_index.h"
#include "interface.h"
#include "fm_build.h"
#include "ds_ssort.h"


#if PY_MAJOR_VERSION >= 3
/* see http://python3porting.com/cextensions.html */
    #define MOD_INIT(name) PyMODINIT_FUNC PyInit_##name(void)
    #define PyInt_AsLong PyLong_AsLong
#else
    #define MOD_INIT(name) PyMODINIT_FUNC init##name(void)
#endif

/* module doc string */
PyDoc_STRVAR(fmindex__doc__, "python bindings for fm-index\n");

/* see http://docs.python.org/2/extending/newtypes.html 
and http://docs.python.org/3/extending/newtypes.html */

char err_buffer[512];

/*--------------------------------------------------------------------------*/
typedef struct {
    PyObject_HEAD
    fm_index* idx;
    int fm_flag;    // does nothing could remove this
} FMIndex;

static void
FMIndex_dealloc(FMIndex* self) {
    free_index(self->idx);
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject *
FMIndex_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    FMIndex *self;

    self = (FMIndex *)type->tp_alloc(type, 0);
    if (self != NULL) {
        self->fm_flag = 7;
    }
    return (PyObject *)self;
}

fm_index * FMIndex_data(FMIndex* self) {
    return self->idx;
}

static int
FMIndex_init(FMIndex *self, PyObject *args, PyObject *kwds) {

    char *from_fmi_file=NULL;
    int from_fmi_file_length;

    char *from_text_file=NULL;
    int from_text_file_length;

    int error;
    fm_index * self_idx;

    int strip_newline = 1; // strip by default

    char * in_text=NULL;
    int in_text_length;
    unsigned long overshoot;

    uchar *text=NULL;
    ulong text_len;
    ulong bsl1 = 16;
    ulong bsl2 = 512;
    ulong freq = 64;
    suint tc = MULTIH;


    static char *kwlist[] = {"from_fmi_file", 
                            "from_text_file",
                            "text",
                            "superbucket_size",
                            "bucket_size",
                            "frequency",
                            "strip_newline",
                            "fm_flag",
                            NULL};

    if (self == NULL) {
        return -1;
    }

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|s#s#s#iiii", kwlist, 
                                      &from_fmi_file, &from_fmi_file_length,
                                      &from_text_file, &from_text_file_length,
                                      &in_text, &in_text_length,
                                      &bsl1, &bsl2, &freq,
                                      &strip_newline,
                                      &self->fm_flag))  {
        PyErr_SetString(PyExc_TypeError, "FMIndex_init: arg problem\n");
        return -1; 
    }

    self_idx = self->idx;
    if (from_fmi_file != NULL) {
        error = load_index(from_fmi_file, (void*) &self_idx);
        if (error) {
            sprintf(err_buffer, "FMIndex_init: could not load fmi file: %s\n", from_fmi_file); 
            PyErr_SetString(PyExc_IOError, err_buffer);
            return -1;
        }
    } else { // must create from scratch
        if (from_text_file != NULL) {
            error = fm_read_file2(from_text_file, &text, &text_len, strip_newline);
            if (error) {
                sprintf(err_buffer, "FMIndex_init: could not load text file: %s\n", from_text_file);
                PyErr_SetString(PyExc_IOError, err_buffer);
                return -1;
            }
        } else {
            /* assumes a copy to a C-string is cool */
            if (in_text == NULL) {
                sprintf(err_buffer, "FMIndex_init: input text string missing\n");
                PyErr_SetString(PyExc_TypeError, err_buffer);
                return -1;
            }
            overshoot = (ulong) init_ds_ssort(500, 2000); 
            text = malloc((in_text_length + overshoot)*sizeof(*text));
            strncpy((char *) text, in_text, in_text_length);
            text[in_text_length]='\0';
            // printf((char*) text);
            text_len = in_text_length;
        }

        /* allocate memory */
        self_idx = malloc(sizeof(fm_index));

        if (self_idx == NULL) {
            PyErr_SetString(PyExc_MemoryError, "FMIndex_init: no memory for fm_index\n");
            return -1;
        }
        self->idx = self_idx;
        /* configure */
        error = fm_build_config(self_idx, tc, freq, bsl1, bsl2, 1);
        if (error) {
                sprintf(err_buffer, "FMIndex_init: bad configuration args %d, %d, %d\n", 
                                            (int) freq, (int) bsl1, (int) bsl2);
                PyErr_SetString(PyExc_IOError, err_buffer);
                return -1;
        }
        // printf("building: %d, %s\n", text_len, text);
        error =  fm_build(self_idx, text, text_len);
        if (error) {
            sprintf(err_buffer, "FMIndex_init: build error %d\n", error);
            PyErr_SetString(PyExc_IOError, err_buffer);
            return -1;
        }

    } /* end else */


    return 0;
}

static PyObject *
FMIndex_save(FMIndex* self, PyObject *args) {
    char * outfile;
    int error; 

    if (self->idx == NULL) {
        PyErr_SetString(PyExc_AttributeError, "FMIndex.save: missing fm index\n");
        return NULL;
    }

    if (!PyArg_ParseTuple(args, "s", &outfile)) {
        PyErr_SetString(PyExc_TypeError, "FMIndex.save: missing output file\n");
        return NULL;
    }

    error = save_index(self->idx, outfile);
    if (error) {
        sprintf(err_buffer, "FMIndex.save: save error %d\n", error);
        PyErr_SetString(PyExc_IOError, err_buffer);
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyObject * 
FMIndex_extract(FMIndex* self, PyObject *args, PyObject *kwds) {
    /*
    pos_substring       extract a text substring from pos to pos+numchars-1
    length_substring    length of the extracted substring (default 10) (use with -e)
    */

    uchar * text=NULL;
    int eposition=0;
    int elength=10;
    PyObject * out_ptr;
    int error;
    ulong read;

    static char *kwlist[] = {   "position",
                                "length",
                                NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "i|i", kwlist, 
                                      &eposition,
                                      &elength))  {
        PyErr_SetString(PyExc_TypeError, "FMIndex_extract: arg problem\n");
        return NULL; 
    }

    error = extract(self->idx, eposition, eposition+elength-1, &text, &read);
    if (error) {
        sprintf(err_buffer, "FMIndex_extract: extract problem: %d\n", error);
        PyErr_SetString(PyExc_IOError, err_buffer);
        free(text);
        return NULL; 
    }
    if (read > 0) {
        out_ptr = Py_BuildValue("s#", (char *) text, (int) read);
        free(text);
        return out_ptr;
    } else {
        Py_RETURN_NONE;
    }
}

static PyObject * 
FMIndex_count(FMIndex* self, PyObject *args) {
    /*
    count   count number of occurrences of 'pattern' in the text
    */

    ulong numocc = 0;
    char * pattern=NULL;
    int pattern_length=0;
    int error;

    if (!PyArg_ParseTuple(args, "s#", &pattern, &pattern_length))  {
        PyErr_SetString(PyExc_TypeError, "FMIndex_count: arg problem\n");
        return NULL; 
    }
    if (pattern_length == 0) {
        return NULL; 
    }
    error = count(self->idx, (uchar *) pattern, pattern_length, &numocc);
    if (error) {
        sprintf(err_buffer, "FMIndex_count: extract problem: %d\n", error);
        PyErr_SetString(PyExc_IOError, err_buffer);
        return NULL; 
    }
    return Py_BuildValue("l", (long) numocc);
}

static PyObject * 
FMIndex_locate(FMIndex* self, PyObject *args) {
    /*
    locate  locate occurrences of 'pattern' and return a tuple
    of indices
    */

    ulong *occs;
    ulong numocc = 0;
    char * pattern=NULL;
    int pattern_length=0;
    int error, i;
    PyObject* outtuple = NULL;

    Py

    if (!PyArg_ParseTuple(args, "s#", &pattern, &pattern_length))  {
        PyErr_SetString(PyExc_TypeError, "FMIndex_locate: arg problem\n");
        return NULL; 
    }
    if (pattern_length == 0) {
        return NULL; 
    }
    error = locate(self->idx, (uchar *) pattern, pattern_length, &occs, &numocc);
    if (error) {
        sprintf(err_buffer, "FMIndex_locate: extract problem: %d\n", error);
        PyErr_SetString(PyExc_IOError, err_buffer);
        if (numocc) {
            free(occs);
        }
        return NULL; 
    }
    if (numocc) {
        outtuple = PyTuple_New(numocc);
        for (i=0; i < numocc; i++) {
            PyTuple_SET_ITEM(outtuple, i, PyLong_FromLong(occs[i]));
        }
        free(occs);
        return outtuple;
    } else {
        Py_RETURN_NONE;
    }
}

static PyObject * 
FMIndex_display(FMIndex* self, PyObject *args) {
    /*
    Returns a tuple of all strings snippets surrounding any occurrence of the 
    substring pattern[0..length-1] within the text indexed by index. The 
    snippet must include nchars before and after the pattern occurrence, 
    totalizing length+2*nchars characters, or less if the text boundaries are 
    reached.  
    */

    ulong numocc, i, *length_snippet, p, len;
    int nchars;
    uchar *snippet_text;
    char * pattern=NULL;
    int pattern_length=0;
    int error;

    PyObject* outtuple = NULL;

    if (!PyArg_ParseTuple(args, "s#i", &pattern, &pattern_length, &nchars))  {
        PyErr_SetString(PyExc_TypeError, "FMIndex_display: arg problem\n");
        return NULL; 
    }
    if (pattern_length == 0) {
        return NULL; 
    }

    len = pattern_length + 2*nchars;
    numocc = 0;

    error = display(self->idx, (uchar *) pattern, pattern_length, 
                        (ulong) nchars, &numocc,
                        &snippet_text, &length_snippet);

    if (error) {
        sprintf(err_buffer, "FMIndex_display: display problem: %d\n", error);
        PyErr_SetString(PyExc_IOError, err_buffer);
        if (numocc) {
            free (length_snippet);
            free (snippet_text);
        }
        return NULL; 
    }
    if (numocc) {
        outtuple = PyTuple_New(numocc);
        for (i = 0, p = 0; i < numocc; i++, p+=len) {
            PyTuple_SET_ITEM(outtuple, i, Py_BuildValue("s#", &snippet_text[p], length_snippet[i]));
        }
        free (length_snippet);
        free (snippet_text);
        return outtuple;
    } else {
        Py_RETURN_NONE;
    }
}

static PyMemberDef FMIndex_members[] = {
    {"fm_flag", T_INT, offsetof(FMIndex, fm_flag), 0,
     "fm_index fm_flag"},
    {NULL}  /* Sentinel */
};

static PyGetSetDef FMIndex_getseters[] = {
    {NULL}  /* Sentinel */
};

static PyMethodDef FMIndex_methods[] = {
    {"save", (PyCFunction)FMIndex_save, METH_VARARGS,
     "save an FMIndex to a file"
    },
    {"extract", (PyCFunction)FMIndex_extract, METH_VARARGS | METH_KEYWORDS,
     "extract a text substring from pos to pos+numchars-1"
    },
    {"count", (PyCFunction)FMIndex_count, METH_VARARGS,
     "count number of occurrences of 'pattern'"
    },
    {"locate", (PyCFunction)FMIndex_locate, METH_VARARGS,
     "locate occurrences of 'pattern'"
    },
    {"display", (PyCFunction)FMIndex_display, METH_VARARGS,
     "display 'numchars' chars sourrounding each occ of 'pattern'"
    },
    {NULL}  /* Sentinel */
};

static PyTypeObject FMIndexType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "_fmindex.FMIndex",              /*tp_name*/
    sizeof(FMIndex),                /*tp_basicsize*/
    0,                              /*tp_itemsize*/
    (destructor)FMIndex_dealloc,    /*tp_dealloc*/
    0,                              /*tp_print*/
    0,                              /*tp_getattr*/
    0,                              /*tp_setattr*/
    0,                              /*tp_compare*/
    0,                              /*tp_repr*/
    0,                              /*tp_as_number*/
    0,                              /*tp_as_sequence*/
    0,                              /*tp_as_mapping*/
    0,                              /*tp_hash */
    0,                              /*tp_call*/
    0,                              /*tp_str*/
    0,                              /*tp_getattro*/
    0,                              /*tp_setattro*/
    0,                              /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "FMIndex objects",              /* tp_doc */
    0,                              /* tp_traverse */
    0,                              /* tp_clear */
    0,                              /* tp_richcompare */
    0,                              /* tp_weaklistoffset */
    0,                              /* tp_iter */
    0,                              /* tp_iternext */
    FMIndex_methods,                /* tp_methods */
    FMIndex_members,                /* tp_members */
    FMIndex_getseters,              /* tp_getset */
    0,                              /* tp_base */
    0,                              /* tp_dict */
    0,                              /* tp_descr_get */
    0,                              /* tp_descr_set */
    0,                              /* tp_dictoffset */
    (initproc)FMIndex_init,         /* tp_init */
    0,                              /* tp_alloc */
    FMIndex_new,                    /* tp_new */
};


static PyMethodDef fmindex_mod_methods[] = {
    { NULL }
};

MOD_INIT(_fmindex){
    if (PyType_Ready(&FMIndexType) < 0) {
        return NULL;
    }
    /* standard numpy compatible initiation */
#if PY_MAJOR_VERSION >= 3
    static struct PyModuleDef moduledef = {
        PyModuleDef_HEAD_INIT,
        "_fmindex",              /* m_name */
        fmindex__doc__,         /* m_doc */
        -1,                     /* m_size */
        fmindex_mod_methods,    /* m_methods */
        NULL,                   /* m_reload */
        NULL,                   /* m_traverse */
        NULL,                   /* m_clear */
        NULL,                   /* m_free */
    };
    PyObject* m = PyModule_Create(&moduledef);
    if (m == NULL) { return NULL; }
      
    import_array();
    Py_INCREF(&FMIndexType);
    PyModule_AddObject(m, "FMIndex", (PyObject *)&FMIndexType);
    return m;
#else
    PyObject* m = Py_InitModule3("_fmindex", fmindex_mod_methods, fmindex__doc__);
    if (m == NULL) { return; }

    import_array();
    Py_INCREF(&FMIndexType);
    PyModule_AddObject(m, "FMIndex", (PyObject *)&FMIndexType);

#endif
}
