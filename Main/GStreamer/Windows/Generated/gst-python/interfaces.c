/* -- THIS FILE IS GENERATED - DO NOT EDIT *//* -*- Mode: C; c-basic-offset: 4 -*- */

#include <Python.h>



#line 24 "..\\..\\..\\Source\\gst-python\\gst\\interfaces.override"
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define NO_IMPORT_PYGOBJECT

#include "common.h"

#include <gst/gst.h>

#include <gst/interfaces/colorbalance.h>
#include <gst/interfaces/navigation.h>
#include <gst/interfaces/propertyprobe.h>
#include <gst/interfaces/tuner.h>
#include <gst/interfaces/mixer.h>
#ifdef HAVE_VIDEO_ORIENTATION_INTERFACE
#include <gst/interfaces/videoorientation.h>
#endif
#ifdef HAVE_STREAM_VOLUME_INTERFACE
#include <gst/interfaces/streamvolume.h>
#endif


#line 23 "xoverlay.override"

#include <gst/interfaces/xoverlay.h>
#line 35 "..\\..\\..\\Source\\gst-python\\gst\\interfaces.c"


/* ---------- types from other modules ---------- */
static PyTypeObject *_PyGObject_Type;
#define PyGObject_Type (*_PyGObject_Type)
static PyTypeObject *_PyGstObject_Type;
#define PyGstObject_Type (*_PyGstObject_Type)
static PyTypeObject *_PyGstStructure_Type;
#define PyGstStructure_Type (*_PyGstStructure_Type)
static PyTypeObject *_PyGstElement_Type;
#define PyGstElement_Type (*_PyGstElement_Type)
static PyTypeObject *_PyGstQuery_Type;
#define PyGstQuery_Type (*_PyGstQuery_Type)


/* ---------- forward type declarations ---------- */
PyTypeObject PyGstColorBalanceChannel_Type;
PyTypeObject PyGstMixerTrack_Type;
PyTypeObject PyGstMixerOptions_Type;
PyTypeObject PyGstTunerChannel_Type;
PyTypeObject PyGstTunerNorm_Type;
PyTypeObject PyGstXOverlay_Type;
PyTypeObject PyGstColorBalance_Type;
PyTypeObject PyGstMixer_Type;
PyTypeObject PyGstNavigation_Type;
PyTypeObject PyGstPropertyProbe_Type;
PyTypeObject PyGstTuner_Type;
PyTypeObject PyGstVideoOrientation_Type;

#line 65 "..\\..\\..\\Source\\gst-python\\gst\\interfaces.c"



/* ----------- GstColorBalanceChannel ----------- */

static PyObject *
_wrap_gst_color_balance_channel__get_label(PyObject *self, void *closure)
{
    const gchar *ret;

    ret = GST_COLOR_BALANCE_CHANNEL(pygobject_get(self))->label;
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_color_balance_channel__get_min_value(PyObject *self, void *closure)
{
    int ret;

    ret = GST_COLOR_BALANCE_CHANNEL(pygobject_get(self))->min_value;
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gst_color_balance_channel__get_max_value(PyObject *self, void *closure)
{
    int ret;

    ret = GST_COLOR_BALANCE_CHANNEL(pygobject_get(self))->max_value;
    return PyInt_FromLong(ret);
}

static const PyGetSetDef gst_color_balance_channel_getsets[] = {
    { "label", (getter)_wrap_gst_color_balance_channel__get_label, (setter)0 },
    { "min_value", (getter)_wrap_gst_color_balance_channel__get_min_value, (setter)0 },
    { "max_value", (getter)_wrap_gst_color_balance_channel__get_max_value, (setter)0 },
    { NULL, (getter)0, (setter)0 },
};

PyTypeObject PyGstColorBalanceChannel_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.interfaces.ColorBalanceChannel",                   /* tp_name */
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
    (struct PyGetSetDef*)gst_color_balance_channel_getsets,  /* tp_getset */
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



/* ----------- GstMixerTrack ----------- */

static PyObject *
_wrap_gst_mixer_track__get_label(PyObject *self, void *closure)
{
    const gchar *ret;

    ret = GST_MIXER_TRACK(pygobject_get(self))->label;
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_mixer_track__get_flags(PyObject *self, void *closure)
{
    guint ret;

    ret = GST_MIXER_TRACK(pygobject_get(self))->flags;
    return pyg_flags_from_gtype(GST_TYPE_MIXER_TRACK_FLAGS, ret);
}

static PyObject *
_wrap_gst_mixer_track__get_num_channels(PyObject *self, void *closure)
{
    int ret;

    ret = GST_MIXER_TRACK(pygobject_get(self))->num_channels;
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gst_mixer_track__get_min_volume(PyObject *self, void *closure)
{
    int ret;

    ret = GST_MIXER_TRACK(pygobject_get(self))->min_volume;
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gst_mixer_track__get_max_volume(PyObject *self, void *closure)
{
    int ret;

    ret = GST_MIXER_TRACK(pygobject_get(self))->max_volume;
    return PyInt_FromLong(ret);
}

static const PyGetSetDef gst_mixer_track_getsets[] = {
    { "label", (getter)_wrap_gst_mixer_track__get_label, (setter)0 },
    { "flags", (getter)_wrap_gst_mixer_track__get_flags, (setter)0 },
    { "num_channels", (getter)_wrap_gst_mixer_track__get_num_channels, (setter)0 },
    { "min_volume", (getter)_wrap_gst_mixer_track__get_min_volume, (setter)0 },
    { "max_volume", (getter)_wrap_gst_mixer_track__get_max_volume, (setter)0 },
    { NULL, (getter)0, (setter)0 },
};

PyTypeObject PyGstMixerTrack_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.interfaces.MixerTrack",                   /* tp_name */
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
    (struct PyGetSetDef*)gst_mixer_track_getsets,  /* tp_getset */
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



/* ----------- GstMixerOptions ----------- */

#line 161 "..\\..\\..\\Source\\gst-python\\gst\\interfaces.override"
static PyObject *
_wrap_gst_mixer_options_get_values (PyGObject *self)
{
    GList	*l, *list;
    PyObject	*py_list;

    g_return_val_if_fail (GST_IS_MIXER_OPTIONS (self->obj), PyList_New(0));

    pyg_begin_allow_threads;
    list = gst_mixer_options_get_values (GST_MIXER_OPTIONS (self->obj));
    pyg_end_allow_threads;

    py_list = PyList_New(0);
    for (l = list; l; l = l->next) {
	gchar *value = (gchar *) l->data;
	PyObject *py_string = PyString_FromString(g_strdup(value));
	PyList_Append(py_list, py_string);
	Py_DECREF (py_string);
    }

    return py_list;
}

#line 287 "..\\..\\..\\Source\\gst-python\\gst\\interfaces.c"


static const PyMethodDef _PyGstMixerOptions_methods[] = {
    { "get_values", (PyCFunction)_wrap_gst_mixer_options_get_values, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject PyGstMixerOptions_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.interfaces.MixerOptions",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGstMixerOptions_methods, /* tp_methods */
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



/* ----------- GstTunerChannel ----------- */

static PyObject *
_wrap_gst_tuner_channel__get_label(PyObject *self, void *closure)
{
    const gchar *ret;

    ret = GST_TUNER_CHANNEL(pygobject_get(self))->label;
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_tuner_channel__get_flags(PyObject *self, void *closure)
{
    guint ret;

    ret = GST_TUNER_CHANNEL(pygobject_get(self))->flags;
    return pyg_flags_from_gtype(GST_TYPE_TUNER_CHANNEL_FLAGS, ret);
}

static PyObject *
_wrap_gst_tuner_channel__get_freq_multiplicator(PyObject *self, void *closure)
{
    double ret;

    ret = GST_TUNER_CHANNEL(pygobject_get(self))->freq_multiplicator;
    return PyFloat_FromDouble(ret);
}

static PyObject *
_wrap_gst_tuner_channel__get_min_frequency(PyObject *self, void *closure)
{
    gulong ret;

    ret = GST_TUNER_CHANNEL(pygobject_get(self))->min_frequency;
    return PyLong_FromUnsignedLong(ret);

}

static PyObject *
_wrap_gst_tuner_channel__get_max_frequency(PyObject *self, void *closure)
{
    gulong ret;

    ret = GST_TUNER_CHANNEL(pygobject_get(self))->max_frequency;
    return PyLong_FromUnsignedLong(ret);

}

static PyObject *
_wrap_gst_tuner_channel__get_min_signal(PyObject *self, void *closure)
{
    int ret;

    ret = GST_TUNER_CHANNEL(pygobject_get(self))->min_signal;
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gst_tuner_channel__get_max_signal(PyObject *self, void *closure)
{
    int ret;

    ret = GST_TUNER_CHANNEL(pygobject_get(self))->max_signal;
    return PyInt_FromLong(ret);
}

static const PyGetSetDef gst_tuner_channel_getsets[] = {
    { "label", (getter)_wrap_gst_tuner_channel__get_label, (setter)0 },
    { "flags", (getter)_wrap_gst_tuner_channel__get_flags, (setter)0 },
    { "freq_multiplicator", (getter)_wrap_gst_tuner_channel__get_freq_multiplicator, (setter)0 },
    { "min_frequency", (getter)_wrap_gst_tuner_channel__get_min_frequency, (setter)0 },
    { "max_frequency", (getter)_wrap_gst_tuner_channel__get_max_frequency, (setter)0 },
    { "min_signal", (getter)_wrap_gst_tuner_channel__get_min_signal, (setter)0 },
    { "max_signal", (getter)_wrap_gst_tuner_channel__get_max_signal, (setter)0 },
    { NULL, (getter)0, (setter)0 },
};

PyTypeObject PyGstTunerChannel_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.interfaces.TunerChannel",                   /* tp_name */
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
    (struct PyGetSetDef*)gst_tuner_channel_getsets,  /* tp_getset */
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



/* ----------- GstTunerNorm ----------- */

static PyObject *
_wrap_gst_tuner_norm__get_label(PyObject *self, void *closure)
{
    const gchar *ret;

    ret = GST_TUNER_NORM(pygobject_get(self))->label;
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyGetSetDef gst_tuner_norm_getsets[] = {
    { "label", (getter)_wrap_gst_tuner_norm__get_label, (setter)0 },
    { NULL, (getter)0, (setter)0 },
};

PyTypeObject PyGstTunerNorm_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.interfaces.TunerNorm",                   /* tp_name */
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
    (struct PyGetSetDef*)gst_tuner_norm_getsets,  /* tp_getset */
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



/* ----------- GstXOverlay ----------- */

static PyObject *
_wrap_gst_x_overlay_set_xwindow_id(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "xwindow_id", NULL };
    unsigned long xwindow_id;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"k:GstXOverlay.set_xwindow_id", kwlist, &xwindow_id))
        return NULL;
    pyg_begin_allow_threads;
    gst_x_overlay_set_xwindow_id(GST_X_OVERLAY(self->obj), xwindow_id);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_x_overlay_expose(PyGObject *self)
{
    pyg_begin_allow_threads;
    gst_x_overlay_expose(GST_X_OVERLAY(self->obj));
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_x_overlay_got_xwindow_id(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "xwindow_id", NULL };
    unsigned long xwindow_id;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"k:GstXOverlay.got_xwindow_id", kwlist, &xwindow_id))
        return NULL;
    pyg_begin_allow_threads;
    gst_x_overlay_got_xwindow_id(GST_X_OVERLAY(self->obj), xwindow_id);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_x_overlay_prepare_xwindow_id(PyGObject *self)
{
    pyg_begin_allow_threads;
    gst_x_overlay_prepare_xwindow_id(GST_X_OVERLAY(self->obj));
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGstXOverlay_methods[] = {
    { "set_xwindow_id", (PyCFunction)_wrap_gst_x_overlay_set_xwindow_id, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "expose", (PyCFunction)_wrap_gst_x_overlay_expose, METH_NOARGS,
      NULL },
    { "got_xwindow_id", (PyCFunction)_wrap_gst_x_overlay_got_xwindow_id, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "prepare_xwindow_id", (PyCFunction)_wrap_gst_x_overlay_prepare_xwindow_id, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject PyGstXOverlay_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.interfaces.XOverlay",                   /* tp_name */
    sizeof(PyObject),          /* tp_basicsize */
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
    0,             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGstXOverlay_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    0,                 /* tp_dictoffset */
    (initproc)0,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GstColorBalance ----------- */

#line 137 "..\\..\\..\\Source\\gst-python\\gst\\interfaces.override"
static PyObject *
_wrap_gst_color_balance_list_channels(PyGObject *self)
{
    const GList *l, *list;
    PyObject *py_list;

    g_return_val_if_fail (GST_IS_COLOR_BALANCE (self->obj), PyList_New(0));

    pyg_begin_allow_threads;
    list = gst_color_balance_list_channels(GST_COLOR_BALANCE(self->obj));
    pyg_end_allow_threads;

    py_list = PyList_New(0);
    for (l = list; l; l = l->next) {
        GstColorBalanceChannel *channel = (GstColorBalanceChannel*)l->data;
        PyObject *py_channel = pygobject_new(G_OBJECT(channel));
        PyList_Append(py_list, py_channel);
        Py_DECREF(py_channel);
    }

    return py_list;
}
#line 673 "..\\..\\..\\Source\\gst-python\\gst\\interfaces.c"


static PyObject *
_wrap_gst_color_balance_set_value(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "channel", "value", NULL };
    PyGObject *channel;
    int value;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:GstColorBalance.set_value", kwlist, &PyGstColorBalanceChannel_Type, &channel, &value))
        return NULL;
    pyg_begin_allow_threads;
    gst_color_balance_set_value(GST_COLOR_BALANCE(self->obj), GST_COLOR_BALANCE_CHANNEL(channel->obj), value);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_color_balance_get_value(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "channel", NULL };
    PyGObject *channel;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstColorBalance.get_value", kwlist, &PyGstColorBalanceChannel_Type, &channel))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_color_balance_get_value(GST_COLOR_BALANCE(self->obj), GST_COLOR_BALANCE_CHANNEL(channel->obj));
    pyg_end_allow_threads;
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gst_color_balance_value_changed(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "channel", "value", NULL };
    PyGObject *channel;
    int value;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:GstColorBalance.value_changed", kwlist, &PyGstColorBalanceChannel_Type, &channel, &value))
        return NULL;
    pyg_begin_allow_threads;
    gst_color_balance_value_changed(GST_COLOR_BALANCE(self->obj), GST_COLOR_BALANCE_CHANNEL(channel->obj), value);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GstColorBalance__do_set_value(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GstColorBalanceClass *iface;
    static char *kwlist[] = { "self", "channel", "value", NULL };
    PyGObject *self, *channel;
    int value;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!i:GstColorBalance.set_value", kwlist, &PyGstColorBalance_Type, &self, &PyGstColorBalanceChannel_Type, &channel, &value))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GST_TYPE_COLOR_BALANCE);
    if (iface->set_value)
        iface->set_value(GST_COLOR_BALANCE(self->obj), GST_COLOR_BALANCE_CHANNEL(channel->obj), value);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GstColorBalance.set_value not implemented");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GstColorBalance__do_get_value(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GstColorBalanceClass *iface;
    static char *kwlist[] = { "self", "channel", NULL };
    PyGObject *self, *channel;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!:GstColorBalance.get_value", kwlist, &PyGstColorBalance_Type, &self, &PyGstColorBalanceChannel_Type, &channel))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GST_TYPE_COLOR_BALANCE);
    if (iface->get_value)
        ret = iface->get_value(GST_COLOR_BALANCE(self->obj), GST_COLOR_BALANCE_CHANNEL(channel->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GstColorBalance.get_value not implemented");
        return NULL;
    }
    return PyInt_FromLong(ret);
}

static const PyMethodDef _PyGstColorBalance_methods[] = {
    { "list_colorbalance_channels", (PyCFunction)_wrap_gst_color_balance_list_channels, METH_NOARGS,
      NULL },
    { "set_value", (PyCFunction)_wrap_gst_color_balance_set_value, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_value", (PyCFunction)_wrap_gst_color_balance_get_value, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "value_changed", (PyCFunction)_wrap_gst_color_balance_value_changed, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "do_set_value", (PyCFunction)_wrap_GstColorBalance__do_set_value, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_value", (PyCFunction)_wrap_GstColorBalance__do_get_value, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject PyGstColorBalance_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.interfaces.ColorBalance",                   /* tp_name */
    sizeof(PyObject),          /* tp_basicsize */
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
    0,             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGstColorBalance_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    0,                 /* tp_dictoffset */
    (initproc)0,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};

static void
_wrap_GstColorBalance__proxy_do_set_value(GstColorBalance *self, GstColorBalanceChannel*channel, gint value)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_channel = NULL;
    PyObject *py_value;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return;
    }
    if (channel)
        py_channel = pygobject_new((GObject *) channel);
    else {
        Py_INCREF(Py_None);
        py_channel = Py_None;
    }
    py_value = PyInt_FromLong(value);
    
    py_args = PyTuple_New(2);
    PyTuple_SET_ITEM(py_args, 0, py_channel);
    PyTuple_SET_ITEM(py_args, 1, py_value);
    
    py_method = PyObject_GetAttrString(py_self, "do_set_value");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_method);
        Py_DECREF(py_args);
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
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    Py_DECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
}
static gint
_wrap_GstColorBalance__proxy_do_get_value(GstColorBalance *self, GstColorBalanceChannel*channel)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_channel = NULL;
    gint retval;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    if (channel)
        py_channel = pygobject_new((GObject *) channel);
    else {
        Py_INCREF(Py_None);
        py_channel = Py_None;
    }
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_channel);
    
    py_method = PyObject_GetAttrString(py_self, "do_get_value");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_method);
        Py_DECREF(py_args);
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
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    
    
    Py_DECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}

static void
__GstColorBalance__interface_init(GstColorBalanceClass *iface, PyTypeObject *pytype)
{
    GstColorBalanceClass *parent_iface = g_type_interface_peek_parent(iface);
    PyObject *py_method;

    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_set_value") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->set_value = _wrap_GstColorBalance__proxy_do_set_value;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->set_value = parent_iface->set_value;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_get_value") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->get_value = _wrap_GstColorBalance__proxy_do_get_value;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->get_value = parent_iface->get_value;
        }
    Py_XDECREF(py_method);
    }
}


static const GInterfaceInfo __GstColorBalance__iinfo = {
    (GInterfaceInitFunc) __GstColorBalance__interface_init,
    NULL,
    NULL
};


/* ----------- GstMixer ----------- */

#line 113 "..\\..\\..\\Source\\gst-python\\gst\\interfaces.override"
static PyObject *
_wrap_gst_mixer_list_tracks(PyGObject *self)
{
    const GList *l, *list;
    PyObject *py_list;
	
    g_return_val_if_fail (GST_IS_MIXER (self->obj), PyList_New(0));

    pyg_begin_allow_threads;
    list = gst_mixer_list_tracks(GST_MIXER(self->obj));
    pyg_end_allow_threads;
    
    py_list = PyList_New(0);
    for (l = list; l; l = l->next) {
	GstMixerTrack *track = (GstMixerTrack*)l->data;
	PyObject *py_track = pygobject_new(G_OBJECT(track));
	PyList_Append(py_list, py_track);
	Py_DECREF(py_track);
    }
		
    return py_list;
}
#line 1025 "..\\..\\..\\Source\\gst-python\\gst\\interfaces.c"


#line 186 "..\\..\\..\\Source\\gst-python\\gst\\interfaces.override"
static PyObject *
_wrap_gst_mixer_set_volume (PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "track", "volumes", NULL };
    PyGObject *track;
    PyObject *py_tuple;
    gint *volumes = NULL;
    gint channels;
    int i;
    PyObject *ret;

    if (!PyArg_ParseTupleAndKeywords (args, kwargs, "O!O:GstMixer.set_volume",
        kwlist, &PyGstMixerTrack_Type, &track, &py_tuple))
      return NULL;

    g_object_get (GST_MIXER_TRACK (track->obj), "num-channels", &channels,
      NULL);

    if (channels != PyTuple_Size (py_tuple)) {
      PyErr_Format (PyExc_TypeError,
          "Track channel count %d != volume tuple size %d",
          channels, (gint) PyTuple_Size (py_tuple));
      return NULL;
    }

    Py_INCREF(Py_None);
    ret = Py_None;

    if (channels == 0)
      return ret;

    volumes = g_malloc (channels * sizeof (gint));
    for (i = 0; i < channels; ++i) {
      volumes[i] = PyInt_AsLong (PyTuple_GET_ITEM (py_tuple, i));
    }
    
    pyg_begin_allow_threads;
    gst_mixer_set_volume (GST_MIXER (self->obj), GST_MIXER_TRACK (track->obj),
          volumes);
    pyg_end_allow_threads;

    g_free (volumes);

    return ret;
}

#line 1075 "..\\..\\..\\Source\\gst-python\\gst\\interfaces.c"


#line 234 "..\\..\\..\\Source\\gst-python\\gst\\interfaces.override"
static PyObject *
_wrap_gst_mixer_get_volume (PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "track", NULL };
    PyGObject *track;
    PyObject *py_tuple;
    gint *volumes = NULL;
    gint channels;
    int i;

    if (!PyArg_ParseTupleAndKeywords (args, kwargs, "O!:GstMixer.get_volume",
        kwlist, &PyGstMixerTrack_Type, &track))
      return NULL;

    g_object_get (GST_MIXER_TRACK (track->obj), "num-channels", &channels,
      NULL);

    volumes = g_malloc (channels * sizeof (gint));
    /* 0 channels will cause volume to be a NULL pointer, but we still want
     * our (empty) tuple */
    if (channels) {
	pyg_begin_allow_threads;
	gst_mixer_get_volume (GST_MIXER (self->obj), GST_MIXER_TRACK (track->obj),
			      volumes);
	pyg_end_allow_threads;
    }

    py_tuple = PyTuple_New (channels);

    for (i = 0; i < channels; ++i) {
      PyTuple_SET_ITEM (py_tuple, i, PyInt_FromLong (volumes[i]));
    }
    g_free (volumes);

    return py_tuple;
}

#line 1116 "..\\..\\..\\Source\\gst-python\\gst\\interfaces.c"


static PyObject *
_wrap_gst_mixer_set_mute(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "track", "mute", NULL };
    PyGObject *track;
    int mute;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:GstMixer.set_mute", kwlist, &PyGstMixerTrack_Type, &track, &mute))
        return NULL;
    pyg_begin_allow_threads;
    gst_mixer_set_mute(GST_MIXER(self->obj), GST_MIXER_TRACK(track->obj), mute);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_mixer_set_record(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "track", "record", NULL };
    PyGObject *track;
    int record;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:GstMixer.set_record", kwlist, &PyGstMixerTrack_Type, &track, &record))
        return NULL;
    pyg_begin_allow_threads;
    gst_mixer_set_record(GST_MIXER(self->obj), GST_MIXER_TRACK(track->obj), record);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_mixer_set_option(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "opts", "value", NULL };
    PyGObject *opts;
    char *value;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!s:GstMixer.set_option", kwlist, &PyGstMixerOptions_Type, &opts, &value))
        return NULL;
    pyg_begin_allow_threads;
    gst_mixer_set_option(GST_MIXER(self->obj), GST_MIXER_OPTIONS(opts->obj), value);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_mixer_get_option(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "opts", NULL };
    PyGObject *opts;
    const gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstMixer.get_option", kwlist, &PyGstMixerOptions_Type, &opts))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_mixer_get_option(GST_MIXER(self->obj), GST_MIXER_OPTIONS(opts->obj));
    pyg_end_allow_threads;
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_mixer_mute_toggled(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "track", "mute", NULL };
    PyGObject *track;
    int mute;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:GstMixer.mute_toggled", kwlist, &PyGstMixerTrack_Type, &track, &mute))
        return NULL;
    pyg_begin_allow_threads;
    gst_mixer_mute_toggled(GST_MIXER(self->obj), GST_MIXER_TRACK(track->obj), mute);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_mixer_record_toggled(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "track", "record", NULL };
    PyGObject *track;
    int record;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:GstMixer.record_toggled", kwlist, &PyGstMixerTrack_Type, &track, &record))
        return NULL;
    pyg_begin_allow_threads;
    gst_mixer_record_toggled(GST_MIXER(self->obj), GST_MIXER_TRACK(track->obj), record);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_mixer_option_changed(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "opts", "value", NULL };
    PyGObject *opts;
    char *value;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!s:GstMixer.option_changed", kwlist, &PyGstMixerOptions_Type, &opts, &value))
        return NULL;
    pyg_begin_allow_threads;
    gst_mixer_option_changed(GST_MIXER(self->obj), GST_MIXER_OPTIONS(opts->obj), value);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_mixer_get_mixer_flags(PyGObject *self)
{
    guint ret;

    pyg_begin_allow_threads;
    ret = gst_mixer_get_mixer_flags(GST_MIXER(self->obj));
    pyg_end_allow_threads;
    return pyg_flags_from_gtype(GST_TYPE_MIXER_FLAGS, ret);
}

static PyObject *
_wrap_GstMixer__do_set_mute(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GstMixerClass *iface;
    static char *kwlist[] = { "self", "track", "mute", NULL };
    PyGObject *self, *track;
    int mute;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!i:GstMixer.set_mute", kwlist, &PyGstMixer_Type, &self, &PyGstMixerTrack_Type, &track, &mute))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GST_TYPE_MIXER);
    if (iface->set_mute)
        iface->set_mute(GST_MIXER(self->obj), GST_MIXER_TRACK(track->obj), mute);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GstMixer.set_mute not implemented");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GstMixer__do_set_record(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GstMixerClass *iface;
    static char *kwlist[] = { "self", "track", "record", NULL };
    PyGObject *self, *track;
    int record;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!i:GstMixer.set_record", kwlist, &PyGstMixer_Type, &self, &PyGstMixerTrack_Type, &track, &record))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GST_TYPE_MIXER);
    if (iface->set_record)
        iface->set_record(GST_MIXER(self->obj), GST_MIXER_TRACK(track->obj), record);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GstMixer.set_record not implemented");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGstMixer_methods[] = {
    { "list_tracks", (PyCFunction)_wrap_gst_mixer_list_tracks, METH_NOARGS,
      NULL },
    { "set_volume", (PyCFunction)_wrap_gst_mixer_set_volume, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_volume", (PyCFunction)_wrap_gst_mixer_get_volume, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_mute", (PyCFunction)_wrap_gst_mixer_set_mute, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_record", (PyCFunction)_wrap_gst_mixer_set_record, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_option", (PyCFunction)_wrap_gst_mixer_set_option, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_option", (PyCFunction)_wrap_gst_mixer_get_option, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "mute_toggled", (PyCFunction)_wrap_gst_mixer_mute_toggled, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "record_toggled", (PyCFunction)_wrap_gst_mixer_record_toggled, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "option_changed", (PyCFunction)_wrap_gst_mixer_option_changed, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_mixer_flags", (PyCFunction)_wrap_gst_mixer_get_mixer_flags, METH_NOARGS,
      NULL },
    { "do_set_mute", (PyCFunction)_wrap_GstMixer__do_set_mute, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_set_record", (PyCFunction)_wrap_GstMixer__do_set_record, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject PyGstMixer_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.interfaces.Mixer",                   /* tp_name */
    sizeof(PyObject),          /* tp_basicsize */
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
    0,             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGstMixer_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    0,                 /* tp_dictoffset */
    (initproc)0,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};

static void
_wrap_GstMixer__proxy_do_set_mute(GstMixer *self, GstMixerTrack*track, gboolean mute)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_track = NULL;
    PyObject *py_mute;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return;
    }
    if (track)
        py_track = pygobject_new((GObject *) track);
    else {
        Py_INCREF(Py_None);
        py_track = Py_None;
    }
    py_mute = mute? Py_True : Py_False;
    
    py_args = PyTuple_New(2);
    PyTuple_SET_ITEM(py_args, 0, py_track);
    Py_INCREF(py_mute);
    PyTuple_SET_ITEM(py_args, 1, py_mute);
    
    py_method = PyObject_GetAttrString(py_self, "do_set_mute");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_method);
        Py_DECREF(py_args);
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
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    Py_DECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
}
static void
_wrap_GstMixer__proxy_do_set_record(GstMixer *self, GstMixerTrack*track, gboolean record)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_track = NULL;
    PyObject *py_record;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return;
    }
    if (track)
        py_track = pygobject_new((GObject *) track);
    else {
        Py_INCREF(Py_None);
        py_track = Py_None;
    }
    py_record = record? Py_True : Py_False;
    
    py_args = PyTuple_New(2);
    PyTuple_SET_ITEM(py_args, 0, py_track);
    Py_INCREF(py_record);
    PyTuple_SET_ITEM(py_args, 1, py_record);
    
    py_method = PyObject_GetAttrString(py_self, "do_set_record");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_method);
        Py_DECREF(py_args);
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
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    Py_DECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
}

static void
__GstMixer__interface_init(GstMixerClass *iface, PyTypeObject *pytype)
{
    GstMixerClass *parent_iface = g_type_interface_peek_parent(iface);
    PyObject *py_method;

    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_set_mute") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->set_mute = _wrap_GstMixer__proxy_do_set_mute;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->set_mute = parent_iface->set_mute;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_set_record") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->set_record = _wrap_GstMixer__proxy_do_set_record;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->set_record = parent_iface->set_record;
        }
    Py_XDECREF(py_method);
    }
}


static const GInterfaceInfo __GstMixer__iinfo = {
    (GInterfaceInitFunc) __GstMixer__interface_init,
    NULL,
    NULL
};


/* ----------- GstNavigation ----------- */

static PyObject *
_wrap_gst_navigation_send_event(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "structure", NULL };
    PyObject *py_structure;
    GstStructure *structure = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:GstNavigation.send_event", kwlist, &py_structure))
        return NULL;
    if (pyg_boxed_check(py_structure, GST_TYPE_STRUCTURE))
        structure = pyg_boxed_get(py_structure, GstStructure);
    else {
        PyErr_SetString(PyExc_TypeError, "structure should be a GstStructure");
        return NULL;
    }
    pyg_begin_allow_threads;
    gst_navigation_send_event(GST_NAVIGATION(self->obj), structure);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_navigation_send_key_event(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "event", "key", NULL };
    char *event, *key;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ss:GstNavigation.send_key_event", kwlist, &event, &key))
        return NULL;
    pyg_begin_allow_threads;
    gst_navigation_send_key_event(GST_NAVIGATION(self->obj), event, key);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_navigation_send_mouse_event(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "event", "button", "x", "y", NULL };
    char *event;
    int button;
    double x, y;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"sidd:GstNavigation.send_mouse_event", kwlist, &event, &button, &x, &y))
        return NULL;
    pyg_begin_allow_threads;
    gst_navigation_send_mouse_event(GST_NAVIGATION(self->obj), event, button, x, y);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_navigation_send_command(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "command", NULL };
    PyObject *py_command = NULL;
    GstNavigationCommand command;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:GstNavigation.send_command", kwlist, &py_command))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_NAVIGATION_COMMAND, py_command, (gint *)&command))
        return NULL;
    pyg_begin_allow_threads;
    gst_navigation_send_command(GST_NAVIGATION(self->obj), command);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GstNavigation__do_send_event(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GstNavigationInterface *iface;
    static char *kwlist[] = { "self", "structure", NULL };
    PyGObject *self;
    PyObject *py_structure;
    GstStructure *structure = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O:GstNavigation.send_event", kwlist, &PyGstNavigation_Type, &self, &py_structure))
        return NULL;
    if (pyg_boxed_check(py_structure, GST_TYPE_STRUCTURE))
        structure = pyg_boxed_get(py_structure, GstStructure);
    else {
        PyErr_SetString(PyExc_TypeError, "structure should be a GstStructure");
        return NULL;
    }
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GST_TYPE_NAVIGATION);
    if (iface->send_event)
        iface->send_event(GST_NAVIGATION(self->obj), structure);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GstNavigation.send_event not implemented");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGstNavigation_methods[] = {
    { "send_event", (PyCFunction)_wrap_gst_navigation_send_event, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "send_key_event", (PyCFunction)_wrap_gst_navigation_send_key_event, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "send_mouse_event", (PyCFunction)_wrap_gst_navigation_send_mouse_event, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "send_command", (PyCFunction)_wrap_gst_navigation_send_command, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "do_send_event", (PyCFunction)_wrap_GstNavigation__do_send_event, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject PyGstNavigation_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.interfaces.Navigation",                   /* tp_name */
    sizeof(PyObject),          /* tp_basicsize */
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
    0,             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGstNavigation_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    0,                 /* tp_dictoffset */
    (initproc)0,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};

static void
_wrap_GstNavigation__proxy_do_send_event(GstNavigation *self, GstStructure*structure)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_structure;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return;
    }
    py_structure = pyg_boxed_new(GST_TYPE_STRUCTURE, structure, FALSE, FALSE);
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_structure);
    
    py_method = PyObject_GetAttrString(py_self, "do_send_event");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_method);
        Py_DECREF(py_args);
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
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    Py_DECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
}

static void
__GstNavigation__interface_init(GstNavigationInterface *iface, PyTypeObject *pytype)
{
    GstNavigationInterface *parent_iface = g_type_interface_peek_parent(iface);
    PyObject *py_method;

    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_send_event") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->send_event = _wrap_GstNavigation__proxy_do_send_event;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->send_event = parent_iface->send_event;
        }
    Py_XDECREF(py_method);
    }
}


static const GInterfaceInfo __GstNavigation__iinfo = {
    (GInterfaceInitFunc) __GstNavigation__interface_init,
    NULL,
    NULL
};


/* ----------- GstPropertyProbe ----------- */

#line 296 "..\\..\\..\\Source\\gst-python\\gst\\interfaces.override"
static PyObject *
_wrap_gst_property_probe_get_properties (PyGObject *self)
{
    const GList *l, *list;
    PyObject *py_list;

    g_return_val_if_fail (GST_IS_PROPERTY_PROBE (self->obj), PyList_New(0));

    pyg_begin_allow_threads;
    list = gst_property_probe_get_properties (GST_PROPERTY_PROBE (self->obj));
    pyg_end_allow_threads;

    py_list = PyList_New(0);
    for (l = list; l; l = l->next) {
        GParamSpec *spec = (GParamSpec*)l->data;
        PyObject *py_gspec = pyg_param_spec_new((GParamSpec*)spec);
        PyList_Append(py_list, py_gspec);
        Py_DECREF(py_gspec);
    }

    return py_list;
}
#line 1812 "..\\..\\..\\Source\\gst-python\\gst\\interfaces.c"


#line 273 "..\\..\\..\\Source\\gst-python\\gst\\interfaces.override"
static PyObject *
_wrap_gst_property_probe_get_property (PyGObject *self, PyObject *args)
{
    const char *name;
    const GParamSpec *spec;
    
    if (!PyArg_ParseTuple(args, "s:ProbeProperty.get_property", &name))
	return NULL;
    
    pyg_begin_allow_threads;
    spec = gst_property_probe_get_property (GST_PROPERTY_PROBE (self->obj),
					    name);
    pyg_end_allow_threads;

    if (!spec) {
	PyErr_Format(PyExc_ValueError, "unknown property: %s", name);
	return NULL;
    }

    return pyg_param_spec_new((GParamSpec*)spec);
}
#line 1837 "..\\..\\..\\Source\\gst-python\\gst\\interfaces.c"


static PyObject *
_wrap_gst_property_probe_probe_property_name(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", NULL };
    char *name;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:GstPropertyProbe.probe_property_name", kwlist, &name))
        return NULL;
    pyg_begin_allow_threads;
    gst_property_probe_probe_property_name(GST_PROPERTY_PROBE(self->obj), name);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_property_probe_needs_probe_name(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", NULL };
    char *name;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:GstPropertyProbe.needs_probe_name", kwlist, &name))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_property_probe_needs_probe_name(GST_PROPERTY_PROBE(self->obj), name);
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

#line 320 "..\\..\\..\\Source\\gst-python\\gst\\interfaces.override"
static PyObject *
_wrap_gst_property_probe_get_values_name (PyGObject *self, PyObject *args)
{
    const char *name;
    GValueArray *array;
    PyObject *py_list;
    int i;
    
    g_return_val_if_fail (GST_IS_PROPERTY_PROBE (self->obj), PyList_New(0));

    if (!PyArg_ParseTuple(args, "s:ProbeProperty.get_values_name", &name))
	return NULL;

    pyg_begin_allow_threads;
    array = gst_property_probe_get_values_name (GST_PROPERTY_PROBE (self->obj),
						name);
    pyg_end_allow_threads;

    py_list = PyList_New(0);

    if (array) {
	for (i = 0; i < array->n_values; i++) {
	    GValue *value = g_value_array_get_nth(array, i);
	    PyObject *py_value = pyg_value_as_pyobject(value, TRUE);
	    PyList_Append(py_list, py_value);
	    Py_DECREF(py_value);
	    
	}
	g_value_array_free(array);
    }
    
    return py_list;
}
#line 1905 "..\\..\\..\\Source\\gst-python\\gst\\interfaces.c"


static const PyMethodDef _PyGstPropertyProbe_methods[] = {
    { "probe_get_properties", (PyCFunction)_wrap_gst_property_probe_get_properties, METH_NOARGS,
      NULL },
    { "probe_get_property", (PyCFunction)_wrap_gst_property_probe_get_property, METH_VARARGS,
      NULL },
    { "probe_property_name", (PyCFunction)_wrap_gst_property_probe_probe_property_name, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "needs_probe_name", (PyCFunction)_wrap_gst_property_probe_needs_probe_name, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "probe_get_values_name", (PyCFunction)_wrap_gst_property_probe_get_values_name, METH_VARARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject PyGstPropertyProbe_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.interfaces.PropertyProbe",                   /* tp_name */
    sizeof(PyObject),          /* tp_basicsize */
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
    0,             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGstPropertyProbe_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    0,                 /* tp_dictoffset */
    (initproc)0,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GstTuner ----------- */

#line 65 "..\\..\\..\\Source\\gst-python\\gst\\interfaces.override"
static PyObject *
_wrap_gst_tuner_list_channels(PyGObject *self)
{
    const GList *l, *list;
    PyObject *py_list;
	
    g_return_val_if_fail (GST_IS_TUNER (self->obj), PyList_New(0));

    pyg_begin_allow_threads;
    list = gst_tuner_list_channels(GST_TUNER(self->obj));
    pyg_end_allow_threads;
    
    py_list = PyList_New(0);
    for (l = list; l; l = l->next) {
	GstTunerChannel *channel = (GstTunerChannel*)l->data;
	PyObject *py_channel = pygobject_new(G_OBJECT(channel));
	PyList_Append(py_list, py_channel);
	Py_DECREF(py_channel);
    }
		
    return py_list;
}
#line 1994 "..\\..\\..\\Source\\gst-python\\gst\\interfaces.c"


static PyObject *
_wrap_gst_tuner_set_channel(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "channel", NULL };
    PyGObject *channel;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstTuner.set_channel", kwlist, &PyGstTunerChannel_Type, &channel))
        return NULL;
    pyg_begin_allow_threads;
    gst_tuner_set_channel(GST_TUNER(self->obj), GST_TUNER_CHANNEL(channel->obj));
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_tuner_get_channel(PyGObject *self)
{
    GstTunerChannel *ret;

    pyg_begin_allow_threads;
    ret = gst_tuner_get_channel(GST_TUNER(self->obj));
    pyg_end_allow_threads;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

#line 89 "..\\..\\..\\Source\\gst-python\\gst\\interfaces.override"
static PyObject *
_wrap_gst_tuner_list_norms(PyGObject *self)
{
    const GList *l, *list;
    PyObject *py_list;
	
    g_return_val_if_fail (GST_IS_TUNER (self->obj), PyList_New(0));

    pyg_begin_allow_threads;
    list = gst_tuner_list_norms(GST_TUNER(self->obj));
    pyg_end_allow_threads;
    
    py_list = PyList_New(0);
    for (l = list; l; l = l->next) {
	GstTunerNorm *norm = (GstTunerNorm*)l->data;
	PyObject *py_norm = pygobject_new(G_OBJECT(norm));
	PyList_Append(py_list, py_norm);
	Py_DECREF(py_norm);
    }
		
    return py_list;
}
#line 2047 "..\\..\\..\\Source\\gst-python\\gst\\interfaces.c"


static PyObject *
_wrap_gst_tuner_set_norm(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "channel", NULL };
    PyGObject *channel;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstTuner.set_norm", kwlist, &PyGstTunerNorm_Type, &channel))
        return NULL;
    pyg_begin_allow_threads;
    gst_tuner_set_norm(GST_TUNER(self->obj), GST_TUNER_NORM(channel->obj));
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_tuner_get_norm(PyGObject *self)
{
    GstTunerNorm *ret;

    pyg_begin_allow_threads;
    ret = gst_tuner_get_norm(GST_TUNER(self->obj));
    pyg_end_allow_threads;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gst_tuner_set_frequency(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "channel", "frequency", NULL };
    PyGObject *channel;
    unsigned long frequency;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!k:GstTuner.set_frequency", kwlist, &PyGstTunerChannel_Type, &channel, &frequency))
        return NULL;
    pyg_begin_allow_threads;
    gst_tuner_set_frequency(GST_TUNER(self->obj), GST_TUNER_CHANNEL(channel->obj), frequency);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_tuner_get_frequency(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "channel", NULL };
    PyGObject *channel;
    gulong ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstTuner.get_frequency", kwlist, &PyGstTunerChannel_Type, &channel))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_tuner_get_frequency(GST_TUNER(self->obj), GST_TUNER_CHANNEL(channel->obj));
    pyg_end_allow_threads;
    return PyLong_FromUnsignedLong(ret);

}

static PyObject *
_wrap_gst_tuner_signal_strength(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "channel", NULL };
    PyGObject *channel;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstTuner.signal_strength", kwlist, &PyGstTunerChannel_Type, &channel))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_tuner_signal_strength(GST_TUNER(self->obj), GST_TUNER_CHANNEL(channel->obj));
    pyg_end_allow_threads;
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gst_tuner_find_norm_by_name(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "norm", NULL };
    char *norm;
    GstTunerNorm *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:GstTuner.find_norm_by_name", kwlist, &norm))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_tuner_find_norm_by_name(GST_TUNER(self->obj), norm);
    pyg_end_allow_threads;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gst_tuner_find_channel_by_name(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "channel", NULL };
    char *channel;
    GstTunerChannel *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:GstTuner.find_channel_by_name", kwlist, &channel))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_tuner_find_channel_by_name(GST_TUNER(self->obj), channel);
    pyg_end_allow_threads;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gst_tuner_channel_changed(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "channel", NULL };
    PyGObject *channel;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstTuner.channel_changed", kwlist, &PyGstTunerChannel_Type, &channel))
        return NULL;
    pyg_begin_allow_threads;
    gst_tuner_channel_changed(GST_TUNER(self->obj), GST_TUNER_CHANNEL(channel->obj));
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_tuner_norm_changed(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "norm", NULL };
    PyGObject *norm;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstTuner.norm_changed", kwlist, &PyGstTunerNorm_Type, &norm))
        return NULL;
    pyg_begin_allow_threads;
    gst_tuner_norm_changed(GST_TUNER(self->obj), GST_TUNER_NORM(norm->obj));
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_tuner_frequency_changed(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "channel", "frequency", NULL };
    PyGObject *channel;
    unsigned long frequency;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!k:GstTuner.frequency_changed", kwlist, &PyGstTunerChannel_Type, &channel, &frequency))
        return NULL;
    pyg_begin_allow_threads;
    gst_tuner_frequency_changed(GST_TUNER(self->obj), GST_TUNER_CHANNEL(channel->obj), frequency);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_tuner_signal_changed(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "channel", "signal", NULL };
    PyGObject *channel;
    int signal;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:GstTuner.signal_changed", kwlist, &PyGstTunerChannel_Type, &channel, &signal))
        return NULL;
    pyg_begin_allow_threads;
    gst_tuner_signal_changed(GST_TUNER(self->obj), GST_TUNER_CHANNEL(channel->obj), signal);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GstTuner__do_set_channel(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GstTunerClass *iface;
    static char *kwlist[] = { "self", "channel", NULL };
    PyGObject *self, *channel;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!:GstTuner.set_channel", kwlist, &PyGstTuner_Type, &self, &PyGstTunerChannel_Type, &channel))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GST_TYPE_TUNER);
    if (iface->set_channel)
        iface->set_channel(GST_TUNER(self->obj), GST_TUNER_CHANNEL(channel->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GstTuner.set_channel not implemented");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GstTuner__do_get_channel(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GstTunerClass *iface;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    GstTunerChannel *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstTuner.get_channel", kwlist, &PyGstTuner_Type, &self))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GST_TYPE_TUNER);
    if (iface->get_channel)
        ret = iface->get_channel(GST_TUNER(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GstTuner.get_channel not implemented");
        return NULL;
    }
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_GstTuner__do_set_norm(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GstTunerClass *iface;
    static char *kwlist[] = { "self", "channel", NULL };
    PyGObject *self, *channel;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!:GstTuner.set_norm", kwlist, &PyGstTuner_Type, &self, &PyGstTunerNorm_Type, &channel))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GST_TYPE_TUNER);
    if (iface->set_norm)
        iface->set_norm(GST_TUNER(self->obj), GST_TUNER_NORM(channel->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GstTuner.set_norm not implemented");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GstTuner__do_get_norm(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GstTunerClass *iface;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    GstTunerNorm *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstTuner.get_norm", kwlist, &PyGstTuner_Type, &self))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GST_TYPE_TUNER);
    if (iface->get_norm)
        ret = iface->get_norm(GST_TUNER(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GstTuner.get_norm not implemented");
        return NULL;
    }
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_GstTuner__do_set_frequency(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GstTunerClass *iface;
    static char *kwlist[] = { "self", "channel", "frequency", NULL };
    PyGObject *self, *channel;
    unsigned long frequency;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!k:GstTuner.set_frequency", kwlist, &PyGstTuner_Type, &self, &PyGstTunerChannel_Type, &channel, &frequency))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GST_TYPE_TUNER);
    if (iface->set_frequency)
        iface->set_frequency(GST_TUNER(self->obj), GST_TUNER_CHANNEL(channel->obj), frequency);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GstTuner.set_frequency not implemented");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GstTuner__do_get_frequency(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GstTunerClass *iface;
    static char *kwlist[] = { "self", "channel", NULL };
    PyGObject *self, *channel;
    gulong ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!:GstTuner.get_frequency", kwlist, &PyGstTuner_Type, &self, &PyGstTunerChannel_Type, &channel))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GST_TYPE_TUNER);
    if (iface->get_frequency)
        ret = iface->get_frequency(GST_TUNER(self->obj), GST_TUNER_CHANNEL(channel->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GstTuner.get_frequency not implemented");
        return NULL;
    }
    return PyLong_FromUnsignedLong(ret);

}

static PyObject *
_wrap_GstTuner__do_signal_strength(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GstTunerClass *iface;
    static char *kwlist[] = { "self", "channel", NULL };
    PyGObject *self, *channel;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!:GstTuner.signal_strength", kwlist, &PyGstTuner_Type, &self, &PyGstTunerChannel_Type, &channel))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GST_TYPE_TUNER);
    if (iface->signal_strength)
        ret = iface->signal_strength(GST_TUNER(self->obj), GST_TUNER_CHANNEL(channel->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GstTuner.signal_strength not implemented");
        return NULL;
    }
    return PyInt_FromLong(ret);
}

static const PyMethodDef _PyGstTuner_methods[] = {
    { "list_channels", (PyCFunction)_wrap_gst_tuner_list_channels, METH_NOARGS,
      NULL },
    { "set_channel", (PyCFunction)_wrap_gst_tuner_set_channel, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_channel", (PyCFunction)_wrap_gst_tuner_get_channel, METH_NOARGS,
      NULL },
    { "list_norms", (PyCFunction)_wrap_gst_tuner_list_norms, METH_NOARGS,
      NULL },
    { "set_norm", (PyCFunction)_wrap_gst_tuner_set_norm, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_norm", (PyCFunction)_wrap_gst_tuner_get_norm, METH_NOARGS,
      NULL },
    { "set_frequency", (PyCFunction)_wrap_gst_tuner_set_frequency, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_frequency", (PyCFunction)_wrap_gst_tuner_get_frequency, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "signal_strength", (PyCFunction)_wrap_gst_tuner_signal_strength, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "find_norm_by_name", (PyCFunction)_wrap_gst_tuner_find_norm_by_name, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "find_channel_by_name", (PyCFunction)_wrap_gst_tuner_find_channel_by_name, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "channel_changed", (PyCFunction)_wrap_gst_tuner_channel_changed, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "norm_changed", (PyCFunction)_wrap_gst_tuner_norm_changed, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "frequency_changed", (PyCFunction)_wrap_gst_tuner_frequency_changed, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "signal_changed", (PyCFunction)_wrap_gst_tuner_signal_changed, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "do_set_channel", (PyCFunction)_wrap_GstTuner__do_set_channel, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_channel", (PyCFunction)_wrap_GstTuner__do_get_channel, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_set_norm", (PyCFunction)_wrap_GstTuner__do_set_norm, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_norm", (PyCFunction)_wrap_GstTuner__do_get_norm, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_set_frequency", (PyCFunction)_wrap_GstTuner__do_set_frequency, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_frequency", (PyCFunction)_wrap_GstTuner__do_get_frequency, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_signal_strength", (PyCFunction)_wrap_GstTuner__do_signal_strength, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject PyGstTuner_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.interfaces.Tuner",                   /* tp_name */
    sizeof(PyObject),          /* tp_basicsize */
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
    0,             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGstTuner_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    0,                 /* tp_dictoffset */
    (initproc)0,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};

static void
_wrap_GstTuner__proxy_do_set_channel(GstTuner *self, GstTunerChannel*channel)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_channel = NULL;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return;
    }
    if (channel)
        py_channel = pygobject_new((GObject *) channel);
    else {
        Py_INCREF(Py_None);
        py_channel = Py_None;
    }
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_channel);
    
    py_method = PyObject_GetAttrString(py_self, "do_set_channel");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_method);
        Py_DECREF(py_args);
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
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    Py_DECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
}
static GstTunerChannel*
_wrap_GstTuner__proxy_do_get_channel(GstTuner *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    GstTunerChannel* retval;
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_get_channel");
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
    retval = (GstTunerChannel*) pygobject_get(py_retval);
    g_object_ref((GObject *) retval);
    
    
    Py_DECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static void
_wrap_GstTuner__proxy_do_set_norm(GstTuner *self, GstTunerNorm*channel)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_channel = NULL;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return;
    }
    if (channel)
        py_channel = pygobject_new((GObject *) channel);
    else {
        Py_INCREF(Py_None);
        py_channel = Py_None;
    }
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_channel);
    
    py_method = PyObject_GetAttrString(py_self, "do_set_norm");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_method);
        Py_DECREF(py_args);
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
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    Py_DECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
}
static GstTunerNorm*
_wrap_GstTuner__proxy_do_get_norm(GstTuner *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    GstTunerNorm* retval;
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_get_norm");
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
    retval = (GstTunerNorm*) pygobject_get(py_retval);
    g_object_ref((GObject *) retval);
    
    
    Py_DECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static void
_wrap_GstTuner__proxy_do_set_frequency(GstTuner *self, GstTunerChannel*channel, gulong frequency)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_channel = NULL;
    PyObject *py_frequency;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return;
    }
    if (channel)
        py_channel = pygobject_new((GObject *) channel);
    else {
        Py_INCREF(Py_None);
        py_channel = Py_None;
    }
    py_frequency = PyLong_FromUnsignedLong(frequency);
    
    py_args = PyTuple_New(2);
    PyTuple_SET_ITEM(py_args, 0, py_channel);
    PyTuple_SET_ITEM(py_args, 1, py_frequency);
    
    py_method = PyObject_GetAttrString(py_self, "do_set_frequency");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_method);
        Py_DECREF(py_args);
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
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    Py_DECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
}
static gulong
_wrap_GstTuner__proxy_do_get_frequency(GstTuner *self, GstTunerChannel*channel)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_channel = NULL;
    gulong retval;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    if (channel)
        py_channel = pygobject_new((GObject *) channel);
    else {
        Py_INCREF(Py_None);
        py_channel = Py_None;
    }
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_channel);
    
    py_method = PyObject_GetAttrString(py_self, "do_get_frequency");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    if (!PyLong_Check(py_retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        PyErr_SetString(PyExc_TypeError, "retval should be an long");
        Py_DECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    retval = PyLong_AsUnsignedLongMask(py_retval);
    
    
    Py_DECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gint
_wrap_GstTuner__proxy_do_signal_strength(GstTuner *self, GstTunerChannel*channel)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_channel = NULL;
    gint retval;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    if (channel)
        py_channel = pygobject_new((GObject *) channel);
    else {
        Py_INCREF(Py_None);
        py_channel = Py_None;
    }
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_channel);
    
    py_method = PyObject_GetAttrString(py_self, "do_signal_strength");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_method);
        Py_DECREF(py_args);
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
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    
    
    Py_DECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}

static void
__GstTuner__interface_init(GstTunerClass *iface, PyTypeObject *pytype)
{
    GstTunerClass *parent_iface = g_type_interface_peek_parent(iface);
    PyObject *py_method;

    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_set_channel") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->set_channel = _wrap_GstTuner__proxy_do_set_channel;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->set_channel = parent_iface->set_channel;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_get_channel") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->get_channel = _wrap_GstTuner__proxy_do_get_channel;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->get_channel = parent_iface->get_channel;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_set_norm") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->set_norm = _wrap_GstTuner__proxy_do_set_norm;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->set_norm = parent_iface->set_norm;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_get_norm") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->get_norm = _wrap_GstTuner__proxy_do_get_norm;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->get_norm = parent_iface->get_norm;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_set_frequency") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->set_frequency = _wrap_GstTuner__proxy_do_set_frequency;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->set_frequency = parent_iface->set_frequency;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_get_frequency") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->get_frequency = _wrap_GstTuner__proxy_do_get_frequency;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->get_frequency = parent_iface->get_frequency;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_signal_strength") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->signal_strength = _wrap_GstTuner__proxy_do_signal_strength;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->signal_strength = parent_iface->signal_strength;
        }
    Py_XDECREF(py_method);
    }
}


static const GInterfaceInfo __GstTuner__iinfo = {
    (GInterfaceInitFunc) __GstTuner__interface_init,
    NULL,
    NULL
};


/* ----------- GstVideoOrientation ----------- */

#line 437 "..\\..\\..\\Source\\gst-python\\gst\\interfaces.override"
static PyObject *
_wrap_gst_video_orientation_get_hflip (PyGObject * self)
{
    gboolean flip, res;

    res = gst_video_orientation_get_hflip (GST_VIDEO_ORIENTATION (self->obj), &flip);

    return Py_BuildValue("(OO)",
			 PyBool_FromLong(res),
			 PyBool_FromLong(flip));
}
#line 3008 "..\\..\\..\\Source\\gst-python\\gst\\interfaces.c"


#line 450 "..\\..\\..\\Source\\gst-python\\gst\\interfaces.override"
static PyObject *
_wrap_gst_video_orientation_get_vflip (PyGObject * self)
{
    gboolean flip, res;

    res = gst_video_orientation_get_vflip (GST_VIDEO_ORIENTATION (self->obj), &flip);

    return Py_BuildValue("(OO)",
			 PyBool_FromLong(res),
			 PyBool_FromLong(flip));
}
#line 3023 "..\\..\\..\\Source\\gst-python\\gst\\interfaces.c"


#line 463 "..\\..\\..\\Source\\gst-python\\gst\\interfaces.override"
static PyObject *
_wrap_gst_video_orientation_get_hcenter (PyGObject * self)
{
    gboolean res;
    gint center;

    res = gst_video_orientation_get_hcenter (GST_VIDEO_ORIENTATION (self->obj), &center);

    return Py_BuildValue("(Oi)",
			 PyBool_FromLong(res),
			 center);
}
#line 3039 "..\\..\\..\\Source\\gst-python\\gst\\interfaces.c"


#line 477 "..\\..\\..\\Source\\gst-python\\gst\\interfaces.override"
static PyObject *
_wrap_gst_video_orientation_get_vcenter (PyGObject * self)
{
    gboolean res;
    gint center;

    res = gst_video_orientation_get_vcenter (GST_VIDEO_ORIENTATION (self->obj), &center);

    return Py_BuildValue("(Oi)",
			 PyBool_FromLong(res),
			 center);
}
#line 3055 "..\\..\\..\\Source\\gst-python\\gst\\interfaces.c"


static PyObject *
_wrap_gst_video_orientation_set_hflip(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "flip", NULL };
    int flip, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GstVideoOrientation.set_hflip", kwlist, &flip))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_video_orientation_set_hflip(GST_VIDEO_ORIENTATION(self->obj), flip);
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_video_orientation_set_vflip(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "flip", NULL };
    int flip, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GstVideoOrientation.set_vflip", kwlist, &flip))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_video_orientation_set_vflip(GST_VIDEO_ORIENTATION(self->obj), flip);
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_video_orientation_set_hcenter(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "center", NULL };
    int center, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GstVideoOrientation.set_hcenter", kwlist, &center))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_video_orientation_set_hcenter(GST_VIDEO_ORIENTATION(self->obj), center);
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_video_orientation_set_vcenter(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "center", NULL };
    int center, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GstVideoOrientation.set_vcenter", kwlist, &center))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_video_orientation_set_vcenter(GST_VIDEO_ORIENTATION(self->obj), center);
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static const PyMethodDef _PyGstVideoOrientation_methods[] = {
    { "get_hflip", (PyCFunction)_wrap_gst_video_orientation_get_hflip, METH_NOARGS,
      NULL },
    { "get_vflip", (PyCFunction)_wrap_gst_video_orientation_get_vflip, METH_NOARGS,
      NULL },
    { "get_hcenter", (PyCFunction)_wrap_gst_video_orientation_get_hcenter, METH_NOARGS,
      NULL },
    { "get_vcenter", (PyCFunction)_wrap_gst_video_orientation_get_vcenter, METH_NOARGS,
      NULL },
    { "set_hflip", (PyCFunction)_wrap_gst_video_orientation_set_hflip, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_vflip", (PyCFunction)_wrap_gst_video_orientation_set_vflip, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_hcenter", (PyCFunction)_wrap_gst_video_orientation_set_hcenter, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_vcenter", (PyCFunction)_wrap_gst_video_orientation_set_vcenter, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject PyGstVideoOrientation_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.interfaces.VideoOrientation",                   /* tp_name */
    sizeof(PyObject),          /* tp_basicsize */
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
    0,             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGstVideoOrientation_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    0,                 /* tp_dictoffset */
    (initproc)0,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- functions ----------- */

#line 355 "..\\..\\..\\Source\\gst-python\\gst\\interfaces.override"
static PyObject *
_wrap_gst_mixer_message_parse_mute_toggled (PyGstMiniObject * self)
{
    GstMixerTrack *track;
    gboolean mute;

    if (GST_MESSAGE(self->obj)->type != GST_MIXER_MESSAGE_MUTE_TOGGLED) {
	PyErr_SetString(PyExc_TypeError, "Message is not a mute-toggled message");
	return NULL;
    }
    gst_mixer_message_parse_mute_toggled (GST_MESSAGE(self->obj), &track, &mute);

    return Py_BuildValue("(OO)",
			 pygobject_new(G_OBJECT (track)),
			 PyBool_FromLong(mute));
}
#line 3204 "..\\..\\..\\Source\\gst-python\\gst\\interfaces.c"


#line 373 "..\\..\\..\\Source\\gst-python\\gst\\interfaces.override"
static PyObject *
_wrap_gst_mixer_message_parse_record_toggled (PyGstMiniObject * self)
{
    GstMixerTrack *track;
    gboolean record;

    if (GST_MESSAGE(self->obj)->type != GST_MIXER_MESSAGE_RECORD_TOGGLED) {
	PyErr_SetString(PyExc_TypeError, "Message is not a record-toggled message");
	return NULL;
    }
    gst_mixer_message_parse_record_toggled (GST_MESSAGE(self->obj), &track, &record);

    return Py_BuildValue("(OO)",
			 pygobject_new(G_OBJECT (track)),
			 PyBool_FromLong(record));
}
#line 3224 "..\\..\\..\\Source\\gst-python\\gst\\interfaces.c"


#line 391 "..\\..\\..\\Source\\gst-python\\gst\\interfaces.override"
static PyObject *
_wrap_gst_mixer_message_parse_volume_changed (PyGstMiniObject * self)
{
    GstMixerTrack *track;
    gint *volumes;
    gint num_channels;
    PyObject *pvolumes;
    int i;

    if (GST_MESSAGE(self->obj)->type != GST_MIXER_MESSAGE_VOLUME_CHANGED) {
	PyErr_SetString(PyExc_TypeError, "Message is not a volume-changed message");
	return NULL;
    }
    gst_mixer_message_parse_volume_changed (GST_MESSAGE(self->obj), &track, &volumes, &num_channels);

    pvolumes = PyList_New (num_channels);

    for (i = 0; i < num_channels; ++i) {
	PyList_SET_ITEM (pvolumes, i, PyInt_FromLong (volumes[i]));
    }
    g_free (volumes);

    return Py_BuildValue("(OOi)",
			 pygobject_new(G_OBJECT (track)),
			 pvolumes, num_channels);
}
#line 3254 "..\\..\\..\\Source\\gst-python\\gst\\interfaces.c"


#line 419 "..\\..\\..\\Source\\gst-python\\gst\\interfaces.override"
static PyObject *
_wrap_gst_mixer_message_parse_option_changed (PyGstMiniObject * self)
{
    GstMixerOptions *options;
    const gchar *value = NULL;

    if (GST_MESSAGE(self->obj)->type != GST_MIXER_MESSAGE_OPTION_CHANGED) {
	PyErr_SetString(PyExc_TypeError, "Message is not a option-changed message");
	return NULL;
    }
    gst_mixer_message_parse_option_changed (GST_MESSAGE(self->obj), &options, &value);

    return Py_BuildValue("(Os)",
			 pygobject_new(G_OBJECT (options)),
			 value);
}
#line 3274 "..\\..\\..\\Source\\gst-python\\gst\\interfaces.c"


static PyObject *
_wrap_gst_navigation_query_new_commands(PyObject *self)
{
    PyObject *py_ret;
    GstQuery *ret;

    pyg_begin_allow_threads;
    ret = gst_navigation_query_new_commands();
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_navigation_query_new_angles(PyObject *self)
{
    PyObject *py_ret;
    GstQuery *ret;

    pyg_begin_allow_threads;
    ret = gst_navigation_query_new_angles();
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_navigation_query_set_angles(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "query", "cur_angle", "n_angles", NULL };
    PyObject *py_cur_angle = NULL, *py_n_angles = NULL;
    guint cur_angle = 0, n_angles = 0;
    PyGstMiniObject *query;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!OO:navigation_query_set_angles", kwlist, &PyGstQuery_Type, &query, &py_cur_angle, &py_n_angles))
        return NULL;
    if (py_cur_angle) {
        if (PyLong_Check(py_cur_angle))
            cur_angle = PyLong_AsUnsignedLong(py_cur_angle);
        else if (PyInt_Check(py_cur_angle))
            cur_angle = PyInt_AsLong(py_cur_angle);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'cur_angle' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    if (py_n_angles) {
        if (PyLong_Check(py_n_angles))
            n_angles = PyLong_AsUnsignedLong(py_n_angles);
        else if (PyInt_Check(py_n_angles))
            n_angles = PyInt_AsLong(py_n_angles);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'n_angles' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    pyg_begin_allow_threads;
    gst_navigation_query_set_angles(GST_QUERY(query->obj), cur_angle, n_angles);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_navigation_message_new_mouse_over(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "src", "active", NULL };
    PyGObject *src;
    int active;
    PyObject *py_ret;
    GstMessage *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:navigation_message_new_mouse_over", kwlist, &PyGstObject_Type, &src, &active))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_navigation_message_new_mouse_over(GST_OBJECT(src->obj), active);
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_navigation_message_new_commands_changed(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "src", NULL };
    PyGObject *src;
    GstMessage *ret;
    PyObject *py_ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:navigation_message_new_commands_changed", kwlist, &PyGstObject_Type, &src))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_navigation_message_new_commands_changed(GST_OBJECT(src->obj));
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_navigation_message_new_angles_changed(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "src", "cur_angle", "n_angles", NULL };
    PyGObject *src;
    PyObject *py_cur_angle = NULL, *py_n_angles = NULL, *py_ret;
    guint cur_angle = 0, n_angles = 0;
    GstMessage *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!OO:navigation_message_new_angles_changed", kwlist, &PyGstObject_Type, &src, &py_cur_angle, &py_n_angles))
        return NULL;
    if (py_cur_angle) {
        if (PyLong_Check(py_cur_angle))
            cur_angle = PyLong_AsUnsignedLong(py_cur_angle);
        else if (PyInt_Check(py_cur_angle))
            cur_angle = PyInt_AsLong(py_cur_angle);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'cur_angle' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    if (py_n_angles) {
        if (PyLong_Check(py_n_angles))
            n_angles = PyLong_AsUnsignedLong(py_n_angles);
        else if (PyInt_Check(py_n_angles))
            n_angles = PyInt_AsLong(py_n_angles);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'n_angles' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    pyg_begin_allow_threads;
    ret = gst_navigation_message_new_angles_changed(GST_OBJECT(src->obj), cur_angle, n_angles);
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_stream_volume_convert_volume(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "from_", "to", "val", NULL };
    GstStreamVolumeFormat from, to;
    PyObject *py_from = NULL, *py_to = NULL;
    double val, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"OOd:stream_volume_convert_volume", kwlist, &py_from, &py_to, &val))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_STREAM_VOLUME_FORMAT, py_from, (gint *)&from))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_STREAM_VOLUME_FORMAT, py_to, (gint *)&to))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_stream_volume_convert_volume(from, to, val);
    pyg_end_allow_threads;
    return PyFloat_FromDouble(ret);
}

const PyMethodDef pyinterfaces_functions[] = {
    { "mixer_message_parse_mute_toggled", (PyCFunction)_wrap_gst_mixer_message_parse_mute_toggled, METH_NOARGS,
      NULL },
    { "mixer_message_parse_record_toggled", (PyCFunction)_wrap_gst_mixer_message_parse_record_toggled, METH_NOARGS,
      NULL },
    { "mixer_message_parse_volume_changed", (PyCFunction)_wrap_gst_mixer_message_parse_volume_changed, METH_NOARGS,
      NULL },
    { "mixer_message_parse_option_changed", (PyCFunction)_wrap_gst_mixer_message_parse_option_changed, METH_NOARGS,
      NULL },
    { "navigation_query_new_commands", (PyCFunction)_wrap_gst_navigation_query_new_commands, METH_NOARGS,
      NULL },
    { "navigation_query_new_angles", (PyCFunction)_wrap_gst_navigation_query_new_angles, METH_NOARGS,
      NULL },
    { "navigation_query_set_angles", (PyCFunction)_wrap_gst_navigation_query_set_angles, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "navigation_message_new_mouse_over", (PyCFunction)_wrap_gst_navigation_message_new_mouse_over, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "navigation_message_new_commands_changed", (PyCFunction)_wrap_gst_navigation_message_new_commands_changed, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "navigation_message_new_angles_changed", (PyCFunction)_wrap_gst_navigation_message_new_angles_changed, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "stream_volume_convert_volume", (PyCFunction)_wrap_gst_stream_volume_convert_volume, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};


/* ----------- enums and flags ----------- */

void
pyinterfaces_add_constants(PyObject *module, const gchar *strip_prefix)
{
  pyg_enum_add(module, "ColorBalanceType", strip_prefix, GST_TYPE_COLOR_BALANCE_TYPE);
  pyg_enum_add(module, "MixerType", strip_prefix, GST_TYPE_MIXER_TYPE);
  pyg_enum_add(module, "MixerMessageType", strip_prefix, GST_TYPE_MIXER_MESSAGE_TYPE);
  pyg_flags_add(module, "MixerFlags", strip_prefix, GST_TYPE_MIXER_FLAGS);
  pyg_flags_add(module, "MixerTrackFlags", strip_prefix, GST_TYPE_MIXER_TRACK_FLAGS);
  pyg_enum_add(module, "NavigationCommand", strip_prefix, GST_TYPE_NAVIGATION_COMMAND);
  pyg_enum_add(module, "NavigationQueryType", strip_prefix, GST_TYPE_NAVIGATION_QUERY_TYPE);
  pyg_enum_add(module, "NavigationMessageType", strip_prefix, GST_TYPE_NAVIGATION_MESSAGE_TYPE);
  pyg_enum_add(module, "NavigationEventType", strip_prefix, GST_TYPE_NAVIGATION_EVENT_TYPE);
  pyg_enum_add(module, "StreamVolumeFormat", strip_prefix, GST_TYPE_STREAM_VOLUME_FORMAT);
  pyg_flags_add(module, "TunerChannelFlags", strip_prefix, GST_TYPE_TUNER_CHANNEL_FLAGS);

  if (PyErr_Occurred())
    PyErr_Print();
}

/* initialise stuff extension classes */
void
pyinterfaces_register_classes(PyObject *d)
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
        _PyGstQuery_Type = (PyTypeObject *)PyDict_GetItemString(moddict, "Query");
        if (_PyGstQuery_Type == NULL) {
            PyErr_SetString(PyExc_ImportError,
                "cannot import name Query from gst");
            return;
        }
    } else {
        PyErr_SetString(PyExc_ImportError,
            "could not import gst");
        return;
    }


#line 3545 "..\\..\\..\\Source\\gst-python\\gst\\interfaces.c"
    pyg_register_interface(d, "XOverlay", GST_TYPE_X_OVERLAY, &PyGstXOverlay_Type);
    pyg_register_interface(d, "ColorBalance", GST_TYPE_COLOR_BALANCE, &PyGstColorBalance_Type);
    pyg_register_interface_info(GST_TYPE_COLOR_BALANCE, &__GstColorBalance__iinfo);
    pyg_register_interface(d, "Mixer", GST_TYPE_MIXER, &PyGstMixer_Type);
    pyg_register_interface_info(GST_TYPE_MIXER, &__GstMixer__iinfo);
    pyg_register_interface(d, "Navigation", GST_TYPE_NAVIGATION, &PyGstNavigation_Type);
    pyg_register_interface_info(GST_TYPE_NAVIGATION, &__GstNavigation__iinfo);
    pyg_register_interface(d, "PropertyProbe", GST_TYPE_PROPERTY_PROBE, &PyGstPropertyProbe_Type);
    pyg_register_interface(d, "Tuner", GST_TYPE_TUNER, &PyGstTuner_Type);
    pyg_register_interface_info(GST_TYPE_TUNER, &__GstTuner__iinfo);
    pyg_register_interface(d, "VideoOrientation", GST_TYPE_VIDEO_ORIENTATION, &PyGstVideoOrientation_Type);
    pygobject_register_class(d, "GstColorBalanceChannel", GST_TYPE_COLOR_BALANCE_CHANNEL, &PyGstColorBalanceChannel_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_set_object_has_new_constructor(GST_TYPE_COLOR_BALANCE_CHANNEL);
    pygobject_register_class(d, "GstMixerTrack", GST_TYPE_MIXER_TRACK, &PyGstMixerTrack_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_set_object_has_new_constructor(GST_TYPE_MIXER_TRACK);
    pygobject_register_class(d, "GstMixerOptions", GST_TYPE_MIXER_OPTIONS, &PyGstMixerOptions_Type, Py_BuildValue("(O)", &PyGstMixerTrack_Type));
    pyg_set_object_has_new_constructor(GST_TYPE_MIXER_OPTIONS);
    pygobject_register_class(d, "GstTunerChannel", GST_TYPE_TUNER_CHANNEL, &PyGstTunerChannel_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_set_object_has_new_constructor(GST_TYPE_TUNER_CHANNEL);
    pygobject_register_class(d, "GstTunerNorm", GST_TYPE_TUNER_NORM, &PyGstTunerNorm_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_set_object_has_new_constructor(GST_TYPE_TUNER_NORM);
}
