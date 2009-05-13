/* -- THIS FILE IS GENERATED - DO NOT EDIT *//* -*- Mode: C; c-basic-offset: 4 -*- */

#include <Python.h>



#line 22 "..\\..\\Source\\gst-python\\gst\\pbutils.override"

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define NO_IMPORT_PYGOBJECT
#include "common.h"

#include <gst/gst.h>

#include <gst/pbutils/pbutils.h>
#include "pygstminiobject.h"
GST_DEBUG_CATEGORY_EXTERN (pygst_debug);
#define GST_CAT_DEFAULT pygst_debug

/* Boonky define that allows for backwards compatibility with Python 2.4 */
#if PY_VERSION_HEX < 0x02050000
#define Py_ssize_t int
#endif

#ifdef HAVE_PLUGINS_INSTALL
static void
install_plugins_result_handler(GstInstallPluginsReturn result, gpointer user_data)
{
    PyGILState_STATE state;
    PyObject *callback, *args;
    PyObject *py_user_data;
    PyObject *py_result;
    PyObject *ret;
    gint i, len;
    
    if (user_data == NULL)
	return;

    state = pyg_gil_state_ensure();

    py_user_data = (PyObject*) user_data;
    py_result = pyg_enum_from_gtype(GST_TYPE_INSTALL_PLUGINS_RETURN, result);

    callback = PyTuple_GetItem(py_user_data, 0);
    args = Py_BuildValue("(N)", py_result);

    len = PyTuple_Size(py_user_data);
    for (i = 1; i < len; ++i) {
	PyObject *tuple = args;
	args = PySequence_Concat(tuple, PyTuple_GetItem(py_user_data, i));
	Py_DECREF(tuple);
    }
    
    ret = PyObject_CallObject(callback, args);

    if (PyErr_Occurred())
	PyErr_Print();

    Py_DECREF(args);
    pyg_gil_state_release(state);

}
#endif
#line 68 "..\\..\\Source\\gst-python\\gst\\pbutils.c"


/* ---------- types from other modules ---------- */
static PyTypeObject *_PyGObject_Type;
#define PyGObject_Type (*_PyGObject_Type)
static PyTypeObject *_PyGstObject_Type;
#define PyGstObject_Type (*_PyGstObject_Type)
static PyTypeObject *_PyGstStructure_Type;
#define PyGstStructure_Type (*_PyGstStructure_Type)
static PyTypeObject *_PyGstElement_Type;
#define PyGstElement_Type (*_PyGstElement_Type)
static PyTypeObject *_PyGstMessage_Type;
#define PyGstMessage_Type (*_PyGstMessage_Type)


/* ---------- forward type declarations ---------- */
PyTypeObject PyGstInstallPluginsContext_Type;

#line 87 "..\\..\\Source\\gst-python\\gst\\pbutils.c"



/* ----------- GstInstallPluginsContext ----------- */

static int
_wrap_gst_install_plugins_context_new(PyGBoxed *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,":GstInstallPluginsContext.__init__", kwlist))
        return -1;
    self->gtype = GST_TYPE_INSTALL_PLUGINS_CONTEXT;
    self->free_on_dealloc = FALSE;
    self->boxed = gst_install_plugins_context_new();

    if (!self->boxed) {
        PyErr_SetString(PyExc_RuntimeError, "could not create GstInstallPluginsContext object");
        return -1;
    }
    self->free_on_dealloc = TRUE;
    return 0;
}

static PyObject *
_wrap_gst_install_plugins_context_set_xid(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "xid", NULL };
    PyObject *py_xid = NULL;
    guint xid = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:GstInstallPluginsContext.set_xid", kwlist, &py_xid))
        return NULL;
    if (py_xid) {
        if (PyLong_Check(py_xid))
            xid = PyLong_AsUnsignedLong(py_xid);
        else if (PyInt_Check(py_xid))
            xid = PyInt_AsLong(py_xid);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'xid' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    pyg_begin_allow_threads;
    gst_install_plugins_context_set_xid(pyg_boxed_get(self, GstInstallPluginsContext), xid);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGstInstallPluginsContext_methods[] = {
    { "set_xid", (PyCFunction)_wrap_gst_install_plugins_context_set_xid, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject PyGstInstallPluginsContext_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.pbutils.InstallPluginsContext",                   /* tp_name */
    sizeof(PyGBoxed),          /* tp_basicsize */
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
    (struct PyMethodDef*)_PyGstInstallPluginsContext_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    0,                 /* tp_dictoffset */
    (initproc)_wrap_gst_install_plugins_context_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- functions ----------- */

static PyObject *
_wrap_gst_pb_utils_add_codec_description_to_tag_list(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "taglist", "codec_tag", "caps", NULL };
    GstCaps *caps;
    GstTagList *taglist = NULL;
    char *codec_tag;
    PyObject *py_taglist, *py_caps;
    int ret;
    gboolean caps_is_copy;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"OsO:add_codec_description_to_tag_list", kwlist, &py_taglist, &codec_tag, &py_caps))
        return NULL;
    if (pyg_boxed_check(py_taglist, GST_TYPE_TAG_LIST))
        taglist = pyg_boxed_get(py_taglist, GstTagList);
    else {
        PyErr_SetString(PyExc_TypeError, "taglist should be a GstTagList");
        return NULL;
    }
    caps = pygst_caps_from_pyobject (py_caps, &caps_is_copy);
    if (PyErr_Occurred())
      return NULL;
    pyg_begin_allow_threads;
    ret = gst_pb_utils_add_codec_description_to_tag_list(taglist, codec_tag, caps);
    pyg_end_allow_threads;
    if (caps && caps_is_copy)
        gst_caps_unref (caps);
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_pb_utils_get_codec_description(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "caps", NULL };
    PyObject *py_caps;
    gchar *ret;
    gboolean caps_is_copy;
    GstCaps *caps;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:get_codec_description", kwlist, &py_caps))
        return NULL;
    caps = pygst_caps_from_pyobject (py_caps, &caps_is_copy);
    if (PyErr_Occurred())
      return NULL;
    pyg_begin_allow_threads;
    ret = gst_pb_utils_get_codec_description(caps);
    pyg_end_allow_threads;
    if (caps && caps_is_copy)
        gst_caps_unref (caps);
    if (ret) {
        PyObject *py_ret = PyString_FromString(ret);
        g_free(ret);
        return py_ret;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_pb_utils_get_source_description(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "protocol", NULL };
    char *protocol;
    gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:get_source_description", kwlist, &protocol))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_pb_utils_get_source_description(protocol);
    pyg_end_allow_threads;
    if (ret) {
        PyObject *py_ret = PyString_FromString(ret);
        g_free(ret);
        return py_ret;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_pb_utils_get_sink_description(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "protocol", NULL };
    char *protocol;
    gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:get_sink_description", kwlist, &protocol))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_pb_utils_get_sink_description(protocol);
    pyg_end_allow_threads;
    if (ret) {
        PyObject *py_ret = PyString_FromString(ret);
        g_free(ret);
        return py_ret;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_pb_utils_get_decoder_description(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "caps", NULL };
    PyObject *py_caps;
    gchar *ret;
    gboolean caps_is_copy;
    GstCaps *caps;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:get_decoder_description", kwlist, &py_caps))
        return NULL;
    caps = pygst_caps_from_pyobject (py_caps, &caps_is_copy);
    if (PyErr_Occurred())
      return NULL;
    pyg_begin_allow_threads;
    ret = gst_pb_utils_get_decoder_description(caps);
    pyg_end_allow_threads;
    if (caps && caps_is_copy)
        gst_caps_unref (caps);
    if (ret) {
        PyObject *py_ret = PyString_FromString(ret);
        g_free(ret);
        return py_ret;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_pb_utils_get_encoder_description(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "caps", NULL };
    PyObject *py_caps;
    gchar *ret;
    gboolean caps_is_copy;
    GstCaps *caps;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:get_encoder_description", kwlist, &py_caps))
        return NULL;
    caps = pygst_caps_from_pyobject (py_caps, &caps_is_copy);
    if (PyErr_Occurred())
      return NULL;
    pyg_begin_allow_threads;
    ret = gst_pb_utils_get_encoder_description(caps);
    pyg_end_allow_threads;
    if (caps && caps_is_copy)
        gst_caps_unref (caps);
    if (ret) {
        PyObject *py_ret = PyString_FromString(ret);
        g_free(ret);
        return py_ret;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_pb_utils_get_element_description(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "factory_name", NULL };
    char *factory_name;
    gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:get_element_description", kwlist, &factory_name))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_pb_utils_get_element_description(factory_name);
    pyg_end_allow_threads;
    if (ret) {
        PyObject *py_ret = PyString_FromString(ret);
        g_free(ret);
        return py_ret;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

#line 166 "..\\..\\Source\\gst-python\\gst\\pbutils.override"
static PyObject *
_wrap_gst_install_plugins_async(PyGObject *self, PyObject *args)
{
    PyObject *py_ctx, *py_ret, *py_details, *callback, *cbargs, *data;
    GstInstallPluginsContext *ctx;
    GstInstallPluginsReturn ret;
    gchar **details;
    gint len;
    Py_ssize_t i;

    if (PyTuple_Size(args) < 3) {
	PyErr_SetString(PyExc_TypeError, "install_plugins_async requires at least 3 arguments");
	return NULL;
    }

    py_ctx = PySequence_GetItem(args, 1);

    if (!pyg_boxed_check(py_ctx, GST_TYPE_INSTALL_PLUGINS_CONTEXT)) {
	PyErr_SetString(PyExc_TypeError, "Argument 2 must be a gst.pbutils.InstallPluginsContext");
	Py_DECREF(py_ctx);
	return NULL;
    }

    py_details = PySequence_GetItem(args, 0);
    if ((!PySequence_Check(py_details)) || (PySequence_Size(py_details) < 1)) {
	PyErr_SetString(PyExc_TypeError, "Details need to be a non-empty list or tuple of strings");
	Py_DECREF(py_ctx);
	Py_DECREF(py_details);
	return NULL;
    }

    len = PySequence_Size(py_details);
    details = g_new0(gchar*, len+1);

    /* Check all items in py_details are strings */
    for (i = 0; i < len; i++) {
	PyObject *py_str = PySequence_GetItem(py_details, i);
	gchar *str;

	if (!PyString_Check(py_str)) {
	    PyErr_SetString(PyExc_TypeError, "Details need to be a non-empty list or tuple of strings");
	    Py_DECREF(py_str);
	    Py_DECREF(py_ctx);
	    Py_DECREF(py_details);
	    g_strfreev(details);
	    return NULL;
	}
	if (!(str = PyString_AsString(py_str))) {
	    Py_DECREF(py_str);
	    Py_DECREF(py_ctx);
	    Py_DECREF(py_details);
	    g_strfreev(details);
	    return NULL;
	}
	details[i] = g_strdup(str);
	Py_DECREF(py_str);
    }

    callback = PySequence_GetItem(args, 2);
    if (!PyCallable_Check(callback)) {
	PyErr_SetString(PyExc_TypeError, "callback is not callable");
	Py_DECREF(callback);
	Py_DECREF(py_ctx);
	Py_DECREF(py_details);
	g_strfreev(details);
    }
    
    if (!(cbargs = PySequence_GetSlice(args, 3, PyTuple_Size(args)))) {
	Py_DECREF(callback);
	Py_DECREF(py_ctx);
	Py_DECREF(py_details);
	g_strfreev(details);
	return NULL;
    }
    if (!(data = Py_BuildValue("(ON)", callback, cbargs))) {
	Py_DECREF(py_details);
	Py_DECREF(py_ctx);
	Py_DECREF(callback);
	Py_DECREF(cbargs);
    }

    ctx = (GstInstallPluginsContext *) pyg_boxed_get(py_ctx, GstInstallPluginsContext);
    pyg_begin_allow_threads;
    ret = gst_install_plugins_async(details, ctx,
				    (GstInstallPluginsResultFunc) install_plugins_result_handler,
				    data);
    pyg_end_allow_threads;

    g_strfreev(details);

    py_ret = pyg_enum_from_gtype(GST_TYPE_INSTALL_PLUGINS_RETURN, ret);
    return py_ret;
}
#line 465 "..\\..\\Source\\gst-python\\gst\\pbutils.c"


#line 100 "..\\..\\Source\\gst-python\\gst\\pbutils.override"
static PyObject *
_wrap_gst_install_plugins_sync(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "details", "context", NULL };
    PyObject *py_ctx;
    GstInstallPluginsContext *ctx;
    GstInstallPluginsReturn ret;
    gchar **details;
    gint len;
    PyObject *py_ret;
    PyObject *py_details;
    Py_ssize_t i;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OO:install_plugins_sync",
				     kwlist, &py_details, &py_ctx))
	return NULL;

    if (!pyg_boxed_check(py_ctx, GST_TYPE_INSTALL_PLUGINS_CONTEXT)) {
	PyErr_SetString(PyExc_TypeError, "Argument 2 must be a gst.pbutils.InstallPluginsContext");
	return NULL;
    }

    len = PySequence_Size(py_details);
    if ((!PySequence_Check(py_details)) || (len < 1)) {
	PyErr_SetString(PyExc_TypeError, "Details need to be a non-empty list or tuple of strings");
	Py_DECREF(py_details);
	return NULL;
    }

    details = g_new0(gchar*, len+1);

    /* Check all items in py_details are strings */
    for (i = 0; i < len; i++) {
	PyObject *py_str = PySequence_GetItem(py_details, i);
	gchar *str;

	if (!PyString_Check(py_str)) {
	    PyErr_SetString(PyExc_TypeError, "Details need to be a non-empty list or tuple of strings");
	    Py_DECREF(py_str);
	    Py_DECREF(py_details);
	    g_strfreev(details);
	    return NULL;
	}
	if (!(str = PyString_AsString(py_str))) {
	    Py_DECREF(py_str);
	    Py_DECREF(py_details);
	    g_strfreev(details);
	    return NULL;
	}
	details[i] = g_strdup(str);
	Py_DECREF(py_str);
    }
    
    ctx = (GstInstallPluginsContext *) pyg_boxed_get(py_ctx, GstInstallPluginsContext);

    pyg_begin_allow_threads;
    ret = gst_install_plugins_sync(details, ctx);
    pyg_end_allow_threads;

    g_strfreev(details);

    py_ret = pyg_enum_from_gtype(GST_TYPE_INSTALL_PLUGINS_RETURN, ret);
    return py_ret;
}
#line 533 "..\\..\\Source\\gst-python\\gst\\pbutils.c"


static PyObject *
_wrap_gst_install_plugins_installation_in_progress(PyObject *self)
{
    int ret;

    pyg_begin_allow_threads;
    ret = gst_install_plugins_installation_in_progress();
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_install_plugins_supported(PyObject *self)
{
    int ret;

    pyg_begin_allow_threads;
    ret = gst_install_plugins_supported();
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_missing_uri_source_message_new(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "element", "protocol", NULL };
    PyGObject *element;
    char *protocol;
    GstMessage *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!s:missing_uri_source_message_new", kwlist, &PyGstElement_Type, &element, &protocol))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_missing_uri_source_message_new(GST_ELEMENT(element->obj), protocol);
    pyg_end_allow_threads;
    /* pygobject_new handles NULL checking */
    return pygstminiobject_new((GstMiniObject *)ret);
}

static PyObject *
_wrap_gst_missing_uri_sink_message_new(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "element", "protocol", NULL };
    PyGObject *element;
    char *protocol;
    GstMessage *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!s:missing_uri_sink_message_new", kwlist, &PyGstElement_Type, &element, &protocol))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_missing_uri_sink_message_new(GST_ELEMENT(element->obj), protocol);
    pyg_end_allow_threads;
    /* pygobject_new handles NULL checking */
    return pygstminiobject_new((GstMiniObject *)ret);
}

static PyObject *
_wrap_gst_missing_element_message_new(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "element", "factory_name", NULL };
    PyGObject *element;
    char *factory_name;
    GstMessage *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!s:missing_element_message_new", kwlist, &PyGstElement_Type, &element, &factory_name))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_missing_element_message_new(GST_ELEMENT(element->obj), factory_name);
    pyg_end_allow_threads;
    /* pygobject_new handles NULL checking */
    return pygstminiobject_new((GstMiniObject *)ret);
}

static PyObject *
_wrap_gst_missing_decoder_message_new(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "element", "decode_caps", NULL };
    PyGObject *element;
    PyObject *py_decode_caps;
    gboolean decode_caps_is_copy;
    GstCaps *decode_caps;
    GstMessage *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O:missing_decoder_message_new", kwlist, &PyGstElement_Type, &element, &py_decode_caps))
        return NULL;
    decode_caps = pygst_caps_from_pyobject (py_decode_caps, &decode_caps_is_copy);
    if (PyErr_Occurred())
      return NULL;
    pyg_begin_allow_threads;
    ret = gst_missing_decoder_message_new(GST_ELEMENT(element->obj), decode_caps);
    pyg_end_allow_threads;
    if (decode_caps && decode_caps_is_copy)
        gst_caps_unref (decode_caps);
    /* pygobject_new handles NULL checking */
    return pygstminiobject_new((GstMiniObject *)ret);
}

static PyObject *
_wrap_gst_missing_encoder_message_new(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "element", "encode_caps", NULL };
    PyGObject *element;
    PyObject *py_encode_caps;
    gboolean encode_caps_is_copy;
    GstCaps *encode_caps;
    GstMessage *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O:missing_encoder_message_new", kwlist, &PyGstElement_Type, &element, &py_encode_caps))
        return NULL;
    encode_caps = pygst_caps_from_pyobject (py_encode_caps, &encode_caps_is_copy);
    if (PyErr_Occurred())
      return NULL;
    pyg_begin_allow_threads;
    ret = gst_missing_encoder_message_new(GST_ELEMENT(element->obj), encode_caps);
    pyg_end_allow_threads;
    if (encode_caps && encode_caps_is_copy)
        gst_caps_unref (encode_caps);
    /* pygobject_new handles NULL checking */
    return pygstminiobject_new((GstMiniObject *)ret);
}

static PyObject *
_wrap_gst_missing_plugin_message_get_installer_detail(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "msg", NULL };
    gchar *ret;
    PyGstMiniObject *msg;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:missing_plugin_message_get_installer_detail", kwlist, &PyGstMessage_Type, &msg))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_missing_plugin_message_get_installer_detail(GST_MESSAGE(msg->obj));
    pyg_end_allow_threads;
    if (ret) {
        PyObject *py_ret = PyString_FromString(ret);
        g_free(ret);
        return py_ret;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_missing_plugin_message_get_description(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "msg", NULL };
    gchar *ret;
    PyGstMiniObject *msg;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:missing_plugin_message_get_description", kwlist, &PyGstMessage_Type, &msg))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_missing_plugin_message_get_description(GST_MESSAGE(msg->obj));
    pyg_end_allow_threads;
    if (ret) {
        PyObject *py_ret = PyString_FromString(ret);
        g_free(ret);
        return py_ret;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_is_missing_plugin_message(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "msg", NULL };
    int ret;
    PyGstMiniObject *msg;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:is_missing_plugin_message", kwlist, &PyGstMessage_Type, &msg))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_is_missing_plugin_message(GST_MESSAGE(msg->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_missing_uri_source_installer_detail_new(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "protocol", NULL };
    char *protocol;
    gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:missing_uri_source_installer_detail_new", kwlist, &protocol))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_missing_uri_source_installer_detail_new(protocol);
    pyg_end_allow_threads;
    if (ret) {
        PyObject *py_ret = PyString_FromString(ret);
        g_free(ret);
        return py_ret;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_missing_uri_sink_installer_detail_new(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "protocol", NULL };
    char *protocol;
    gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:missing_uri_sink_installer_detail_new", kwlist, &protocol))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_missing_uri_sink_installer_detail_new(protocol);
    pyg_end_allow_threads;
    if (ret) {
        PyObject *py_ret = PyString_FromString(ret);
        g_free(ret);
        return py_ret;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_missing_element_installer_detail_new(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "factory_name", NULL };
    char *factory_name;
    gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:missing_element_installer_detail_new", kwlist, &factory_name))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_missing_element_installer_detail_new(factory_name);
    pyg_end_allow_threads;
    if (ret) {
        PyObject *py_ret = PyString_FromString(ret);
        g_free(ret);
        return py_ret;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_missing_decoder_installer_detail_new(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "decode_caps", NULL };
    PyObject *py_decode_caps;
    gchar *ret;
    gboolean decode_caps_is_copy;
    GstCaps *decode_caps;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:missing_decoder_installer_detail_new", kwlist, &py_decode_caps))
        return NULL;
    decode_caps = pygst_caps_from_pyobject (py_decode_caps, &decode_caps_is_copy);
    if (PyErr_Occurred())
      return NULL;
    pyg_begin_allow_threads;
    ret = gst_missing_decoder_installer_detail_new(decode_caps);
    pyg_end_allow_threads;
    if (decode_caps && decode_caps_is_copy)
        gst_caps_unref (decode_caps);
    if (ret) {
        PyObject *py_ret = PyString_FromString(ret);
        g_free(ret);
        return py_ret;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_missing_encoder_installer_detail_new(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "encode_caps", NULL };
    PyObject *py_encode_caps;
    gchar *ret;
    gboolean encode_caps_is_copy;
    GstCaps *encode_caps;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:missing_encoder_installer_detail_new", kwlist, &py_encode_caps))
        return NULL;
    encode_caps = pygst_caps_from_pyobject (py_encode_caps, &encode_caps_is_copy);
    if (PyErr_Occurred())
      return NULL;
    pyg_begin_allow_threads;
    ret = gst_missing_encoder_installer_detail_new(encode_caps);
    pyg_end_allow_threads;
    if (encode_caps && encode_caps_is_copy)
        gst_caps_unref (encode_caps);
    if (ret) {
        PyObject *py_ret = PyString_FromString(ret);
        g_free(ret);
        return py_ret;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

const PyMethodDef pypbutils_functions[] = {
    { "add_codec_description_to_tag_list", (PyCFunction)_wrap_gst_pb_utils_add_codec_description_to_tag_list, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_codec_description", (PyCFunction)_wrap_gst_pb_utils_get_codec_description, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_source_description", (PyCFunction)_wrap_gst_pb_utils_get_source_description, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_sink_description", (PyCFunction)_wrap_gst_pb_utils_get_sink_description, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_decoder_description", (PyCFunction)_wrap_gst_pb_utils_get_decoder_description, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_encoder_description", (PyCFunction)_wrap_gst_pb_utils_get_encoder_description, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_element_description", (PyCFunction)_wrap_gst_pb_utils_get_element_description, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "install_plugins_async", (PyCFunction)_wrap_gst_install_plugins_async, METH_VARARGS,
      NULL },
    { "install_plugins_sync", (PyCFunction)_wrap_gst_install_plugins_sync, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "install_plugins_installation_in_progress", (PyCFunction)_wrap_gst_install_plugins_installation_in_progress, METH_NOARGS,
      NULL },
    { "install_plugins_supported", (PyCFunction)_wrap_gst_install_plugins_supported, METH_NOARGS,
      NULL },
    { "missing_uri_source_message_new", (PyCFunction)_wrap_gst_missing_uri_source_message_new, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "missing_uri_sink_message_new", (PyCFunction)_wrap_gst_missing_uri_sink_message_new, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "missing_element_message_new", (PyCFunction)_wrap_gst_missing_element_message_new, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "missing_decoder_message_new", (PyCFunction)_wrap_gst_missing_decoder_message_new, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "missing_encoder_message_new", (PyCFunction)_wrap_gst_missing_encoder_message_new, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "missing_plugin_message_get_installer_detail", (PyCFunction)_wrap_gst_missing_plugin_message_get_installer_detail, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "missing_plugin_message_get_description", (PyCFunction)_wrap_gst_missing_plugin_message_get_description, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "is_missing_plugin_message", (PyCFunction)_wrap_gst_is_missing_plugin_message, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "missing_uri_source_installer_detail_new", (PyCFunction)_wrap_gst_missing_uri_source_installer_detail_new, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "missing_uri_sink_installer_detail_new", (PyCFunction)_wrap_gst_missing_uri_sink_installer_detail_new, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "missing_element_installer_detail_new", (PyCFunction)_wrap_gst_missing_element_installer_detail_new, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "missing_decoder_installer_detail_new", (PyCFunction)_wrap_gst_missing_decoder_installer_detail_new, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "missing_encoder_installer_detail_new", (PyCFunction)_wrap_gst_missing_encoder_installer_detail_new, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};


/* ----------- enums and flags ----------- */

void
pypbutils_add_constants(PyObject *module, const gchar *strip_prefix)
{
  pyg_enum_add(module, "InstallPluginsReturn", strip_prefix, GST_TYPE_INSTALL_PLUGINS_RETURN);

  if (PyErr_Occurred())
    PyErr_Print();
}

/* initialise stuff extension classes */
void
pypbutils_register_classes(PyObject *d)
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
        _PyGstMessage_Type = (PyTypeObject *)PyDict_GetItemString(moddict, "Message");
        if (_PyGstMessage_Type == NULL) {
            PyErr_SetString(PyExc_ImportError,
                "cannot import name Message from gst");
            return;
        }
    } else {
        PyErr_SetString(PyExc_ImportError,
            "could not import gst");
        return;
    }


#line 954 "..\\..\\Source\\gst-python\\gst\\pbutils.c"
    pyg_register_boxed(d, "InstallPluginsContext", GST_TYPE_INSTALL_PLUGINS_CONTEXT, &PyGstInstallPluginsContext_Type);
}
