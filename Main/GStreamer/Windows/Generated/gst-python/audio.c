/* -- THIS FILE IS GENERATED - DO NOT EDIT *//* -*- Mode: C; c-basic-offset: 4 -*- */

#include <Python.h>



#line 22 "..\\..\\..\\Source\\gst-python\\gst\\audio.override"

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define NO_IMPORT_PYGOBJECT
#include "common.h"

#include <gst/gst.h>

#include <gst/audio/audio.h>
#include <gst/audio/gstaudioclock.h>
#include <gst/audio/gstaudiofilter.h>
#include <gst/audio/gstaudiosink.h>
#include <gst/audio/gstaudiosrc.h>
#include <gst/audio/gstbaseaudiosink.h>
#include <gst/audio/gstbaseaudiosrc.h>
#include <gst/audio/multichannel.h>
#include "pygstminiobject.h"
GST_DEBUG_CATEGORY_EXTERN (pygst_debug);
#define GST_CAT_DEFAULT pygst_debug

/* Boonky define that allows for backwards compatibility with Python 2.4 */
#if PY_VERSION_HEX < 0x02050000
#define Py_ssize_t int
#endif

#line 36 "..\\..\\..\\Source\\gst-python\\gst\\audio.c"


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
static PyTypeObject *_PyGstClock_Type;
#define PyGstClock_Type (*_PyGstClock_Type)


/* ---------- forward type declarations ---------- */
PyTypeObject PyGstAudioClock_Type;
PyTypeObject PyGstAudioFilter_Type;
PyTypeObject PyGstBaseAudioSink_Type;
PyTypeObject PyGstAudioSink_Type;
PyTypeObject PyGstRingBuffer_Type;

#line 71 "..\\..\\..\\Source\\gst-python\\gst\\audio.c"



/* ----------- GstAudioClock ----------- */

static int
pygobject_no_constructor(PyObject *self, PyObject *args, PyObject *kwargs)
{
    gchar buf[512];

    g_snprintf(buf, sizeof(buf), "%s is an abstract widget", self->ob_type->tp_name);
    PyErr_SetString(PyExc_NotImplementedError, buf);
    return -1;
}

static PyObject *
_wrap_gst_audio_clock_reset(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "time", NULL };
    guint64 time;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"K:GstAudioClock.reset", kwlist, &time))
        return NULL;
    pyg_begin_allow_threads;
    gst_audio_clock_reset(GST_AUDIO_CLOCK(self->obj), time);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGstAudioClock_methods[] = {
    { "reset", (PyCFunction)_wrap_gst_audio_clock_reset, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject PyGstAudioClock_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.audio.AudioClock",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGstAudioClock_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)pygobject_no_constructor,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GstAudioFilter ----------- */

PyTypeObject PyGstAudioFilter_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.audio.AudioFilter",                   /* tp_name */
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


static int
__GstAudioFilter_class_init(gpointer gclass, PyTypeObject *pyclass)
{

    /* overriding do_setup is currently not supported */
    return 0;
}


/* ----------- GstBaseAudioSink ----------- */

static PyObject *
_wrap_gst_base_audio_sink_create_ringbuffer(PyGObject *self)
{
    GstRingBuffer *ret;

    pyg_begin_allow_threads;
    ret = gst_base_audio_sink_create_ringbuffer(GST_BASE_AUDIO_SINK(self->obj));
    pyg_end_allow_threads;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gst_base_audio_sink_set_provide_clock(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "provide", NULL };
    int provide;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GstBaseAudioSink.set_provide_clock", kwlist, &provide))
        return NULL;
    pyg_begin_allow_threads;
    gst_base_audio_sink_set_provide_clock(GST_BASE_AUDIO_SINK(self->obj), provide);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_base_audio_sink_get_provide_clock(PyGObject *self)
{
    int ret;

    pyg_begin_allow_threads;
    ret = gst_base_audio_sink_get_provide_clock(GST_BASE_AUDIO_SINK(self->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_base_audio_sink_set_slave_method(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "method", NULL };
    PyObject *py_method = NULL;
    GstBaseAudioSinkSlaveMethod method;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:GstBaseAudioSink.set_slave_method", kwlist, &py_method))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_BASE_AUDIO_SINK_SLAVE_METHOD, py_method, (gint *)&method))
        return NULL;
    pyg_begin_allow_threads;
    gst_base_audio_sink_set_slave_method(GST_BASE_AUDIO_SINK(self->obj), method);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_base_audio_sink_get_slave_method(PyGObject *self)
{
    gint ret;

    pyg_begin_allow_threads;
    ret = gst_base_audio_sink_get_slave_method(GST_BASE_AUDIO_SINK(self->obj));
    pyg_end_allow_threads;
    return pyg_enum_from_gtype(GST_TYPE_BASE_AUDIO_SINK_SLAVE_METHOD, ret);
}

static PyObject *
_wrap_GstBaseAudioSink__do_create_ringbuffer(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    GstRingBuffer *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstBaseAudioSink.create_ringbuffer", kwlist, &PyGstBaseAudioSink_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_BASE_AUDIO_SINK_CLASS(klass)->create_ringbuffer) {
        pyg_begin_allow_threads;
        ret = GST_BASE_AUDIO_SINK_CLASS(klass)->create_ringbuffer(GST_BASE_AUDIO_SINK(self->obj));
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstBaseAudioSink.create_ringbuffer not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static const PyMethodDef _PyGstBaseAudioSink_methods[] = {
    { "create_ringbuffer", (PyCFunction)_wrap_gst_base_audio_sink_create_ringbuffer, METH_NOARGS,
      NULL },
    { "set_provide_clock", (PyCFunction)_wrap_gst_base_audio_sink_set_provide_clock, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_provide_clock", (PyCFunction)_wrap_gst_base_audio_sink_get_provide_clock, METH_NOARGS,
      NULL },
    { "set_slave_method", (PyCFunction)_wrap_gst_base_audio_sink_set_slave_method, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_slave_method", (PyCFunction)_wrap_gst_base_audio_sink_get_slave_method, METH_NOARGS,
      NULL },
    { "do_create_ringbuffer", (PyCFunction)_wrap_GstBaseAudioSink__do_create_ringbuffer, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject PyGstBaseAudioSink_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.audio.BaseAudioSink",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGstBaseAudioSink_methods, /* tp_methods */
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

static GstRingBuffer*
_wrap_GstBaseAudioSink__proxy_do_create_ringbuffer(GstBaseAudioSink *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    GstRingBuffer* retval;
    PyObject *py_retval;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    
    
    py_method = PyObject_GetAttrString(py_self, "do_create_ringbuffer");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    py_retval = PyObject_CallObject(py_method, NULL);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    if (!PyObject_TypeCheck(py_retval, &PyGObject_Type)) {
        PyErr_SetString(PyExc_TypeError, "retval should be a GObject");
        PyErr_Print();
        Py_DECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    retval = (GstRingBuffer*) pygobject_get(py_retval);
    g_object_ref((GObject *) retval);
    
    
    Py_DECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}

static int
__GstBaseAudioSink_class_init(gpointer gclass, PyTypeObject *pyclass)
{
    PyObject *o;
    GstBaseAudioSinkClass *klass = GST_BASE_AUDIO_SINK_CLASS(gclass);
    PyObject *gsignals = PyDict_GetItemString(pyclass->tp_dict, "__gsignals__");

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_create_ringbuffer");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "create_ringbuffer")))
            klass->create_ringbuffer = _wrap_GstBaseAudioSink__proxy_do_create_ringbuffer;
        Py_DECREF(o);
    }
    return 0;
}


/* ----------- GstAudioSink ----------- */

static PyObject *
_wrap_GstAudioSink__do_open(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstAudioSink.open", kwlist, &PyGstAudioSink_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_AUDIO_SINK_CLASS(klass)->open) {
        pyg_begin_allow_threads;
        ret = GST_AUDIO_SINK_CLASS(klass)->open(GST_AUDIO_SINK(self->obj));
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstAudioSink.open not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_GstAudioSink__do_unprepare(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstAudioSink.unprepare", kwlist, &PyGstAudioSink_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_AUDIO_SINK_CLASS(klass)->unprepare) {
        pyg_begin_allow_threads;
        ret = GST_AUDIO_SINK_CLASS(klass)->unprepare(GST_AUDIO_SINK(self->obj));
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstAudioSink.unprepare not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_GstAudioSink__do_close(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstAudioSink.close", kwlist, &PyGstAudioSink_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_AUDIO_SINK_CLASS(klass)->close) {
        pyg_begin_allow_threads;
        ret = GST_AUDIO_SINK_CLASS(klass)->close(GST_AUDIO_SINK(self->obj));
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstAudioSink.close not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_GstAudioSink__do_delay(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    guint ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstAudioSink.delay", kwlist, &PyGstAudioSink_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_AUDIO_SINK_CLASS(klass)->delay) {
        pyg_begin_allow_threads;
        ret = GST_AUDIO_SINK_CLASS(klass)->delay(GST_AUDIO_SINK(self->obj));
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstAudioSink.delay not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyLong_FromUnsignedLong(ret);
}

static PyObject *
_wrap_GstAudioSink__do_reset(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstAudioSink.reset", kwlist, &PyGstAudioSink_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_AUDIO_SINK_CLASS(klass)->reset) {
        pyg_begin_allow_threads;
        GST_AUDIO_SINK_CLASS(klass)->reset(GST_AUDIO_SINK(self->obj));
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstAudioSink.reset not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGstAudioSink_methods[] = {
    { "do_open", (PyCFunction)_wrap_GstAudioSink__do_open, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_unprepare", (PyCFunction)_wrap_GstAudioSink__do_unprepare, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_close", (PyCFunction)_wrap_GstAudioSink__do_close, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_delay", (PyCFunction)_wrap_GstAudioSink__do_delay, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_reset", (PyCFunction)_wrap_GstAudioSink__do_reset, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject PyGstAudioSink_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.audio.AudioSink",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGstAudioSink_methods, /* tp_methods */
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

static gboolean
_wrap_GstAudioSink__proxy_do_open(GstAudioSink *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    gboolean retval;
    PyObject *py_main_retval;
    PyObject *py_retval;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    
    py_method = PyObject_GetAttrString(py_self, "do_open");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_retval = PyObject_CallObject(py_method, NULL);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "O", &py_main_retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_DECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gboolean
_wrap_GstAudioSink__proxy_do_unprepare(GstAudioSink *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    gboolean retval;
    PyObject *py_main_retval;
    PyObject *py_retval;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    
    py_method = PyObject_GetAttrString(py_self, "do_unprepare");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_retval = PyObject_CallObject(py_method, NULL);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "O", &py_main_retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_DECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gboolean
_wrap_GstAudioSink__proxy_do_close(GstAudioSink *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    gboolean retval;
    PyObject *py_main_retval;
    PyObject *py_retval;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    
    py_method = PyObject_GetAttrString(py_self, "do_close");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_retval = PyObject_CallObject(py_method, NULL);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "O", &py_main_retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_DECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static guint
_wrap_GstAudioSink__proxy_do_delay(GstAudioSink *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    guint retval;
    PyObject *py_retval;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    
    
    py_method = PyObject_GetAttrString(py_self, "do_delay");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    py_retval = PyObject_CallObject(py_method, NULL);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "i", &retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    
    
    Py_DECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static void
_wrap_GstAudioSink__proxy_do_reset(GstAudioSink *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_retval;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    py_method = PyObject_GetAttrString(py_self, "do_reset");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    py_retval = PyObject_CallObject(py_method, NULL);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    if (py_retval != Py_None) {
        if (PyErr_Occurred())
            PyErr_Print();
        PyErr_SetString(PyExc_TypeError, "retval should be None");
        Py_DECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    Py_DECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
}

static int
__GstAudioSink_class_init(gpointer gclass, PyTypeObject *pyclass)
{
    PyObject *o;
    GstAudioSinkClass *klass = GST_AUDIO_SINK_CLASS(gclass);
    PyObject *gsignals = PyDict_GetItemString(pyclass->tp_dict, "__gsignals__");

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_open");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "open")))
            klass->open = _wrap_GstAudioSink__proxy_do_open;
        Py_DECREF(o);
    }

    /* overriding do_prepare is currently not supported */

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_unprepare");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "unprepare")))
            klass->unprepare = _wrap_GstAudioSink__proxy_do_unprepare;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_close");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "close")))
            klass->close = _wrap_GstAudioSink__proxy_do_close;
        Py_DECREF(o);
    }

    /* overriding do_write is currently not supported */

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_delay");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "delay")))
            klass->delay = _wrap_GstAudioSink__proxy_do_delay;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_reset");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "reset")))
            klass->reset = _wrap_GstAudioSink__proxy_do_reset;
        Py_DECREF(o);
    }
    return 0;
}


/* ----------- GstRingBuffer ----------- */

static PyObject *
_wrap_gst_ring_buffer_open_device(PyGObject *self)
{
    int ret;

    pyg_begin_allow_threads;
    ret = gst_ring_buffer_open_device(GST_RING_BUFFER(self->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_ring_buffer_close_device(PyGObject *self)
{
    int ret;

    pyg_begin_allow_threads;
    ret = gst_ring_buffer_close_device(GST_RING_BUFFER(self->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_ring_buffer_device_is_open(PyGObject *self)
{
    int ret;

    pyg_begin_allow_threads;
    ret = gst_ring_buffer_device_is_open(GST_RING_BUFFER(self->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_ring_buffer_release(PyGObject *self)
{
    int ret;

    pyg_begin_allow_threads;
    ret = gst_ring_buffer_release(GST_RING_BUFFER(self->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_ring_buffer_is_acquired(PyGObject *self)
{
    int ret;

    pyg_begin_allow_threads;
    ret = gst_ring_buffer_is_acquired(GST_RING_BUFFER(self->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_ring_buffer_activate(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "active", NULL };
    int active, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GstRingBuffer.activate", kwlist, &active))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_ring_buffer_activate(GST_RING_BUFFER(self->obj), active);
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_ring_buffer_is_active(PyGObject *self)
{
    int ret;

    pyg_begin_allow_threads;
    ret = gst_ring_buffer_is_active(GST_RING_BUFFER(self->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_ring_buffer_set_flushing(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "flushing", NULL };
    int flushing;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GstRingBuffer.set_flushing", kwlist, &flushing))
        return NULL;
    pyg_begin_allow_threads;
    gst_ring_buffer_set_flushing(GST_RING_BUFFER(self->obj), flushing);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_ring_buffer_start(PyGObject *self)
{
    int ret;

    pyg_begin_allow_threads;
    ret = gst_ring_buffer_start(GST_RING_BUFFER(self->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_ring_buffer_pause(PyGObject *self)
{
    int ret;

    pyg_begin_allow_threads;
    ret = gst_ring_buffer_pause(GST_RING_BUFFER(self->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_ring_buffer_stop(PyGObject *self)
{
    int ret;

    pyg_begin_allow_threads;
    ret = gst_ring_buffer_stop(GST_RING_BUFFER(self->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_ring_buffer_delay(PyGObject *self)
{
    guint ret;

    pyg_begin_allow_threads;
    ret = gst_ring_buffer_delay(GST_RING_BUFFER(self->obj));
    pyg_end_allow_threads;
    return PyLong_FromUnsignedLong(ret);
}

static PyObject *
_wrap_gst_ring_buffer_samples_done(PyGObject *self)
{
    guint64 ret;

    pyg_begin_allow_threads;
    ret = gst_ring_buffer_samples_done(GST_RING_BUFFER(self->obj));
    pyg_end_allow_threads;
    return PyLong_FromUnsignedLongLong(ret);
}

static PyObject *
_wrap_gst_ring_buffer_set_sample(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "sample", NULL };
    guint64 sample;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"K:GstRingBuffer.set_sample", kwlist, &sample))
        return NULL;
    pyg_begin_allow_threads;
    gst_ring_buffer_set_sample(GST_RING_BUFFER(self->obj), sample);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_ring_buffer_clear_all(PyGObject *self)
{
    pyg_begin_allow_threads;
    gst_ring_buffer_clear_all(GST_RING_BUFFER(self->obj));
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_ring_buffer_commit(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "sample", "data", "len", NULL };
    int data_len;
    PyObject *py_len = NULL;
    guchar *data;
    guint len = 0, ret;
    guint64 sample;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"Ks#O:GstRingBuffer.commit", kwlist, &sample, &data, &data_len, &py_len))
        return NULL;
    if (py_len) {
        if (PyLong_Check(py_len))
            len = PyLong_AsUnsignedLong(py_len);
        else if (PyInt_Check(py_len))
            len = PyInt_AsLong(py_len);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'len' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    pyg_begin_allow_threads;
    ret = gst_ring_buffer_commit(GST_RING_BUFFER(self->obj), sample, data, len);
    pyg_end_allow_threads;
    return PyLong_FromUnsignedLong(ret);
}

static PyObject *
_wrap_gst_ring_buffer_read(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "sample", "data", "len", NULL };
    int data_len;
    PyObject *py_len = NULL;
    guchar *data;
    guint len = 0, ret;
    guint64 sample;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"Ks#O:GstRingBuffer.read", kwlist, &sample, &data, &data_len, &py_len))
        return NULL;
    if (py_len) {
        if (PyLong_Check(py_len))
            len = PyLong_AsUnsignedLong(py_len);
        else if (PyInt_Check(py_len))
            len = PyInt_AsLong(py_len);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'len' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    pyg_begin_allow_threads;
    ret = gst_ring_buffer_read(GST_RING_BUFFER(self->obj), sample, data, len);
    pyg_end_allow_threads;
    return PyLong_FromUnsignedLong(ret);
}

static PyObject *
_wrap_gst_ring_buffer_clear(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "segment", NULL };
    int segment;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GstRingBuffer.clear", kwlist, &segment))
        return NULL;
    pyg_begin_allow_threads;
    gst_ring_buffer_clear(GST_RING_BUFFER(self->obj), segment);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_ring_buffer_advance(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "advance", NULL };
    PyObject *py_advance = NULL;
    guint advance = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:GstRingBuffer.advance", kwlist, &py_advance))
        return NULL;
    if (py_advance) {
        if (PyLong_Check(py_advance))
            advance = PyLong_AsUnsignedLong(py_advance);
        else if (PyInt_Check(py_advance))
            advance = PyInt_AsLong(py_advance);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'advance' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    pyg_begin_allow_threads;
    gst_ring_buffer_advance(GST_RING_BUFFER(self->obj), advance);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_ring_buffer_may_start(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "allowed", NULL };
    int allowed;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GstRingBuffer.may_start", kwlist, &allowed))
        return NULL;
    pyg_begin_allow_threads;
    gst_ring_buffer_may_start(GST_RING_BUFFER(self->obj), allowed);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GstRingBuffer__do_open_device(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstRingBuffer.open_device", kwlist, &PyGstRingBuffer_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_RING_BUFFER_CLASS(klass)->open_device) {
        pyg_begin_allow_threads;
        ret = GST_RING_BUFFER_CLASS(klass)->open_device(GST_RING_BUFFER(self->obj));
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstRingBuffer.open_device not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_GstRingBuffer__do_release(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstRingBuffer.release", kwlist, &PyGstRingBuffer_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_RING_BUFFER_CLASS(klass)->release) {
        pyg_begin_allow_threads;
        ret = GST_RING_BUFFER_CLASS(klass)->release(GST_RING_BUFFER(self->obj));
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstRingBuffer.release not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_GstRingBuffer__do_close_device(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstRingBuffer.close_device", kwlist, &PyGstRingBuffer_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_RING_BUFFER_CLASS(klass)->close_device) {
        pyg_begin_allow_threads;
        ret = GST_RING_BUFFER_CLASS(klass)->close_device(GST_RING_BUFFER(self->obj));
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstRingBuffer.close_device not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_GstRingBuffer__do_start(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstRingBuffer.start", kwlist, &PyGstRingBuffer_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_RING_BUFFER_CLASS(klass)->start) {
        pyg_begin_allow_threads;
        ret = GST_RING_BUFFER_CLASS(klass)->start(GST_RING_BUFFER(self->obj));
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstRingBuffer.start not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_GstRingBuffer__do_pause(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstRingBuffer.pause", kwlist, &PyGstRingBuffer_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_RING_BUFFER_CLASS(klass)->pause) {
        pyg_begin_allow_threads;
        ret = GST_RING_BUFFER_CLASS(klass)->pause(GST_RING_BUFFER(self->obj));
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstRingBuffer.pause not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_GstRingBuffer__do_resume(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstRingBuffer.resume", kwlist, &PyGstRingBuffer_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_RING_BUFFER_CLASS(klass)->resume) {
        pyg_begin_allow_threads;
        ret = GST_RING_BUFFER_CLASS(klass)->resume(GST_RING_BUFFER(self->obj));
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstRingBuffer.resume not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_GstRingBuffer__do_stop(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstRingBuffer.stop", kwlist, &PyGstRingBuffer_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_RING_BUFFER_CLASS(klass)->stop) {
        pyg_begin_allow_threads;
        ret = GST_RING_BUFFER_CLASS(klass)->stop(GST_RING_BUFFER(self->obj));
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstRingBuffer.stop not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_GstRingBuffer__do_delay(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    guint ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstRingBuffer.delay", kwlist, &PyGstRingBuffer_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_RING_BUFFER_CLASS(klass)->delay) {
        pyg_begin_allow_threads;
        ret = GST_RING_BUFFER_CLASS(klass)->delay(GST_RING_BUFFER(self->obj));
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstRingBuffer.delay not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyLong_FromUnsignedLong(ret);
}

static PyObject *
_wrap_GstRingBuffer__do_activate(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "active", NULL };
    PyGObject *self;
    int active, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:GstRingBuffer.activate", kwlist, &PyGstRingBuffer_Type, &self, &active))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_RING_BUFFER_CLASS(klass)->activate) {
        pyg_begin_allow_threads;
        ret = GST_RING_BUFFER_CLASS(klass)->activate(GST_RING_BUFFER(self->obj), active);
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstRingBuffer.activate not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyBool_FromLong(ret);

}

static const PyMethodDef _PyGstRingBuffer_methods[] = {
    { "open_device", (PyCFunction)_wrap_gst_ring_buffer_open_device, METH_NOARGS,
      NULL },
    { "close_device", (PyCFunction)_wrap_gst_ring_buffer_close_device, METH_NOARGS,
      NULL },
    { "device_is_open", (PyCFunction)_wrap_gst_ring_buffer_device_is_open, METH_NOARGS,
      NULL },
    { "release", (PyCFunction)_wrap_gst_ring_buffer_release, METH_NOARGS,
      NULL },
    { "is_acquired", (PyCFunction)_wrap_gst_ring_buffer_is_acquired, METH_NOARGS,
      NULL },
    { "activate", (PyCFunction)_wrap_gst_ring_buffer_activate, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "is_active", (PyCFunction)_wrap_gst_ring_buffer_is_active, METH_NOARGS,
      NULL },
    { "set_flushing", (PyCFunction)_wrap_gst_ring_buffer_set_flushing, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "start", (PyCFunction)_wrap_gst_ring_buffer_start, METH_NOARGS,
      NULL },
    { "pause", (PyCFunction)_wrap_gst_ring_buffer_pause, METH_NOARGS,
      NULL },
    { "stop", (PyCFunction)_wrap_gst_ring_buffer_stop, METH_NOARGS,
      NULL },
    { "delay", (PyCFunction)_wrap_gst_ring_buffer_delay, METH_NOARGS,
      NULL },
    { "samples_done", (PyCFunction)_wrap_gst_ring_buffer_samples_done, METH_NOARGS,
      NULL },
    { "set_sample", (PyCFunction)_wrap_gst_ring_buffer_set_sample, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "clear_all", (PyCFunction)_wrap_gst_ring_buffer_clear_all, METH_NOARGS,
      NULL },
    { "commit", (PyCFunction)_wrap_gst_ring_buffer_commit, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "read", (PyCFunction)_wrap_gst_ring_buffer_read, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "clear", (PyCFunction)_wrap_gst_ring_buffer_clear, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "advance", (PyCFunction)_wrap_gst_ring_buffer_advance, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "may_start", (PyCFunction)_wrap_gst_ring_buffer_may_start, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "do_open_device", (PyCFunction)_wrap_GstRingBuffer__do_open_device, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_release", (PyCFunction)_wrap_GstRingBuffer__do_release, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_close_device", (PyCFunction)_wrap_GstRingBuffer__do_close_device, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_start", (PyCFunction)_wrap_GstRingBuffer__do_start, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_pause", (PyCFunction)_wrap_GstRingBuffer__do_pause, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_resume", (PyCFunction)_wrap_GstRingBuffer__do_resume, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_stop", (PyCFunction)_wrap_GstRingBuffer__do_stop, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_delay", (PyCFunction)_wrap_GstRingBuffer__do_delay, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_activate", (PyCFunction)_wrap_GstRingBuffer__do_activate, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject PyGstRingBuffer_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.audio.RingBuffer",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGstRingBuffer_methods, /* tp_methods */
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

static gboolean
_wrap_GstRingBuffer__proxy_do_open_device(GstRingBuffer *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    gboolean retval;
    PyObject *py_main_retval;
    PyObject *py_retval;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    
    py_method = PyObject_GetAttrString(py_self, "do_open_device");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_retval = PyObject_CallObject(py_method, NULL);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "O", &py_main_retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_DECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gboolean
_wrap_GstRingBuffer__proxy_do_release(GstRingBuffer *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    gboolean retval;
    PyObject *py_main_retval;
    PyObject *py_retval;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    
    py_method = PyObject_GetAttrString(py_self, "do_release");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_retval = PyObject_CallObject(py_method, NULL);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "O", &py_main_retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_DECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gboolean
_wrap_GstRingBuffer__proxy_do_close_device(GstRingBuffer *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    gboolean retval;
    PyObject *py_main_retval;
    PyObject *py_retval;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    
    py_method = PyObject_GetAttrString(py_self, "do_close_device");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_retval = PyObject_CallObject(py_method, NULL);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "O", &py_main_retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_DECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gboolean
_wrap_GstRingBuffer__proxy_do_start(GstRingBuffer *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    gboolean retval;
    PyObject *py_main_retval;
    PyObject *py_retval;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    
    py_method = PyObject_GetAttrString(py_self, "do_start");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_retval = PyObject_CallObject(py_method, NULL);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "O", &py_main_retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_DECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gboolean
_wrap_GstRingBuffer__proxy_do_pause(GstRingBuffer *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    gboolean retval;
    PyObject *py_main_retval;
    PyObject *py_retval;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    
    py_method = PyObject_GetAttrString(py_self, "do_pause");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_retval = PyObject_CallObject(py_method, NULL);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "O", &py_main_retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_DECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gboolean
_wrap_GstRingBuffer__proxy_do_resume(GstRingBuffer *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    gboolean retval;
    PyObject *py_main_retval;
    PyObject *py_retval;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    
    py_method = PyObject_GetAttrString(py_self, "do_resume");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_retval = PyObject_CallObject(py_method, NULL);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "O", &py_main_retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_DECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gboolean
_wrap_GstRingBuffer__proxy_do_stop(GstRingBuffer *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    gboolean retval;
    PyObject *py_main_retval;
    PyObject *py_retval;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    
    py_method = PyObject_GetAttrString(py_self, "do_stop");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_retval = PyObject_CallObject(py_method, NULL);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "O", &py_main_retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_DECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static guint
_wrap_GstRingBuffer__proxy_do_delay(GstRingBuffer *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    guint retval;
    PyObject *py_retval;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    
    
    py_method = PyObject_GetAttrString(py_self, "do_delay");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    py_retval = PyObject_CallObject(py_method, NULL);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "i", &retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    
    
    Py_DECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gboolean
_wrap_GstRingBuffer__proxy_do_activate(GstRingBuffer *self, gboolean active)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_active;
    gboolean retval;
    PyObject *py_main_retval;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_active = active? Py_True : Py_False;
    
    py_args = PyTuple_New(1);
    Py_INCREF(py_active);
    PyTuple_SET_ITEM(py_args, 0, py_active);
    
    py_method = PyObject_GetAttrString(py_self, "do_activate");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "O", &py_main_retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_DECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}

static int
__GstRingBuffer_class_init(gpointer gclass, PyTypeObject *pyclass)
{
    PyObject *o;
    GstRingBufferClass *klass = GST_RING_BUFFER_CLASS(gclass);
    PyObject *gsignals = PyDict_GetItemString(pyclass->tp_dict, "__gsignals__");

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_open_device");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "open_device")))
            klass->open_device = _wrap_GstRingBuffer__proxy_do_open_device;
        Py_DECREF(o);
    }

    /* overriding do_acquire is currently not supported */

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_release");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "release")))
            klass->release = _wrap_GstRingBuffer__proxy_do_release;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_close_device");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "close_device")))
            klass->close_device = _wrap_GstRingBuffer__proxy_do_close_device;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_start");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "start")))
            klass->start = _wrap_GstRingBuffer__proxy_do_start;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_pause");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "pause")))
            klass->pause = _wrap_GstRingBuffer__proxy_do_pause;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_resume");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "resume")))
            klass->resume = _wrap_GstRingBuffer__proxy_do_resume;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_stop");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "stop")))
            klass->stop = _wrap_GstRingBuffer__proxy_do_stop;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_delay");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "delay")))
            klass->delay = _wrap_GstRingBuffer__proxy_do_delay;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_activate");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "activate")))
            klass->activate = _wrap_GstRingBuffer__proxy_do_activate;
        Py_DECREF(o);
    }
    return 0;
}


/* ----------- functions ----------- */

static PyObject *
_wrap_gst_audio_frame_byte_size(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "pad", NULL };
    PyGObject *pad;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:frame_byte_size", kwlist, &PyGstPad_Type, &pad))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_audio_frame_byte_size(GST_PAD(pad->obj));
    pyg_end_allow_threads;
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gst_audio_frame_length(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "pad", "buf", NULL };
    PyGObject *pad;
    long ret;
    PyGstMiniObject *buf;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!:frame_length", kwlist, &PyGstPad_Type, &pad, &PyGstBuffer_Type, &buf))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_audio_frame_length(GST_PAD(pad->obj), GST_BUFFER(buf->obj));
    pyg_end_allow_threads;
    return PyInt_FromLong(ret);

}

static PyObject *
_wrap_gst_audio_duration_from_pad_buffer(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "pad", "buf", NULL };
    PyGObject *pad;
    guint64 ret;
    PyGstMiniObject *buf;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!:duration_from_pad_buffer", kwlist, &PyGstPad_Type, &pad, &PyGstBuffer_Type, &buf))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_audio_duration_from_pad_buffer(GST_PAD(pad->obj), GST_BUFFER(buf->obj));
    pyg_end_allow_threads;
    return PyLong_FromUnsignedLongLong(ret);
}

static PyObject *
_wrap_gst_audio_is_buffer_framed(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "pad", "buf", NULL };
    PyGObject *pad;
    int ret;
    PyGstMiniObject *buf;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!:is_buffer_framed", kwlist, &PyGstPad_Type, &pad, &PyGstBuffer_Type, &buf))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_audio_is_buffer_framed(GST_PAD(pad->obj), GST_BUFFER(buf->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_audio_buffer_clip(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "buffer", "segment", "rate", "frame_size", NULL };
    PyObject *py_segment;
    int rate, frame_size;
    GstSegment *segment = NULL;
    GstBuffer *ret;
    PyGstMiniObject *buffer;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!Oii:buffer_clip", kwlist, &PyGstBuffer_Type, &buffer, &py_segment, &rate, &frame_size))
        return NULL;
    if (pyg_boxed_check(py_segment, GST_TYPE_SEGMENT))
        segment = pyg_boxed_get(py_segment, GstSegment);
    else {
        PyErr_SetString(PyExc_TypeError, "segment should be a GstSegment");
        return NULL;
    }
    pyg_begin_allow_threads;
    ret = gst_audio_buffer_clip(GST_BUFFER(buffer->obj), segment, rate, frame_size);
    pyg_end_allow_threads;
    /* pygobject_new handles NULL checking */
    return pygstminiobject_new((GstMiniObject *)ret);
}

static PyObject *
_wrap_gst_audio_clock_get_time(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "clock", NULL };
    PyGObject *clock;
    guint64 ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:clock_get_time", kwlist, &PyGstClock_Type, &clock))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_audio_clock_get_time(GST_CLOCK(clock->obj));
    pyg_end_allow_threads;
    return PyLong_FromUnsignedLongLong(ret);
}

static PyObject *
_wrap_gst_audio_clock_adjust(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "clock", "time", NULL };
    PyGObject *clock;
    guint64 time, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!K:clock_adjust", kwlist, &PyGstClock_Type, &clock, &time))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_audio_clock_adjust(GST_CLOCK(clock->obj), time);
    pyg_end_allow_threads;
    return PyLong_FromUnsignedLongLong(ret);
}

const PyMethodDef pyaudio_functions[] = {
    { "frame_byte_size", (PyCFunction)_wrap_gst_audio_frame_byte_size, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "frame_length", (PyCFunction)_wrap_gst_audio_frame_length, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "duration_from_pad_buffer", (PyCFunction)_wrap_gst_audio_duration_from_pad_buffer, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "is_buffer_framed", (PyCFunction)_wrap_gst_audio_is_buffer_framed, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "buffer_clip", (PyCFunction)_wrap_gst_audio_buffer_clip, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "clock_get_time", (PyCFunction)_wrap_gst_audio_clock_get_time, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "clock_adjust", (PyCFunction)_wrap_gst_audio_clock_adjust, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};


/* ----------- enums and flags ----------- */

void
pyaudio_add_constants(PyObject *module, const gchar *strip_prefix)
{
  pyg_enum_add(module, "BaseAudioSinkSlaveMethod", strip_prefix, GST_TYPE_BASE_AUDIO_SINK_SLAVE_METHOD);
  pyg_enum_add(module, "BaseAudioSrcSlaveMethod", strip_prefix, GST_TYPE_BASE_AUDIO_SRC_SLAVE_METHOD);
  pyg_enum_add(module, "RingBufferState", strip_prefix, GST_TYPE_RING_BUFFER_STATE);
  pyg_enum_add(module, "RingBufferSegState", strip_prefix, GST_TYPE_RING_BUFFER_SEG_STATE);
  pyg_enum_add(module, "BufferFormatType", strip_prefix, GST_TYPE_BUFFER_FORMAT_TYPE);
  pyg_enum_add(module, "BufferFormat", strip_prefix, GST_TYPE_BUFFER_FORMAT);
  pyg_enum_add(module, "AudioChannelPosition", strip_prefix, GST_TYPE_AUDIO_CHANNEL_POSITION);

  if (PyErr_Occurred())
    PyErr_Print();
}

/* initialise stuff extension classes */
void
pyaudio_register_classes(PyObject *d)
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
        _PyGstClock_Type = (PyTypeObject *)PyDict_GetItemString(moddict, "Clock");
        if (_PyGstClock_Type == NULL) {
            PyErr_SetString(PyExc_ImportError,
                "cannot import name Clock from gst");
            return;
        }
    } else {
        PyErr_SetString(PyExc_ImportError,
            "could not import gst");
        return;
    }


#line 2474 "..\\..\\..\\Source\\gst-python\\gst\\audio.c"
    pygobject_register_class(d, "GstAudioClock", GST_TYPE_AUDIO_CLOCK, &PyGstAudioClock_Type, Py_BuildValue("(O)", &PyGstSystemClock_Type));
    pygobject_register_class(d, "GstAudioFilter", GST_TYPE_AUDIO_FILTER, &PyGstAudioFilter_Type, Py_BuildValue("(O)", &PyGstBaseTransform_Type));
    pyg_register_class_init(GST_TYPE_AUDIO_FILTER, __GstAudioFilter_class_init);
    pygobject_register_class(d, "GstBaseAudioSink", GST_TYPE_BASE_AUDIO_SINK, &PyGstBaseAudioSink_Type, Py_BuildValue("(O)", &PyGstBaseSink_Type));
    pyg_register_class_init(GST_TYPE_BASE_AUDIO_SINK, __GstBaseAudioSink_class_init);
    pygobject_register_class(d, "GstAudioSink", GST_TYPE_AUDIO_SINK, &PyGstAudioSink_Type, Py_BuildValue("(O)", &PyGstBaseAudioSink_Type));
    pyg_register_class_init(GST_TYPE_AUDIO_SINK, __GstAudioSink_class_init);
    pygobject_register_class(d, "GstRingBuffer", GST_TYPE_RING_BUFFER, &PyGstRingBuffer_Type, Py_BuildValue("(O)", &PyGstObject_Type));
    pyg_register_class_init(GST_TYPE_RING_BUFFER, __GstRingBuffer_class_init);
}
