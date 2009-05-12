/* -- THIS FILE IS GENERATED - DO NOT EDIT *//* -*- Mode: C; c-basic-offset: 4 -*- */

#include <Python.h>



#line 22 "gst\video.override"

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define NO_IMPORT_PYGOBJECT
#include "common.h"

#include <gst/gst.h>

#include <gst/video/video.h>
#include <gst/video/gstvideofilter.h>
#include <gst/video/gstvideosink.h>
#include "pygstminiobject.h"
GST_DEBUG_CATEGORY_EXTERN (pygst_debug);
#define GST_CAT_DEFAULT pygst_debug

/* Boonky define that allows for backwards compatibility with Python 2.4 */
#if PY_VERSION_HEX < 0x02050000
#define Py_ssize_t int
#endif

#line 31 "gst\video.c"


/* ---------- types from other modules ---------- */
static PyTypeObject *_PyGObject_Type;
#define PyGObject_Type (*_PyGObject_Type)
static PyTypeObject *_PyGstObject_Type;
#define PyGstObject_Type (*_PyGstObject_Type)
static PyTypeObject *_PyGstStructure_Type;
#define PyGstStructure_Type (*_PyGstStructure_Type)
static PyTypeObject *_PyGstElement_Type;
#define PyGstElement_Type (*_PyGstElement_Type)
static PyTypeObject *_PyGstPad_Type;
#define PyGstPad_Type (*_PyGstPad_Type)
static PyTypeObject *_PyGstBuffer_Type;
#define PyGstBuffer_Type (*_PyGstBuffer_Type)
static PyTypeObject *_PyGstMessage_Type;
#define PyGstMessage_Type (*_PyGstMessage_Type)
static PyTypeObject *_PyGstSystemClock_Type;
#define PyGstSystemClock_Type (*_PyGstSystemClock_Type)
static PyTypeObject *_PyGstBaseTransform_Type;
#define PyGstBaseTransform_Type (*_PyGstBaseTransform_Type)
static PyTypeObject *_PyGstBaseSink_Type;
#define PyGstBaseSink_Type (*_PyGstBaseSink_Type)


/* ---------- forward type declarations ---------- */
PyTypeObject PyGstVideoFilter_Type;
PyTypeObject PyGstVideoSink_Type;

#line 61 "gst\video.c"



/* ----------- GstVideoFilter ----------- */

PyTypeObject PyGstVideoFilter_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.video.VideoFilter",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)NULL, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)0,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GstVideoSink ----------- */

PyTypeObject PyGstVideoSink_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.video.VideoSink",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)NULL, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)0,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- functions ----------- */

static PyObject *
_wrap_gst_video_format_from_fourcc(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "fourcc", NULL };
    unsigned long fourcc;
    gint ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"k:format_from_fourcc", kwlist, &fourcc))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_video_format_from_fourcc(fourcc);
    pyg_end_allow_threads;
    return pyg_enum_from_gtype(GST_TYPE_VIDEO_FORMAT, ret);
}

const PyMethodDef pyvideo_functions[] = {
    { "format_from_fourcc", (PyCFunction)_wrap_gst_video_format_from_fourcc, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};


/* ----------- enums and flags ----------- */

void
pyvideo_add_constants(PyObject *module, const gchar *strip_prefix)
{
  pyg_enum_add(module, "VideoFormat", strip_prefix, GST_TYPE_VIDEO_FORMAT);

  if (PyErr_Occurred())
    PyErr_Print();
}

/* initialise stuff extension classes */
void
pyvideo_register_classes(PyObject *d)
{
    PyObject *module;

    if ((module = PyImport_ImportModule("gobject")) != NULL) {
        PyObject *moddict = PyModule_GetDict(module);

        _PyGObject_Type = (PyTypeObject *)PyDict_GetItemString(moddict, "GObject");
        if (_PyGObject_Type == NULL) {
            PyErr_SetString(PyExc_ImportError,
                "cannot import name GObject from gobject");
            return;
        }
    } else {
        PyErr_SetString(PyExc_ImportError,
            "could not import gobject");
        return;
    }
    if ((module = PyImport_ImportModule("gst")) != NULL) {
        PyObject *moddict = PyModule_GetDict(module);

        _PyGstObject_Type = (PyTypeObject *)PyDict_GetItemString(moddict, "Object");
        if (_PyGstObject_Type == NULL) {
            PyErr_SetString(PyExc_ImportError,
                "cannot import name Object from gst");
            return;
        }
        _PyGstStructure_Type = (PyTypeObject *)PyDict_GetItemString(moddict, "Structure");
        if (_PyGstStructure_Type == NULL) {
            PyErr_SetString(PyExc_ImportError,
                "cannot import name Structure from gst");
            return;
        }
        _PyGstElement_Type = (PyTypeObject *)PyDict_GetItemString(moddict, "Element");
        if (_PyGstElement_Type == NULL) {
            PyErr_SetString(PyExc_ImportError,
                "cannot import name Element from gst");
            return;
        }
        _PyGstPad_Type = (PyTypeObject *)PyDict_GetItemString(moddict, "Pad");
        if (_PyGstPad_Type == NULL) {
            PyErr_SetString(PyExc_ImportError,
                "cannot import name Pad from gst");
            return;
        }
        _PyGstBuffer_Type = (PyTypeObject *)PyDict_GetItemString(moddict, "Buffer");
        if (_PyGstBuffer_Type == NULL) {
            PyErr_SetString(PyExc_ImportError,
                "cannot import name Buffer from gst");
            return;
        }
        _PyGstMessage_Type = (PyTypeObject *)PyDict_GetItemString(moddict, "Message");
        if (_PyGstMessage_Type == NULL) {
            PyErr_SetString(PyExc_ImportError,
                "cannot import name Message from gst");
            return;
        }
        _PyGstSystemClock_Type = (PyTypeObject *)PyDict_GetItemString(moddict, "SystemClock");
        if (_PyGstSystemClock_Type == NULL) {
            PyErr_SetString(PyExc_ImportError,
                "cannot import name SystemClock from gst");
            return;
        }
        _PyGstBaseTransform_Type = (PyTypeObject *)PyDict_GetItemString(moddict, "BaseTransform");
        if (_PyGstBaseTransform_Type == NULL) {
            PyErr_SetString(PyExc_ImportError,
                "cannot import name BaseTransform from gst");
            return;
        }
        _PyGstBaseSink_Type = (PyTypeObject *)PyDict_GetItemString(moddict, "BaseSink");
        if (_PyGstBaseSink_Type == NULL) {
            PyErr_SetString(PyExc_ImportError,
                "cannot import name BaseSink from gst");
            return;
        }
    } else {
        PyErr_SetString(PyExc_ImportError,
            "could not import gst");
        return;
    }


#line 282 "gst\video.c"
    pygobject_register_class(d, "GstVideoFilter", GST_TYPE_VIDEO_FILTER, &PyGstVideoFilter_Type, Py_BuildValue("(O)", &PyGstBaseTransform_Type));
    pygobject_register_class(d, "GstVideoSink", GST_TYPE_VIDEO_SINK, &PyGstVideoSink_Type, Py_BuildValue("(O)", &PyGstBaseSink_Type));
}
