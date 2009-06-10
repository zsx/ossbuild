/* -- THIS FILE IS GENERATED - DO NOT EDIT *//* -*- Mode: C; c-basic-offset: 4 -*- */

#include <Python.h>



#line 25 "..\\..\\Source\\gst-python\\gst\\gst.override"
/* define this for all source files that don't run init_pygobject()
 * before including pygobject.h */
#define NO_IMPORT_PYGOBJECT

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "common.h"

#include <gst/gst.h>
#include <gst/gsterror.h>
#include <gst/gsttypefind.h>
#include <gst/gsttagsetter.h>

#include <gst/controller/gstcontroller.h>
#if ((GST_VERSION_MICRO >= 15) || (GST_VERSION_MICRO == 14 && GST_VERSION_NANO > 0))
#include <gst/controller/gstlfocontrolsource.h>
#endif
#include <gst/dataprotocol/dataprotocol.h>
#include <gst/base/gstadapter.h>
#include <gst/base/gstbasesrc.h>
#include <gst/base/gstpushsrc.h>
#include <gst/base/gstbasesink.h>
#include <gst/base/gstbasetransform.h>
#include <gst/base/gstcollectpads.h>
#include <gst/base/gsttypefindhelper.h>
#if ((GST_VERSION_MICRO >= 11) || (GST_VERSION_MICRO == 10 && GST_VERSION_NANO > 0 ))
#include <gst/base/gstdataqueue.h>
#endif

#include <gst/net/gstnet.h>

#include "pygstvalue.h"
#include "pygstminiobject.h"
#include "pygstexception.h"

/* These headers have been included directly to get around multiple 
 * GetAttrString calls */
#include <compile.h>
#include <frameobject.h>

/* Boonky define that allows for backwards compatibility with Python 2.4 */
#if PY_VERSION_HEX < 0x02050000 
#define Py_ssize_t int
#endif 

GST_DEBUG_CATEGORY_EXTERN (python_debug);
GST_DEBUG_CATEGORY_EXTERN (pygst_debug);
#define GST_CAT_DEFAULT pygst_debug

/* This function checks if a former Python code threw an exception and if
 * so, transforms this exception into an error on the given GstElement.
 * This allows code run on the element to just raise an exception instead of
 * returning GStreamer specific return values.
 * The exception is cleared afterwards.
 */
gboolean
_pygst_element_check_error (GstElement *element)
{
  PyObject *type, *value, *traceback, *lineno, *msg, *typemsg;
  PyFrameObject *frame;

  if (!PyErr_Occurred())
	  return FALSE;

  PyErr_Fetch (&type, &value, &traceback);
  if (traceback) {
    frame = (PyFrameObject *) PyObject_GetAttrString (traceback, "tb_frame");
    lineno = PyObject_GetAttrString (traceback, "tb_lineno");
  } else {
    frame = NULL;
    lineno = NULL;
  }
  msg = PyObject_Str (value);
  typemsg = PyObject_Str (type);
  if (msg && PyString_Check (msg)) {
	    gst_element_message_full (element, GST_MESSAGE_ERROR,
                    GST_LIBRARY_ERROR,
		    GST_LIBRARY_ERROR_FAILED,
		    g_strdup (PyString_AsString (msg)), 
		    typemsg ? g_strconcat (PyString_AsString (typemsg), 
			    ": ", PyString_AsString (msg), NULL)
		    : g_strdup (PyString_AsString (msg)),
		    frame ? PyString_AsString(frame->f_code->co_filename) : "???",
		    frame ? PyString_AsString(frame->f_code->co_name) : "???",
		    lineno ? PyInt_AsLong (lineno) : 0);
  } else {
	gst_element_message_full (element, GST_MESSAGE_ERROR,
                    GST_LIBRARY_ERROR,
		    GST_LIBRARY_ERROR_TOO_LAZY,
		    NULL, NULL,
		    frame ? PyString_AsString(frame->f_code->co_filename) : "???",
		    frame ? PyString_AsString(frame->f_code->co_name) : "???",
		    lineno ? PyInt_AsLong (lineno) : 0);
  }

  PyErr_Clear ();
  Py_XDECREF (frame);
  Py_XDECREF (lineno);
  Py_DECREF (msg);
  Py_DECREF (typemsg);

  return TRUE;
}

PyTypeObject PyGstPadTemplate_Type;
static int
add_templates (gpointer gclass, PyObject *templates)
{
  gint i, len;
  PyGObject *templ;
  
  if (pygobject_check(templates, &PyGstPadTemplate_Type)) {
    gst_element_class_add_pad_template (gclass, GST_PAD_TEMPLATE (pygobject_get (templates)));
    return 0;
  }
  
  if (!PyTuple_Check(templates)) {
    PyErr_SetString(PyExc_TypeError, "__gsttemplates__ attribute neither a tuple nor a GstPadTemplate!");
    return -1;
  }
  len = PyTuple_Size(templates);
  if (len == 0)
    return 0;
  
  for (i = 0; i < len; i++) {
    templ = (PyGObject*) PyTuple_GetItem(templates, i);
    if (!pygobject_check(templ, &PyGstPadTemplate_Type)) {
      PyErr_SetString(PyExc_TypeError, "entries for __gsttemplates__ must be of type GstPadTemplate");
      return -1;
    }
  }
	
  for (i = 0; i < len; i++) {
    templ = (PyGObject*) PyTuple_GetItem(templates, i);
    gst_element_class_add_pad_template (gclass, GST_PAD_TEMPLATE (templ->obj));
  }
  return 0;
}

static int
_pygst_element_set_details (gpointer gclass, PyObject *details)
{
  GstElementDetails gstdetails = { 0, };
  
  if (!PyTuple_Check (details)) {
    PyErr_SetString(PyExc_TypeError, "__gstdetails__ must be a tuple");
    return -1;
  }
  if (PyTuple_Size (details) != 4) {
    PyErr_SetString(PyExc_TypeError, "__gstdetails__ must be contain 4 elements");
    return -1;
  }
  if (!PyArg_ParseTuple (details, "ssss", &gstdetails.longname, &gstdetails.klass,
    &gstdetails.description, &gstdetails.author)) {
    PyErr_SetString (PyExc_TypeError, "__gstdetails__ must be contain 4 strings");
    return -1;
  }
  gst_element_class_set_details (gclass, &gstdetails);
  return 0;
}

static int
_pygst_element_init (gpointer gclass, PyTypeObject *pyclass)
{
  PyObject *templates, *details;
  
  templates = PyDict_GetItemString(pyclass->tp_dict, "__gsttemplates__");
  if (templates) {
    if (add_templates(gclass, templates) != 0)
      return -1;
  } else {
    PyErr_Clear();
  }
  details = PyDict_GetItemString(pyclass->tp_dict, "__gstdetails__");
  if (details) {
    if (_pygst_element_set_details (gclass, details) != 0)
      return -1;
    PyDict_DelItemString(pyclass->tp_dict, "__gstdetails__");
  } else {
    PyErr_Clear();
  }

  return 0;
}

static PyObject *
pygst_debug_log (PyObject *pyobject, PyObject *string, GstDebugLevel level,
    gboolean isgstobject)
{
#ifndef GST_DISABLE_GST_DEBUG
    gchar	*str;
    gchar	*function;
    gchar	*filename;
    int		lineno;
    PyFrameObject	*frame;
    GObject   *object = NULL;

    if (!PyArg_ParseTuple(string, "s:gst.debug_log", &str)) {
	PyErr_SetString(PyExc_TypeError, "Need a string!");
	return NULL;
    }

    frame = PyEval_GetFrame();
    function = PyString_AsString(frame->f_code->co_name);
    filename = g_path_get_basename(PyString_AsString(frame->f_code->co_filename));
    lineno = PyCode_Addr2Line(frame->f_code, frame->f_lasti);
    /* gst_debug_log : category, level, file, function, line, object, format, va_list */
    if (isgstobject)
      object = G_OBJECT (pygobject_get (pyobject));
    gst_debug_log (python_debug, level, filename, function, lineno, object, "%s", str);
    if (filename)
	g_free(filename);
#endif
    Py_INCREF (Py_None);
    return Py_None;    
}

GType
gst_tag_get_tag_type (const gchar * tag)
{
  return gst_tag_get_type (tag);
}


#line 25 "gstbuffer.override"

static Py_ssize_t gst_buffer_getreadbuffer (PyObject *self, 
                           Py_ssize_t index, void **ptr);
static Py_ssize_t gst_buffer_getwritebuf   (PyObject   *self,
			   Py_ssize_t index, void **ptr);
static Py_ssize_t gst_buffer_getsegcount   (PyObject   *self,
			   Py_ssize_t *lenp);

#if PY_VERSION_HEX >= 0x02050000
static Py_ssize_t gst_buffer_getcharbuf    (PyObject   *self,
			   Py_ssize_t index, char **ptr);
#else
static Py_ssize_t gst_buffer_getcharbuf    (PyObject   *self,
			   Py_ssize_t index, const char **ptr);
#endif

#line 31 "gstbus.override"
static GstBusSyncReply
bus_sync_handler (GstBus *bus, GstMessage *message, gpointer user_data)
{
    PyGILState_STATE state;
    GstBusSyncReply    res;
    PyObject    *py_userdata;
    PyObject    *py_msg;
    PyObject    *callback, *args;
    PyObject    *ret;
    gint        i, len;

    g_return_val_if_fail (user_data != NULL, GST_BUS_PASS);

    state = pyg_gil_state_ensure ();

    py_userdata = (PyObject *) user_data;
    py_msg = pygstminiobject_new (GST_MINI_OBJECT (message));
    callback = PyTuple_GetItem (py_userdata, 0);

    /* Using N we give away our references to the args tuple */
    args = Py_BuildValue ("(NN)",
                          pygobject_new (G_OBJECT (bus)),
                          py_msg);

    /* add all *args to the args tuple object */
    len = PyTuple_Size (py_userdata);
    for (i = 1; i < len; ++i) {
        PyObject *tuple = args;
        args = PySequence_Concat (tuple, PyTuple_GetItem (py_userdata, i));
        Py_DECREF (tuple);
    }
    ret = PyObject_CallObject (callback, args);

    if (!ret) {
        PyErr_Print ();
        res = GST_BUS_PASS;
    } else {
        if (ret == Py_None) {
            PyErr_SetString (PyExc_TypeError,
                "callback should return a BusSyncReply");
            PyErr_Print ();
            res = GST_BUS_PASS;    
        } else if (pyg_enum_get_value (GST_TYPE_BUS_SYNC_REPLY, ret,
                                       (gint *) &res))
            res = GST_BUS_PASS;

        Py_DECREF (ret);
    }
    Py_DECREF (args);

    pyg_gil_state_release (state);

    return res;
}

static gboolean
bus_func (GstBus *bus, GstMessage *message, gpointer user_data)
{
    PyGILState_STATE state;
    gboolean    res;
    PyObject    *py_userdata;
    PyObject    *py_msg;
    PyObject    *callback, *args;
    PyObject    *ret;
    gint        i, len;

    g_return_val_if_fail (user_data != NULL, TRUE);

    GST_DEBUG_OBJECT (bus, "dispatching message %p", message);

    state = pyg_gil_state_ensure ();

    py_userdata = (PyObject *) user_data;
    g_assert (PyTuple_Check (py_userdata));

    py_msg = pygstminiobject_new (GST_MINI_OBJECT (message));
    callback = PyTuple_GetItem (py_userdata, 0);

    /* Using N we give away our references to the args tuple */
    args = Py_BuildValue ("(NN)",
                          pygobject_new (G_OBJECT (bus)),
                          py_msg);
    g_assert (args);

    /* add all *args to the args tuple object */
    len = PyTuple_Size (py_userdata);
    for (i = 1; i < len; ++i) {
        PyObject *item;
        PyObject *tuple = args;

        item = PyTuple_GetItem (py_userdata, i);
        g_assert (item);

        args = PySequence_Concat (tuple, item);
        g_assert (args);

        Py_DECREF (tuple);
    }
    ret = PyObject_CallObject(callback, args);

    if (!ret) {
        PyErr_Print ();
        res = TRUE;
    } else {
        if (ret == Py_None) {
            PyErr_SetString (PyExc_TypeError,
                "callback should return True or False");
            PyErr_Print ();
            res = TRUE;
        } else 
            res = PyObject_IsTrue (ret);
        Py_DECREF (ret);
    }
    Py_DECREF (args);

    pyg_gil_state_release (state);

    GST_DEBUG_OBJECT (bus, "dispatched message %p", message);

    return res;
}


#line 26 "gstcaps.override"
/* This is a (hopefully) smart hack to allow access to a caps' 
 * structures without falling into traps when the caps get destroyed
 * before the structures get.
 * This Hash Table uses the structure PyObjects as keys and the caps 
 * PyObjects as values. No clue if this is a fast data structure but it
 * probably doesn't matter anyway.
 */
static GHashTable *structure_caps_map = NULL;

static void
pygst_caps_map_add (PyObject *structure, PyObject *caps)
{
  /* we can't have free_on_dealloc stuff in here */
  g_assert (((PyGBoxed *)structure)->free_on_dealloc == FALSE);
  g_hash_table_insert (structure_caps_map, structure, caps);
}

static void
pygst_caps_map_remove_structure (PyObject *structure)
{
  g_hash_table_remove (structure_caps_map, structure);
}

static gboolean
pygst_caps_map_foreach (gpointer structure, gpointer caps, gpointer match)
{
  PyGBoxed *boxed = structure;
  
  if (match != caps)
    return FALSE;

  /* we can't have free_on_dealloc stuff in here */
  g_assert (boxed->free_on_dealloc == FALSE);
  boxed->boxed = gst_structure_copy (boxed->boxed);
  boxed->free_on_dealloc = TRUE;
  return TRUE;
}

static void
pygst_caps_map_modified (PyObject *caps)
{
  g_hash_table_foreach_remove (structure_caps_map, pygst_caps_map_foreach, caps);
}


#line 26 "gstpad.override"

#if ((GST_VERSION_MAJOR > 0) ||                                    \
     (GST_VERSION_MAJOR == 0 && GST_VERSION_MINOR > 10) ||         \
     (GST_VERSION_MAJOR == 0 && GST_VERSION_MINOR == 10 &&         \
      GST_VERSION_MICRO >= 23) ||						   \
     (GST_VERSION_MAJOR == 0 && GST_VERSION_MINOR == 10 &&         \
      GST_VERSION_MICRO == 22 && GST_VERSION_NANO > 0))
#define HAVE_SET_BLOCKED_ASYNC_FULL 1
#else
#define HAVE_SET_BLOCKED_ASYNC_FULL 0
#endif

/* we need to do this until PyClosures get exception handlers */
#ifndef pyg_closure_set_exception_handler
#  define pyg_closure_set_exception_handler(ig, nore)
#  define EXCEPTION_HANDLER G_GNUC_UNUSED
#else
#  define EXCEPTION_HANDLER
#endif
#define SET_PAD_CLOSURE(self, args, kwargs, name) \
    static char *kwlist[] = { G_STRINGIFY (name), NULL }; \
    PyObject *function; \
    GstPad *pad; \
    GClosure *closure; \
    PyGstPadPrivate *priv; \
\
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, \
                     "O:GstPad.set_" G_STRINGIFY (name), \
                     kwlist, \
                     &function)) { \
        return NULL; \
    } \
\
    if (!PyCallable_Check(function)) { \
        PyErr_SetString(PyExc_TypeError, G_STRINGIFY (name) " not callable"); \
        return NULL; \
    } \
\
    closure = pyg_closure_new (function, NULL, NULL); \
    pyg_closure_set_exception_handler (closure, handle_ ## name ## _exception); \
    pygobject_watch_closure((PyObject *)self, closure); \
    priv = py_pad_private(self);\
    if (priv->name) { \
      g_closure_invalidate (priv->name); \
      g_closure_unref (priv->name); \
    } \
    priv->name = closure; \
    pad = (GstPad*)pygobject_get(self); \
    gst_pad_set_ ## name (pad, call_ ## name); \
\
    Py_INCREF(Py_None); \
    return Py_None;
    
static void
free_pad_private (gpointer data)
{
  PyGstPadPrivate *private = data;
#define INVALIDATE_CLOSURE(closure) \
  if (closure) { \
    g_closure_invalidate (closure); \
    g_closure_unref (closure); \
    closure = NULL; \
  } 
  INVALIDATE_CLOSURE (private->link_function)
  INVALIDATE_CLOSURE (private->event_function)
  INVALIDATE_CLOSURE (private->chain_function)
  INVALIDATE_CLOSURE (private->get_function)
  INVALIDATE_CLOSURE (private->getcaps_function)
  INVALIDATE_CLOSURE (private->setcaps_function)
  INVALIDATE_CLOSURE (private->activate_function)
  INVALIDATE_CLOSURE (private->activatepull_function)
  INVALIDATE_CLOSURE (private->activatepush_function)
#undef INVALIDATE_CLOSURE

  if (private->query_function) {
    Py_DECREF (private->query_function);
    private->query_function = NULL;
  }
}

static PyGstPadPrivate*
pad_private(GstPad *pad)
{
    PyGstPadPrivate *private;
    static GQuark padprivate = 0;

    if (!padprivate)
        padprivate = g_quark_from_static_string ("PyGst::PadPrivate");
    private = g_object_get_qdata (G_OBJECT (pad), padprivate);
    if (private == NULL) {
        private = g_new0(PyGstPadPrivate, 1);
        private->pad = (PyGObject *) pygobject_new (G_OBJECT (pad));
        Py_DECREF (private->pad);
        g_object_set_qdata_full (G_OBJECT (pad), padprivate, private, free_pad_private);
    }
    return private;
}
    
static PyGstPadPrivate*
py_pad_private(PyGObject *pad)
{
    return pad_private ((GstPad *)pygobject_get(pad));
}

#line 527 "..\\..\\Source\\gst-python\\gst\\gst.c"


/* ---------- types from other modules ---------- */
static PyTypeObject *_PyGObject_Type;
#define PyGObject_Type (*_PyGObject_Type)


/* ---------- forward type declarations ---------- */
PyTypeObject PyGstCaps_Type;
PyTypeObject PyGstStructure_Type;
PyTypeObject PyGstTagList_Type;
PyTypeObject PyGError_Type;
PyTypeObject PyGstDate_Type;
PyTypeObject PyGstSegment_Type;
PyTypeObject PyGstIndexEntry_Type;
PyTypeObject PyGstObject_Type;
PyTypeObject PyGstIndex_Type;
PyTypeObject PyGstElement_Type;
PyTypeObject PyGstBin_Type;
PyTypeObject PyGstClock_Type;
PyTypeObject PyGstBus_Type;
PyTypeObject PyGstPad_Type;
PyTypeObject PyGstGhostPad_Type;
PyTypeObject PyGstPadTemplate_Type;
PyTypeObject PyGstPipeline_Type;
PyTypeObject PyGstPluginFeature_Type;
PyTypeObject PyGstIndexFactory_Type;
PyTypeObject PyGstElementFactory_Type;
PyTypeObject PyGstRegistry_Type;
PyTypeObject PyGstSystemClock_Type;
PyTypeObject PyGstTask_Type;
PyTypeObject PyGstTypeFindFactory_Type;
PyTypeObject PyGstXML_Type;
PyTypeObject PyGstNetClientClock_Type;
PyTypeObject PyGstNetTimeProvider_Type;
PyTypeObject PyGstPlugin_Type;
PyTypeObject PyGstControlSource_Type;
PyTypeObject PyGstController_Type;
PyTypeObject PyGstInterpolationControlSource_Type;
PyTypeObject PyGstLFOControlSource_Type;
PyTypeObject PyGstAdapter_Type;
PyTypeObject PyGstBaseSink_Type;
PyTypeObject PyGstBaseSrc_Type;
PyTypeObject PyGstBaseTransform_Type;
PyTypeObject PyGstCollectPads_Type;
PyTypeObject PyGstDataQueue_Type;
PyTypeObject PyGstBuffer_Type;
PyTypeObject PyGstEvent_Type;
PyTypeObject PyGstMessage_Type;
PyTypeObject PyGstQuery_Type;
PyTypeObject PyGstURIHandler_Type;
PyTypeObject PyGstTagSetter_Type;
PyTypeObject PyGstImplementsInterface_Type;
PyTypeObject PyGstPreset_Type;

#line 583 "..\\..\\Source\\gst-python\\gst\\gst.c"



/* ----------- GstCaps ----------- */

#line 95 "gstcaps.override"
static int
_wrap_gst_caps_new_empty(PyGBoxed *self, PyObject *args, PyObject *kwargs)
{
	PyObject* item;
	int len, i;

        /* we wrap caps_new, caps_from_string and caps_new_full */
        len = PyTuple_Size(args);
        self->gtype = GST_TYPE_CAPS;
	self->free_on_dealloc = TRUE;
	
        if (len == 0) {
		/* 0 length creates a new empty caps */
		self->boxed = gst_caps_new_empty();
	} else if (len == 1) {
		item = PyTuple_GetItem(args, 0);
		/* 1 length is either a string or a structure */
		self->boxed = pygst_caps_from_pyobject (item, NULL);
	} else {
		/* it's multiple arguments that can all be made to caps */
		GstCaps *append;
		self->boxed = gst_caps_new_empty();
		for (i = 0; i < len; i++)
		{
			item = PyTuple_GetItem(args, i);
			append = pygst_caps_from_pyobject (item, NULL);
			if (!append) {
			      gst_caps_unref (self->boxed);
			      self->boxed = NULL;
			      break;
			}
			gst_caps_append (self->boxed, append);
		}
	}
	if (!self->boxed) {
		PyErr_SetString(PyExc_TypeError, "wrong arguments when creating GstCaps object");
		return -1;
	}
	return 0;
}

#line 631 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_caps_make_writable(PyObject *self)
{
    GstCaps *ret;

    pyg_begin_allow_threads;
    ret = gst_caps_make_writable(pyg_boxed_get(self, GstCaps));
    pyg_end_allow_threads;
    return pyg_boxed_new (GST_TYPE_CAPS, ret, FALSE, TRUE);
}

static PyObject *
_wrap_gst_caps_append(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "caps2", NULL };
    PyObject *py_caps2;
    GstCaps *caps2;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:GstCaps.append", kwlist, &py_caps2))
        return NULL;
    caps2 = pygst_caps_from_pyobject (py_caps2, NULL);
    if (PyErr_Occurred())
      return NULL;
    pyg_begin_allow_threads;
    gst_caps_append(pyg_boxed_get(self, GstCaps), caps2);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

#line 454 "gstcaps.override"
static PyObject *
_wrap_gst_caps_append_structure(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "structure", NULL };
    PyObject *py_structure;
    GstStructure *structure = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:GstCaps.append_structure", kwlist, &py_structure))
        return NULL;
    if (pyg_boxed_check(py_structure, GST_TYPE_STRUCTURE))
        structure = gst_structure_copy(pyg_boxed_get(py_structure, GstStructure));
    else {
        PyErr_SetString(PyExc_TypeError, "structure should be a GstStructure");
        return NULL;
    }
    pyg_begin_allow_threads;
    gst_caps_append_structure(pyg_boxed_get(self, GstCaps), structure);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}
#line 686 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_caps_get_size(PyObject *self)
{
    guint ret;

    pyg_begin_allow_threads;
    ret = gst_caps_get_size(pyg_boxed_get(self, GstCaps));
    pyg_end_allow_threads;
    return PyLong_FromUnsignedLong(ret);
}

#line 80 "gstcaps.override"
static PyObject *pygst_caps_sq_item(PyObject *self, Py_ssize_t i);
static PyObject *
_wrap_gst_caps_get_structure(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *kwlist[] = { "index", NULL };
	int index;

	if (PyErr_Warn(PyExc_DeprecationWarning, "caps.get_structure(i) is deprecated, use caps[i]") < 0)
		return NULL;
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "i:GstCaps.get_structure", kwlist, &index))
		return NULL;
	return pygst_caps_sq_item (self, index);
}
#line 714 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_caps_copy_nth(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "nth", NULL };
    PyObject *py_nth = NULL;
    GstCaps *ret;
    guint nth = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:GstCaps.copy_nth", kwlist, &py_nth))
        return NULL;
    if (py_nth) {
        if (PyLong_Check(py_nth))
            nth = PyLong_AsUnsignedLong(py_nth);
        else if (PyInt_Check(py_nth))
            nth = PyInt_AsLong(py_nth);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'nth' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    pyg_begin_allow_threads;
    ret = gst_caps_copy_nth(pyg_boxed_get(self, GstCaps), nth);
    pyg_end_allow_threads;
    return pyg_boxed_new (GST_TYPE_CAPS, ret, FALSE, TRUE);
}

static PyObject *
_wrap_gst_caps_truncate(PyObject *self)
{
    pyg_begin_allow_threads;
    gst_caps_truncate(pyg_boxed_get(self, GstCaps));
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_caps_is_any(PyObject *self)
{
    int ret;

    pyg_begin_allow_threads;
    ret = gst_caps_is_any(pyg_boxed_get(self, GstCaps));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_caps_is_empty(PyObject *self)
{
    int ret;

    pyg_begin_allow_threads;
    ret = gst_caps_is_empty(pyg_boxed_get(self, GstCaps));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_caps_is_fixed(PyObject *self)
{
    int ret;

    pyg_begin_allow_threads;
    ret = gst_caps_is_fixed(pyg_boxed_get(self, GstCaps));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_caps_is_always_compatible(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "caps2", NULL };
    PyObject *py_caps2;
    int ret;
    gboolean caps2_is_copy;
    GstCaps *caps2;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:GstCaps.is_always_compatible", kwlist, &py_caps2))
        return NULL;
    caps2 = pygst_caps_from_pyobject (py_caps2, &caps2_is_copy);
    if (PyErr_Occurred())
      return NULL;
    pyg_begin_allow_threads;
    ret = gst_caps_is_always_compatible(pyg_boxed_get(self, GstCaps), caps2);
    pyg_end_allow_threads;
    if (caps2 && caps2_is_copy)
        gst_caps_unref (caps2);
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_caps_is_subset(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "superset", NULL };
    PyObject *py_superset;
    int ret;
    gboolean superset_is_copy;
    GstCaps *superset;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:GstCaps.is_subset", kwlist, &py_superset))
        return NULL;
    superset = pygst_caps_from_pyobject (py_superset, &superset_is_copy);
    if (PyErr_Occurred())
      return NULL;
    pyg_begin_allow_threads;
    ret = gst_caps_is_subset(pyg_boxed_get(self, GstCaps), superset);
    pyg_end_allow_threads;
    if (superset && superset_is_copy)
        gst_caps_unref (superset);
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_caps_is_equal(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "caps2", NULL };
    PyObject *py_caps2;
    int ret;
    gboolean caps2_is_copy;
    GstCaps *caps2;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:GstCaps.is_equal", kwlist, &py_caps2))
        return NULL;
    caps2 = pygst_caps_from_pyobject (py_caps2, &caps2_is_copy);
    if (PyErr_Occurred())
      return NULL;
    pyg_begin_allow_threads;
    ret = gst_caps_is_equal(pyg_boxed_get(self, GstCaps), caps2);
    pyg_end_allow_threads;
    if (caps2 && caps2_is_copy)
        gst_caps_unref (caps2);
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_caps_intersect(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "caps2", NULL };
    PyObject *py_caps2;
    gboolean caps2_is_copy;
    GstCaps *caps2, *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:GstCaps.intersect", kwlist, &py_caps2))
        return NULL;
    caps2 = pygst_caps_from_pyobject (py_caps2, &caps2_is_copy);
    if (PyErr_Occurred())
      return NULL;
    pyg_begin_allow_threads;
    ret = gst_caps_intersect(pyg_boxed_get(self, GstCaps), caps2);
    pyg_end_allow_threads;
    if (caps2 && caps2_is_copy)
        gst_caps_unref (caps2);
    return pyg_boxed_new (GST_TYPE_CAPS, ret, FALSE, TRUE);
}

static PyObject *
_wrap_gst_caps_subtract(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "subtrahend", NULL };
    PyObject *py_subtrahend;
    gboolean subtrahend_is_copy;
    GstCaps *subtrahend, *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:GstCaps.subtract", kwlist, &py_subtrahend))
        return NULL;
    subtrahend = pygst_caps_from_pyobject (py_subtrahend, &subtrahend_is_copy);
    if (PyErr_Occurred())
      return NULL;
    pyg_begin_allow_threads;
    ret = gst_caps_subtract(pyg_boxed_get(self, GstCaps), subtrahend);
    pyg_end_allow_threads;
    if (subtrahend && subtrahend_is_copy)
        gst_caps_unref (subtrahend);
    return pyg_boxed_new (GST_TYPE_CAPS, ret, FALSE, TRUE);
}

static PyObject *
_wrap_gst_caps_union(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "caps2", NULL };
    PyObject *py_caps2;
    gboolean caps2_is_copy;
    GstCaps *caps2, *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:GstCaps.union", kwlist, &py_caps2))
        return NULL;
    caps2 = pygst_caps_from_pyobject (py_caps2, &caps2_is_copy);
    if (PyErr_Occurred())
      return NULL;
    pyg_begin_allow_threads;
    ret = gst_caps_union(pyg_boxed_get(self, GstCaps), caps2);
    pyg_end_allow_threads;
    if (caps2 && caps2_is_copy)
        gst_caps_unref (caps2);
    return pyg_boxed_new (GST_TYPE_CAPS, ret, FALSE, TRUE);
}

static PyObject *
_wrap_gst_caps_normalize(PyObject *self)
{
    GstCaps *ret;

    pyg_begin_allow_threads;
    ret = gst_caps_normalize(pyg_boxed_get(self, GstCaps));
    pyg_end_allow_threads;
    return pyg_boxed_new (GST_TYPE_CAPS, ret, FALSE, TRUE);
}

static PyObject *
_wrap_gst_caps_do_simplify(PyObject *self)
{
    int ret;

    pyg_begin_allow_threads;
    ret = gst_caps_do_simplify(pyg_boxed_get(self, GstCaps));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_caps_merge(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "caps2", NULL };
    PyObject *py_caps2;
    GstCaps *caps2;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:GstCaps.merge", kwlist, &py_caps2))
        return NULL;
    caps2 = pygst_caps_from_pyobject (py_caps2, NULL);
    if (PyErr_Occurred())
      return NULL;
    pyg_begin_allow_threads;
    gst_caps_merge(pyg_boxed_get(self, GstCaps), caps2);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_caps_merge_structure(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "structure", NULL };
    PyObject *py_structure;
    GstStructure *structure = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:GstCaps.merge_structure", kwlist, &py_structure))
        return NULL;
    if (pyg_boxed_check(py_structure, GST_TYPE_STRUCTURE))
        structure = pyg_boxed_get(py_structure, GstStructure);
    else {
        PyErr_SetString(PyExc_TypeError, "structure should be a GstStructure");
        return NULL;
    }
    pyg_begin_allow_threads;
    gst_caps_merge_structure(pyg_boxed_get(self, GstCaps), structure);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_caps_to_string(PyObject *self)
{
    gchar *ret;

    pyg_begin_allow_threads;
    ret = gst_caps_to_string(pyg_boxed_get(self, GstCaps));
    pyg_end_allow_threads;
    if (ret) {
        PyObject *py_ret = PyString_FromString(ret);
        g_free(ret);
        return py_ret;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGstCaps_methods[] = {
    { "make_writable", (PyCFunction)_wrap_gst_caps_make_writable, METH_NOARGS,
      NULL },
    { "append", (PyCFunction)_wrap_gst_caps_append, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "append_structure", (PyCFunction)_wrap_gst_caps_append_structure, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_size", (PyCFunction)_wrap_gst_caps_get_size, METH_NOARGS,
      NULL },
    { "get_structure", (PyCFunction)_wrap_gst_caps_get_structure, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "copy_nth", (PyCFunction)_wrap_gst_caps_copy_nth, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "truncate", (PyCFunction)_wrap_gst_caps_truncate, METH_NOARGS,
      NULL },
    { "is_any", (PyCFunction)_wrap_gst_caps_is_any, METH_NOARGS,
      NULL },
    { "is_empty", (PyCFunction)_wrap_gst_caps_is_empty, METH_NOARGS,
      NULL },
    { "is_fixed", (PyCFunction)_wrap_gst_caps_is_fixed, METH_NOARGS,
      NULL },
    { "is_always_compatible", (PyCFunction)_wrap_gst_caps_is_always_compatible, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "is_subset", (PyCFunction)_wrap_gst_caps_is_subset, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "is_equal", (PyCFunction)_wrap_gst_caps_is_equal, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "intersect", (PyCFunction)_wrap_gst_caps_intersect, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "subtract", (PyCFunction)_wrap_gst_caps_subtract, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "union", (PyCFunction)_wrap_gst_caps_union, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "normalize", (PyCFunction)_wrap_gst_caps_normalize, METH_NOARGS,
      NULL },
    { "do_simplify", (PyCFunction)_wrap_gst_caps_do_simplify, METH_NOARGS,
      NULL },
    { "merge", (PyCFunction)_wrap_gst_caps_merge, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "merge_structure", (PyCFunction)_wrap_gst_caps_merge_structure, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "to_string", (PyCFunction)_wrap_gst_caps_to_string, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

#line 445 "gstcaps.override"

static PyObject *
_wrap_gst_caps__get___refcount__(PyGObject *self, void *closure)
{
    return PyInt_FromLong(GST_CAPS_REFCOUNT(self->obj));
}

#line 1056 "..\\..\\Source\\gst-python\\gst\\gst.c"


static const PyGetSetDef gst_caps_getsets[] = {
    { "__refcount__", (getter)_wrap_gst_caps__get___refcount__, (setter)0 },
    { NULL, (getter)0, (setter)0 },
};

#line 227 "gstcaps.override"

/* In this number code, we mimic the Python set.Set datatype.
 * The same operations are supported. If you want to have an operation
 * supported for caps, add it to Python's Set type first.
 */
#define BINARY_FUNC(name,func) \
static PyObject * \
name (PyObject *py_caps1, PyObject *py_caps2) \
{ \
  GstCaps *caps1, *caps2, *ret; \
  gboolean caps2_is_copy; \
\
  caps1 = pyg_boxed_get (py_caps1, GstCaps); \
  caps2 = pygst_caps_from_pyobject (py_caps2, &caps2_is_copy); \
  if (PyErr_Occurred()) \
    return NULL; \
  ret = func (caps1, caps2); \
  if (caps2 && caps2_is_copy) \
    gst_caps_unref (caps2); \
  return pyg_boxed_new (GST_TYPE_CAPS, ret, FALSE, TRUE); \
}

BINARY_FUNC (pygst_caps_nb_subtract, gst_caps_subtract)
BINARY_FUNC (pygst_caps_nb_and, gst_caps_intersect)
BINARY_FUNC (pygst_caps_nb_or, gst_caps_union)
static GstCaps *
pygst_caps_xor (const GstCaps *caps1, const GstCaps *caps2)
{
  GstCaps *intersect, *_union, *ret;
  intersect = gst_caps_intersect (caps1, caps2);
  _union = gst_caps_union (caps1, caps2);
  ret = gst_caps_subtract (_union, intersect);
  gst_caps_unref (_union);
  gst_caps_unref (intersect);
  gst_caps_do_simplify (ret);
  return ret;
}
BINARY_FUNC (pygst_caps_nb_xor, pygst_caps_xor)

static int
pygst_caps_nb_nonzero (PyObject *py_caps)
{
  GstCaps *caps = pyg_boxed_get (py_caps, GstCaps);

  if (gst_caps_is_empty (caps))
    return 0;
  else
    return 1;
}

static int
pygst_caps_nb_coerce (PyObject **py_caps1, PyObject **py_caps2)
{
  GstCaps *caps1, *caps2 = NULL;
  gboolean caps1_is_copy, caps2_is_copy;
  
  caps1 = pygst_caps_from_pyobject (*py_caps1, &caps1_is_copy);
  if (!caps1)
    goto error;
  caps2 = pygst_caps_from_pyobject (*py_caps2, &caps2_is_copy);
  if (!caps2)
    goto error;
  /* if they're not copies, they're caps already */
  if (caps1_is_copy)
    *py_caps1 = pyg_boxed_new (GST_TYPE_CAPS, caps1, FALSE, TRUE);
  else
    Py_INCREF (*py_caps1);
  if (caps2_is_copy)
    *py_caps2 = pyg_boxed_new (GST_TYPE_CAPS, caps2, FALSE, TRUE);
  else
    Py_INCREF (*py_caps2);
  return 0;

error:
  g_assert (PyErr_Occurred ());
  PyErr_Clear ();
  if (caps1 && !caps1_is_copy)
    gst_caps_unref (caps1);
  return 1;
}

static PyNumberMethods _wrap_gst_caps_tp_as_number = {
	0,			      		/* nb_add */
	pygst_caps_nb_subtract,			/* nb_subtract */
	0,					/* nb_multiply */
	0,	  				/* nb_divide */
	0,					/* nb_remainder */
	0,					/* nb_divmod */
	0,			  		/* nb_power */
	0,					/* nb_negative */
	0,					/* nb_positive */
	0,					/* nb_absolute */
	pygst_caps_nb_nonzero,	      		/* nb_nonzero */
	0,					/* nb_invert */
	0,					/* nb_lshift */
	0,					/* nb_rshift */
	pygst_caps_nb_and,			/* nb_and */
	pygst_caps_nb_xor,			/* nb_xor */
	pygst_caps_nb_or,			/* nb_or */
	pygst_caps_nb_coerce,	      		/* nb_coerce */
	0,					/* nb_int */
	0,					/* nb_long */
	0,					/* nb_float */
	0,					/* nb_oct */
	0,		 			/* nb_hex */
	0,					/* nb_inplace_add */
	0,					/* nb_inplace_subtract */
	0,					/* nb_inplace_multiply */
	0,					/* nb_inplace_divide */
	0,					/* nb_inplace_remainder */
	0,					/* nb_inplace_power */
	0,					/* nb_inplace_lshift */
	0,					/* nb_inplace_rshift */
	0,					/* nb_inplace_and */
	0,					/* nb_inplace_xor */
	0,					/* nb_inplace_or */
	0,					/* nb_floor_divide */
	0,					/* nb_true_divide */
	0,					/* nb_inplace_floor_divide */
	0,					/* nb_inplace_true_divide */
};
#line 1186 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 350 "gstcaps.override"
static Py_ssize_t
pygst_caps_sq_length(PyObject *self)
{
	GstCaps *caps = pyg_boxed_get (self, GstCaps);
	return gst_caps_get_size(caps);
}

static PyObject *
pygst_caps_sq_item(PyObject *self, Py_ssize_t i)
{
	GstCaps *caps = pyg_boxed_get (self, GstCaps);
	GstStructure *structure;
	PyObject *ret;
	
	if (i < 0 || i >= gst_caps_get_size(caps)) {
		PyErr_SetString(PyExc_IndexError, "list index out of range");
		return NULL;
	}
	
	structure = gst_caps_get_structure(caps, i);
				    
	/* pyg_boxed_new handles NULL checking */
	ret = pyg_boxed_new(GST_TYPE_STRUCTURE, 
	                    gst_caps_get_structure(pyg_boxed_get(self, GstCaps), i), 
			    FALSE, FALSE);
	if (ret)
	  pygst_caps_map_add (ret, self);
	return ret;
}

/* FIXME: This syntax sucks */
static PyObject *
pygst_caps_sq_slice(PyObject *self, Py_ssize_t start, Py_ssize_t end)
{
	GstCaps *caps = pyg_boxed_get (self, GstCaps);
	GstCaps *ret = gst_caps_new_empty ();
	int i;

	if (start < 0)
	  start = 0;
	if (end > gst_caps_get_size (caps))
	  end = gst_caps_get_size (caps);

	for (i = start; i < end; i++)
	  gst_caps_append_structure (ret, gst_structure_copy (gst_caps_get_structure (caps, i)));
	
	return pyg_boxed_new(GST_TYPE_CAPS, ret, FALSE, TRUE);
}

static PySequenceMethods _wrap_gst_caps_tp_as_sequence = {
	pygst_caps_sq_length,
        NULL, /* not allowed for sets, use | instead of + */
        NULL, /* doesn't make sense, because it'd still be the same */
	pygst_caps_sq_item,
        pygst_caps_sq_slice, 
	NULL, /* doesn't make sense, you can only append */
	NULL, /* doesn't make sense, you can only append */
        NULL, /* doesn't make sense really, unless you use is_subset */
        NULL, /* not allowed for sets, use | instead of + */
	NULL /* doesn't make sense, because it'd still be the same */
};
#line 1251 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 430 "gstcaps.override"
static PyObject *
_wrap_gst_caps_tp_str(PyGObject *self)
{
    gchar *tmp;
    PyObject *retval;
    
    tmp = gst_caps_to_string((GstCaps*)self->obj);
    retval = PyString_FromString(tmp);
    g_free(tmp);

    return retval;
}

#line 1268 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 138 "gstcaps.override"

static gboolean
pygst_caps_is_true_subset (GstCaps *caps1, GstCaps *caps2)
{
  GstCaps *tmp;
  gboolean ret;
  
  /* order is important here */
  if (gst_caps_is_any (caps1))
    return FALSE;
  if (gst_caps_is_any (caps2))
    return TRUE;
  if (gst_caps_is_empty (caps2))
    return FALSE;
  if (gst_caps_is_empty (caps1))
    return TRUE;

  tmp = gst_caps_subtract (caps1, caps2);
  ret = gst_caps_is_empty (tmp);
  gst_caps_unref (tmp);
  if (!ret)
    return FALSE;
  tmp = gst_caps_subtract (caps2, caps1);
  ret = gst_caps_is_empty (tmp);
  gst_caps_unref (tmp);
  return !ret;
}

static PyObject *
_wrap_gst_caps_tp_richcompare (PyObject *py_caps1, PyObject *py_caps2, int comparison)
{
  GstCaps *caps1, *caps2; 
  gboolean caps2_is_copy;
  PyObject *ret;

  caps1 = pyg_boxed_get (py_caps1, GstCaps);
  caps2 = pygst_caps_from_pyobject (py_caps2, &caps2_is_copy);
  if (PyErr_Occurred()) {
    /* the second arg is not a caps */
    switch (comparison) {
      case Py_EQ:
        PyErr_Clear();
        ret = Py_False;
        Py_INCREF (ret);
        return ret;
      case Py_NE:
        PyErr_Clear();
        ret = Py_True;
        Py_INCREF (ret);
        return ret;
      default:
        return NULL;
    }
  }

  switch (comparison) {
    case Py_LT:
      ret = pygst_caps_is_true_subset (caps1, caps2) ? Py_True : Py_False;
      break;
    case Py_LE:
      ret = gst_caps_is_subset (caps1, caps2) ? Py_True : Py_False;
      break;
    case Py_NE:
      ret = gst_caps_is_equal (caps1, caps2) ? Py_False : Py_True;
      break;
    case Py_EQ:
      ret = gst_caps_is_equal (caps1, caps2) ? Py_True : Py_False;
      break;
    case Py_GE:
      ret = gst_caps_is_subset (caps2, caps1) ? Py_True : Py_False;
      break;
    case Py_GT:
      ret = pygst_caps_is_true_subset (caps2, caps1) ? Py_True : Py_False;
      break;
    default:
      PyErr_SetString (PyExc_RuntimeError, "invalid comparison operation");
      if (caps2 && caps2_is_copy)
	gst_caps_unref (caps2);
      return NULL;
  }
  if (caps2 && caps2_is_copy)
    gst_caps_unref (caps2);
  
  Py_INCREF (ret);
  return ret;
}

#line 1359 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 413 "gstcaps.override"
static void
_wrap_gst_caps_tp_dealloc (PyObject *self)
{
  PyGBoxed *boxed = (PyGBoxed *) self;

  if (boxed->free_on_dealloc && boxed->boxed) {
      pygst_caps_map_modified (self);
    GST_DEBUG ("unreffing caps %" GST_PTR_FORMAT " at %p with refcount %d", 
      boxed->boxed, boxed->boxed, GST_CAPS_REFCOUNT (boxed->boxed));
    gst_caps_unref (boxed->boxed);
  }

  self->ob_type->tp_free((PyObject *)self);
}

#line 1378 "..\\..\\Source\\gst-python\\gst\\gst.c"


PyTypeObject PyGstCaps_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.Caps",                   /* tp_name */
    sizeof(PyGBoxed),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)_wrap_gst_caps_tp_dealloc,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)&_wrap_gst_caps_tp_as_number,     /* tp_as_number */
    (PySequenceMethods*)&_wrap_gst_caps_tp_as_sequence, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)_wrap_gst_caps_tp_str,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)_wrap_gst_caps_tp_richcompare,   /* tp_richcompare */
    0,             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGstCaps_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)gst_caps_getsets,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    0,                 /* tp_dictoffset */
    (initproc)_wrap_gst_caps_new_empty,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GstStructure ----------- */

#line 44 "gststructure.override"
static int
_wrap_gst_structure_new(PyGBoxed *self, PyObject *args, PyObject *kwargs)
{
	static char *kwlist[] = { "name", NULL };
	char *name;

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s:GstStructure.__init__", kwlist, &name))
		return -1;
	
	self->gtype = GST_TYPE_STRUCTURE;
	self->free_on_dealloc = FALSE;

	self->boxed = gst_structure_new(name, NULL);

	if (!self->boxed) {
		PyErr_SetString(PyExc_RuntimeError, "could not create GstStructure object");
		return -1;
	}
	return 0;
}
#line 1451 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_structure_get_name(PyObject *self)
{
    const gchar *ret;

    pyg_begin_allow_threads;
    ret = gst_structure_get_name(pyg_boxed_get(self, GstStructure));
    pyg_end_allow_threads;
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_structure_has_name(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", NULL };
    char *name;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:GstStructure.has_name", kwlist, &name))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_structure_has_name(pyg_boxed_get(self, GstStructure), name);
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_structure_set_name(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", NULL };
    char *name;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:GstStructure.set_name", kwlist, &name))
        return NULL;
    pyg_begin_allow_threads;
    gst_structure_set_name(pyg_boxed_get(self, GstStructure), name);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

#line 66 "gststructure.override"
static PyObject *
_wrap_gst_structure_set_value(PyObject *self, PyObject *args, PyObject *kwargs)
{
	static char *kwlist[] = { "field", "value", "type_name", NULL };
	char *field;
	PyObject *py_value = NULL;
        char *type_name = NULL;
	GType type;
	GValue value = { 0 };

	if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                "sO|s:GstStructure.set_value",
                kwlist, &field, &py_value,
                &type_name)) {
            return NULL;
        }

        if (type_name) {
            if (strcmp (type_name, "char") == 0) {
                type = G_TYPE_CHAR;
            } else if (strcmp (type_name, "uchar") == 0) {
                type = G_TYPE_UCHAR;
            } else if (strcmp (type_name, "boolean") == 0) {
                type = G_TYPE_BOOLEAN;
            } else if (strcmp (type_name, "int") == 0) {
                type = G_TYPE_INT;
            } else if (strcmp (type_name, "uint") == 0) {
                type = G_TYPE_UINT;
            } else if (strcmp (type_name, "long") == 0) {
                type = G_TYPE_LONG;
            } else if (strcmp (type_name, "ulong") == 0) {
                type = G_TYPE_ULONG;
            } else if (strcmp (type_name, "int64") == 0) {
                type = G_TYPE_INT64;
            } else if (strcmp (type_name, "uint64") == 0) {
                type = G_TYPE_UINT64;
            } else if (strcmp (type_name, "float") == 0) {
                type = G_TYPE_FLOAT;
            } else if (strcmp (type_name, "double") == 0) {
                type = G_TYPE_DOUBLE;
            } else if (strcmp (type_name, "string") == 0) {
                type = G_TYPE_STRING;
            } else {
                PyErr_SetString(PyExc_TypeError,
                    "invalid type name");
                return NULL;
            }
        } else {
            /* Let PyGTK guess a GType for the object. */
            type = pyg_type_from_object((PyObject *) py_value->ob_type);
        }

	if (type != G_TYPE_INVALID) {
	    g_value_init(&value, type);
	} else if (!pygst_value_init_for_pyobject(&value, py_value)) {
	    return NULL;
	}

	if (pygst_value_from_pyobject(&value, py_value) != 0) {
	    return NULL;
	}
	gst_structure_set_value(pyg_boxed_get(self, GstStructure), field,
            &value);

	Py_INCREF(Py_None);
	return Py_None;
}
#line 1567 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_structure_remove_field(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "fieldname", NULL };
    char *fieldname;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:GstStructure.remove_field", kwlist, &fieldname))
        return NULL;
    pyg_begin_allow_threads;
    gst_structure_remove_field(pyg_boxed_get(self, GstStructure), fieldname);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_structure_remove_all_fields(PyObject *self)
{
    pyg_begin_allow_threads;
    gst_structure_remove_all_fields(pyg_boxed_get(self, GstStructure));
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_structure_get_field_type(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "fieldname", NULL };
    char *fieldname;
    GType ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:GstStructure.get_field_type", kwlist, &fieldname))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_structure_get_field_type(pyg_boxed_get(self, GstStructure), fieldname);
    pyg_end_allow_threads;
    return pyg_type_wrapper_new(ret);
}

#line 231 "gststructure.override"
static gboolean
pygst_structure_foreach_marshal(GQuark field_id,
				const GValue *value,
				gpointer user_data)
{
	PyGstCustomNotify *cunote = user_data;
	PyObject *py_field, *py_value, *retobj;
	gboolean retval = TRUE;
	PyGILState_STATE state;
	
	g_assert(cunote->func);

	state = pyg_gil_state_ensure();

	py_field = Py_BuildValue("s", g_quark_to_string(field_id));
	py_value = pygst_value_as_pyobject(value, FALSE);
	if (cunote->data)
		retobj = PyEval_CallFunction(cunote->func, "(NNO)",
					     py_field, py_value,
					     cunote->data);
	else
		retobj = PyEval_CallFunction(cunote->func, "(NN)",
					     py_field, py_value);

	if (PyErr_Occurred () || (retobj == NULL) || (retobj == Py_None)) {
		PyErr_Print ();
		retval = FALSE;
	} else if (retobj != Py_None) {
		retval = PyInt_AsLong(retobj);
	}

	Py_XDECREF(retobj);

	pyg_gil_state_release(state);
	
	return retval;
}

static PyObject *
_wrap_gst_structure_foreach (PyGObject *self,
			     PyObject *args,
			     PyObject *kwargs)
{
	static char *kwlist[] = { "foreach_function", "args", NULL };
	PyObject *pyfunc, *pyarg = NULL;
	PyGstCustomNotify cunote;

	if (!PyArg_ParseTupleAndKeywords(args, kwargs,
					 "O|O:GstStructure.foreach",
					 kwlist,
					 &pyfunc, &pyarg)) {
		return NULL;
	}

	if (!PyCallable_Check(pyfunc)) {
		PyErr_SetString(PyExc_TypeError, "foreach_function not callable");
		return NULL;
	}

	cunote.func = pyfunc;
	cunote.data = pyarg;
	gst_structure_foreach(pyg_boxed_get(self, GstStructure),
			      pygst_structure_foreach_marshal,
			      &cunote);

	Py_INCREF(Py_None);
	return Py_None;
}
#line 1679 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_structure_n_fields(PyObject *self)
{
    int ret;

    pyg_begin_allow_threads;
    ret = gst_structure_n_fields(pyg_boxed_get(self, GstStructure));
    pyg_end_allow_threads;
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gst_structure_nth_field_name(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "index", NULL };
    PyObject *py_index = NULL;
    const gchar *ret;
    guint index = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:GstStructure.nth_field_name", kwlist, &py_index))
        return NULL;
    if (py_index) {
        if (PyLong_Check(py_index))
            index = PyLong_AsUnsignedLong(py_index);
        else if (PyInt_Check(py_index))
            index = PyInt_AsLong(py_index);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'index' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    pyg_begin_allow_threads;
    ret = gst_structure_nth_field_name(pyg_boxed_get(self, GstStructure), index);
    pyg_end_allow_threads;
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_structure_has_field(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "fieldname", NULL };
    char *fieldname;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:GstStructure.has_field", kwlist, &fieldname))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_structure_has_field(pyg_boxed_get(self, GstStructure), fieldname);
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_structure_has_field_typed(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "fieldname", "type", NULL };
    char *fieldname;
    PyObject *py_type = NULL;
    GType type;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"sO:GstStructure.has_field_typed", kwlist, &fieldname, &py_type))
        return NULL;
    if ((type = pyg_type_from_object(py_type)) == 0)
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_structure_has_field_typed(pyg_boxed_get(self, GstStructure), fieldname, type);
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

#line 151 "gststructure.override"
static PyObject *
_wrap_gst_structure_keys (PyObject *self)
{
    GstStructure *s;
    int i, n;
    PyObject *ret;

    s = pyg_boxed_get(self, GstStructure);
    n = gst_structure_n_fields(s);
    ret = PyList_New(n);

    for (i = 0; i < n; ++i) {
        const gchar *name = gst_structure_nth_field_name (s, i);
        PyList_SetItem(ret, i, PyString_FromString(name));
    }

    return ret;
}


#line 1779 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_structure_to_string(PyObject *self)
{
    gchar *ret;

    pyg_begin_allow_threads;
    ret = gst_structure_to_string(pyg_boxed_get(self, GstStructure));
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
_wrap_gst_structure_fixate_field_nearest_int(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "field_name", "target", NULL };
    char *field_name;
    int target, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"si:GstStructure.fixate_field_nearest_int", kwlist, &field_name, &target))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_structure_fixate_field_nearest_int(pyg_boxed_get(self, GstStructure), field_name, target);
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_structure_fixate_field_nearest_double(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "field_name", "target", NULL };
    char *field_name;
    double target;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"sd:GstStructure.fixate_field_nearest_double", kwlist, &field_name, &target))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_structure_fixate_field_nearest_double(pyg_boxed_get(self, GstStructure), field_name, target);
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_structure_fixate_field_boolean(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "field_name", "target", NULL };
    char *field_name;
    int target, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"si:GstStructure.fixate_field_boolean", kwlist, &field_name, &target))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_structure_fixate_field_boolean(pyg_boxed_get(self, GstStructure), field_name, target);
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

#line 135 "gststructure.override"
static PyObject*
_wrap_gst_structure_has_key(PyGObject *self, PyObject *args)
{
	gchar *key;
	gboolean has_field;
    
	if (!PyArg_ParseTuple(args, "s:GstStructure.has_key", &key))
		return NULL;

	has_field = gst_structure_has_field((GstStructure*)self->obj, key);
	    
	return PyBool_FromLong(has_field);
}    

#line 1863 "..\\..\\Source\\gst-python\\gst\\gst.c"


static const PyMethodDef _PyGstStructure_methods[] = {
    { "get_name", (PyCFunction)_wrap_gst_structure_get_name, METH_NOARGS,
      NULL },
    { "has_name", (PyCFunction)_wrap_gst_structure_has_name, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_name", (PyCFunction)_wrap_gst_structure_set_name, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_value", (PyCFunction)_wrap_gst_structure_set_value, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "remove_field", (PyCFunction)_wrap_gst_structure_remove_field, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "remove_all_fields", (PyCFunction)_wrap_gst_structure_remove_all_fields, METH_NOARGS,
      NULL },
    { "get_field_type", (PyCFunction)_wrap_gst_structure_get_field_type, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "foreach", (PyCFunction)_wrap_gst_structure_foreach, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "n_fields", (PyCFunction)_wrap_gst_structure_n_fields, METH_NOARGS,
      NULL },
    { "nth_field_name", (PyCFunction)_wrap_gst_structure_nth_field_name, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "has_field", (PyCFunction)_wrap_gst_structure_has_field, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "has_field_typed", (PyCFunction)_wrap_gst_structure_has_field_typed, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "keys", (PyCFunction)_wrap_gst_structure_keys, METH_NOARGS,
      NULL },
    { "to_string", (PyCFunction)_wrap_gst_structure_to_string, METH_NOARGS,
      NULL },
    { "fixate_field_nearest_int", (PyCFunction)_wrap_gst_structure_fixate_field_nearest_int, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "fixate_field_nearest_double", (PyCFunction)_wrap_gst_structure_fixate_field_nearest_double, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "fixate_field_boolean", (PyCFunction)_wrap_gst_structure_fixate_field_boolean, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "has_key", (PyCFunction)_wrap_gst_structure_has_key, METH_VARARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

#line 301 "gststructure.override"
static PyObject *
_wrap_gst_structure_tp_repr (PyGObject *self)
{
	char *buf;
	PyObject *retval;
	
	buf = g_strdup_printf("<GstStructure (%s) at %lx>",
			      gst_structure_get_name((GstStructure*)self->obj),
			      (long)self->obj);

	retval = PyString_FromString(buf);
	g_free(buf);
	return retval;
}
#line 1921 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 173 "gststructure.override"
static Py_ssize_t
_wrap_gst_structure_length(PyObject *self)
{
        PyGObject *gself = (PyGObject *)self;
	return gst_structure_n_fields((GstStructure*)gself->obj);
}

static PyObject *
_wrap_gst_structure_subscript(PyGObject *self, PyObject *py_key)
{
	PyObject *v = NULL;
	const char *field = PyString_AsString(py_key);
	
	if (gst_structure_has_field((GstStructure*)self->obj, field)) {
		const GValue *gvalue;
		gvalue = gst_structure_get_value((GstStructure*)self->obj, field);
		g_assert(gvalue != NULL);
		v = pygst_value_as_pyobject(gvalue, TRUE);
	} else {
		PyErr_SetString(PyExc_KeyError, field);
	}
		
	return v;
}

static int
_wrap_gst_structure_ass_subscript(PyGObject *self,
				  PyObject *py_key,
				  PyObject *py_value)
{
	const char *key;
	GstStructure* structure;

	structure = (GstStructure*)self->obj;
	key = PyString_AsString(py_key);
	if (py_value != NULL) {
            GValue v = { 0, };
            if (!pygst_value_init_for_pyobject (&v, py_value))
                return -1;
	    if (pygst_value_from_pyobject(&v, py_value))
		return -1;
	    gst_structure_set_value(structure, key, &v);
	    g_value_unset(&v);
	} else {
	    gst_structure_remove_field(structure, key);
	}
	
	return 0;
}

static PyMappingMethods _wrap_gst_structure_tp_as_mapping = {
	_wrap_gst_structure_length,         /* mp_length */
	(binaryfunc)_wrap_gst_structure_subscript,   /* mp_subscript */
	(objobjargproc)_wrap_gst_structure_ass_subscript /* mp_ass_subscript */
};

#line 1981 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 334 "gststructure.override"
static void
_wrap_gst_structure_tp_dealloc (PyObject *self)
{
  PyGBoxed *boxed = (PyGBoxed *) self;

  if (boxed->free_on_dealloc && boxed->boxed) {
    gst_structure_free (boxed->boxed);
  } else if (boxed->boxed) {
    pygst_caps_map_remove_structure (self);
  }

  self->ob_type->tp_free((PyObject *)self);
}
#line 1998 "..\\..\\Source\\gst-python\\gst\\gst.c"


PyTypeObject PyGstStructure_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.Structure",                   /* tp_name */
    sizeof(PyGBoxed),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)_wrap_gst_structure_tp_dealloc,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)_wrap_gst_structure_tp_repr,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)&_wrap_gst_structure_tp_as_mapping,   /* tp_as_mapping */
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
    (struct PyMethodDef*)_PyGstStructure_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    0,                 /* tp_dictoffset */
    (initproc)_wrap_gst_structure_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GstTagList ----------- */

static int
_wrap_gst_tag_list_new(PyGBoxed *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,":GstTagList.__init__", kwlist))
        return -1;
    self->gtype = GST_TYPE_TAG_LIST;
    self->free_on_dealloc = FALSE;
    self->boxed = gst_tag_list_new();

    if (!self->boxed) {
        PyErr_SetString(PyExc_RuntimeError, "could not create GstTagList object");
        return -1;
    }
    self->free_on_dealloc = TRUE;
    return 0;
}

static PyObject *
_wrap_gst_tag_list_is_empty(PyObject *self)
{
    int ret;

    pyg_begin_allow_threads;
    ret = gst_tag_list_is_empty(pyg_boxed_get(self, GstTagList));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_tag_list_insert(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "from_", "mode", NULL };
    GstTagList *from = NULL;
    PyObject *py_from, *py_mode = NULL;
    GstTagMergeMode mode;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"OO:GstTagList.insert", kwlist, &py_from, &py_mode))
        return NULL;
    if (pyg_boxed_check(py_from, GST_TYPE_TAG_LIST))
        from = pyg_boxed_get(py_from, GstTagList);
    else {
        PyErr_SetString(PyExc_TypeError, "from should be a GstTagList");
        return NULL;
    }
    if (pyg_enum_get_value(GST_TYPE_TAG_MERGE_MODE, py_mode, (gint *)&mode))
        return NULL;
    pyg_begin_allow_threads;
    gst_tag_list_insert(pyg_boxed_get(self, GstTagList), from, mode);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_tag_list_merge(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "list2", "mode", NULL };
    GstTagList *list2 = NULL, *ret;
    PyObject *py_list2, *py_mode = NULL;
    GstTagMergeMode mode;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"OO:GstTagList.merge", kwlist, &py_list2, &py_mode))
        return NULL;
    if (pyg_boxed_check(py_list2, GST_TYPE_TAG_LIST))
        list2 = pyg_boxed_get(py_list2, GstTagList);
    else {
        PyErr_SetString(PyExc_TypeError, "list2 should be a GstTagList");
        return NULL;
    }
    if (pyg_enum_get_value(GST_TYPE_TAG_MERGE_MODE, py_mode, (gint *)&mode))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_tag_list_merge(pyg_boxed_get(self, GstTagList), list2, mode);
    pyg_end_allow_threads;
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(GST_TYPE_TAG_LIST, ret, FALSE, TRUE);
}

#line 38 "gsttaglist.override"
static void
tag_foreach_func_list (const GstTagList *list,
		       const gchar      *tag,
		       PyObject         *py_list)
{
	int count;
    
	count = gst_tag_list_get_tag_size(GST_TAG_LIST(list), tag);
	if (count == 0)
		PyErr_SetString(PyExc_KeyError, tag);
	else if (count > 0)
		PyList_Append(py_list, PyString_FromString(tag));
}

static PyObject*
_wrap_gst_tag_list_keys(PyGObject *self)
{
	PyObject *dict;

	dict = PyList_New(0);
    
	gst_tag_list_foreach(GST_TAG_LIST(self->obj),
			     (GstTagForeachFunc)tag_foreach_func_list,
			     (gpointer)dict);
	return dict;
}
#line 2158 "..\\..\\Source\\gst-python\\gst\\gst.c"


static const PyMethodDef _PyGstTagList_methods[] = {
    { "is_empty", (PyCFunction)_wrap_gst_tag_list_is_empty, METH_NOARGS,
      NULL },
    { "insert", (PyCFunction)_wrap_gst_tag_list_insert, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "merge", (PyCFunction)_wrap_gst_tag_list_merge, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "keys", (PyCFunction)_wrap_gst_tag_list_keys, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

#line 123 "gsttaglist.override"
static int
_wrap_gst_tag_list_contains(PyGObject *self, PyObject *py_key)
{
	return gst_structure_has_field((GstStructure*)self->obj,
				       PyString_AsString(py_key));
}

static PySequenceMethods _wrap_gst_tag_list_tp_as_sequence = {
	(lenfunc)NULL,
	(binaryfunc)NULL,
	(ssizeargfunc)NULL,
	(ssizeargfunc)NULL,
	(ssizessizeargfunc)NULL,
	(ssizeobjargproc)NULL,
	(ssizessizeobjargproc)NULL,
	(objobjproc)_wrap_gst_tag_list_contains,
	(binaryfunc)NULL,
	(ssizeargfunc)NULL,
};

#line 2194 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 66 "gsttaglist.override"
static Py_ssize_t
_wrap_gst_tag_list_length(PyObject *self)
{
	PyGObject *gself = (PyGObject *)self;
	return gst_structure_n_fields((GstStructure*)gself->obj);
}

static PyObject *
_wrap_gst_tag_list_subscript(PyGObject *self, PyObject *py_key)
{
	PyObject *v = NULL;
	const char *field = PyString_AsString(py_key);
	
	if (gst_structure_has_field((GstStructure*)self->obj, field)) {
		const GValue *gvalue;
		gvalue = gst_structure_get_value((GstStructure*)self->obj, field);
		g_assert(gvalue != NULL);
		v = pygst_value_as_pyobject(gvalue, TRUE);
	} else {
		PyErr_SetString(PyExc_KeyError, field);
	}
		
	return v;
}

static int
_wrap_gst_tag_list_ass_subscript(PyGObject *self,
				  PyObject *py_key,
				  PyObject *py_value)
{
	const char *key;
	GstStructure* structure;

	structure = (GstStructure*)self->obj;
	key = PyString_AsString(py_key);
	if (py_value != NULL) {
            GValue v = { 0, };
            if (!pygst_value_init_for_pyobject (&v, py_value))
                return -1;
	    if (pygst_value_from_pyobject(&v, py_value))
		return -1;
	    gst_structure_set_value(structure, key, &v);
	    g_value_unset(&v);
	} else {
	    gst_structure_remove_field(structure, key);
	}
	
	return 0;
}

static PyMappingMethods _wrap_gst_tag_list_tp_as_mapping = {
	_wrap_gst_tag_list_length,         /* mp_length */
	(binaryfunc)_wrap_gst_tag_list_subscript,   /* mp_subscript */
	(objobjargproc)_wrap_gst_tag_list_ass_subscript /* mp_ass_subscript */
};
#line 2253 "..\\..\\Source\\gst-python\\gst\\gst.c"


PyTypeObject PyGstTagList_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.TagList",                   /* tp_name */
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
    (PySequenceMethods*)&_wrap_gst_tag_list_tp_as_sequence, /* tp_as_sequence */
    (PyMappingMethods*)&_wrap_gst_tag_list_tp_as_mapping,   /* tp_as_mapping */
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
    (struct PyMethodDef*)_PyGstTagList_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    0,                 /* tp_dictoffset */
    (initproc)_wrap_gst_tag_list_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GError ----------- */

#line 1633 "..\\..\\Source\\gst-python\\gst\\gst.override"
static int
_wrap_g_error_new(PyGBoxed *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "domain", "code", "message", NULL };
    int code;
    gchar *message;
    gchar *domain;
    GQuark domainq;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"sis:GError.__init__", kwlist, &domain, &code, &message))
        return -1;
    domainq = g_quark_from_string(domain);

    self->gtype = GST_TYPE_G_ERROR;
    self->free_on_dealloc = FALSE;
    self->boxed = g_error_new(domainq, code, "%s", message);

    if (!self->boxed) {
        PyErr_SetString(PyExc_RuntimeError, "could not create GError object");
        return -1;
    }
    self->free_on_dealloc = TRUE;
    return 0;
}
#line 2330 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 708 "..\\..\\Source\\gst-python\\gst\\gst.override"
static PyObject *
_wrap_gst_g_error__get_domain(PyGObject *self, void *closure)
{
    return PyString_FromString(g_quark_to_string(((GError*)self->obj)->domain));
}
#line 2339 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_g_error__get_code(PyObject *self, void *closure)
{
    int ret;

    ret = pyg_boxed_get(self, GError)->code;
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gst_g_error__get_message(PyObject *self, void *closure)
{
    const gchar *ret;

    ret = pyg_boxed_get(self, GError)->message;
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyGetSetDef gst_g_error_getsets[] = {
    { "domain", (getter)_wrap_gst_g_error__get_domain, (setter)0 },
    { "code", (getter)_wrap_gst_g_error__get_code, (setter)0 },
    { "message", (getter)_wrap_gst_g_error__get_message, (setter)0 },
    { NULL, (getter)0, (setter)0 },
};

#line 715 "..\\..\\Source\\gst-python\\gst\\gst.override"
static PyObject *
_wrap_gst_g_error_tp_str(PyGObject *self)
{
    GError *error = (GError*)self->obj;
    return PyString_FromString(gst_error_get_message (error->domain,
						      error->code));
}
#line 2378 "..\\..\\Source\\gst-python\\gst\\gst.c"


PyTypeObject PyGError_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.GError",                   /* tp_name */
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
    (reprfunc)_wrap_gst_g_error_tp_str,              /* tp_str */
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
    (struct PyMethodDef*)NULL, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)gst_g_error_getsets,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    0,                 /* tp_dictoffset */
    (initproc)_wrap_g_error_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GstDate ----------- */

static int
pygobject_no_constructor(PyObject *self, PyObject *args, PyObject *kwargs)
{
    gchar buf[512];

    g_snprintf(buf, sizeof(buf), "%s is an abstract widget", self->ob_type->tp_name);
    PyErr_SetString(PyExc_NotImplementedError, buf);
    return -1;
}

#line 381 "..\\..\\Source\\gst-python\\gst\\gst.override"
static PyObject *
_wrap_gst_date__get_day(PyGObject *self, void *closure)
{
    return PyInt_FromLong(g_date_get_day((GDate*)self->obj));
}

static int
_wrap_gst_date__set_day(PyGObject *self, PyObject *value, void *closure)
{
    GDate	*date = (GDate *) self->obj;
    
    if (!(PyInt_Check(value)))
	return -1;

    g_date_set_day(date, (int) PyInt_AsLong(value));
    return 0;
}
#line 2458 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 400 "..\\..\\Source\\gst-python\\gst\\gst.override"
static PyObject *
_wrap_gst_date__get_month(PyGObject *self, void *closure)
{
    return PyInt_FromLong(g_date_get_month((GDate*)self->obj));
}
static int
_wrap_gst_date__set_month(PyGObject *self, PyObject *value, void *closure)
{
    GDate	*date = (GDate *) self->obj;
    
    if (!(PyInt_Check(value)))
	return -1;

    g_date_set_month(date, (int) PyInt_AsLong(value));
    return 0;
}
#line 2478 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 418 "..\\..\\Source\\gst-python\\gst\\gst.override"
static PyObject *
_wrap_gst_date__get_year(PyGObject *self, void *closure)
{
    return PyInt_FromLong(g_date_get_year((GDate*)self->obj));
}
static int
_wrap_gst_date__set_year(PyGObject *self, PyObject *value, void *closure)
{
    GDate	*date = (GDate *) self->obj;
    
    if (!(PyInt_Check(value)))
	return -1;

    g_date_set_year(date, (int) PyInt_AsLong(value));
    return 0;
}
#line 2498 "..\\..\\Source\\gst-python\\gst\\gst.c"


static const PyGetSetDef gst_date_getsets[] = {
    { "day", (getter)_wrap_gst_date__get_day, (setter)_wrap_gst_date__set_day },
    { "month", (getter)_wrap_gst_date__get_month, (setter)_wrap_gst_date__set_month },
    { "year", (getter)_wrap_gst_date__get_year, (setter)_wrap_gst_date__set_year },
    { NULL, (getter)0, (setter)0 },
};

#line 436 "..\\..\\Source\\gst-python\\gst\\gst.override"
static PyObject *
_wrap_gst_date_tp_repr(PyGObject *self)
{
    GDate	*date = (GDate *) self->obj;

    return PyString_FromFormat ("<GstDate: %2d/%2d/%4d>",
				g_date_get_day(date),
				g_date_get_month(date),
				g_date_get_year(date));
}
#line 2519 "..\\..\\Source\\gst-python\\gst\\gst.c"


PyTypeObject PyGstDate_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.Date",                   /* tp_name */
    sizeof(PyGBoxed),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)_wrap_gst_date_tp_repr,             /* tp_repr */
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
    (struct PyMethodDef*)NULL, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)gst_date_getsets,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    0,                 /* tp_dictoffset */
    (initproc)pygobject_no_constructor,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GstSegment ----------- */

static int
_wrap_gst_segment_new(PyGBoxed *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,":GstSegment.__init__", kwlist))
        return -1;
    self->gtype = GST_TYPE_SEGMENT;
    self->free_on_dealloc = FALSE;
    self->boxed = gst_segment_new();

    if (!self->boxed) {
        PyErr_SetString(PyExc_RuntimeError, "could not create GstSegment object");
        return -1;
    }
    self->free_on_dealloc = TRUE;
    return 0;
}

static PyObject *
_wrap_gst_segment_init(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "format", NULL };
    PyObject *py_format = NULL;
    GstFormat format;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:GstSegment.init", kwlist, &py_format))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_FORMAT, py_format, (gint *)&format))
        return NULL;
    pyg_begin_allow_threads;
    gst_segment_init(pyg_boxed_get(self, GstSegment), format);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_segment_set_duration(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "format", "duration", NULL };
    PyObject *py_format = NULL;
    GstFormat format;
    gint64 duration;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"OL:GstSegment.set_duration", kwlist, &py_format, &duration))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_FORMAT, py_format, (gint *)&format))
        return NULL;
    pyg_begin_allow_threads;
    gst_segment_set_duration(pyg_boxed_get(self, GstSegment), format, duration);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_segment_set_last_stop(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "format", "position", NULL };
    PyObject *py_format = NULL;
    GstFormat format;
    gint64 position;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"OL:GstSegment.set_last_stop", kwlist, &py_format, &position))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_FORMAT, py_format, (gint *)&format))
        return NULL;
    pyg_begin_allow_threads;
    gst_segment_set_last_stop(pyg_boxed_get(self, GstSegment), format, position);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

#line 1368 "..\\..\\Source\\gst-python\\gst\\gst.override"
static PyObject *
_wrap_gst_segment_set_seek (PyObject * self, PyObject * args, PyObject * kwargs)
{
    static char *kwlist[] = { "rate", "format", "flags", "start_type", "start",
			      "stop_type", "stop", NULL };
    GstSeekType start_type, stop_type;
    PyObject *py_format = NULL, *py_flags = NULL, *py_start_type = NULL;
    PyObject *py_stop_type = NULL, *py_ret;
    double rate;
    GstFormat format;
    gint64 start, stop;
    GstSeekFlags flags;
    gboolean update = FALSE;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"dOOOLOL:GstSegment.set_seek",
				     kwlist, &rate, &py_format, &py_flags,
				     &py_start_type, &start, &py_stop_type,
				     &stop))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_FORMAT, py_format, (gint *)&format))
        return NULL;
    if (pyg_flags_get_value(GST_TYPE_SEEK_FLAGS, py_flags, (gint *)&flags))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_SEEK_TYPE, py_start_type, (gint *)&start_type))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_SEEK_TYPE, py_stop_type, (gint *)&stop_type))
        return NULL;
    pyg_begin_allow_threads;
    gst_segment_set_seek(pyg_boxed_get(self, GstSegment), rate, format, flags,
			 start_type, start, stop_type, stop, &update);
    pyg_end_allow_threads;
    py_ret = PyBool_FromLong(update);
    return py_ret;
}
#line 2681 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_segment_set_newsegment(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "update", "rate", "format", "start", "stop", "time", NULL };
    int update;
    double rate;
    GstFormat format;
    gint64 start, stop, time;
    PyObject *py_format = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"idOLLL:GstSegment.set_newsegment", kwlist, &update, &rate, &py_format, &start, &stop, &time))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_FORMAT, py_format, (gint *)&format))
        return NULL;
    pyg_begin_allow_threads;
    gst_segment_set_newsegment(pyg_boxed_get(self, GstSegment), update, rate, format, start, stop, time);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_segment_set_newsegment_full(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "update", "rate", "applied_rate", "format", "start", "stop", "time", NULL };
    int update;
    double rate, applied_rate;
    GstFormat format;
    gint64 start, stop, time;
    PyObject *py_format = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"iddOLLL:GstSegment.set_newsegment_full", kwlist, &update, &rate, &applied_rate, &py_format, &start, &stop, &time))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_FORMAT, py_format, (gint *)&format))
        return NULL;
    pyg_begin_allow_threads;
    gst_segment_set_newsegment_full(pyg_boxed_get(self, GstSegment), update, rate, applied_rate, format, start, stop, time);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_segment_to_stream_time(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "format", "position", NULL };
    PyObject *py_format = NULL;
    GstFormat format;
    gint64 position, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"OL:GstSegment.to_stream_time", kwlist, &py_format, &position))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_FORMAT, py_format, (gint *)&format))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_segment_to_stream_time(pyg_boxed_get(self, GstSegment), format, position);
    pyg_end_allow_threads;
    return PyLong_FromLongLong(ret);
}

static PyObject *
_wrap_gst_segment_to_running_time(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "format", "position", NULL };
    PyObject *py_format = NULL;
    GstFormat format;
    gint64 position, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"OL:GstSegment.to_running_time", kwlist, &py_format, &position))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_FORMAT, py_format, (gint *)&format))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_segment_to_running_time(pyg_boxed_get(self, GstSegment), format, position);
    pyg_end_allow_threads;
    return PyLong_FromLongLong(ret);
}

#line 1404 "..\\..\\Source\\gst-python\\gst\\gst.override"
static PyObject *
_wrap_gst_segment_clip (PyObject * self, PyObject * args, PyObject * kwargs)
{
    static char *kwlist[] = { "format", "start", "stop", NULL};
    GstFormat format;
    gint64 start, stop;
    gint64 cstart = -1;
    gint64 cstop = -1;
    gboolean ret;
    PyObject *py_ret, *py_format;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OLL:GstSegment.clip",
				     kwlist, &py_format, &start, &stop))
	return NULL;
    if (pyg_enum_get_value(GST_TYPE_FORMAT, py_format, (gint *)&format))
	return NULL;
    pyg_begin_allow_threads;
    ret = gst_segment_clip (pyg_boxed_get(self, GstSegment), format, start, stop,
			    &cstart, &cstop);
    pyg_end_allow_threads;

    /* Returns gboolean ret, gint64 clip_start, gint64 clip_stop */
    py_ret = PyList_New(3);
    PyList_SetItem(py_ret, 0, PyBool_FromLong(ret));
    PyList_SetItem(py_ret, 1, PyLong_FromLongLong(cstart));
    PyList_SetItem(py_ret, 2, PyLong_FromLongLong(cstop));

    return py_ret;
}

#line 2793 "..\\..\\Source\\gst-python\\gst\\gst.c"


static const PyMethodDef _PyGstSegment_methods[] = {
    { "init", (PyCFunction)_wrap_gst_segment_init, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_duration", (PyCFunction)_wrap_gst_segment_set_duration, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_last_stop", (PyCFunction)_wrap_gst_segment_set_last_stop, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_seek", (PyCFunction)_wrap_gst_segment_set_seek, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_newsegment", (PyCFunction)_wrap_gst_segment_set_newsegment, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_newsegment_full", (PyCFunction)_wrap_gst_segment_set_newsegment_full, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "to_stream_time", (PyCFunction)_wrap_gst_segment_to_stream_time, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "to_running_time", (PyCFunction)_wrap_gst_segment_to_running_time, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "clip", (PyCFunction)_wrap_gst_segment_clip, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

static PyObject *
_wrap_gst_segment__get_rate(PyObject *self, void *closure)
{
    double ret;

    ret = pyg_boxed_get(self, GstSegment)->rate;
    return PyFloat_FromDouble(ret);
}

static PyObject *
_wrap_gst_segment__get_abs_rate(PyObject *self, void *closure)
{
    double ret;

    ret = pyg_boxed_get(self, GstSegment)->abs_rate;
    return PyFloat_FromDouble(ret);
}

static PyObject *
_wrap_gst_segment__get_format(PyObject *self, void *closure)
{
    gint ret;

    ret = pyg_boxed_get(self, GstSegment)->format;
    return pyg_enum_from_gtype(GST_TYPE_FORMAT, ret);
}

static PyObject *
_wrap_gst_segment__get_flags(PyObject *self, void *closure)
{
    guint ret;

    ret = pyg_boxed_get(self, GstSegment)->flags;
    return pyg_flags_from_gtype(GST_TYPE_SEEK_FLAGS, ret);
}

static PyObject *
_wrap_gst_segment__get_start(PyObject *self, void *closure)
{
    gint64 ret;

    ret = pyg_boxed_get(self, GstSegment)->start;
    return PyLong_FromLongLong(ret);
}

static PyObject *
_wrap_gst_segment__get_stop(PyObject *self, void *closure)
{
    gint64 ret;

    ret = pyg_boxed_get(self, GstSegment)->stop;
    return PyLong_FromLongLong(ret);
}

static PyObject *
_wrap_gst_segment__get_time(PyObject *self, void *closure)
{
    gint64 ret;

    ret = pyg_boxed_get(self, GstSegment)->time;
    return PyLong_FromLongLong(ret);
}

static PyObject *
_wrap_gst_segment__get_accum(PyObject *self, void *closure)
{
    gint64 ret;

    ret = pyg_boxed_get(self, GstSegment)->accum;
    return PyLong_FromLongLong(ret);
}

static PyObject *
_wrap_gst_segment__get_last_stop(PyObject *self, void *closure)
{
    gint64 ret;

    ret = pyg_boxed_get(self, GstSegment)->last_stop;
    return PyLong_FromLongLong(ret);
}

static PyObject *
_wrap_gst_segment__get_duration(PyObject *self, void *closure)
{
    gint64 ret;

    ret = pyg_boxed_get(self, GstSegment)->duration;
    return PyLong_FromLongLong(ret);
}

static const PyGetSetDef gst_segment_getsets[] = {
    { "rate", (getter)_wrap_gst_segment__get_rate, (setter)0 },
    { "abs_rate", (getter)_wrap_gst_segment__get_abs_rate, (setter)0 },
    { "format", (getter)_wrap_gst_segment__get_format, (setter)0 },
    { "flags", (getter)_wrap_gst_segment__get_flags, (setter)0 },
    { "start", (getter)_wrap_gst_segment__get_start, (setter)0 },
    { "stop", (getter)_wrap_gst_segment__get_stop, (setter)0 },
    { "time", (getter)_wrap_gst_segment__get_time, (setter)0 },
    { "accum", (getter)_wrap_gst_segment__get_accum, (setter)0 },
    { "last_stop", (getter)_wrap_gst_segment__get_last_stop, (setter)0 },
    { "duration", (getter)_wrap_gst_segment__get_duration, (setter)0 },
    { NULL, (getter)0, (setter)0 },
};

PyTypeObject PyGstSegment_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.Segment",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGstSegment_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)gst_segment_getsets,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    0,                 /* tp_dictoffset */
    (initproc)_wrap_gst_segment_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GstIndexEntry ----------- */

static PyObject *
_wrap_GST_INDEX_ASSOC_FORMAT(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "i", NULL };
    PyObject *py_i = NULL;
    gint ret;
    guint i = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:GstIndexEntry.ASSOC_FORMAT", kwlist, &py_i))
        return NULL;
    if (py_i) {
        if (PyLong_Check(py_i))
            i = PyLong_AsUnsignedLong(py_i);
        else if (PyInt_Check(py_i))
            i = PyInt_AsLong(py_i);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'i' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    pyg_begin_allow_threads;
    ret = GST_INDEX_ASSOC_FORMAT(pyg_boxed_get(self, GstIndexEntry), i);
    pyg_end_allow_threads;
    return pyg_enum_from_gtype(GST_TYPE_FORMAT, ret);
}

static PyObject *
_wrap_GST_INDEX_ASSOC_VALUE(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "i", NULL };
    PyObject *py_i = NULL;
    gint64 ret;
    guint i = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:GstIndexEntry.ASSOC_VALUE", kwlist, &py_i))
        return NULL;
    if (py_i) {
        if (PyLong_Check(py_i))
            i = PyLong_AsUnsignedLong(py_i);
        else if (PyInt_Check(py_i))
            i = PyInt_AsLong(py_i);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'i' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    pyg_begin_allow_threads;
    ret = GST_INDEX_ASSOC_VALUE(pyg_boxed_get(self, GstIndexEntry), i);
    pyg_end_allow_threads;
    return PyLong_FromLongLong(ret);
}

static const PyMethodDef _PyGstIndexEntry_methods[] = {
    { "ASSOC_FORMAT", (PyCFunction)_wrap_GST_INDEX_ASSOC_FORMAT, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "ASSOC_VALUE", (PyCFunction)_wrap_GST_INDEX_ASSOC_VALUE, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

static PyObject *
_wrap_gst_index_entry__get_type(PyObject *self, void *closure)
{
    gint ret;

    ret = pyg_boxed_get(self, GstIndexEntry)->type;
    return pyg_enum_from_gtype(GST_TYPE_INDEX_ENTRY_TYPE, ret);
}

static PyObject *
_wrap_gst_index_entry__get_id(PyObject *self, void *closure)
{
    int ret;

    ret = pyg_boxed_get(self, GstIndexEntry)->id;
    return PyInt_FromLong(ret);
}

#line 1659 "..\\..\\Source\\gst-python\\gst\\gst.override"
static PyObject *
_wrap_gst_index_entry__get_NASSOCS(PyObject *self, void *closure)
{
    GstIndexEntry *entry;

    g_assert (self);
    entry = (GstIndexEntry*) pyg_boxed_get(self, GstIndexEntry);
    g_assert (entry);

    if (entry->type != GST_INDEX_ENTRY_ASSOCIATION) {
	PyErr_SetString(PyExc_RuntimeError, "IndexEntry is not an AssociationEntry");
	return NULL;
    }
    return PyInt_FromLong(GST_INDEX_NASSOCS(entry));
}
#line 3065 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 1676 "..\\..\\Source\\gst-python\\gst\\gst.override"
static PyObject *
_wrap_gst_index_entry__get_ASSOC_FLAGS(PyObject *self, void *closure)
{
    GstIndexEntry *entry;

    g_assert (self);
    entry = (GstIndexEntry*) pyg_boxed_get(self, GstIndexEntry);
    g_assert (entry);

    if (entry->type != GST_INDEX_ENTRY_ASSOCIATION) {
	PyErr_SetString(PyExc_RuntimeError, "IndexEntry is not an AssociationEntry");
	return NULL;
    }
    return pyg_flags_from_gtype(GST_TYPE_ASSOC_FLAGS,
				GST_INDEX_ASSOC_FLAGS (entry));
}
#line 3085 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 1694 "..\\..\\Source\\gst-python\\gst\\gst.override"
static PyObject *
_wrap_gst_index_entry__get_ID_DESCRIPTION(PyObject *self, void *closure)
{
    GstIndexEntry *entry;

    g_assert (self);
    entry = (GstIndexEntry*) pyg_boxed_get(self, GstIndexEntry);
    g_assert (entry);

    if (entry->type != GST_INDEX_ENTRY_ID) {
	PyErr_SetString(PyExc_RuntimeError, "IndexEntry is not an ID Entry");
	return NULL;
    }
    if (GST_INDEX_ID_DESCRIPTION (entry))
	return PyString_FromString(GST_INDEX_ID_DESCRIPTION (entry));
    Py_INCREF(Py_None);
    return Py_None;
}
#line 3107 "..\\..\\Source\\gst-python\\gst\\gst.c"


static const PyGetSetDef gst_index_entry_getsets[] = {
    { "type", (getter)_wrap_gst_index_entry__get_type, (setter)0 },
    { "id", (getter)_wrap_gst_index_entry__get_id, (setter)0 },
    { "NASSOCS", (getter)_wrap_gst_index_entry__get_NASSOCS, (setter)0 },
    { "ASSOC_FLAGS", (getter)_wrap_gst_index_entry__get_ASSOC_FLAGS, (setter)0 },
    { "ID_DESCRIPTION", (getter)_wrap_gst_index_entry__get_ID_DESCRIPTION, (setter)0 },
    { NULL, (getter)0, (setter)0 },
};

PyTypeObject PyGstIndexEntry_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.IndexEntry",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGstIndexEntry_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)gst_index_entry_getsets,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    0,                 /* tp_dictoffset */
    (initproc)pygobject_no_constructor,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GstStaticPadTemplate ----------- */

static PyObject *
_wrap_gst_static_pad_template_get(PyObject *self)
{
    PyObject *py_ret;
    GstPadTemplate *ret;

    ret = gst_static_pad_template_get(pyg_pointer_get(self, GstStaticPadTemplate));
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref((GObject *)ret);
    return py_ret;
}

#line 1659 "gstpad.override"
static PyObject *
_wrap_gst_static_pad_template_get_caps(PyObject *self)
{
    GstCaps *ret;

    ret = gst_static_pad_template_get_caps(pyg_pointer_get(self, GstStaticPadTemplate));
    /* We take a copy of the caps so they don't disappear */
    ret = gst_caps_copy(ret);
    return pyg_boxed_new (GST_TYPE_CAPS, ret, FALSE, TRUE);
}
#line 3192 "..\\..\\Source\\gst-python\\gst\\gst.c"


static const PyMethodDef _PyGstStaticPadTemplate_methods[] = {
    { "get", (PyCFunction)_wrap_gst_static_pad_template_get, METH_NOARGS,
      NULL },
    { "get_caps", (PyCFunction)_wrap_gst_static_pad_template_get_caps, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

static PyObject *
_wrap_gst_static_pad_template__get_name_template(PyObject *self, void *closure)
{
    const gchar *ret;

    ret = pyg_pointer_get(self, GstStaticPadTemplate)->name_template;
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_static_pad_template__get_direction(PyObject *self, void *closure)
{
    gint ret;

    ret = pyg_pointer_get(self, GstStaticPadTemplate)->direction;
    return pyg_enum_from_gtype(GST_TYPE_PAD_DIRECTION, ret);
}

static PyObject *
_wrap_gst_static_pad_template__get_presence(PyObject *self, void *closure)
{
    gint ret;

    ret = pyg_pointer_get(self, GstStaticPadTemplate)->presence;
    return pyg_enum_from_gtype(GST_TYPE_PAD_PRESENCE, ret);
}

#line 1566 "gstpad.override"
static PyObject *
_wrap_gst_static_pad_template__get_static_caps(PyObject *self, void *closure)
{
    GstStaticPadTemplate *templ;


    templ = pyg_pointer_get(self, GstStaticPadTemplate);
    return pyg_pointer_new(GST_TYPE_STATIC_CAPS, &(templ->static_caps));
}
#line 3243 "..\\..\\Source\\gst-python\\gst\\gst.c"


static const PyGetSetDef gst_static_pad_template_getsets[] = {
    { "name_template", (getter)_wrap_gst_static_pad_template__get_name_template, (setter)0 },
    { "direction", (getter)_wrap_gst_static_pad_template__get_direction, (setter)0 },
    { "presence", (getter)_wrap_gst_static_pad_template__get_presence, (setter)0 },
    { "static_caps", (getter)_wrap_gst_static_pad_template__get_static_caps, (setter)0 },
    { NULL, (getter)0, (setter)0 },
};

PyTypeObject PyGstStaticPadTemplate_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.StaticPadTemplate",                   /* tp_name */
    sizeof(PyGPointer),          /* tp_basicsize */
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
    (struct PyMethodDef*)_PyGstStaticPadTemplate_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)gst_static_pad_template_getsets,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    0,                 /* tp_dictoffset */
    (initproc)pygobject_no_constructor,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GstStaticCaps ----------- */

static PyObject *
_wrap_gst_static_caps_get(PyObject *self)
{
    GstCaps *ret;

    ret = gst_static_caps_get(pyg_pointer_get(self, GstStaticCaps));
    return pyg_boxed_new (GST_TYPE_CAPS, ret, FALSE, TRUE);
}

static const PyMethodDef _PyGstStaticCaps_methods[] = {
    { "get", (PyCFunction)_wrap_gst_static_caps_get, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

static PyObject *
_wrap_gst_static_caps__get_string(PyObject *self, void *closure)
{
    const gchar *ret;

    ret = pyg_pointer_get(self, GstStaticCaps)->string;
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyGetSetDef gst_static_caps_getsets[] = {
    { "string", (getter)_wrap_gst_static_caps__get_string, (setter)0 },
    { NULL, (getter)0, (setter)0 },
};

PyTypeObject PyGstStaticCaps_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.StaticCaps",                   /* tp_name */
    sizeof(PyGPointer),          /* tp_basicsize */
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
    (struct PyMethodDef*)_PyGstStaticCaps_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)gst_static_caps_getsets,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    0,                 /* tp_dictoffset */
    (initproc)pygobject_no_constructor,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GstTypeFind ----------- */

#line 1341 "..\\..\\Source\\gst-python\\gst\\gst.override"
static PyObject *
_wrap_gst_type_find_peek (PyObject * self, PyObject * args, PyObject * kwargs)
{
    static char *kwlist[] = { "offset", "size", NULL };
    gint64 offset;
    guint size;
    GstTypeFind *typefind;  
    guint8 *data;
    
    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"LI:GstTypeFind.peek",
            kwlist, &offset, &size))
        return NULL;
    
    typefind = pyg_pointer_get(self, GstTypeFind);
    pyg_begin_allow_threads;
    data = gst_type_find_peek(typefind, offset, size);
    pyg_end_allow_threads;
 
    if (data == NULL)
        /* return the empty string */
        return PyString_FromStringAndSize(NULL, 0);

    return PyString_FromStringAndSize((char *) data, size);
}

#line 3410 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_type_find_suggest(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "probability", "caps", NULL };
    PyObject *py_probability = NULL, *py_caps;
    GstCaps *caps;
    guint probability = 0;
    gboolean caps_is_copy;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"OO:GstTypeFind.suggest", kwlist, &py_probability, &py_caps))
        return NULL;
    if (py_probability) {
        if (PyLong_Check(py_probability))
            probability = PyLong_AsUnsignedLong(py_probability);
        else if (PyInt_Check(py_probability))
            probability = PyInt_AsLong(py_probability);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'probability' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    caps = pygst_caps_from_pyobject (py_caps, &caps_is_copy);
    if (PyErr_Occurred())
      return NULL;
    gst_type_find_suggest(pyg_pointer_get(self, GstTypeFind), probability, caps);
    if (caps && caps_is_copy)
        gst_caps_unref (caps);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_type_find_get_length(PyObject *self)
{
    guint64 ret;

    ret = gst_type_find_get_length(pyg_pointer_get(self, GstTypeFind));
    return PyLong_FromUnsignedLongLong(ret);
}

static const PyMethodDef _PyGstTypeFind_methods[] = {
    { "peek", (PyCFunction)_wrap_gst_type_find_peek, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "suggest", (PyCFunction)_wrap_gst_type_find_suggest, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_length", (PyCFunction)_wrap_gst_type_find_get_length, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject PyGstTypeFind_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.TypeFind",                   /* tp_name */
    sizeof(PyGPointer),          /* tp_basicsize */
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
    (struct PyMethodDef*)_PyGstTypeFind_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    0,                 /* tp_dictoffset */
    (initproc)pygobject_no_constructor,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GstObject ----------- */

static PyObject *
_wrap_GST_OBJECT_FLAG_SET(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "flags", NULL };
    PyObject *py_flags = NULL;
    GstObjectFlags flags;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:GstObject.set_flag", kwlist, &py_flags))
        return NULL;
    if (pyg_flags_get_value(GST_TYPE_OBJECT_FLAGS, py_flags, (gint *)&flags))
        return NULL;
    pyg_begin_allow_threads;
    GST_OBJECT_FLAG_SET(GST_OBJECT(self->obj), flags);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GST_OBJECT_FLAG_UNSET(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "flag", NULL };
    PyObject *py_flag = NULL;
    GstObjectFlags flag;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:GstObject.unset_flag", kwlist, &py_flag))
        return NULL;
    if (pyg_flags_get_value(GST_TYPE_OBJECT_FLAGS, py_flag, (gint *)&flag))
        return NULL;
    pyg_begin_allow_threads;
    GST_OBJECT_FLAG_UNSET(GST_OBJECT(self->obj), flag);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

#line 822 "..\\..\\Source\\gst-python\\gst\\gst.override"
static PyObject *
_wrap_gst_object_log (PyObject *whatever, PyObject *string)
{
    return pygst_debug_log (whatever, string, GST_LEVEL_LOG, TRUE);
}
#line 3554 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 829 "..\\..\\Source\\gst-python\\gst\\gst.override"
static PyObject *
_wrap_gst_object_debug (PyObject *whatever, PyObject *string)
{
    return pygst_debug_log (whatever, string, GST_LEVEL_DEBUG, TRUE);
}
#line 3563 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 836 "..\\..\\Source\\gst-python\\gst\\gst.override"
static PyObject *
_wrap_gst_object_info (PyObject *whatever, PyObject *string)
{
    return pygst_debug_log (whatever, string, GST_LEVEL_INFO, TRUE);
}
#line 3572 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 843 "..\\..\\Source\\gst-python\\gst\\gst.override"
static PyObject *
_wrap_gst_object_warning (PyObject *whatever, PyObject *string)
{
    return pygst_debug_log (whatever, string, GST_LEVEL_WARNING, TRUE);
}
#line 3581 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 850 "..\\..\\Source\\gst-python\\gst\\gst.override"
static PyObject *
_wrap_gst_object_error (PyObject *whatever, PyObject *string)
{
    return pygst_debug_log (whatever, string, GST_LEVEL_ERROR, TRUE);
}
#line 3590 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 857 "..\\..\\Source\\gst-python\\gst\\gst.override"
static PyObject *
_wrap_gst_object_fixme (PyObject *whatever, PyObject *string)
{
    return pygst_debug_log (whatever, string, GST_LEVEL_FIXME, TRUE);
}
#line 3599 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 864 "..\\..\\Source\\gst-python\\gst\\gst.override"
static PyObject *
_wrap_gst_object_memdump (PyObject *whatever, PyObject *string)
{
    return pygst_debug_log (whatever, string, GST_LEVEL_MEMDUMP, TRUE);
}
#line 3608 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_object_set_name(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", NULL };
    char *name;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:GstObject.set_name", kwlist, &name))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_object_set_name(GST_OBJECT(self->obj), name);
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_object_get_name(PyGObject *self)
{
    gchar *ret;

    pyg_begin_allow_threads;
    ret = gst_object_get_name(GST_OBJECT(self->obj));
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
_wrap_gst_object_set_name_prefix(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name_prefix", NULL };
    char *name_prefix;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:GstObject.set_name_prefix", kwlist, &name_prefix))
        return NULL;
    pyg_begin_allow_threads;
    gst_object_set_name_prefix(GST_OBJECT(self->obj), name_prefix);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_object_get_name_prefix(PyGObject *self)
{
    gchar *ret;

    pyg_begin_allow_threads;
    ret = gst_object_get_name_prefix(GST_OBJECT(self->obj));
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
_wrap_gst_object_set_parent(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "parent", NULL };
    PyGObject *parent;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstObject.set_parent", kwlist, &PyGstObject_Type, &parent))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_object_set_parent(GST_OBJECT(self->obj), GST_OBJECT(parent->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_object_get_parent(PyGObject *self)
{
    PyObject *py_ret;
    GstObject *ret;

    pyg_begin_allow_threads;
    ret = gst_object_get_parent(GST_OBJECT(self->obj));
    pyg_end_allow_threads;
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref((GObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_object_unparent(PyGObject *self)
{
    pyg_begin_allow_threads;
    gst_object_unparent(GST_OBJECT(self->obj));
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_object_has_ancestor(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "ancestor", NULL };
    PyGObject *ancestor;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstObject.has_ancestor", kwlist, &PyGstObject_Type, &ancestor))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_object_has_ancestor(GST_OBJECT(self->obj), GST_OBJECT(ancestor->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

#line 80 "gstobject.override"

/*
 * REMOVE THE FOLLOWING CODE, once pygobject has fixed the issue of not
 * releasing the GIL when calling g_object_set_property.
 * 
 * See bug #395048 : set_property() doesn't release the GIL
 **/

static gboolean
set_property_from_pspec(GObject *obj,
			char *attr_name,
			GParamSpec *pspec,
			PyObject *pvalue)
{
    GValue value = { 0, };

    if (pspec->flags & G_PARAM_CONSTRUCT_ONLY) {
	PyErr_Format(PyExc_TypeError,
		     "property '%s' can only be set in constructor",
		     attr_name);
	return FALSE;
    }	

    if (!(pspec->flags & G_PARAM_WRITABLE)) {
	PyErr_Format(PyExc_TypeError,
		     "property '%s' is not writable", attr_name);
	return FALSE;
    }	

    g_value_init(&value, G_PARAM_SPEC_VALUE_TYPE(pspec));
/* FIXME: bug in pygtk 2.8 series; see
http://svn.gnome.org/viewcvs/pygobject/trunk/gobject/pygobject.h?rev=566&r1=564&r2=566 */

#ifndef PYGOBJECT_2_12
#undef pyg_param_gvalue_from_pyobject
#define pyg_param_gvalue_from_pyobject (_PyGObject_API->gvalue_from_param_pyobject)
#endif

    if (pyg_param_gvalue_from_pyobject(&value, pvalue, pspec) < 0) {
	PyErr_SetString(PyExc_TypeError,
			"could not convert argument to correct param type");
	return FALSE;
    }

    pyg_begin_allow_threads;
    g_object_set_property(obj, attr_name, &value);
    pyg_end_allow_threads;

    g_value_unset(&value);
    
    return TRUE;
}

static PyObject *
_wrap_gst_object_set_property(PyGObject *self, PyObject *args)
{
    gchar *param_name;
    GParamSpec *pspec;
    PyObject *pvalue;

    if (!PyArg_ParseTuple(args, "sO:gst.Object.set_property", &param_name,
			  &pvalue))
      return NULL;
    
    if (!GST_IS_OBJECT (self->obj)) {
      PyErr_Format (PyExc_TypeError,
		    "object at %p of type %s is not initialized",
		    self, self->ob_type->tp_name);
      return NULL;
    }
    
    pspec = g_object_class_find_property(G_OBJECT_GET_CLASS(self->obj),
					 param_name);
    if (!pspec) {
	PyErr_Format(PyExc_TypeError,
		     "object of type `%s' does not have property `%s'",
		     g_type_name(G_OBJECT_TYPE(self->obj)), param_name);
	return NULL;
    }
    
    if (!set_property_from_pspec(self->obj, param_name, pspec, pvalue))
	return NULL;
    
    Py_INCREF(Py_None);
    return Py_None;
}
#line 3820 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_object_get_path_string(PyGObject *self)
{
    gchar *ret;

    pyg_begin_allow_threads;
    ret = gst_object_get_path_string(GST_OBJECT(self->obj));
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
_wrap_gst_object_default_error(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "error", "debug", NULL };
    PyObject *py_error;
    char *debug;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"Os:GstObject.default_error", kwlist, &py_error, &debug))
        return NULL;
    if (pyg_boxed_check(py_error, GST_TYPE_G_ERROR))
        error = pyg_boxed_get(py_error, GError);
    else {
        PyErr_SetString(PyExc_TypeError, "error should be a GError");
        return NULL;
    }
    pyg_begin_allow_threads;
    gst_object_default_error(GST_OBJECT(self->obj), error, debug);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGstObject_methods[] = {
    { "set_flag", (PyCFunction)_wrap_GST_OBJECT_FLAG_SET, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "unset_flag", (PyCFunction)_wrap_GST_OBJECT_FLAG_UNSET, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "log", (PyCFunction)_wrap_gst_object_log, METH_VARARGS,
      NULL },
    { "debug", (PyCFunction)_wrap_gst_object_debug, METH_VARARGS,
      NULL },
    { "info", (PyCFunction)_wrap_gst_object_info, METH_VARARGS,
      NULL },
    { "warning", (PyCFunction)_wrap_gst_object_warning, METH_VARARGS,
      NULL },
    { "error", (PyCFunction)_wrap_gst_object_error, METH_VARARGS,
      NULL },
    { "fixme", (PyCFunction)_wrap_gst_object_fixme, METH_VARARGS,
      NULL },
    { "memdump", (PyCFunction)_wrap_gst_object_memdump, METH_VARARGS,
      NULL },
    { "set_name", (PyCFunction)_wrap_gst_object_set_name, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_name", (PyCFunction)_wrap_gst_object_get_name, METH_NOARGS,
      NULL },
    { "set_name_prefix", (PyCFunction)_wrap_gst_object_set_name_prefix, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_name_prefix", (PyCFunction)_wrap_gst_object_get_name_prefix, METH_NOARGS,
      NULL },
    { "set_parent", (PyCFunction)_wrap_gst_object_set_parent, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_parent", (PyCFunction)_wrap_gst_object_get_parent, METH_NOARGS,
      NULL },
    { "unparent", (PyCFunction)_wrap_gst_object_unparent, METH_NOARGS,
      NULL },
    { "has_ancestor", (PyCFunction)_wrap_gst_object_has_ancestor, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_property", (PyCFunction)_wrap_gst_object_set_property, METH_VARARGS,
      NULL },
    { "get_path_string", (PyCFunction)_wrap_gst_object_get_path_string, METH_NOARGS,
      NULL },
    { "default_error", (PyCFunction)_wrap_gst_object_default_error, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

#line 31 "gstobject.override"

/* keep this attribute around even after 2.8 for compatibility reasons */
static PyObject *
_wrap_gst_object__get___gstrefcount__ (PyGObject * self, void *closure)
{
  return PyInt_FromLong (GST_OBJECT_REFCOUNT_VALUE (self->obj));
}

#line 3916 "..\\..\\Source\\gst-python\\gst\\gst.c"


static const PyGetSetDef gst_object_getsets[] = {
    { "__gstrefcount__", (getter)_wrap_gst_object__get___gstrefcount__, (setter)0 },
    { NULL, (getter)0, (setter)0 },
};

#line 41 "gstobject.override"
static PyObject *
_wrap_gst_object_tp_repr (PyObject * self)
{
  gchar *repr;
  PyObject *ret;
  GstObject *object = GST_OBJECT (pygobject_get (self));

  repr = g_strdup_printf ("<%s object (%s) at 0x%lx>",
      self->ob_type->tp_name,
      object ? (
        GST_OBJECT_NAME (object) ? GST_OBJECT_NAME (object) : "unnamed"
      ) : "(null)",
      (long) self);
  ret = PyString_FromString (repr);
  g_free (repr);
  return ret;
}

#line 3943 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 61 "gstobject.override"
static PyObject *
_wrap_gst_object_tp_str (PyObject * self)
{
  gchar *repr, *path;
  PyObject *ret;
  GstObject *object = GST_OBJECT (pygobject_get (self));

  pyg_begin_allow_threads;
  path = gst_object_get_path_string (object);
  pyg_end_allow_threads;

  repr = g_strdup_printf ("%s (%s)", path, self->ob_type->tp_name);
  ret = PyString_FromString (repr);
  g_free (repr);
  g_free (path);
  return ret;
}
#line 3964 "..\\..\\Source\\gst-python\\gst\\gst.c"


PyTypeObject PyGstObject_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.Object",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)_wrap_gst_object_tp_repr,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)_wrap_gst_object_tp_str,              /* tp_str */
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
    (struct PyMethodDef*)_PyGstObject_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)gst_object_getsets,  /* tp_getset */
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



/* ----------- GstIndex ----------- */

static int
_wrap_gst_index_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char* kwlist[] = { NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     ":gst.Index.__init__",
                                     kwlist))
        return -1;

    pygobject_constructv(self, 0, NULL);
    if (!self->obj) {
        PyErr_SetString(
            PyExc_RuntimeError, 
            "could not create gst.Index object");
        return -1;
    }
    return 0;
}

static PyObject *
_wrap_gst_index_commit(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "id", NULL };
    int id;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GstIndex.commit", kwlist, &id))
        return NULL;
    pyg_begin_allow_threads;
    gst_index_commit(GST_INDEX(self->obj), id);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_index_get_group(PyGObject *self)
{
    int ret;

    pyg_begin_allow_threads;
    ret = gst_index_get_group(GST_INDEX(self->obj));
    pyg_end_allow_threads;
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gst_index_new_group(PyGObject *self)
{
    int ret;

    pyg_begin_allow_threads;
    ret = gst_index_new_group(GST_INDEX(self->obj));
    pyg_end_allow_threads;
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gst_index_set_group(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "groupnum", NULL };
    int groupnum, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GstIndex.set_group", kwlist, &groupnum))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_index_set_group(GST_INDEX(self->obj), groupnum);
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_index_set_certainty(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "certainty", NULL };
    PyObject *py_certainty = NULL;
    GstIndexCertainty certainty;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:GstIndex.set_certainty", kwlist, &py_certainty))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_INDEX_CERTAINTY, py_certainty, (gint *)&certainty))
        return NULL;
    pyg_begin_allow_threads;
    gst_index_set_certainty(GST_INDEX(self->obj), certainty);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_index_get_certainty(PyGObject *self)
{
    gint ret;

    pyg_begin_allow_threads;
    ret = gst_index_get_certainty(GST_INDEX(self->obj));
    pyg_end_allow_threads;
    return pyg_enum_from_gtype(GST_TYPE_INDEX_CERTAINTY, ret);
}

static PyObject *
_wrap_gst_index_add_format(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "id", "format", NULL };
    int id;
    PyObject *py_format = NULL;
    GstFormat format;
    GstIndexEntry *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"iO:GstIndex.add_format", kwlist, &id, &py_format))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_FORMAT, py_format, (gint *)&format))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_index_add_format(GST_INDEX(self->obj), id, format);
    pyg_end_allow_threads;
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(GST_TYPE_INDEX_ENTRY, ret, TRUE, TRUE);
}

static PyObject *
_wrap_gst_index_add_id(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "id", "description", NULL };
    int id;
    char *description;
    GstIndexEntry *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"is:GstIndex.add_id", kwlist, &id, &description))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_index_add_id(GST_INDEX(self->obj), id, description);
    pyg_end_allow_threads;
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(GST_TYPE_INDEX_ENTRY, ret, TRUE, TRUE);
}

static PyObject *
_wrap_gst_index_get_assoc_entry(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "id", "method", "flags", "format", "value", NULL };
    GstAssocFlags flags;
    int id;
    GstIndexEntry *ret;
    PyObject *py_method = NULL, *py_flags = NULL, *py_format = NULL;
    GstFormat format;
    GstIndexLookupMethod method;
    gint64 value;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"iOOOL:GstIndex.get_assoc_entry", kwlist, &id, &py_method, &py_flags, &py_format, &value))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_INDEX_LOOKUP_METHOD, py_method, (gint *)&method))
        return NULL;
    if (pyg_flags_get_value(GST_TYPE_ASSOC_FLAGS, py_flags, (gint *)&flags))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_FORMAT, py_format, (gint *)&format))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_index_get_assoc_entry(GST_INDEX(self->obj), id, method, flags, format, value);
    pyg_end_allow_threads;
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(GST_TYPE_INDEX_ENTRY, ret, TRUE, TRUE);
}

static PyObject *
_wrap_GstIndex__do_add_entry(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "entry", NULL };
    PyGObject *self;
    PyObject *py_entry;
    GstIndexEntry *entry = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O:GstIndex.add_entry", kwlist, &PyGstIndex_Type, &self, &py_entry))
        return NULL;
    if (pyg_boxed_check(py_entry, GST_TYPE_INDEX_ENTRY))
        entry = pyg_boxed_get(py_entry, GstIndexEntry);
    else {
        PyErr_SetString(PyExc_TypeError, "entry should be a GstIndexEntry");
        return NULL;
    }
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_INDEX_CLASS(klass)->add_entry) {
        pyg_begin_allow_threads;
        GST_INDEX_CLASS(klass)->add_entry(GST_INDEX(self->obj), entry);
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstIndex.add_entry not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGstIndex_methods[] = {
    { "commit", (PyCFunction)_wrap_gst_index_commit, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_group", (PyCFunction)_wrap_gst_index_get_group, METH_NOARGS,
      NULL },
    { "new_group", (PyCFunction)_wrap_gst_index_new_group, METH_NOARGS,
      NULL },
    { "set_group", (PyCFunction)_wrap_gst_index_set_group, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_certainty", (PyCFunction)_wrap_gst_index_set_certainty, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_certainty", (PyCFunction)_wrap_gst_index_get_certainty, METH_NOARGS,
      NULL },
    { "add_format", (PyCFunction)_wrap_gst_index_add_format, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "add_id", (PyCFunction)_wrap_gst_index_add_id, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_assoc_entry", (PyCFunction)_wrap_gst_index_get_assoc_entry, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "do_add_entry", (PyCFunction)_wrap_GstIndex__do_add_entry, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject PyGstIndex_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.Index",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGstIndex_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_gst_index_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};

static void
_wrap_GstIndex__proxy_do_add_entry(GstIndex *self, GstIndexEntry*entry)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_entry;
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
    py_entry = pyg_boxed_new(GST_TYPE_INDEX_ENTRY, entry, FALSE, FALSE);
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_entry);
    
    py_method = PyObject_GetAttrString(py_self, "do_add_entry");
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

static int
__GstIndex_class_init(gpointer gclass, PyTypeObject *pyclass)
{
    PyObject *o;
    GstIndexClass *klass = GST_INDEX_CLASS(gclass);
    PyObject *gsignals = PyDict_GetItemString(pyclass->tp_dict, "__gsignals__");

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_add_entry");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "add_entry")))
            klass->add_entry = _wrap_GstIndex__proxy_do_add_entry;
        Py_DECREF(o);
    }

    /* overriding do_get_assoc_entry is currently not supported */
    return 0;
}


/* ----------- GstElement ----------- */

static PyObject *
_wrap_GST_OBJECT_FLAGS(PyGObject *self)
{
    guint ret;

    pyg_begin_allow_threads;
    ret = GST_OBJECT_FLAGS(GST_ELEMENT(self->obj));
    pyg_end_allow_threads;
    return pyg_flags_from_gtype(GST_TYPE_ELEMENT_FLAGS, ret);
}

#line 30 "gstelement.override"
static PyObject *
_wrap_gst_element_get_pad_template(PyGObject *self, PyObject *args)
{
        PyObject *ret;
	gchar *name;
        GstPadTemplate *tmpl;
    
	if (!PyArg_ParseTuple(args, "s:GstElement.get_pad_template", &name))
		return NULL;

        pyg_begin_allow_threads;
        tmpl = gst_element_class_get_pad_template
            (GST_ELEMENT_GET_CLASS (self->obj), name);
        pyg_end_allow_threads;

        if (tmpl) {
                ret = pygobject_new (G_OBJECT (tmpl));
        } else {
                ret = Py_None;
                Py_INCREF (ret);
        }

        return ret;
}
#line 4404 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 523 "gstelement.override"
static PyObject *
_wrap_gst_element_get_pad_template_list(PyGObject *self)
{
        PyObject *ret;
	GList *res = NULL;
	guint i;
    
        pyg_begin_allow_threads;
        res = gst_element_class_get_pad_template_list
            (GST_ELEMENT_GET_CLASS (self->obj));
        pyg_end_allow_threads;

        if (res) {
		i = g_list_length (res);
		ret = PyList_New (i);
		for (i = 0 ; res ; res = g_list_next (res), i++) {
			GstPadTemplate * tmpl = (GstPadTemplate *) res->data;
			PyList_SetItem (ret, i, pygobject_new (G_OBJECT (tmpl)));
		}
        } else {
                ret = Py_None;
                Py_INCREF (ret);
        }

        return ret;
}
#line 4434 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_element_requires_clock(PyGObject *self)
{
    int ret;

    pyg_begin_allow_threads;
    ret = gst_element_requires_clock(GST_ELEMENT(self->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_element_provides_clock(PyGObject *self)
{
    int ret;

    pyg_begin_allow_threads;
    ret = gst_element_provides_clock(GST_ELEMENT(self->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_element_provide_clock(PyGObject *self)
{
    PyObject *py_ret;
    GstClock *ret;

    pyg_begin_allow_threads;
    ret = gst_element_provide_clock(GST_ELEMENT(self->obj));
    pyg_end_allow_threads;
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref((GObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_element_set_clock(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "clock", NULL };
    PyGObject *clock;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstElement.set_clock", kwlist, &PyGstClock_Type, &clock))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_element_set_clock(GST_ELEMENT(self->obj), GST_CLOCK(clock->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_element_set_base_time(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "time", NULL };
    guint64 time;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"K:GstElement.set_base_time", kwlist, &time))
        return NULL;
    pyg_begin_allow_threads;
    gst_element_set_base_time(GST_ELEMENT(self->obj), time);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_element_get_base_time(PyGObject *self)
{
    guint64 ret;

    pyg_begin_allow_threads;
    ret = gst_element_get_base_time(GST_ELEMENT(self->obj));
    pyg_end_allow_threads;
    return PyLong_FromUnsignedLongLong(ret);
}

static PyObject *
_wrap_gst_element_is_indexable(PyGObject *self)
{
    int ret;

    pyg_begin_allow_threads;
    ret = gst_element_is_indexable(GST_ELEMENT(self->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_element_set_index(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "index", NULL };
    PyGObject *index;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstElement.set_index", kwlist, &PyGstIndex_Type, &index))
        return NULL;
    pyg_begin_allow_threads;
    gst_element_set_index(GST_ELEMENT(self->obj), GST_INDEX(index->obj));
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_element_get_index(PyGObject *self)
{
    PyObject *py_ret;
    GstIndex *ret;

    pyg_begin_allow_threads;
    ret = gst_element_get_index(GST_ELEMENT(self->obj));
    pyg_end_allow_threads;
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref((GObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_element_set_bus(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "bus", NULL };
    PyGObject *bus;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstElement.set_bus", kwlist, &PyGstBus_Type, &bus))
        return NULL;
    pyg_begin_allow_threads;
    gst_element_set_bus(GST_ELEMENT(self->obj), GST_BUS(bus->obj));
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_element_get_bus(PyGObject *self)
{
    PyObject *py_ret;
    GstBus *ret;

    pyg_begin_allow_threads;
    ret = gst_element_get_bus(GST_ELEMENT(self->obj));
    pyg_end_allow_threads;
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref((GObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_element_get_clock(PyGObject *self)
{
    PyObject *py_ret;
    GstClock *ret;

    pyg_begin_allow_threads;
    ret = gst_element_get_clock(GST_ELEMENT(self->obj));
    pyg_end_allow_threads;
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref((GObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_element_add_pad(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "pad", NULL };
    PyGObject *pad;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstElement.add_pad", kwlist, &PyGstPad_Type, &pad))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_element_add_pad(GST_ELEMENT(self->obj), GST_PAD(pad->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_element_remove_pad(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "pad", NULL };
    PyGObject *pad;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstElement.remove_pad", kwlist, &PyGstPad_Type, &pad))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_element_remove_pad(GST_ELEMENT(self->obj), GST_PAD(pad->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_element_no_more_pads(PyGObject *self)
{
    pyg_begin_allow_threads;
    gst_element_no_more_pads(GST_ELEMENT(self->obj));
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_element_get_pad(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", NULL };
    char *name;
    PyObject *py_ret;
    GstPad *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:GstElement.get_pad", kwlist, &name))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_element_get_pad(GST_ELEMENT(self->obj), name);
    pyg_end_allow_threads;
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref((GObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_element_get_static_pad(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", NULL };
    char *name;
    PyObject *py_ret;
    GstPad *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:GstElement.get_static_pad", kwlist, &name))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_element_get_static_pad(GST_ELEMENT(self->obj), name);
    pyg_end_allow_threads;
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref((GObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_element_get_request_pad(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", NULL };
    char *name;
    PyObject *py_ret;
    GstPad *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:GstElement.get_request_pad", kwlist, &name))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_element_get_request_pad(GST_ELEMENT(self->obj), name);
    pyg_end_allow_threads;
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref((GObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_element_release_request_pad(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "pad", NULL };
    PyGObject *pad;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstElement.release_request_pad", kwlist, &PyGstPad_Type, &pad))
        return NULL;
    pyg_begin_allow_threads;
    gst_element_release_request_pad(GST_ELEMENT(self->obj), GST_PAD(pad->obj));
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_element_iterate_pads(PyGObject *self)
{
    GstIterator *ret;

    pyg_begin_allow_threads;
    ret = gst_element_iterate_pads(GST_ELEMENT(self->obj));
    pyg_end_allow_threads;
    return pygst_iterator_new(ret);
}

static PyObject *
_wrap_gst_element_iterate_src_pads(PyGObject *self)
{
    GstIterator *ret;

    pyg_begin_allow_threads;
    ret = gst_element_iterate_src_pads(GST_ELEMENT(self->obj));
    pyg_end_allow_threads;
    return pygst_iterator_new(ret);
}

static PyObject *
_wrap_gst_element_iterate_sink_pads(PyGObject *self)
{
    GstIterator *ret;

    pyg_begin_allow_threads;
    ret = gst_element_iterate_sink_pads(GST_ELEMENT(self->obj));
    pyg_end_allow_threads;
    return pygst_iterator_new(ret);
}

#line 361 "gstelement.override"
static PyObject *
_wrap_gst_element_send_event(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "event", NULL };
    PyObject *py_event;
    int ret;
    GstEvent *event = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:GstElement.send_event", kwlist, &py_event))
        return NULL;
    if (pygstminiobject_check (py_event, &PyGstEvent_Type)) {
        event = GST_EVENT (pygstminiobject_get (py_event));
    } else {
        PyErr_SetString(PyExc_TypeError, "event should be a GstEvent");
        return NULL;
    }

    /* The pipeline unrefs the event, but we want to keep the ownership */
    gst_event_ref(event);
    
    pyg_begin_allow_threads;
    ret = gst_element_send_event(GST_ELEMENT(self->obj), event);
    pyg_end_allow_threads;

    return PyBool_FromLong(ret);
}
#line 4779 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_element_seek(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "rate", "format", "flags", "cur_type", "cur", "stop_type", "stop", NULL };
    GstSeekType cur_type, stop_type;
    PyObject *py_format = NULL, *py_flags = NULL, *py_cur_type = NULL, *py_stop_type = NULL;
    double rate;
    int ret;
    GstFormat format;
    gint64 cur, stop;
    GstSeekFlags flags;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"dOOOLOL:GstElement.seek", kwlist, &rate, &py_format, &py_flags, &py_cur_type, &cur, &py_stop_type, &stop))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_FORMAT, py_format, (gint *)&format))
        return NULL;
    if (pyg_flags_get_value(GST_TYPE_SEEK_FLAGS, py_flags, (gint *)&flags))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_SEEK_TYPE, py_cur_type, (gint *)&cur_type))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_SEEK_TYPE, py_stop_type, (gint *)&stop_type))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_element_seek(GST_ELEMENT(self->obj), rate, format, flags, cur_type, cur, stop_type, stop);
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

#line 490 "gstelement.override"
static PyObject *
_wrap_gst_element_get_query_types (PyGObject *self)
{
    PyObject	*ret;
    PyObject	*item;
    int		i;
    GstQueryType	*tab;

    pyg_begin_allow_threads;
    tab = (GstQueryType*) gst_element_get_query_types(GST_ELEMENT(self->obj));
    pyg_end_allow_threads;
    if (tab == NULL) {
	Py_INCREF(Py_None);
	return Py_None;
    }

    ret = PyList_New(0);
    for (i = 0; tab[i] != 0; i++) {
	item = pyg_enum_from_gtype (GST_TYPE_QUERY_TYPE, tab[i]);
	PyList_Append(ret, item);
    }
    
    return ret;
}
#line 4836 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_element_query(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "query", NULL };
    int ret;
    PyGstMiniObject *query;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstElement.query", kwlist, &PyGstQuery_Type, &query))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_element_query(GST_ELEMENT(self->obj), GST_QUERY(query->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_element_post_message(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "message", NULL };
    int ret;
    PyGstMiniObject *message;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstElement.post_message", kwlist, &PyGstMessage_Type, &message))
        return NULL;
    gst_mini_object_ref(GST_MINI_OBJECT(message->obj));
    pyg_begin_allow_threads;
    ret = gst_element_post_message(GST_ELEMENT(self->obj), GST_MESSAGE(message->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_element_is_locked_state(PyGObject *self)
{
    int ret;

    pyg_begin_allow_threads;
    ret = gst_element_is_locked_state(GST_ELEMENT(self->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_element_set_locked_state(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "locked_state", NULL };
    int locked_state, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GstElement.set_locked_state", kwlist, &locked_state))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_element_set_locked_state(GST_ELEMENT(self->obj), locked_state);
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_element_sync_state_with_parent(PyGObject *self)
{
    int ret;

    pyg_begin_allow_threads;
    ret = gst_element_sync_state_with_parent(GST_ELEMENT(self->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

#line 79 "gstelement.override"
static PyObject *
_wrap_gst_element_get_state(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "timeout", NULL };
    GstState state;
    GstState pending;
    GstStateChangeReturn ret;
    GstClockTime timeout = GST_CLOCK_TIME_NONE;
    PyObject *tuple;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "|K:GstElement.get_state", kwlist,
                                     &timeout)) {
	PyErr_SetString(PyExc_RuntimeError, "Timeout not specified correctly");
        return NULL;
    }

    pyg_begin_allow_threads;

    ret = gst_element_get_state(GST_ELEMENT (self->obj), &state, &pending,
        timeout);

    pyg_end_allow_threads;

    tuple = Py_BuildValue("(OOO)",
        pyg_enum_from_gtype (GST_TYPE_STATE_CHANGE_RETURN, ret),
        pyg_enum_from_gtype (GST_TYPE_STATE, state),
        pyg_enum_from_gtype (GST_TYPE_STATE, pending));

    return tuple;
}
#line 4943 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 56 "gstelement.override"
static PyObject *
_wrap_gst_element_set_state(PyGObject *self, PyObject *args, PyObject *kwargs)
{
	static char *kwlist[] = { "state", NULL };
	PyObject *py_state = NULL;
	GstState state;
	gint ret;

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:GstElement.set_state", kwlist, &py_state))
		return NULL;
	if (pyg_enum_get_value(GST_TYPE_STATE, py_state, (gint *)&state))
		return NULL;

	pyg_begin_allow_threads;
    
	ret = gst_element_set_state(GST_ELEMENT(self->obj), state);
    
	pyg_end_allow_threads;

	return pyg_enum_from_gtype(GST_TYPE_STATE_CHANGE_RETURN, ret);
}
#line 4968 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_element_abort_state(PyGObject *self)
{
    pyg_begin_allow_threads;
    gst_element_abort_state(GST_ELEMENT(self->obj));
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_element_change_state(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "transition", NULL };
    PyObject *py_transition = NULL;
    gint ret;
    GstStateChange transition;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:GstElement.change_state", kwlist, &py_transition))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_STATE_CHANGE, py_transition, (gint *)&transition))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_element_change_state(GST_ELEMENT(self->obj), transition);
    pyg_end_allow_threads;
    return pyg_enum_from_gtype(GST_TYPE_STATE_CHANGE_RETURN, ret);
}

static PyObject *
_wrap_gst_element_continue_state(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "retstate", NULL };
    PyObject *py_retstate = NULL;
    gint ret;
    GstStateChangeReturn retstate;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:GstElement.continue_state", kwlist, &py_retstate))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_STATE_CHANGE_RETURN, py_retstate, (gint *)&retstate))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_element_continue_state(GST_ELEMENT(self->obj), retstate);
    pyg_end_allow_threads;
    return pyg_enum_from_gtype(GST_TYPE_STATE_CHANGE_RETURN, ret);
}

static PyObject *
_wrap_gst_element_lost_state(PyGObject *self)
{
    pyg_begin_allow_threads;
    gst_element_lost_state(GST_ELEMENT(self->obj));
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_element_get_factory(PyGObject *self)
{
    GstElementFactory *ret;

    pyg_begin_allow_threads;
    ret = gst_element_get_factory(GST_ELEMENT(self->obj));
    pyg_end_allow_threads;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gst_element_implements_interface(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "iface_type", NULL };
    PyObject *py_iface_type = NULL;
    int ret;
    GType iface_type;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:GstElement.implements_interface", kwlist, &py_iface_type))
        return NULL;
    if ((iface_type = pyg_type_from_object(py_iface_type)) == 0)
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_element_implements_interface(GST_ELEMENT(self->obj), iface_type);
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_element_found_tags_for_pad(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "pad", "list", NULL };
    PyGObject *pad;
    GstTagList *list = NULL;
    PyObject *py_list;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O:GstElement.found_tags_for_pad", kwlist, &PyGstPad_Type, &pad, &py_list))
        return NULL;
    if (pyg_boxed_check(py_list, GST_TYPE_TAG_LIST))
        list = pyg_boxed_get(py_list, GstTagList);
    else {
        PyErr_SetString(PyExc_TypeError, "list should be a GstTagList");
        return NULL;
    }
    pyg_begin_allow_threads;
    gst_element_found_tags_for_pad(GST_ELEMENT(self->obj), GST_PAD(pad->obj), list);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_element_found_tags(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "list", NULL };
    GstTagList *list = NULL;
    PyObject *py_list;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:GstElement.found_tags", kwlist, &py_list))
        return NULL;
    if (pyg_boxed_check(py_list, GST_TYPE_TAG_LIST))
        list = pyg_boxed_get(py_list, GstTagList);
    else {
        PyErr_SetString(PyExc_TypeError, "list should be a GstTagList");
        return NULL;
    }
    pyg_begin_allow_threads;
    gst_element_found_tags(GST_ELEMENT(self->obj), list);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_element_create_all_pads(PyGObject *self)
{
    pyg_begin_allow_threads;
    gst_element_create_all_pads(GST_ELEMENT(self->obj));
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_element_get_compatible_pad(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "pad", "caps", NULL };
    PyGObject *pad;
    PyObject *py_caps = NULL;
    gboolean caps_is_copy;
    GstCaps *caps;
    GstPad *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!|O:GstElement.get_compatible_pad", kwlist, &PyGstPad_Type, &pad, &py_caps))
        return NULL;
    if (py_caps == Py_None || py_caps == NULL)
        caps = NULL;
    else
      caps = pygst_caps_from_pyobject (py_caps, &caps_is_copy);
    if (PyErr_Occurred())
      return NULL;
    pyg_begin_allow_threads;
    ret = gst_element_get_compatible_pad(GST_ELEMENT(self->obj), GST_PAD(pad->obj), caps);
    pyg_end_allow_threads;
    if (caps && caps_is_copy)
        gst_caps_unref (caps);
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gst_element_get_compatible_pad_template(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "compattempl", NULL };
    PyGObject *compattempl;
    GstPadTemplate *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstElement.get_compatible_pad_template", kwlist, &PyGstPadTemplate_Type, &compattempl))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_element_get_compatible_pad_template(GST_ELEMENT(self->obj), GST_PAD_TEMPLATE(compattempl->obj));
    pyg_end_allow_threads;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

#line 174 "gstelement.override"
static PyObject *
_wrap_gst_element_link(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "dest", "filtercaps", NULL };
    PyGObject *dest;
    PyObject *py_caps = NULL;
    int ret;
    GstCaps *caps = NULL;
    gboolean caps_is_copy;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O!|O:GstElement.link",
                                     kwlist, &PyGstElement_Type, &dest, &py_caps))
        return NULL;
    if (py_caps == NULL)
      caps = NULL;
    else {
      caps = pygst_caps_from_pyobject (py_caps, &caps_is_copy);
      if (caps == NULL)
        /* an error happened, propagate up */
        return NULL;
    }

    pyg_begin_allow_threads;
    ret = gst_element_link_filtered(GST_ELEMENT(self->obj), GST_ELEMENT(dest->obj), caps);
    pyg_end_allow_threads;

    if (caps && caps_is_copy)
        gst_caps_unref (caps);
    if (!ret) {
	PyErr_Format(PyGstExc_LinkError,
		     "failed to link %s with %s",
		     GST_ELEMENT_NAME(self->obj),
		     GST_ELEMENT_NAME(dest->obj));
        return NULL;
    }
    return PyBool_FromLong(ret);
}

#line 5195 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_element_unlink(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "dest", NULL };
    PyGObject *dest;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstElement.unlink", kwlist, &PyGstElement_Type, &dest))
        return NULL;
    pyg_begin_allow_threads;
    gst_element_unlink(GST_ELEMENT(self->obj), GST_ELEMENT(dest->obj));
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

#line 214 "gstelement.override"
static PyObject *
_wrap_gst_element_link_filtered(PyGObject *self, PyObject *args,
                                PyObject *kwargs)
{
    if (PyErr_Warn(PyExc_DeprecationWarning, "element.link_filtered is deprecated, use element.link") < 0)
	return NULL;
    return _wrap_gst_element_link (self, args, kwargs);
}

#line 5223 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 225 "gstelement.override"
static gboolean
pad_name_from_object (PyObject *object, const gchar **name)
{
  if (object == Py_None) {
    *name = NULL;
    return TRUE;
  } else if (PyString_Check (object)) {
    *name = PyString_AsString (object);
    return TRUE;
  } else if (pygobject_check (object, &PyGstPad_Type)) {
    GstObject *obj = GST_OBJECT (pygobject_get (object));
    pyg_begin_allow_threads;
    *name = gst_object_get_name (obj);
    pyg_end_allow_threads;
    return TRUE;
  }
  PyErr_SetString(PyExc_TypeError, "argument could not be converted to a pad");
  return FALSE;
}

static PyObject *
_wrap_gst_element_link_pads(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "srcpadname", "dest", "destpadname", NULL };
    const char *srcpadname, *destpadname;
    PyGObject *dest;
    PyObject *srcpad, *destpad;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "OO!O:GstElement.link_pads", kwlist,
                                     &srcpad, &PyGstElement_Type, &dest,
                                     &destpad))
        return NULL;
    if (!pad_name_from_object (srcpad, &srcpadname) ||
        !pad_name_from_object (destpad, &destpadname))
	return NULL;

    pyg_begin_allow_threads;
    ret = gst_element_link_pads(GST_ELEMENT(self->obj), srcpadname,
                                GST_ELEMENT(dest->obj), destpadname);
    pyg_end_allow_threads;
    if (!ret) {
        PyErr_SetString(PyGstExc_LinkError, "link failed");
        return NULL;
    }
    return PyBool_FromLong(ret);

}

#line 5277 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_element_unlink_pads(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "srcpadname", "dest", "destpadname", NULL };
    char *srcpadname, *destpadname;
    PyGObject *dest;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"sO!s:GstElement.unlink_pads", kwlist, &srcpadname, &PyGstElement_Type, &dest, &destpadname))
        return NULL;
    pyg_begin_allow_threads;
    gst_element_unlink_pads(GST_ELEMENT(self->obj), srcpadname, GST_ELEMENT(dest->obj), destpadname);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

#line 277 "gstelement.override"
static PyObject *
_wrap_gst_element_link_pads_filtered(PyGObject *self, PyObject *args,
                                     PyObject *kwargs)
{
    static char *kwlist[] = { "srcpadname", "dest", "destpadname",
                              "filtercaps", NULL };
    char *srcpadname, *destpadname;
    PyGObject *dest;
    int ret;
    PyObject *py_filtercaps;
    GstCaps *filtercaps = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "sO!sO:GstElement.link_pads_filtered",
                                     kwlist, &srcpadname, &PyGstElement_Type,
                                     &dest, &destpadname, &py_filtercaps))
        return NULL;
    if (pyg_boxed_check(py_filtercaps, GST_TYPE_CAPS))
        filtercaps = pyg_boxed_get(py_filtercaps, GstCaps);
    else {
        PyErr_SetString(PyExc_TypeError, "filtercaps should be a GstCaps");
        return NULL;
    }
    pyg_begin_allow_threads;
    ret = gst_element_link_pads_filtered(GST_ELEMENT(self->obj), srcpadname,
                                         GST_ELEMENT(dest->obj), destpadname,
                                         filtercaps);
    pyg_end_allow_threads;
    if (!ret) {
        PyErr_SetString(PyGstExc_LinkError, "link failed");
        return NULL;
    }
    return PyBool_FromLong(ret);
}

#line 5332 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_element_seek_simple(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "format", "seek_flags", "seek_pos", NULL };
    PyObject *py_format = NULL, *py_seek_flags = NULL;
    int ret;
    GstFormat format;
    GstSeekFlags seek_flags;
    gint64 seek_pos;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"OOL:GstElement.seek_simple", kwlist, &py_format, &py_seek_flags, &seek_pos))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_FORMAT, py_format, (gint *)&format))
        return NULL;
    if (pyg_flags_get_value(GST_TYPE_SEEK_FLAGS, py_seek_flags, (gint *)&seek_flags))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_element_seek_simple(GST_ELEMENT(self->obj), format, seek_flags, seek_pos);
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

#line 389 "gstelement.override"
static PyObject *
_wrap_gst_element_query_position (PyGObject *self, PyObject *args)
{
    gint64	cur;
    gint	format;
    PyObject	*pformat;
    gboolean    res;

    pformat = (PyObject*)PyTuple_GetItem(args, 0);
    if (pyg_enum_get_value (GST_TYPE_FORMAT, pformat, &format)) {
        PyErr_SetString(PyExc_TypeError, "argument should be a GstFormat");
        return NULL;
    }

    pyg_begin_allow_threads;
    res = gst_element_query_position(GST_ELEMENT (self->obj), (GstFormat*) &format, &cur);
    pyg_end_allow_threads;

    if (!res) {
	PyErr_Format(PyGstExc_QueryError,
		     "query failed");
        return NULL;
    }

    return Py_BuildValue("(LO)", cur,
			 pyg_enum_from_gtype(GST_TYPE_FORMAT, format));
}
#line 5386 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 418 "gstelement.override"
static PyObject *
_wrap_gst_element_query_duration (PyGObject *self, PyObject *args)
{
    gint64	cur;
    gint	format;
    PyObject	*pformat;
    gboolean    res;

    pformat = (PyObject*)PyTuple_GetItem(args, 0);
    if (pyg_enum_get_value (GST_TYPE_FORMAT, pformat, &format)) {
        PyErr_SetString(PyExc_TypeError, "argument should be a GstFormat");
        return NULL;
    }

    pyg_begin_allow_threads;
    res = gst_element_query_duration(GST_ELEMENT (self->obj), (GstFormat*) &format, &cur);
    pyg_end_allow_threads;

    if (!res) {
	PyErr_Format(PyGstExc_QueryError,
		     "query failed");
        return NULL;
    }

    return Py_BuildValue("(LO)", cur,
			 pyg_enum_from_gtype(GST_TYPE_FORMAT, format));
}
#line 5417 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 447 "gstelement.override"
static PyObject *
_wrap_gst_element_query_convert (PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "fromformat", "fromvalue", "destformat", NULL };
    PyObject	*pfromformat, *pdestformat;
    GstFormat	srcformat, destformat;
    gint64	fromval, dstval;
    gboolean    res;

    /* Input : src_format, src_val, dst_format */
    /* Returns : dst_format, dst_val OR None */

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "OLO:GstElement.query_convert",
				     kwlist, &pfromformat, &fromval, &pdestformat))
	return NULL;

    if (pyg_enum_get_value(GST_TYPE_FORMAT, pfromformat, (gint *) &srcformat)) {
	PyErr_SetString(PyExc_TypeError, "argument should be a GstFormat");
	return NULL;
    }
    if (pyg_enum_get_value(GST_TYPE_FORMAT, pdestformat, (gint *) &destformat)) {
	PyErr_SetString(PyExc_TypeError, "argument should be a GstFormat");
	return NULL;
    }

    pyg_begin_allow_threads;
    res = gst_element_query_convert (GST_ELEMENT(self->obj),
				     srcformat, fromval,
				     &destformat, &dstval);
    pyg_end_allow_threads;

    if (!res) {
	Py_INCREF(Py_None);
	return Py_None;
    }

    return Py_BuildValue("(OL)", 
			 pyg_enum_from_gtype(GST_TYPE_FORMAT, destformat),
			 dstval);					     
}
#line 5462 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_GstElement__do_send_event(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "event", NULL };
    PyGObject *self;
    int ret;
    PyGstMiniObject *event;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!:GstElement.send_event", kwlist, &PyGstElement_Type, &self, &PyGstEvent_Type, &event))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_ELEMENT_CLASS(klass)->send_event) {
        pyg_begin_allow_threads;
        ret = GST_ELEMENT_CLASS(klass)->send_event(GST_ELEMENT(self->obj), GST_EVENT(event->obj));
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstElement.send_event not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_GstElement__do_query(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "query", NULL };
    PyGObject *self;
    int ret;
    PyGstMiniObject *query;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!:GstElement.query", kwlist, &PyGstElement_Type, &self, &PyGstQuery_Type, &query))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_ELEMENT_CLASS(klass)->query) {
        pyg_begin_allow_threads;
        ret = GST_ELEMENT_CLASS(klass)->query(GST_ELEMENT(self->obj), GST_QUERY(query->obj));
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstElement.query not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_GstElement__do_change_state(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "transition", NULL };
    PyGObject *self;
    PyObject *py_transition = NULL;
    gint ret;
    GstStateChange transition;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O:GstElement.change_state", kwlist, &PyGstElement_Type, &self, &py_transition))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_STATE_CHANGE, py_transition, (gint *)&transition))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_ELEMENT_CLASS(klass)->change_state) {
        pyg_begin_allow_threads;
        ret = GST_ELEMENT_CLASS(klass)->change_state(GST_ELEMENT(self->obj), transition);
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstElement.change_state not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return pyg_enum_from_gtype(GST_TYPE_STATE_CHANGE_RETURN, ret);
}

static PyObject *
_wrap_GstElement__do_request_new_pad(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "templ", "name", NULL };
    PyGObject *self, *templ;
    char *name;
    GstPad *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!s:GstElement.request_new_pad", kwlist, &PyGstElement_Type, &self, &PyGstPadTemplate_Type, &templ, &name))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_ELEMENT_CLASS(klass)->request_new_pad) {
        pyg_begin_allow_threads;
        ret = GST_ELEMENT_CLASS(klass)->request_new_pad(GST_ELEMENT(self->obj), GST_PAD_TEMPLATE(templ->obj), name);
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstElement.request_new_pad not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_GstElement__do_release_pad(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "pad", NULL };
    PyGObject *self, *pad;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!:GstElement.release_pad", kwlist, &PyGstElement_Type, &self, &PyGstPad_Type, &pad))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_ELEMENT_CLASS(klass)->release_pad) {
        pyg_begin_allow_threads;
        GST_ELEMENT_CLASS(klass)->release_pad(GST_ELEMENT(self->obj), GST_PAD(pad->obj));
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstElement.release_pad not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GstElement__do_provide_clock(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    GstClock *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstElement.provide_clock", kwlist, &PyGstElement_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_ELEMENT_CLASS(klass)->provide_clock) {
        pyg_begin_allow_threads;
        ret = GST_ELEMENT_CLASS(klass)->provide_clock(GST_ELEMENT(self->obj));
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstElement.provide_clock not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_GstElement__do_set_clock(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "clock", NULL };
    PyGObject *self, *clock;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!:GstElement.set_clock", kwlist, &PyGstElement_Type, &self, &PyGstClock_Type, &clock))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_ELEMENT_CLASS(klass)->set_clock) {
        pyg_begin_allow_threads;
        ret = GST_ELEMENT_CLASS(klass)->set_clock(GST_ELEMENT(self->obj), GST_CLOCK(clock->obj));
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstElement.set_clock not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_GstElement__do_get_index(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    GstIndex *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstElement.get_index", kwlist, &PyGstElement_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_ELEMENT_CLASS(klass)->get_index) {
        pyg_begin_allow_threads;
        ret = GST_ELEMENT_CLASS(klass)->get_index(GST_ELEMENT(self->obj));
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstElement.get_index not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_GstElement__do_set_index(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "index", NULL };
    PyGObject *self, *index;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!:GstElement.set_index", kwlist, &PyGstElement_Type, &self, &PyGstIndex_Type, &index))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_ELEMENT_CLASS(klass)->set_index) {
        pyg_begin_allow_threads;
        GST_ELEMENT_CLASS(klass)->set_index(GST_ELEMENT(self->obj), GST_INDEX(index->obj));
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstElement.set_index not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GstElement__do_set_bus(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "bus", NULL };
    PyGObject *self, *bus;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!:GstElement.set_bus", kwlist, &PyGstElement_Type, &self, &PyGstBus_Type, &bus))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_ELEMENT_CLASS(klass)->set_bus) {
        pyg_begin_allow_threads;
        GST_ELEMENT_CLASS(klass)->set_bus(GST_ELEMENT(self->obj), GST_BUS(bus->obj));
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstElement.set_bus not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGstElement_methods[] = {
    { "flags", (PyCFunction)_wrap_GST_OBJECT_FLAGS, METH_NOARGS,
      NULL },
    { "get_pad_template", (PyCFunction)_wrap_gst_element_get_pad_template, METH_VARARGS,
      NULL },
    { "get_pad_template_list", (PyCFunction)_wrap_gst_element_get_pad_template_list, METH_NOARGS,
      NULL },
    { "requires_clock", (PyCFunction)_wrap_gst_element_requires_clock, METH_NOARGS,
      NULL },
    { "provides_clock", (PyCFunction)_wrap_gst_element_provides_clock, METH_NOARGS,
      NULL },
    { "provide_clock", (PyCFunction)_wrap_gst_element_provide_clock, METH_NOARGS,
      NULL },
    { "set_clock", (PyCFunction)_wrap_gst_element_set_clock, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_base_time", (PyCFunction)_wrap_gst_element_set_base_time, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_base_time", (PyCFunction)_wrap_gst_element_get_base_time, METH_NOARGS,
      NULL },
    { "is_indexable", (PyCFunction)_wrap_gst_element_is_indexable, METH_NOARGS,
      NULL },
    { "set_index", (PyCFunction)_wrap_gst_element_set_index, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_index", (PyCFunction)_wrap_gst_element_get_index, METH_NOARGS,
      NULL },
    { "set_bus", (PyCFunction)_wrap_gst_element_set_bus, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_bus", (PyCFunction)_wrap_gst_element_get_bus, METH_NOARGS,
      NULL },
    { "get_clock", (PyCFunction)_wrap_gst_element_get_clock, METH_NOARGS,
      NULL },
    { "add_pad", (PyCFunction)_wrap_gst_element_add_pad, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "remove_pad", (PyCFunction)_wrap_gst_element_remove_pad, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "no_more_pads", (PyCFunction)_wrap_gst_element_no_more_pads, METH_NOARGS,
      NULL },
    { "get_pad", (PyCFunction)_wrap_gst_element_get_pad, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_static_pad", (PyCFunction)_wrap_gst_element_get_static_pad, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_request_pad", (PyCFunction)_wrap_gst_element_get_request_pad, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "release_request_pad", (PyCFunction)_wrap_gst_element_release_request_pad, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "pads", (PyCFunction)_wrap_gst_element_iterate_pads, METH_NOARGS,
      NULL },
    { "src_pads", (PyCFunction)_wrap_gst_element_iterate_src_pads, METH_NOARGS,
      NULL },
    { "sink_pads", (PyCFunction)_wrap_gst_element_iterate_sink_pads, METH_NOARGS,
      NULL },
    { "send_event", (PyCFunction)_wrap_gst_element_send_event, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "seek", (PyCFunction)_wrap_gst_element_seek, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_query_types", (PyCFunction)_wrap_gst_element_get_query_types, METH_NOARGS,
      NULL },
    { "query", (PyCFunction)_wrap_gst_element_query, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "post_message", (PyCFunction)_wrap_gst_element_post_message, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "is_locked_state", (PyCFunction)_wrap_gst_element_is_locked_state, METH_NOARGS,
      NULL },
    { "set_locked_state", (PyCFunction)_wrap_gst_element_set_locked_state, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "sync_state_with_parent", (PyCFunction)_wrap_gst_element_sync_state_with_parent, METH_NOARGS,
      NULL },
    { "get_state", (PyCFunction)_wrap_gst_element_get_state, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_state", (PyCFunction)_wrap_gst_element_set_state, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "abort_state", (PyCFunction)_wrap_gst_element_abort_state, METH_NOARGS,
      NULL },
    { "change_state", (PyCFunction)_wrap_gst_element_change_state, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "continue_state", (PyCFunction)_wrap_gst_element_continue_state, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "lost_state", (PyCFunction)_wrap_gst_element_lost_state, METH_NOARGS,
      NULL },
    { "get_factory", (PyCFunction)_wrap_gst_element_get_factory, METH_NOARGS,
      NULL },
    { "implements_interface", (PyCFunction)_wrap_gst_element_implements_interface, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "found_tags_for_pad", (PyCFunction)_wrap_gst_element_found_tags_for_pad, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "found_tags", (PyCFunction)_wrap_gst_element_found_tags, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "create_all_pads", (PyCFunction)_wrap_gst_element_create_all_pads, METH_NOARGS,
      NULL },
    { "get_compatible_pad", (PyCFunction)_wrap_gst_element_get_compatible_pad, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_compatible_pad_template", (PyCFunction)_wrap_gst_element_get_compatible_pad_template, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "link", (PyCFunction)_wrap_gst_element_link, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "unlink", (PyCFunction)_wrap_gst_element_unlink, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "link_filtered", (PyCFunction)_wrap_gst_element_link_filtered, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "link_pads", (PyCFunction)_wrap_gst_element_link_pads, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "unlink_pads", (PyCFunction)_wrap_gst_element_unlink_pads, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "link_pads_filtered", (PyCFunction)_wrap_gst_element_link_pads_filtered, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "seek_simple", (PyCFunction)_wrap_gst_element_seek_simple, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "query_position", (PyCFunction)_wrap_gst_element_query_position, METH_VARARGS,
      NULL },
    { "query_duration", (PyCFunction)_wrap_gst_element_query_duration, METH_VARARGS,
      NULL },
    { "query_convert", (PyCFunction)_wrap_gst_element_query_convert, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "do_send_event", (PyCFunction)_wrap_GstElement__do_send_event, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_query", (PyCFunction)_wrap_GstElement__do_query, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_change_state", (PyCFunction)_wrap_GstElement__do_change_state, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_request_new_pad", (PyCFunction)_wrap_GstElement__do_request_new_pad, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_release_pad", (PyCFunction)_wrap_GstElement__do_release_pad, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_provide_clock", (PyCFunction)_wrap_GstElement__do_provide_clock, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_set_clock", (PyCFunction)_wrap_GstElement__do_set_clock, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_index", (PyCFunction)_wrap_GstElement__do_get_index, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_set_index", (PyCFunction)_wrap_GstElement__do_set_index, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_set_bus", (PyCFunction)_wrap_GstElement__do_set_bus, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { NULL, NULL, 0, NULL }
};

#line 516 "gstelement.override"
static PyObject *
_wrap_gst_element_tp_iter(PyGObject *self)
{
    return _wrap_gst_element_iterate_pads(self);
}
#line 5860 "..\\..\\Source\\gst-python\\gst\\gst.c"


PyTypeObject PyGstElement_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.Element",                   /* tp_name */
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
    (getiterfunc)_wrap_gst_element_tp_iter,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGstElement_methods, /* tp_methods */
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
_wrap_GstElement__proxy_do_send_event(GstElement *self, GstEvent*event)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_event = NULL;
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
    if (event) {
        py_event = pygstminiobject_new((GstMiniObject *) event);
        gst_mini_object_unref ((GstMiniObject *) event);
    } else {
        Py_INCREF(Py_None);
        py_event = Py_None;
    }
    
    py_args = PyTuple_New(1);
    Py_INCREF(py_event);
    PyTuple_SET_ITEM(py_args, 0, py_event);
    
    py_method = PyObject_GetAttrString(py_self, "do_send_event");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        gst_mini_object_ref ((GstMiniObject *) event); Py_DECREF(py_event);
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
        gst_mini_object_ref ((GstMiniObject *) event); Py_DECREF(py_event);
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
        gst_mini_object_ref ((GstMiniObject *) event); Py_DECREF(py_event);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_DECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    gst_mini_object_ref ((GstMiniObject *) event); Py_DECREF(py_event);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gboolean
_wrap_GstElement__proxy_do_query(GstElement *self, GstQuery*query)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_query = NULL;
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
    if (query) {
        py_query = pygstminiobject_new((GstMiniObject *) query);
        gst_mini_object_unref ((GstMiniObject *) query);
    } else {
        Py_INCREF(Py_None);
        py_query = Py_None;
    }
    
    py_args = PyTuple_New(1);
    Py_INCREF(py_query);
    PyTuple_SET_ITEM(py_args, 0, py_query);
    
    py_method = PyObject_GetAttrString(py_self, "do_query");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        gst_mini_object_ref ((GstMiniObject *) query); Py_DECREF(py_query);
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
        gst_mini_object_ref ((GstMiniObject *) query); Py_DECREF(py_query);
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
        gst_mini_object_ref ((GstMiniObject *) query); Py_DECREF(py_query);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_DECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    gst_mini_object_ref ((GstMiniObject *) query); Py_DECREF(py_query);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static GstStateChangeReturn
_wrap_GstElement__proxy_do_change_state(GstElement *self, GstStateChange transition)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_transition;
    GstStateChangeReturn retval;
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
    py_transition = pyg_enum_from_gtype(GST_TYPE_STATE_CHANGE, transition);
    if (!py_transition) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_transition);
    
    py_method = PyObject_GetAttrString(py_self, "do_change_state");
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
    if (pyg_enum_get_value(GST_TYPE_STATE_CHANGE_RETURN, py_retval, (gint *)&retval)) {
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
#line 551 "gstelement.override"
static GstPad*
_wrap_GstElement__proxy_do_request_new_pad(GstElement *self, GstPadTemplate*templ, const gchar*name)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_templ = NULL;
    PyObject *py_name;
    GstPad* retval;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    if (templ)
        py_templ = pygobject_new((GObject *) templ);
    else {
        Py_INCREF(Py_None);
        py_templ = Py_None;
    }

    if (name == NULL) {
	    Py_INCREF(Py_None);
	    py_name = Py_None;
    } else {
	    py_name = PyString_FromString(name);
	    if (!py_name) {
		    if (PyErr_Occurred())
			    PyErr_Print();
		    Py_DECREF(py_templ);
		    Py_DECREF(py_self);
		    pyg_gil_state_release(__py_state);
		    return NULL;
	    }
    }
    
    py_args = PyTuple_New(2);
    PyTuple_SET_ITEM(py_args, 0, py_templ);
    PyTuple_SET_ITEM(py_args, 1, py_name);
    
    py_method = PyObject_GetAttrString(py_self, "do_request_new_pad");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    if (!PyObject_TypeCheck(py_retval, &PyGObject_Type)) {
        PyErr_SetString(PyExc_TypeError, "retval should be a GObject");
        PyErr_Print();
        Py_DECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    retval = (GstPad*) pygobject_get(py_retval);
    g_object_ref((GObject *) retval);
    
    
    Py_DECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
#line 6220 "..\\..\\Source\\gst-python\\gst\\gst.c"


static void
_wrap_GstElement__proxy_do_release_pad(GstElement *self, GstPad*pad)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_pad = NULL;
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
    if (pad)
        py_pad = pygobject_new((GObject *) pad);
    else {
        Py_INCREF(Py_None);
        py_pad = Py_None;
    }
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_pad);
    
    py_method = PyObject_GetAttrString(py_self, "do_release_pad");
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
static GstClock*
_wrap_GstElement__proxy_do_provide_clock(GstElement *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    GstClock* retval;
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_provide_clock");
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
    retval = (GstClock*) pygobject_get(py_retval);
    g_object_ref((GObject *) retval);
    
    
    Py_DECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gboolean
_wrap_GstElement__proxy_do_set_clock(GstElement *self, GstClock*clock)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_clock = NULL;
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
    if (clock)
        py_clock = pygobject_new((GObject *) clock);
    else {
        Py_INCREF(Py_None);
        py_clock = Py_None;
    }
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_clock);
    
    py_method = PyObject_GetAttrString(py_self, "do_set_clock");
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
static GstIndex*
_wrap_GstElement__proxy_do_get_index(GstElement *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    GstIndex* retval;
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_get_index");
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
    retval = (GstIndex*) pygobject_get(py_retval);
    g_object_ref((GObject *) retval);
    
    
    Py_DECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static void
_wrap_GstElement__proxy_do_set_index(GstElement *self, GstIndex*index)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_index = NULL;
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
    if (index)
        py_index = pygobject_new((GObject *) index);
    else {
        Py_INCREF(Py_None);
        py_index = Py_None;
    }
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_index);
    
    py_method = PyObject_GetAttrString(py_self, "do_set_index");
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
_wrap_GstElement__proxy_do_set_bus(GstElement *self, GstBus*bus)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_bus = NULL;
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
    if (bus)
        py_bus = pygobject_new((GObject *) bus);
    else {
        Py_INCREF(Py_None);
        py_bus = Py_None;
    }
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_bus);
    
    py_method = PyObject_GetAttrString(py_self, "do_set_bus");
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

static int
__GstElement_class_init(gpointer gclass, PyTypeObject *pyclass)
{
    PyObject *o;
    GstElementClass *klass = GST_ELEMENT_CLASS(gclass);
    PyObject *gsignals = PyDict_GetItemString(pyclass->tp_dict, "__gsignals__");

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_send_event");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "send_event")))
            klass->send_event = _wrap_GstElement__proxy_do_send_event;
        Py_DECREF(o);
    }

    /* overriding do_get_query_types is currently not supported */

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_query");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "query")))
            klass->query = _wrap_GstElement__proxy_do_query;
        Py_DECREF(o);
    }

    /* overriding do_get_state is currently not supported */

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_change_state");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "change_state")))
            klass->change_state = _wrap_GstElement__proxy_do_change_state;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_request_new_pad");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "request_new_pad")))
            klass->request_new_pad = _wrap_GstElement__proxy_do_request_new_pad;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_release_pad");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "release_pad")))
            klass->release_pad = _wrap_GstElement__proxy_do_release_pad;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_provide_clock");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "provide_clock")))
            klass->provide_clock = _wrap_GstElement__proxy_do_provide_clock;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_set_clock");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "set_clock")))
            klass->set_clock = _wrap_GstElement__proxy_do_set_clock;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_get_index");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "get_index")))
            klass->get_index = _wrap_GstElement__proxy_do_get_index;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_set_index");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "set_index")))
            klass->set_index = _wrap_GstElement__proxy_do_set_index;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_set_bus");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "set_bus")))
            klass->set_bus = _wrap_GstElement__proxy_do_set_bus;
        Py_DECREF(o);
    }
    return 0;
}


/* ----------- GstBin ----------- */

static int
_wrap_gst_bin_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    GType obj_type = pyg_type_from_object((PyObject *) self);
    GParameter params[1];
    PyObject *parsed_args[1] = {NULL, };
    char *arg_names[] = {"name", NULL };
    char *prop_names[] = {"name", NULL };
    guint nparams, i;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|O:gst.Bin.__init__" , arg_names , &parsed_args[0]))
        return -1;

    memset(params, 0, sizeof(GParameter)*1);
    if (!pyg_parse_constructor_args(obj_type, arg_names,
                                    prop_names, params, 
                                    &nparams, parsed_args))
        return -1;
    pygobject_constructv(self, nparams, params);
    for (i = 0; i < nparams; ++i)
        g_value_unset(&params[i].value);
    if (!self->obj) {
        PyErr_SetString(
            PyExc_RuntimeError, 
            "could not create gst.Bin object");
        return -1;
    }
    return 0;
}

#line 28 "gstbin.override"
static PyObject *
_wrap_gst_bin_add(PyGObject *self, PyObject *args)
{
	PyGObject *element;
	int i, len;

	len = PyTuple_Size(args);
	if (len == 0) {
		PyErr_SetString(PyExc_TypeError, "GstBin.add_many requires at least one argument");
		return NULL;
	}
       
    
	for (i = 0; i < len; i++) {
		element = (PyGObject*)PyTuple_GetItem(args, i);
		if (!pygobject_check(element, &PyGstElement_Type))
		{
			PyErr_SetString(PyExc_TypeError, "argument must be a GstElement");
			return NULL;
		}
	}
	
	for (i = 0; i < len; i++) {
		gboolean rest;

		element = (PyGObject*)PyTuple_GetItem(args, i);
		pyg_begin_allow_threads;
		rest = gst_bin_add(GST_BIN(self->obj), GST_ELEMENT(element->obj));
		pyg_end_allow_threads;
		if (!rest) {
			PyErr_Format(PyGstExc_AddError, "Could not add element '%s'", GST_OBJECT_NAME(element->obj));
			return NULL;
                }
	}

	Py_INCREF(Py_None);
	return Py_None;
}

#line 6791 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 69 "gstbin.override"
static PyObject *
_wrap_gst_bin_add_many(PyGObject *self, PyObject *args)
{
    if (PyErr_Warn(PyExc_DeprecationWarning, "gst.Bin.add_many() is deprecated, use gst.Bin.add()") < 0)
        return NULL;
    return _wrap_gst_bin_add (self, args);
}

#line 6803 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 79 "gstbin.override"
static PyObject *
_wrap_gst_bin_remove(PyGObject *self, PyObject *args)
{
	PyGObject *element;
	int i, len;

	len = PyTuple_Size(args);
	if (len == 0) {
		PyErr_SetString(PyExc_TypeError, "GstBin.remove_many requires at least one argument");
		return NULL;
	}
       
    
	for (i = 0; i < len; i++) {
		element = (PyGObject*)PyTuple_GetItem(args, i);
		if (!pygobject_check(element, &PyGstElement_Type))
		{
			PyErr_SetString(PyExc_TypeError, "argument must be a GstElement");
			return NULL;
		}
	}
	
	for (i = 0; i < len; i++) {
		gboolean rest;
		element = (PyGObject*)PyTuple_GetItem(args, i);
		pyg_begin_allow_threads;
		rest = gst_bin_remove(GST_BIN(self->obj), GST_ELEMENT(element->obj));
		pyg_end_allow_threads;
		if (!rest) {
			PyErr_Format(PyGstExc_RemoveError, "Could not remove element '%s'", GST_OBJECT_NAME(element->obj));
			return NULL;
	        }
	}

	Py_INCREF(Py_None);
	return Py_None;
}

#line 6845 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 119 "gstbin.override"
static PyObject *
_wrap_gst_bin_remove_many(PyGObject *self, PyObject *args)
{
    if (PyErr_Warn(PyExc_DeprecationWarning, "gst.Bin.remove_many() is deprecated, use gst.Bin.remove()") < 0)
        return NULL;
    return _wrap_gst_bin_remove (self, args);
}
#line 6856 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 128 "gstbin.override"
static PyObject *
_wrap_gst_bin_get_by_name(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", "recurse", NULL };
    char *name;
    gboolean recurse = FALSE;
    GstElement *el;
    PyObject *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s|b:GstBin.get_by_name",
				     kwlist, &name, &recurse))
        return NULL;
    
    if (recurse)
	    el = gst_bin_get_by_name_recurse_up(GST_BIN(self->obj), name);
    else
	    el = gst_bin_get_by_name(GST_BIN(self->obj), name);
    
    /* pygobject_new handles NULL checking */
    ret = pygobject_new((GObject *)el);
    if (el)
	    gst_object_unref (el); /* from get_by_name */
    return ret;
}
#line 6884 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_bin_get_by_interface(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "iface_gtype", NULL };
    PyObject *py_iface_gtype = NULL, *py_ret;
    GType iface_gtype;
    GstElement *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:GstBin.get_by_interface", kwlist, &py_iface_gtype))
        return NULL;
    if ((iface_gtype = pyg_type_from_object(py_iface_gtype)) == 0)
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_bin_get_by_interface(GST_BIN(self->obj), iface_gtype);
    pyg_end_allow_threads;
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref((GObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_bin_iterate_elements(PyGObject *self)
{
    GstIterator *ret;

    pyg_begin_allow_threads;
    ret = gst_bin_iterate_elements(GST_BIN(self->obj));
    pyg_end_allow_threads;
    return pygst_iterator_new(ret);
}

static PyObject *
_wrap_gst_bin_iterate_sorted(PyGObject *self)
{
    GstIterator *ret;

    pyg_begin_allow_threads;
    ret = gst_bin_iterate_sorted(GST_BIN(self->obj));
    pyg_end_allow_threads;
    return pygst_iterator_new(ret);
}

static PyObject *
_wrap_gst_bin_iterate_recurse(PyGObject *self)
{
    GstIterator *ret;

    pyg_begin_allow_threads;
    ret = gst_bin_iterate_recurse(GST_BIN(self->obj));
    pyg_end_allow_threads;
    return pygst_iterator_new(ret);
}

static PyObject *
_wrap_gst_bin_iterate_sinks(PyGObject *self)
{
    GstIterator *ret;

    pyg_begin_allow_threads;
    ret = gst_bin_iterate_sinks(GST_BIN(self->obj));
    pyg_end_allow_threads;
    return pygst_iterator_new(ret);
}

static PyObject *
_wrap_gst_bin_iterate_sources(PyGObject *self)
{
    GstIterator *ret;

    pyg_begin_allow_threads;
    ret = gst_bin_iterate_sources(GST_BIN(self->obj));
    pyg_end_allow_threads;
    return pygst_iterator_new(ret);
}

static PyObject *
_wrap_gst_bin_iterate_all_by_interface(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "iface_gtype", NULL };
    PyObject *py_iface_gtype = NULL;
    GType iface_gtype;
    GstIterator *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:GstBin.iterate_all_by_interface", kwlist, &py_iface_gtype))
        return NULL;
    if ((iface_gtype = pyg_type_from_object(py_iface_gtype)) == 0)
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_bin_iterate_all_by_interface(GST_BIN(self->obj), iface_gtype);
    pyg_end_allow_threads;
    return pygst_iterator_new(ret);
}

static PyObject *
_wrap_gst_bin_recalculate_latency(PyGObject *self)
{
    int ret;

    pyg_begin_allow_threads;
    ret = gst_bin_recalculate_latency(GST_BIN(self->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_bin_find_unlinked_pad(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "direction", NULL };
    PyObject *py_direction = NULL;
    GstPadDirection direction;
    GstPad *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:GstBin.find_unlinked_pad", kwlist, &py_direction))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_PAD_DIRECTION, py_direction, (gint *)&direction))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_bin_find_unlinked_pad(GST_BIN(self->obj), direction);
    pyg_end_allow_threads;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gst_bin_find_unconnected_pad(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "direction", NULL };
    PyObject *py_direction = NULL;
    GstPadDirection direction;
    GstPad *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:GstBin.find_unconnected_pad", kwlist, &py_direction))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_PAD_DIRECTION, py_direction, (gint *)&direction))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_bin_find_unconnected_pad(GST_BIN(self->obj), direction);
    pyg_end_allow_threads;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_GstBin__do_add_element(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "element", NULL };
    PyGObject *self, *element;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!:GstBin.add_element", kwlist, &PyGstBin_Type, &self, &PyGstElement_Type, &element))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_BIN_CLASS(klass)->add_element) {
        pyg_begin_allow_threads;
        ret = GST_BIN_CLASS(klass)->add_element(GST_BIN(self->obj), GST_ELEMENT(element->obj));
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstBin.add_element not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_GstBin__do_remove_element(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "element", NULL };
    PyGObject *self, *element;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!:GstBin.remove_element", kwlist, &PyGstBin_Type, &self, &PyGstElement_Type, &element))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_BIN_CLASS(klass)->remove_element) {
        pyg_begin_allow_threads;
        ret = GST_BIN_CLASS(klass)->remove_element(GST_BIN(self->obj), GST_ELEMENT(element->obj));
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstBin.remove_element not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyBool_FromLong(ret);

}

#line 161 "gstbin.override"
static PyObject *
_wrap_GstBin__do_handle_message(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "message", NULL };
    PyGObject *self;
    PyGstMiniObject *message;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!:GstBin.handle_message", kwlist, &PyGstBin_Type, &self, &PyGstMessage_Type, &message))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_BIN_CLASS(klass)->handle_message) {
	    gst_mini_object_ref (message->obj);
	    pyg_begin_allow_threads;
	    GST_BIN_CLASS(klass)->handle_message(GST_BIN(self->obj), GST_MESSAGE(message->obj));
	    pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstBin.handle_message not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    Py_INCREF(Py_None);
    return Py_None;
}

#line 7108 "..\\..\\Source\\gst-python\\gst\\gst.c"


static const PyMethodDef _PyGstBin_methods[] = {
    { "add", (PyCFunction)_wrap_gst_bin_add, METH_VARARGS,
      NULL },
    { "add_many", (PyCFunction)_wrap_gst_bin_add_many, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "remove", (PyCFunction)_wrap_gst_bin_remove, METH_VARARGS,
      NULL },
    { "remove_many", (PyCFunction)_wrap_gst_bin_remove_many, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_by_name", (PyCFunction)_wrap_gst_bin_get_by_name, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_by_interface", (PyCFunction)_wrap_gst_bin_get_by_interface, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "elements", (PyCFunction)_wrap_gst_bin_iterate_elements, METH_NOARGS,
      NULL },
    { "sorted", (PyCFunction)_wrap_gst_bin_iterate_sorted, METH_NOARGS,
      NULL },
    { "recurse", (PyCFunction)_wrap_gst_bin_iterate_recurse, METH_NOARGS,
      NULL },
    { "sinks", (PyCFunction)_wrap_gst_bin_iterate_sinks, METH_NOARGS,
      NULL },
    { "iterate_sources", (PyCFunction)_wrap_gst_bin_iterate_sources, METH_NOARGS,
      NULL },
    { "iterate_all_by_interface", (PyCFunction)_wrap_gst_bin_iterate_all_by_interface, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "recalculate_latency", (PyCFunction)_wrap_gst_bin_recalculate_latency, METH_NOARGS,
      NULL },
    { "remove_many", (PyCFunction)_wrap_gst_bin_remove_many, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "find_unlinked_pad", (PyCFunction)_wrap_gst_bin_find_unlinked_pad, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "find_unconnected_pad", (PyCFunction)_wrap_gst_bin_find_unconnected_pad, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "do_add_element", (PyCFunction)_wrap_GstBin__do_add_element, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_remove_element", (PyCFunction)_wrap_GstBin__do_remove_element, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_handle_message", (PyCFunction)_wrap_GstBin__do_handle_message, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { NULL, NULL, 0, NULL }
};

#line 154 "gstbin.override"
static PyObject *
_wrap_gst_bin_tp_iter(PyGObject *self)
{
    return _wrap_gst_bin_iterate_elements(self);
}
#line 7159 "..\\..\\Source\\gst-python\\gst\\gst.c"


PyTypeObject PyGstBin_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.Bin",                   /* tp_name */
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
    (getiterfunc)_wrap_gst_bin_tp_iter,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGstBin_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_gst_bin_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};

static gboolean
_wrap_GstBin__proxy_do_add_element(GstBin *self, GstElement*element)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_element = NULL;
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
    if (element)
        py_element = pygobject_new((GObject *) element);
    else {
        Py_INCREF(Py_None);
        py_element = Py_None;
    }
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_element);
    
    py_method = PyObject_GetAttrString(py_self, "do_add_element");
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
static gboolean
_wrap_GstBin__proxy_do_remove_element(GstBin *self, GstElement*element)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_element = NULL;
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
    if (element)
        py_element = pygobject_new((GObject *) element);
    else {
        Py_INCREF(Py_None);
        py_element = Py_None;
    }
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_element);
    
    py_method = PyObject_GetAttrString(py_self, "do_remove_element");
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
#line 189 "gstbin.override"
static void
_wrap_GstBin__proxy_do_handle_message(GstBin *self, GstMessage*message)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_message = NULL;
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
    if (message) {
        py_message = pygstminiobject_new((GstMiniObject *) message);
        gst_mini_object_unref ((GstMiniObject *) message);
    } else {
        Py_INCREF(Py_None);
        py_message = Py_None;
    }
    
    py_args = PyTuple_New(1);
    Py_INCREF(py_message);
    PyTuple_SET_ITEM(py_args, 0, py_message);
    
    py_method = PyObject_GetAttrString(py_self, "do_handle_message");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        gst_mini_object_ref ((GstMiniObject *) message); Py_DECREF(py_message);
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
        gst_mini_object_ref ((GstMiniObject *) message); Py_DECREF(py_message);
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
        gst_mini_object_ref ((GstMiniObject *) message); Py_DECREF(py_message);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    Py_DECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    gst_mini_object_ref ((GstMiniObject *) message); Py_DECREF(py_message);

    /* #577735: since the bus handler will return BUS_DROP, we should unref.
      This is the only change from the generated code. */
    gst_mini_object_unref ((GstMiniObject *) message);

    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
}
#line 7427 "..\\..\\Source\\gst-python\\gst\\gst.c"



static int
__GstBin_class_init(gpointer gclass, PyTypeObject *pyclass)
{
    PyObject *o;
    GstBinClass *klass = GST_BIN_CLASS(gclass);
    PyObject *gsignals = PyDict_GetItemString(pyclass->tp_dict, "__gsignals__");

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_add_element");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "add_element")))
            klass->add_element = _wrap_GstBin__proxy_do_add_element;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_remove_element");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "remove_element")))
            klass->remove_element = _wrap_GstBin__proxy_do_remove_element;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_handle_message");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "handle_message")))
            klass->handle_message = _wrap_GstBin__proxy_do_handle_message;
        Py_DECREF(o);
    }
    return 0;
}


/* ----------- GstClock ----------- */

static PyObject *
_wrap_gst_clock_set_resolution(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "resolution", NULL };
    guint64 resolution, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"K:GstClock.set_resolution", kwlist, &resolution))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_clock_set_resolution(GST_CLOCK(self->obj), resolution);
    pyg_end_allow_threads;
    return PyLong_FromUnsignedLongLong(ret);
}

static PyObject *
_wrap_gst_clock_get_resolution(PyGObject *self)
{
    guint64 ret;

    pyg_begin_allow_threads;
    ret = gst_clock_get_resolution(GST_CLOCK(self->obj));
    pyg_end_allow_threads;
    return PyLong_FromUnsignedLongLong(ret);
}

static PyObject *
_wrap_gst_clock_get_time(PyGObject *self)
{
    guint64 ret;

    pyg_begin_allow_threads;
    ret = gst_clock_get_time(GST_CLOCK(self->obj));
    pyg_end_allow_threads;
    return PyLong_FromUnsignedLongLong(ret);
}

static PyObject *
_wrap_gst_clock_set_calibration(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "internal", "external", "rate_num", "rate_denom", NULL };
    guint64 internal, external, rate_num, rate_denom;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"KKKK:GstClock.set_calibration", kwlist, &internal, &external, &rate_num, &rate_denom))
        return NULL;
    pyg_begin_allow_threads;
    gst_clock_set_calibration(GST_CLOCK(self->obj), internal, external, rate_num, rate_denom);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

#line 956 "..\\..\\Source\\gst-python\\gst\\gst.override"
static PyObject *
_wrap_gst_clock_get_calibration (PyGObject * self)
{
    PyObject		*ret;
    GstClockTime	internal;
    GstClockTime	external;
    GstClockTime	rate_num;
    GstClockTime	rate_denom;

    gst_clock_get_calibration (GST_CLOCK (self->obj),
			       &internal,
			       &external,
			       &rate_num,
			       &rate_denom);

    ret = PyTuple_New(4);
    PyTuple_SetItem(ret, 0, PyLong_FromUnsignedLongLong(internal));
    PyTuple_SetItem(ret, 1, PyLong_FromUnsignedLongLong(external));
    PyTuple_SetItem(ret, 2, PyLong_FromUnsignedLongLong(rate_num));
    PyTuple_SetItem(ret, 3, PyLong_FromUnsignedLongLong(rate_denom));
    
    return ret;
}
#line 7548 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_clock_set_master(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "master", NULL };
    PyGObject *master;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstClock.set_master", kwlist, &PyGstClock_Type, &master))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_clock_set_master(GST_CLOCK(self->obj), GST_CLOCK(master->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_clock_get_master(PyGObject *self)
{
    GstClock *ret;

    pyg_begin_allow_threads;
    ret = gst_clock_get_master(GST_CLOCK(self->obj));
    pyg_end_allow_threads;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

#line 981 "..\\..\\Source\\gst-python\\gst\\gst.override"
static PyObject *
_wrap_gst_clock_add_observation (PyGObject *self, PyObject * args, PyObject * kwargs)
{
    static char *kwlist[] = { "slave", "master", NULL};
    GstClockTime master, slave;
    gdouble squared = 1.0;
    PyObject *py_ret;
    gboolean ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "KK:GstClock.add_observation",
				     kwlist, &master, &slave))
	return NULL;
    
    ret = gst_clock_add_observation (GST_CLOCK (self->obj), master, slave,
				     &squared);

    py_ret = PyList_New(2);
    PyList_SetItem(py_ret, 0, PyBool_FromLong(ret));
    PyList_SetItem(py_ret, 1, PyFloat_FromDouble(squared));
    return py_ret;
}

#line 7602 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_clock_get_internal_time(PyGObject *self)
{
    guint64 ret;

    pyg_begin_allow_threads;
    ret = gst_clock_get_internal_time(GST_CLOCK(self->obj));
    pyg_end_allow_threads;
    return PyLong_FromUnsignedLongLong(ret);
}

static PyObject *
_wrap_gst_clock_adjust_unlocked(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "internal", NULL };
    guint64 internal, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"K:GstClock.adjust_unlocked", kwlist, &internal))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_clock_adjust_unlocked(GST_CLOCK(self->obj), internal);
    pyg_end_allow_threads;
    return PyLong_FromUnsignedLongLong(ret);
}

static PyObject *
_wrap_gst_clock_unadjust_unlocked(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "external", NULL };
    guint64 external, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"K:GstClock.unadjust_unlocked", kwlist, &external))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_clock_unadjust_unlocked(GST_CLOCK(self->obj), external);
    pyg_end_allow_threads;
    return PyLong_FromUnsignedLongLong(ret);
}

static PyObject *
_wrap_gst_clock_new_single_shot_id(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "time", NULL };
    gpointer ret;
    guint64 time;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"K:GstClock.new_single_shot_id", kwlist, &time))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_clock_new_single_shot_id(GST_CLOCK(self->obj), time);
    pyg_end_allow_threads;
    /* pyg_pointer_new handles NULL checking */
    return pyg_pointer_new(G_TYPE_POINTER, &ret);
}

static PyObject *
_wrap_gst_clock_new_periodic_id(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "start_time", "interval", NULL };
    gpointer ret;
    guint64 start_time, interval;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"KK:GstClock.new_periodic_id", kwlist, &start_time, &interval))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_clock_new_periodic_id(GST_CLOCK(self->obj), start_time, interval);
    pyg_end_allow_threads;
    /* pyg_pointer_new handles NULL checking */
    return pyg_pointer_new(G_TYPE_POINTER, &ret);
}

static PyObject *
_wrap_GstClock__do_change_resolution(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "old_resolution", "new_resolution", NULL };
    PyGObject *self;
    guint64 old_resolution, new_resolution, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!KK:GstClock.change_resolution", kwlist, &PyGstClock_Type, &self, &old_resolution, &new_resolution))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_CLOCK_CLASS(klass)->change_resolution) {
        pyg_begin_allow_threads;
        ret = GST_CLOCK_CLASS(klass)->change_resolution(GST_CLOCK(self->obj), old_resolution, new_resolution);
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstClock.change_resolution not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyLong_FromUnsignedLongLong(ret);
}

static PyObject *
_wrap_GstClock__do_get_resolution(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    guint64 ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstClock.get_resolution", kwlist, &PyGstClock_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_CLOCK_CLASS(klass)->get_resolution) {
        pyg_begin_allow_threads;
        ret = GST_CLOCK_CLASS(klass)->get_resolution(GST_CLOCK(self->obj));
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstClock.get_resolution not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyLong_FromUnsignedLongLong(ret);
}

static PyObject *
_wrap_GstClock__do_get_internal_time(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    guint64 ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstClock.get_internal_time", kwlist, &PyGstClock_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_CLOCK_CLASS(klass)->get_internal_time) {
        pyg_begin_allow_threads;
        ret = GST_CLOCK_CLASS(klass)->get_internal_time(GST_CLOCK(self->obj));
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstClock.get_internal_time not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyLong_FromUnsignedLongLong(ret);
}

static const PyMethodDef _PyGstClock_methods[] = {
    { "set_resolution", (PyCFunction)_wrap_gst_clock_set_resolution, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_resolution", (PyCFunction)_wrap_gst_clock_get_resolution, METH_NOARGS,
      NULL },
    { "get_time", (PyCFunction)_wrap_gst_clock_get_time, METH_NOARGS,
      NULL },
    { "set_calibration", (PyCFunction)_wrap_gst_clock_set_calibration, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_calibration", (PyCFunction)_wrap_gst_clock_get_calibration, METH_NOARGS,
      NULL },
    { "set_master", (PyCFunction)_wrap_gst_clock_set_master, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_master", (PyCFunction)_wrap_gst_clock_get_master, METH_NOARGS,
      NULL },
    { "add_observation", (PyCFunction)_wrap_gst_clock_add_observation, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_internal_time", (PyCFunction)_wrap_gst_clock_get_internal_time, METH_NOARGS,
      NULL },
    { "adjust_unlocked", (PyCFunction)_wrap_gst_clock_adjust_unlocked, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "unadjust_unlocked", (PyCFunction)_wrap_gst_clock_unadjust_unlocked, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "new_single_shot_id", (PyCFunction)_wrap_gst_clock_new_single_shot_id, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "new_periodic_id", (PyCFunction)_wrap_gst_clock_new_periodic_id, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "do_change_resolution", (PyCFunction)_wrap_GstClock__do_change_resolution, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_resolution", (PyCFunction)_wrap_GstClock__do_get_resolution, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_internal_time", (PyCFunction)_wrap_GstClock__do_get_internal_time, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject PyGstClock_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.Clock",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGstClock_methods, /* tp_methods */
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

static GstClockTime
_wrap_GstClock__proxy_do_change_resolution(GstClock *self, GstClockTime old_resolution, GstClockTime new_resolution)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_old_resolution;
    PyObject *py_new_resolution;
    GstClockTime retval;
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
    py_old_resolution = PyLong_FromUnsignedLongLong(old_resolution);
    py_new_resolution = PyLong_FromUnsignedLongLong(new_resolution);
    
    py_args = PyTuple_New(2);
    PyTuple_SET_ITEM(py_args, 0, py_old_resolution);
    PyTuple_SET_ITEM(py_args, 1, py_new_resolution);
    
    py_method = PyObject_GetAttrString(py_self, "do_change_resolution");
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
    retval = PyLong_AsUnsignedLongLongMask(py_retval);
    
    
    Py_DECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static GstClockTime
_wrap_GstClock__proxy_do_get_resolution(GstClock *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    GstClockTime retval;
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_get_resolution");
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
    if (!PyLong_Check(py_retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        PyErr_SetString(PyExc_TypeError, "retval should be an long");
        Py_DECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    retval = PyLong_AsUnsignedLongLongMask(py_retval);
    
    
    Py_DECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static GstClockTime
_wrap_GstClock__proxy_do_get_internal_time(GstClock *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    GstClockTime retval;
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_get_internal_time");
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
    if (!PyLong_Check(py_retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        PyErr_SetString(PyExc_TypeError, "retval should be an long");
        Py_DECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    retval = PyLong_AsUnsignedLongLongMask(py_retval);
    
    
    Py_DECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}

static int
__GstClock_class_init(gpointer gclass, PyTypeObject *pyclass)
{
    PyObject *o;
    GstClockClass *klass = GST_CLOCK_CLASS(gclass);
    PyObject *gsignals = PyDict_GetItemString(pyclass->tp_dict, "__gsignals__");

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_change_resolution");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "change_resolution")))
            klass->change_resolution = _wrap_GstClock__proxy_do_change_resolution;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_get_resolution");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "get_resolution")))
            klass->get_resolution = _wrap_GstClock__proxy_do_get_resolution;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_get_internal_time");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "get_internal_time")))
            klass->get_internal_time = _wrap_GstClock__proxy_do_get_internal_time;
        Py_DECREF(o);
    }

    /* overriding do_wait is currently not supported */

    /* overriding do_wait_jitter is currently not supported */

    /* overriding do_wait_async is currently not supported */

    /* overriding do_unschedule is currently not supported */
    return 0;
}


/* ----------- GstBus ----------- */

 static int
_wrap_gst_bus_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char* kwlist[] = { NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     ":gst.Bus.__init__",
                                     kwlist))
        return -1;

    pygobject_constructv(self, 0, NULL);
    if (!self->obj) {
        PyErr_SetString(
            PyExc_RuntimeError, 
            "could not create gst.Bus object");
        return -1;
    }
    return 0;
}

static PyObject *
_wrap_gst_bus_post(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "message", NULL };
    int ret;
    PyGstMiniObject *message;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstBus.post", kwlist, &PyGstMessage_Type, &message))
        return NULL;
    gst_mini_object_ref(GST_MINI_OBJECT(message->obj));
    pyg_begin_allow_threads;
    ret = gst_bus_post(GST_BUS(self->obj), GST_MESSAGE(message->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_bus_have_pending(PyGObject *self)
{
    int ret;

    pyg_begin_allow_threads;
    ret = gst_bus_have_pending(GST_BUS(self->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_bus_peek(PyGObject *self)
{
    PyObject *py_ret;
    GstMessage *ret;

    pyg_begin_allow_threads;
    ret = gst_bus_peek(GST_BUS(self->obj));
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_bus_pop(PyGObject *self)
{
    PyObject *py_ret;
    GstMessage *ret;

    pyg_begin_allow_threads;
    ret = gst_bus_pop(GST_BUS(self->obj));
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_bus_pop_filtered(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "types", NULL };
    PyObject *py_types = NULL, *py_ret;
    GstMessage *ret;
    GstMessageType types;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:GstBus.pop_filtered", kwlist, &py_types))
        return NULL;
    if (pyg_flags_get_value(GST_TYPE_MESSAGE_TYPE, py_types, (gint *)&types))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_bus_pop_filtered(GST_BUS(self->obj), types);
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_bus_timed_pop(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "timeout", NULL };
    PyObject *py_ret;
    GstMessage *ret;
    guint64 timeout;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"K:GstBus.timed_pop", kwlist, &timeout))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_bus_timed_pop(GST_BUS(self->obj), timeout);
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_bus_timed_pop_filtered(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "timeout", "types", NULL };
    PyObject *py_types = NULL, *py_ret;
    GstMessage *ret;
    GstMessageType types;
    guint64 timeout;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"KO:GstBus.timed_pop_filtered", kwlist, &timeout, &py_types))
        return NULL;
    if (pyg_flags_get_value(GST_TYPE_MESSAGE_TYPE, py_types, (gint *)&types))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_bus_timed_pop_filtered(GST_BUS(self->obj), timeout, types);
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_bus_set_flushing(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "flushing", NULL };
    int flushing;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GstBus.set_flushing", kwlist, &flushing))
        return NULL;
    pyg_begin_allow_threads;
    gst_bus_set_flushing(GST_BUS(self->obj), flushing);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

#line 155 "gstbus.override"
static PyObject *
_wrap_gst_bus_set_sync_handler (PyGObject *self, PyObject *args)
{
    PyObject *callback = NULL;
    PyObject *cbargs = NULL;
    PyObject *data = NULL;
    PyObject *old_data = NULL;
    gint len;
    static GQuark sync_handler_data_quark = 0;

    len = PyTuple_Size(args);

    if (len < 1) {
	PyErr_SetString(PyExc_TypeError, "Bus requires at least 1 arg");
	return NULL;
    }

    if (sync_handler_data_quark == 0)
	sync_handler_data_quark = \
			g_quark_from_static_string("PyGst::BusSyncHandlerData");

    callback = PySequence_GetItem(args, 0);
    if (callback != Py_None) {
	if (!PyCallable_Check(callback)) {
	    Py_DECREF (callback);
	    PyErr_SetString(PyExc_TypeError, "callback is not callable");
	    return NULL;
	}

	cbargs = PySequence_GetSlice(args, 1, len);
	if (cbargs == NULL) {
	    Py_DECREF (callback);
	    return NULL;
	}

	data = Py_BuildValue("(ON)", callback, cbargs);
	Py_DECREF (cbargs);
	if (data == NULL) {
	    Py_DECREF (callback);
	    return NULL;
	}

	old_data = g_object_get_qdata (self->obj, sync_handler_data_quark);
	if (old_data != NULL) {
	   Py_DECREF (old_data);
	}

	g_object_set_qdata (self->obj, sync_handler_data_quark, data);

	gst_bus_set_sync_handler (GST_BUS (self->obj),
				  (GstBusSyncHandler) bus_sync_handler,
				  data);
    } else {
	old_data = g_object_get_qdata (self->obj, sync_handler_data_quark);
	if (old_data != NULL) {
	    Py_DECREF (old_data);
	}

	g_object_set_qdata (self->obj, sync_handler_data_quark, NULL);

	gst_bus_set_sync_handler (GST_BUS (self->obj), NULL, NULL);
    }

    Py_DECREF (callback);

    Py_INCREF(Py_None);
    return Py_None;
}
#line 8285 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 225 "gstbus.override"
static PyObject *
_wrap_gst_bus_add_watch (PyGObject *self, PyObject *args)
{
    PyObject *callback, *cbargs = NULL, *data;
    guint sigid;
    guint len;

    len = PyTuple_Size(args);

    if (len < 1) {
        PyErr_SetString(PyExc_TypeError, "Bus.add_watch requires at least 1 argument");
        return NULL;
    }

    callback = PySequence_GetItem(args, 0);
    if (!PyCallable_Check(callback)) {
        PyErr_SetString(PyExc_TypeError, "callback is not callable");
        return NULL;
    }
    cbargs = PySequence_GetSlice(args, 1, len);
    if (cbargs == NULL)
        return NULL;
    /* FIXME: thomas: I'm pretty sure the second N needs to be O */
    data = Py_BuildValue("(ON)", callback, cbargs);
    if (data == NULL)
        return NULL;

    sigid = gst_bus_add_watch_full (GST_BUS (self->obj), G_PRIORITY_DEFAULT,
        (GstBusFunc) bus_func, data, (GDestroyNotify)pyg_destroy_notify);

    return PyInt_FromLong(sigid);
}
#line 8321 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_bus_poll(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "events", "timeout", NULL };
    PyObject *py_events = NULL, *py_ret;
    GstMessage *ret;
    gint64 timeout;
    GstMessageType events;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"OL:GstBus.poll", kwlist, &py_events, &timeout))
        return NULL;
    if (pyg_flags_get_value(GST_TYPE_MESSAGE_TYPE, py_events, (gint *)&events))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_bus_poll(GST_BUS(self->obj), events, timeout);
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}

#line 259 "gstbus.override"
static PyObject *
_wrap_gst_bus_add_signal_watch(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "priority", NULL };
    int priority = G_PRIORITY_DEFAULT;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|i:GstBus.add_signal_watch", kwlist, &priority))
        return NULL;
    pyg_begin_allow_threads;
    gst_bus_add_signal_watch_full(GST_BUS(self->obj), priority);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}
#line 8361 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_bus_add_signal_watch_full(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "priority", NULL };
    int priority;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GstBus.add_signal_watch_full", kwlist, &priority))
        return NULL;
    pyg_begin_allow_threads;
    gst_bus_add_signal_watch_full(GST_BUS(self->obj), priority);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_bus_remove_signal_watch(PyGObject *self)
{
    pyg_begin_allow_threads;
    gst_bus_remove_signal_watch(GST_BUS(self->obj));
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_bus_enable_sync_message_emission(PyGObject *self)
{
    pyg_begin_allow_threads;
    gst_bus_enable_sync_message_emission(GST_BUS(self->obj));
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_bus_disable_sync_message_emission(PyGObject *self)
{
    pyg_begin_allow_threads;
    gst_bus_disable_sync_message_emission(GST_BUS(self->obj));
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGstBus_methods[] = {
    { "post", (PyCFunction)_wrap_gst_bus_post, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "have_pending", (PyCFunction)_wrap_gst_bus_have_pending, METH_NOARGS,
      NULL },
    { "peek", (PyCFunction)_wrap_gst_bus_peek, METH_NOARGS,
      NULL },
    { "pop", (PyCFunction)_wrap_gst_bus_pop, METH_NOARGS,
      NULL },
    { "pop_filtered", (PyCFunction)_wrap_gst_bus_pop_filtered, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "timed_pop", (PyCFunction)_wrap_gst_bus_timed_pop, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "timed_pop_filtered", (PyCFunction)_wrap_gst_bus_timed_pop_filtered, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_flushing", (PyCFunction)_wrap_gst_bus_set_flushing, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_sync_handler", (PyCFunction)_wrap_gst_bus_set_sync_handler, METH_VARARGS,
      NULL },
    { "add_watch", (PyCFunction)_wrap_gst_bus_add_watch, METH_VARARGS,
      NULL },
    { "poll", (PyCFunction)_wrap_gst_bus_poll, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "add_signal_watch", (PyCFunction)_wrap_gst_bus_add_signal_watch, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "add_signal_watch_full", (PyCFunction)_wrap_gst_bus_add_signal_watch_full, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "remove_signal_watch", (PyCFunction)_wrap_gst_bus_remove_signal_watch, METH_NOARGS,
      NULL },
    { "enable_sync_message_emission", (PyCFunction)_wrap_gst_bus_enable_sync_message_emission, METH_NOARGS,
      NULL },
    { "disable_sync_message_emission", (PyCFunction)_wrap_gst_bus_disable_sync_message_emission, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject PyGstBus_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.Bus",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGstBus_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_gst_bus_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GstPad ----------- */

#line 816 "gstpad.override"
static int
_wrap_gst_pad_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", "direction", NULL };
    static char *kwlist2[] = { "template", "name", NULL };
    PyGObject *templ;
    char *name = NULL;
    PyObject *py_direction = NULL;
    GstPadDirection direction;

    if (PyArg_ParseTupleAndKeywords (args, kwargs, "zO:GstPad.__init__",
                                     kwlist, &name, &py_direction)) {

        GST_LOG ("gst.Pad.__init__: using gst_pad_new");
        if (pyg_enum_get_value (GST_TYPE_PAD_DIRECTION, py_direction,
                                (gint *) &direction)) {
            GST_LOG ("gst.Pad.__init__: direction is not valid");
            return -1;
        }

        self->obj = (GObject *) gst_pad_new (name, direction);
    } else {
        PyErr_Clear ();

        GST_LOG ("gst.Pad.__init__: using gst_pad_new_from_template");
        if (PyArg_ParseTupleAndKeywords (args, kwargs, "O!|s:GstPad.__init__",
                                         kwlist2, &PyGstPadTemplate_Type,
                                         &templ, &name)) {
            if (name == NULL) 
                name = GST_PAD_TEMPLATE_NAME_TEMPLATE (GST_PAD_TEMPLATE (
                    templ->obj));
            self->obj = (GObject *) gst_pad_new_from_template (
                GST_PAD_TEMPLATE (templ->obj), name);
        }
    }
    if (!self->obj) {
      PyErr_SetString (PyExc_RuntimeError, "could not create GstPad object");
      return -1;
    }
    pygobject_register_wrapper ((PyObject *)self);
    return 0;
}
#line 8537 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_pad_get_direction(PyGObject *self)
{
    gint ret;

    pyg_begin_allow_threads;
    ret = gst_pad_get_direction(GST_PAD(self->obj));
    pyg_end_allow_threads;
    return pyg_enum_from_gtype(GST_TYPE_PAD_DIRECTION, ret);
}

static PyObject *
_wrap_gst_pad_set_active(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "active", NULL };
    int active, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GstPad.set_active", kwlist, &active))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_pad_set_active(GST_PAD(self->obj), active);
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_pad_is_active(PyGObject *self)
{
    int ret;

    pyg_begin_allow_threads;
    ret = gst_pad_is_active(GST_PAD(self->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_pad_activate_pull(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "active", NULL };
    int active, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GstPad.activate_pull", kwlist, &active))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_pad_activate_pull(GST_PAD(self->obj), active);
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_pad_activate_push(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "active", NULL };
    int active, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GstPad.activate_push", kwlist, &active))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_pad_activate_push(GST_PAD(self->obj), active);
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_pad_set_blocked(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "blocked", NULL };
    int blocked, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GstPad.set_blocked", kwlist, &blocked))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_pad_set_blocked(GST_PAD(self->obj), blocked);
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

#line 1349 "gstpad.override"

#if HAVE_SET_BLOCKED_ASYNC_FULL
static void
pad_block_destroy_data (gpointer data)
{
	PyObject *py_data = (PyObject *) data;

	Py_DECREF (py_data);
}
#endif

static void
pad_block_callback_marshal(GstPad *pad, gboolean blocked, gpointer user_data)
{
    PyGILState_STATE state;
    PyObject *callback, *args;
    PyObject *ret;
    PyObject *py_user_data;

    g_return_if_fail(user_data != NULL);

    state = pyg_gil_state_ensure();

    py_user_data = (PyObject *) user_data;

    callback = PyTuple_GetItem(py_user_data, 0);
    args = Py_BuildValue("(NO)",
             pygobject_new(G_OBJECT(pad)),
             blocked ? Py_True : Py_False);
    
    {
        PyObject *tmp = args;
        args = PySequence_Concat(tmp, PyTuple_GetItem(py_user_data, 1));
        Py_DECREF (tmp);
    }

    ret = PyObject_CallObject(callback, args);
    Py_DECREF(args);

    if (!ret)
        PyErr_Print();
    else
        Py_DECREF(ret);
    
    pyg_gil_state_release(state);
}

static PyObject *
_wrap_gst_pad_set_blocked_async (PyGObject *self, PyObject *args)
{
    PyObject *callback, *cbargs = NULL, *data;
    PyObject    *pblocked, *pret;
    gboolean    blocked;
    gboolean    ret;
    gint    len;

    len = PyTuple_Size(args);

    if (len < 2) {
        PyErr_SetString(PyExc_TypeError, "Requires at least 2 arg");
        return NULL;
    }
    pblocked = PyTuple_GetItem(args, 0);
    blocked = PyObject_IsTrue(pblocked);

    callback = PyTuple_GetItem(args, 1);
    if (!PyCallable_Check(callback)) {
        PyErr_SetString(PyExc_TypeError, "callback is not callable");
        return NULL;
    }
    cbargs = PySequence_GetSlice(args, 2, len);
    if (cbargs == NULL)
        return NULL;
    data = Py_BuildValue("(ON)", callback, cbargs);
    if (data == NULL)
        return NULL;
    pyg_begin_allow_threads;
#if HAVE_SET_BLOCKED_ASYNC_FULL
    ret = gst_pad_set_blocked_async_full (GST_PAD (self->obj), blocked,
        (GstPadBlockCallback) pad_block_callback_marshal, data,
				pad_block_destroy_data);
#else
    ret = gst_pad_set_blocked_async (GST_PAD (self->obj), blocked,
        (GstPadBlockCallback) pad_block_callback_marshal, data);
#endif
    pyg_end_allow_threads;
    if (ret)
        pret = Py_True;
    else
        pret = Py_False;
    Py_INCREF(pret);

    return pret;
}
#line 8718 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_pad_is_blocked(PyGObject *self)
{
    int ret;

    pyg_begin_allow_threads;
    ret = gst_pad_is_blocked(GST_PAD(self->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_pad_get_pad_template(PyGObject *self)
{
    GstPadTemplate *ret;

    pyg_begin_allow_threads;
    ret = gst_pad_get_pad_template(GST_PAD(self->obj));
    pyg_end_allow_threads;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

#line 1137 "gstpad.override"
static PyObject *
_wrap_gst_pad_alloc_buffer (PyGObject *self, PyObject * args, PyObject *kwargs)
{
    static char *kwlist[] = {"offset", "size", "caps", NULL};
    guint64    offset;
    gint    size;
    PyObject    *pcaps;
    GstCaps    *caps;
    PyObject    *ret;
    GstBuffer    *buf;
    GstFlowReturn    res;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                     "KiO:GstPad.alloc_buffer",
                     kwlist, &offset, &size, &pcaps))
    return NULL;
    caps = pyg_boxed_get(pcaps, GstCaps);
    res = gst_pad_alloc_buffer (GST_PAD(pygobject_get(self)),
                offset, size, caps, &buf);
    ret = PyList_New(2);
    PyList_SetItem(ret, 0, pyg_enum_from_gtype(GST_TYPE_FLOW_RETURN, res));
    if (res != GST_FLOW_OK) {
        PyList_SetItem(ret, 1, Py_None);
    } else {
        PyList_SetItem(ret, 1, pygstminiobject_new(GST_MINI_OBJECT(buf)));
	/* Bring down the reference count, since we are meant to be the only
	 * one holding a reference to the newly created buffer. */
	gst_buffer_unref (buf);
    }
    return ret;
}
#line 8777 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 1170 "gstpad.override"
static PyObject *
_wrap_gst_pad_alloc_buffer_and_set_caps (PyGObject *self, PyObject * args, PyObject *kwargs)
{
    static char *kwlist[] = {"offset", "size", "caps", NULL};
    guint64    offset;
    gint    size;
    PyObject    *pcaps;
    GstCaps    *caps;
    PyObject    *ret;
    GstBuffer    *buf;
    GstFlowReturn    res;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                     "KiO:GstPad.alloc_buffer_and_set_caps",
                     kwlist, &offset, &size, &pcaps))
    return NULL;
    caps = pyg_boxed_get(pcaps, GstCaps);
    res = gst_pad_alloc_buffer_and_set_caps (GST_PAD(pygobject_get(self)),
                offset, size, caps, &buf);
    ret = PyList_New(2);
    PyList_SetItem(ret, 0, pyg_enum_from_gtype(GST_TYPE_FLOW_RETURN, res));
    if (res != GST_FLOW_OK) {
        PyList_SetItem(ret, 1, Py_None);
    } else {
        PyList_SetItem(ret, 1, pygstminiobject_new(GST_MINI_OBJECT(buf)));
	/* Bring down the reference count, since we are meant to be the only
	 * one holding a reference to the newly created buffer. */
	gst_buffer_unref (buf);
    }
    return ret;
}
#line 8812 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 510 "gstpad.override"

static void EXCEPTION_HANDLER
handle_activate_function_exception (GValue * ret, guint n, const GValue * params)
{
  GstElement *element = GST_ELEMENT (gst_pad_get_parent (g_value_get_object (&params[0])));
  
  if (!_pygst_element_check_error (element)) {
    g_assert_not_reached (); /* only returns FALSE when there's no error */
  }
}

static gboolean
call_activate_function (GstPad * pad)
{
    GClosure * closure;
    GValue ret = { 0, };
    GValue args[1] = { {0, }};
    gboolean bool;

    g_value_init (&ret, G_TYPE_BOOLEAN);
    g_value_set_boolean (&ret, FALSE);
    g_value_init(&args[0], GST_TYPE_PAD);
    g_value_set_object (&args[0], pad);
    closure = pad_private(pad)->activate_function;

    g_closure_invoke (closure, &ret, 1, args, NULL);
    
    bool = g_value_get_boolean (&ret);

    g_value_unset (&ret);
    g_value_unset (&args[0]);
    
    return bool;
}

static PyObject *
_wrap_gst_pad_set_activate_function (PyGObject *self,
				     PyObject *args,
				     PyObject *kwargs)
{
    SET_PAD_CLOSURE (self, args, kwargs, activate_function)
}
#line 8858 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 554 "gstpad.override"

static void EXCEPTION_HANDLER
handle_activatepull_function_exception (GValue * ret, guint n, const GValue * params)
{
  GstElement *element = GST_ELEMENT (gst_pad_get_parent (g_value_get_object (&params[0])));
  
  if (!_pygst_element_check_error (element)) {
    g_assert_not_reached (); /* only returns FALSE when there's no error */
  }
}

static gboolean
call_activatepull_function (GstPad * pad, gboolean active)
{
    GClosure * closure;
    GValue ret = { 0, };
    GValue args[2] = { {0, }, {0, } };
    gboolean bool;

    g_value_init (&ret, G_TYPE_BOOLEAN);
    g_value_set_boolean (&ret, FALSE);
    g_value_init (&args[0], GST_TYPE_PAD);
    g_value_set_object (&args[0], pad);
    g_value_init (&args[1], G_TYPE_BOOLEAN);
    g_value_set_boolean (&args[1], active);
    closure = pad_private(pad)->activatepull_function;

    g_closure_invoke (closure, &ret, 2, args, NULL);

    bool = g_value_get_boolean (&ret);
    
    g_value_unset (&ret);
    g_value_unset (&args[0]);
    g_value_unset (&args[1]);

    return bool;
}

static PyObject *
_wrap_gst_pad_set_activatepull_function (PyGObject *self,
					 PyObject *args,
					 PyObject *kwargs)
{
    SET_PAD_CLOSURE (self, args, kwargs, activatepull_function);
}
#line 8907 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 601 "gstpad.override"

static void EXCEPTION_HANDLER
handle_activatepush_function_exception (GValue * ret, guint n, const GValue * params)
{
  GstElement *element = GST_ELEMENT (gst_pad_get_parent (g_value_get_object (&params[0])));
  
  if (!_pygst_element_check_error (element)) {
    g_assert_not_reached (); /* only returns FALSE when there's no error */
  }
}

static gboolean
call_activatepush_function (GstPad * pad, gboolean active)
{
    GClosure * closure;
    GValue ret = { 0, };
    GValue args[2] = { {0, }, {0, }};
    gboolean bool;

    g_value_init (&ret, G_TYPE_BOOLEAN);
    g_value_set_boolean (&ret, FALSE);
    g_value_init (&args[0], GST_TYPE_PAD);
    g_value_set_object (&args[0], pad);
    g_value_init (&args[1], G_TYPE_BOOLEAN);
    g_value_set_boolean (&args[1], active);
    closure = pad_private(pad)->activatepush_function;

    g_closure_invoke (closure, &ret, 2, args, NULL);

    bool = g_value_get_boolean (&ret);
    
    g_value_unset (&ret);
    g_value_unset (&args[0]);
    g_value_unset (&args[1]);

    return bool;
}

static PyObject *
_wrap_gst_pad_set_activatepush_function (PyGObject *self,
					 PyObject *args,
					 PyObject *kwargs)
{
    SET_PAD_CLOSURE (self, args, kwargs, activatepush_function);
}
#line 8956 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 222 "gstpad.override"

static void EXCEPTION_HANDLER
handle_chain_function_exception (GValue *ret, guint n, const GValue *params)
{
  GstElement *element = GST_ELEMENT (gst_object_get_parent (g_value_get_object (&params[0])));
  
  if (!_pygst_element_check_error (element)) {
    g_assert_not_reached (); /* only returns FALSE when there's no error */
  }
}

static GstFlowReturn
call_chain_function(GstPad *pad, GstBuffer *buffer)
{
    GClosure *closure;
    GValue ret = { 0, };
    GValue args[2] = { { 0, }, { 0, } };
    GstFlowReturn    flow;
    
    g_value_init (&ret, GST_TYPE_FLOW_RETURN);
    g_value_set_enum (&ret, GST_FLOW_ERROR);
    g_value_init (&args[0], GST_TYPE_PAD);
    g_value_init (&args[1], GST_TYPE_BUFFER);

    g_value_set_object (&args[0], pad);
    gst_value_set_mini_object (&args[1], GST_MINI_OBJECT (buffer));
    closure = pad_private(pad)->chain_function;
    
    g_closure_invoke (closure, &ret, 2, args, NULL);
    flow = g_value_get_enum (&ret);

    g_value_unset (&ret);
    g_value_unset (&args[0]);
    g_value_unset (&args[1]);

    /* a chain function takes over the ref of the buffer handed to it;
     * so we should unref after calling the pythonic chain func */
    gst_buffer_unref (buffer);
    return flow;
}

static PyObject*
_wrap_gst_pad_set_chain_function(PyGObject *self,
                PyObject  *args,
                PyObject  *kwargs)
{
    SET_PAD_CLOSURE (self, args, kwargs, chain_function)
}

#line 9009 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 273 "gstpad.override"

static void EXCEPTION_HANDLER
handle_event_function_exception (GValue *ret, guint n, const GValue *params)
{
  GstElement *element = GST_ELEMENT (gst_pad_get_parent (g_value_get_object (&params[0])));
  
  if (!_pygst_element_check_error (element)) {
    g_assert_not_reached (); /* only returns FALSE when there's no error */
  }
}

static gboolean
call_event_function (GstPad *pad, GstEvent *event)
{
    GClosure *closure;
    GValue ret = { 0, };
    GValue args[2] = { { 0, }, { 0, } };
    gboolean bool;
    
    g_value_init (&ret, G_TYPE_BOOLEAN);
    g_value_set_boolean (&ret, FALSE);
    g_value_init (&args[0], GST_TYPE_PAD);
    g_value_init (&args[1], GST_TYPE_EVENT);
    g_value_set_object (&args[0], pad);
    gst_value_set_mini_object (&args[1], GST_MINI_OBJECT (event));
    closure = pad_private(pad)->event_function;
    
    g_closure_invoke (closure, &ret, 2, args, NULL);

    bool = g_value_get_boolean (&ret);

    g_value_unset (&ret);
    g_value_unset (&args[0]);
    g_value_unset (&args[1]);
    return bool;
}

static PyObject*
_wrap_gst_pad_set_event_function (PyGObject *self,
                  PyObject  *args,
                  PyObject  *kwargs)
{
    SET_PAD_CLOSURE (self, args, 
		     kwargs, event_function)
}

#line 9059 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 179 "gstpad.override"

static void EXCEPTION_HANDLER
handle_link_function_exception (GValue *ret, guint n, const GValue *params)
{
  g_value_set_enum (ret, GST_PAD_LINK_REFUSED);
  PyErr_Clear ();
}

static GstPadLinkReturn
call_link_function (GstPad *pad, GstPad *peer)
{
    GClosure *closure;
    GValue ret = { 0, };
    GValue args[2] = { { 0, }, {0, } };
    GstPadLinkReturn i;
    
    g_value_init (&ret, GST_TYPE_PAD_LINK_RETURN);
    g_value_init (&args[0], GST_TYPE_PAD);
    g_value_init (&args[1], GST_TYPE_PAD);
    g_value_set_object (&args[0], pad);
    g_value_set_object (&args[1], peer);
    
    closure = pad_private(pad)->link_function;
    
    g_closure_invoke (closure, &ret, 2, args, NULL);

    i = g_value_get_enum (&ret);
    g_value_unset (&ret);
    g_value_unset (&args[0]);
    g_value_unset (&args[1]);
    return i;
}

static PyObject*
_wrap_gst_pad_set_link_function (PyGObject *self,
        PyObject  *args,
        PyObject  *kwargs)
{
    SET_PAD_CLOSURE (self, args, kwargs, link_function)
}

#line 9104 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 729 "gstpad.override"
static PyObject *
_wrap_gst_pad_link(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "sinkpad", NULL };
    PyGObject *sinkpad;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O!:GstPad.link", kwlist,
                                     &PyGstPad_Type, &sinkpad))
        return NULL;
    ret = gst_pad_link(GST_PAD(self->obj), GST_PAD(sinkpad->obj));
    if (ret) {
        PyObject *exc_val = pyg_enum_from_gtype(GST_TYPE_PAD_LINK_RETURN, ret);
        PyErr_SetObject(PyGstExc_LinkError, exc_val);
        Py_DECREF(exc_val);
        return NULL;
    }
    return PyBool_FromLong(ret);
}

#line 9128 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_pad_unlink(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "sinkpad", NULL };
    PyGObject *sinkpad;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstPad.unlink", kwlist, &PyGstPad_Type, &sinkpad))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_pad_unlink(GST_PAD(self->obj), GST_PAD(sinkpad->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_pad_is_linked(PyGObject *self)
{
    int ret;

    pyg_begin_allow_threads;
    ret = gst_pad_is_linked(GST_PAD(self->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_pad_get_peer(PyGObject *self)
{
    PyObject *py_ret;
    GstPad *ret;

    pyg_begin_allow_threads;
    ret = gst_pad_get_peer(GST_PAD(self->obj));
    pyg_end_allow_threads;
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref((GObject *)ret);
    return py_ret;
}

#line 138 "gstpad.override"

static void EXCEPTION_HANDLER
handle_getcaps_function_exception (GValue *ret, guint n, const GValue *params)
{
  g_value_set_boxed (ret, gst_pad_get_pad_template_caps (
      GST_PAD (g_value_get_object (&params[0]))));
  PyErr_Clear ();
}

static GstCaps *
call_getcaps_function (GstPad *pad)
{
    GClosure *closure;
    GValue ret = { 0, };
    GValue args = { 0, };
    GstCaps *caps;
    
    g_value_init (&ret, GST_TYPE_CAPS);
    g_value_init (&args, GST_TYPE_PAD);
    g_value_set_object (&args, pad);
    
    closure = pad_private(pad)->getcaps_function;
    
    g_closure_invoke (closure, &ret, 1, &args, NULL);

    caps = g_value_dup_boxed (&ret);
    g_value_unset (&ret);
    g_value_unset (&args);
    return caps;
}

static PyObject*
_wrap_gst_pad_set_getcaps_function (PyGObject *self,
        PyObject  *args,
        PyObject  *kwargs)
{
    SET_PAD_CLOSURE (self, args, kwargs, getcaps_function)
}

#line 9214 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 463 "gstpad.override"

static void EXCEPTION_HANDLER
handle_setcaps_function_exception (GValue *ret, guint n, const GValue *params)
{
  GstElement *element = GST_ELEMENT (gst_pad_get_parent (g_value_get_object (&params[0])));
  
  if (!_pygst_element_check_error (element)) {
    g_assert_not_reached (); /* only returns FALSE when there's no error */
  }
}

static gboolean
call_setcaps_function (GstPad *pad, GstCaps *caps)
{
    GClosure *closure;
    GValue ret = { 0, };
    GValue args[2] = { { 0, }, { 0, } };
    gboolean bool;
    
    g_value_init (&ret, G_TYPE_BOOLEAN);
    g_value_set_boolean (&ret, FALSE);
    g_value_init (&args[0], GST_TYPE_PAD);
    g_value_init (&args[1], GST_TYPE_CAPS);
    g_value_set_object (&args[0], pad);
    gst_value_set_caps (&args[1], (const GstCaps*) caps);
    closure = pad_private(pad)->setcaps_function;
    
    g_closure_invoke (closure, &ret, 2, args, NULL);

    bool = g_value_get_boolean (&ret);

    g_value_unset (&ret);
    g_value_unset (&args[0]);
    g_value_unset (&args[1]);
    return bool;
}

static PyObject*
_wrap_gst_pad_set_setcaps_function (PyGObject *self,
                  PyObject  *args,
                  PyObject  *kwargs)
{
    SET_PAD_CLOSURE (self, args, 
		     kwargs, setcaps_function)
}
#line 9263 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 783 "gstpad.override"
static PyObject *
_wrap_gst_pad_get_pad_template_caps(PyGObject *self)
{
    GstCaps *ret = (GstCaps*)gst_pad_get_pad_template_caps(GST_PAD(self->obj));
    return pyg_boxed_new(GST_TYPE_CAPS, ret, TRUE, TRUE);
}
#line 9273 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_pad_get_caps(PyGObject *self)
{
    GstCaps *ret;

    pyg_begin_allow_threads;
    ret = gst_pad_get_caps(GST_PAD(self->obj));
    pyg_end_allow_threads;
    return pyg_boxed_new (GST_TYPE_CAPS, ret, FALSE, TRUE);
}

static PyObject *
_wrap_gst_pad_fixate_caps(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "caps", NULL };
    PyObject *py_caps;
    GstCaps *caps;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:GstPad.fixate_caps", kwlist, &py_caps))
        return NULL;
    caps = pygst_caps_from_pyobject (py_caps, NULL);
    if (PyErr_Occurred())
      return NULL;
    pyg_begin_allow_threads;
    gst_pad_fixate_caps(GST_PAD(self->obj), caps);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_pad_accept_caps(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "caps", NULL };
    PyObject *py_caps;
    int ret;
    GstCaps *caps;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:GstPad.accept_caps", kwlist, &py_caps))
        return NULL;
    caps = pygst_caps_from_pyobject (py_caps, NULL);
    if (PyErr_Occurred())
      return NULL;
    pyg_begin_allow_threads;
    ret = gst_pad_accept_caps(GST_PAD(self->obj), caps);
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

#line 1445 "gstpad.override"
static PyObject *
_wrap_gst_pad_set_caps(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "caps", NULL };
    PyObject *py_caps;
    int ret;
    GstCaps *caps;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:GstPad.set_caps", kwlist, &py_caps))
        return NULL;
    if (py_caps == Py_None) {
	caps = NULL;
    } else {
	caps = pygst_caps_from_pyobject (py_caps, NULL);
	if (PyErr_Occurred())
	    return NULL;
    }
    pyg_begin_allow_threads;
    ret = gst_pad_set_caps(GST_PAD(self->obj), caps);
    if (ret && caps)
	gst_caps_unref (caps);
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}
#line 9352 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_pad_peer_get_caps(PyGObject *self)
{
    GstCaps *ret;

    pyg_begin_allow_threads;
    ret = gst_pad_peer_get_caps(GST_PAD(self->obj));
    pyg_end_allow_threads;
    return pyg_boxed_new (GST_TYPE_CAPS, ret, FALSE, TRUE);
}

static PyObject *
_wrap_gst_pad_peer_accept_caps(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "caps", NULL };
    PyObject *py_caps;
    int ret;
    GstCaps *caps;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:GstPad.peer_accept_caps", kwlist, &py_caps))
        return NULL;
    caps = pygst_caps_from_pyobject (py_caps, NULL);
    if (PyErr_Occurred())
      return NULL;
    pyg_begin_allow_threads;
    ret = gst_pad_peer_accept_caps(GST_PAD(self->obj), caps);
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_pad_get_allowed_caps(PyGObject *self)
{
    GstCaps *ret;

    pyg_begin_allow_threads;
    ret = gst_pad_get_allowed_caps(GST_PAD(self->obj));
    pyg_end_allow_threads;
    return pyg_boxed_new (GST_TYPE_CAPS, ret, FALSE, TRUE);
}

static PyObject *
_wrap_gst_pad_get_negotiated_caps(PyGObject *self)
{
    GstCaps *ret;

    pyg_begin_allow_threads;
    ret = gst_pad_get_negotiated_caps(GST_PAD(self->obj));
    pyg_end_allow_threads;
    return pyg_boxed_new (GST_TYPE_CAPS, ret, FALSE, TRUE);
}

static PyObject *
_wrap_gst_pad_push(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "buffer", NULL };
    gint ret;
    PyGstMiniObject *buffer;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstPad.push", kwlist, &PyGstBuffer_Type, &buffer))
        return NULL;
    gst_mini_object_ref(GST_MINI_OBJECT(buffer->obj));
    pyg_begin_allow_threads;
    ret = gst_pad_push(GST_PAD(self->obj), GST_BUFFER(buffer->obj));
    pyg_end_allow_threads;
    return pyg_enum_from_gtype(GST_TYPE_FLOW_RETURN, ret);
}

static PyObject *
_wrap_gst_pad_check_pull_range(PyGObject *self)
{
    int ret;

    pyg_begin_allow_threads;
    ret = gst_pad_check_pull_range(GST_PAD(self->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

#line 1203 "gstpad.override"
static PyObject *
_wrap_gst_pad_pull_range (PyGObject *self, PyObject * args, PyObject *kwargs)
{
    static char *kwlist[] = {"offset", "size", NULL};
    guint64    offset;
    gint    size;
    PyObject    *ret;
    GstBuffer    *buf;
    GstFlowReturn    res;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                     "Ki:GstPad.pull_range",
                     kwlist, &offset, &size))
    return NULL;
    res = gst_pad_pull_range (GST_PAD(pygobject_get(self)),
                  offset, size, &buf);
    ret = PyList_New(2);
    PyList_SetItem(ret, 0, pyg_enum_from_gtype(GST_TYPE_FLOW_RETURN, res));
    if (res != GST_FLOW_OK) {
        PyList_SetItem(ret, 1, Py_None);
    } else {
        PyList_SetItem(ret, 1, pygstminiobject_new(GST_MINI_OBJECT(buf)));
	/* Bring down the reference count, since we are meant to be the only
	 * one holding a reference to the newly created buffer. */
	gst_buffer_unref (buf);
    }
    return ret;
}
#line 9465 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_pad_push_event(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "event", NULL };
    int ret;
    PyGstMiniObject *event;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstPad.push_event", kwlist, &PyGstEvent_Type, &event))
        return NULL;
    gst_mini_object_ref(GST_MINI_OBJECT(event->obj));
    pyg_begin_allow_threads;
    ret = gst_pad_push_event(GST_PAD(self->obj), GST_EVENT(event->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_pad_event_default(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "event", NULL };
    int ret;
    PyGstMiniObject *event;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstPad.event_default", kwlist, &PyGstEvent_Type, &event))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_pad_event_default(GST_PAD(self->obj), GST_EVENT(event->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_pad_chain(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "buffer", NULL };
    gint ret;
    PyGstMiniObject *buffer;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstPad.chain", kwlist, &PyGstBuffer_Type, &buffer))
        return NULL;
    gst_mini_object_ref(GST_MINI_OBJECT(buffer->obj));
    pyg_begin_allow_threads;
    ret = gst_pad_chain(GST_PAD(self->obj), GST_BUFFER(buffer->obj));
    pyg_end_allow_threads;
    return pyg_enum_from_gtype(GST_TYPE_FLOW_RETURN, ret);
}

#line 1233 "gstpad.override"
static PyObject *
_wrap_gst_pad_get_range (PyGObject *self, PyObject * args, PyObject *kwargs)
{
    static char *kwlist[] = {"offset", "size", NULL};
    guint64    offset;
    gint    size;
    PyObject    *ret;
    GstBuffer    *buf;
    GstFlowReturn    res;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                     "Ki:GstPad.get_range",
                     kwlist, &offset, &size))
    return NULL;
    res = gst_pad_get_range (GST_PAD(pygobject_get(self)),
                 offset, size, &buf);
    ret = PyList_New(2);
    PyList_SetItem(ret, 0, pyg_enum_from_gtype(GST_TYPE_FLOW_RETURN, res));
    if (res != GST_FLOW_OK) {
        PyList_SetItem(ret, 1, Py_None);
    } else {
        PyList_SetItem(ret, 1, pygstminiobject_new(GST_MINI_OBJECT(buf)));
	/* Bring down the reference count, since we are meant to be the only
	 * one holding a reference to the newly created buffer. */
	gst_buffer_unref (buf);
    }
    return ret;
}
#line 9546 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_pad_send_event(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "event", NULL };
    int ret;
    PyGstMiniObject *event;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstPad.send_event", kwlist, &PyGstEvent_Type, &event))
        return NULL;
    gst_mini_object_ref(GST_MINI_OBJECT(event->obj));
    pyg_begin_allow_threads;
    ret = gst_pad_send_event(GST_PAD(self->obj), GST_EVENT(event->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

#line 1577 "gstpad.override"
static void
pad_task_handler(void *data) {
    PyGILState_STATE state;
    PyObject *callback, *args = NULL;
    PyObject *py_user_data;

    if (data == NULL)
	return;

    state = pyg_gil_state_ensure();
    py_user_data = (PyObject*) data;

    callback = PyTuple_GetItem(py_user_data, 0);
    if (!(PyCallable_Check(callback))) {
	PyErr_Print();
	goto beach;
    }
    if (!(args = PyTuple_GetSlice(py_user_data, 1, PyTuple_Size(py_user_data)))) {
	PyErr_Print();
	goto beach;
    }

    if (!(PyTuple_Check(args))) {
	PyErr_Print();
	goto beach;
    }

    PyObject_CallObject(callback, args);

    if(PyErr_Occurred())
	PyErr_Print();

    Py_DECREF(args);

 beach:
    pyg_gil_state_release(state);
}

static PyObject *
_wrap_gst_pad_start_task(PyGObject *self, PyObject *args)
{
    PyObject *callback, *data, *cbargs, *py_ret;
    gboolean ret;

    /* 1st argument must be a callable */
    if (PyTuple_Size(args) < 1) {
	PyErr_SetString(PyExc_TypeError, "gst.Pad.start_task() requires at least 1 argument");
	return NULL;
    }

    callback = PySequence_GetItem(args, 0);
    if (!PyCallable_Check(callback)) {
	PyErr_SetString(PyExc_TypeError, "callback must be a function or method");
	Py_DECREF(callback);
	return NULL;
    }
    
    if (!(cbargs = PySequence_GetSlice(args, 1, PyTuple_Size(args)))) {
	Py_DECREF(callback);
	return NULL;
    }

    if (!(data = Py_BuildValue("(ON)", callback, cbargs))) {
	Py_DECREF(callback);
	Py_DECREF(cbargs);
	return NULL;
    }
    
    pyg_begin_allow_threads;
    ret = gst_pad_start_task(GST_PAD(self->obj), (GstTaskFunction) pad_task_handler, data);
    pyg_end_allow_threads;

    if (ret == TRUE)
	py_ret = Py_True;
    else
	py_ret = Py_False;

    Py_INCREF(py_ret);
    return py_ret;
}
#line 9647 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_pad_pause_task(PyGObject *self)
{
    int ret;

    pyg_begin_allow_threads;
    ret = gst_pad_pause_task(GST_PAD(self->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_pad_stop_task(PyGObject *self)
{
    int ret;

    pyg_begin_allow_threads;
    ret = gst_pad_stop_task(GST_PAD(self->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

#line 1263 "gstpad.override"
static PyObject *
_wrap_gst_pad_get_internal_links (PyGObject * self)
{
    PyObject    *ret;
    GList    *lst, *tmp;
    gint i;

    lst = gst_pad_get_internal_links (GST_PAD (pygobject_get(self)));

    ret = PyList_New(g_list_length(lst));
    for (tmp = lst, i = 0 ; tmp; tmp = g_list_next(tmp), ++i) {
        PyList_SetItem(ret, i, pygobject_new(G_OBJECT(tmp->data)));
    }
    g_list_free(lst);
    return ret;
}
#line 9691 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 1281 "gstpad.override"
static PyObject *
_wrap_gst_pad_get_internal_links_default (PyGObject * self)
{
    PyObject    *ret;
    GList    *lst, *tmp;
    gint i;

    lst = gst_pad_get_internal_links_default (GST_PAD (pygobject_get(self)));

    ret = PyList_New(g_list_length(lst));
    for (tmp = lst, i = 0 ; tmp; tmp = g_list_next(tmp), ++i) {
        PyList_SetItem(ret, i, pygobject_new(G_OBJECT(tmp->data)));
    }
    g_list_free(lst);
    return ret;
}
#line 9711 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_pad_iterate_internal_links(PyGObject *self)
{
    GstIterator *ret;

    pyg_begin_allow_threads;
    ret = gst_pad_iterate_internal_links(GST_PAD(self->obj));
    pyg_end_allow_threads;
    return pygst_iterator_new(ret);
}

static PyObject *
_wrap_gst_pad_iterate_internal_links_default(PyGObject *self)
{
    GstIterator *ret;

    pyg_begin_allow_threads;
    ret = gst_pad_iterate_internal_links_default(GST_PAD(self->obj));
    pyg_end_allow_threads;
    return pygst_iterator_new(ret);
}

#line 1299 "gstpad.override"
static PyObject *
_wrap_gst_pad_get_query_types (PyGObject *self)
{
    PyObject    *ret;
    PyObject    *item;
    int        i;
    GstQueryType    *tab;

    tab = (GstQueryType*) gst_pad_get_query_types(GST_PAD(self->obj));
    if (tab == NULL) {
    Py_INCREF(Py_None);
    return Py_None;
    }

    ret = PyList_New(0);
    for (i = 0; tab[i] != 0; i++) {
        item = pyg_enum_from_gtype (GST_TYPE_QUERY_TYPE, tab[i]);
        PyList_Append(ret, item);
        Py_XDECREF(item);
    }
    
    return ret;
}
#line 9760 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 1324 "gstpad.override"
static PyObject *
_wrap_gst_pad_get_query_types_default (PyGObject *self)
{
    PyObject    *ret;
    PyObject    *item;
    int        i;
    GstQueryType    *tab;

    tab = (GstQueryType*) gst_pad_get_query_types_default(GST_PAD(self->obj));
    if (tab == NULL) {
    Py_INCREF(Py_None);
    return Py_None;
    }

    ret = PyList_New(0);
    for (i = 0; tab[i] != 0; i++) {
        item = pyg_enum_from_gtype (GST_TYPE_QUERY_TYPE, tab[i]);
        PyList_Append(ret, item);
        Py_XDECREF(item);
    }
    
    return ret;
}
#line 9787 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 687 "gstpad.override"
static PyObject *
_wrap_gst_pad_query(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "query", NULL };
    int ret;
    PyGstMiniObject *query;
    
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O!:GstPad.query", kwlist, &PyGstQuery_Type, &query))
        return NULL;

    pyg_begin_allow_threads;
    ret = gst_pad_query(GST_PAD(self->obj), GST_QUERY (query->obj));
    pyg_end_allow_threads;

    return PyBool_FromLong(ret);   
}
#line 9807 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_pad_peer_query(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "query", NULL };
    int ret;
    PyGstMiniObject *query;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstPad.peer_query", kwlist, &PyGstQuery_Type, &query))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_pad_peer_query(GST_PAD(self->obj), GST_QUERY(query->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

#line 321 "gstpad.override"

static gboolean
pypad_copy_struct_members (GQuark field_id, const GValue * value, 
    GstStructure* to_structure)
{
    gst_structure_id_set_value (to_structure, field_id, value);

    return TRUE;
}

static gboolean
call_query_function (GstPad *pad, GstQuery *query)
{
    PyGILState_STATE __py_state;
    PyGObject *py_pad;
    PyGstPadPrivate *priv;

    PyObject *py_ret;
    PyObject *py_args;
    gboolean ret = FALSE;
    GstQuery *query_copy;
    PyObject *py_query;

    /* Push our GIL state */
    __py_state = pyg_gil_state_ensure();

    /* Get the python version of the pad */
    py_pad = (PyGObject *) pygobject_new((GObject*) (pad));
    if (!py_pad) {
        if (PyErr_Occurred())
            PyErr_Print();
        goto beach;
    }
    /* Private data, where our callback should be stored */
    priv = py_pad_private(py_pad);
    if (priv->query_function == NULL) {
        /* FIXME: Generate an error message somewhere? */
        Py_DECREF(py_pad);
        goto beach;
    }
    
    /* Create our arguments tuple and populate */
    py_args = PyTuple_New(2);

    /* We copy the query into a new one so that it can have a refcount
     * of exactly 1 and be owned by python */
    pyg_begin_allow_threads;
    query_copy = gst_query_copy (query);
    pyg_end_allow_threads;
    py_query = pygstminiobject_new((GstMiniObject *)query_copy);
    gst_query_unref (query_copy);

    PyTuple_SetItem(py_args, 0, (PyObject *) (py_pad));
    PyTuple_SetItem(py_args, 1, py_query);

    /* Perform the callback into python, then parse the result */
    py_ret = PyObject_CallObject(priv->query_function, py_args);
    if (!py_ret) {
        if (PyErr_Occurred())
            PyErr_Print();

        Py_DECREF(py_args);
        goto beach;
    }

    ret = (py_ret == Py_True ? TRUE : FALSE);

    /* If the query succeeded, copy the result back into the original query.
     * We still have a refcount to it, because we didn't unref the py_query
     * wrapper yet. */
    if (ret) {
        /* I feel ill violating the poor query like this, but it's the only
         * way to transfer data from our copy back to the original query */
        GstStructure *from, *to;
        
        pyg_begin_allow_threads;
        from = GST_QUERY (query_copy)->structure; 
        to = query->structure;
        gst_structure_foreach (from, 
            (GstStructureForeachFunc) pypad_copy_struct_members, to);
        pyg_end_allow_threads;
    }

    Py_DECREF(py_args);
    Py_DECREF(py_ret);
beach:
    pyg_gil_state_release(__py_state);
    
    return ret;
}

static PyObject*
_wrap_gst_pad_set_query_function (PyGObject *self,
                  PyObject  *args,
                  PyObject  *kwargs)
{
    static char *kwlist[] = { "query_function", NULL };
    PyObject *function;
    GstPad *pad;
    PyGstPadPrivate *priv;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                     "O:GstPad.set_query_function",
                     kwlist,
                     &function)) {
        return NULL;
    }

    pad = (GstPad*)pygobject_get(self);
    priv = py_pad_private(self);

    /* Allow setting query_function to None to clear it to NULL */
    if (function == Py_None) {
        if (priv->query_function) {
            Py_DECREF (priv->query_function);
            priv->query_function = NULL;
        }
        gst_pad_set_query_function (pad, NULL);
        goto out;
    }

    if (!PyCallable_Check(function)) {
        PyErr_SetString(PyExc_TypeError, "Passed query_function not callable");
        return NULL;
    }

    if (priv->query_function) {
        Py_DECREF (priv->query_function);
    }

    Py_INCREF(function);
    priv->query_function = function;

    gst_pad_set_query_function (pad, call_query_function);

out:
    Py_INCREF(Py_None);
    return Py_None;
}

#line 9967 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_pad_query_default(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "query", NULL };
    int ret;
    PyGstMiniObject *query;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstPad.query_default", kwlist, &PyGstQuery_Type, &query))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_pad_query_default(GST_PAD(self->obj), GST_QUERY(query->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

#line 860 "gstpad.override"
static PyObject *
_wrap_gst_pad_add_data_probe(PyGObject *self, PyObject *args)
{
    GstPad *pad = GST_PAD(self->obj);
    PyObject *method = NULL;
    PyObject *rv = NULL;
    PyObject *mylist = PyList_New(1);
    PyObject *mynewlist = NULL;
    PyObject *myargs = NULL;
    PyObject *signalname = NULL;

    signalname = PyString_FromString("have-data");

    if (PyList_SetItem(mylist, 0, signalname)) {
	Py_DECREF(mylist);
	return NULL;
    }

    mynewlist = PySequence_InPlaceConcat(mylist, args);

    Py_DECREF(mylist);

    if (!mynewlist)
	return NULL;

    myargs = PyList_AsTuple(mynewlist);

    Py_DECREF(mynewlist);

    if (!myargs)
	return NULL;

    method = PyObject_GetAttrString((PyObject*)self, "connect");

    if (!method) {
	Py_DECREF(mylist);
        return NULL;
    }

    GST_OBJECT_LOCK (pad);

    rv = PyObject_CallObject(method, myargs);
    if (rv) {
	GST_PAD_DO_BUFFER_SIGNALS (pad)++;
	GST_PAD_DO_EVENT_SIGNALS (pad)++;
    }

    GST_OBJECT_UNLOCK (pad);

    Py_DECREF(myargs);
    Py_DECREF(method);

    return rv;
}
#line 10041 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_pad_remove_data_probe(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "handler_id", NULL };
    PyObject *py_handler_id = NULL;
    guint handler_id = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:GstPad.remove_data_probe", kwlist, &py_handler_id))
        return NULL;
    if (py_handler_id) {
        if (PyLong_Check(py_handler_id))
            handler_id = PyLong_AsUnsignedLong(py_handler_id);
        else if (PyInt_Check(py_handler_id))
            handler_id = PyInt_AsLong(py_handler_id);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'handler_id' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    pyg_begin_allow_threads;
    gst_pad_remove_data_probe(GST_PAD(self->obj), handler_id);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

#line 916 "gstpad.override"
static PyObject *
_wrap_gst_pad_add_event_probe(PyGObject *self, PyObject *args)
{
    GstPad *pad = GST_PAD(self->obj);
    PyObject *method = NULL;
    PyObject *rv = NULL;
    PyObject *mylist = PyList_New(1);
    PyObject *mynewlist = NULL;
    PyObject *myargs = NULL;
    PyObject *signalname = NULL;

    signalname = PyString_FromString("have-data::event");

    if (PyList_SetItem(mylist, 0, signalname)) {
	Py_DECREF(mylist);
	return NULL;
    }

    mynewlist = PySequence_InPlaceConcat(mylist, args);

    Py_DECREF(mylist);

    if (!mynewlist)
	return NULL;

    myargs = PyList_AsTuple(mynewlist);

    Py_DECREF(mynewlist);

    if (!myargs)
	return NULL;

    method = PyObject_GetAttrString((PyObject*)self, "connect");

    if (!method) {
	Py_DECREF(mylist);
        return NULL;
    }

    GST_OBJECT_LOCK (pad);

    rv = PyObject_CallObject(method, myargs);
    if (rv)
	GST_PAD_DO_EVENT_SIGNALS (pad)++;

    GST_PAD_DO_BUFFER_SIGNALS (pad)++;
    GST_DEBUG ("adding event probe to pad %s:%s, now %d probes",
	       GST_DEBUG_PAD_NAME (pad), GST_PAD_DO_BUFFER_SIGNALS (pad));
    GST_OBJECT_UNLOCK (pad);

    Py_DECREF(myargs);
    Py_DECREF(method);

    return rv;
}
#line 10126 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_pad_remove_event_probe(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "handler_id", NULL };
    PyObject *py_handler_id = NULL;
    guint handler_id = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:GstPad.remove_event_probe", kwlist, &py_handler_id))
        return NULL;
    if (py_handler_id) {
        if (PyLong_Check(py_handler_id))
            handler_id = PyLong_AsUnsignedLong(py_handler_id);
        else if (PyInt_Check(py_handler_id))
            handler_id = PyInt_AsLong(py_handler_id);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'handler_id' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    pyg_begin_allow_threads;
    gst_pad_remove_event_probe(GST_PAD(self->obj), handler_id);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

#line 973 "gstpad.override"
static PyObject *
_wrap_gst_pad_add_buffer_probe(PyGObject *self, PyObject *args)
{
    GstPad *pad = GST_PAD(self->obj);
    PyObject *method = NULL;
    PyObject *rv = NULL;
    PyObject *mylist = PyList_New(1);
    PyObject *mynewlist = NULL;
    PyObject *myargs = NULL;
    PyObject *signalname = NULL;

    signalname = PyString_FromString("have-data::buffer");

    if (PyList_SetItem(mylist, 0, signalname)) {
	Py_DECREF(mylist);
	return NULL;
    }

    mynewlist = PySequence_InPlaceConcat(mylist, args);

    Py_DECREF(mylist);

    if (!mynewlist)
	return NULL;

    myargs = PyList_AsTuple(mynewlist);

    Py_DECREF(mynewlist);

    if (!myargs)
	return NULL;

    method = PyObject_GetAttrString((PyObject*)self, "connect");

    if (!method) {
	Py_DECREF(mylist);
        return NULL;
    }

    GST_OBJECT_LOCK (pad);

    rv = PyObject_CallObject(method, myargs);
    if (rv)
	GST_PAD_DO_BUFFER_SIGNALS (pad)++;

    GST_OBJECT_UNLOCK (pad);

    Py_DECREF(myargs);
    Py_DECREF(method);

    return rv;
}
#line 10208 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_pad_remove_buffer_probe(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "handler_id", NULL };
    PyObject *py_handler_id = NULL;
    guint handler_id = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:GstPad.remove_buffer_probe", kwlist, &py_handler_id))
        return NULL;
    if (py_handler_id) {
        if (PyLong_Check(py_handler_id))
            handler_id = PyLong_AsUnsignedLong(py_handler_id);
        else if (PyInt_Check(py_handler_id))
            handler_id = PyInt_AsLong(py_handler_id);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'handler_id' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    pyg_begin_allow_threads;
    gst_pad_remove_buffer_probe(GST_PAD(self->obj), handler_id);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_pad_can_link(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "sinkpad", NULL };
    PyGObject *sinkpad;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstPad.can_link", kwlist, &PyGstPad_Type, &sinkpad))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_pad_can_link(GST_PAD(self->obj), GST_PAD(sinkpad->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_pad_use_fixed_caps(PyGObject *self)
{
    pyg_begin_allow_threads;
    gst_pad_use_fixed_caps(GST_PAD(self->obj));
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_pad_get_fixed_caps_func(PyGObject *self)
{
    GstCaps *ret;

    pyg_begin_allow_threads;
    ret = gst_pad_get_fixed_caps_func(GST_PAD(self->obj));
    pyg_end_allow_threads;
    return pyg_boxed_new (GST_TYPE_CAPS, ret, FALSE, TRUE);
}

static PyObject *
_wrap_gst_pad_proxy_getcaps(PyGObject *self)
{
    GstCaps *ret;

    pyg_begin_allow_threads;
    ret = gst_pad_proxy_getcaps(GST_PAD(self->obj));
    pyg_end_allow_threads;
    return pyg_boxed_new (GST_TYPE_CAPS, ret, FALSE, TRUE);
}

static PyObject *
_wrap_gst_pad_proxy_setcaps(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "caps", NULL };
    PyObject *py_caps;
    int ret;
    GstCaps *caps;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:GstPad.proxy_setcaps", kwlist, &py_caps))
        return NULL;
    caps = pygst_caps_from_pyobject (py_caps, NULL);
    if (PyErr_Occurred())
      return NULL;
    pyg_begin_allow_threads;
    ret = gst_pad_proxy_setcaps(GST_PAD(self->obj), caps);
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_pad_get_parent_element(PyGObject *self)
{
    PyObject *py_ret;
    GstElement *ret;

    pyg_begin_allow_threads;
    ret = gst_pad_get_parent_element(GST_PAD(self->obj));
    pyg_end_allow_threads;
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref((GObject *)ret);
    return py_ret;
}

#line 1048 "gstpad.override"
static PyObject *
_wrap_gst_pad_query_position (PyGObject *self, PyObject *args)
{
    gint64    cur;
    gint    format;
    PyObject    *pformat;
    PyObject    *ret;

    pformat = (PyObject*)PyTuple_GetItem(args, 0);
    if (pyg_enum_get_value (GST_TYPE_FORMAT, pformat, &format)) {
        PyErr_SetString(PyExc_TypeError, "argument should be a GstFormat");
        return NULL;
    }

    if ((gst_pad_query_position(GST_PAD (self->obj), (GstFormat*) &format, &cur))) {
	ret = Py_BuildValue("(LO)", cur,
			    pyg_enum_from_gtype (GST_TYPE_FORMAT, format));
    } else {
        Py_INCREF(Py_None);
        ret = Py_None;
    }

    return ret;
}
#line 10345 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 1074 "gstpad.override"
static PyObject *
_wrap_gst_pad_query_duration (PyGObject *self, PyObject *args)
{
    gint64    cur;
    gint    format;
    PyObject    *pformat;
    PyObject    *ret;

    pformat = (PyObject*)PyTuple_GetItem(args, 0);
    if (pyg_enum_get_value (GST_TYPE_FORMAT, pformat, &format)) {
        PyErr_SetString(PyExc_TypeError, "argument should be a GstFormat");
        return NULL;
    }

    if ((gst_pad_query_duration(GST_PAD (self->obj), (GstFormat*) &format, &cur))) {
	ret = Py_BuildValue("(LO)", cur,
			    pyg_enum_from_gtype (GST_TYPE_FORMAT, format));
    } else {
        Py_INCREF(Py_None);
        ret = Py_None;
    }

    return ret;
}
#line 10373 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 1100 "gstpad.override"
static PyObject *
_wrap_gst_pad_query_convert (PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "fromformat", "fromvalue", "destformat", NULL };
    PyObject    *pfromformat, *pdestformat;
    GstFormat    srcformat, destformat;
    gint64    fromval, dstval;

    /* Input : src_format, src_val, dst_format */
    /* Returns : dst_format, dst_val OR None */

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                     "0L0:GstPad.query_convert",
                     kwlist, &pfromformat, &fromval, &pdestformat))
    return NULL;
    if (pyg_enum_get_value(GST_TYPE_FORMAT, pfromformat, (gint *) &srcformat)) {
    PyErr_SetString(PyExc_TypeError, "argument should be a GstFormat");
    return NULL;
    }
    if (pyg_enum_get_value(GST_TYPE_FORMAT, pdestformat, (gint *) &destformat)) {
    PyErr_SetString(PyExc_TypeError, "argument should be a GstFormat");
    return NULL;
    }

    if (!(gst_pad_query_convert (GST_PAD(self->obj),
                 srcformat, fromval,
                 &destformat, &dstval))) {
    Py_INCREF(Py_None);
    return Py_None;
    }
    
    return Py_BuildValue("(OL)",
			 pyg_enum_from_gtype (GST_TYPE_FORMAT, destformat),
			 dstval);
}
#line 10412 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 1472 "gstpad.override"
static PyObject *
_wrap_gst_pad_query_peer_position (PyGObject *self, PyObject *args)
{
    gint64    cur;
    gint    format;
    PyObject    *pformat;
    PyObject    *ret;

    pformat = (PyObject*)PyTuple_GetItem(args, 0);
    if (pyg_enum_get_value (GST_TYPE_FORMAT, pformat, &format)) {
        PyErr_SetString(PyExc_TypeError, "argument should be a GstFormat");
        return NULL;
    }

    if ((gst_pad_query_peer_position(GST_PAD (self->obj), (GstFormat*) &format, &cur))) {
        ret = PyList_New(2);
        PyList_SetItem(ret, 0, PyLong_FromLongLong(cur));
        PyList_SetItem(ret, 1, pyg_enum_from_gtype (GST_TYPE_FORMAT, format ));
    } else {
        Py_INCREF(Py_None);
        ret = Py_None;
    }

    return ret;
}
#line 10441 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 1499 "gstpad.override"
static PyObject *
_wrap_gst_pad_query_peer_duration (PyGObject *self, PyObject *args)
{
    gint64    cur;
    gint    format;
    PyObject    *pformat;
    PyObject    *ret;

    pformat = (PyObject*)PyTuple_GetItem(args, 0);
    if (pyg_enum_get_value (GST_TYPE_FORMAT, pformat, &format)) {
        PyErr_SetString(PyExc_TypeError, "argument should be a GstFormat");
        return NULL;
    }

    if ((gst_pad_query_peer_duration(GST_PAD (self->obj), (GstFormat*) &format, &cur))) {
        ret = PyList_New(2);
        PyList_SetItem(ret, 0, PyLong_FromLongLong(cur));
        PyList_SetItem(ret, 1, pyg_enum_from_gtype (GST_TYPE_FORMAT, format ));
    } else {
        Py_INCREF(Py_None);
        ret = Py_None;
    }

    return ret;
}
#line 10470 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 1526 "gstpad.override"
static PyObject *
_wrap_gst_pad_query_peer_convert (PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "fromformat", "fromvalue", "destformat", NULL };
    PyObject    *pfromformat, *pdestformat;
    GstFormat    srcformat, destformat;
    gint64    fromval, dstval;
    PyObject    *ret;

    /* Input : src_format, src_val, dst_format */
    /* Returns : dst_format, dst_val OR None */

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                     "0L0:GstPad.query_peer_convert",
                     kwlist, &pfromformat, &fromval, &pdestformat))
    return NULL;
    if (pyg_enum_get_value(GST_TYPE_FORMAT, pfromformat, (gint *) &srcformat)) {
    PyErr_SetString(PyExc_TypeError, "argument should be a GstFormat");
    return NULL;
    }
    if (pyg_enum_get_value(GST_TYPE_FORMAT, pdestformat, (gint *) &destformat)) {
    PyErr_SetString(PyExc_TypeError, "argument should be a GstFormat");
    return NULL;
    }

    if (!(gst_pad_query_peer_convert (GST_PAD(self->obj),
                 srcformat, fromval,
                 &destformat, &dstval))) {
    Py_INCREF(Py_None);
    return Py_None;
    }
    
    ret = PyList_New(2);
    PyList_SetItem(ret, 0, pyg_enum_from_gtype (GST_TYPE_FORMAT, destformat));
    PyList_SetItem(ret, 1, PyLong_FromLongLong(dstval));

    return ret;
}
#line 10512 "..\\..\\Source\\gst-python\\gst\\gst.c"


static const PyMethodDef _PyGstPad_methods[] = {
    { "get_direction", (PyCFunction)_wrap_gst_pad_get_direction, METH_NOARGS,
      NULL },
    { "set_active", (PyCFunction)_wrap_gst_pad_set_active, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "is_active", (PyCFunction)_wrap_gst_pad_is_active, METH_NOARGS,
      NULL },
    { "activate_pull", (PyCFunction)_wrap_gst_pad_activate_pull, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "activate_push", (PyCFunction)_wrap_gst_pad_activate_push, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_blocked", (PyCFunction)_wrap_gst_pad_set_blocked, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_blocked_async", (PyCFunction)_wrap_gst_pad_set_blocked_async, METH_VARARGS,
      NULL },
    { "is_blocked", (PyCFunction)_wrap_gst_pad_is_blocked, METH_NOARGS,
      NULL },
    { "get_pad_template", (PyCFunction)_wrap_gst_pad_get_pad_template, METH_NOARGS,
      NULL },
    { "alloc_buffer", (PyCFunction)_wrap_gst_pad_alloc_buffer, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "alloc_buffer_and_set_caps", (PyCFunction)_wrap_gst_pad_alloc_buffer_and_set_caps, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_activate_function", (PyCFunction)_wrap_gst_pad_set_activate_function, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_activatepull_function", (PyCFunction)_wrap_gst_pad_set_activatepull_function, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_activatepush_function", (PyCFunction)_wrap_gst_pad_set_activatepush_function, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_chain_function", (PyCFunction)_wrap_gst_pad_set_chain_function, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_event_function", (PyCFunction)_wrap_gst_pad_set_event_function, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_link_function", (PyCFunction)_wrap_gst_pad_set_link_function, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "link", (PyCFunction)_wrap_gst_pad_link, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "unlink", (PyCFunction)_wrap_gst_pad_unlink, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "is_linked", (PyCFunction)_wrap_gst_pad_is_linked, METH_NOARGS,
      NULL },
    { "get_peer", (PyCFunction)_wrap_gst_pad_get_peer, METH_NOARGS,
      NULL },
    { "set_getcaps_function", (PyCFunction)_wrap_gst_pad_set_getcaps_function, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_setcaps_function", (PyCFunction)_wrap_gst_pad_set_setcaps_function, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_pad_template_caps", (PyCFunction)_wrap_gst_pad_get_pad_template_caps, METH_NOARGS,
      NULL },
    { "get_caps", (PyCFunction)_wrap_gst_pad_get_caps, METH_NOARGS,
      NULL },
    { "fixate_caps", (PyCFunction)_wrap_gst_pad_fixate_caps, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "accept_caps", (PyCFunction)_wrap_gst_pad_accept_caps, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_caps", (PyCFunction)_wrap_gst_pad_set_caps, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "peer_get_caps", (PyCFunction)_wrap_gst_pad_peer_get_caps, METH_NOARGS,
      NULL },
    { "peer_accept_caps", (PyCFunction)_wrap_gst_pad_peer_accept_caps, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_allowed_caps", (PyCFunction)_wrap_gst_pad_get_allowed_caps, METH_NOARGS,
      NULL },
    { "get_negotiated_caps", (PyCFunction)_wrap_gst_pad_get_negotiated_caps, METH_NOARGS,
      NULL },
    { "push", (PyCFunction)_wrap_gst_pad_push, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "check_pull_range", (PyCFunction)_wrap_gst_pad_check_pull_range, METH_NOARGS,
      NULL },
    { "pull_range", (PyCFunction)_wrap_gst_pad_pull_range, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "push_event", (PyCFunction)_wrap_gst_pad_push_event, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "event_default", (PyCFunction)_wrap_gst_pad_event_default, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "chain", (PyCFunction)_wrap_gst_pad_chain, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_range", (PyCFunction)_wrap_gst_pad_get_range, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "send_event", (PyCFunction)_wrap_gst_pad_send_event, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "start_task", (PyCFunction)_wrap_gst_pad_start_task, METH_VARARGS,
      NULL },
    { "pause_task", (PyCFunction)_wrap_gst_pad_pause_task, METH_NOARGS,
      NULL },
    { "stop_task", (PyCFunction)_wrap_gst_pad_stop_task, METH_NOARGS,
      NULL },
    { "get_internal_links", (PyCFunction)_wrap_gst_pad_get_internal_links, METH_NOARGS,
      NULL },
    { "get_internal_links_default", (PyCFunction)_wrap_gst_pad_get_internal_links_default, METH_NOARGS,
      NULL },
    { "iterate_internal_links", (PyCFunction)_wrap_gst_pad_iterate_internal_links, METH_NOARGS,
      NULL },
    { "iterate_internal_links_default", (PyCFunction)_wrap_gst_pad_iterate_internal_links_default, METH_NOARGS,
      NULL },
    { "get_query_types", (PyCFunction)_wrap_gst_pad_get_query_types, METH_NOARGS,
      NULL },
    { "get_query_types_default", (PyCFunction)_wrap_gst_pad_get_query_types_default, METH_NOARGS,
      NULL },
    { "query", (PyCFunction)_wrap_gst_pad_query, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "peer_query", (PyCFunction)_wrap_gst_pad_peer_query, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_query_function", (PyCFunction)_wrap_gst_pad_set_query_function, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "query_default", (PyCFunction)_wrap_gst_pad_query_default, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "add_data_probe", (PyCFunction)_wrap_gst_pad_add_data_probe, METH_VARARGS,
      NULL },
    { "remove_data_probe", (PyCFunction)_wrap_gst_pad_remove_data_probe, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "add_event_probe", (PyCFunction)_wrap_gst_pad_add_event_probe, METH_VARARGS,
      NULL },
    { "remove_event_probe", (PyCFunction)_wrap_gst_pad_remove_event_probe, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "add_buffer_probe", (PyCFunction)_wrap_gst_pad_add_buffer_probe, METH_VARARGS,
      NULL },
    { "remove_buffer_probe", (PyCFunction)_wrap_gst_pad_remove_buffer_probe, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "can_link", (PyCFunction)_wrap_gst_pad_can_link, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "use_fixed_caps", (PyCFunction)_wrap_gst_pad_use_fixed_caps, METH_NOARGS,
      NULL },
    { "get_fixed_caps_func", (PyCFunction)_wrap_gst_pad_get_fixed_caps_func, METH_NOARGS,
      NULL },
    { "proxy_getcaps", (PyCFunction)_wrap_gst_pad_proxy_getcaps, METH_NOARGS,
      NULL },
    { "proxy_setcaps", (PyCFunction)_wrap_gst_pad_proxy_setcaps, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_parent_element", (PyCFunction)_wrap_gst_pad_get_parent_element, METH_NOARGS,
      NULL },
    { "query_position", (PyCFunction)_wrap_gst_pad_query_position, METH_VARARGS,
      NULL },
    { "query_duration", (PyCFunction)_wrap_gst_pad_query_duration, METH_VARARGS,
      NULL },
    { "query_convert", (PyCFunction)_wrap_gst_pad_query_convert, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "query_peer_position", (PyCFunction)_wrap_gst_pad_query_peer_position, METH_VARARGS,
      NULL },
    { "query_peer_duration", (PyCFunction)_wrap_gst_pad_query_peer_duration, METH_VARARGS,
      NULL },
    { "query_peer_convert", (PyCFunction)_wrap_gst_pad_query_peer_convert, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

#line 648 "gstpad.override"
static PyObject *
_wrap_gst_pad_tp_repr (PyGObject *self)
{
    char *buf;
    PyObject *retval;
    GstPad *pad;
    GstElement *parent;
    gchar *padname = NULL;
    gchar *elementname = NULL;

    pad = GST_PAD(self->obj);

    pyg_begin_allow_threads;
    padname = gst_pad_get_name (pad);

    parent = GST_ELEMENT (gst_pad_get_parent (pad));
    if (parent)
	    elementname = gst_element_get_name (parent);

    buf = g_strdup_printf ("<GstPad (%s:%s) at %lx>",
        parent ? elementname : "---",
        padname, (long) self->obj);

    g_free(padname);

    if (parent) {
        gst_object_unref (parent);
	    g_free(elementname);
    }
    pyg_end_allow_threads;

    retval = PyString_FromString(buf);
    g_free(buf);

    return retval;
}

#line 10699 "..\\..\\Source\\gst-python\\gst\\gst.c"


PyTypeObject PyGstPad_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.Pad",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)_wrap_gst_pad_tp_repr,             /* tp_repr */
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
    (struct PyMethodDef*)_PyGstPad_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_gst_pad_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GstGhostPad ----------- */

static int
_wrap_gst_ghost_pad_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", "target", NULL };
    char *name;
    PyGObject *target;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"sO!:GstGhostPad.__init__", kwlist, &name, &PyGstPad_Type, &target))
        return -1;
    self->obj = (GObject *)gst_ghost_pad_new(name, GST_PAD(target->obj));

    if (!self->obj) {
        PyErr_SetString(PyExc_RuntimeError, "could not create GstGhostPad object");
        return -1;
    }
    pygobject_register_wrapper((PyObject *)self);
    return 0;
}

static PyObject *
_wrap_gst_ghost_pad_get_target(PyGObject *self)
{
    PyObject *py_ret;
    GstPad *ret;

    pyg_begin_allow_threads;
    ret = gst_ghost_pad_get_target(GST_GHOST_PAD(self->obj));
    pyg_end_allow_threads;
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref((GObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_ghost_pad_set_target(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "newtarget", NULL };
    PyGObject *newtarget;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstGhostPad.set_target", kwlist, &PyGstPad_Type, &newtarget))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_ghost_pad_set_target(GST_GHOST_PAD(self->obj), GST_PAD(newtarget->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static const PyMethodDef _PyGstGhostPad_methods[] = {
    { "get_target", (PyCFunction)_wrap_gst_ghost_pad_get_target, METH_NOARGS,
      NULL },
    { "set_target", (PyCFunction)_wrap_gst_ghost_pad_set_target, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject PyGstGhostPad_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.GhostPad",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGstGhostPad_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_gst_ghost_pad_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GstPadTemplate ----------- */

static int
_wrap_gst_pad_template_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name_template", "direction", "presence", "caps", NULL };
    char *name_template;
    PyObject *py_direction = NULL, *py_presence = NULL, *py_caps;
    GstPadDirection direction;
    GstCaps *caps;
    GstPadPresence presence;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"sOOO:GstPadTemplate.__init__", kwlist, &name_template, &py_direction, &py_presence, &py_caps))
        return -1;
    if (pyg_enum_get_value(GST_TYPE_PAD_DIRECTION, py_direction, (gint *)&direction))
        return -1;
    if (pyg_enum_get_value(GST_TYPE_PAD_PRESENCE, py_presence, (gint *)&presence))
        return -1;
    caps = pygst_caps_from_pyobject (py_caps, NULL);
    if (PyErr_Occurred())
      return -1;
    self->obj = (GObject *)gst_pad_template_new(name_template, direction, presence, caps);

    if (!self->obj) {
        PyErr_SetString(PyExc_RuntimeError, "could not create GstPadTemplate object");
        return -1;
    }
    pygobject_register_wrapper((PyObject *)self);
    return 0;
}

#line 791 "gstpad.override"
static PyObject *
_wrap_gst_pad_template_get_caps(PyGObject *self)
{
    GstCaps *ret = (GstCaps*)gst_pad_template_get_caps(GST_PAD_TEMPLATE(self->obj));
    return pyg_boxed_new(GST_TYPE_CAPS, ret, TRUE, TRUE);
}
#line 10894 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_pad_template_pad_created(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "pad", NULL };
    PyGObject *pad;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstPadTemplate.pad_created", kwlist, &PyGstPad_Type, &pad))
        return NULL;
    pyg_begin_allow_threads;
    gst_pad_template_pad_created(GST_PAD_TEMPLATE(self->obj), GST_PAD(pad->obj));
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGstPadTemplate_methods[] = {
    { "get_caps", (PyCFunction)_wrap_gst_pad_template_get_caps, METH_NOARGS,
      NULL },
    { "pad_created", (PyCFunction)_wrap_gst_pad_template_pad_created, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

static PyObject *
_wrap_gst_pad_template__get_name_template(PyObject *self, void *closure)
{
    const gchar *ret;

    ret = GST_PAD_TEMPLATE(pygobject_get(self))->name_template;
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_pad_template__get_direction(PyObject *self, void *closure)
{
    gint ret;

    ret = GST_PAD_TEMPLATE(pygobject_get(self))->direction;
    return pyg_enum_from_gtype(GST_TYPE_PAD_DIRECTION, ret);
}

static PyObject *
_wrap_gst_pad_template__get_presence(PyObject *self, void *closure)
{
    gint ret;

    ret = GST_PAD_TEMPLATE(pygobject_get(self))->presence;
    return pyg_enum_from_gtype(GST_TYPE_PAD_PRESENCE, ret);
}

static PyObject *
_wrap_gst_pad_template__get_caps(PyObject *self, void *closure)
{
    GstCaps *ret;

    ret = GST_PAD_TEMPLATE(pygobject_get(self))->caps;
    return pyg_boxed_new (GST_TYPE_CAPS, ret, FALSE, TRUE);
}

static const PyGetSetDef gst_pad_template_getsets[] = {
    { "name_template", (getter)_wrap_gst_pad_template__get_name_template, (setter)0 },
    { "direction", (getter)_wrap_gst_pad_template__get_direction, (setter)0 },
    { "presence", (getter)_wrap_gst_pad_template__get_presence, (setter)0 },
    { "caps", (getter)_wrap_gst_pad_template__get_caps, (setter)0 },
    { NULL, (getter)0, (setter)0 },
};

#line 1027 "gstpad.override"
#define IS_ATTR(name) (strcmp (name, attr) == 0)
static PyObject *
_wrap_gst_pad_template_tp_getattr(PyObject *self, char *attr)
{
  GstPadTemplate *templ = GST_PAD_TEMPLATE (pygobject_get (self));
  
  if (IS_ATTR ("__members__")) {
    return Py_BuildValue("[ssss]", "name_template", "direction", "presence", "caps" );
  } else if (IS_ATTR ("name_template")) {
    return PyString_FromString (GST_PAD_TEMPLATE_NAME_TEMPLATE(templ));
  } else if (IS_ATTR ("direction")) {
    return pyg_enum_from_gtype(GST_TYPE_PAD_DIRECTION, GST_PAD_TEMPLATE_DIRECTION(templ));
  } else if (IS_ATTR ("presence")) {
    return pyg_enum_from_gtype(GST_TYPE_PAD_PRESENCE, GST_PAD_TEMPLATE_PRESENCE(templ));
  } else if (IS_ATTR ("caps")) {
    return pyg_boxed_new (GST_TYPE_CAPS, GST_PAD_TEMPLATE_CAPS(templ), TRUE, TRUE);
  }
  return Py_FindMethod((PyMethodDef*) _PyGstPadTemplate_methods, self, attr);
}
#line 10987 "..\\..\\Source\\gst-python\\gst\\gst.c"


PyTypeObject PyGstPadTemplate_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.PadTemplate",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)_wrap_gst_pad_template_tp_getattr,       /* tp_getattr */
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
    (struct PyMethodDef*)_PyGstPadTemplate_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)gst_pad_template_getsets,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_gst_pad_template_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GstPipeline ----------- */

static int
_wrap_gst_pipeline_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    GType obj_type = pyg_type_from_object((PyObject *) self);
    GParameter params[1];
    PyObject *parsed_args[1] = {NULL, };
    char *arg_names[] = {"name", NULL };
    char *prop_names[] = {"name", NULL };
    guint nparams, i;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|O:gst.Pipeline.__init__" , arg_names , &parsed_args[0]))
        return -1;

    memset(params, 0, sizeof(GParameter)*1);
    if (!pyg_parse_constructor_args(obj_type, arg_names,
                                    prop_names, params, 
                                    &nparams, parsed_args))
        return -1;
    pygobject_constructv(self, nparams, params);
    for (i = 0; i < nparams; ++i)
        g_value_unset(&params[i].value);
    if (!self->obj) {
        PyErr_SetString(
            PyExc_RuntimeError, 
            "could not create gst.Pipeline object");
        return -1;
    }
    return 0;
}

static PyObject *
_wrap_gst_pipeline_get_bus(PyGObject *self)
{
    PyObject *py_ret;
    GstBus *ret;

    pyg_begin_allow_threads;
    ret = gst_pipeline_get_bus(GST_PIPELINE(self->obj));
    pyg_end_allow_threads;
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref((GObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_pipeline_set_new_stream_time(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "time", NULL };
    guint64 time;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"K:GstPipeline.set_new_stream_time", kwlist, &time))
        return NULL;
    pyg_begin_allow_threads;
    gst_pipeline_set_new_stream_time(GST_PIPELINE(self->obj), time);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_pipeline_get_last_stream_time(PyGObject *self)
{
    guint64 ret;

    pyg_begin_allow_threads;
    ret = gst_pipeline_get_last_stream_time(GST_PIPELINE(self->obj));
    pyg_end_allow_threads;
    return PyLong_FromUnsignedLongLong(ret);
}

static PyObject *
_wrap_gst_pipeline_use_clock(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "clock", NULL };
    PyGObject *clock;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstPipeline.use_clock", kwlist, &PyGstClock_Type, &clock))
        return NULL;
    pyg_begin_allow_threads;
    gst_pipeline_use_clock(GST_PIPELINE(self->obj), GST_CLOCK(clock->obj));
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_pipeline_set_clock(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "clock", NULL };
    PyGObject *clock;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstPipeline.set_clock", kwlist, &PyGstClock_Type, &clock))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_pipeline_set_clock(GST_PIPELINE(self->obj), GST_CLOCK(clock->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_pipeline_get_clock(PyGObject *self)
{
    PyObject *py_ret;
    GstClock *ret;

    pyg_begin_allow_threads;
    ret = gst_pipeline_get_clock(GST_PIPELINE(self->obj));
    pyg_end_allow_threads;
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref((GObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_pipeline_auto_clock(PyGObject *self)
{
    pyg_begin_allow_threads;
    gst_pipeline_auto_clock(GST_PIPELINE(self->obj));
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_pipeline_set_delay(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "delay", NULL };
    guint64 delay;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"K:GstPipeline.set_delay", kwlist, &delay))
        return NULL;
    pyg_begin_allow_threads;
    gst_pipeline_set_delay(GST_PIPELINE(self->obj), delay);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_pipeline_get_delay(PyGObject *self)
{
    guint64 ret;

    pyg_begin_allow_threads;
    ret = gst_pipeline_get_delay(GST_PIPELINE(self->obj));
    pyg_end_allow_threads;
    return PyLong_FromUnsignedLongLong(ret);
}

static PyObject *
_wrap_gst_pipeline_set_auto_flush_bus(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "auto_flush", NULL };
    int auto_flush;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GstPipeline.set_auto_flush_bus", kwlist, &auto_flush))
        return NULL;
    pyg_begin_allow_threads;
    gst_pipeline_set_auto_flush_bus(GST_PIPELINE(self->obj), auto_flush);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_pipeline_get_auto_flush_bus(PyGObject *self)
{
    int ret;

    pyg_begin_allow_threads;
    ret = gst_pipeline_get_auto_flush_bus(GST_PIPELINE(self->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static const PyMethodDef _PyGstPipeline_methods[] = {
    { "get_bus", (PyCFunction)_wrap_gst_pipeline_get_bus, METH_NOARGS,
      NULL },
    { "set_new_stream_time", (PyCFunction)_wrap_gst_pipeline_set_new_stream_time, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_last_stream_time", (PyCFunction)_wrap_gst_pipeline_get_last_stream_time, METH_NOARGS,
      NULL },
    { "use_clock", (PyCFunction)_wrap_gst_pipeline_use_clock, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_clock", (PyCFunction)_wrap_gst_pipeline_set_clock, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_clock", (PyCFunction)_wrap_gst_pipeline_get_clock, METH_NOARGS,
      NULL },
    { "auto_clock", (PyCFunction)_wrap_gst_pipeline_auto_clock, METH_NOARGS,
      NULL },
    { "set_delay", (PyCFunction)_wrap_gst_pipeline_set_delay, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_delay", (PyCFunction)_wrap_gst_pipeline_get_delay, METH_NOARGS,
      NULL },
    { "set_auto_flush_bus", (PyCFunction)_wrap_gst_pipeline_set_auto_flush_bus, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_auto_flush_bus", (PyCFunction)_wrap_gst_pipeline_get_auto_flush_bus, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject PyGstPipeline_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.Pipeline",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGstPipeline_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_gst_pipeline_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GstPluginFeature ----------- */

static PyObject *
_wrap_gst_plugin_feature_load(PyGObject *self)
{
    GstPluginFeature *ret;

    pyg_begin_allow_threads;
    ret = gst_plugin_feature_load(GST_PLUGIN_FEATURE(self->obj));
    pyg_end_allow_threads;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gst_plugin_feature_set_rank(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "rank", NULL };
    PyObject *py_rank = NULL;
    guint rank = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:GstPluginFeature.set_rank", kwlist, &py_rank))
        return NULL;
    if (py_rank) {
        if (PyLong_Check(py_rank))
            rank = PyLong_AsUnsignedLong(py_rank);
        else if (PyInt_Check(py_rank))
            rank = PyInt_AsLong(py_rank);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'rank' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    pyg_begin_allow_threads;
    gst_plugin_feature_set_rank(GST_PLUGIN_FEATURE(self->obj), rank);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_plugin_feature_set_name(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", NULL };
    char *name;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:GstPluginFeature.set_name", kwlist, &name))
        return NULL;
    pyg_begin_allow_threads;
    gst_plugin_feature_set_name(GST_PLUGIN_FEATURE(self->obj), name);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_plugin_feature_get_rank(PyGObject *self)
{
    guint ret;

    pyg_begin_allow_threads;
    ret = gst_plugin_feature_get_rank(GST_PLUGIN_FEATURE(self->obj));
    pyg_end_allow_threads;
    return PyLong_FromUnsignedLong(ret);
}

static PyObject *
_wrap_gst_plugin_feature_get_name(PyGObject *self)
{
    const gchar *ret;

    pyg_begin_allow_threads;
    ret = gst_plugin_feature_get_name(GST_PLUGIN_FEATURE(self->obj));
    pyg_end_allow_threads;
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_plugin_feature_check_version(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "min_major", "min_minor", "min_micro", NULL };
    PyObject *py_min_major = NULL, *py_min_minor = NULL, *py_min_micro = NULL;
    int ret;
    guint min_major = 0, min_minor = 0, min_micro = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"OOO:GstPluginFeature.check_version", kwlist, &py_min_major, &py_min_minor, &py_min_micro))
        return NULL;
    if (py_min_major) {
        if (PyLong_Check(py_min_major))
            min_major = PyLong_AsUnsignedLong(py_min_major);
        else if (PyInt_Check(py_min_major))
            min_major = PyInt_AsLong(py_min_major);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'min_major' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    if (py_min_minor) {
        if (PyLong_Check(py_min_minor))
            min_minor = PyLong_AsUnsignedLong(py_min_minor);
        else if (PyInt_Check(py_min_minor))
            min_minor = PyInt_AsLong(py_min_minor);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'min_minor' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    if (py_min_micro) {
        if (PyLong_Check(py_min_micro))
            min_micro = PyLong_AsUnsignedLong(py_min_micro);
        else if (PyInt_Check(py_min_micro))
            min_micro = PyInt_AsLong(py_min_micro);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'min_micro' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    pyg_begin_allow_threads;
    ret = gst_plugin_feature_check_version(GST_PLUGIN_FEATURE(self->obj), min_major, min_minor, min_micro);
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static const PyMethodDef _PyGstPluginFeature_methods[] = {
    { "load", (PyCFunction)_wrap_gst_plugin_feature_load, METH_NOARGS,
      NULL },
    { "set_rank", (PyCFunction)_wrap_gst_plugin_feature_set_rank, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_name", (PyCFunction)_wrap_gst_plugin_feature_set_name, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_rank", (PyCFunction)_wrap_gst_plugin_feature_get_rank, METH_NOARGS,
      NULL },
    { "get_name", (PyCFunction)_wrap_gst_plugin_feature_get_name, METH_NOARGS,
      NULL },
    { "check_version", (PyCFunction)_wrap_gst_plugin_feature_check_version, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

#line 324 "..\\..\\Source\\gst-python\\gst\\gst.override"
static PyObject *
_wrap_gst_plugin_feature_tp_repr(PyObject *self)
{
    gchar *repr;
    PyObject *ret;
    GstPluginFeature *feature = GST_PLUGIN_FEATURE (pygobject_get (self));

    repr = g_strdup_printf ("<%s %s @ 0x%lx>",
	       self->ob_type->tp_name, gst_plugin_feature_get_name (feature),
	       (long) self);
    ret = PyString_FromString(repr);
    g_free (repr);
    return ret;
}
#line 11450 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 340 "..\\..\\Source\\gst-python\\gst\\gst.override"
static PyObject *
_wrap_gst_plugin_feature_tp_str(PyObject *self)
{
    gchar *repr;
    PyObject *ret;
    GstPluginFeature *feature = GST_PLUGIN_FEATURE (pygobject_get (self));

    repr = g_strdup_printf ("<%s %s (%d)>",
	       self->ob_type->tp_name, gst_plugin_feature_get_name (feature),
	       gst_plugin_feature_get_rank (feature));
    ret = PyString_FromString(repr);
    g_free (repr);
    return ret;
}

#line 11469 "..\\..\\Source\\gst-python\\gst\\gst.c"


PyTypeObject PyGstPluginFeature_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.PluginFeature",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)_wrap_gst_plugin_feature_tp_repr,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)_wrap_gst_plugin_feature_tp_str,              /* tp_str */
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
    (struct PyMethodDef*)_PyGstPluginFeature_methods, /* tp_methods */
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



/* ----------- GstIndexFactory ----------- */

 static int
_wrap_gst_index_factory_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    GType obj_type = pyg_type_from_object((PyObject *) self);
    GParameter params[3];
    PyObject *parsed_args[3] = {NULL, };
    char *arg_names[] = {"name", "longdesc", "type", NULL };
    char *prop_names[] = {"name", "longdesc", "type", NULL };
    guint nparams, i;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OOO:gst.IndexFactory.__init__" , arg_names , &parsed_args[0] , &parsed_args[1] , &parsed_args[2]))
        return -1;

    memset(params, 0, sizeof(GParameter)*3);
    if (!pyg_parse_constructor_args(obj_type, arg_names,
                                    prop_names, params, 
                                    &nparams, parsed_args))
        return -1;
    pygobject_constructv(self, nparams, params);
    for (i = 0; i < nparams; ++i)
        g_value_unset(&params[i].value);
    if (!self->obj) {
        PyErr_SetString(
            PyExc_RuntimeError, 
            "could not create gst.IndexFactory object");
        return -1;
    }
    return 0;
}

static PyObject *
_wrap_gst_index_factory_destroy(PyGObject *self)
{
    pyg_begin_allow_threads;
    gst_index_factory_destroy(GST_INDEX_FACTORY(self->obj));
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_index_factory_create(PyGObject *self)
{
    PyObject *py_ret;
    GstIndex *ret;

    pyg_begin_allow_threads;
    ret = gst_index_factory_create(GST_INDEX_FACTORY(self->obj));
    pyg_end_allow_threads;
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref((GObject *)ret);
    return py_ret;
}

static const PyMethodDef _PyGstIndexFactory_methods[] = {
    { "destroy", (PyCFunction)_wrap_gst_index_factory_destroy, METH_NOARGS,
      NULL },
    { "create", (PyCFunction)_wrap_gst_index_factory_create, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject PyGstIndexFactory_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.IndexFactory",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGstIndexFactory_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_gst_index_factory_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GstElementFactory ----------- */

static PyObject *
_wrap_gst_element_factory_get_element_type(PyGObject *self)
{
    GType ret;

    pyg_begin_allow_threads;
    ret = gst_element_factory_get_element_type(GST_ELEMENT_FACTORY(self->obj));
    pyg_end_allow_threads;
    return pyg_type_wrapper_new(ret);
}

static PyObject *
_wrap_gst_element_factory_get_longname(PyGObject *self)
{
    const gchar *ret;

    pyg_begin_allow_threads;
    ret = gst_element_factory_get_longname(GST_ELEMENT_FACTORY(self->obj));
    pyg_end_allow_threads;
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_element_factory_get_klass(PyGObject *self)
{
    const gchar *ret;

    pyg_begin_allow_threads;
    ret = gst_element_factory_get_klass(GST_ELEMENT_FACTORY(self->obj));
    pyg_end_allow_threads;
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_element_factory_get_description(PyGObject *self)
{
    const gchar *ret;

    pyg_begin_allow_threads;
    ret = gst_element_factory_get_description(GST_ELEMENT_FACTORY(self->obj));
    pyg_end_allow_threads;
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_element_factory_get_author(PyGObject *self)
{
    const gchar *ret;

    pyg_begin_allow_threads;
    ret = gst_element_factory_get_author(GST_ELEMENT_FACTORY(self->obj));
    pyg_end_allow_threads;
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_element_factory_get_num_pad_templates(PyGObject *self)
{
    guint ret;

    pyg_begin_allow_threads;
    ret = gst_element_factory_get_num_pad_templates(GST_ELEMENT_FACTORY(self->obj));
    pyg_end_allow_threads;
    return PyLong_FromUnsignedLong(ret);
}

#line 51 "gstelementfactory.override"
static PyObject *
_wrap_gst_element_factory_get_static_pad_templates(PyGObject *self)
{
  const GList *list;
  GList	*l;
  PyObject *py_list;
  int i = 0;

  pyg_begin_allow_threads;
  list = gst_element_factory_get_static_pad_templates (GST_ELEMENT_FACTORY (self->obj));
  pyg_end_allow_threads;

  py_list = PyList_New(g_list_length ((GList*) list));

  for (l = (GList*) list; l ; l = g_list_next(l), i++) {
    GstStaticPadTemplate *templ = (GstStaticPadTemplate*) l->data;
    PyList_SetItem(py_list, i, pyg_pointer_new(GST_TYPE_STATIC_PAD_TEMPLATE, (gpointer) templ));
  }
  return py_list;
}
#line 11732 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_element_factory_get_uri_type(PyGObject *self)
{
    gint ret;

    pyg_begin_allow_threads;
    ret = gst_element_factory_get_uri_type(GST_ELEMENT_FACTORY(self->obj));
    pyg_end_allow_threads;
    return pyg_enum_from_gtype(GST_TYPE_URI_TYPE, ret);
}

static PyObject *
_wrap_gst_element_factory_get_uri_protocols(PyGObject *self)
{
    gchar **ret;

    pyg_begin_allow_threads;
    ret = gst_element_factory_get_uri_protocols(GST_ELEMENT_FACTORY(self->obj));
    pyg_end_allow_threads;
    if (ret) {
        guint size = g_strv_length(ret);
        PyObject *py_ret = PyTuple_New(size);
        gint i;
        for (i = 0; i < size; i++)
            PyTuple_SetItem(py_ret, i,
                PyString_FromString(ret[i]));
        return py_ret;
    }
    return PyTuple_New (0);

}

static PyObject *
_wrap_gst_element_factory_has_interface(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "interfacename", NULL };
    char *interfacename;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:GstElementFactory.has_interface", kwlist, &interfacename))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_element_factory_has_interface(GST_ELEMENT_FACTORY(self->obj), interfacename);
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_element_factory_create(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", NULL };
    char *name = NULL;
    PyObject *py_ret;
    GstElement *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"|z:GstElementFactory.create", kwlist, &name))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_element_factory_create(GST_ELEMENT_FACTORY(self->obj), name);
    pyg_end_allow_threads;
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref((GObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_element_factory_can_src_caps(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "caps", NULL };
    PyObject *py_caps;
    int ret;
    gboolean caps_is_copy;
    GstCaps *caps;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:GstElementFactory.can_src_caps", kwlist, &py_caps))
        return NULL;
    caps = pygst_caps_from_pyobject (py_caps, &caps_is_copy);
    if (PyErr_Occurred())
      return NULL;
    pyg_begin_allow_threads;
    ret = gst_element_factory_can_src_caps(GST_ELEMENT_FACTORY(self->obj), caps);
    pyg_end_allow_threads;
    if (caps && caps_is_copy)
        gst_caps_unref (caps);
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_element_factory_can_sink_caps(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "caps", NULL };
    PyObject *py_caps;
    int ret;
    gboolean caps_is_copy;
    GstCaps *caps;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:GstElementFactory.can_sink_caps", kwlist, &py_caps))
        return NULL;
    caps = pygst_caps_from_pyobject (py_caps, &caps_is_copy);
    if (PyErr_Occurred())
      return NULL;
    pyg_begin_allow_threads;
    ret = gst_element_factory_can_sink_caps(GST_ELEMENT_FACTORY(self->obj), caps);
    pyg_end_allow_threads;
    if (caps && caps_is_copy)
        gst_caps_unref (caps);
    return PyBool_FromLong(ret);

}

static const PyMethodDef _PyGstElementFactory_methods[] = {
    { "get_element_type", (PyCFunction)_wrap_gst_element_factory_get_element_type, METH_NOARGS,
      NULL },
    { "get_longname", (PyCFunction)_wrap_gst_element_factory_get_longname, METH_NOARGS,
      NULL },
    { "get_klass", (PyCFunction)_wrap_gst_element_factory_get_klass, METH_NOARGS,
      NULL },
    { "get_description", (PyCFunction)_wrap_gst_element_factory_get_description, METH_NOARGS,
      NULL },
    { "get_author", (PyCFunction)_wrap_gst_element_factory_get_author, METH_NOARGS,
      NULL },
    { "get_num_pad_templates", (PyCFunction)_wrap_gst_element_factory_get_num_pad_templates, METH_NOARGS,
      NULL },
    { "get_static_pad_templates", (PyCFunction)_wrap_gst_element_factory_get_static_pad_templates, METH_NOARGS,
      NULL },
    { "get_uri_type", (PyCFunction)_wrap_gst_element_factory_get_uri_type, METH_NOARGS,
      NULL },
    { "get_uri_protocols", (PyCFunction)_wrap_gst_element_factory_get_uri_protocols, METH_NOARGS,
      NULL },
    { "has_interface", (PyCFunction)_wrap_gst_element_factory_has_interface, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "create", (PyCFunction)_wrap_gst_element_factory_create, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "can_src_caps", (PyCFunction)_wrap_gst_element_factory_can_src_caps, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "can_sink_caps", (PyCFunction)_wrap_gst_element_factory_can_sink_caps, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject PyGstElementFactory_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.ElementFactory",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGstElementFactory_methods, /* tp_methods */
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



/* ----------- GstRegistry ----------- */

static PyObject *
_wrap_gst_registry_scan_path(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "path", NULL };
    char *path;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:GstRegistry.scan_path", kwlist, &path))
        return NULL;
    pyg_begin_allow_threads;
    gst_registry_scan_path(GST_REGISTRY(self->obj), path);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_registry_add_path(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "path", NULL };
    char *path;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:GstRegistry.add_path", kwlist, &path))
        return NULL;
    pyg_begin_allow_threads;
    gst_registry_add_path(GST_REGISTRY(self->obj), path);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

#line 448 "..\\..\\Source\\gst-python\\gst\\gst.override"
static PyObject *
_wrap_gst_registry_get_path_list (PyGObject *self)
{
    GstRegistry *registry;
    GList *l, *paths;
    PyObject *list;
    gint i;
    
    registry = GST_REGISTRY (self->obj);
    
    paths = gst_registry_get_path_list (registry);
    
    list = PyList_New (g_list_length(paths));
    for (l = paths, i = 0; l; l = l->next, ++i) {
	gchar *path = (gchar *) l->data;
	PyList_SetItem (list, i, PyString_FromString(path));
    }
    g_list_free (paths);
    
    return list;
}

#line 11980 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_registry_add_plugin(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "plugin", NULL };
    PyGObject *plugin;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstRegistry.add_plugin", kwlist, &PyGstPlugin_Type, &plugin))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_registry_add_plugin(GST_REGISTRY(self->obj), GST_PLUGIN(plugin->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_registry_remove_plugin(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "plugin", NULL };
    PyGObject *plugin;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstRegistry.remove_plugin", kwlist, &PyGstPlugin_Type, &plugin))
        return NULL;
    pyg_begin_allow_threads;
    gst_registry_remove_plugin(GST_REGISTRY(self->obj), GST_PLUGIN(plugin->obj));
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

#line 472 "..\\..\\Source\\gst-python\\gst\\gst.override"
static PyObject *
_wrap_gst_registry_get_plugin_list (PyGObject *self)
{
	GstRegistry *registry;
	GList *l, *plugins;
	PyObject *list;
	gint i;
    
        registry = GST_REGISTRY (self->obj);

	plugins = gst_registry_get_plugin_list (registry);

	list = PyList_New (g_list_length(plugins));
	for (l = plugins, i = 0; l; l = l->next, ++i) {
		GstPlugin *plugin = (GstPlugin *) l->data;
		PyObject *object = pygobject_new (G_OBJECT (plugin));
		gst_object_unref (plugin);

		PyList_SetItem (list, i, object);
	}
	g_list_free (plugins);

	return list;
}

#line 12040 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_registry_find_plugin(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", NULL };
    char *name;
    PyObject *py_ret;
    GstPlugin *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:GstRegistry.find_plugin", kwlist, &name))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_registry_find_plugin(GST_REGISTRY(self->obj), name);
    pyg_end_allow_threads;
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref((GObject *)ret);
    return py_ret;
}

#line 499 "..\\..\\Source\\gst-python\\gst\\gst.override"
static PyObject *
_wrap_gst_registry_get_feature_list (PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "type", NULL };
    PyObject *py_type = NULL;
    GType type;

    GstRegistry *registry;
    GList *l, *features;
    PyObject *list;
    gint i;
	

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
            "O:GstRegistry.get_feature_list", kwlist, &py_type))
        return NULL;
    if ((type = pyg_type_from_object(py_type)) == 0)
        return NULL;

    registry = GST_REGISTRY (self->obj);

    pyg_begin_allow_threads;
    features = gst_registry_get_feature_list (registry, type);
    pyg_end_allow_threads;

    list = PyList_New (g_list_length(features));
    for (l = features, i = 0; l; l = l->next, ++i) {
        GstPluginFeature *feature = (GstPluginFeature *) l->data;
        PyList_SetItem (list, i, pygobject_new (G_OBJECT (feature)));
	gst_object_unref (feature);
    }
    g_list_free (features);
	
    return list;
}

#line 12099 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 537 "..\\..\\Source\\gst-python\\gst\\gst.override"
static PyObject *
_wrap_gst_registry_get_feature_list_by_plugin (PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", NULL };
    gchar * name = NULL;

    GstRegistry *registry;
    GList *l, *features;
    PyObject *list;
	gint i;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
            "s:GstRegistry.get_feature_list_by_plugin", kwlist, &name))
        return NULL;

    registry = GST_REGISTRY (self->obj);

    pyg_begin_allow_threads;
    features = gst_registry_get_feature_list_by_plugin (registry, name);
    pyg_end_allow_threads;

    list = PyList_New (g_list_length(features));
    for (l = features, i = 0; l; l = l->next, ++i) {
        GstPluginFeature *feature = (GstPluginFeature *) l->data;
        PyList_SetItem (list, i, pygobject_new (G_OBJECT (feature)));
    }
    g_list_free (features);
	
    return list;
}

#line 12134 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_registry_add_feature(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "feature", NULL };
    PyGObject *feature;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstRegistry.add_feature", kwlist, &PyGstPluginFeature_Type, &feature))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_registry_add_feature(GST_REGISTRY(self->obj), GST_PLUGIN_FEATURE(feature->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_registry_remove_feature(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "feature", NULL };
    PyGObject *feature;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstRegistry.remove_feature", kwlist, &PyGstPluginFeature_Type, &feature))
        return NULL;
    pyg_begin_allow_threads;
    gst_registry_remove_feature(GST_REGISTRY(self->obj), GST_PLUGIN_FEATURE(feature->obj));
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_registry_find_feature(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", "type", NULL };
    char *name;
    PyObject *py_type = NULL, *py_ret;
    GType type;
    GstPluginFeature *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"sO:GstRegistry.find_feature", kwlist, &name, &py_type))
        return NULL;
    if ((type = pyg_type_from_object(py_type)) == 0)
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_registry_find_feature(GST_REGISTRY(self->obj), name, type);
    pyg_end_allow_threads;
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref((GObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_registry_lookup_feature(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", NULL };
    char *name;
    PyObject *py_ret;
    GstPluginFeature *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:GstRegistry.lookup_feature", kwlist, &name))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_registry_lookup_feature(GST_REGISTRY(self->obj), name);
    pyg_end_allow_threads;
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref((GObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_registry_lookup(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "filename", NULL };
    char *filename;
    PyObject *py_ret;
    GstPlugin *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:GstRegistry.lookup", kwlist, &filename))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_registry_lookup(GST_REGISTRY(self->obj), filename);
    pyg_end_allow_threads;
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref((GObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_registry_xml_read_cache(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "location", NULL };
    char *location;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:GstRegistry.xml_read_cache", kwlist, &location))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_registry_xml_read_cache(GST_REGISTRY(self->obj), location);
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_registry_xml_write_cache(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "location", NULL };
    char *location;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:GstRegistry.xml_write_cache", kwlist, &location))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_registry_xml_write_cache(GST_REGISTRY(self->obj), location);
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static const PyMethodDef _PyGstRegistry_methods[] = {
    { "scan_path", (PyCFunction)_wrap_gst_registry_scan_path, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "add_path", (PyCFunction)_wrap_gst_registry_add_path, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_path_list", (PyCFunction)_wrap_gst_registry_get_path_list, METH_VARARGS,
      NULL },
    { "add_plugin", (PyCFunction)_wrap_gst_registry_add_plugin, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "remove_plugin", (PyCFunction)_wrap_gst_registry_remove_plugin, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_plugin_list", (PyCFunction)_wrap_gst_registry_get_plugin_list, METH_VARARGS,
      NULL },
    { "find_plugin", (PyCFunction)_wrap_gst_registry_find_plugin, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_feature_list", (PyCFunction)_wrap_gst_registry_get_feature_list, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_feature_list_by_plugin", (PyCFunction)_wrap_gst_registry_get_feature_list_by_plugin, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "add_feature", (PyCFunction)_wrap_gst_registry_add_feature, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "remove_feature", (PyCFunction)_wrap_gst_registry_remove_feature, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "find_feature", (PyCFunction)_wrap_gst_registry_find_feature, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "lookup_feature", (PyCFunction)_wrap_gst_registry_lookup_feature, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "lookup", (PyCFunction)_wrap_gst_registry_lookup, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "xml_read_cache", (PyCFunction)_wrap_gst_registry_xml_read_cache, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "xml_write_cache", (PyCFunction)_wrap_gst_registry_xml_write_cache, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject PyGstRegistry_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.Registry",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGstRegistry_methods, /* tp_methods */
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



/* ----------- GstSystemClock ----------- */

PyTypeObject PyGstSystemClock_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.SystemClock",                   /* tp_name */
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



/* ----------- GstTask ----------- */

static PyObject *
_wrap_gst_task_get_state(PyGObject *self)
{
    gint ret;

    pyg_begin_allow_threads;
    ret = gst_task_get_state(GST_TASK(self->obj));
    pyg_end_allow_threads;
    return pyg_enum_from_gtype(GST_TYPE_TASK_STATE, ret);
}

static PyObject *
_wrap_gst_task_start(PyGObject *self)
{
    int ret;

    pyg_begin_allow_threads;
    ret = gst_task_start(GST_TASK(self->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_task_stop(PyGObject *self)
{
    int ret;

    pyg_begin_allow_threads;
    ret = gst_task_stop(GST_TASK(self->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_task_pause(PyGObject *self)
{
    int ret;

    pyg_begin_allow_threads;
    ret = gst_task_pause(GST_TASK(self->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static const PyMethodDef _PyGstTask_methods[] = {
    { "get_state", (PyCFunction)_wrap_gst_task_get_state, METH_NOARGS,
      NULL },
    { "start", (PyCFunction)_wrap_gst_task_start, METH_NOARGS,
      NULL },
    { "stop", (PyCFunction)_wrap_gst_task_stop, METH_NOARGS,
      NULL },
    { "pause", (PyCFunction)_wrap_gst_task_pause, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject PyGstTask_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.Task",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGstTask_methods, /* tp_methods */
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



/* ----------- GstTypeFindFactory ----------- */

static PyObject *
_wrap_gst_type_find_factory_get_extensions(PyGObject *self)
{
    gchar **ret;

    pyg_begin_allow_threads;
    ret = gst_type_find_factory_get_extensions(GST_TYPE_FIND_FACTORY(self->obj));
    pyg_end_allow_threads;
    if (ret) {
        guint size = g_strv_length(ret);
        PyObject *py_ret = PyTuple_New(size);
        gint i;
        for (i = 0; i < size; i++)
            PyTuple_SetItem(py_ret, i,
                PyString_FromString(ret[i]));
        return py_ret;
    }
    return PyTuple_New (0);

}

#line 357 "..\\..\\Source\\gst-python\\gst\\gst.override"
static PyObject *
_wrap_gst_type_find_factory_get_caps(PyGObject *self)
{
	GstCaps *ret = (GstCaps*)gst_type_find_factory_get_caps(GST_TYPE_FIND_FACTORY(self->obj));
	return pyg_boxed_new(GST_TYPE_CAPS, ret, TRUE, TRUE);
}
#line 12530 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_type_find_factory_call_function(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "find", NULL };
    PyObject *py_find;
    GstTypeFind *find = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:GstTypeFindFactory.call_function", kwlist, &py_find))
        return NULL;
    if (pyg_pointer_check(py_find, GST_TYPE_TYPE_FIND))
        find = pyg_pointer_get(py_find, GstTypeFind);
    else {
        PyErr_SetString(PyExc_TypeError, "find should be a GstTypeFind");
        return NULL;
    }
    pyg_begin_allow_threads;
    gst_type_find_factory_call_function(GST_TYPE_FIND_FACTORY(self->obj), find);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGstTypeFindFactory_methods[] = {
    { "get_extensions", (PyCFunction)_wrap_gst_type_find_factory_get_extensions, METH_NOARGS,
      NULL },
    { "get_caps", (PyCFunction)_wrap_gst_type_find_factory_get_caps, METH_NOARGS,
      NULL },
    { "call_function", (PyCFunction)_wrap_gst_type_find_factory_call_function, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject PyGstTypeFindFactory_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.TypeFindFactory",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGstTypeFindFactory_methods, /* tp_methods */
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




/* ----------- GstNetClientClock ----------- */

static int
_wrap_gst_net_client_clock_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", "remote_address", "remote_port", "base_time", NULL };
    char *name = NULL, *remote_address = "127.0.0.1";
    int remote_port;
    guint64 base_time;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"|zsiK:GstNetClientClock.__init__", kwlist, &name, &remote_address, &remote_port, &base_time))
        return -1;
    self->obj = (GObject *)gst_net_client_clock_new(name, remote_address, remote_port, base_time);

    if (!self->obj) {
        PyErr_SetString(PyExc_RuntimeError, "could not create GstNetClientClock object");
        return -1;
    }
    pygobject_register_wrapper((PyObject *)self);
    return 0;
}

PyTypeObject PyGstNetClientClock_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.NetClientClock",                   /* tp_name */
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
    (initproc)_wrap_gst_net_client_clock_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GstNetTimeProvider ----------- */

static int
_wrap_gst_net_time_provider_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "clock", "address", "port", NULL };
    PyGObject *clock;
    char *address = NULL;
    int port;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!|zi:GstNetTimeProvider.__init__", kwlist, &PyGstClock_Type, &clock, &address, &port))
        return -1;
    self->obj = (GObject *)gst_net_time_provider_new(GST_CLOCK(clock->obj), address, port);

    if (!self->obj) {
        PyErr_SetString(PyExc_RuntimeError, "could not create GstNetTimeProvider object");
        return -1;
    }
    pygobject_register_wrapper((PyObject *)self);
    return 0;
}

PyTypeObject PyGstNetTimeProvider_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.NetTimeProvider",                   /* tp_name */
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
    (initproc)_wrap_gst_net_time_provider_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GstPlugin ----------- */

static PyObject *
_wrap_gst_plugin_get_name(PyGObject *self)
{
    const gchar *ret;

    pyg_begin_allow_threads;
    ret = gst_plugin_get_name(GST_PLUGIN(self->obj));
    pyg_end_allow_threads;
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_plugin_get_description(PyGObject *self)
{
    const gchar *ret;

    pyg_begin_allow_threads;
    ret = gst_plugin_get_description(GST_PLUGIN(self->obj));
    pyg_end_allow_threads;
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_plugin_get_filename(PyGObject *self)
{
    const gchar *ret;

    pyg_begin_allow_threads;
    ret = gst_plugin_get_filename(GST_PLUGIN(self->obj));
    pyg_end_allow_threads;
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_plugin_get_version(PyGObject *self)
{
    const gchar *ret;

    pyg_begin_allow_threads;
    ret = gst_plugin_get_version(GST_PLUGIN(self->obj));
    pyg_end_allow_threads;
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_plugin_get_license(PyGObject *self)
{
    const gchar *ret;

    pyg_begin_allow_threads;
    ret = gst_plugin_get_license(GST_PLUGIN(self->obj));
    pyg_end_allow_threads;
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_plugin_get_source(PyGObject *self)
{
    const gchar *ret;

    pyg_begin_allow_threads;
    ret = gst_plugin_get_source(GST_PLUGIN(self->obj));
    pyg_end_allow_threads;
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_plugin_get_package(PyGObject *self)
{
    const gchar *ret;

    pyg_begin_allow_threads;
    ret = gst_plugin_get_package(GST_PLUGIN(self->obj));
    pyg_end_allow_threads;
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_plugin_get_origin(PyGObject *self)
{
    const gchar *ret;

    pyg_begin_allow_threads;
    ret = gst_plugin_get_origin(GST_PLUGIN(self->obj));
    pyg_end_allow_threads;
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_plugin_is_loaded(PyGObject *self)
{
    int ret;

    pyg_begin_allow_threads;
    ret = gst_plugin_is_loaded(GST_PLUGIN(self->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_plugin_name_filter(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", NULL };
    char *name;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:GstPlugin.name_filter", kwlist, &name))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_plugin_name_filter(GST_PLUGIN(self->obj), name);
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_plugin_load(PyGObject *self)
{
    GstPlugin *ret;

    pyg_begin_allow_threads;
    ret = gst_plugin_load(GST_PLUGIN(self->obj));
    pyg_end_allow_threads;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static const PyMethodDef _PyGstPlugin_methods[] = {
    { "get_name", (PyCFunction)_wrap_gst_plugin_get_name, METH_NOARGS,
      NULL },
    { "get_description", (PyCFunction)_wrap_gst_plugin_get_description, METH_NOARGS,
      NULL },
    { "get_filename", (PyCFunction)_wrap_gst_plugin_get_filename, METH_NOARGS,
      NULL },
    { "get_version", (PyCFunction)_wrap_gst_plugin_get_version, METH_NOARGS,
      NULL },
    { "get_license", (PyCFunction)_wrap_gst_plugin_get_license, METH_NOARGS,
      NULL },
    { "get_source", (PyCFunction)_wrap_gst_plugin_get_source, METH_NOARGS,
      NULL },
    { "get_package", (PyCFunction)_wrap_gst_plugin_get_package, METH_NOARGS,
      NULL },
    { "get_origin", (PyCFunction)_wrap_gst_plugin_get_origin, METH_NOARGS,
      NULL },
    { "is_loaded", (PyCFunction)_wrap_gst_plugin_is_loaded, METH_NOARGS,
      NULL },
    { "name_filter", (PyCFunction)_wrap_gst_plugin_name_filter, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "load", (PyCFunction)_wrap_gst_plugin_load, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject PyGstPlugin_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.Plugin",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGstPlugin_methods, /* tp_methods */
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



/* ----------- GstControlSource ----------- */

PyTypeObject PyGstControlSource_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.ControlSource",                   /* tp_name */
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



/* ----------- GstController ----------- */

#line 95 "gstlibs.override"
static int
_wrap_gst_controller_new_list(PyGObject *self, PyObject *args)
{
    PyObject    *target_pyobj;
    PyGObject	*target;
    gint	len;
    GList	*list = NULL;

    if ((len = PyTuple_Size(args)) < 1) {
	PyErr_SetString(PyExc_TypeError, "Controller requires at least a target object");
	return -1;
    }

    target_pyobj = PyTuple_GetItem(args, 0);
    if (!PyObject_TypeCheck(target_pyobj, &PyGObject_Type)) {
	PyErr_Format(PyExc_TypeError,
		     "argument 1 must be %s, not %s",
		     PyGObject_Type.tp_name,
		     target_pyobj == Py_None ? "None" : target_pyobj->ob_type->tp_name);
	return -1;
    }

    target = (PyGObject *) target_pyobj;
    
    if (len > 1)
	while (len-- > 1) {
	    PyObject	*temp;
	    gchar	*str;

	    temp = PyTuple_GetItem(args, len);
	    str = PyString_AsString(temp);
	    if (str == NULL) {
		g_list_free (list);
		return -1;
	    }
	    GST_INFO("prepending %s [%d]", str, len);
	    list = g_list_prepend(list, str);
	}

    self->obj = (GObject *) gst_controller_new_list(target->obj, list);
    g_list_free (list);

    if (!self->obj) {
	PyErr_SetString(PyExc_RuntimeError, "could not create GstController object");
	return -1;
    }
    
    pygobject_register_wrapper((PyObject *) self);
    return 0;
}
#line 13080 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 147 "gstlibs.override"
static PyObject *
_wrap_gst_controller_remove_properties (PyGObject *self, PyObject *args)
{
    GstController *controller = (GstController *) self->obj;
    gint	len;
    GList	*list = NULL;
    gboolean	res = FALSE;
    PyObject	*pret;

    if ((len = PyTuple_Size(args)) < 1) {
	PyErr_SetString(PyExc_TypeError, "Please give at least one property name to remove");
	return NULL;
    }

    while (len--) {
	PyObject	*temp;
	gchar	*str;
	
	temp = PyTuple_GetItem(args, len);
	str = PyString_AsString(temp);
	if (str == NULL) {
	    g_list_free (list);
	    return NULL;
	}
	GST_INFO("prepending %s [%d]", str, len);
	list = g_list_prepend(list, str);
    }
    
    res = gst_controller_remove_properties_list(controller, list);
    g_list_free (list);

    if (res)
	pret = Py_True;
    else
	pret = Py_False;

    Py_INCREF (pret);

    return pret;
}
#line 13124 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_controller_set_disabled(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "disabled", NULL };
    int disabled;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GstController.set_disabled", kwlist, &disabled))
        return NULL;
    pyg_begin_allow_threads;
    gst_controller_set_disabled(GST_CONTROLLER(self->obj), disabled);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_controller_set_property_disabled(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "property_name", "disabled", NULL };
    char *property_name;
    int disabled;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"si:GstController.set_property_disabled", kwlist, &property_name, &disabled))
        return NULL;
    pyg_begin_allow_threads;
    gst_controller_set_property_disabled(GST_CONTROLLER(self->obj), property_name, disabled);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_controller_set_control_source(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "property_name", "csource", NULL };
    char *property_name;
    PyGObject *csource;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"sO!:GstController.set_control_source", kwlist, &property_name, &PyGstControlSource_Type, &csource))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_controller_set_control_source(GST_CONTROLLER(self->obj), property_name, GST_CONTROL_SOURCE(csource->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_controller_get_control_source(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "property_name", NULL };
    char *property_name;
    GstControlSource *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:GstController.get_control_source", kwlist, &property_name))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_controller_get_control_source(GST_CONTROLLER(self->obj), property_name);
    pyg_end_allow_threads;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

#line 30 "gstlibs.override"
static PyObject *
_wrap_gst_controller_set (PyGObject *self, PyObject *args)
{
  GstController	*controller = (GstController *) self->obj;
  gchar		*param_name;
  GParamSpec	*pspec = NULL;
  GstClockTime	timestamp;
  GValue	value = { 0, };
  PyObject	*pvalue;
  gboolean	res;

  if (!PyArg_ParseTuple(args, "sLO:GstController.set",
			&param_name, &timestamp, &pvalue))
    return NULL;

  if (!(pspec = g_object_class_find_property(G_OBJECT_GET_CLASS (controller->object),
					     (const gchar*) param_name))) {
      PyErr_SetString (PyExc_TypeError,
		       "The controlled object doesn't have the given property");
      return NULL;
  }

  g_value_init (&value, pspec->value_type);

  if (pyg_value_from_pyobject (&value, pvalue)) {
    PyErr_SetString (PyExc_TypeError,
		     "Couldn't convert the given value to the good type");
    return NULL;
  }

  res = gst_controller_set (controller, param_name, timestamp, &value);
  if (res) {
    Py_INCREF (Py_True);
    return Py_True;
  }
  Py_INCREF (Py_False);
  return Py_False;
}
#line 13230 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 189 "gstlibs.override"
static PyObject *
_wrap_gst_controller_set_from_list (PyGObject *self, PyObject *args)
{
    GstController *controller = (GstController *) self->obj;
    PyObject	*temp;
    gint	len;
    gchar	*pname;
    GSList	*list = NULL;
    GParamSpec	*pspec = NULL;
    gboolean	res = FALSE;
    PyObject	*pret;

    if ((len = PyTuple_Size(args)) < 2) {
	PyErr_SetString(PyExc_TypeError, "Please give a property name and a tuple of (time,value)");
	return NULL;
    }

    temp = PyTuple_GetItem(args, 0);
    if (!PyString_Check (temp)) {
	PyErr_SetString(PyExc_TypeError, "First argument must be a string");
	return NULL;
    }
    pname = PyString_AsString(temp);

    if (!(pspec = g_object_class_find_property(G_OBJECT_GET_CLASS (controller->object),
					       (const gchar*) pname)))
	goto error;

    while (len-- > 1) {
	PyObject	*temp2;
	GstTimedValue	*tval;
	
	temp2 = PyTuple_GetItem(args, len);
	if (!PyTuple_Check (temp2)) {
	    PyErr_SetString (PyExc_TypeError, "Tuple doesn't contain tuples !");
	    goto error;
	}
	tval = g_new0(GstTimedValue, 1);
	tval->timestamp = PyLong_AsUnsignedLongLong(PyTuple_GetItem(temp2, 0));
	g_value_init (&tval->value, pspec->value_type);
	if ((pyg_value_from_pyobject (&tval->value, PyTuple_GetItem (temp2, 1))) < 0) {
	    PyErr_SetString (PyExc_TypeError, "Couldn't convert value to correct type");
	    goto error;
	};

	list = g_slist_prepend(list, tval);
    }
    
    res = gst_controller_set_from_list(controller, pname, list);
    g_slist_free (list);

    if (res)
	pret = Py_True;
    else
	pret = Py_False;

    Py_INCREF (pret);

    return pret;

 error:
    while (list) {
	g_free(list->data);
	list = g_slist_next(list);
    }
    g_slist_free (list);
    return NULL;
}
#line 13302 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_controller_unset(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "property_name", "timestamp", NULL };
    char *property_name;
    int ret;
    guint64 timestamp;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"sK:GstController.unset", kwlist, &property_name, &timestamp))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_controller_unset(GST_CONTROLLER(self->obj), property_name, timestamp);
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_controller_unset_all(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "property_name", NULL };
    char *property_name;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:GstController.unset_all", kwlist, &property_name))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_controller_unset_all(GST_CONTROLLER(self->obj), property_name);
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

#line 70 "gstlibs.override"
static PyObject *
_wrap_gst_controller_get (PyGObject *self, PyObject *args, PyObject *kwargs)
{
    GstController	*controller = (GstController *) self->obj;
    static char		*kwlist[] = { "propertyname", "timestamp", NULL };
    gchar	*propertyname;
    GstClockTime	timestamp;
    GValue	*value = NULL;
    PyObject	*pyvalue;

    if (!PyArg_ParseTupleAndKeywords (args, kwargs,
				      "sL:GstController.get",
				      kwlist, &propertyname, &timestamp))
	return NULL;

    value = gst_controller_get (controller, propertyname, timestamp);
    if (value) {
	pyvalue = pyg_value_as_pyobject (value, FALSE);
	return pyvalue;
    }
    Py_INCREF (Py_None);
    return Py_None;
}
#line 13362 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_controller_suggest_next_sync(PyGObject *self)
{
    guint64 ret;

    pyg_begin_allow_threads;
    ret = gst_controller_suggest_next_sync(GST_CONTROLLER(self->obj));
    pyg_end_allow_threads;
    return PyLong_FromUnsignedLongLong(ret);
}

static PyObject *
_wrap_gst_controller_sync_values(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "timestamp", NULL };
    int ret;
    guint64 timestamp;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"K:GstController.sync_values", kwlist, &timestamp))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_controller_sync_values(GST_CONTROLLER(self->obj), timestamp);
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_controller_set_interpolation_mode(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "property_name", "mode", NULL };
    char *property_name;
    GstInterpolateMode mode;
    int ret;
    PyObject *py_mode = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"sO:GstController.set_interpolation_mode", kwlist, &property_name, &py_mode))
        return NULL;
    if (pyg_enum_get_value(G_TYPE_NONE, py_mode, (gint *)&mode))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_controller_set_interpolation_mode(GST_CONTROLLER(self->obj), property_name, mode);
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static const PyMethodDef _PyGstController_methods[] = {
    { "remove_properties", (PyCFunction)_wrap_gst_controller_remove_properties, METH_VARARGS,
      NULL },
    { "set_disabled", (PyCFunction)_wrap_gst_controller_set_disabled, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_property_disabled", (PyCFunction)_wrap_gst_controller_set_property_disabled, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_control_source", (PyCFunction)_wrap_gst_controller_set_control_source, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_control_source", (PyCFunction)_wrap_gst_controller_get_control_source, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set", (PyCFunction)_wrap_gst_controller_set, METH_VARARGS,
      NULL },
    { "set_from_list", (PyCFunction)_wrap_gst_controller_set_from_list, METH_VARARGS,
      NULL },
    { "unset", (PyCFunction)_wrap_gst_controller_unset, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "unset_all", (PyCFunction)_wrap_gst_controller_unset_all, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get", (PyCFunction)_wrap_gst_controller_get, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "suggest_next_sync", (PyCFunction)_wrap_gst_controller_suggest_next_sync, METH_NOARGS,
      NULL },
    { "sync_values", (PyCFunction)_wrap_gst_controller_sync_values, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_interpolation_mode", (PyCFunction)_wrap_gst_controller_set_interpolation_mode, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject PyGstController_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.Controller",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGstController_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_gst_controller_new_list,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GstInterpolationControlSource ----------- */

 static int
_wrap_gst_interpolation_control_source_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char* kwlist[] = { NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     ":gst.InterpolationControlSource.__init__",
                                     kwlist))
        return -1;

    pygobject_constructv(self, 0, NULL);
    if (!self->obj) {
        PyErr_SetString(
            PyExc_RuntimeError, 
            "could not create gst.InterpolationControlSource object");
        return -1;
    }
    return 0;
}

static PyObject *
_wrap_gst_interpolation_control_source_set_interpolation_mode(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "mode", NULL };
    GstInterpolateMode mode;
    PyObject *py_mode = NULL;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:GstInterpolationControlSource.set_interpolation_mode", kwlist, &py_mode))
        return NULL;
    if (pyg_enum_get_value(G_TYPE_NONE, py_mode, (gint *)&mode))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_interpolation_control_source_set_interpolation_mode(GST_INTERPOLATION_CONTROL_SOURCE(self->obj), mode);
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_interpolation_control_source_unset(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "timestamp", NULL };
    int ret;
    guint64 timestamp;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"K:GstInterpolationControlSource.unset", kwlist, &timestamp))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_interpolation_control_source_unset(GST_INTERPOLATION_CONTROL_SOURCE(self->obj), timestamp);
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_interpolation_control_source_unset_all(PyGObject *self)
{
    pyg_begin_allow_threads;
    gst_interpolation_control_source_unset_all(GST_INTERPOLATION_CONTROL_SOURCE(self->obj));
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_interpolation_control_source_get_count(PyGObject *self)
{
    int ret;

    pyg_begin_allow_threads;
    ret = gst_interpolation_control_source_get_count(GST_INTERPOLATION_CONTROL_SOURCE(self->obj));
    pyg_end_allow_threads;
    return PyInt_FromLong(ret);
}

static const PyMethodDef _PyGstInterpolationControlSource_methods[] = {
    { "set_interpolation_mode", (PyCFunction)_wrap_gst_interpolation_control_source_set_interpolation_mode, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "unset", (PyCFunction)_wrap_gst_interpolation_control_source_unset, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "unset_all", (PyCFunction)_wrap_gst_interpolation_control_source_unset_all, METH_NOARGS,
      NULL },
    { "get_count", (PyCFunction)_wrap_gst_interpolation_control_source_get_count, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject PyGstInterpolationControlSource_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.InterpolationControlSource",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGstInterpolationControlSource_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_gst_interpolation_control_source_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GstLFOControlSource ----------- */

PyTypeObject PyGstLFOControlSource_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.LFOControlSource",                   /* tp_name */
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



/* ----------- GstAdapter ----------- */

static int
_wrap_gst_adapter_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char* kwlist[] = { NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     ":gst.Adapter.__init__",
                                     kwlist))
        return -1;

    pygobject_constructv(self, 0, NULL);
    if (!self->obj) {
        PyErr_SetString(
            PyExc_RuntimeError, 
            "could not create gst.Adapter object");
        return -1;
    }
    return 0;
}

static PyObject *
_wrap_gst_adapter_clear(PyGObject *self)
{
    pyg_begin_allow_threads;
    gst_adapter_clear(GST_ADAPTER(self->obj));
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_adapter_push(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "buf", NULL };
    PyGstMiniObject *buf;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstAdapter.push", kwlist, &PyGstBuffer_Type, &buf))
        return NULL;
    gst_mini_object_ref(GST_MINI_OBJECT(buf->obj));
    pyg_begin_allow_threads;
    gst_adapter_push(GST_ADAPTER(self->obj), GST_BUFFER(buf->obj));
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

#line 878 "gstbase.override"
static PyObject *
_wrap_gst_adapter_peek(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "size", NULL };
    PyObject *py_size = NULL;
    const guint8 *ret;
    guint size = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:GstAdapter.peek", kwlist, &py_size))
        return NULL;
    if (py_size) {
        if (PyLong_Check(py_size))
            size = PyLong_AsUnsignedLong(py_size);
        else if (PyInt_Check(py_size))
            size = PyInt_AsLong(py_size);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'size' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    pyg_begin_allow_threads;
    ret = gst_adapter_peek(GST_ADAPTER(self->obj), size);
    pyg_end_allow_threads;
    if (ret)
        return PyString_FromStringAndSize((gchar*) ret, size);
    Py_INCREF(Py_None);
    return Py_None;
}
#line 13752 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_adapter_flush(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "flush", NULL };
    PyObject *py_flush = NULL;
    guint flush = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:GstAdapter.flush", kwlist, &py_flush))
        return NULL;
    if (py_flush) {
        if (PyLong_Check(py_flush))
            flush = PyLong_AsUnsignedLong(py_flush);
        else if (PyInt_Check(py_flush))
            flush = PyInt_AsLong(py_flush);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'flush' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    pyg_begin_allow_threads;
    gst_adapter_flush(GST_ADAPTER(self->obj), flush);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

#line 845 "gstbase.override"
static PyObject *
_wrap_gst_adapter_take(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "nbytes", NULL };
    PyObject *py_nbytes = NULL;
    guint nbytes = 0;
    guint8 *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:GstAdapter.take", kwlist, &py_nbytes))
        return NULL;
    if (py_nbytes) {
        if (PyLong_Check(py_nbytes))
            nbytes = PyLong_AsUnsignedLong(py_nbytes);
        else if (PyInt_Check(py_nbytes))
            nbytes = PyInt_AsLong(py_nbytes);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'nbytes' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    pyg_begin_allow_threads;
    ret = gst_adapter_take(GST_ADAPTER(self->obj), nbytes);
    pyg_end_allow_threads;
    if (ret) {
        PyObject *py_ret = PyString_FromStringAndSize((gchar*) ret, nbytes);
        g_free(ret);
        return py_ret;
    }
    Py_INCREF(Py_None);
    return Py_None;
}
#line 13813 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_adapter_take_buffer(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "nbytes", NULL };
    PyObject *py_nbytes = NULL, *py_ret;
    GstBuffer *ret;
    guint nbytes = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:GstAdapter.take_buffer", kwlist, &py_nbytes))
        return NULL;
    if (py_nbytes) {
        if (PyLong_Check(py_nbytes))
            nbytes = PyLong_AsUnsignedLong(py_nbytes);
        else if (PyInt_Check(py_nbytes))
            nbytes = PyInt_AsLong(py_nbytes);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'nbytes' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    pyg_begin_allow_threads;
    ret = gst_adapter_take_buffer(GST_ADAPTER(self->obj), nbytes);
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_adapter_available(PyGObject *self)
{
    guint ret;

    pyg_begin_allow_threads;
    ret = gst_adapter_available(GST_ADAPTER(self->obj));
    pyg_end_allow_threads;
    return PyLong_FromUnsignedLong(ret);
}

static PyObject *
_wrap_gst_adapter_available_fast(PyGObject *self)
{
    guint ret;

    pyg_begin_allow_threads;
    ret = gst_adapter_available_fast(GST_ADAPTER(self->obj));
    pyg_end_allow_threads;
    return PyLong_FromUnsignedLong(ret);
}

static const PyMethodDef _PyGstAdapter_methods[] = {
    { "clear", (PyCFunction)_wrap_gst_adapter_clear, METH_NOARGS,
      NULL },
    { "push", (PyCFunction)_wrap_gst_adapter_push, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "peek", (PyCFunction)_wrap_gst_adapter_peek, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "flush", (PyCFunction)_wrap_gst_adapter_flush, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "take", (PyCFunction)_wrap_gst_adapter_take, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "take_buffer", (PyCFunction)_wrap_gst_adapter_take_buffer, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "available", (PyCFunction)_wrap_gst_adapter_available, METH_NOARGS,
      NULL },
    { "available_fast", (PyCFunction)_wrap_gst_adapter_available_fast, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject PyGstAdapter_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.Adapter",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGstAdapter_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_gst_adapter_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GstBaseSink ----------- */

static PyObject *
_wrap_gst_base_sink_wait_preroll(PyGObject *self)
{
    gint ret;

    pyg_begin_allow_threads;
    ret = gst_base_sink_wait_preroll(GST_BASE_SINK(self->obj));
    pyg_end_allow_threads;
    return pyg_enum_from_gtype(GST_TYPE_FLOW_RETURN, ret);
}

static PyObject *
_wrap_gst_base_sink_set_sync(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "sync", NULL };
    int sync;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GstBaseSink.set_sync", kwlist, &sync))
        return NULL;
    pyg_begin_allow_threads;
    gst_base_sink_set_sync(GST_BASE_SINK(self->obj), sync);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_base_sink_get_sync(PyGObject *self)
{
    int ret;

    pyg_begin_allow_threads;
    ret = gst_base_sink_get_sync(GST_BASE_SINK(self->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_base_sink_set_max_lateness(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "max_lateness", NULL };
    gint64 max_lateness;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"L:GstBaseSink.set_max_lateness", kwlist, &max_lateness))
        return NULL;
    pyg_begin_allow_threads;
    gst_base_sink_set_max_lateness(GST_BASE_SINK(self->obj), max_lateness);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_base_sink_get_max_lateness(PyGObject *self)
{
    gint64 ret;

    pyg_begin_allow_threads;
    ret = gst_base_sink_get_max_lateness(GST_BASE_SINK(self->obj));
    pyg_end_allow_threads;
    return PyLong_FromLongLong(ret);
}

static PyObject *
_wrap_gst_base_sink_set_qos_enabled(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "enabled", NULL };
    int enabled;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GstBaseSink.set_qos_enabled", kwlist, &enabled))
        return NULL;
    pyg_begin_allow_threads;
    gst_base_sink_set_qos_enabled(GST_BASE_SINK(self->obj), enabled);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_base_sink_is_qos_enabled(PyGObject *self)
{
    int ret;

    pyg_begin_allow_threads;
    ret = gst_base_sink_is_qos_enabled(GST_BASE_SINK(self->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_base_sink_set_async_enabled(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "enabled", NULL };
    int enabled;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GstBaseSink.set_async_enabled", kwlist, &enabled))
        return NULL;
    pyg_begin_allow_threads;
    gst_base_sink_set_async_enabled(GST_BASE_SINK(self->obj), enabled);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_base_sink_is_async_enabled(PyGObject *self)
{
    int ret;

    pyg_begin_allow_threads;
    ret = gst_base_sink_is_async_enabled(GST_BASE_SINK(self->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_base_sink_set_ts_offset(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "offset", NULL };
    gint64 offset;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"L:GstBaseSink.set_ts_offset", kwlist, &offset))
        return NULL;
    pyg_begin_allow_threads;
    gst_base_sink_set_ts_offset(GST_BASE_SINK(self->obj), offset);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_base_sink_get_ts_offset(PyGObject *self)
{
    gint64 ret;

    pyg_begin_allow_threads;
    ret = gst_base_sink_get_ts_offset(GST_BASE_SINK(self->obj));
    pyg_end_allow_threads;
    return PyLong_FromLongLong(ret);
}

static PyObject *
_wrap_gst_base_sink_get_last_buffer(PyGObject *self)
{
    GstBuffer *ret;

    pyg_begin_allow_threads;
    ret = gst_base_sink_get_last_buffer(GST_BASE_SINK(self->obj));
    pyg_end_allow_threads;
    /* pygobject_new handles NULL checking */
    return pygstminiobject_new((GstMiniObject *)ret);
}

#line 786 "gstbase.override"
static PyObject *
_wrap_gst_base_sink_query_latency (PyGObject *self)
{
    gboolean res, live = FALSE, upstream_live = FALSE;
    GstClockTime minlat = GST_CLOCK_TIME_NONE, maxlat = GST_CLOCK_TIME_NONE;

    res = gst_base_sink_query_latency (GST_BASE_SINK (self->obj), &live, &upstream_live,
				       &minlat, &maxlat);
    return Py_BuildValue("(OOOKK)",
			 PyBool_FromLong(res),
			 PyBool_FromLong(live),
			 PyBool_FromLong(upstream_live),
			 minlat, maxlat);
}
#line 14107 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_base_sink_get_latency(PyGObject *self)
{
    guint64 ret;

    pyg_begin_allow_threads;
    ret = gst_base_sink_get_latency(GST_BASE_SINK(self->obj));
    pyg_end_allow_threads;
    return PyLong_FromUnsignedLongLong(ret);
}

static PyObject *
_wrap_GstBaseSink__do_get_caps(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    GstCaps *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstBaseSink.get_caps", kwlist, &PyGstBaseSink_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_BASE_SINK_CLASS(klass)->get_caps) {
        pyg_begin_allow_threads;
        ret = GST_BASE_SINK_CLASS(klass)->get_caps(GST_BASE_SINK(self->obj));
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstBaseSink.get_caps not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return pyg_boxed_new (GST_TYPE_CAPS, ret, FALSE, TRUE);
}

static PyObject *
_wrap_GstBaseSink__do_set_caps(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "caps", NULL };
    PyGObject *self;
    PyObject *py_caps;
    int ret;
    GstCaps *caps;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O:GstBaseSink.set_caps", kwlist, &PyGstBaseSink_Type, &self, &py_caps))
        return NULL;
    caps = pygst_caps_from_pyobject (py_caps, NULL);
    if (PyErr_Occurred())
      return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_BASE_SINK_CLASS(klass)->set_caps) {
        pyg_begin_allow_threads;
        ret = GST_BASE_SINK_CLASS(klass)->set_caps(GST_BASE_SINK(self->obj), caps);
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstBaseSink.set_caps not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyBool_FromLong(ret);

}

#line 749 "gstbase.override"
static PyObject *
_wrap_GstBaseSink__do_get_times (PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "buffer", NULL };
    PyGObject *self;
    PyGstMiniObject *py_buffer;
    GstClockTime start = 0;
    GstClockTime end = 0;
    PyObject *py_ret;
    
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O!O!:GstBaseSink.get_times",
				     kwlist, &PyGstBaseSink_Type, &self,
				     &PyGstBuffer_Type, &py_buffer))
	return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_BASE_SINK_CLASS(klass)->get_times) {
	pyg_begin_allow_threads;
	GST_BASE_SINK_CLASS(klass)->get_times(GST_BASE_SINK(self->obj),
					      GST_BUFFER(py_buffer->obj),
					      &start, &end);
	pyg_end_allow_threads;
    } else {
      PyErr_SetString(PyExc_NotImplementedError, "virtual method GstBaseSink.get_times not implemented");
      g_type_class_unref(klass);
      return NULL;
    }
    g_type_class_unref(klass);
    
    py_ret = PyTuple_New(2);
    PyTuple_SetItem(py_ret, 0, PyLong_FromUnsignedLongLong(start));
    PyTuple_SetItem(py_ret, 1, PyLong_FromUnsignedLongLong(end));

    return py_ret;
}
#line 14211 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_GstBaseSink__do_start(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstBaseSink.start", kwlist, &PyGstBaseSink_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_BASE_SINK_CLASS(klass)->start) {
        pyg_begin_allow_threads;
        ret = GST_BASE_SINK_CLASS(klass)->start(GST_BASE_SINK(self->obj));
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstBaseSink.start not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_GstBaseSink__do_stop(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstBaseSink.stop", kwlist, &PyGstBaseSink_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_BASE_SINK_CLASS(klass)->stop) {
        pyg_begin_allow_threads;
        ret = GST_BASE_SINK_CLASS(klass)->stop(GST_BASE_SINK(self->obj));
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstBaseSink.stop not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_GstBaseSink__do_unlock(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstBaseSink.unlock", kwlist, &PyGstBaseSink_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_BASE_SINK_CLASS(klass)->unlock) {
        pyg_begin_allow_threads;
        ret = GST_BASE_SINK_CLASS(klass)->unlock(GST_BASE_SINK(self->obj));
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstBaseSink.unlock not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_GstBaseSink__do_event(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "event", NULL };
    PyGObject *self;
    int ret;
    PyGstMiniObject *event;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!:GstBaseSink.event", kwlist, &PyGstBaseSink_Type, &self, &PyGstEvent_Type, &event))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_BASE_SINK_CLASS(klass)->event) {
        pyg_begin_allow_threads;
        ret = GST_BASE_SINK_CLASS(klass)->event(GST_BASE_SINK(self->obj), GST_EVENT(event->obj));
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstBaseSink.event not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_GstBaseSink__do_preroll(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "buffer", NULL };
    PyGObject *self;
    gint ret;
    PyGstMiniObject *buffer;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!:GstBaseSink.preroll", kwlist, &PyGstBaseSink_Type, &self, &PyGstBuffer_Type, &buffer))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_BASE_SINK_CLASS(klass)->preroll) {
        pyg_begin_allow_threads;
        ret = GST_BASE_SINK_CLASS(klass)->preroll(GST_BASE_SINK(self->obj), GST_BUFFER(buffer->obj));
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstBaseSink.preroll not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return pyg_enum_from_gtype(GST_TYPE_FLOW_RETURN, ret);
}

static PyObject *
_wrap_GstBaseSink__do_render(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "buffer", NULL };
    PyGObject *self;
    gint ret;
    PyGstMiniObject *buffer;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!:GstBaseSink.render", kwlist, &PyGstBaseSink_Type, &self, &PyGstBuffer_Type, &buffer))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_BASE_SINK_CLASS(klass)->render) {
        pyg_begin_allow_threads;
        ret = GST_BASE_SINK_CLASS(klass)->render(GST_BASE_SINK(self->obj), GST_BUFFER(buffer->obj));
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstBaseSink.render not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return pyg_enum_from_gtype(GST_TYPE_FLOW_RETURN, ret);
}

static PyObject *
_wrap_GstBaseSink__do_activate_pull(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "active", NULL };
    PyGObject *self;
    int active, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:GstBaseSink.activate_pull", kwlist, &PyGstBaseSink_Type, &self, &active))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_BASE_SINK_CLASS(klass)->activate_pull) {
        pyg_begin_allow_threads;
        ret = GST_BASE_SINK_CLASS(klass)->activate_pull(GST_BASE_SINK(self->obj), active);
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstBaseSink.activate_pull not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_GstBaseSink__do_fixate(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "caps", NULL };
    PyGObject *self;
    PyObject *py_caps;
    GstCaps *caps;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O:GstBaseSink.fixate", kwlist, &PyGstBaseSink_Type, &self, &py_caps))
        return NULL;
    caps = pygst_caps_from_pyobject (py_caps, NULL);
    if (PyErr_Occurred())
      return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_BASE_SINK_CLASS(klass)->fixate) {
        pyg_begin_allow_threads;
        GST_BASE_SINK_CLASS(klass)->fixate(GST_BASE_SINK(self->obj), caps);
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstBaseSink.fixate not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GstBaseSink__do_unlock_stop(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstBaseSink.unlock_stop", kwlist, &PyGstBaseSink_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_BASE_SINK_CLASS(klass)->unlock_stop) {
        pyg_begin_allow_threads;
        ret = GST_BASE_SINK_CLASS(klass)->unlock_stop(GST_BASE_SINK(self->obj));
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstBaseSink.unlock_stop not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyBool_FromLong(ret);

}

static const PyMethodDef _PyGstBaseSink_methods[] = {
    { "wait_preroll", (PyCFunction)_wrap_gst_base_sink_wait_preroll, METH_NOARGS,
      NULL },
    { "set_sync", (PyCFunction)_wrap_gst_base_sink_set_sync, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_sync", (PyCFunction)_wrap_gst_base_sink_get_sync, METH_NOARGS,
      NULL },
    { "set_max_lateness", (PyCFunction)_wrap_gst_base_sink_set_max_lateness, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_max_lateness", (PyCFunction)_wrap_gst_base_sink_get_max_lateness, METH_NOARGS,
      NULL },
    { "set_qos_enabled", (PyCFunction)_wrap_gst_base_sink_set_qos_enabled, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "is_qos_enabled", (PyCFunction)_wrap_gst_base_sink_is_qos_enabled, METH_NOARGS,
      NULL },
    { "set_async_enabled", (PyCFunction)_wrap_gst_base_sink_set_async_enabled, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "is_async_enabled", (PyCFunction)_wrap_gst_base_sink_is_async_enabled, METH_NOARGS,
      NULL },
    { "set_ts_offset", (PyCFunction)_wrap_gst_base_sink_set_ts_offset, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_ts_offset", (PyCFunction)_wrap_gst_base_sink_get_ts_offset, METH_NOARGS,
      NULL },
    { "get_last_buffer", (PyCFunction)_wrap_gst_base_sink_get_last_buffer, METH_NOARGS,
      NULL },
    { "query_latency", (PyCFunction)_wrap_gst_base_sink_query_latency, METH_NOARGS,
      NULL },
    { "get_latency", (PyCFunction)_wrap_gst_base_sink_get_latency, METH_NOARGS,
      NULL },
    { "do_get_caps", (PyCFunction)_wrap_GstBaseSink__do_get_caps, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_set_caps", (PyCFunction)_wrap_GstBaseSink__do_set_caps, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_times", (PyCFunction)_wrap_GstBaseSink__do_get_times, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_start", (PyCFunction)_wrap_GstBaseSink__do_start, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_stop", (PyCFunction)_wrap_GstBaseSink__do_stop, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_unlock", (PyCFunction)_wrap_GstBaseSink__do_unlock, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_event", (PyCFunction)_wrap_GstBaseSink__do_event, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_preroll", (PyCFunction)_wrap_GstBaseSink__do_preroll, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_render", (PyCFunction)_wrap_GstBaseSink__do_render, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_activate_pull", (PyCFunction)_wrap_GstBaseSink__do_activate_pull, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_fixate", (PyCFunction)_wrap_GstBaseSink__do_fixate, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_unlock_stop", (PyCFunction)_wrap_GstBaseSink__do_unlock_stop, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject PyGstBaseSink_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.BaseSink",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGstBaseSink_methods, /* tp_methods */
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

static GstCaps*
_wrap_GstBaseSink__proxy_do_get_caps(GstBaseSink *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    GstCaps* retval;
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_get_caps");
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
    retval = (GstCaps*) pygst_caps_from_pyobject (py_retval, NULL);
    
    
    Py_DECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gboolean
_wrap_GstBaseSink__proxy_do_set_caps(GstBaseSink *self, GstCaps*caps)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_caps = NULL;
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
    if (caps)
        py_caps = pyg_boxed_new (GST_TYPE_CAPS, caps, FALSE, TRUE);
    else {
        Py_INCREF(Py_None);
        py_caps = Py_None;
    }
    
    py_args = PyTuple_New(1);
    Py_INCREF(py_caps);
    PyTuple_SET_ITEM(py_args, 0, py_caps);
    
    py_method = PyObject_GetAttrString(py_self, "do_set_caps");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        gst_caps_ref(caps);
        Py_DECREF(py_caps);
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
        gst_caps_ref(caps);
        Py_DECREF(py_caps);
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
        gst_caps_ref(caps);
        Py_DECREF(py_caps);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_DECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    gst_caps_ref(caps);
    Py_DECREF(py_caps);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
#line 688 "gstbase.override"
static void
_wrap_GstBaseSink__proxy_do_get_times (GstBaseSink * self,
				       GstBuffer *buffer,
				       GstClockTime * start,
				       GstClockTime * end)
{
    PyGILState_STATE __py_state;
    PyObject *py_args;
    PyObject *py_self;
    PyObject *py_method;
    PyObject *py_ret;
    
    __py_state = pyg_gil_state_ensure();
    
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
	pyg_gil_state_release(__py_state);
	return;
    }

    py_args = Py_BuildValue ("(N)",
			     pygstminiobject_new((GstMiniObject *)buffer));
    
    py_method = PyObject_GetAttrString(py_self, "do_get_times");
    
    Py_DECREF(py_self);

    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
	goto beach;
    }

    py_ret = PyObject_CallObject(py_method, py_args);

    Py_DECREF(py_method);

    if (!py_ret) {
        if (PyErr_Occurred())
            PyErr_Print();
	goto beach;
    }

    /*
      If the method returned a numeric, the return value will be TRUE.
      For ANY other case, we don't set size and the return value is FALSE.
    */

    if ((PyTuple_Check(py_ret)) && (PyTuple_Size (py_ret) == 2))
	PyArg_ParseTuple (py_ret, "KK", start, end);

    Py_DECREF (py_ret);
 beach:
    Py_DECREF (py_args);
    pyg_gil_state_release(__py_state);
    return;
}
#line 14731 "..\\..\\Source\\gst-python\\gst\\gst.c"


static gboolean
_wrap_GstBaseSink__proxy_do_start(GstBaseSink *self)
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
_wrap_GstBaseSink__proxy_do_stop(GstBaseSink *self)
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
static gboolean
_wrap_GstBaseSink__proxy_do_unlock(GstBaseSink *self)
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_unlock");
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
_wrap_GstBaseSink__proxy_do_event(GstBaseSink *self, GstEvent*event)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_event = NULL;
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
    if (event) {
        py_event = pygstminiobject_new((GstMiniObject *) event);
        gst_mini_object_unref ((GstMiniObject *) event);
    } else {
        Py_INCREF(Py_None);
        py_event = Py_None;
    }
    
    py_args = PyTuple_New(1);
    Py_INCREF(py_event);
    PyTuple_SET_ITEM(py_args, 0, py_event);
    
    py_method = PyObject_GetAttrString(py_self, "do_event");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        gst_mini_object_ref ((GstMiniObject *) event); Py_DECREF(py_event);
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
        gst_mini_object_ref ((GstMiniObject *) event); Py_DECREF(py_event);
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
        gst_mini_object_ref ((GstMiniObject *) event); Py_DECREF(py_event);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_DECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    gst_mini_object_ref ((GstMiniObject *) event); Py_DECREF(py_event);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static GstFlowReturn
_wrap_GstBaseSink__proxy_do_preroll(GstBaseSink *self, GstBuffer*buffer)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_buffer = NULL;
    GstFlowReturn retval;
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
    if (buffer) {
        py_buffer = pygstminiobject_new((GstMiniObject *) buffer);
        gst_mini_object_unref ((GstMiniObject *) buffer);
    } else {
        Py_INCREF(Py_None);
        py_buffer = Py_None;
    }
    
    py_args = PyTuple_New(1);
    Py_INCREF(py_buffer);
    PyTuple_SET_ITEM(py_args, 0, py_buffer);
    
    py_method = PyObject_GetAttrString(py_self, "do_preroll");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        gst_mini_object_ref ((GstMiniObject *) buffer); Py_DECREF(py_buffer);
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
        gst_mini_object_ref ((GstMiniObject *) buffer); Py_DECREF(py_buffer);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    if (pyg_enum_get_value(GST_TYPE_FLOW_RETURN, py_retval, (gint *)&retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        gst_mini_object_ref ((GstMiniObject *) buffer); Py_DECREF(py_buffer);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    
    
    Py_DECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    gst_mini_object_ref ((GstMiniObject *) buffer); Py_DECREF(py_buffer);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static GstFlowReturn
_wrap_GstBaseSink__proxy_do_render(GstBaseSink *self, GstBuffer*buffer)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_buffer = NULL;
    GstFlowReturn retval;
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
    if (buffer) {
        py_buffer = pygstminiobject_new((GstMiniObject *) buffer);
        gst_mini_object_unref ((GstMiniObject *) buffer);
    } else {
        Py_INCREF(Py_None);
        py_buffer = Py_None;
    }
    
    py_args = PyTuple_New(1);
    Py_INCREF(py_buffer);
    PyTuple_SET_ITEM(py_args, 0, py_buffer);
    
    py_method = PyObject_GetAttrString(py_self, "do_render");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        gst_mini_object_ref ((GstMiniObject *) buffer); Py_DECREF(py_buffer);
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
        gst_mini_object_ref ((GstMiniObject *) buffer); Py_DECREF(py_buffer);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    if (pyg_enum_get_value(GST_TYPE_FLOW_RETURN, py_retval, (gint *)&retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        gst_mini_object_ref ((GstMiniObject *) buffer); Py_DECREF(py_buffer);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    
    
    Py_DECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    gst_mini_object_ref ((GstMiniObject *) buffer); Py_DECREF(py_buffer);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gboolean
_wrap_GstBaseSink__proxy_do_activate_pull(GstBaseSink *self, gboolean active)
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
    
    py_method = PyObject_GetAttrString(py_self, "do_activate_pull");
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
static void
_wrap_GstBaseSink__proxy_do_fixate(GstBaseSink *self, GstCaps*caps)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_caps = NULL;
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
    if (caps)
        py_caps = pyg_boxed_new (GST_TYPE_CAPS, caps, FALSE, TRUE);
    else {
        Py_INCREF(Py_None);
        py_caps = Py_None;
    }
    
    py_args = PyTuple_New(1);
    Py_INCREF(py_caps);
    PyTuple_SET_ITEM(py_args, 0, py_caps);
    
    py_method = PyObject_GetAttrString(py_self, "do_fixate");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        gst_caps_ref(caps);
        Py_DECREF(py_caps);
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
        gst_caps_ref(caps);
        Py_DECREF(py_caps);
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
        gst_caps_ref(caps);
        Py_DECREF(py_caps);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    Py_DECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    gst_caps_ref(caps);
    Py_DECREF(py_caps);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
}
static gboolean
_wrap_GstBaseSink__proxy_do_unlock_stop(GstBaseSink *self)
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_unlock_stop");
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

static int
__GstBaseSink_class_init(gpointer gclass, PyTypeObject *pyclass)
{
    PyObject *o;
    GstBaseSinkClass *klass = GST_BASE_SINK_CLASS(gclass);
    PyObject *gsignals = PyDict_GetItemString(pyclass->tp_dict, "__gsignals__");

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_get_caps");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "get_caps")))
            klass->get_caps = _wrap_GstBaseSink__proxy_do_get_caps;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_set_caps");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "set_caps")))
            klass->set_caps = _wrap_GstBaseSink__proxy_do_set_caps;
        Py_DECREF(o);
    }

    /* overriding do_buffer_alloc is currently not supported */

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_get_times");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "get_times")))
            klass->get_times = _wrap_GstBaseSink__proxy_do_get_times;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_start");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "start")))
            klass->start = _wrap_GstBaseSink__proxy_do_start;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_stop");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "stop")))
            klass->stop = _wrap_GstBaseSink__proxy_do_stop;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_unlock");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "unlock")))
            klass->unlock = _wrap_GstBaseSink__proxy_do_unlock;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_event");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "event")))
            klass->event = _wrap_GstBaseSink__proxy_do_event;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_preroll");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "preroll")))
            klass->preroll = _wrap_GstBaseSink__proxy_do_preroll;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_render");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "render")))
            klass->render = _wrap_GstBaseSink__proxy_do_render;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_activate_pull");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "activate_pull")))
            klass->activate_pull = _wrap_GstBaseSink__proxy_do_activate_pull;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_fixate");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "fixate")))
            klass->fixate = _wrap_GstBaseSink__proxy_do_fixate;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_unlock_stop");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "unlock_stop")))
            klass->unlock_stop = _wrap_GstBaseSink__proxy_do_unlock_stop;
        Py_DECREF(o);
    }
    return 0;
}


/* ----------- GstBaseSrc ----------- */

static PyObject *
_wrap_gst_base_src_wait_playing(PyGObject *self)
{
    gint ret;

    pyg_begin_allow_threads;
    ret = gst_base_src_wait_playing(GST_BASE_SRC(self->obj));
    pyg_end_allow_threads;
    return pyg_enum_from_gtype(GST_TYPE_FLOW_RETURN, ret);
}

static PyObject *
_wrap_gst_base_src_set_live(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "live", NULL };
    int live;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GstBaseSrc.set_live", kwlist, &live))
        return NULL;
    pyg_begin_allow_threads;
    gst_base_src_set_live(GST_BASE_SRC(self->obj), live);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_base_src_is_live(PyGObject *self)
{
    int ret;

    pyg_begin_allow_threads;
    ret = gst_base_src_is_live(GST_BASE_SRC(self->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_base_src_set_format(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "format", NULL };
    PyObject *py_format = NULL;
    GstFormat format;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:GstBaseSrc.set_format", kwlist, &py_format))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_FORMAT, py_format, (gint *)&format))
        return NULL;
    pyg_begin_allow_threads;
    gst_base_src_set_format(GST_BASE_SRC(self->obj), format);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

#line 802 "gstbase.override"
static PyObject *
_wrap_gst_base_src_query_latency (PyGObject *self)
{
    gboolean res, live = FALSE;
    GstClockTime minlat = GST_CLOCK_TIME_NONE, maxlat = GST_CLOCK_TIME_NONE;

    res = gst_base_src_query_latency (GST_BASE_SRC (self->obj), &live,
				      &minlat, &maxlat);
    return Py_BuildValue("(OOKK)",
			 PyBool_FromLong(res),
			 PyBool_FromLong(live),
			 minlat, maxlat);
}
#line 15534 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_base_src_set_do_timestamp(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "timestamp", NULL };
    int timestamp;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GstBaseSrc.set_do_timestamp", kwlist, &timestamp))
        return NULL;
    pyg_begin_allow_threads;
    gst_base_src_set_do_timestamp(GST_BASE_SRC(self->obj), timestamp);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_base_src_get_do_timestamp(PyGObject *self)
{
    int ret;

    pyg_begin_allow_threads;
    ret = gst_base_src_get_do_timestamp(GST_BASE_SRC(self->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_GstBaseSrc__do_get_caps(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    GstCaps *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstBaseSrc.get_caps", kwlist, &PyGstBaseSrc_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_BASE_SRC_CLASS(klass)->get_caps) {
        pyg_begin_allow_threads;
        ret = GST_BASE_SRC_CLASS(klass)->get_caps(GST_BASE_SRC(self->obj));
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstBaseSrc.get_caps not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return pyg_boxed_new (GST_TYPE_CAPS, ret, FALSE, TRUE);
}

static PyObject *
_wrap_GstBaseSrc__do_set_caps(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "caps", NULL };
    PyGObject *self;
    PyObject *py_caps;
    int ret;
    GstCaps *caps;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O:GstBaseSrc.set_caps", kwlist, &PyGstBaseSrc_Type, &self, &py_caps))
        return NULL;
    caps = pygst_caps_from_pyobject (py_caps, NULL);
    if (PyErr_Occurred())
      return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_BASE_SRC_CLASS(klass)->set_caps) {
        pyg_begin_allow_threads;
        ret = GST_BASE_SRC_CLASS(klass)->set_caps(GST_BASE_SRC(self->obj), caps);
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstBaseSrc.set_caps not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_GstBaseSrc__do_negotiate(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstBaseSrc.negotiate", kwlist, &PyGstBaseSrc_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_BASE_SRC_CLASS(klass)->negotiate) {
        pyg_begin_allow_threads;
        ret = GST_BASE_SRC_CLASS(klass)->negotiate(GST_BASE_SRC(self->obj));
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstBaseSrc.negotiate not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_GstBaseSrc__do_newsegment(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstBaseSrc.newsegment", kwlist, &PyGstBaseSrc_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_BASE_SRC_CLASS(klass)->newsegment) {
        pyg_begin_allow_threads;
        ret = GST_BASE_SRC_CLASS(klass)->newsegment(GST_BASE_SRC(self->obj));
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstBaseSrc.newsegment not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_GstBaseSrc__do_start(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstBaseSrc.start", kwlist, &PyGstBaseSrc_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_BASE_SRC_CLASS(klass)->start) {
        pyg_begin_allow_threads;
        ret = GST_BASE_SRC_CLASS(klass)->start(GST_BASE_SRC(self->obj));
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstBaseSrc.start not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_GstBaseSrc__do_stop(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstBaseSrc.stop", kwlist, &PyGstBaseSrc_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_BASE_SRC_CLASS(klass)->stop) {
        pyg_begin_allow_threads;
        ret = GST_BASE_SRC_CLASS(klass)->stop(GST_BASE_SRC(self->obj));
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstBaseSrc.stop not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyBool_FromLong(ret);

}

#line 329 "gstbase.override"
static PyObject *
_wrap_GstBaseSrc__do_get_times (PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "buffer", NULL };
    PyGObject *self;
    PyGstMiniObject *py_buffer;
    GstClockTime start = 0;
    GstClockTime end = 0;
    PyObject *py_ret;
    
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O!O!:GstBaseSrc.get_times",
				     kwlist, &PyGstBaseSrc_Type, &self,
				     &PyGstBuffer_Type, &py_buffer))
	return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_BASE_SRC_CLASS(klass)->get_times) {
	pyg_begin_allow_threads;
	GST_BASE_SRC_CLASS(klass)->get_times(GST_BASE_SRC(self->obj),
					     GST_BUFFER(py_buffer->obj),
					     &start, &end);
	pyg_end_allow_threads;
    } else {
      PyErr_SetString(PyExc_NotImplementedError, "virtual method GstBaseSrc.get_times not implemented");
      g_type_class_unref(klass);
      return NULL;
    }
    g_type_class_unref(klass);
    
    py_ret = PyTuple_New(2);
    PyTuple_SetItem(py_ret, 0, PyLong_FromUnsignedLongLong(start));
    PyTuple_SetItem(py_ret, 1, PyLong_FromUnsignedLongLong(end));

    return py_ret;
}
#line 15754 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 238 "gstbase.override"
static PyObject *
_wrap_GstBaseSrc__do_get_size (PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    gboolean ret;
    guint64 size = 0;
    PyObject *py_ret;
    
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O!:GstBaseSrc.get_size",
				     kwlist, &PyGstBaseSrc_Type, &self))
	return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_BASE_SRC_CLASS(klass)->get_size) {
      pyg_begin_allow_threads;
      ret = GST_BASE_SRC_CLASS(klass)->get_size(GST_BASE_SRC(self->obj), &size);
      pyg_end_allow_threads;
    } else {
      PyErr_SetString(PyExc_NotImplementedError, "virtual method GstBaseSrc.get_size not implemented");
      g_type_class_unref(klass);
      return NULL;
    }
    g_type_class_unref(klass);
    
    py_ret = PyLong_FromUnsignedLongLong(size);

    return py_ret;
}
#line 15787 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_GstBaseSrc__do_is_seekable(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstBaseSrc.is_seekable", kwlist, &PyGstBaseSrc_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_BASE_SRC_CLASS(klass)->is_seekable) {
        pyg_begin_allow_threads;
        ret = GST_BASE_SRC_CLASS(klass)->is_seekable(GST_BASE_SRC(self->obj));
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstBaseSrc.is_seekable not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_GstBaseSrc__do_unlock(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstBaseSrc.unlock", kwlist, &PyGstBaseSrc_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_BASE_SRC_CLASS(klass)->unlock) {
        pyg_begin_allow_threads;
        ret = GST_BASE_SRC_CLASS(klass)->unlock(GST_BASE_SRC(self->obj));
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstBaseSrc.unlock not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_GstBaseSrc__do_event(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "event", NULL };
    PyGObject *self;
    int ret;
    PyGstMiniObject *event;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!:GstBaseSrc.event", kwlist, &PyGstBaseSrc_Type, &self, &PyGstEvent_Type, &event))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_BASE_SRC_CLASS(klass)->event) {
        pyg_begin_allow_threads;
        ret = GST_BASE_SRC_CLASS(klass)->event(GST_BASE_SRC(self->obj), GST_EVENT(event->obj));
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstBaseSrc.event not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyBool_FromLong(ret);

}

#line 138 "gstbase.override"
static PyObject *
_wrap_GstBaseSrc__do_create (PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "offset", "size", NULL };
    PyGObject *self;
    guint64 offset;
    guint size;
    PyObject *py_ret;
    GstFlowReturn flow;
    GstBuffer *buffer = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O!KI:GstBaseSrc.create",
				     kwlist, &PyGstBaseSrc_Type, &self, &offset, &size))
        return NULL;

    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_BASE_SRC_CLASS(klass)->create) {
	pyg_begin_allow_threads;
      flow = GST_BASE_SRC_CLASS(klass)->create(GST_BASE_SRC(self->obj), offset, size, &buffer);
      pyg_end_allow_threads;
    } else {
      PyErr_SetString(PyExc_NotImplementedError, "virtual method GstBaseSrc.set_caps not implemented");
      g_type_class_unref(klass);
      return NULL;
    }
    g_type_class_unref(klass);

    /* We now need to return a tuple with (flow, buffer) */
    if (buffer)
      py_ret = PyTuple_New(2);
    else
      py_ret = PyTuple_New(1);

    PyTuple_SET_ITEM(py_ret, 0, pyg_enum_from_gtype(GST_TYPE_FLOW_RETURN, flow));

    if (buffer)
      PyTuple_SET_ITEM(py_ret, 1, pygstminiobject_new(GST_MINI_OBJECT (buffer)));

    return py_ret;
}
#line 15908 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_GstBaseSrc__do_do_seek(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "segment", NULL };
    PyGObject *self;
    PyObject *py_segment;
    int ret;
    GstSegment *segment = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O:GstBaseSrc.do_seek", kwlist, &PyGstBaseSrc_Type, &self, &py_segment))
        return NULL;
    if (pyg_boxed_check(py_segment, GST_TYPE_SEGMENT))
        segment = pyg_boxed_get(py_segment, GstSegment);
    else {
        PyErr_SetString(PyExc_TypeError, "segment should be a GstSegment");
        return NULL;
    }
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_BASE_SRC_CLASS(klass)->do_seek) {
        pyg_begin_allow_threads;
        ret = GST_BASE_SRC_CLASS(klass)->do_seek(GST_BASE_SRC(self->obj), segment);
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstBaseSrc.do_seek not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_GstBaseSrc__do_query(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "query", NULL };
    PyGObject *self;
    int ret;
    PyGstMiniObject *query;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!:GstBaseSrc.query", kwlist, &PyGstBaseSrc_Type, &self, &PyGstQuery_Type, &query))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_BASE_SRC_CLASS(klass)->query) {
        pyg_begin_allow_threads;
        ret = GST_BASE_SRC_CLASS(klass)->query(GST_BASE_SRC(self->obj), GST_QUERY(query->obj));
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstBaseSrc.query not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_GstBaseSrc__do_check_get_range(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstBaseSrc.check_get_range", kwlist, &PyGstBaseSrc_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_BASE_SRC_CLASS(klass)->check_get_range) {
        pyg_begin_allow_threads;
        ret = GST_BASE_SRC_CLASS(klass)->check_get_range(GST_BASE_SRC(self->obj));
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstBaseSrc.check_get_range not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_GstBaseSrc__do_fixate(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "caps", NULL };
    PyGObject *self;
    PyObject *py_caps;
    GstCaps *caps;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O:GstBaseSrc.fixate", kwlist, &PyGstBaseSrc_Type, &self, &py_caps))
        return NULL;
    caps = pygst_caps_from_pyobject (py_caps, NULL);
    if (PyErr_Occurred())
      return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_BASE_SRC_CLASS(klass)->fixate) {
        pyg_begin_allow_threads;
        GST_BASE_SRC_CLASS(klass)->fixate(GST_BASE_SRC(self->obj), caps);
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstBaseSrc.fixate not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GstBaseSrc__do_unlock_stop(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstBaseSrc.unlock_stop", kwlist, &PyGstBaseSrc_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_BASE_SRC_CLASS(klass)->unlock_stop) {
        pyg_begin_allow_threads;
        ret = GST_BASE_SRC_CLASS(klass)->unlock_stop(GST_BASE_SRC(self->obj));
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstBaseSrc.unlock_stop not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_GstBaseSrc__do_prepare_seek_segment(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "seek", "segment", NULL };
    PyGObject *self;
    PyObject *py_segment;
    int ret;
    GstSegment *segment = NULL;
    PyGstMiniObject *seek;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!O:GstBaseSrc.prepare_seek_segment", kwlist, &PyGstBaseSrc_Type, &self, &PyGstEvent_Type, &seek, &py_segment))
        return NULL;
    if (pyg_boxed_check(py_segment, GST_TYPE_SEGMENT))
        segment = pyg_boxed_get(py_segment, GstSegment);
    else {
        PyErr_SetString(PyExc_TypeError, "segment should be a GstSegment");
        return NULL;
    }
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_BASE_SRC_CLASS(klass)->prepare_seek_segment) {
        pyg_begin_allow_threads;
        ret = GST_BASE_SRC_CLASS(klass)->prepare_seek_segment(GST_BASE_SRC(self->obj), GST_EVENT(seek->obj), segment);
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstBaseSrc.prepare_seek_segment not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyBool_FromLong(ret);

}

static const PyMethodDef _PyGstBaseSrc_methods[] = {
    { "wait_playing", (PyCFunction)_wrap_gst_base_src_wait_playing, METH_NOARGS,
      NULL },
    { "set_live", (PyCFunction)_wrap_gst_base_src_set_live, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "is_live", (PyCFunction)_wrap_gst_base_src_is_live, METH_NOARGS,
      NULL },
    { "set_format", (PyCFunction)_wrap_gst_base_src_set_format, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "query_latency", (PyCFunction)_wrap_gst_base_src_query_latency, METH_NOARGS,
      NULL },
    { "set_do_timestamp", (PyCFunction)_wrap_gst_base_src_set_do_timestamp, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_do_timestamp", (PyCFunction)_wrap_gst_base_src_get_do_timestamp, METH_NOARGS,
      NULL },
    { "do_get_caps", (PyCFunction)_wrap_GstBaseSrc__do_get_caps, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_set_caps", (PyCFunction)_wrap_GstBaseSrc__do_set_caps, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_negotiate", (PyCFunction)_wrap_GstBaseSrc__do_negotiate, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_newsegment", (PyCFunction)_wrap_GstBaseSrc__do_newsegment, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_start", (PyCFunction)_wrap_GstBaseSrc__do_start, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_stop", (PyCFunction)_wrap_GstBaseSrc__do_stop, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_times", (PyCFunction)_wrap_GstBaseSrc__do_get_times, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_size", (PyCFunction)_wrap_GstBaseSrc__do_get_size, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_is_seekable", (PyCFunction)_wrap_GstBaseSrc__do_is_seekable, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_unlock", (PyCFunction)_wrap_GstBaseSrc__do_unlock, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_event", (PyCFunction)_wrap_GstBaseSrc__do_event, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_create", (PyCFunction)_wrap_GstBaseSrc__do_create, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_do_seek", (PyCFunction)_wrap_GstBaseSrc__do_do_seek, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_query", (PyCFunction)_wrap_GstBaseSrc__do_query, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_check_get_range", (PyCFunction)_wrap_GstBaseSrc__do_check_get_range, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_fixate", (PyCFunction)_wrap_GstBaseSrc__do_fixate, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_unlock_stop", (PyCFunction)_wrap_GstBaseSrc__do_unlock_stop, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_prepare_seek_segment", (PyCFunction)_wrap_GstBaseSrc__do_prepare_seek_segment, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject PyGstBaseSrc_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.BaseSrc",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGstBaseSrc_methods, /* tp_methods */
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

static GstCaps*
_wrap_GstBaseSrc__proxy_do_get_caps(GstBaseSrc *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    GstCaps* retval;
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_get_caps");
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
    retval = (GstCaps*) pygst_caps_from_pyobject (py_retval, NULL);
    
    
    Py_DECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gboolean
_wrap_GstBaseSrc__proxy_do_set_caps(GstBaseSrc *self, GstCaps*caps)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_caps = NULL;
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
    if (caps)
        py_caps = pyg_boxed_new (GST_TYPE_CAPS, caps, FALSE, TRUE);
    else {
        Py_INCREF(Py_None);
        py_caps = Py_None;
    }
    
    py_args = PyTuple_New(1);
    Py_INCREF(py_caps);
    PyTuple_SET_ITEM(py_args, 0, py_caps);
    
    py_method = PyObject_GetAttrString(py_self, "do_set_caps");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        gst_caps_ref(caps);
        Py_DECREF(py_caps);
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
        gst_caps_ref(caps);
        Py_DECREF(py_caps);
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
        gst_caps_ref(caps);
        Py_DECREF(py_caps);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_DECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    gst_caps_ref(caps);
    Py_DECREF(py_caps);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gboolean
_wrap_GstBaseSrc__proxy_do_negotiate(GstBaseSrc *self)
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_negotiate");
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
_wrap_GstBaseSrc__proxy_do_newsegment(GstBaseSrc *self)
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_newsegment");
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
_wrap_GstBaseSrc__proxy_do_start(GstBaseSrc *self)
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
_wrap_GstBaseSrc__proxy_do_stop(GstBaseSrc *self)
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
#line 269 "gstbase.override"
static void
_wrap_GstBaseSrc__proxy_do_get_times (GstBaseSrc * self,
				      GstBuffer *buffer,
				      GstClockTime * start,
				      GstClockTime * end)
{
    PyGILState_STATE __py_state;
    PyObject *py_args;
    PyObject *py_self;
    PyObject *py_method;
    PyObject *py_ret;
    
    __py_state = pyg_gil_state_ensure();
    
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
	return;
    }

    py_args = Py_BuildValue ("(N)",
			     pygstminiobject_new((GstMiniObject *)buffer));
    
    py_method = PyObject_GetAttrString(py_self, "do_get_times");
    
    Py_DECREF(py_self);

    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
	goto beach;
    }

    py_ret = PyObject_CallObject(py_method, py_args);

    Py_DECREF(py_method);

    if (!py_ret) {
        if (PyErr_Occurred())
            PyErr_Print();
	goto beach;
    }

    /*
      If the method returned a numeric, the return value will be TRUE.
      For ANY other case, we don't set size and the return value is FALSE.
    */

    if ((PyTuple_Check(py_ret)) && (PyTuple_Size (py_ret) == 2))
	PyArg_ParseTuple (py_ret, "KK", start, end);

    Py_DECREF (py_ret);
 beach:
    Py_DECREF (py_args);
    pyg_gil_state_release(__py_state);
    return;
}
#line 16595 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 181 "gstbase.override"
static gboolean
_wrap_GstBaseSrc__proxy_do_get_size (GstBaseSrc * self,
				     guint64 * size)
{
  PyGILState_STATE __py_state;
  PyObject *py_self;
  gboolean ret = FALSE;
  PyObject *py_method;
  PyObject *py_ret;

    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
	goto beach;
    }
    
    py_method = PyObject_GetAttrString(py_self, "do_get_size");

    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
	goto beach;
    }

    py_ret = PyObject_CallObject(py_method, NULL);
    if (!py_ret) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_method);
        Py_DECREF(py_self);
	goto beach;
    }

    /*
      If the method returned a numeric, the return value will be TRUE.
      For ANY other case, we don't set size and the return value is FALSE.
    */

    if (PyLong_Check(py_ret)) {
	*size = PyLong_AsUnsignedLongLongMask(py_ret);
	ret = TRUE;
    }

    Py_DECREF(py_method);
    Py_DECREF(py_self);
    Py_DECREF(py_ret);

 beach:
    pyg_gil_state_release(__py_state);
    
    return ret;
}
#line 16654 "..\\..\\Source\\gst-python\\gst\\gst.c"


static gboolean
_wrap_GstBaseSrc__proxy_do_is_seekable(GstBaseSrc *self)
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_is_seekable");
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
_wrap_GstBaseSrc__proxy_do_unlock(GstBaseSrc *self)
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_unlock");
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
_wrap_GstBaseSrc__proxy_do_event(GstBaseSrc *self, GstEvent*event)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_event = NULL;
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
    if (event) {
        py_event = pygstminiobject_new((GstMiniObject *) event);
        gst_mini_object_unref ((GstMiniObject *) event);
    } else {
        Py_INCREF(Py_None);
        py_event = Py_None;
    }
    
    py_args = PyTuple_New(1);
    Py_INCREF(py_event);
    PyTuple_SET_ITEM(py_args, 0, py_event);
    
    py_method = PyObject_GetAttrString(py_self, "do_event");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        gst_mini_object_ref ((GstMiniObject *) event); Py_DECREF(py_event);
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
        gst_mini_object_ref ((GstMiniObject *) event); Py_DECREF(py_event);
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
        gst_mini_object_ref ((GstMiniObject *) event); Py_DECREF(py_event);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_DECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    gst_mini_object_ref ((GstMiniObject *) event); Py_DECREF(py_event);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
#line 25 "gstbase.override"
static GstFlowReturn
_wrap_GstBaseSrc__proxy_do_create (GstBaseSrc * self,
				   guint64 offset,
				   guint size,
				   GstBuffer ** buf)
{
  PyGILState_STATE __py_state;
  PyObject *py_self;
  GstFlowReturn retval = GST_FLOW_ERROR;
  PyObject *py_ret;
  PyObject *py_flow;
  PyObject *py_buffer;
  PyObject *py_args;
  PyObject *py_method;

    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
	goto beach;
    }

    py_args = PyTuple_New(2);
    PyTuple_SET_ITEM(py_args, 0, PyLong_FromUnsignedLongLong(offset));
    PyTuple_SET_ITEM(py_args, 1, PyInt_FromLong(size));
    
    py_method = PyObject_GetAttrString(py_self, "do_create");

    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
	goto beach;
    }

    py_ret = PyObject_CallObject(py_method, py_args);
    if (!py_ret) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
	goto beach;
    }
    
    /* process the python return value */
    /* Should be a list containing the gst.FlowReturn and the gst.Buffer */
    if (PyTuple_Check(py_ret)) {
      /* gst.FlowReturn */
      py_flow = PyTuple_GetItem(py_ret, 0);
      
      if (!py_flow) {
        if (PyErr_Occurred())
	  PyErr_Print();
	Py_DECREF(py_ret);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
	goto beach;
      }
      
      if (py_flow == Py_None) {
	  GST_ERROR ("None return flow is not valid");
	  goto beach;
      }

      GST_DEBUG ("py_flow:%p", py_flow);

      if (pyg_enum_get_value(GST_TYPE_FLOW_RETURN, py_flow, (gint*) &retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
	Py_DECREF(py_ret);
	Py_DECREF(py_flow);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
	retval = GST_FLOW_ERROR;
	goto beach;
      }

      py_buffer = PyTuple_GetItem(py_ret, 1);
      if (!py_buffer) {
        if (PyErr_Occurred())
	  PyErr_Print();
	Py_DECREF(py_ret);
	Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
	goto beach;
      }
      
      if (pygstminiobject_check(py_buffer, &PyGstBuffer_Type)) {
	*buf = GST_BUFFER (pygstminiobject_get (py_buffer));
	gst_buffer_ref (*buf);
      } else {
	*buf = NULL;
      }
    }
    
    Py_DECREF(py_ret);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);

 beach:
    pyg_gil_state_release(__py_state);
    
    return retval;
}
#line 16960 "..\\..\\Source\\gst-python\\gst\\gst.c"


static gboolean
_wrap_GstBaseSrc__proxy_do_do_seek(GstBaseSrc *self, GstSegment*segment)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_segment;
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
    py_segment = pyg_boxed_new(GST_TYPE_SEGMENT, segment, FALSE, FALSE);
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_segment);
    
    py_method = PyObject_GetAttrString(py_self, "do_do_seek");
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
static gboolean
_wrap_GstBaseSrc__proxy_do_query(GstBaseSrc *self, GstQuery*query)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_query = NULL;
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
    if (query) {
        py_query = pygstminiobject_new((GstMiniObject *) query);
        gst_mini_object_unref ((GstMiniObject *) query);
    } else {
        Py_INCREF(Py_None);
        py_query = Py_None;
    }
    
    py_args = PyTuple_New(1);
    Py_INCREF(py_query);
    PyTuple_SET_ITEM(py_args, 0, py_query);
    
    py_method = PyObject_GetAttrString(py_self, "do_query");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        gst_mini_object_ref ((GstMiniObject *) query); Py_DECREF(py_query);
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
        gst_mini_object_ref ((GstMiniObject *) query); Py_DECREF(py_query);
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
        gst_mini_object_ref ((GstMiniObject *) query); Py_DECREF(py_query);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_DECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    gst_mini_object_ref ((GstMiniObject *) query); Py_DECREF(py_query);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gboolean
_wrap_GstBaseSrc__proxy_do_check_get_range(GstBaseSrc *self)
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_check_get_range");
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
static void
_wrap_GstBaseSrc__proxy_do_fixate(GstBaseSrc *self, GstCaps*caps)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_caps = NULL;
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
    if (caps)
        py_caps = pyg_boxed_new (GST_TYPE_CAPS, caps, FALSE, TRUE);
    else {
        Py_INCREF(Py_None);
        py_caps = Py_None;
    }
    
    py_args = PyTuple_New(1);
    Py_INCREF(py_caps);
    PyTuple_SET_ITEM(py_args, 0, py_caps);
    
    py_method = PyObject_GetAttrString(py_self, "do_fixate");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        gst_caps_ref(caps);
        Py_DECREF(py_caps);
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
        gst_caps_ref(caps);
        Py_DECREF(py_caps);
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
        gst_caps_ref(caps);
        Py_DECREF(py_caps);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    Py_DECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    gst_caps_ref(caps);
    Py_DECREF(py_caps);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
}
static gboolean
_wrap_GstBaseSrc__proxy_do_unlock_stop(GstBaseSrc *self)
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_unlock_stop");
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
_wrap_GstBaseSrc__proxy_do_prepare_seek_segment(GstBaseSrc *self, GstEvent*seek, GstSegment*segment)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_seek = NULL;
    PyObject *py_segment;
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
    if (seek) {
        py_seek = pygstminiobject_new((GstMiniObject *) seek);
        gst_mini_object_unref ((GstMiniObject *) seek);
    } else {
        Py_INCREF(Py_None);
        py_seek = Py_None;
    }
    py_segment = pyg_boxed_new(GST_TYPE_SEGMENT, segment, FALSE, FALSE);
    
    py_args = PyTuple_New(2);
    Py_INCREF(py_seek);
    PyTuple_SET_ITEM(py_args, 0, py_seek);
    PyTuple_SET_ITEM(py_args, 1, py_segment);
    
    py_method = PyObject_GetAttrString(py_self, "do_prepare_seek_segment");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        gst_mini_object_ref ((GstMiniObject *) seek); Py_DECREF(py_seek);
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
        gst_mini_object_ref ((GstMiniObject *) seek); Py_DECREF(py_seek);
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
        gst_mini_object_ref ((GstMiniObject *) seek); Py_DECREF(py_seek);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_DECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    gst_mini_object_ref ((GstMiniObject *) seek); Py_DECREF(py_seek);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}

static int
__GstBaseSrc_class_init(gpointer gclass, PyTypeObject *pyclass)
{
    PyObject *o;
    GstBaseSrcClass *klass = GST_BASE_SRC_CLASS(gclass);
    PyObject *gsignals = PyDict_GetItemString(pyclass->tp_dict, "__gsignals__");

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_get_caps");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "get_caps")))
            klass->get_caps = _wrap_GstBaseSrc__proxy_do_get_caps;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_set_caps");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "set_caps")))
            klass->set_caps = _wrap_GstBaseSrc__proxy_do_set_caps;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_negotiate");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "negotiate")))
            klass->negotiate = _wrap_GstBaseSrc__proxy_do_negotiate;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_newsegment");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "newsegment")))
            klass->newsegment = _wrap_GstBaseSrc__proxy_do_newsegment;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_start");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "start")))
            klass->start = _wrap_GstBaseSrc__proxy_do_start;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_stop");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "stop")))
            klass->stop = _wrap_GstBaseSrc__proxy_do_stop;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_get_times");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "get_times")))
            klass->get_times = _wrap_GstBaseSrc__proxy_do_get_times;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_get_size");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "get_size")))
            klass->get_size = _wrap_GstBaseSrc__proxy_do_get_size;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_is_seekable");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "is_seekable")))
            klass->is_seekable = _wrap_GstBaseSrc__proxy_do_is_seekable;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_unlock");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "unlock")))
            klass->unlock = _wrap_GstBaseSrc__proxy_do_unlock;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_event");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "event")))
            klass->event = _wrap_GstBaseSrc__proxy_do_event;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_create");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "create")))
            klass->create = _wrap_GstBaseSrc__proxy_do_create;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_do_seek");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "do_seek")))
            klass->do_seek = _wrap_GstBaseSrc__proxy_do_do_seek;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_query");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "query")))
            klass->query = _wrap_GstBaseSrc__proxy_do_query;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_check_get_range");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "check_get_range")))
            klass->check_get_range = _wrap_GstBaseSrc__proxy_do_check_get_range;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_fixate");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "fixate")))
            klass->fixate = _wrap_GstBaseSrc__proxy_do_fixate;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_unlock_stop");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "unlock_stop")))
            klass->unlock_stop = _wrap_GstBaseSrc__proxy_do_unlock_stop;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_prepare_seek_segment");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "prepare_seek_segment")))
            klass->prepare_seek_segment = _wrap_GstBaseSrc__proxy_do_prepare_seek_segment;
        Py_DECREF(o);
    }
    return 0;
}


/* ----------- GstBaseTransform ----------- */

static PyObject *
_wrap_gst_base_transform_set_passthrough(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "passthrough", NULL };
    int passthrough;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GstBaseTransform.set_passthrough", kwlist, &passthrough))
        return NULL;
    pyg_begin_allow_threads;
    gst_base_transform_set_passthrough(GST_BASE_TRANSFORM(self->obj), passthrough);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_base_transform_is_passthrough(PyGObject *self)
{
    int ret;

    pyg_begin_allow_threads;
    ret = gst_base_transform_is_passthrough(GST_BASE_TRANSFORM(self->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_base_transform_set_in_place(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "in_place", NULL };
    int in_place;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GstBaseTransform.set_in_place", kwlist, &in_place))
        return NULL;
    pyg_begin_allow_threads;
    gst_base_transform_set_in_place(GST_BASE_TRANSFORM(self->obj), in_place);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_base_transform_is_in_place(PyGObject *self)
{
    int ret;

    pyg_begin_allow_threads;
    ret = gst_base_transform_is_in_place(GST_BASE_TRANSFORM(self->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_base_transform_update_qos(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "proportion", "diff", "timestamp", NULL };
    double proportion;
    gint64 diff;
    guint64 timestamp;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"dLK:GstBaseTransform.update_qos", kwlist, &proportion, &diff, &timestamp))
        return NULL;
    pyg_begin_allow_threads;
    gst_base_transform_update_qos(GST_BASE_TRANSFORM(self->obj), proportion, diff, timestamp);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_base_transform_set_qos_enabled(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "enabled", NULL };
    int enabled;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GstBaseTransform.set_qos_enabled", kwlist, &enabled))
        return NULL;
    pyg_begin_allow_threads;
    gst_base_transform_set_qos_enabled(GST_BASE_TRANSFORM(self->obj), enabled);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_base_transform_is_qos_enabled(PyGObject *self)
{
    int ret;

    pyg_begin_allow_threads;
    ret = gst_base_transform_is_qos_enabled(GST_BASE_TRANSFORM(self->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_base_transform_set_gap_aware(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "gap_aware", NULL };
    int gap_aware;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GstBaseTransform.set_gap_aware", kwlist, &gap_aware))
        return NULL;
    pyg_begin_allow_threads;
    gst_base_transform_set_gap_aware(GST_BASE_TRANSFORM(self->obj), gap_aware);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_base_transform_suggest(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "caps", "size", NULL };
    PyObject *py_caps, *py_size = NULL;
    guint size = 0;
    GstCaps *caps;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"OO:GstBaseTransform.suggest", kwlist, &py_caps, &py_size))
        return NULL;
    if (py_caps == Py_None || py_caps == NULL)
        caps = NULL;
    else
      caps = pygst_caps_from_pyobject (py_caps, NULL);
    if (PyErr_Occurred())
      return NULL;
    if (py_size) {
        if (PyLong_Check(py_size))
            size = PyLong_AsUnsignedLong(py_size);
        else if (PyInt_Check(py_size))
            size = PyInt_AsLong(py_size);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'size' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    pyg_begin_allow_threads;
    gst_base_transform_suggest(GST_BASE_TRANSFORM(self->obj), caps, size);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_base_transform_reconfigure(PyGObject *self)
{
    pyg_begin_allow_threads;
    gst_base_transform_reconfigure(GST_BASE_TRANSFORM(self->obj));
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GstBaseTransform__do_transform_caps(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "direction", "caps", NULL };
    PyGObject *self;
    PyObject *py_direction = NULL, *py_caps;
    GstPadDirection direction;
    GstCaps *caps, *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!OO:GstBaseTransform.transform_caps", kwlist, &PyGstBaseTransform_Type, &self, &py_direction, &py_caps))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_PAD_DIRECTION, py_direction, (gint *)&direction))
        return NULL;
    caps = pygst_caps_from_pyobject (py_caps, NULL);
    if (PyErr_Occurred())
      return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_BASE_TRANSFORM_CLASS(klass)->transform_caps) {
        pyg_begin_allow_threads;
        ret = GST_BASE_TRANSFORM_CLASS(klass)->transform_caps(GST_BASE_TRANSFORM(self->obj), direction, caps);
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstBaseTransform.transform_caps not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return pyg_boxed_new (GST_TYPE_CAPS, ret, FALSE, TRUE);
}

static PyObject *
_wrap_GstBaseTransform__do_fixate_caps(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "direction", "caps", "othercaps", NULL };
    PyGObject *self;
    PyObject *py_direction = NULL, *py_caps, *py_othercaps;
    GstPadDirection direction;
    GstCaps *caps, *othercaps;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!OOO:GstBaseTransform.fixate_caps", kwlist, &PyGstBaseTransform_Type, &self, &py_direction, &py_caps, &py_othercaps))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_PAD_DIRECTION, py_direction, (gint *)&direction))
        return NULL;
    caps = pygst_caps_from_pyobject (py_caps, NULL);
    if (PyErr_Occurred())
      return NULL;
    othercaps = pygst_caps_from_pyobject (py_othercaps, NULL);
    if (PyErr_Occurred())
      return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_BASE_TRANSFORM_CLASS(klass)->fixate_caps) {
        pyg_begin_allow_threads;
        GST_BASE_TRANSFORM_CLASS(klass)->fixate_caps(GST_BASE_TRANSFORM(self->obj), direction, caps, othercaps);
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstBaseTransform.fixate_caps not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    Py_INCREF(Py_None);
    return Py_None;
}

#line 646 "gstbase.override"
static PyObject *
_wrap_GstBaseTransform__do_transform_size (PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "direction", "caps", "size", "othercaps", NULL };
    PyGObject *self;
    PyGObject *py_direction;
    PyGObject *py_caps;
    PyGObject *py_othercaps;
    gboolean ret;
    GstPadDirection direction;
    guint size = 0;
    guint othersize = 0;
    PyObject *py_ret;
    
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O!O!O!iO!:GstBaseTransform.get_unit_size",
        kwlist, &PyGstBaseTransform_Type, &self, &PyGEnum_Type, &py_direction,
        &PyGstCaps_Type, &py_caps, &size, &PyGstCaps_Type, &py_othercaps))
        return NULL;
    
    pyg_enum_get_value(GST_TYPE_PAD_DIRECTION,
        (PyObject *) py_direction, (gint *) &direction);

    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_BASE_TRANSFORM_CLASS(klass)->transform_size) {
      pyg_begin_allow_threads;
      ret = GST_BASE_TRANSFORM_CLASS(klass)->transform_size(GST_BASE_TRANSFORM(self->obj),
          direction, GST_CAPS(py_caps->obj), size, 
          GST_CAPS(py_othercaps->obj), &othersize);
      pyg_end_allow_threads;
    } else {
      PyErr_SetString(PyExc_NotImplementedError, "virtual method GstBaseTransform.transform_size not implemented");
      g_type_class_unref(klass);
      return NULL;
    }
    g_type_class_unref(klass);

    py_ret = PyLong_FromUnsignedLongLong(othersize);
    return py_ret;
}
#line 17831 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 533 "gstbase.override"
static PyObject *
_wrap_GstBaseTransform__do_get_unit_size (PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "caps", NULL };
    PyGObject *self;
    PyGObject *caps;
    gboolean ret;
    guint size = 0;
    PyObject *py_ret;
    
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O!O!:GstBaseTransform.get_unit_size",
				     kwlist, &PyGstBaseTransform_Type, &self, &PyGstCaps_Type, &caps))
	return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_BASE_TRANSFORM_CLASS(klass)->get_unit_size) {
      pyg_begin_allow_threads;
      ret = GST_BASE_TRANSFORM_CLASS(klass)->get_unit_size(GST_BASE_TRANSFORM(self->obj), GST_CAPS(caps->obj), &size);
      pyg_end_allow_threads;
    } else {
      PyErr_SetString(PyExc_NotImplementedError, "virtual method GstBaseTransform.get_unit_size not implemented");
      g_type_class_unref(klass);
      return NULL;
    }
    g_type_class_unref(klass);
    
    py_ret = PyLong_FromUnsignedLongLong(size);

    return py_ret;
}
#line 17865 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_GstBaseTransform__do_set_caps(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "incaps", "outcaps", NULL };
    PyGObject *self;
    PyObject *py_incaps, *py_outcaps;
    int ret;
    GstCaps *incaps, *outcaps;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!OO:GstBaseTransform.set_caps", kwlist, &PyGstBaseTransform_Type, &self, &py_incaps, &py_outcaps))
        return NULL;
    incaps = pygst_caps_from_pyobject (py_incaps, NULL);
    if (PyErr_Occurred())
      return NULL;
    outcaps = pygst_caps_from_pyobject (py_outcaps, NULL);
    if (PyErr_Occurred())
      return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_BASE_TRANSFORM_CLASS(klass)->set_caps) {
        pyg_begin_allow_threads;
        ret = GST_BASE_TRANSFORM_CLASS(klass)->set_caps(GST_BASE_TRANSFORM(self->obj), incaps, outcaps);
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstBaseTransform.set_caps not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_GstBaseTransform__do_start(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstBaseTransform.start", kwlist, &PyGstBaseTransform_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_BASE_TRANSFORM_CLASS(klass)->start) {
        pyg_begin_allow_threads;
        ret = GST_BASE_TRANSFORM_CLASS(klass)->start(GST_BASE_TRANSFORM(self->obj));
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstBaseTransform.start not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_GstBaseTransform__do_stop(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstBaseTransform.stop", kwlist, &PyGstBaseTransform_Type, &self))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_BASE_TRANSFORM_CLASS(klass)->stop) {
        pyg_begin_allow_threads;
        ret = GST_BASE_TRANSFORM_CLASS(klass)->stop(GST_BASE_TRANSFORM(self->obj));
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstBaseTransform.stop not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_GstBaseTransform__do_event(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "event", NULL };
    PyGObject *self;
    int ret;
    PyGstMiniObject *event;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!:GstBaseTransform.event", kwlist, &PyGstBaseTransform_Type, &self, &PyGstEvent_Type, &event))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_BASE_TRANSFORM_CLASS(klass)->event) {
        pyg_begin_allow_threads;
        ret = GST_BASE_TRANSFORM_CLASS(klass)->event(GST_BASE_TRANSFORM(self->obj), GST_EVENT(event->obj));
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstBaseTransform.event not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_GstBaseTransform__do_transform(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "inbuf", "outbuf", NULL };
    PyGObject *self;
    gint ret;
    PyGstMiniObject *inbuf, *outbuf;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!O!:GstBaseTransform.transform", kwlist, &PyGstBaseTransform_Type, &self, &PyGstBuffer_Type, &inbuf, &PyGstBuffer_Type, &outbuf))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_BASE_TRANSFORM_CLASS(klass)->transform) {
        pyg_begin_allow_threads;
        ret = GST_BASE_TRANSFORM_CLASS(klass)->transform(GST_BASE_TRANSFORM(self->obj), GST_BUFFER(inbuf->obj), GST_BUFFER(outbuf->obj));
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstBaseTransform.transform not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return pyg_enum_from_gtype(GST_TYPE_FLOW_RETURN, ret);
}

static PyObject *
_wrap_GstBaseTransform__do_transform_ip(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "buf", NULL };
    PyGObject *self;
    gint ret;
    PyGstMiniObject *buf;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!:GstBaseTransform.transform_ip", kwlist, &PyGstBaseTransform_Type, &self, &PyGstBuffer_Type, &buf))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_BASE_TRANSFORM_CLASS(klass)->transform_ip) {
        pyg_begin_allow_threads;
        ret = GST_BASE_TRANSFORM_CLASS(klass)->transform_ip(GST_BASE_TRANSFORM(self->obj), GST_BUFFER(buf->obj));
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstBaseTransform.transform_ip not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return pyg_enum_from_gtype(GST_TYPE_FLOW_RETURN, ret);
}

#line 817 "gstbase.override"
static PyObject *
_wrap_GstBaseTransform__do_src_event(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "event", NULL };
    PyGObject *self;
    int ret;
    PyGstMiniObject *event;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!:GstBaseTransform.src_event", kwlist, &PyGstBaseTransform_Type, &self, &PyGstEvent_Type, &event))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GST_BASE_TRANSFORM_CLASS(klass)->src_event) {
        pyg_begin_allow_threads;
        ret = GST_BASE_TRANSFORM_CLASS(klass)->src_event(GST_BASE_TRANSFORM(self->obj),
            gst_event_ref (GST_EVENT(event->obj)));
        pyg_end_allow_threads;
    } else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GstBaseTransform.src_event not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyBool_FromLong(ret);

}
#line 18054 "..\\..\\Source\\gst-python\\gst\\gst.c"


static const PyMethodDef _PyGstBaseTransform_methods[] = {
    { "set_passthrough", (PyCFunction)_wrap_gst_base_transform_set_passthrough, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "is_passthrough", (PyCFunction)_wrap_gst_base_transform_is_passthrough, METH_NOARGS,
      NULL },
    { "set_in_place", (PyCFunction)_wrap_gst_base_transform_set_in_place, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "is_in_place", (PyCFunction)_wrap_gst_base_transform_is_in_place, METH_NOARGS,
      NULL },
    { "update_qos", (PyCFunction)_wrap_gst_base_transform_update_qos, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_qos_enabled", (PyCFunction)_wrap_gst_base_transform_set_qos_enabled, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "is_qos_enabled", (PyCFunction)_wrap_gst_base_transform_is_qos_enabled, METH_NOARGS,
      NULL },
    { "set_gap_aware", (PyCFunction)_wrap_gst_base_transform_set_gap_aware, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "suggest", (PyCFunction)_wrap_gst_base_transform_suggest, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "reconfigure", (PyCFunction)_wrap_gst_base_transform_reconfigure, METH_NOARGS,
      NULL },
    { "do_transform_caps", (PyCFunction)_wrap_GstBaseTransform__do_transform_caps, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_fixate_caps", (PyCFunction)_wrap_GstBaseTransform__do_fixate_caps, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_transform_size", (PyCFunction)_wrap_GstBaseTransform__do_transform_size, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_unit_size", (PyCFunction)_wrap_GstBaseTransform__do_get_unit_size, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_transform_size", (PyCFunction)_wrap_GstBaseTransform__do_transform_size, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_set_caps", (PyCFunction)_wrap_GstBaseTransform__do_set_caps, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_start", (PyCFunction)_wrap_GstBaseTransform__do_start, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_stop", (PyCFunction)_wrap_GstBaseTransform__do_stop, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_event", (PyCFunction)_wrap_GstBaseTransform__do_event, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_transform", (PyCFunction)_wrap_GstBaseTransform__do_transform, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_transform_ip", (PyCFunction)_wrap_GstBaseTransform__do_transform_ip, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_src_event", (PyCFunction)_wrap_GstBaseTransform__do_src_event, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject PyGstBaseTransform_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.BaseTransform",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGstBaseTransform_methods, /* tp_methods */
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

static GstCaps*
_wrap_GstBaseTransform__proxy_do_transform_caps(GstBaseTransform *self, GstPadDirection direction, GstCaps*caps)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_direction;
    PyObject *py_caps = NULL;
    GstCaps* retval;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    py_direction = pyg_enum_from_gtype(GST_TYPE_PAD_DIRECTION, direction);
    if (!py_direction) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    if (caps)
        py_caps = pyg_boxed_new (GST_TYPE_CAPS, caps, FALSE, TRUE);
    else {
        Py_INCREF(Py_None);
        py_caps = Py_None;
    }
    
    py_args = PyTuple_New(2);
    PyTuple_SET_ITEM(py_args, 0, py_direction);
    Py_INCREF(py_caps);
    PyTuple_SET_ITEM(py_args, 1, py_caps);
    
    py_method = PyObject_GetAttrString(py_self, "do_transform_caps");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        gst_caps_ref(caps);
        Py_DECREF(py_caps);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        gst_caps_ref(caps);
        Py_DECREF(py_caps);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    retval = (GstCaps*) pygst_caps_from_pyobject (py_retval, NULL);
    
    
    Py_DECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    gst_caps_ref(caps);
    Py_DECREF(py_caps);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static void
_wrap_GstBaseTransform__proxy_do_fixate_caps(GstBaseTransform *self, GstPadDirection direction, GstCaps*caps, GstCaps*othercaps)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_direction;
    PyObject *py_caps = NULL;
    PyObject *py_othercaps = NULL;
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
    py_direction = pyg_enum_from_gtype(GST_TYPE_PAD_DIRECTION, direction);
    if (!py_direction) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    if (caps)
        py_caps = pyg_boxed_new (GST_TYPE_CAPS, caps, FALSE, TRUE);
    else {
        Py_INCREF(Py_None);
        py_caps = Py_None;
    }
    if (othercaps)
        py_othercaps = pyg_boxed_new (GST_TYPE_CAPS, othercaps, FALSE, TRUE);
    else {
        Py_INCREF(Py_None);
        py_othercaps = Py_None;
    }
    
    py_args = PyTuple_New(3);
    PyTuple_SET_ITEM(py_args, 0, py_direction);
    Py_INCREF(py_caps);
    PyTuple_SET_ITEM(py_args, 1, py_caps);
    Py_INCREF(py_othercaps);
    PyTuple_SET_ITEM(py_args, 2, py_othercaps);
    
    py_method = PyObject_GetAttrString(py_self, "do_fixate_caps");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        gst_caps_ref(othercaps);
        Py_DECREF(py_othercaps);
        gst_caps_ref(caps);
        Py_DECREF(py_caps);
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
        gst_caps_ref(othercaps);
        Py_DECREF(py_othercaps);
        gst_caps_ref(caps);
        Py_DECREF(py_caps);
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
        gst_caps_ref(othercaps);
        Py_DECREF(py_othercaps);
        gst_caps_ref(caps);
        Py_DECREF(py_caps);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    Py_DECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    gst_caps_ref(othercaps);
    Py_DECREF(py_othercaps);
    gst_caps_ref(caps);
    Py_DECREF(py_caps);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
}
#line 565 "gstbase.override"
static gboolean
_wrap_GstBaseTransform__proxy_do_transform_size (GstBaseTransform * self,
    GstPadDirection direction,
    GstCaps * caps,
    guint size,
    GstCaps * othercaps,
    guint * othersize)
{
    PyGILState_STATE __py_state;
    PyObject *py_self = NULL;
    PyObject *py_direction = NULL;
    PyObject *py_caps = NULL;
    PyObject *py_size = NULL;
    PyObject *py_othercaps = NULL;
    PyObject *py_args = NULL;
    PyObject *py_method = NULL;
    PyObject *py_ret = NULL;
    gboolean ret = FALSE;

    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        goto beach;
    }

    py_direction = pyg_enum_from_gtype(GST_TYPE_PAD_DIRECTION, direction);

    if (caps)
        py_caps = pyg_boxed_new(GST_TYPE_CAPS, caps, FALSE, FALSE); // should copyval be TRUE instead?
    else {
        Py_INCREF (Py_None);
        py_caps = Py_None;
    }
    
    py_size = PyInt_FromLong(size);

    if (othercaps)
        py_othercaps = pyg_boxed_new(GST_TYPE_CAPS, caps, FALSE, FALSE); // should copyval be TRUE instead?
    else {
        Py_INCREF (Py_None);
        py_othercaps = Py_None;
    }

    py_args = PyTuple_New(4);
    PyTuple_SET_ITEM(py_args, 0, py_direction);
    PyTuple_SET_ITEM(py_args, 1, py_caps);
    PyTuple_SET_ITEM(py_args, 2, py_size);
    PyTuple_SET_ITEM(py_args, 3, py_othercaps);

    py_method = PyObject_GetAttrString(py_self, "do_transform_size");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        goto beach;
    }
    
    py_ret = PyObject_CallObject(py_method, py_args);
    if (!py_ret) {
        if (PyErr_Occurred())
            PyErr_Print();
        goto beach;
    }

    if (PyInt_Check(py_ret)) {
        *othersize = PyInt_AsLong(py_ret);
        ret = TRUE;
    }

beach:
    Py_XDECREF(py_self);
    Py_XDECREF(py_args);
    Py_XDECREF(py_method);
    Py_XDECREF(py_ret);

    pyg_gil_state_release(__py_state);
    return ret;
}
#line 18408 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 461 "gstbase.override"
static gboolean
_wrap_GstBaseTransform__proxy_do_get_unit_size (GstBaseTransform * self,
						GstCaps * caps,
						guint * size)
{
  PyGILState_STATE __py_state;
  PyObject *py_self;
  PyObject *py_caps;
  gboolean ret = FALSE;
  PyObject *py_args;
  PyObject *py_method;
  PyObject *py_ret;

    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
	goto beach;
    }

    if (caps)
        py_caps = pyg_boxed_new(GST_TYPE_CAPS, caps, FALSE, FALSE); // should copyval be TRUE instead?
    else {
        Py_INCREF (Py_None);
        py_caps = Py_None;
    }

    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_caps);
    
    py_method = PyObject_GetAttrString(py_self, "do_get_unit_size");

    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
	goto beach;
    }
    
    py_ret = PyObject_CallObject(py_method, py_args);
    if (!py_ret) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
	goto beach;
    }

    /*
      If the method returned a numeric, the return value will be TRUE.
      For ANY other case, we don't set size and the return value is FALSE.
    */

    if (PyInt_Check(py_ret)) {
	*size = PyInt_AsLong(py_ret);
	ret = TRUE;
    }
    
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    Py_DECREF(py_ret);

 beach:
    pyg_gil_state_release(__py_state);
    return ret;
}
#line 18482 "..\\..\\Source\\gst-python\\gst\\gst.c"


static gboolean
_wrap_GstBaseTransform__proxy_do_set_caps(GstBaseTransform *self, GstCaps*incaps, GstCaps*outcaps)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_incaps = NULL;
    PyObject *py_outcaps = NULL;
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
    if (incaps)
        py_incaps = pyg_boxed_new (GST_TYPE_CAPS, incaps, FALSE, TRUE);
    else {
        Py_INCREF(Py_None);
        py_incaps = Py_None;
    }
    if (outcaps)
        py_outcaps = pyg_boxed_new (GST_TYPE_CAPS, outcaps, FALSE, TRUE);
    else {
        Py_INCREF(Py_None);
        py_outcaps = Py_None;
    }
    
    py_args = PyTuple_New(2);
    Py_INCREF(py_incaps);
    PyTuple_SET_ITEM(py_args, 0, py_incaps);
    Py_INCREF(py_outcaps);
    PyTuple_SET_ITEM(py_args, 1, py_outcaps);
    
    py_method = PyObject_GetAttrString(py_self, "do_set_caps");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        gst_caps_ref(outcaps);
        Py_DECREF(py_outcaps);
        gst_caps_ref(incaps);
        Py_DECREF(py_incaps);
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
        gst_caps_ref(outcaps);
        Py_DECREF(py_outcaps);
        gst_caps_ref(incaps);
        Py_DECREF(py_incaps);
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
        gst_caps_ref(outcaps);
        Py_DECREF(py_outcaps);
        gst_caps_ref(incaps);
        Py_DECREF(py_incaps);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_DECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    gst_caps_ref(outcaps);
    Py_DECREF(py_outcaps);
    gst_caps_ref(incaps);
    Py_DECREF(py_incaps);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gboolean
_wrap_GstBaseTransform__proxy_do_start(GstBaseTransform *self)
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
_wrap_GstBaseTransform__proxy_do_stop(GstBaseTransform *self)
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
static gboolean
_wrap_GstBaseTransform__proxy_do_event(GstBaseTransform *self, GstEvent*event)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_event = NULL;
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
    if (event) {
        py_event = pygstminiobject_new((GstMiniObject *) event);
        gst_mini_object_unref ((GstMiniObject *) event);
    } else {
        Py_INCREF(Py_None);
        py_event = Py_None;
    }
    
    py_args = PyTuple_New(1);
    Py_INCREF(py_event);
    PyTuple_SET_ITEM(py_args, 0, py_event);
    
    py_method = PyObject_GetAttrString(py_self, "do_event");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        gst_mini_object_ref ((GstMiniObject *) event); Py_DECREF(py_event);
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
        gst_mini_object_ref ((GstMiniObject *) event); Py_DECREF(py_event);
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
        gst_mini_object_ref ((GstMiniObject *) event); Py_DECREF(py_event);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_DECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    gst_mini_object_ref ((GstMiniObject *) event); Py_DECREF(py_event);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static GstFlowReturn
_wrap_GstBaseTransform__proxy_do_transform(GstBaseTransform *self, GstBuffer*inbuf, GstBuffer*outbuf)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_inbuf = NULL;
    PyObject *py_outbuf = NULL;
    GstFlowReturn retval;
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
    if (inbuf) {
        py_inbuf = pygstminiobject_new((GstMiniObject *) inbuf);
        gst_mini_object_unref ((GstMiniObject *) inbuf);
    } else {
        Py_INCREF(Py_None);
        py_inbuf = Py_None;
    }
    if (outbuf) {
        py_outbuf = pygstminiobject_new((GstMiniObject *) outbuf);
        gst_mini_object_unref ((GstMiniObject *) outbuf);
    } else {
        Py_INCREF(Py_None);
        py_outbuf = Py_None;
    }
    
    py_args = PyTuple_New(2);
    Py_INCREF(py_inbuf);
    PyTuple_SET_ITEM(py_args, 0, py_inbuf);
    Py_INCREF(py_outbuf);
    PyTuple_SET_ITEM(py_args, 1, py_outbuf);
    
    py_method = PyObject_GetAttrString(py_self, "do_transform");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        gst_mini_object_ref ((GstMiniObject *) outbuf); Py_DECREF(py_outbuf);
        gst_mini_object_ref ((GstMiniObject *) inbuf); Py_DECREF(py_inbuf);
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
        gst_mini_object_ref ((GstMiniObject *) outbuf); Py_DECREF(py_outbuf);
        gst_mini_object_ref ((GstMiniObject *) inbuf); Py_DECREF(py_inbuf);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    if (pyg_enum_get_value(GST_TYPE_FLOW_RETURN, py_retval, (gint *)&retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        gst_mini_object_ref ((GstMiniObject *) outbuf); Py_DECREF(py_outbuf);
        gst_mini_object_ref ((GstMiniObject *) inbuf); Py_DECREF(py_inbuf);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    
    
    Py_DECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    gst_mini_object_ref ((GstMiniObject *) outbuf); Py_DECREF(py_outbuf);
    gst_mini_object_ref ((GstMiniObject *) inbuf); Py_DECREF(py_inbuf);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static GstFlowReturn
_wrap_GstBaseTransform__proxy_do_transform_ip(GstBaseTransform *self, GstBuffer*buf)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_buf = NULL;
    GstFlowReturn retval;
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
    if (buf) {
        py_buf = pygstminiobject_new((GstMiniObject *) buf);
        gst_mini_object_unref ((GstMiniObject *) buf);
    } else {
        Py_INCREF(Py_None);
        py_buf = Py_None;
    }
    
    py_args = PyTuple_New(1);
    Py_INCREF(py_buf);
    PyTuple_SET_ITEM(py_args, 0, py_buf);
    
    py_method = PyObject_GetAttrString(py_self, "do_transform_ip");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        gst_mini_object_ref ((GstMiniObject *) buf); Py_DECREF(py_buf);
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
        gst_mini_object_ref ((GstMiniObject *) buf); Py_DECREF(py_buf);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    if (pyg_enum_get_value(GST_TYPE_FLOW_RETURN, py_retval, (gint *)&retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        gst_mini_object_ref ((GstMiniObject *) buf); Py_DECREF(py_buf);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    
    
    Py_DECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    gst_mini_object_ref ((GstMiniObject *) buf); Py_DECREF(py_buf);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gboolean
_wrap_GstBaseTransform__proxy_do_src_event(GstBaseTransform *self, GstEvent*event)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_event = NULL;
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
    if (event) {
        py_event = pygstminiobject_new((GstMiniObject *) event);
        gst_mini_object_unref ((GstMiniObject *) event);
    } else {
        Py_INCREF(Py_None);
        py_event = Py_None;
    }
    
    py_args = PyTuple_New(1);
    Py_INCREF(py_event);
    PyTuple_SET_ITEM(py_args, 0, py_event);
    
    py_method = PyObject_GetAttrString(py_self, "do_src_event");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        gst_mini_object_ref ((GstMiniObject *) event); Py_DECREF(py_event);
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
        gst_mini_object_ref ((GstMiniObject *) event); Py_DECREF(py_event);
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
        gst_mini_object_ref ((GstMiniObject *) event); Py_DECREF(py_event);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_DECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    gst_mini_object_ref ((GstMiniObject *) event); Py_DECREF(py_event);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}

static int
__GstBaseTransform_class_init(gpointer gclass, PyTypeObject *pyclass)
{
    PyObject *o;
    GstBaseTransformClass *klass = GST_BASE_TRANSFORM_CLASS(gclass);
    PyObject *gsignals = PyDict_GetItemString(pyclass->tp_dict, "__gsignals__");

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_transform_caps");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "transform_caps")))
            klass->transform_caps = _wrap_GstBaseTransform__proxy_do_transform_caps;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_fixate_caps");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "fixate_caps")))
            klass->fixate_caps = _wrap_GstBaseTransform__proxy_do_fixate_caps;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_transform_size");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "transform_size")))
            klass->transform_size = _wrap_GstBaseTransform__proxy_do_transform_size;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_get_unit_size");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "get_unit_size")))
            klass->get_unit_size = _wrap_GstBaseTransform__proxy_do_get_unit_size;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_transform_size");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "transform_size")))
            klass->transform_size = _wrap_GstBaseTransform__proxy_do_transform_size;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_set_caps");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "set_caps")))
            klass->set_caps = _wrap_GstBaseTransform__proxy_do_set_caps;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_start");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "start")))
            klass->start = _wrap_GstBaseTransform__proxy_do_start;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_stop");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "stop")))
            klass->stop = _wrap_GstBaseTransform__proxy_do_stop;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_event");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "event")))
            klass->event = _wrap_GstBaseTransform__proxy_do_event;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_transform");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "transform")))
            klass->transform = _wrap_GstBaseTransform__proxy_do_transform;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_transform_ip");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "transform_ip")))
            klass->transform_ip = _wrap_GstBaseTransform__proxy_do_transform_ip;
        Py_DECREF(o);
    }

    /* overriding do_prepare_output_buffer is currently not supported */

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_src_event");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "src_event")))
            klass->src_event = _wrap_GstBaseTransform__proxy_do_src_event;
        Py_DECREF(o);
    }
    return 0;
}


/* ----------- GstCollectPads ----------- */

static int
_wrap_gst_collect_pads_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char* kwlist[] = { NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     ":gst.CollectPads.__init__",
                                     kwlist))
        return -1;

    pygobject_constructv(self, 0, NULL);
    if (!self->obj) {
        PyErr_SetString(
            PyExc_RuntimeError, 
            "could not create gst.CollectPads object");
        return -1;
    }
    return 0;
}

static PyObject *
_wrap_gst_collect_pads_remove_pad(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "pad", NULL };
    PyGObject *pad;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstCollectPads.remove_pad", kwlist, &PyGstPad_Type, &pad))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_collect_pads_remove_pad(GST_COLLECT_PADS(self->obj), GST_PAD(pad->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_collect_pads_is_active(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "pad", NULL };
    PyGObject *pad;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstCollectPads.is_active", kwlist, &PyGstPad_Type, &pad))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_collect_pads_is_active(GST_COLLECT_PADS(self->obj), GST_PAD(pad->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_collect_pads_collect(PyGObject *self)
{
    gint ret;

    pyg_begin_allow_threads;
    ret = gst_collect_pads_collect(GST_COLLECT_PADS(self->obj));
    pyg_end_allow_threads;
    return pyg_enum_from_gtype(GST_TYPE_FLOW_RETURN, ret);
}

static PyObject *
_wrap_gst_collect_pads_collect_range(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "offset", "length", NULL };
    PyObject *py_length = NULL;
    gint ret;
    guint length = 0;
    guint64 offset;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"KO:GstCollectPads.collect_range", kwlist, &offset, &py_length))
        return NULL;
    if (py_length) {
        if (PyLong_Check(py_length))
            length = PyLong_AsUnsignedLong(py_length);
        else if (PyInt_Check(py_length))
            length = PyInt_AsLong(py_length);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'length' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    pyg_begin_allow_threads;
    ret = gst_collect_pads_collect_range(GST_COLLECT_PADS(self->obj), offset, length);
    pyg_end_allow_threads;
    return pyg_enum_from_gtype(GST_TYPE_FLOW_RETURN, ret);
}

static PyObject *
_wrap_gst_collect_pads_start(PyGObject *self)
{
    pyg_begin_allow_threads;
    gst_collect_pads_start(GST_COLLECT_PADS(self->obj));
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_collect_pads_stop(PyGObject *self)
{
    pyg_begin_allow_threads;
    gst_collect_pads_stop(GST_COLLECT_PADS(self->obj));
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_collect_pads_set_flushing(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "flushing", NULL };
    int flushing;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GstCollectPads.set_flushing", kwlist, &flushing))
        return NULL;
    pyg_begin_allow_threads;
    gst_collect_pads_set_flushing(GST_COLLECT_PADS(self->obj), flushing);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_collect_pads_available(PyGObject *self)
{
    guint ret;

    pyg_begin_allow_threads;
    ret = gst_collect_pads_available(GST_COLLECT_PADS(self->obj));
    pyg_end_allow_threads;
    return PyLong_FromUnsignedLong(ret);
}

static const PyMethodDef _PyGstCollectPads_methods[] = {
    { "remove_pad", (PyCFunction)_wrap_gst_collect_pads_remove_pad, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "is_active", (PyCFunction)_wrap_gst_collect_pads_is_active, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "collect", (PyCFunction)_wrap_gst_collect_pads_collect, METH_NOARGS,
      NULL },
    { "collect_range", (PyCFunction)_wrap_gst_collect_pads_collect_range, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "start", (PyCFunction)_wrap_gst_collect_pads_start, METH_NOARGS,
      NULL },
    { "stop", (PyCFunction)_wrap_gst_collect_pads_stop, METH_NOARGS,
      NULL },
    { "set_flushing", (PyCFunction)_wrap_gst_collect_pads_set_flushing, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "available", (PyCFunction)_wrap_gst_collect_pads_available, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject PyGstCollectPads_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.CollectPads",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGstCollectPads_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_gst_collect_pads_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GstDataQueue ----------- */

static PyObject *
_wrap_gst_data_queue_flush(PyGObject *self)
{
    pyg_begin_allow_threads;
    gst_data_queue_flush(GST_DATA_QUEUE(self->obj));
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_data_queue_set_flushing(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "flushing", NULL };
    int flushing;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GstDataQueue.set_flushing", kwlist, &flushing))
        return NULL;
    pyg_begin_allow_threads;
    gst_data_queue_set_flushing(GST_DATA_QUEUE(self->obj), flushing);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_data_queue_drop_head(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "type", NULL };
    PyObject *py_type = NULL;
    int ret;
    GType type;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:GstDataQueue.drop_head", kwlist, &py_type))
        return NULL;
    if ((type = pyg_type_from_object(py_type)) == 0)
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_data_queue_drop_head(GST_DATA_QUEUE(self->obj), type);
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_data_queue_is_full(PyGObject *self)
{
    int ret;

    pyg_begin_allow_threads;
    ret = gst_data_queue_is_full(GST_DATA_QUEUE(self->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_data_queue_is_empty(PyGObject *self)
{
    int ret;

    pyg_begin_allow_threads;
    ret = gst_data_queue_is_empty(GST_DATA_QUEUE(self->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_data_queue_limits_changed(PyGObject *self)
{
    pyg_begin_allow_threads;
    gst_data_queue_limits_changed(GST_DATA_QUEUE(self->obj));
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGstDataQueue_methods[] = {
    { "flush", (PyCFunction)_wrap_gst_data_queue_flush, METH_NOARGS,
      NULL },
    { "set_flushing", (PyCFunction)_wrap_gst_data_queue_set_flushing, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "drop_head", (PyCFunction)_wrap_gst_data_queue_drop_head, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "is_full", (PyCFunction)_wrap_gst_data_queue_is_full, METH_NOARGS,
      NULL },
    { "is_empty", (PyCFunction)_wrap_gst_data_queue_is_empty, METH_NOARGS,
      NULL },
    { "limits_changed", (PyCFunction)_wrap_gst_data_queue_limits_changed, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject PyGstDataQueue_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.DataQueue",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGstDataQueue_methods, /* tp_methods */
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



/* ----------- GstBuffer ----------- */

#line 42 "gstbuffer.override"
static int
_wrap_gst_buffer_new(PyGstMiniObject *self, PyObject *args, PyObject *kwargs)
{
	static char *kwlist[] = { "data", "buffer_size", NULL };
	char *data = NULL;
	int size = 0;
	int buf_size = -1;

	GST_INFO("self:%p", self);
	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|z#i:GstBuffer.__init__", kwlist,
					 &data, &size, &buf_size))
		return -1;
	
	if (size < 0) {
		PyErr_SetString(PyExc_TypeError, "buffer size must be >= 0");
		return -1;
	}
	if (buf_size < 0)
		buf_size = size;
	if (buf_size < size) {
		PyErr_SetString(PyExc_TypeError, "buffer size must be >= data size");
		return -1;
	}
	
	self->obj = GST_MINI_OBJECT(gst_buffer_new_and_alloc(buf_size));
	GST_INFO ("pyo:%p pyr:%"G_GSSIZE_FORMAT" minio:%p minir:%d",
		  self, ((PyObject*)self)->ob_refcnt,
		  self->obj, GST_MINI_OBJECT_REFCOUNT_VALUE(self->obj));
	if (!self->obj) {
		PyErr_SetString(PyExc_RuntimeError, "could not create GstBuffer object");
		return -1;
	}
	pygstminiobject_register_wrapper((PyObject *) self);

	if (data == NULL)
		return 0;

	memcpy (GST_BUFFER_DATA (self->obj), data, size);
	GST_BUFFER_SIZE (self->obj) = size;
	
	return 0;
}
#line 19538 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_buffer_is_metadata_writable(PyGstMiniObject *self)
{
    int ret;

    pyg_begin_allow_threads;
    ret = gst_buffer_is_metadata_writable(GST_BUFFER(self->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_buffer_make_metadata_writable(PyGstMiniObject *self)
{
    GstBuffer *ret;

    pyg_begin_allow_threads;
    ret = gst_buffer_make_metadata_writable(GST_BUFFER(self->obj));
    pyg_end_allow_threads;
    /* pygobject_new handles NULL checking */
    return pygstminiobject_new((GstMiniObject *)ret);
}

static PyObject *
_wrap_gst_buffer_get_caps(PyGstMiniObject *self)
{
    GstCaps *ret;

    pyg_begin_allow_threads;
    ret = gst_buffer_get_caps(GST_BUFFER(self->obj));
    pyg_end_allow_threads;
    return pyg_boxed_new (GST_TYPE_CAPS, ret, FALSE, TRUE);
}

#line 555 "gstbuffer.override"
static PyObject *
_wrap_gst_buffer_set_caps(PyGstMiniObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "caps", NULL };
    PyObject *py_caps;
    GstCaps *caps;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:GstBuffer.set_caps", kwlist, &py_caps))
        return NULL;
    caps = pygst_caps_from_pyobject (py_caps, NULL);
    if (PyErr_Occurred())
      return NULL;
    pyg_begin_allow_threads;
    gst_buffer_set_caps(GST_BUFFER(self->obj), caps);
    gst_caps_unref (caps);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}
#line 19596 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_buffer_create_sub(PyGstMiniObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "offset", "size", NULL };
    PyObject *py_offset = NULL, *py_size = NULL, *py_ret;
    GstBuffer *ret;
    guint offset = 0, size = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"OO:GstBuffer.create_sub", kwlist, &py_offset, &py_size))
        return NULL;
    if (py_offset) {
        if (PyLong_Check(py_offset))
            offset = PyLong_AsUnsignedLong(py_offset);
        else if (PyInt_Check(py_offset))
            offset = PyInt_AsLong(py_offset);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'offset' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    if (py_size) {
        if (PyLong_Check(py_size))
            size = PyLong_AsUnsignedLong(py_size);
        else if (PyInt_Check(py_size))
            size = PyInt_AsLong(py_size);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'size' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    pyg_begin_allow_threads;
    ret = gst_buffer_create_sub(GST_BUFFER(self->obj), offset, size);
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_buffer_is_span_fast(PyGstMiniObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "buf2", NULL };
    int ret;
    PyGstMiniObject *buf2;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstBuffer.is_span_fast", kwlist, &PyGstBuffer_Type, &buf2))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_buffer_is_span_fast(GST_BUFFER(self->obj), GST_BUFFER(buf2->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_buffer_span(PyGstMiniObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "offset", "buf2", "len", NULL };
    PyObject *py_ret;
    unsigned long offset, len;
    GstBuffer *ret;
    PyGstMiniObject *buf2;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"kO!k:GstBuffer.span", kwlist, &offset, &PyGstBuffer_Type, &buf2, &len))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_buffer_span(GST_BUFFER(self->obj), offset, GST_BUFFER(buf2->obj), len);
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_buffer_merge(PyGstMiniObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "buf2", NULL };
    PyObject *py_ret;
    GstBuffer *ret;
    PyGstMiniObject *buf2;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstBuffer.merge", kwlist, &PyGstBuffer_Type, &buf2))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_buffer_merge(GST_BUFFER(self->obj), GST_BUFFER(buf2->obj));
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_buffer_join(PyGstMiniObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "buf2", NULL };
    PyObject *py_ret;
    GstBuffer *ret;
    PyGstMiniObject *buf2;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstBuffer.join", kwlist, &PyGstBuffer_Type, &buf2))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_buffer_join(GST_BUFFER(self->obj), GST_BUFFER(buf2->obj));
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}

#line 504 "gstbuffer.override"
static PyObject *
_wrap_gst_buffer_stamp (PyGstMiniObject *self, PyObject *args, PyObject *kwargs)
{
	static char	*kwlist[] = { "src", NULL };
	PyGstMiniObject	*srcobj;
	GstBuffer	*dest, *src;

	if (!PyArg_ParseTupleAndKeywords (args, kwargs,
					  "O:GstBuffer.stamp",
					  kwlist, &srcobj))
		return NULL;
	dest = GST_BUFFER(pygstminiobject_get(self));
	src = GST_BUFFER(pygstminiobject_get(srcobj));
	gst_buffer_stamp (dest, (const GstBuffer*) src);

	Py_INCREF(Py_None);
	return Py_None;
}
#line 19731 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 299 "gstbuffer.override"
static PyObject *
_wrap_gst_buffer_copy_on_write (PyObject *self)
{
	GstBuffer *buf = GST_BUFFER(pygstminiobject_get(self));

	GST_INFO("INCREF");
	if (gst_buffer_is_writable (buf)) {
	  Py_INCREF (self);
	  return self;
	}
	buf = gst_buffer_copy (buf);
	self = pygstminiobject_new ((GstMiniObject *)(buf));
        gst_buffer_unref (buf);
	return self;
}
#line 19750 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 353 "gstbuffer.override"
static PyObject *
_wrap_gst_buffer_flag_unset(PyObject *self, PyObject *args)
{
	int flag;
	GstBuffer *buf;
	
	if (!PyArg_ParseTuple(args, "i:GstBuffer.unset", &flag))
		return NULL;

        buf = GST_BUFFER(pygstminiobject_get(self));
	g_assert(GST_IS_BUFFER(buf));
	GST_BUFFER_FLAG_UNSET(buf, flag);

	Py_INCREF(Py_None);
	return Py_None;
}

#line 19771 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 335 "gstbuffer.override"
static PyObject *
_wrap_gst_buffer_flag_set(PyObject *self, PyObject *args)
{
	int flag;
	GstBuffer *buf;
	
	if (!PyArg_ParseTuple(args, "i:GstBuffer.set", &flag))
		return NULL;

	buf = GST_BUFFER(pygstminiobject_get(self));
	g_assert(GST_IS_BUFFER(buf));
	GST_BUFFER_FLAG_SET(buf, flag);

	Py_INCREF(Py_None);
	return Py_None;
}
#line 19791 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 316 "gstbuffer.override"
static PyObject *
_wrap_gst_buffer_flag_is_set(PyObject *self, PyObject *args)
{
	int flag;
	PyObject *retval;
	GstBuffer *buf;
	
	if (!PyArg_ParseTuple(args, "i:GstBuffer.flag_is_set", &flag))
		return NULL;

	buf = GST_BUFFER(pygstminiobject_get(self));
	g_assert(GST_IS_BUFFER(buf));
	
	retval = GST_BUFFER_FLAG_IS_SET(buf, flag) ? Py_True : Py_False;
	Py_INCREF(retval);
	return retval;
}
#line 19812 "..\\..\\Source\\gst-python\\gst\\gst.c"


static const PyMethodDef _PyGstBuffer_methods[] = {
    { "is_metadata_writable", (PyCFunction)_wrap_gst_buffer_is_metadata_writable, METH_NOARGS,
      NULL },
    { "make_metadata_writable", (PyCFunction)_wrap_gst_buffer_make_metadata_writable, METH_NOARGS,
      NULL },
    { "get_caps", (PyCFunction)_wrap_gst_buffer_get_caps, METH_NOARGS,
      NULL },
    { "set_caps", (PyCFunction)_wrap_gst_buffer_set_caps, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "create_sub", (PyCFunction)_wrap_gst_buffer_create_sub, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "is_span_fast", (PyCFunction)_wrap_gst_buffer_is_span_fast, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "span", (PyCFunction)_wrap_gst_buffer_span, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "merge", (PyCFunction)_wrap_gst_buffer_merge, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "join", (PyCFunction)_wrap_gst_buffer_join, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "stamp", (PyCFunction)_wrap_gst_buffer_stamp, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "copy_on_write", (PyCFunction)_wrap_gst_buffer_copy_on_write, METH_VARARGS,
      NULL },
    { "flag_unset", (PyCFunction)_wrap_gst_buffer_flag_unset, METH_VARARGS,
      NULL },
    { "flag_set", (PyCFunction)_wrap_gst_buffer_flag_set, METH_VARARGS,
      NULL },
    { "flag_is_set", (PyCFunction)_wrap_gst_buffer_flag_is_set, METH_VARARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

#line 372 "gstbuffer.override"
static PyObject *
_wrap_gst_buffer__get_data(PyObject *self, void *closure)
{
	GstBuffer *buf;

	g_assert (self);
        buf = GST_BUFFER(pygstminiobject_get(self));
	g_assert (buf);

	return PyString_FromStringAndSize((const gchar*) GST_BUFFER_DATA(buf),
					  (gint) GST_BUFFER_SIZE(buf));
}

#line 19861 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_buffer__get_size(PyObject *self, void *closure)
{
    guint ret;

    ret = GST_BUFFER(pygstminiobject_get(self))->size;
    return PyLong_FromUnsignedLong(ret);
}

#line 387 "gstbuffer.override"
static PyObject *
_wrap_gst_buffer__get_timestamp(PyObject *self, void *closure)
{
    guint64 ret;

    ret = GST_BUFFER(pygstminiobject_get(self))->timestamp;
    return PyLong_FromUnsignedLongLong(ret);
}
static int
_wrap_gst_buffer__set_timestamp(PyGstMiniObject *self, PyObject *value, void *closure)
{
    guint64 val;

    if (PyInt_CheckExact(value))
      val = PyInt_AsUnsignedLongLongMask(value);
    else
      val = PyLong_AsUnsignedLongLong(value);
    if (PyErr_Occurred())
        return -1;
    
    GST_BUFFER(self->obj)->timestamp = val;
    return 0;
}
#line 19897 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 412 "gstbuffer.override"
static PyObject *
_wrap_gst_buffer__get_duration(PyObject *self, void *closure)
{
    guint64 ret;

    ret = GST_BUFFER(pygstminiobject_get(self))->duration;
    return PyLong_FromUnsignedLongLong(ret);
}
static int
_wrap_gst_buffer__set_duration(PyGstMiniObject *self, PyObject *value, void *closure)
{
    guint64 val;

    if (PyInt_CheckExact(value))
      val = PyInt_AsUnsignedLongLongMask(value);
    else
      val = PyLong_AsUnsignedLongLong(value);
    if (PyErr_Occurred())
        return -1;
    
    GST_BUFFER(self->obj)->duration = val;
    return 0;
}

#line 19925 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 438 "gstbuffer.override"
static PyObject *
_wrap_gst_buffer__get_offset (PyObject *self, void *closure)
{
    guint64 ret;
    GstMiniObject *miniobject;
    g_assert (self);

    miniobject = pygstminiobject_get (self);
    g_assert (miniobject);

    ret = GST_BUFFER_OFFSET (GST_BUFFER (miniobject));
    return PyLong_FromUnsignedLongLong (ret);
}

static int
_wrap_gst_buffer__set_offset (PyGstMiniObject *self, PyObject *value, void *closure)
{
    guint64 val;
    g_assert (self);

    if (PyInt_CheckExact (value))
      val = PyInt_AsUnsignedLongLongMask (value);
    else
      val = PyLong_AsUnsignedLongLong (value);
    if (PyErr_Occurred())
        return -1;
    
    GST_BUFFER_OFFSET (GST_BUFFER (self->obj)) = val;
    return 0;
}

#line 19960 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 471 "gstbuffer.override"
static PyObject *
_wrap_gst_buffer__get_offset_end (PyObject *self, void *closure)
{
    guint64 ret;
    GstMiniObject *miniobject;
    g_assert (self);

    miniobject = pygstminiobject_get (self);
    g_assert (miniobject);
    
    ret = GST_BUFFER_OFFSET_END (GST_BUFFER (miniobject));
    return PyLong_FromUnsignedLongLong (ret);
}

static int
_wrap_gst_buffer__set_offset_end (PyGstMiniObject *self, PyObject *value, void *closure)
{
    guint64 val;
    g_assert (self);

    if (PyInt_CheckExact (value))
      val = PyInt_AsUnsignedLongLongMask (value);
    else
      val = PyLong_AsUnsignedLongLong (value);
    if (PyErr_Occurred ())
        return -1;
    
    GST_BUFFER_OFFSET_END (GST_BUFFER (self->obj)) = val;
    return 0;
}

#line 19995 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 524 "gstbuffer.override"
static PyObject *
_wrap_gst_buffer__get_caps (PyObject *self, void *closure)
{
    GstMiniObject *miniobject;
    GstCaps *ret;

    miniobject = pygstminiobject_get (self);
    g_assert (miniobject);

    pyg_begin_allow_threads;
    ret = gst_buffer_get_caps(GST_BUFFER(miniobject));
    pyg_end_allow_threads;
    return pyg_boxed_new (GST_TYPE_CAPS, ret, FALSE, TRUE);    
}
static int
_wrap_gst_buffer__set_caps (PyGstMiniObject *self, PyObject *value, void *closure)
{
    GstCaps *caps;
    g_assert (self);

    caps = pygst_caps_from_pyobject (value, NULL);
    if (PyErr_Occurred())
	return -1;
    pyg_begin_allow_threads;
    gst_buffer_set_caps(GST_BUFFER(self->obj), caps);
    gst_caps_unref (caps);
    pyg_end_allow_threads;
    return 0;
}
#line 20028 "..\\..\\Source\\gst-python\\gst\\gst.c"


static const PyGetSetDef gst_buffer_getsets[] = {
    { "data", (getter)_wrap_gst_buffer__get_data, (setter)0 },
    { "size", (getter)_wrap_gst_buffer__get_size, (setter)0 },
    { "timestamp", (getter)_wrap_gst_buffer__get_timestamp, (setter)_wrap_gst_buffer__set_timestamp },
    { "duration", (getter)_wrap_gst_buffer__get_duration, (setter)_wrap_gst_buffer__set_duration },
    { "offset", (getter)_wrap_gst_buffer__get_offset, (setter)_wrap_gst_buffer__set_offset },
    { "offset_end", (getter)_wrap_gst_buffer__get_offset_end, (setter)_wrap_gst_buffer__set_offset_end },
    { "caps", (getter)_wrap_gst_buffer__get_caps, (setter)_wrap_gst_buffer__set_caps },
    { NULL, (getter)0, (setter)0 },
};

#line 101 "gstbuffer.override"
static PyObject *
_wrap_gst_buffer_tp_repr (PyGstMiniObject *self)
{
    GstBuffer *buf;
    guchar *data;
    gchar *repr;
    gint size = 0;
    PyObject *ret;

    g_assert (self);
    buf = GST_BUFFER(self->obj);
    g_assert (buf);

    size = GST_BUFFER_SIZE (buf);
    
    if (size == 0) {
        repr = g_strdup_printf ("<gst.Buffer %p of size %d>", buf, size);
    } else {
        data = GST_BUFFER_DATA (buf);
        repr = g_strdup_printf (
            "<gst.Buffer %p of size %d and data 0x%02x%02x%02x%02x>", buf, size,
            *data,
            size > 0 ? *(data + 1) : 0,
            size > 1 ? *(data + 2) : 0,
            size > 2 ? *(data + 3) : 0
            );
    }
    ret = PyString_FromStringAndSize(repr, strlen (repr));
    g_free (repr);
    return ret;
}

#line 20075 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 203 "gstbuffer.override"
/* FIXME: should buffer parts be buffers or strings? */

static Py_ssize_t
pygst_buffer_length(PyObject *self)
{
	return GST_BUFFER_SIZE(pygobject_get (self));
}

static PyObject *
pygst_buffer_slice(PyObject *self, Py_ssize_t start, Py_ssize_t end)
{
  GstBuffer *buf = GST_BUFFER (pygobject_get (self));
  if (start < 0)
    start = 0;
  if (end < 0)
    end = 0;
  if (end > GST_BUFFER_SIZE(buf))
    end = GST_BUFFER_SIZE(buf);
    
  if (end <= start) {
    PyErr_SetString(PyExc_IndexError, "buffer index out of range");
    return NULL;
  }
  return PyString_FromStringAndSize((gchar *) GST_BUFFER_DATA (buf) + start, end - start);
}

static PyObject *
pygst_buffer_item(PyObject *self, Py_ssize_t index)
{
  return pygst_buffer_slice (self, index, index + 1);
}

static int
pygst_buffer_ass_slice (PyObject *self, Py_ssize_t start, 
        Py_ssize_t end, PyObject *val)
{
  GstBuffer *buf = GST_BUFFER (pygobject_get (self));
  const void *data;
  Py_ssize_t len;

  if (!gst_buffer_is_writable (buf)) {
    PyErr_SetString(PyExc_TypeError, "buffer is not writable");
    return -1;
  }
  /* FIXME: policy? */
  if (start < 0 || end <= start || end > GST_BUFFER_SIZE (buf)) {
    PyErr_SetString(PyExc_IndexError, "buffer index out of range");
    return -1;
  }
  end -= start;
  if (PyObject_AsReadBuffer(val, &data, &len))
    return -1;
  if (len > end)
    len = end;
  memcpy (GST_BUFFER_DATA (buf) + start, data, len);
  return 0;
}

static int
pygst_buffer_ass_item (PyObject *self, Py_ssize_t index, PyObject *val)
{
  GstBuffer *buf = GST_BUFFER (pygobject_get (self));
  const void *data;
  Py_ssize_t len;

  if (!gst_buffer_is_writable (buf)) {
    PyErr_SetString(PyExc_TypeError, "buffer is not writable");
    return -1;
  }
  if (index < 0 || index > GST_BUFFER_SIZE (buf)) {
    PyErr_SetString(PyExc_IndexError, "buffer index out of range");
    return -1;
  }
  if (PyObject_AsReadBuffer(val, &data, &len))
    return -1;
  /* FIXME: how do we handle this? */
  if (len > GST_BUFFER_SIZE (buf) - index)
    len = GST_BUFFER_SIZE (buf) - index;
  memcpy (GST_BUFFER_DATA (buf) + index, data, len);
  return 0;
}

static PySequenceMethods _wrap_gst_buffer_tp_as_sequence = {
	pygst_buffer_length,                /* sq_length */
	NULL,                               /* sq_concat */
	NULL,                               /* sq_repeat */
	pygst_buffer_item,		    /* sq_item */
	pygst_buffer_slice,		    /* sq_slice */
	pygst_buffer_ass_item,              /* sq_ass_item */
	pygst_buffer_ass_slice,             /* sq_ass_slice */
	NULL,                               /* sq_contains */
	NULL,                               /* sq_inplace_concat */
	NULL,                               /* sq_inplace_repeat */
};
#line 20173 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 86 "gstbuffer.override"
static PyObject *
_wrap_gst_buffer_tp_str (PyGstMiniObject *self)
{
	GstBuffer *buf;

	g_assert (self);
        buf = GST_BUFFER(pygstminiobject_get(self));
	g_assert (buf);

	return PyString_FromStringAndSize((const gchar*) GST_BUFFER_DATA(buf),
					  (gint) GST_BUFFER_SIZE(buf));
}

#line 20190 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 135 "gstbuffer.override"
static PyBufferProcs _wrap_gst_buffer_tp_as_buffer = {
        gst_buffer_getreadbuffer, /* bf_getreadbuffer */
	gst_buffer_getwritebuf,  /* bf_getwritebuffer */
	gst_buffer_getsegcount,     /* bf_getsegcount */
	gst_buffer_getcharbuf,    /* bf_getcharbuffer */
};

static Py_ssize_t
gst_buffer_getreadbuffer(PyObject *self, Py_ssize_t index, 
    void **ptr)
{
	GstBuffer *buf = GST_BUFFER(pygstminiobject_get(self));

	if ( index != 0 ) {
		PyErr_SetString(PyExc_SystemError,
				"accessing non-existent GstBuffer segment");
		return -1;
	}
	
	*ptr = GST_BUFFER_DATA(buf);
	return GST_BUFFER_SIZE(buf);
}

static Py_ssize_t
gst_buffer_getsegcount(PyObject *self, Py_ssize_t *lenp)
{
	GstBuffer *buf = GST_BUFFER(pygstminiobject_get(self));

	if (lenp)
		*lenp = GST_BUFFER_SIZE(buf);
	return 1;
}

/* Need a version that has const char ** for Python 2.4 */
#if PY_VERSION_HEX >= 0x02050000
static Py_ssize_t gst_buffer_getcharbuf    (PyObject   *self,
			   Py_ssize_t index, char **ptr)
#else
static Py_ssize_t gst_buffer_getcharbuf    (PyObject   *self,
			   Py_ssize_t index, const char **ptr)
#endif
{
	return gst_buffer_getreadbuffer (self, index, (void **) ptr);
}

static Py_ssize_t
gst_buffer_getwritebuf(PyObject *self, Py_ssize_t index, void **ptr)
{
	GstBuffer *buf = GST_BUFFER(pygstminiobject_get(self));

	if ( index != 0 ) {
		PyErr_SetString(PyExc_SystemError,
				"accessing non-existent GstBuffer segment");
		return -1;
	}
	
	if (!gst_buffer_is_writable (buf)) {
		PyErr_SetString(PyExc_TypeError,
				"buffer is not writable");
		return -1;
	}

	*ptr = GST_BUFFER_DATA(buf);
	return GST_BUFFER_SIZE(buf);
}

#line 20260 "..\\..\\Source\\gst-python\\gst\\gst.c"


PyTypeObject PyGstBuffer_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.Buffer",                   /* tp_name */
    sizeof(PyGstMiniObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)_wrap_gst_buffer_tp_repr,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)&_wrap_gst_buffer_tp_as_sequence, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)_wrap_gst_buffer_tp_str,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)&_wrap_gst_buffer_tp_as_buffer,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGstMiniObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGstBuffer_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)gst_buffer_getsets,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGstMiniObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_gst_buffer_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GstEvent ----------- */

#line 24 "gstevent.override"
static PyObject *
_wrap_gst_event_get_structure(PyGstMiniObject *self)
{
     GstStructure *ret;

     ret = (GstStructure *) gst_event_get_structure(GST_EVENT(self->obj));
     /* pyg_boxed_new handles NULL checking */
     return pyg_boxed_new(GST_TYPE_STRUCTURE, ret, TRUE, TRUE);
}

#line 20323 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_event_has_name(PyGstMiniObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", NULL };
    char *name;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:GstEvent.has_name", kwlist, &name))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_event_has_name(GST_EVENT(self->obj), name);
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_event_get_seqnum(PyGstMiniObject *self)
{
    guint32 ret;

    pyg_begin_allow_threads;
    ret = gst_event_get_seqnum(GST_EVENT(self->obj));
    pyg_end_allow_threads;
    return PyLong_FromUnsignedLong(ret);

}

static PyObject *
_wrap_gst_event_set_seqnum(PyGstMiniObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "seqnum", NULL };
    unsigned long seqnum;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"k:GstEvent.set_seqnum", kwlist, &seqnum))
        return NULL;
    pyg_begin_allow_threads;
    gst_event_set_seqnum(GST_EVENT(self->obj), seqnum);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

#line 54 "gstevent.override"
static PyObject *
_wrap_gst_event_parse_new_segment (PyGstMiniObject *self)
{
     gboolean   update;
     gdouble	rate;
     GstFormat	format;
     gint64	start_value, stop_value, base;

     if (GST_EVENT_TYPE(self->obj) != GST_EVENT_NEWSEGMENT) {
	  PyErr_SetString(PyExc_TypeError, "Even is not a 'NewSegment' event");
	  return NULL;
     }
     
     gst_event_parse_new_segment (GST_EVENT(self->obj), &update, &rate, &format,
				 &start_value, &stop_value, &base);

     return Py_BuildValue("(OdOLLL)",
			  PyBool_FromLong(update),
			  rate, pyg_enum_from_gtype (GST_TYPE_FORMAT, format),
			  start_value, stop_value, base);
}
#line 20391 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 77 "gstevent.override"
static PyObject *
_wrap_gst_event_parse_tag (PyGstMiniObject *self)
{
	PyObject	*ret;
	GstTagList	*taglist;
	
	if (GST_EVENT_TYPE(self->obj) != GST_EVENT_TAG) {
		PyErr_SetString(PyExc_TypeError, "Event is not an 'Tag' event");
		return NULL;
	}

	gst_event_parse_tag (GST_EVENT(self->obj), &taglist);

	ret = pyg_boxed_new (GST_TYPE_TAG_LIST, taglist, TRUE, TRUE);

	return ret;
}
#line 20412 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 145 "gstevent.override"
static PyObject *
_wrap_gst_event_parse_buffer_size (PyGstMiniObject *self)
{
     GstFormat		format;
     gint64		minsize;
     gint64		maxsize;
     gboolean		async;

     if (GST_EVENT_TYPE (self->obj) != GST_EVENT_BUFFERSIZE) {
	  PyErr_SetString(PyExc_TypeError, "Event is not an 'BufferSize' event");
	  return NULL;
     }
     
     gst_event_parse_buffer_size (GST_EVENT (self->obj),
				  &format, &minsize,
				  &maxsize, &async);

     return Py_BuildValue("(OLLO)",
			  pyg_enum_from_gtype(GST_TYPE_FORMAT, format),
			  minsize, maxsize,
			  PyBool_FromLong(async));
}
#line 20438 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 96 "gstevent.override"
static PyObject *
_wrap_gst_event_parse_qos (PyGstMiniObject *self)
{
	gdouble		proportion;
	GstClockTimeDiff	diff;
	GstClockTime	timestamp;
	
	if (GST_EVENT_TYPE(self->obj) != GST_EVENT_QOS) {
		PyErr_SetString(PyExc_TypeError, "Event is not an 'Qos' event");
		return NULL;
	}

	gst_event_parse_qos (GST_EVENT(self->obj), &proportion,
			     &diff, &timestamp);
	
       return Py_BuildValue("(dLK)", proportion, diff, timestamp);
}
#line 20459 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 115 "gstevent.override"
static PyObject *
_wrap_gst_event_parse_seek (PyGstMiniObject *self)
{
	gdouble	        rate;
	GstFormat	format;
	GstSeekFlags	flags;
	GstSeekType	cur_type;
	gint64		cur;
	GstSeekType	stop_type;
	gint64		stop;
	
	if (GST_EVENT_TYPE(self->obj) != GST_EVENT_SEEK) {
		PyErr_SetString(PyExc_TypeError, "Event is not an 'Seek' event");
		return NULL;
	}

	gst_event_parse_seek (GST_EVENT(self->obj), &rate, &format, &flags,
			      &cur_type, &cur, &stop_type, &stop);
	
	return Py_BuildValue("(dOOOLOL)",
			     rate,
			     pyg_enum_from_gtype (GST_TYPE_FORMAT, format),
			     pyg_flags_from_gtype (GST_TYPE_SEEK_FLAGS, flags),
			     pyg_enum_from_gtype (GST_TYPE_SEEK_TYPE, cur_type),
			     cur,
			     pyg_enum_from_gtype (GST_TYPE_SEEK_TYPE, stop_type),
			     stop);
}
#line 20491 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 169 "gstevent.override"
static PyObject *
_wrap_gst_event_parse_latency (PyGstMiniObject * self)
{
     GstClockTime	ctime = GST_CLOCK_TIME_NONE;

     if (GST_EVENT_TYPE (self->obj) != GST_EVENT_LATENCY) {
	  PyErr_SetString(PyExc_TypeError, "Event is not a 'latency' event");
	  return NULL;
     }
     
     gst_event_parse_latency (GST_EVENT (self->obj), &ctime);
     
     return PyLong_FromUnsignedLongLong(ctime);
}
#line 20509 "..\\..\\Source\\gst-python\\gst\\gst.c"


static const PyMethodDef _PyGstEvent_methods[] = {
    { "get_structure", (PyCFunction)_wrap_gst_event_get_structure, METH_NOARGS,
      NULL },
    { "has_name", (PyCFunction)_wrap_gst_event_has_name, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_seqnum", (PyCFunction)_wrap_gst_event_get_seqnum, METH_NOARGS,
      NULL },
    { "set_seqnum", (PyCFunction)_wrap_gst_event_set_seqnum, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "parse_new_segment", (PyCFunction)_wrap_gst_event_parse_new_segment, METH_NOARGS,
      NULL },
    { "parse_tag", (PyCFunction)_wrap_gst_event_parse_tag, METH_NOARGS,
      NULL },
    { "parse_buffer_size", (PyCFunction)_wrap_gst_event_parse_buffer_size, METH_NOARGS,
      NULL },
    { "parse_qos", (PyCFunction)_wrap_gst_event_parse_qos, METH_NOARGS,
      NULL },
    { "parse_seek", (PyCFunction)_wrap_gst_event_parse_seek, METH_NOARGS,
      NULL },
    { "parse_latency", (PyCFunction)_wrap_gst_event_parse_latency, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

static PyObject *
_wrap_gst_event__get_type(PyObject *self, void *closure)
{
    gint ret;

    ret = GST_EVENT(pygstminiobject_get(self))->type;
    return pyg_enum_from_gtype(GST_TYPE_EVENT_TYPE, ret);
}

static PyObject *
_wrap_gst_event__get_timestamp(PyObject *self, void *closure)
{
    guint64 ret;

    ret = GST_EVENT(pygstminiobject_get(self))->timestamp;
    return PyLong_FromUnsignedLongLong(ret);
}

static PyObject *
_wrap_gst_event__get_src(PyObject *self, void *closure)
{
    GstObject *ret;

    ret = GST_EVENT(pygstminiobject_get(self))->src;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static const PyGetSetDef gst_event_getsets[] = {
    { "type", (getter)_wrap_gst_event__get_type, (setter)0 },
    { "timestamp", (getter)_wrap_gst_event__get_timestamp, (setter)0 },
    { "src", (getter)_wrap_gst_event__get_src, (setter)0 },
    { NULL, (getter)0, (setter)0 },
};

#line 36 "gstevent.override"
static PyObject *
_wrap_gst_event_tp_repr (PyGObject *self)
{
     char *buf;
     PyObject *retval;
     GstEvent *event;

     event = GST_EVENT(self->obj);

     buf = g_strdup_printf ("<GstEvent (%s) at %lx>",
			    gst_event_type_get_name (event->type), (long) self->obj);

     retval = PyString_FromString(buf);
     g_free(buf);
     return retval;
}
#line 20588 "..\\..\\Source\\gst-python\\gst\\gst.c"


PyTypeObject PyGstEvent_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.Event",                   /* tp_name */
    sizeof(PyGstMiniObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)_wrap_gst_event_tp_repr,             /* tp_repr */
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
    offsetof(PyGstMiniObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGstEvent_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)gst_event_getsets,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGstMiniObject, inst_dict),                 /* tp_dictoffset */
    (initproc)pygobject_no_constructor,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GstMessage ----------- */

static PyObject *
_wrap_gst_message_set_seqnum(PyGstMiniObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "seqnum", NULL };
    unsigned long seqnum;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"k:GstMessage.set_seqnum", kwlist, &seqnum))
        return NULL;
    pyg_begin_allow_threads;
    gst_message_set_seqnum(GST_MESSAGE(self->obj), seqnum);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

#line 120 "gstmessage.override"
static PyObject *
_wrap_gst_message_parse_error (PyGstMiniObject *self)
{
	PyObject	*ret;
	GError	*error = NULL;
	gchar	*debug;
	
	if (GST_MESSAGE_TYPE(self->obj) != GST_MESSAGE_ERROR) {
		PyErr_SetString(PyExc_TypeError, "Message is not an error message");
		return NULL;
	}

	gst_message_parse_error (GST_MESSAGE(self->obj), &error, &debug);

	ret = PyList_New(2);
	PyList_SetItem(ret, 0, pyg_boxed_new (GST_TYPE_G_ERROR, error, TRUE, TRUE));
	if (error)
		g_error_free (error);
	if (debug != NULL) {
		PyList_SetItem(ret, 1, PyString_FromString(debug));
	} else {
		Py_INCREF (Py_None);
		PyList_SetItem(ret, 1, Py_None);
	}
	g_free(debug);
	return ret;
}
#line 20683 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 149 "gstmessage.override"
static PyObject *
_wrap_gst_message_parse_warning (PyGstMiniObject *self)
{
	PyObject	*ret;
	GError	*warning = NULL;
	gchar	*debug;
	
	if (GST_MESSAGE_TYPE(self->obj) != GST_MESSAGE_WARNING) {
		PyErr_SetString(PyExc_TypeError, "Message is not an warning message");
		return NULL;
	}

	gst_message_parse_warning (GST_MESSAGE(self->obj), &warning, &debug);

	ret = PyList_New(2);
	PyList_SetItem(ret, 0, pyg_boxed_new (GST_TYPE_G_ERROR, warning, TRUE, TRUE));
	if (warning)
		g_error_free (warning);
	if (debug != NULL) {
		PyList_SetItem(ret, 1, PyString_FromString(debug));
	} else {
		Py_INCREF (Py_None);
		PyList_SetItem(ret, 1, Py_None);
	}
	g_free(debug);
	return ret;
}
#line 20714 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 178 "gstmessage.override"
static PyObject *
_wrap_gst_message_parse_info (PyGstMiniObject *self)
{
	PyObject	*ret;
	GError	*info = NULL;
	gchar	*debug;
	
	if (GST_MESSAGE_TYPE(self->obj) != GST_MESSAGE_INFO) {
		PyErr_SetString(PyExc_TypeError, "Message is not an info message");
		return NULL;
	}

	gst_message_parse_info (GST_MESSAGE(self->obj), &info, &debug);

	ret = PyList_New(2);
	PyList_SetItem(ret, 0, pyg_boxed_new (GST_TYPE_G_ERROR, info, TRUE, TRUE));
	if (info)
		g_error_free (info);
	if (debug != NULL) {
		PyList_SetItem(ret, 1, PyString_FromString(debug));
	} else {
		Py_INCREF (Py_None);
		PyList_SetItem(ret, 1, Py_None);
	}
	g_free(debug);
	return ret;
}
#line 20745 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 207 "gstmessage.override"
static PyObject *
_wrap_gst_message_parse_tag (PyGstMiniObject *self)
{
	PyObject	*ret;
	GstTagList	*taglist;
	
	if (GST_MESSAGE_TYPE(self->obj) != GST_MESSAGE_TAG) {
		PyErr_SetString(PyExc_TypeError, "Message is not an Tag message");
		return NULL;
	}

	gst_message_parse_tag (GST_MESSAGE(self->obj), &taglist);

	ret = pyg_boxed_new (GST_TYPE_TAG_LIST, taglist, TRUE, TRUE);
	gst_tag_list_free (taglist);
	return ret;
}
#line 20766 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 312 "gstmessage.override"
static PyObject *
_wrap_gst_message_parse_buffering (PyGstMiniObject *self)
{
	gint percent;

	if (GST_MESSAGE(self->obj)->type != GST_MESSAGE_BUFFERING) {
		PyErr_SetString(PyExc_TypeError, "Message is not a 'buffering' message");
		return NULL;
	}

	gst_message_parse_buffering (GST_MESSAGE(self->obj), &percent);

	return Py_BuildValue("i", percent);
}
#line 20784 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_message_set_buffering_stats(PyGstMiniObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "mode", "avg_in", "avg_out", "buffering_left", NULL };
    PyObject *py_mode = NULL;
    int avg_in, avg_out;
    gint64 buffering_left;
    GstBufferingMode mode;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"OiiL:GstMessage.set_buffering_stats", kwlist, &py_mode, &avg_in, &avg_out, &buffering_left))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_BUFFERING_MODE, py_mode, (gint *)&mode))
        return NULL;
    pyg_begin_allow_threads;
    gst_message_set_buffering_stats(GST_MESSAGE(self->obj), mode, avg_in, avg_out, buffering_left);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

#line 59 "gstmessage.override"
static PyObject *
_wrap_gst_message_parse_state_changed (PyGstMiniObject *self)
{
  GstState	old;
  GstState	new;
  GstState	pen;

  /* Should raise an exception if it's not a state-changed message */
  if (GST_MESSAGE(self->obj)->type != GST_MESSAGE_STATE_CHANGED) {
	  PyErr_SetString(PyExc_TypeError, "Message is not a state-changed message");
	  return NULL;
  }
  gst_message_parse_state_changed (GST_MESSAGE(self->obj), &old, &new, &pen);

  return Py_BuildValue("[OOO]",
		       pyg_enum_from_gtype(GST_TYPE_STATE, old),
		       pyg_enum_from_gtype(GST_TYPE_STATE, new),
		       pyg_enum_from_gtype(GST_TYPE_STATE, pen));
}
#line 20827 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 226 "gstmessage.override"
static PyObject *
_wrap_gst_message_parse_clock_provide (PyGstMiniObject *self)
{
  GstClock	*clock;
  gboolean	ready;

  if (GST_MESSAGE(self->obj)->type != GST_MESSAGE_CLOCK_PROVIDE) {
	  PyErr_SetString(PyExc_TypeError, "Message is not a 'clock provide' message");
	  return NULL;
  }

  gst_message_parse_clock_provide (GST_MESSAGE(self->obj), &clock, &ready);

  return Py_BuildValue("(OO)",
		       pygobject_new(G_OBJECT (clock)),
		       PyBool_FromLong(ready));
}
#line 20848 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 245 "gstmessage.override"
static PyObject *
_wrap_gst_message_parse_clock_lost (PyGstMiniObject *self)
{
  GstClock	*clock;

  if (GST_MESSAGE(self->obj)->type != GST_MESSAGE_CLOCK_LOST) {
	  PyErr_SetString(PyExc_TypeError, "Message is not a 'clock lost' message");
	  return NULL;
  }

  gst_message_parse_clock_lost (GST_MESSAGE(self->obj), &clock);

  return pygobject_new(G_OBJECT(clock));
}
#line 20866 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 261 "gstmessage.override"
static PyObject *
_wrap_gst_message_parse_new_clock (PyGstMiniObject *self)
{
  GstClock	*clock;

  if (GST_MESSAGE(self->obj)->type != GST_MESSAGE_NEW_CLOCK) {
	  PyErr_SetString(PyExc_TypeError, "Message is not a 'new clock' message");
	  return NULL;
  }

  gst_message_parse_new_clock (GST_MESSAGE(self->obj), &clock);

  return pygobject_new(G_OBJECT(clock));
}
#line 20884 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 80 "gstmessage.override"
static PyObject *
_wrap_gst_message_parse_segment_start (PyGstMiniObject *self)
{
  gint64		position;
  GstFormat             format;

  /* Should raise an exception if it's not a segment start message */
  if (GST_MESSAGE(self->obj)->type != GST_MESSAGE_SEGMENT_START) {
	  PyErr_SetString(PyExc_TypeError, "Message is not a segment start message");
	  return NULL;
  }
  gst_message_parse_segment_start (GST_MESSAGE(self->obj), &format, &position);

  return Py_BuildValue("(OL)",
		       pyg_enum_from_gtype(GST_TYPE_FORMAT, format),
		       position);
}

#line 20906 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 100 "gstmessage.override"
static PyObject *
_wrap_gst_message_parse_segment_done (PyGstMiniObject *self)
{
  gint64		position;
  GstFormat             format;

  /* Should raise an exception if it's not a segment done message */
  if (GST_MESSAGE(self->obj)->type != GST_MESSAGE_SEGMENT_DONE) {
	  PyErr_SetString(PyExc_TypeError, "Message is not a segment done message");
	  return NULL;
  }
  gst_message_parse_segment_done (GST_MESSAGE(self->obj), &format, &position);

  return Py_BuildValue("(OL)",
		       pyg_enum_from_gtype(GST_TYPE_FORMAT, format),
		       position);
}

#line 20928 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 277 "gstmessage.override"
static PyObject *
_wrap_gst_message_parse_duration (PyGstMiniObject *self)
{
  GstFormat	format;
  gint64	duration;

  if (GST_MESSAGE(self->obj)->type != GST_MESSAGE_DURATION) {
	  PyErr_SetString(PyExc_TypeError, "Message is not a 'duration' message");
	  return NULL;
  }

  gst_message_parse_duration (GST_MESSAGE(self->obj), &format, &duration);

  return Py_BuildValue("(OL)",
		       pyg_enum_from_gtype (GST_TYPE_FORMAT, format),
		       duration);
}
#line 20949 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 296 "gstmessage.override"
static PyObject *
_wrap_gst_message_parse_async_start (PyGstMiniObject *self)
{
	gboolean res = FALSE;

	if (GST_MESSAGE(self->obj)->type != GST_MESSAGE_ASYNC_START) {
		PyErr_SetString(PyExc_TypeError, "Message is not an 'async-start' message");
		return NULL;
	}

	gst_message_parse_async_start (GST_MESSAGE(self->obj), &res);

	return PyBool_FromLong (res);
}
#line 20967 "..\\..\\Source\\gst-python\\gst\\gst.c"


static const PyMethodDef _PyGstMessage_methods[] = {
    { "set_seqnum", (PyCFunction)_wrap_gst_message_set_seqnum, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "parse_error", (PyCFunction)_wrap_gst_message_parse_error, METH_NOARGS,
      NULL },
    { "parse_warning", (PyCFunction)_wrap_gst_message_parse_warning, METH_NOARGS,
      NULL },
    { "parse_info", (PyCFunction)_wrap_gst_message_parse_info, METH_NOARGS,
      NULL },
    { "parse_tag", (PyCFunction)_wrap_gst_message_parse_tag, METH_NOARGS,
      NULL },
    { "parse_buffering", (PyCFunction)_wrap_gst_message_parse_buffering, METH_NOARGS,
      NULL },
    { "set_buffering_stats", (PyCFunction)_wrap_gst_message_set_buffering_stats, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "parse_state_changed", (PyCFunction)_wrap_gst_message_parse_state_changed, METH_NOARGS,
      NULL },
    { "parse_clock_provide", (PyCFunction)_wrap_gst_message_parse_clock_provide, METH_NOARGS,
      NULL },
    { "parse_clock_lost", (PyCFunction)_wrap_gst_message_parse_clock_lost, METH_NOARGS,
      NULL },
    { "parse_new_clock", (PyCFunction)_wrap_gst_message_parse_new_clock, METH_NOARGS,
      NULL },
    { "parse_segment_start", (PyCFunction)_wrap_gst_message_parse_segment_start, METH_NOARGS,
      NULL },
    { "parse_segment_done", (PyCFunction)_wrap_gst_message_parse_segment_done, METH_NOARGS,
      NULL },
    { "parse_duration", (PyCFunction)_wrap_gst_message_parse_duration, METH_NOARGS,
      NULL },
    { "parse_async_start", (PyCFunction)_wrap_gst_message_parse_async_start, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

static PyObject *
_wrap_gst_message__get_type(PyObject *self, void *closure)
{
    guint ret;

    ret = GST_MESSAGE(pygstminiobject_get(self))->type;
    return pyg_flags_from_gtype(GST_TYPE_MESSAGE_TYPE, ret);
}

static PyObject *
_wrap_gst_message__get_timestamp(PyObject *self, void *closure)
{
    guint64 ret;

    ret = GST_MESSAGE(pygstminiobject_get(self))->timestamp;
    return PyLong_FromUnsignedLongLong(ret);
}

static PyObject *
_wrap_gst_message__get_src(PyObject *self, void *closure)
{
    GstObject *ret;

    ret = GST_MESSAGE(pygstminiobject_get(self))->src;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gst_message__get_structure(PyObject *self, void *closure)
{
    GstStructure *ret;

    ret = GST_MESSAGE(pygstminiobject_get(self))->structure;
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(GST_TYPE_STRUCTURE, ret, TRUE, TRUE);
}

static const PyGetSetDef gst_message_getsets[] = {
    { "type", (getter)_wrap_gst_message__get_type, (setter)0 },
    { "timestamp", (getter)_wrap_gst_message__get_timestamp, (setter)0 },
    { "src", (getter)_wrap_gst_message__get_src, (setter)0 },
    { "structure", (getter)_wrap_gst_message__get_structure, (setter)0 },
    { NULL, (getter)0, (setter)0 },
};

#line 25 "gstmessage.override"
static PyObject *
_wrap_gst_message_tp_repr (PyGstMiniObject *self)
{
    GstMessage *msg;
    gchar *repr, *structure_str, *src_str;
    PyObject *ret;

    g_assert (self);
    msg = GST_MESSAGE (self->obj);
    g_assert (msg);

    structure_str = msg->structure ? gst_structure_to_string (msg->structure)
        : g_strdup ("(none)");
    if (msg->src) {
      pyg_begin_allow_threads; 
      src_str = gst_object_get_name (msg->src);
      pyg_end_allow_threads; 
    } else {
      src_str = g_strdup ("(no src)");
    }

    repr = g_strdup_printf ("<gst.Message %s from %s at %p>",
        structure_str, src_str, msg);
    g_free (src_str);
    g_free (structure_str);

    ret = PyString_FromStringAndSize(repr, strlen (repr));
    g_free (repr);

    return ret;
}

#line 21083 "..\\..\\Source\\gst-python\\gst\\gst.c"


PyTypeObject PyGstMessage_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.Message",                   /* tp_name */
    sizeof(PyGstMiniObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)_wrap_gst_message_tp_repr,             /* tp_repr */
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
    offsetof(PyGstMiniObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGstMessage_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)gst_message_getsets,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGstMiniObject, inst_dict),                 /* tp_dictoffset */
    (initproc)pygobject_no_constructor,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GstQuery ----------- */

static PyObject *
_wrap_gst_query_set_position(PyGstMiniObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "format", "cur", NULL };
    PyObject *py_format = NULL;
    GstFormat format;
    gint64 cur;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"OL:GstQuery.set_position", kwlist, &py_format, &cur))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_FORMAT, py_format, (gint *)&format))
        return NULL;
    pyg_begin_allow_threads;
    gst_query_set_position(GST_QUERY(self->obj), format, cur);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

#line 27 "gstquery.override"
static PyObject *
_wrap_gst_query_parse_position (PyGstMiniObject *self)
{
	GstFormat	format;
	gint64		cur;

	if (GST_QUERY_TYPE(self->obj) != GST_QUERY_POSITION) {
		PyErr_SetString(PyExc_TypeError, "Query is not a 'Position' query");
		return NULL;
	}
	
	gst_query_parse_position (GST_QUERY(self->obj), &format, &cur);
	
	return Py_BuildValue("(OL)",
			     pyg_enum_from_gtype (GST_TYPE_FORMAT, format),
			     cur);
}
#line 21172 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_query_set_duration(PyGstMiniObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "format", "duration", NULL };
    PyObject *py_format = NULL;
    GstFormat format;
    gint64 duration;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"OL:GstQuery.set_duration", kwlist, &py_format, &duration))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_FORMAT, py_format, (gint *)&format))
        return NULL;
    pyg_begin_allow_threads;
    gst_query_set_duration(GST_QUERY(self->obj), format, duration);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

#line 46 "gstquery.override"
static PyObject *
_wrap_gst_query_parse_duration (PyGstMiniObject *self)
{
	GstFormat	format;
	gint64		cur;

	if (GST_QUERY_TYPE(self->obj) != GST_QUERY_DURATION) {
		PyErr_SetString(PyExc_TypeError, "Query is not a 'Duration' query");
		return NULL;
	}
	
	gst_query_parse_duration (GST_QUERY(self->obj), &format, &cur);
	
	return Py_BuildValue("(OL)",
			     pyg_enum_from_gtype (GST_TYPE_FORMAT, format),
			     cur);
}
#line 21212 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_query_set_latency(PyGstMiniObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "live", "min_latency", "max_latency", NULL };
    int live;
    guint64 min_latency, max_latency;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"iKK:GstQuery.set_latency", kwlist, &live, &min_latency, &max_latency))
        return NULL;
    pyg_begin_allow_threads;
    gst_query_set_latency(GST_QUERY(self->obj), live, min_latency, max_latency);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

#line 228 "gstquery.override"
static PyObject *
_wrap_gst_query_parse_latency (PyGstMiniObject * self)
{
	gboolean islive;
	GstClockTime	minlat, maxlat;

     if (GST_QUERY_TYPE (self->obj) != GST_QUERY_LATENCY) {
	  PyErr_SetString(PyExc_TypeError, "Query is not a 'latency' query");
	  return NULL;
     }
     
     gst_query_parse_latency (GST_QUERY (self->obj), &islive, &minlat, &maxlat);
     
     return Py_BuildValue("(OKK)",
			  PyBool_FromLong(islive),
			  minlat, maxlat);
}
#line 21249 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_query_set_convert(PyGstMiniObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "src_format", "src_value", "dest_format", "dest_value", NULL };
    PyObject *py_src_format = NULL, *py_dest_format = NULL;
    GstFormat src_format, dest_format;
    gint64 src_value, dest_value;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"OLOL:GstQuery.set_convert", kwlist, &py_src_format, &src_value, &py_dest_format, &dest_value))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_FORMAT, py_src_format, (gint *)&src_format))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_FORMAT, py_dest_format, (gint *)&dest_format))
        return NULL;
    pyg_begin_allow_threads;
    gst_query_set_convert(GST_QUERY(self->obj), src_format, src_value, dest_format, dest_value);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

#line 65 "gstquery.override"
static PyObject *
_wrap_gst_query_parse_convert (PyGstMiniObject *self)
{
	GstFormat	srcformat, destformat;
	gint64		srcvalue, destvalue;

	if (GST_QUERY_TYPE(self->obj) != GST_QUERY_CONVERT) {
		PyErr_SetString(PyExc_TypeError, "Query is not a 'Convert' query");
		return NULL;
	}

	gst_query_parse_convert (GST_QUERY(self->obj), 
				 &srcformat, &srcvalue,
				 &destformat, &destvalue);
	
	return Py_BuildValue("(OLOL)",
			     pyg_enum_from_gtype(GST_TYPE_FORMAT, srcformat),
			     srcvalue,
			     pyg_enum_from_gtype(GST_TYPE_FORMAT, destformat),
			     destvalue);
}
#line 21295 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_query_set_segment(PyGstMiniObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "rate", "format", "start_value", "stop_value", NULL };
    PyObject *py_format = NULL;
    double rate;
    GstFormat format;
    gint64 start_value, stop_value;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"dOLL:GstQuery.set_segment", kwlist, &rate, &py_format, &start_value, &stop_value))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_FORMAT, py_format, (gint *)&format))
        return NULL;
    pyg_begin_allow_threads;
    gst_query_set_segment(GST_QUERY(self->obj), rate, format, start_value, stop_value);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

#line 88 "gstquery.override"
static PyObject *
_wrap_gst_query_parse_segment (PyGstMiniObject *self)
{
	gdouble		rate;
	GstFormat	format;
	gint64		start_value;
	gint64		stop_value;

	if (GST_QUERY_TYPE(self->obj) != GST_QUERY_SEGMENT) {
		PyErr_SetString(PyExc_TypeError, "Query is not a 'Segment' query");
		return NULL;
	}

	gst_query_parse_segment (GST_QUERY(self->obj),
				 &rate, &format,
				 &start_value, &stop_value);
	
	return Py_BuildValue("(dOLL)",
			     rate,
			     pyg_enum_from_gtype (GST_TYPE_FORMAT, format),
			     start_value, stop_value);
}
#line 21341 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 215 "gstquery.override"
static PyObject *
_wrap_gst_query_get_structure(PyGstMiniObject *self)
{
    GstStructure *ret;

    pyg_begin_allow_threads;
    ret = gst_query_get_structure(GST_QUERY(self->obj));
    pyg_end_allow_threads;
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(GST_TYPE_STRUCTURE, ret, FALSE, FALSE);
}
#line 21356 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_query_set_seeking(PyGstMiniObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "format", "seekable", "segment_start", "segment_end", NULL };
    PyObject *py_format = NULL;
    int seekable;
    GstFormat format;
    gint64 segment_start, segment_end;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"OiLL:GstQuery.set_seeking", kwlist, &py_format, &seekable, &segment_start, &segment_end))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_FORMAT, py_format, (gint *)&format))
        return NULL;
    pyg_begin_allow_threads;
    gst_query_set_seeking(GST_QUERY(self->obj), format, seekable, segment_start, segment_end);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

#line 112 "gstquery.override"
static PyObject *
_wrap_gst_query_parse_seeking (PyGstMiniObject *self)
{
	GstFormat	format;
	gboolean	seekable;
	gint64		segment_start, segment_end;

	if (GST_QUERY_TYPE(self->obj) != GST_QUERY_SEEKING) {
		PyErr_SetString(PyExc_TypeError, "Query is not a 'Seeking' query");
		return NULL;
	}

	gst_query_parse_seeking (GST_QUERY(self->obj),
				 &format, &seekable,
				 &segment_start, &segment_end);
	
	return Py_BuildValue("(OOLL)",
			     pyg_enum_from_gtype (GST_TYPE_FORMAT, format),
			     PyBool_FromLong(seekable),
			     segment_start,
			     segment_end);
}
#line 21402 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 178 "gstquery.override"
static PyObject *
_wrap_gst_query_set_formats (PyGstMiniObject *self, PyObject *args)
{
	PyObject	*ret = NULL;
	gint	len, i;
	GstFormat *formats;

	if (GST_QUERY_TYPE (self->obj) != GST_QUERY_FORMATS) {
		PyErr_SetString(PyExc_TypeError, "Query is not a 'Formats' query");
		return NULL;
	}

	if ((len = PyTuple_Size(args)) < 1) {
		PyErr_SetString(PyExc_TypeError, "You need to supply at least one gst.Format");
		return NULL;
	}

	formats = g_new0(GstFormat, len);

	for (i = 0; i < len; i++) {
		if (pyg_enum_get_value(GST_TYPE_FORMAT, 
				       PyTuple_GetItem(args, i), 
				       (gint *) &formats[i]))
			goto beach;
	}

	gst_query_set_formatsv (GST_QUERY(self->obj), len, formats);

	Py_INCREF(Py_None);
	ret = Py_None;

  beach:
	g_free(formats);
	return ret;
}
#line 21441 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 136 "gstquery.override"
static PyObject *
_wrap_gst_query_parse_formats_length (PyGstMiniObject *self)
{
	PyObject	*ret;
	guint		n_formats;
	
	if (GST_QUERY_TYPE(self->obj) != GST_QUERY_FORMATS) {
		PyErr_SetString(PyExc_TypeError, "Query is not a 'Formats' query");
		return NULL;
	}

	gst_query_parse_formats_length (GST_QUERY (self->obj),
					&n_formats);

	ret = PyInt_FromLong(n_formats);
	
	return ret;
}
#line 21463 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 156 "gstquery.override"
static PyObject *
_wrap_gst_query_parse_formats_nth (PyGstMiniObject *self, PyObject *args, PyObject *kwargs)
{
	static char *kwlist[] = {"nth", NULL};
	guint		nth;
	GstFormat	format;

	if (GST_QUERY_TYPE (self->obj) != GST_QUERY_FORMATS) {
		PyErr_SetString(PyExc_TypeError, "Query is not a 'Formats' query");
		return NULL;
	}

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "I:GstQuery.parse_formats_nth", kwlist, &nth))
		return NULL;

	gst_query_parse_formats_nth (GST_QUERY (self->obj),
				     nth, &format);

	return pyg_enum_from_gtype (GST_TYPE_FORMAT, format);
}
#line 21487 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_query_set_buffering_percent(PyGstMiniObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "busy", "percent", NULL };
    int busy, percent;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ii:GstQuery.set_buffering_percent", kwlist, &busy, &percent))
        return NULL;
    pyg_begin_allow_threads;
    gst_query_set_buffering_percent(GST_QUERY(self->obj), busy, percent);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_query_set_buffering_stats(PyGstMiniObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "mode", "avg_in", "avg_out", "buffering_left", NULL };
    PyObject *py_mode = NULL;
    int avg_in, avg_out;
    gint64 buffering_left;
    GstBufferingMode mode;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"OiiL:GstQuery.set_buffering_stats", kwlist, &py_mode, &avg_in, &avg_out, &buffering_left))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_BUFFERING_MODE, py_mode, (gint *)&mode))
        return NULL;
    pyg_begin_allow_threads;
    gst_query_set_buffering_stats(GST_QUERY(self->obj), mode, avg_in, avg_out, buffering_left);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_query_set_buffering_range(PyGstMiniObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "format", "start", "stop", "estimated_total", NULL };
    PyObject *py_format = NULL;
    GstFormat format;
    gint64 start, stop, estimated_total;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"OLLL:GstQuery.set_buffering_range", kwlist, &py_format, &start, &stop, &estimated_total))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_FORMAT, py_format, (gint *)&format))
        return NULL;
    pyg_begin_allow_threads;
    gst_query_set_buffering_range(GST_QUERY(self->obj), format, start, stop, estimated_total);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_query_set_uri(PyGstMiniObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "uri", NULL };
    char *uri;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:GstQuery.set_uri", kwlist, &uri))
        return NULL;
    pyg_begin_allow_threads;
    gst_query_set_uri(GST_QUERY(self->obj), uri);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGstQuery_methods[] = {
    { "set_position", (PyCFunction)_wrap_gst_query_set_position, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "parse_position", (PyCFunction)_wrap_gst_query_parse_position, METH_NOARGS,
      NULL },
    { "set_duration", (PyCFunction)_wrap_gst_query_set_duration, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "parse_duration", (PyCFunction)_wrap_gst_query_parse_duration, METH_NOARGS,
      NULL },
    { "set_latency", (PyCFunction)_wrap_gst_query_set_latency, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "parse_latency", (PyCFunction)_wrap_gst_query_parse_latency, METH_NOARGS,
      NULL },
    { "set_convert", (PyCFunction)_wrap_gst_query_set_convert, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "parse_convert", (PyCFunction)_wrap_gst_query_parse_convert, METH_NOARGS,
      NULL },
    { "set_segment", (PyCFunction)_wrap_gst_query_set_segment, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "parse_segment", (PyCFunction)_wrap_gst_query_parse_segment, METH_NOARGS,
      NULL },
    { "get_structure", (PyCFunction)_wrap_gst_query_get_structure, METH_NOARGS,
      NULL },
    { "set_seeking", (PyCFunction)_wrap_gst_query_set_seeking, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "parse_seeking", (PyCFunction)_wrap_gst_query_parse_seeking, METH_NOARGS,
      NULL },
    { "set_formats", (PyCFunction)_wrap_gst_query_set_formats, METH_VARARGS,
      NULL },
    { "parse_formats_length", (PyCFunction)_wrap_gst_query_parse_formats_length, METH_NOARGS,
      NULL },
    { "parse_formats_nth", (PyCFunction)_wrap_gst_query_parse_formats_nth, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_buffering_percent", (PyCFunction)_wrap_gst_query_set_buffering_percent, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_buffering_stats", (PyCFunction)_wrap_gst_query_set_buffering_stats, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_buffering_range", (PyCFunction)_wrap_gst_query_set_buffering_range, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_uri", (PyCFunction)_wrap_gst_query_set_uri, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

static PyObject *
_wrap_gst_query__get_type(PyObject *self, void *closure)
{
    gint ret;

    ret = GST_QUERY(pygstminiobject_get(self))->type;
    return pyg_enum_from_gtype(GST_TYPE_QUERY_TYPE, ret);
}

static PyObject *
_wrap_gst_query__get_structure(PyObject *self, void *closure)
{
    GstStructure *ret;

    ret = GST_QUERY(pygstminiobject_get(self))->structure;
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(GST_TYPE_STRUCTURE, ret, TRUE, TRUE);
}

static const PyGetSetDef gst_query_getsets[] = {
    { "type", (getter)_wrap_gst_query__get_type, (setter)0 },
    { "structure", (getter)_wrap_gst_query__get_structure, (setter)0 },
    { NULL, (getter)0, (setter)0 },
};

PyTypeObject PyGstQuery_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.Query",                   /* tp_name */
    sizeof(PyGstMiniObject),          /* tp_basicsize */
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
    offsetof(PyGstMiniObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGstQuery_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)gst_query_getsets,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGstMiniObject, inst_dict),                 /* tp_dictoffset */
    (initproc)pygobject_no_constructor,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GstURIHandler ----------- */

static PyObject *
_wrap_gst_uri_handler_get_uri_type(PyGObject *self)
{
    gint ret;

    pyg_begin_allow_threads;
    ret = gst_uri_handler_get_uri_type(GST_URI_HANDLER(self->obj));
    pyg_end_allow_threads;
    return pyg_enum_from_gtype(GST_TYPE_URI_TYPE, ret);
}

static PyObject *
_wrap_gst_uri_handler_get_protocols(PyGObject *self)
{
    gchar **ret;

    pyg_begin_allow_threads;
    ret = gst_uri_handler_get_protocols(GST_URI_HANDLER(self->obj));
    pyg_end_allow_threads;
    if (ret) {
        guint size = g_strv_length(ret);
        PyObject *py_ret = PyTuple_New(size);
        gint i;
        for (i = 0; i < size; i++)
            PyTuple_SetItem(py_ret, i,
                PyString_FromString(ret[i]));
        return py_ret;
    }
    return PyTuple_New (0);

}

static PyObject *
_wrap_gst_uri_handler_get_uri(PyGObject *self)
{
    const gchar *ret;

    pyg_begin_allow_threads;
    ret = gst_uri_handler_get_uri(GST_URI_HANDLER(self->obj));
    pyg_end_allow_threads;
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_uri_handler_set_uri(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "uri", NULL };
    char *uri;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:GstURIHandler.set_uri", kwlist, &uri))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_uri_handler_set_uri(GST_URI_HANDLER(self->obj), uri);
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_uri_handler_new_uri(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "uri", NULL };
    char *uri;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:GstURIHandler.new_uri", kwlist, &uri))
        return NULL;
    pyg_begin_allow_threads;
    gst_uri_handler_new_uri(GST_URI_HANDLER(self->obj), uri);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

#line 1572 "..\\..\\Source\\gst-python\\gst\\gst.override"
static PyObject *
_wrap_GstURIHandler__do_get_type_full(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GstURIHandlerInterface *iface;
    static char *kwlist[] = { "self", "type", NULL };
    PyGObject *self;
    PyObject *py_type = NULL;
    GType type;
    guint ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O:GstURIHandler.get_type_full", kwlist, &PyGstURIHandler_Type, &self, &py_type))
        return NULL;
    if ((type = pyg_type_from_object(py_type)) == 0)
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GST_TYPE_URI_HANDLER);
    if (iface->get_type_full)
        ret = iface->get_type_full(type);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GstURIHandler.get_type_full not implemented");
        return NULL;
    }
    return PyLong_FromUnsignedLong(ret);
}
#line 21778 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 1597 "..\\..\\Source\\gst-python\\gst\\gst.override"
static PyObject *
_wrap_GstURIHandler__do_get_protocols_full(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GstURIHandlerInterface *iface;
    static char *kwlist[] = { "self", "type", NULL };
    PyGObject *self;
    PyObject *py_type = NULL;
    GType type;
    gchar **ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O:GstURIHandler.get_protocols_full", kwlist, &PyGstURIHandler_Type, &self, &py_type))
        return NULL;
    if ((type = pyg_type_from_object(py_type)) == 0)
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GST_TYPE_URI_HANDLER);
    if (iface->get_protocols_full)
        ret = iface->get_protocols_full(type);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GstURIHandler.get_protocols_full not implemented");
        return NULL;
    }
    if (ret) {
        guint size = g_strv_length(ret);
        PyObject *py_ret = PyTuple_New(size);
        gint i;
        for (i = 0; i < size; i++)
            PyTuple_SetItem(py_ret, i,
                PyString_FromString(ret[i]));
        return py_ret;
    }
    return PyTuple_New (0);

}

#line 21816 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_GstURIHandler__do_get_uri(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GstURIHandlerInterface *iface;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    const gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GstURIHandler.get_uri", kwlist, &PyGstURIHandler_Type, &self))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GST_TYPE_URI_HANDLER);
    if (iface->get_uri)
        ret = iface->get_uri(GST_URI_HANDLER(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GstURIHandler.get_uri not implemented");
        return NULL;
    }
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GstURIHandler__do_set_uri(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GstURIHandlerInterface *iface;
    static char *kwlist[] = { "self", "uri", NULL };
    PyGObject *self;
    char *uri;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!s:GstURIHandler.set_uri", kwlist, &PyGstURIHandler_Type, &self, &uri))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GST_TYPE_URI_HANDLER);
    if (iface->set_uri)
        ret = iface->set_uri(GST_URI_HANDLER(self->obj), uri);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GstURIHandler.set_uri not implemented");
        return NULL;
    }
    return PyBool_FromLong(ret);

}

static const PyMethodDef _PyGstURIHandler_methods[] = {
    { "get_uri_type", (PyCFunction)_wrap_gst_uri_handler_get_uri_type, METH_NOARGS,
      NULL },
    { "get_protocols", (PyCFunction)_wrap_gst_uri_handler_get_protocols, METH_NOARGS,
      NULL },
    { "get_uri", (PyCFunction)_wrap_gst_uri_handler_get_uri, METH_NOARGS,
      NULL },
    { "set_uri", (PyCFunction)_wrap_gst_uri_handler_set_uri, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "new_uri", (PyCFunction)_wrap_gst_uri_handler_new_uri, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "do_get_type_full", (PyCFunction)_wrap_GstURIHandler__do_get_type_full, METH_VARARGS|METH_CLASS,
      NULL },
    { "do_get_protocols_full", (PyCFunction)_wrap_GstURIHandler__do_get_protocols_full, METH_VARARGS|METH_CLASS,
      NULL },
    { "do_get_uri", (PyCFunction)_wrap_GstURIHandler__do_get_uri, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_set_uri", (PyCFunction)_wrap_GstURIHandler__do_set_uri, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject PyGstURIHandler_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.URIHandler",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGstURIHandler_methods, /* tp_methods */
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

#line 1436 "..\\..\\Source\\gst-python\\gst\\gst.override"
static GstURIType
_wrap_GstURIHandler__proxy_do_get_type_full (GType type)
{
    PyGILState_STATE __py_state;
    PyTypeObject *py_class;
    PyObject *py_method;
    PyObject *py_retval;
    guint retval;
    
    __py_state = pyg_gil_state_ensure();
    py_class = pygobject_lookup_class (type);
    if (py_class == NULL) {
      pyg_gil_state_release (__py_state);
      return GST_URI_UNKNOWN;
    }

    py_method = PyObject_GetAttrString((PyObject *) py_class, "do_get_type_full");
    Py_DECREF (py_class);
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return GST_URI_UNKNOWN;
    }

    py_retval = PyObject_CallObject(py_method, NULL);
    Py_DECREF (py_method);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return GST_URI_UNKNOWN;
    }

    retval = PyLong_AsLong (py_retval);
    Py_DECREF(py_retval);
    pyg_gil_state_release(__py_state);
    
    return retval;
}

#line 21973 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 1479 "..\\..\\Source\\gst-python\\gst\\gst.override"
static gchar **
_wrap_GstURIHandler__proxy_do_get_protocols_full (GType type)
{
    PyGILState_STATE __py_state;
    PyTypeObject *py_class;
    PyObject *py_method;
    PyObject *py_retval;
    Py_ssize_t ret_size, i;
    gchar **retval;

    __py_state = pyg_gil_state_ensure();
    py_class = pygobject_lookup_class (type);
    if (py_class == NULL) {
      pyg_gil_state_release (__py_state);
      return NULL;
    }

    py_method = PyObject_GetAttrString((PyObject *) py_class, "do_get_protocols_full");
    Py_DECREF (py_class);
    if (!py_method) {
      if (PyErr_Occurred())
          PyErr_Print();
      pyg_gil_state_release(__py_state);
      return NULL;
    }

    py_retval = PyObject_CallObject(py_method, NULL);
    Py_DECREF (py_method);
    if (!py_retval) {
      if (PyErr_Occurred())
          PyErr_Print();
      pyg_gil_state_release(__py_state);
      return NULL;
    }

    if (!PySequence_Check (py_retval)) {
      PyErr_SetString (PyExc_TypeError, "GstURIHandler.do_get_protocols_full "
        "must return a sequence of strings");
      Py_DECREF (py_retval);
      return NULL;
    }
    
    ret_size = PySequence_Size (py_retval);
    if (ret_size == -1) {
      Py_DECREF (py_retval);
      pyg_gil_state_release(__py_state);
      return NULL;
    }

    retval = g_new (gchar *, ret_size + 1);
    retval[ret_size] = NULL;
    for (i = 0; i < PySequence_Size (py_retval); ++i) {
      PyObject *item = PySequence_GetItem (py_retval, i);
      if (!item) {
        if (PyErr_Occurred ())
          PyErr_Print ();
        g_strfreev (retval);
        Py_DECREF (py_retval);
        pyg_gil_state_release(__py_state);
        return NULL;
      }

      if (!PyString_Check (item)) {
        PyErr_SetString (PyExc_TypeError, "GstURIHandler.do_get_protocols_full "
          "must return a sequence of strings");
        Py_DECREF (item);
        g_strfreev (retval);
        Py_DECREF (py_retval);
        pyg_gil_state_release(__py_state);
        return NULL;
      }

      retval [i] = PyString_AsString (item);
      if (!retval [i]) {
        if (PyErr_Occurred ())
          PyErr_Print ();
        g_strfreev (retval);
        Py_DECREF (item);
        Py_DECREF (py_retval);
        pyg_gil_state_release(__py_state);
        return NULL;
      }

      Py_DECREF (item);
    }

    Py_DECREF(py_retval);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
#line 22068 "..\\..\\Source\\gst-python\\gst\\gst.c"


static const gchar *
_wrap_GstURIHandler__proxy_do_get_uri(GstURIHandler *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    const gchar *retval;
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
    
    
    py_method = PyObject_GetAttrString(py_self, "do_get_uri");
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
    if (!PyString_Check(py_retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        PyErr_SetString(PyExc_TypeError, "retval should be a string");
        Py_DECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    retval = g_strdup(PyString_AsString(py_retval));
    
    
    Py_DECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gboolean
_wrap_GstURIHandler__proxy_do_set_uri(GstURIHandler *self, const gchar*uri)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_uri;
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
    py_uri = PyString_FromString(uri);
    if (!py_uri) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_uri);
    
    py_method = PyObject_GetAttrString(py_self, "do_set_uri");
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

static void
__GstURIHandler__interface_init(GstURIHandlerInterface *iface, PyTypeObject *pytype)
{
    GstURIHandlerInterface *parent_iface = g_type_interface_peek_parent(iface);
    PyObject *py_method;

    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_get_type_full") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->get_type_full = _wrap_GstURIHandler__proxy_do_get_type_full;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->get_type_full = parent_iface->get_type_full;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_get_protocols_full") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->get_protocols_full = _wrap_GstURIHandler__proxy_do_get_protocols_full;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->get_protocols_full = parent_iface->get_protocols_full;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_get_uri") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->get_uri = _wrap_GstURIHandler__proxy_do_get_uri;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->get_uri = parent_iface->get_uri;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_set_uri") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->set_uri = _wrap_GstURIHandler__proxy_do_set_uri;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->set_uri = parent_iface->set_uri;
        }
    Py_XDECREF(py_method);
    }
}


static const GInterfaceInfo __GstURIHandler__iinfo = {
    (GInterfaceInitFunc) __GstURIHandler__interface_init,
    NULL,
    NULL
};


/* ----------- GstTagSetter ----------- */

static PyObject *
_wrap_gst_tag_setter_reset_tags(PyGObject *self)
{
    pyg_begin_allow_threads;
    gst_tag_setter_reset_tags(GST_TAG_SETTER(self->obj));
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_tag_setter_merge_tags(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "list", "mode", NULL };
    GstTagList *list = NULL;
    PyObject *py_list, *py_mode = NULL;
    GstTagMergeMode mode;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"OO:GstTagSetter.merge_tags", kwlist, &py_list, &py_mode))
        return NULL;
    if (pyg_boxed_check(py_list, GST_TYPE_TAG_LIST))
        list = pyg_boxed_get(py_list, GstTagList);
    else {
        PyErr_SetString(PyExc_TypeError, "list should be a GstTagList");
        return NULL;
    }
    if (pyg_enum_get_value(GST_TYPE_TAG_MERGE_MODE, py_mode, (gint *)&mode))
        return NULL;
    pyg_begin_allow_threads;
    gst_tag_setter_merge_tags(GST_TAG_SETTER(self->obj), list, mode);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

#line 651 "..\\..\\Source\\gst-python\\gst\\gst.override"
static PyObject *
_wrap_gst_tag_setter_get_tag_list(PyGObject *self)
{
    GstTagList *ret;

    ret = (GstTagList*)gst_tag_setter_get_tag_list(GST_TAG_SETTER(self->obj));
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(GST_TYPE_TAG_LIST, ret, TRUE, TRUE);
}
#line 22304 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_tag_setter_set_tag_merge_mode(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "mode", NULL };
    PyObject *py_mode = NULL;
    GstTagMergeMode mode;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:GstTagSetter.set_tag_merge_mode", kwlist, &py_mode))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_TAG_MERGE_MODE, py_mode, (gint *)&mode))
        return NULL;
    pyg_begin_allow_threads;
    gst_tag_setter_set_tag_merge_mode(GST_TAG_SETTER(self->obj), mode);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_tag_setter_get_tag_merge_mode(PyGObject *self)
{
    gint ret;

    pyg_begin_allow_threads;
    ret = gst_tag_setter_get_tag_merge_mode(GST_TAG_SETTER(self->obj));
    pyg_end_allow_threads;
    return pyg_enum_from_gtype(GST_TYPE_TAG_MERGE_MODE, ret);
}

static const PyMethodDef _PyGstTagSetter_methods[] = {
    { "reset_tags", (PyCFunction)_wrap_gst_tag_setter_reset_tags, METH_NOARGS,
      NULL },
    { "merge_tags", (PyCFunction)_wrap_gst_tag_setter_merge_tags, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_tag_list", (PyCFunction)_wrap_gst_tag_setter_get_tag_list, METH_NOARGS,
      NULL },
    { "set_tag_merge_mode", (PyCFunction)_wrap_gst_tag_setter_set_tag_merge_mode, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_tag_merge_mode", (PyCFunction)_wrap_gst_tag_setter_get_tag_merge_mode, METH_NOARGS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject PyGstTagSetter_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.TagSetter",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGstTagSetter_methods, /* tp_methods */
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



/* ----------- GstImplementsInterface ----------- */

static PyObject *
_wrap_GstImplementsInterface__do_supported(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GstImplementsInterfaceClass *iface;
    static char *kwlist[] = { "self", "iface_type", NULL };
    PyGObject *self;
    PyObject *py_iface_type = NULL;
    GType iface_type;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O:GstImplementsInterface.supported", kwlist, &PyGstImplementsInterface_Type, &self, &py_iface_type))
        return NULL;
    if ((iface_type = pyg_type_from_object(py_iface_type)) == 0)
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GST_TYPE_IMPLEMENTS_INTERFACE);
    if (iface->supported)
        ret = iface->supported(GST_IMPLEMENTS_INTERFACE(self->obj), iface_type);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GstImplementsInterface.supported not implemented");
        return NULL;
    }
    return PyBool_FromLong(ret);

}

static const PyMethodDef _PyGstImplementsInterface_methods[] = {
    { "do_supported", (PyCFunction)_wrap_GstImplementsInterface__do_supported, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject PyGstImplementsInterface_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.ImplementsInterface",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGstImplementsInterface_methods, /* tp_methods */
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

static gboolean
_wrap_GstImplementsInterface__proxy_do_supported(GstImplementsInterface *self, GType iface_type)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_iface_type;
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
    py_iface_type = PyInt_FromLong(iface_type);
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_iface_type);
    
    py_method = PyObject_GetAttrString(py_self, "do_supported");
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

static void
__GstImplementsInterface__interface_init(GstImplementsInterfaceClass *iface, PyTypeObject *pytype)
{
    GstImplementsInterfaceClass *parent_iface = g_type_interface_peek_parent(iface);
    PyObject *py_method;

    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_supported") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->supported = _wrap_GstImplementsInterface__proxy_do_supported;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->supported = parent_iface->supported;
        }
    Py_XDECREF(py_method);
    }
}


static const GInterfaceInfo __GstImplementsInterface__iinfo = {
    (GInterfaceInitFunc) __GstImplementsInterface__interface_init,
    NULL,
    NULL
};


/* ----------- GstPreset ----------- */

static PyObject *
_wrap_gst_preset_load_preset(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", NULL };
    char *name;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:GstPreset.load_preset", kwlist, &name))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_preset_load_preset(GST_PRESET(self->obj), name);
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_preset_save_preset(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", NULL };
    char *name;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:GstPreset.save_preset", kwlist, &name))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_preset_save_preset(GST_PRESET(self->obj), name);
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_preset_rename_preset(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "old_name", "new_name", NULL };
    char *old_name, *new_name;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ss:GstPreset.rename_preset", kwlist, &old_name, &new_name))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_preset_rename_preset(GST_PRESET(self->obj), old_name, new_name);
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_preset_delete_preset(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", NULL };
    char *name;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:GstPreset.delete_preset", kwlist, &name))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_preset_delete_preset(GST_PRESET(self->obj), name);
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_preset_set_meta(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", "tag", "value", NULL };
    char *name, *tag, *value;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"sss:GstPreset.set_meta", kwlist, &name, &tag, &value))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_preset_set_meta(GST_PRESET(self->obj), name, tag, value);
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static const PyMethodDef _PyGstPreset_methods[] = {
    { "load_preset", (PyCFunction)_wrap_gst_preset_load_preset, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "save_preset", (PyCFunction)_wrap_gst_preset_save_preset, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "rename_preset", (PyCFunction)_wrap_gst_preset_rename_preset, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "delete_preset", (PyCFunction)_wrap_gst_preset_delete_preset, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_meta", (PyCFunction)_wrap_gst_preset_set_meta, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject PyGstPreset_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "gst.Preset",                   /* tp_name */
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
    (struct PyMethodDef*)_PyGstPreset_methods, /* tp_methods */
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

#line 772 "..\\..\\Source\\gst-python\\gst\\gst.override"
static PyObject *
_wrap_gst_log (PyObject *whatever, PyObject *string)
{
    return pygst_debug_log (whatever, string, GST_LEVEL_LOG, FALSE);
}
#line 22719 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 779 "..\\..\\Source\\gst-python\\gst\\gst.override"
static PyObject *
_wrap_gst_debug (PyObject *whatever, PyObject *string)
{
    return pygst_debug_log (whatever, string, GST_LEVEL_DEBUG, FALSE);
}
#line 22728 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 786 "..\\..\\Source\\gst-python\\gst\\gst.override"
static PyObject *
_wrap_gst_info (PyObject *whatever, PyObject *string)
{
    return pygst_debug_log (whatever, string, GST_LEVEL_INFO, FALSE);
}
#line 22737 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 793 "..\\..\\Source\\gst-python\\gst\\gst.override"
static PyObject *
_wrap_gst_warning (PyObject *whatever, PyObject *string)
{
    return pygst_debug_log (whatever, string, GST_LEVEL_WARNING, FALSE);
}
#line 22746 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 800 "..\\..\\Source\\gst-python\\gst\\gst.override"
static PyObject *
_wrap_gst_error (PyObject *whatever, PyObject *string)
{
    return pygst_debug_log (whatever, string, GST_LEVEL_ERROR, FALSE);
}
#line 22755 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 807 "..\\..\\Source\\gst-python\\gst\\gst.override"
static PyObject *
_wrap_gst_fixme (PyObject *whatever, PyObject *string)
{
    return pygst_debug_log (whatever, string, GST_LEVEL_FIXME, FALSE);
}
#line 22764 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 814 "..\\..\\Source\\gst-python\\gst\\gst.override"
static PyObject *
_wrap_gst_memdump (PyObject *whatever, PyObject *string)
{
    return pygst_debug_log (whatever, string, GST_LEVEL_MEMDUMP, FALSE);
}

#line 22774 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_object_get_controller(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "object", NULL };
    PyGObject *object;
    GstController *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:object_get_controller", kwlist, &PyGObject_Type, &object))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_object_get_controller(G_OBJECT(object->obj));
    pyg_end_allow_threads;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gst_object_set_controller(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "object", "controller", NULL };
    PyGObject *object, *controller;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!:object_set_controller", kwlist, &PyGObject_Type, &object, &PyGstController_Type, &controller))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_object_set_controller(G_OBJECT(object->obj), GST_CONTROLLER(controller->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_object_suggest_next_sync(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "object", NULL };
    PyGObject *object;
    guint64 ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:object_suggest_next_sync", kwlist, &PyGObject_Type, &object))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_object_suggest_next_sync(G_OBJECT(object->obj));
    pyg_end_allow_threads;
    return PyLong_FromUnsignedLongLong(ret);
}

static PyObject *
_wrap_gst_object_sync_values(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "object", "timestamp", NULL };
    PyGObject *object;
    int ret;
    guint64 timestamp;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!K:object_sync_values", kwlist, &PyGObject_Type, &object, &timestamp))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_object_sync_values(G_OBJECT(object->obj), timestamp);
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_object_set_control_source(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "object", "property_name", "csource", NULL };
    PyGObject *object, *csource;
    char *property_name;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!sO!:gst_object_set_control_source", kwlist, &PyGObject_Type, &object, &property_name, &PyGstControlSource_Type, &csource))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_object_set_control_source(G_OBJECT(object->obj), property_name, GST_CONTROL_SOURCE(csource->obj));
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_object_get_control_source(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "object", "property_name", NULL };
    PyGObject *object;
    char *property_name;
    GstControlSource *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!s:gst_object_get_control_source", kwlist, &PyGObject_Type, &object, &property_name))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_object_get_control_source(G_OBJECT(object->obj), property_name);
    pyg_end_allow_threads;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gst_object_get_control_rate(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "object", NULL };
    PyGObject *object;
    guint64 ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:object_get_control_rate", kwlist, &PyGObject_Type, &object))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_object_get_control_rate(G_OBJECT(object->obj));
    pyg_end_allow_threads;
    return PyLong_FromUnsignedLongLong(ret);
}

static PyObject *
_wrap_gst_object_set_control_rate(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "object", "control_rate", NULL };
    PyGObject *object;
    guint64 control_rate;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!K:object_set_control_rate", kwlist, &PyGObject_Type, &object, &control_rate))
        return NULL;
    pyg_begin_allow_threads;
    gst_object_set_control_rate(G_OBJECT(object->obj), control_rate);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_dp_init(PyObject *self)
{
    pyg_begin_allow_threads;
    gst_dp_init();
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

#line 259 "gstlibs.override"
static PyObject *
_wrap_gst_dp_header_payload_length(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "header", NULL };
    char *header;
    gulong ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s:dp_header_payload_length", kwlist, &header))
        return NULL;
    ret = gst_dp_header_payload_length((guint8*)header);
    return PyLong_FromUnsignedLong(ret);
}
#line 22929 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 390 "gstlibs.override"
static PyObject *
_wrap_gst_dp_header_payload_type(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "header", NULL };
    char *header;
    gint ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s:dp_header_payload_type", kwlist, &header))
        return NULL;
    ret = gst_dp_header_payload_type((guint8*) header);
    return pyg_enum_from_gtype(G_TYPE_NONE, ret);
}

#line 22946 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 287 "gstlibs.override"
static PyObject *
_wrap_gst_dp_buffer_from_header(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "header_length", "header", NULL };
    char *header;
    GstBuffer *ret;
    guint header_length;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "Is:dp_buffer_from_header", kwlist, &header_length, &header))
        return NULL;
    ret = gst_dp_buffer_from_header(header_length, (guint8*)header);
    /* pygobject_new handles NULL checking */
    return pygstminiobject_new((GstMiniObject *)ret);
}
#line 22964 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 303 "gstlibs.override"
static PyObject *
_wrap_gst_dp_caps_from_packet(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "header_length", "header", "payload", NULL };
    char *header, *payload;
    GstCaps *ret;
    guint header_length;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "Iss:dp_caps_from_packet", kwlist, &header_length, &header, &payload))
        return NULL;
    ret = gst_dp_caps_from_packet(header_length, 
				  (guint8*) header,
				  (guint8*) payload);
    return pyg_boxed_new (GST_TYPE_CAPS, ret, FALSE, TRUE);
}
#line 22983 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 320 "gstlibs.override"
static PyObject *
_wrap_gst_dp_event_from_packet(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "header_length", "header", "payload", NULL };
    char *header, *payload;
    GstEvent *ret;
    guint header_length;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "Iss:dp_event_from_packet", kwlist, &header_length, &header, &payload))
        return NULL;
    ret = gst_dp_event_from_packet(header_length, 
				   (guint8*) header,
				   (guint8*) payload);
    /* pygobject_new handles NULL checking */
    return pygstminiobject_new((GstMiniObject *)ret);
}
#line 23003 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 338 "gstlibs.override"
static PyObject *
_wrap_gst_dp_validate_header(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "header_length", "header", NULL };
    char *header;
    int ret;
    guint header_length;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "Is:dp_validate_header", kwlist, &header_length, &header))
        return NULL;
    ret = gst_dp_validate_header(header_length, (guint8*) header);
    return PyBool_FromLong(ret);

}
#line 23021 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 354 "gstlibs.override"
static PyObject *
_wrap_gst_dp_validate_payload(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "header_length", "header", "payload", NULL };
    char *header, *payload;
    int ret;
    guint header_length;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "Iss:dp_validate_payload", kwlist, &header_length, &header, &payload))
        return NULL;
    ret = gst_dp_validate_payload(header_length,
				  (guint8*) header,
				  (guint8*) payload);
    return PyBool_FromLong(ret);

}
#line 23041 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 372 "gstlibs.override"
static PyObject *
_wrap_gst_dp_validate_packet(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "header_length", "header", "payload", NULL };
    char *header, *payload;
    int ret;
    guint header_length;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "Iss:dp_validate_packet", kwlist, &header_length, &header, &payload))
        return NULL;
    ret = gst_dp_validate_packet(header_length, 
				 (guint8*) header,
				 (guint8*) payload);
    return PyBool_FromLong(ret);

}
#line 23061 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_type_find_helper(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "src", "size", NULL };
    PyGObject *src;
    GstCaps *ret;
    guint64 size;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!K:type_find_helper", kwlist, &PyGstPad_Type, &src, &size))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_type_find_helper(GST_PAD(src->obj), size);
    pyg_end_allow_threads;
    return pyg_boxed_new (GST_TYPE_CAPS, ret, FALSE, TRUE);
}

#line 1005 "..\\..\\Source\\gst-python\\gst\\gst.override"
static PyObject *
_wrap_gst_type_find_helper_for_buffer (PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "object", "buffer", NULL };
    PyGObject *py_object;
    PyGstMiniObject *py_buffer;
    PyObject *py_ret;
    GstTypeFindProbability prob = 0;
    GstCaps *caps = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O!O!:type_find_helper_for_buffer",
				     kwlist, &PyGstObject_Type, &py_object,
				     &PyGstBuffer_Type, &py_buffer))
	return NULL;
    
    caps = gst_type_find_helper_for_buffer (GST_OBJECT (py_object->obj),
					    GST_BUFFER (py_buffer->obj),
					    &prob);
    py_ret = PyTuple_New(2);
    if (caps)
	PyTuple_SetItem(py_ret, 0, pyg_boxed_new (GST_TYPE_CAPS, caps, FALSE, TRUE));
    else {
	Py_INCREF(Py_None);
	PyTuple_SetItem(py_ret, 0, Py_None);
    }

    if (prob)
	PyTuple_SetItem(py_ret, 1, pyg_enum_from_gtype(GST_TYPE_TYPE_FIND_PROBABILITY, prob));
    else {
	Py_INCREF(Py_None);
	PyTuple_SetItem(py_ret, 1, Py_None);
    }
    
    return py_ret;
}
#line 23116 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_type_find_helper_for_extension(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "obj", "extension", NULL };
    PyGObject *obj;
    char *extension;
    GstCaps *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!s:type_find_helper_for_extension", kwlist, &PyGstObject_Type, &obj, &extension))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_type_find_helper_for_extension(GST_OBJECT(obj->obj), extension);
    pyg_end_allow_threads;
    return pyg_boxed_new (GST_TYPE_CAPS, ret, FALSE, TRUE);
}

static PyObject *
_wrap_gst_version_string(PyObject *self)
{
    gchar *ret;

    pyg_begin_allow_threads;
    ret = gst_version_string();
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
_wrap_gst_segtrap_is_enabled(PyObject *self)
{
    int ret;

    pyg_begin_allow_threads;
    ret = gst_segtrap_is_enabled();
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_segtrap_set_enabled(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "enabled", NULL };
    int enabled;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:segtrap_set_enabled", kwlist, &enabled))
        return NULL;
    pyg_begin_allow_threads;
    gst_segtrap_set_enabled(enabled);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_registry_fork_is_enabled(PyObject *self)
{
    int ret;

    pyg_begin_allow_threads;
    ret = gst_registry_fork_is_enabled();
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_registry_fork_set_enabled(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "enabled", NULL };
    int enabled;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:registry_fork_set_enabled", kwlist, &enabled))
        return NULL;
    pyg_begin_allow_threads;
    gst_registry_fork_set_enabled(enabled);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_update_registry(PyObject *self)
{
    int ret;

    pyg_begin_allow_threads;
    ret = gst_update_registry();
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

#line 927 "..\\..\\Source\\gst-python\\gst\\gst.override"
static PyObject *
_wrap_gst_get_gst_version (PyObject *self)
{
    guint	major, minor, micro, nano;
    PyObject	*py_tuple;

    gst_version (&major, &minor, &micro, &nano);
    py_tuple = PyTuple_New(4);
    PyTuple_SetItem(py_tuple, 0, PyInt_FromLong(major));
    PyTuple_SetItem(py_tuple, 1, PyInt_FromLong(minor));
    PyTuple_SetItem(py_tuple, 2, PyInt_FromLong(micro));
    PyTuple_SetItem(py_tuple, 3, PyInt_FromLong(nano));

    return py_tuple;
}
#line 23234 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 944 "..\\..\\Source\\gst-python\\gst\\gst.override"
static PyObject *
_wrap_gst_get_pygst_version (PyObject *self)
{
    PyObject	*py_tuple;

    py_tuple = Py_BuildValue ("(iiii)", PYGST_MAJOR_VERSION, PYGST_MINOR_VERSION,
                              PYGST_MICRO_VERSION, PYGST_NANO_VERSION);

    return py_tuple;
}
#line 23248 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_buffer_new_and_alloc(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "size", NULL };
    PyObject *py_size = NULL, *py_ret;
    GstBuffer *ret;
    guint size = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:buffer_new_and_alloc", kwlist, &py_size))
        return NULL;
    if (py_size) {
        if (PyLong_Check(py_size))
            size = PyLong_AsUnsignedLong(py_size);
        else if (PyInt_Check(py_size))
            size = PyInt_AsLong(py_size);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'size' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    pyg_begin_allow_threads;
    ret = gst_buffer_new_and_alloc(size);
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_buffer_try_new_and_alloc(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "size", NULL };
    PyObject *py_size = NULL, *py_ret;
    GstBuffer *ret;
    guint size = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:buffer_try_new_and_alloc", kwlist, &py_size))
        return NULL;
    if (py_size) {
        if (PyLong_Check(py_size))
            size = PyLong_AsUnsignedLong(py_size);
        else if (PyInt_Check(py_size))
            size = PyInt_AsLong(py_size);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'size' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    pyg_begin_allow_threads;
    ret = gst_buffer_try_new_and_alloc(size);
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_caps_new_any(PyObject *self)
{
    GstCaps *ret;

    pyg_begin_allow_threads;
    ret = gst_caps_new_any();
    pyg_end_allow_threads;
    return pyg_boxed_new (GST_TYPE_CAPS, ret, FALSE, TRUE);
}

static PyObject *
_wrap_gst_caps_from_string(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "string", NULL };
    char *string;
    GstCaps *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:caps_from_string", kwlist, &string))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_caps_from_string(string);
    pyg_end_allow_threads;
    return pyg_boxed_new (GST_TYPE_CAPS, ret, FALSE, TRUE);
}

static PyObject *
_wrap_GST_DEBUG_BIN_TO_DOT_FILE(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "bin", "details", "filename", NULL };
    PyGObject *bin;
    GstDebugGraphDetails details;
    char *filename;
    PyObject *py_details = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!Os:DEBUG_BIN_TO_DOT_FILE", kwlist, &PyGstBin_Type, &bin, &py_details, &filename))
        return NULL;
    if (pyg_flags_get_value(GST_TYPE_DEBUG_GRAPH_DETAILS, py_details, (gint *)&details))
        return NULL;
    pyg_begin_allow_threads;
    GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(bin->obj), details, filename);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "bin", "details", "filename", NULL };
    PyGObject *bin;
    GstDebugGraphDetails details;
    char *filename;
    PyObject *py_details = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!Os:DEBUG_BIN_TO_DOT_FILE_WITH_TS", kwlist, &PyGstBin_Type, &bin, &py_details, &filename))
        return NULL;
    if (pyg_flags_get_value(GST_TYPE_DEBUG_GRAPH_DETAILS, py_details, (gint *)&details))
        return NULL;
    pyg_begin_allow_threads;
    GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS(GST_BIN(bin->obj), details, filename);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_element_factory_find(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", NULL };
    char *name;
    GstElementFactory *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:element_factory_find", kwlist, &name))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_element_factory_find(name);
    pyg_end_allow_threads;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

#line 27 "gstelementfactory.override"
static PyObject *
_wrap_gst_element_factory_make(PyObject *self, PyObject *args, PyObject *kwargs){
    static char *kwlist[] = { "factoryname", "name", NULL };
    char *factoryname, *name = NULL;
    PyObject *py_ret;
    GstElement *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s|z:element_factory_make", kwlist, &factoryname, &name))
        return NULL;

    pyg_begin_allow_threads;
    ret = gst_element_factory_make(factoryname, name);
    pyg_end_allow_threads;

    if (ret == NULL) {
        PyErr_SetString(PyGstExc_ElementNotFoundError, factoryname);
        return NULL;
    }
    py_ret = pygobject_new((GObject *)ret);
    g_object_unref((GObject *)ret);
    return py_ret;
}
#line 23414 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 662 "..\\..\\Source\\gst-python\\gst\\gst.override"

static GstPlugin *
_pygst_get_plugin(void)
{
  PyObject *dict = NULL, *module = NULL, *pyplugin = NULL;
  GstPlugin *ret;
  
  if (!(module = PyImport_ImportModule ("gst")))
    goto err;
  if (!(dict = PyModule_GetDict (module)))
    goto err;
  if (!(pyplugin = PyDict_GetItemString (dict, "__plugin__")))
    goto err;
  ret = pyg_boxed_get (pyplugin, GstPlugin);
  
  Py_DECREF (module);
  return ret;
  
err:
  Py_XDECREF (module);
  PyErr_Clear ();
  return NULL;
}

static PyObject *
_wrap_gst_element_register(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "type", "elementname", "rank", NULL };
    PyObject *py_type = NULL;
    guint rank = GST_RANK_NONE;
    char *elementname = NULL;
    int ret;
    GType type;

    /* FIXME: can we make the name optional, too? Anyone know a good default? */
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "Os|I:element_register", kwlist,
	&py_type, &elementname, &rank))
        return NULL;
    if ((type = pyg_type_from_object(py_type)) == 0)
        return NULL;
    
    ret = gst_element_register(_pygst_get_plugin(), elementname, rank, type);
    return PyBool_FromLong(ret);
}
#line 23462 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 211 "gstevent.override"
static PyObject *
_wrap_gst_event_new_custom(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "type", "structure", NULL };
    PyObject *py_type = NULL, *py_structure, *py_ret;
    GstEvent *ret;
    GstStructure *structure = NULL;
    GstEventType type;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"OO:event_new_custom", kwlist, &py_type, &py_structure))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_EVENT_TYPE, py_type, (gint *)&type))
        return NULL;
    if (pyg_boxed_check(py_structure, GST_TYPE_STRUCTURE))
        structure = pyg_boxed_get(py_structure, GstStructure);
    else {
        PyErr_SetString(PyExc_TypeError, "structure should be a GstStructure");
        return NULL;
    }
    pyg_begin_allow_threads;
    ret = gst_event_new_custom(type, gst_structure_copy(structure));
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}
#line 23493 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_event_new_flush_start(PyObject *self)
{
    PyObject *py_ret;
    GstEvent *ret;

    pyg_begin_allow_threads;
    ret = gst_event_new_flush_start();
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_event_new_flush_stop(PyObject *self)
{
    PyObject *py_ret;
    GstEvent *ret;

    pyg_begin_allow_threads;
    ret = gst_event_new_flush_stop();
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_event_new_eos(PyObject *self)
{
    PyObject *py_ret;
    GstEvent *ret;

    pyg_begin_allow_threads;
    ret = gst_event_new_eos();
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_event_new_new_segment(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "update", "rate", "format", "start_value", "stop_value", "stream_time", NULL };
    int update;
    double rate;
    GstEvent *ret;
    PyObject *py_format = NULL, *py_ret;
    GstFormat format;
    gint64 start_value, stop_value, stream_time;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"idOLLL:event_new_new_segment", kwlist, &update, &rate, &py_format, &start_value, &stop_value, &stream_time))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_FORMAT, py_format, (gint *)&format))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_event_new_new_segment(update, rate, format, start_value, stop_value, stream_time);
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_event_new_new_segment_full(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "update", "rate", "applied_rate", "format", "start", "stop", "position", NULL };
    int update;
    double rate, applied_rate;
    GstEvent *ret;
    PyObject *py_format = NULL, *py_ret;
    GstFormat format;
    gint64 start, stop, position;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"iddOLLL:event_new_new_segment_full", kwlist, &update, &rate, &applied_rate, &py_format, &start, &stop, &position))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_FORMAT, py_format, (gint *)&format))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_event_new_new_segment_full(update, rate, applied_rate, format, start, stop, position);
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}

#line 240 "gstevent.override"
static PyObject *
_wrap_gst_event_new_tag(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "taglist", NULL };
    GstTagList *taglist = NULL;
    PyObject *py_taglist, *py_ret;
    GstEvent *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:event_new_tag", kwlist, &py_taglist))
        return NULL;
    if (pyg_boxed_check(py_taglist, GST_TYPE_TAG_LIST))
        taglist = pyg_boxed_get(py_taglist, GstTagList);
    else {
        PyErr_SetString(PyExc_TypeError, "taglist should be a GstTagList");
        return NULL;
    }
    pyg_begin_allow_threads;
    ret = gst_event_new_tag(gst_tag_list_copy(taglist));
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}
#line 23614 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_event_new_buffer_size(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "format", "minsize", "maxsize", "async", NULL };
    PyObject *py_format = NULL, *py_ret;
    int async;
    GstFormat format;
    GstEvent *ret;
    gint64 minsize, maxsize;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"OLLi:event_new_buffer_size", kwlist, &py_format, &minsize, &maxsize, &async))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_FORMAT, py_format, (gint *)&format))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_event_new_buffer_size(format, minsize, maxsize, async);
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_event_new_qos(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "proportion", "diff", "timestamp", NULL };
    PyObject *py_ret;
    double proportion;
    gint64 diff;
    GstEvent *ret;
    guint64 timestamp;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"dLK:event_new_qos", kwlist, &proportion, &diff, &timestamp))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_event_new_qos(proportion, diff, timestamp);
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_event_new_seek(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "rate", "format", "flags", "start_type", "start", "stop_type", "stop", NULL };
    GstSeekType start_type, stop_type;
    PyObject *py_format = NULL, *py_flags = NULL, *py_start_type = NULL, *py_stop_type = NULL, *py_ret;
    double rate;
    GstEvent *ret;
    GstFormat format;
    gint64 start, stop;
    GstSeekFlags flags;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"dOOOLOL:event_new_seek", kwlist, &rate, &py_format, &py_flags, &py_start_type, &start, &py_stop_type, &stop))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_FORMAT, py_format, (gint *)&format))
        return NULL;
    if (pyg_flags_get_value(GST_TYPE_SEEK_FLAGS, py_flags, (gint *)&flags))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_SEEK_TYPE, py_start_type, (gint *)&start_type))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_SEEK_TYPE, py_stop_type, (gint *)&stop_type))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_event_new_seek(rate, format, flags, start_type, start, stop_type, stop);
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}

#line 185 "gstevent.override"
static PyObject *
_wrap_gst_event_new_navigation(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "structure", NULL };
    PyObject *py_structure, *py_ret;
    GstEvent *ret;
    GstStructure *structure = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:event_new_navigation", kwlist, &py_structure))
        return NULL;
    if (pyg_boxed_check(py_structure, GST_TYPE_STRUCTURE))
        structure = pyg_boxed_get(py_structure, GstStructure);
    else {
        PyErr_SetString(PyExc_TypeError, "structure should be a GstStructure");
        return NULL;
    }
    pyg_begin_allow_threads;
    ret = gst_event_new_navigation(gst_structure_copy(structure));
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}
#line 23717 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_event_new_latency(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "latency", NULL };
    PyObject *py_ret;
    GstEvent *ret;
    guint64 latency;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"K:event_new_latency", kwlist, &latency))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_event_new_latency(latency);
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_format_register(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "nick", "description", NULL };
    char *nick, *description;
    gint ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ss:format_register", kwlist, &nick, &description))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_format_register(nick, description);
    pyg_end_allow_threads;
    return pyg_enum_from_gtype(GST_TYPE_FORMAT, ret);
}

static PyObject *
_wrap_gst_format_get_by_nick(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "nick", NULL };
    char *nick;
    gint ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:format_get_by_nick", kwlist, &nick))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_format_get_by_nick(nick);
    pyg_end_allow_threads;
    return pyg_enum_from_gtype(GST_TYPE_FORMAT, ret);
}

static PyObject *
_wrap_gst_format_iterate_definitions(PyObject *self)
{
    GstIterator *ret;

    pyg_begin_allow_threads;
    ret = gst_format_iterate_definitions();
    pyg_end_allow_threads;
    return pygst_iterator_new(ret);
}

static PyObject *
_wrap_gst_ghost_pad_new_no_target(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", "dir", NULL };
    char *name;
    PyObject *py_dir = NULL, *py_ret;
    GstPadDirection dir;
    GstPad *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"sO:ghost_pad_new_notarget", kwlist, &name, &py_dir))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_PAD_DIRECTION, py_dir, (gint *)&dir))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_ghost_pad_new_no_target(name, dir);
    pyg_end_allow_threads;
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref((GObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_ghost_pad_new_from_template(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", "target", "templ", NULL };
    char *name;
    PyGObject *target, *templ;
    GstPad *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"sO!O!:ghost_pad_new_from_template", kwlist, &name, &PyGstPad_Type, &target, &PyGstPadTemplate_Type, &templ))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_ghost_pad_new_from_template(name, GST_PAD(target->obj), GST_PAD_TEMPLATE(templ->obj));
    pyg_end_allow_threads;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gst_ghost_pad_new_no_target_from_template(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", "templ", NULL };
    char *name;
    PyGObject *templ;
    GstPad *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"sO!:ghost_pad_new_no_target_from_template", kwlist, &name, &PyGstPadTemplate_Type, &templ))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_ghost_pad_new_no_target_from_template(name, GST_PAD_TEMPLATE(templ->obj));
    pyg_end_allow_threads;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gst_index_factory_find(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", NULL };
    char *name;
    GstIndexFactory *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:index_factory_find", kwlist, &name))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_index_factory_find(name);
    pyg_end_allow_threads;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gst_index_factory_make(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", NULL };
    char *name;
    PyObject *py_ret;
    GstIndex *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:index_factory_make", kwlist, &name))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_index_factory_make(name);
    pyg_end_allow_threads;
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref((GObject *)ret);
    return py_ret;
}

#line 745 "..\\..\\Source\\gst-python\\gst\\gst.override"
static PyObject *
_wrap_gst_debug_log (PyObject *whatever, PyObject *string)
{
#ifndef GST_DISABLE_GST_DEBUG
    gchar	*filename;
    gchar	*func;
    gint	lineno;
    gchar	*message;

    PyObject *py_level = NULL;
    GstDebugLevel level;

    if (!PyArg_ParseTuple(string, "Ossis:gst.debug_log", &py_level, &filename, &func, &lineno, &message)) {
	return NULL;
    }
    if (pyg_enum_get_value(GST_TYPE_DEBUG_LEVEL, py_level, (gint *)&level)) {
        return NULL;
    }

    /* gst_debug_log : category, level, file, function, line, object, format, va_list */
    gst_debug_log (python_debug, level, filename, func, lineno, NULL, "%s", message);
#endif
    Py_INCREF (Py_None);
    return Py_None;
}
#line 23897 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_debug_set_active(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "active", NULL };
    int active;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:debug_set_active", kwlist, &active))
        return NULL;
    pyg_begin_allow_threads;
    gst_debug_set_active(active);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_debug_is_active(PyObject *self)
{
    int ret;

    pyg_begin_allow_threads;
    ret = gst_debug_is_active();
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_debug_set_colored(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "colored", NULL };
    int colored;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:debug_set_colored", kwlist, &colored))
        return NULL;
    pyg_begin_allow_threads;
    gst_debug_set_colored(colored);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_debug_is_colored(PyObject *self)
{
    int ret;

    pyg_begin_allow_threads;
    ret = gst_debug_is_colored();
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_debug_set_default_threshold(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "level", NULL };
    PyObject *py_level = NULL;
    GstDebugLevel level;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:debug_set_default_threshold", kwlist, &py_level))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_DEBUG_LEVEL, py_level, (gint *)&level))
        return NULL;
    pyg_begin_allow_threads;
    gst_debug_set_default_threshold(level);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_debug_get_default_threshold(PyObject *self)
{
    gint ret;

    pyg_begin_allow_threads;
    ret = gst_debug_get_default_threshold();
    pyg_end_allow_threads;
    return pyg_enum_from_gtype(GST_TYPE_DEBUG_LEVEL, ret);
}

static PyObject *
_wrap_gst_debug_set_threshold_for_name(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", "level", NULL };
    char *name;
    PyObject *py_level = NULL;
    GstDebugLevel level;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"sO:debug_set_threshold_for_name", kwlist, &name, &py_level))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_DEBUG_LEVEL, py_level, (gint *)&level))
        return NULL;
    pyg_begin_allow_threads;
    gst_debug_set_threshold_for_name(name, level);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_debug_unset_threshold_for_name(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", NULL };
    char *name;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:debug_unset_threshold_for_name", kwlist, &name))
        return NULL;
    pyg_begin_allow_threads;
    gst_debug_unset_threshold_for_name(name);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_debug_construct_term_color(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "colorinfo", NULL };
    PyObject *py_colorinfo = NULL;
    gchar *ret;
    guint colorinfo = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:debug_construct_term_color", kwlist, &py_colorinfo))
        return NULL;
    if (py_colorinfo) {
        if (PyLong_Check(py_colorinfo))
            colorinfo = PyLong_AsUnsignedLong(py_colorinfo);
        else if (PyInt_Check(py_colorinfo))
            colorinfo = PyInt_AsLong(py_colorinfo);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'colorinfo' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    pyg_begin_allow_threads;
    ret = gst_debug_construct_term_color(colorinfo);
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
_wrap_gst_debug_construct_win_color(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "colorinfo", NULL };
    PyObject *py_colorinfo = NULL;
    int ret;
    guint colorinfo = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:debug_construct_win_color", kwlist, &py_colorinfo))
        return NULL;
    if (py_colorinfo) {
        if (PyLong_Check(py_colorinfo))
            colorinfo = PyLong_AsUnsignedLong(py_colorinfo);
        else if (PyInt_Check(py_colorinfo))
            colorinfo = PyInt_AsLong(py_colorinfo);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'colorinfo' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    pyg_begin_allow_threads;
    ret = gst_debug_construct_win_color(colorinfo);
    pyg_end_allow_threads;
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gst_message_new_eos(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "src", NULL };
    PyGObject *src;
    GstMessage *ret;
    PyObject *py_ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:message_new_eos", kwlist, &PyGstObject_Type, &src))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_message_new_eos(GST_OBJECT(src->obj));
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_message_new_error(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "src", "error", "debug", NULL };
    PyGObject *src;
    PyObject *py_error, *py_ret;
    char *debug;
    GError *error = NULL;
    GstMessage *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!Os:message_new_error", kwlist, &PyGstObject_Type, &src, &py_error, &debug))
        return NULL;
    if (pyg_boxed_check(py_error, GST_TYPE_G_ERROR))
        error = pyg_boxed_get(py_error, GError);
    else {
        PyErr_SetString(PyExc_TypeError, "error should be a GError");
        return NULL;
    }
    pyg_begin_allow_threads;
    ret = gst_message_new_error(GST_OBJECT(src->obj), error, debug);
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_message_new_warning(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "src", "error", "debug", NULL };
    PyGObject *src;
    PyObject *py_error, *py_ret;
    char *debug;
    GError *error = NULL;
    GstMessage *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!Os:message_new_warning", kwlist, &PyGstObject_Type, &src, &py_error, &debug))
        return NULL;
    if (pyg_boxed_check(py_error, GST_TYPE_G_ERROR))
        error = pyg_boxed_get(py_error, GError);
    else {
        PyErr_SetString(PyExc_TypeError, "error should be a GError");
        return NULL;
    }
    pyg_begin_allow_threads;
    ret = gst_message_new_warning(GST_OBJECT(src->obj), error, debug);
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_message_new_info(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "src", "error", "debug", NULL };
    PyGObject *src;
    PyObject *py_error, *py_ret;
    char *debug;
    GError *error = NULL;
    GstMessage *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!Os:message_new_info", kwlist, &PyGstObject_Type, &src, &py_error, &debug))
        return NULL;
    if (pyg_boxed_check(py_error, GST_TYPE_G_ERROR))
        error = pyg_boxed_get(py_error, GError);
    else {
        PyErr_SetString(PyExc_TypeError, "error should be a GError");
        return NULL;
    }
    pyg_begin_allow_threads;
    ret = gst_message_new_info(GST_OBJECT(src->obj), error, debug);
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_message_new_tag(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "src", "tag_list", NULL };
    PyGObject *src;
    GstTagList *tag_list = NULL;
    PyObject *py_tag_list, *py_ret;
    GstMessage *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O:message_new_tag", kwlist, &PyGstObject_Type, &src, &py_tag_list))
        return NULL;
    if (pyg_boxed_check(py_tag_list, GST_TYPE_TAG_LIST))
        tag_list = pyg_boxed_get(py_tag_list, GstTagList);
    else {
        PyErr_SetString(PyExc_TypeError, "tag_list should be a GstTagList");
        return NULL;
    }
   tag_list = g_boxed_copy(GST_TYPE_TAG_LIST, tag_list);
    pyg_begin_allow_threads;
    ret = gst_message_new_tag(GST_OBJECT(src->obj), tag_list);
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_message_new_buffering(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "src", "percent", NULL };
    PyGObject *src;
    int percent;
    GstMessage *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:message_new_buffering", kwlist, &PyGstObject_Type, &src, &percent))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_message_new_buffering(GST_OBJECT(src->obj), percent);
    pyg_end_allow_threads;
    /* pygobject_new handles NULL checking */
    return pygstminiobject_new((GstMiniObject *)ret);
}

static PyObject *
_wrap_gst_message_new_state_changed(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "src", "old_state", "new_state", "pending_state", NULL };
    PyGObject *src;
    PyObject *py_old_state = NULL, *py_new_state = NULL, *py_pending_state = NULL, *py_ret;
    GstState old_state, new_state, pending_state;
    GstMessage *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!OOO:message_new_state_changed", kwlist, &PyGstObject_Type, &src, &py_old_state, &py_new_state, &py_pending_state))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_STATE, py_old_state, (gint *)&old_state))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_STATE, py_new_state, (gint *)&new_state))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_STATE, py_pending_state, (gint *)&pending_state))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_message_new_state_changed(GST_OBJECT(src->obj), old_state, new_state, pending_state);
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_message_new_state_dirty(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "src", NULL };
    PyGObject *src;
    GstMessage *ret;
    PyObject *py_ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:message_new_state_dirty", kwlist, &PyGstObject_Type, &src))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_message_new_state_dirty(GST_OBJECT(src->obj));
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_message_new_clock_provide(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "src", "clock", "ready", NULL };
    PyGObject *src, *clock;
    int ready;
    PyObject *py_ret;
    GstMessage *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!i:message_new_clock_provide", kwlist, &PyGstObject_Type, &src, &PyGstClock_Type, &clock, &ready))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_message_new_clock_provide(GST_OBJECT(src->obj), GST_CLOCK(clock->obj), ready);
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_message_new_clock_lost(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "src", "clock", NULL };
    PyGObject *src, *clock;
    GstMessage *ret;
    PyObject *py_ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!:message_new_clock_lost", kwlist, &PyGstObject_Type, &src, &PyGstClock_Type, &clock))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_message_new_clock_lost(GST_OBJECT(src->obj), GST_CLOCK(clock->obj));
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_message_new_new_clock(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "src", "clock", NULL };
    PyGObject *src, *clock;
    GstMessage *ret;
    PyObject *py_ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!:message_new_new_clock", kwlist, &PyGstObject_Type, &src, &PyGstClock_Type, &clock))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_message_new_new_clock(GST_OBJECT(src->obj), GST_CLOCK(clock->obj));
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_message_new_segment_start(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "src", "format", "position", NULL };
    PyGObject *src;
    PyObject *py_format = NULL, *py_ret;
    GstFormat format;
    gint64 position;
    GstMessage *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!OL:message_new_segment_start", kwlist, &PyGstObject_Type, &src, &py_format, &position))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_FORMAT, py_format, (gint *)&format))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_message_new_segment_start(GST_OBJECT(src->obj), format, position);
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_message_new_segment_done(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "src", "format", "position", NULL };
    PyGObject *src;
    PyObject *py_format = NULL, *py_ret;
    GstFormat format;
    gint64 position;
    GstMessage *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!OL:message_new_segment_done", kwlist, &PyGstObject_Type, &src, &py_format, &position))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_FORMAT, py_format, (gint *)&format))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_message_new_segment_done(GST_OBJECT(src->obj), format, position);
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_message_new_application(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "src", "structure", NULL };
    PyGObject *src;
    PyObject *py_structure, *py_ret;
    GstStructure *structure = NULL;
    GstMessage *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O:message_new_application", kwlist, &PyGstObject_Type, &src, &py_structure))
        return NULL;
    if (pyg_boxed_check(py_structure, GST_TYPE_STRUCTURE))
        structure = pyg_boxed_get(py_structure, GstStructure);
    else {
        PyErr_SetString(PyExc_TypeError, "structure should be a GstStructure");
        return NULL;
    }
   structure = g_boxed_copy(GST_TYPE_STRUCTURE, structure);
    pyg_begin_allow_threads;
    ret = gst_message_new_application(GST_OBJECT(src->obj), structure);
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_message_new_element(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "src", "structure", NULL };
    PyGObject *src;
    PyObject *py_structure, *py_ret;
    GstStructure *structure = NULL;
    GstMessage *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O:message_new_element", kwlist, &PyGstObject_Type, &src, &py_structure))
        return NULL;
    if (pyg_boxed_check(py_structure, GST_TYPE_STRUCTURE))
        structure = pyg_boxed_get(py_structure, GstStructure);
    else {
        PyErr_SetString(PyExc_TypeError, "structure should be a GstStructure");
        return NULL;
    }
   structure = g_boxed_copy(GST_TYPE_STRUCTURE, structure);
    pyg_begin_allow_threads;
    ret = gst_message_new_element(GST_OBJECT(src->obj), structure);
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_message_new_duration(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "src", "format", "duration", NULL };
    PyGObject *src;
    PyObject *py_format = NULL, *py_ret;
    GstFormat format;
    gint64 duration;
    GstMessage *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!OL:message_new_duration", kwlist, &PyGstObject_Type, &src, &py_format, &duration))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_FORMAT, py_format, (gint *)&format))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_message_new_duration(GST_OBJECT(src->obj), format, duration);
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_message_new_async_start(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "src", "new_base_time", NULL };
    PyGObject *src;
    int new_base_time;
    PyObject *py_ret;
    GstMessage *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:message_new_async_start", kwlist, &PyGstObject_Type, &src, &new_base_time))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_message_new_async_start(GST_OBJECT(src->obj), new_base_time);
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_message_new_async_done(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "src", NULL };
    PyGObject *src;
    GstMessage *ret;
    PyObject *py_ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:message_new_async_done", kwlist, &PyGstObject_Type, &src))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_message_new_async_done(GST_OBJECT(src->obj));
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_message_new_latency(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "src", NULL };
    PyGObject *src;
    GstMessage *ret;
    PyObject *py_ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:message_new_latency", kwlist, &PyGstObject_Type, &src))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_message_new_latency(GST_OBJECT(src->obj));
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_message_new_structure_change(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "src", "type", "owner", "busy", NULL };
    PyGObject *src, *owner;
    PyObject *py_type = NULL, *py_ret;
    int busy;
    GstStructureChangeType type;
    GstMessage *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!OO!i:message_new_structure_change", kwlist, &PyGstObject_Type, &src, &py_type, &PyGstElement_Type, &owner, &busy))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_STRUCTURE_CHANGE_TYPE, py_type, (gint *)&type))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_message_new_structure_change(GST_OBJECT(src->obj), type, GST_ELEMENT(owner->obj), busy);
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_message_new_request_state(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "src", "state", NULL };
    PyGObject *src;
    PyObject *py_state = NULL, *py_ret;
    GstState state;
    GstMessage *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O:message_new_request_state", kwlist, &PyGstObject_Type, &src, &py_state))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_STATE, py_state, (gint *)&state))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_message_new_request_state(GST_OBJECT(src->obj), state);
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_message_new_custom(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "type", "src", "structure", NULL };
    PyObject *py_type = NULL, *py_structure, *py_ret;
    PyGObject *src;
    GstMessage *ret;
    GstMessageType type;
    GstStructure *structure = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"OO!O:message_new_custom", kwlist, &py_type, &PyGstObject_Type, &src, &py_structure))
        return NULL;
    if (pyg_flags_get_value(GST_TYPE_MESSAGE_TYPE, py_type, (gint *)&type))
        return NULL;
    if (pyg_boxed_check(py_structure, GST_TYPE_STRUCTURE))
        structure = pyg_boxed_get(py_structure, GstStructure);
    else {
        PyErr_SetString(PyExc_TypeError, "structure should be a GstStructure");
        return NULL;
    }
   structure = g_boxed_copy(GST_TYPE_STRUCTURE, structure);
    pyg_begin_allow_threads;
    ret = gst_message_new_custom(type, GST_OBJECT(src->obj), structure);
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}

#line 724 "..\\..\\Source\\gst-python\\gst\\gst.override"
static PyObject *
_wrap_gst_flow_get_name(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "ret", NULL };
    PyObject *py_ret = NULL;
    const gchar *ret;
    GstFlowReturn flow;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:gst_flow_get_name", kwlist, &py_ret))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_FLOW_RETURN, py_ret, (gint *)&flow))
        return NULL;
    if ((ret = gst_flow_get_name(flow))) {
        return PyString_FromString(ret);
    }
    Py_INCREF(Py_None);
    return Py_None;
}

#line 24598 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_pad_new_from_static_template(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "templ", "name", NULL };
    PyObject *py_templ;
    char *name;
    GstStaticPadTemplate *templ = NULL;
    GstPad *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"Os:pad_new_from_static_template", kwlist, &py_templ, &name))
        return NULL;
    if (pyg_pointer_check(py_templ, GST_TYPE_STATIC_PAD_TEMPLATE))
        templ = pyg_pointer_get(py_templ, GstStaticPadTemplate);
    else {
        PyErr_SetString(PyExc_TypeError, "templ should be a GstStaticPadTemplate");
        return NULL;
    }
    pyg_begin_allow_threads;
    ret = gst_pad_new_from_static_template(templ, name);
    pyg_end_allow_threads;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gst_parse_bin_from_description(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "bin_description", "ghost_unconnected_pads", NULL };
    char *bin_description;
    int ghost_unconnected_pads;
    GstElement *ret;
    GError *err = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"si:parse_bin_from_description", kwlist, &bin_description, &ghost_unconnected_pads))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_parse_bin_from_description(bin_description, ghost_unconnected_pads, &err);
    pyg_end_allow_threads;
    if (pyg_error_check(&err))
        return NULL;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gst_util_get_timestamp(PyObject *self)
{
    guint64 ret;

    pyg_begin_allow_threads;
    ret = gst_util_get_timestamp();
    pyg_end_allow_threads;
    return PyLong_FromUnsignedLongLong(ret);
}

static PyObject *
_wrap_gst_parse_launch(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "pipeline_description", NULL };
    char *pipeline_description;
    PyObject *py_ret;
    GstElement *ret;
    GError *error = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:parse_launch", kwlist, &pipeline_description))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_parse_launch(pipeline_description, &error);
    pyg_end_allow_threads;
    if (pyg_error_check(&error))
        return NULL;
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref((GObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_plugin_load_file(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "filename", NULL };
    char *filename;
    GError *error = NULL;
    GstPlugin *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:plugin_load_file", kwlist, &filename))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_plugin_load_file(filename, &error);
    pyg_end_allow_threads;
    if (pyg_error_check(&error))
        return NULL;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gst_plugin_load_by_name(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", NULL };
    char *name;
    GstPlugin *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:plugin_load_by_name", kwlist, &name))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_plugin_load_by_name(name);
    pyg_end_allow_threads;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gst_query_type_register(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "nick", "description", NULL };
    char *nick, *description;
    gint ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ss:query_type_register", kwlist, &nick, &description))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_query_type_register(nick, description);
    pyg_end_allow_threads;
    return pyg_enum_from_gtype(GST_TYPE_QUERY_TYPE, ret);
}

static PyObject *
_wrap_gst_query_type_get_by_nick(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "nick", NULL };
    char *nick;
    gint ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:query_type_get_by_nick", kwlist, &nick))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_query_type_get_by_nick(nick);
    pyg_end_allow_threads;
    return pyg_enum_from_gtype(GST_TYPE_QUERY_TYPE, ret);
}

static PyObject *
_wrap_gst_query_type_iterate_definitions(PyObject *self)
{
    GstIterator *ret;

    pyg_begin_allow_threads;
    ret = gst_query_type_iterate_definitions();
    pyg_end_allow_threads;
    return pygst_iterator_new(ret);
}

static PyObject *
_wrap_gst_query_new_position(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "format", NULL };
    PyObject *py_format = NULL, *py_ret;
    GstFormat format;
    GstQuery *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:query_new_position", kwlist, &py_format))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_FORMAT, py_format, (gint *)&format))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_query_new_position(format);
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_query_new_duration(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "format", NULL };
    PyObject *py_format = NULL, *py_ret;
    GstFormat format;
    GstQuery *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:query_new_duration", kwlist, &py_format))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_FORMAT, py_format, (gint *)&format))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_query_new_duration(format);
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_query_new_latency(PyObject *self)
{
    PyObject *py_ret;
    GstQuery *ret;

    pyg_begin_allow_threads;
    ret = gst_query_new_latency();
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_query_new_convert(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "src_fmt", "value", "dest_fmt", NULL };
    PyObject *py_src_fmt = NULL, *py_dest_fmt = NULL, *py_ret;
    GstQuery *ret;
    GstFormat src_fmt, dest_fmt;
    gint64 value;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"OLO:query_new_convert", kwlist, &py_src_fmt, &value, &py_dest_fmt))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_FORMAT, py_src_fmt, (gint *)&src_fmt))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_FORMAT, py_dest_fmt, (gint *)&dest_fmt))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_query_new_convert(src_fmt, value, dest_fmt);
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_query_new_segment(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "format", NULL };
    PyObject *py_format = NULL, *py_ret;
    GstFormat format;
    GstQuery *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:query_new_segment", kwlist, &py_format))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_FORMAT, py_format, (gint *)&format))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_query_new_segment(format);
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_query_new_application(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "type", "structure", NULL };
    PyObject *py_type = NULL, *py_structure, *py_ret;
    GstQuery *ret;
    GstStructure *structure = NULL;
    GstQueryType type;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"OO:query_new_application", kwlist, &py_type, &py_structure))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_QUERY_TYPE, py_type, (gint *)&type))
        return NULL;
    if (pyg_boxed_check(py_structure, GST_TYPE_STRUCTURE))
        structure = pyg_boxed_get(py_structure, GstStructure);
    else {
        PyErr_SetString(PyExc_TypeError, "structure should be a GstStructure");
        return NULL;
    }
    pyg_begin_allow_threads;
    ret = gst_query_new_application(type, structure);
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_query_new_seeking(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "format", NULL };
    PyObject *py_format = NULL, *py_ret;
    GstFormat format;
    GstQuery *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:query_new_seeking", kwlist, &py_format))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_FORMAT, py_format, (gint *)&format))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_query_new_seeking(format);
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_query_new_formats(PyObject *self)
{
    PyObject *py_ret;
    GstQuery *ret;

    pyg_begin_allow_threads;
    ret = gst_query_new_formats();
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_query_new_buffering(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "format", NULL };
    PyObject *py_format = NULL, *py_ret;
    GstFormat format;
    GstQuery *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:query_new_buffering", kwlist, &py_format))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_FORMAT, py_format, (gint *)&format))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_query_new_buffering(format);
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_query_new_uri(PyObject *self)
{
    PyObject *py_ret;
    GstQuery *ret;

    pyg_begin_allow_threads;
    ret = gst_query_new_uri();
    pyg_end_allow_threads;
    py_ret = pygstminiobject_new((GstMiniObject *)ret);
    if (ret != NULL)
       gst_mini_object_unref((GstMiniObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_registry_get_default(PyObject *self)
{
    GstRegistry *ret;

    pyg_begin_allow_threads;
    ret = gst_registry_get_default();
    pyg_end_allow_threads;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gst_default_registry_check_feature_version(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "feature_name", "min_major", "min_minor", "min_micro", NULL };
    char *feature_name;
    PyObject *py_min_major = NULL, *py_min_minor = NULL, *py_min_micro = NULL;
    int ret;
    guint min_major = 0, min_minor = 0, min_micro = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"sOOO:default_registry_check_feature_version", kwlist, &feature_name, &py_min_major, &py_min_minor, &py_min_micro))
        return NULL;
    if (py_min_major) {
        if (PyLong_Check(py_min_major))
            min_major = PyLong_AsUnsignedLong(py_min_major);
        else if (PyInt_Check(py_min_major))
            min_major = PyInt_AsLong(py_min_major);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'min_major' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    if (py_min_minor) {
        if (PyLong_Check(py_min_minor))
            min_minor = PyLong_AsUnsignedLong(py_min_minor);
        else if (PyInt_Check(py_min_minor))
            min_minor = PyInt_AsLong(py_min_minor);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'min_minor' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    if (py_min_micro) {
        if (PyLong_Check(py_min_micro))
            min_micro = PyLong_AsUnsignedLong(py_min_micro);
        else if (PyInt_Check(py_min_micro))
            min_micro = PyInt_AsLong(py_min_micro);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'min_micro' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    pyg_begin_allow_threads;
    ret = gst_default_registry_check_feature_version(feature_name, min_major, min_minor, min_micro);
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

#line 317 "gststructure.override"
static PyObject *
_wrap_gst_structure_from_string(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "string", NULL };
    char *string;
    GstStructure *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s:structure_from_string", kwlist, &string))
        return NULL;
    
    ret = gst_structure_from_string(string, NULL);
    
    /* pyg_boxed_new handles NULL checking */
    return pyg_boxed_new(GST_TYPE_STRUCTURE, ret, FALSE, TRUE);
}
#line 25032 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_system_clock_obtain(PyObject *self)
{
    PyObject *py_ret;
    GstClock *ret;

    pyg_begin_allow_threads;
    ret = gst_system_clock_obtain();
    pyg_end_allow_threads;
    py_ret = pygobject_new((GObject *)ret);
    if (ret != NULL)
        g_object_unref((GObject *)ret);
    return py_ret;
}

static PyObject *
_wrap_gst_tag_exists(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "tag", NULL };
    char *tag;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:tag_exists", kwlist, &tag))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_tag_exists(tag);
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_tag_get_tag_type(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "tag", NULL };
    char *tag;
    GType ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:tag_get_tag_type", kwlist, &tag))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_tag_get_tag_type(tag);
    pyg_end_allow_threads;
    return pyg_type_wrapper_new(ret);
}

static PyObject *
_wrap_gst_tag_get_nick(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "tag", NULL };
    char *tag;
    const gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:tag_get_nick", kwlist, &tag))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_tag_get_nick(tag);
    pyg_end_allow_threads;
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_tag_get_description(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "tag", NULL };
    char *tag;
    const gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:tag_get_description", kwlist, &tag))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_tag_get_description(tag);
    pyg_end_allow_threads;
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_tag_get_flag(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "tag", NULL };
    char *tag;
    gint ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:tag_get_flag", kwlist, &tag))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_tag_get_flag(tag);
    pyg_end_allow_threads;
    return pyg_enum_from_gtype(GST_TYPE_TAG_FLAG, ret);
}

static PyObject *
_wrap_gst_tag_is_fixed(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "tag", NULL };
    char *tag;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:tag_is_fixed", kwlist, &tag))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_tag_is_fixed(tag);
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_alloc_trace_available(PyObject *self)
{
    int ret;

    pyg_begin_allow_threads;
    ret = gst_alloc_trace_available();
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_alloc_trace_live_all(PyObject *self)
{
    int ret;

    pyg_begin_allow_threads;
    ret = gst_alloc_trace_live_all();
    pyg_end_allow_threads;
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gst_alloc_trace_print_all(PyObject *self)
{
    pyg_begin_allow_threads;
    gst_alloc_trace_print_all();
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_alloc_trace_print_live(PyObject *self)
{
    pyg_begin_allow_threads;
    gst_alloc_trace_print_live();
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_alloc_trace_set_flags_all(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "flags", NULL };
    PyObject *py_flags = NULL;
    GstAllocTraceFlags flags;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:alloc_trace_set_flags_all", kwlist, &py_flags))
        return NULL;
    if (pyg_flags_get_value(GST_TYPE_ALLOC_TRACE_FLAGS, py_flags, (gint *)&flags))
        return NULL;
    pyg_begin_allow_threads;
    gst_alloc_trace_set_flags_all(flags);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

#line 1042 "..\\..\\Source\\gst-python\\gst\\gst.override"

static guint8 *
gst_type_find_peek_handler (gpointer data, gint64 offset, guint size)
{
    PyGILState_STATE state;
    guint8 * ret = NULL;
    PyObject *py_data;
    PyObject *callback, *args;
    PyObject *py_ret;

    GST_DEBUG ("mkay");

    g_return_val_if_fail (data != NULL, NULL);
    py_data = (PyObject *) data;
    g_assert (PyTuple_Check (py_data));

    state = pyg_gil_state_ensure ();

    /* Figure out the callback and create the arguments */
    if (!(callback = PyTuple_GetItem(py_data, 1)))
	goto beach;

    args = Py_BuildValue ("(OLI)", 
			  PyTuple_GetItem(py_data, 0),
			  offset, size);
    if (!args)
	goto beach;

    /* Call python method */
    py_ret = PyObject_CallObject (callback, args);
    
    /* transform return value (a string) */
    if (!py_ret) {
	Py_DECREF (args);
	goto beach;
    }
    if (!PyString_Check(py_ret)) {
	Py_DECREF (py_ret);
	Py_DECREF (args);
	goto beach;
    } else {
	gchar *str;
	Py_ssize_t len;

	if ((PyString_AsStringAndSize(py_ret, &str, &len)) == -1) {
	    Py_DECREF (py_ret);
	    Py_DECREF (args);
	    goto beach;
	}
	GST_DEBUG ("got string of len %"G_GSSIZE_FORMAT, len);
	if (len)
	    ret = g_memdup((gconstpointer) str, (guint) len);
    }
    Py_DECREF (py_ret);
    Py_DECREF (args);

 beach:
    pyg_gil_state_release (state);
    return ret;
}

static void
gst_type_find_suggest_handler (gpointer data, guint probability, const GstCaps * caps)
{
    PyGILState_STATE state;
    PyObject *py_data;
    PyObject *callback, *args;

    GST_DEBUG ("mkay");

    if (!data)
	return;
    py_data = (PyObject *) data;
    g_assert (PyTuple_Check (py_data));

    state = pyg_gil_state_ensure ();

    /* Figure out the callback and create the arguments */
    if (!(callback = PyTuple_GetItem(py_data, 2)))
	goto beach;

    args = Py_BuildValue ("(OIN)", 
			  PyTuple_GetItem(py_data, 0),
			  probability, pyg_boxed_new (GST_TYPE_CAPS, (GstCaps*) caps, TRUE, TRUE));
    if (!args)
	goto beach;

    /* Call python method */
    PyObject_CallObject (callback, args);
    
    Py_DECREF (args);

 beach:
    pyg_gil_state_release (state);
    return;
}

static guint64
gst_type_find_get_length_handler (gpointer data)
{
    guint64 ret = 0;

    /* Call python method */
    return ret;
}

static PyObject *
_wrap_gst_type_find_new (PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "data", "peekfunction", "suggestfunction", "getlengthfunction", NULL };
    PyObject *py_data;
    gpointer data;
    PyObject *peekfunction;
    PyObject *suggestfunction;
    PyObject *getlengthfunction = NULL;
    PyObject *pytypefind = NULL;
    GstTypeFind *typefind = NULL;

    GST_DEBUG ("poeut");

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "OOO|O:type_find_new",
				     kwlist, &py_data, &peekfunction,
				     &suggestfunction, &getlengthfunction)) {
	PyErr_SetString (PyExc_TypeError, "Error parsing values ...");
	return NULL;
    }

    if (!PyCallable_Check(peekfunction)) {
	PyErr_SetString (PyExc_TypeError, "peekfunction is not callable");
	return NULL;
    }
    if (!PyCallable_Check(suggestfunction)) {
	PyErr_SetString (PyExc_TypeError, "suggestfunction is not callable");
	return NULL;
    }
    if (getlengthfunction && (!PyCallable_Check(suggestfunction))) {
	PyErr_SetString (PyExc_TypeError, "getlengthfunction is not callable");
	return NULL;
    }

    /* Create a python list to put in typefind->data */
    if (getlengthfunction)
	data = Py_BuildValue("(OOOO)", py_data, peekfunction, suggestfunction, getlengthfunction);
    else
	data = Py_BuildValue("(OOO)", py_data, peekfunction, suggestfunction);
    
    typefind = g_new0(GstTypeFind, 1);
    typefind->peek = gst_type_find_peek_handler;
    typefind->suggest = gst_type_find_suggest_handler;
    typefind->data = data;
    if (getlengthfunction)
	typefind->get_length = gst_type_find_get_length_handler;

    pytypefind = pyg_pointer_new (GST_TYPE_TYPE_FIND, typefind);

    if (!pytypefind) {
	PyErr_SetString (PyExc_TypeError, "pyg_pointer_new failed");
    }

    GST_DEBUG ("poeut : %p", pytypefind);

    return pytypefind;
}

#line 25374 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 1208 "..\\..\\Source\\gst-python\\gst\\gst.override"

static void
type_find_function (GstTypeFind *find, gpointer user_data)
{
    PyGILState_STATE state;
    PyObject *data;
    PyObject *callback, *args, *old_args;
    PyObject *typefind;

    state = pyg_gil_state_ensure ();

    typefind = pyg_pointer_new(GST_TYPE_TYPE_FIND, find);

    data = (PyObject *) user_data;
    callback = PyTuple_GET_ITEM(data, 0);
    args = Py_BuildValue("(O)", typefind);
    if (PyTuple_GET_SIZE(data) > 1) {
      old_args = args;
      args = PySequence_Concat(args, PyTuple_GET_ITEM(data, 1));
      Py_DECREF(old_args);
    }

    PyObject_CallObject(callback, args); 
    
    Py_DECREF(args);
    Py_DECREF(typefind);

    pyg_gil_state_release (state);
    
    return;
}

static void
type_find_function_data_destroy_notify(gpointer data)
{
  Py_DECREF((PyObject *) data);
}

static PyObject *
_wrap_gst_type_find_register (PyObject *self, PyObject *args)
{
    guint rank;
    PyObject *required_args;
    PyObject *function;
    PyObject *function_args = NULL;
    PyObject *py_extensions = NULL, *ext;
    PyObject *py_possible_caps = NULL;
    PyObject *py_res = NULL;
    gchar *name;
    gpointer *data = NULL;
    GStrv extensions = NULL;
    guint i, n_extensions;
    GstCaps *possible_caps = NULL;
    gboolean res = FALSE;
    
    if (PyTuple_GET_SIZE(args) > 5) {
      required_args = PyTuple_GetSlice(args, 0, 5);
      function_args = PyTuple_GetSlice(args, 5, PyTuple_GET_SIZE(args));
    } else {
      required_args = args;
    }

    if (!PyArg_ParseTuple(required_args, "siO|OO:type_find_register",
             &name, &rank, &function, &py_extensions, &py_possible_caps)) {
      goto out;
    }

    if (!PyCallable_Check(function)) {
      PyErr_SetString (PyExc_TypeError, "function is not a callable");
      goto out;
    }

    if (py_extensions) {
      n_extensions = PySequence_Size(py_extensions);
      if (n_extensions == -1) {
        goto out;
      }

      if (n_extensions > 0) {
        extensions = (char **) g_malloc(sizeof(char *) * n_extensions + 1);
        for(i = 0; i < n_extensions; ++i) {
          ext = PySequence_GetItem(py_extensions, i);
          
          if (!PyString_Check(ext)) {
            PyErr_SetString(PyExc_TypeError, "extension is not a string");
            goto out;
          }

          extensions[i] = g_strdup(PyString_AS_STRING(ext));
        }

        extensions[n_extensions] = NULL;
      }
    }
    
    if (py_possible_caps)
      possible_caps = pygst_caps_from_pyobject(py_possible_caps, NULL);

    if (function_args)
      data = (gpointer) Py_BuildValue("(OO)", function, function_args);
    else
      data = (gpointer) Py_BuildValue("(O)", function);

    pyg_begin_allow_threads;
    res = gst_type_find_register(NULL, name, rank,
        type_find_function, extensions, possible_caps,
        data, type_find_function_data_destroy_notify);
    pyg_end_allow_threads;

    py_res = PyBool_FromLong(res);

out:
    if (required_args != args) {
      Py_DECREF(required_args);
    }

    Py_XDECREF(function_args);

    if (extensions)
      g_strfreev(extensions);

    if (possible_caps)
      gst_caps_unref(possible_caps);

    if (res == FALSE && data) {
      Py_DECREF((PyObject *) data);
    }

    return py_res;
}

#line 25509 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 905 "..\\..\\Source\\gst-python\\gst\\gst.override"
static PyObject *
_wrap_gst_type_find_factory_get_list (PyObject *self)
{
    GList	*l, *list;
    PyObject	*py_list;
    int		i = 0;

    list = gst_type_find_factory_get_list ();

    py_list = PyList_New(g_list_length(list));
    for (l = list; l ; l = g_list_next(l), i++) {
	GstTypeFindFactory *fact = (GstTypeFindFactory*) l->data;

	PyList_SetItem(py_list, i,
		       pygobject_new (G_OBJECT (fact)));
    }
    g_list_free (list);

    return py_list;
}
#line 25533 "..\\..\\Source\\gst-python\\gst\\gst.c"


static PyObject *
_wrap_gst_uri_protocol_is_valid(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "protocol", NULL };
    char *protocol;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:uri_protocol_is_valid", kwlist, &protocol))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_uri_protocol_is_valid(protocol);
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_uri_protocol_is_supported(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "type", "protocol", NULL };
    PyObject *py_type = NULL;
    char *protocol;
    int ret;
    GstURIType type;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"Os:uri_protocol_is_supported", kwlist, &py_type, &protocol))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_URI_TYPE, py_type, (gint *)&type))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_uri_protocol_is_supported(type, protocol);
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_uri_is_valid(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "uri", NULL };
    char *uri;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:uri_is_valid", kwlist, &uri))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_uri_is_valid(uri);
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_uri_get_protocol(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "uri", NULL };
    char *uri;
    gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:uri_get_protocol", kwlist, &uri))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_uri_get_protocol(uri);
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
_wrap_gst_uri_has_protocol(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "uri", "protocol", NULL };
    char *uri, *protocol;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ss:uri_has_protocol", kwlist, &uri, &protocol))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_uri_has_protocol(uri, protocol);
    pyg_end_allow_threads;
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gst_uri_get_location(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "uri", NULL };
    char *uri;
    gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:uri_get_location", kwlist, &uri))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_uri_get_location(uri);
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
_wrap_gst_uri_construct(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "protocol", "location", NULL };
    char *protocol, *location;
    gchar *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ss:uri_construct", kwlist, &protocol, &location))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_uri_construct(protocol, location);
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
_wrap_gst_element_make_from_uri(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "type", "uri", "elementname", NULL };
    PyObject *py_type = NULL;
    char *uri, *elementname = NULL;
    GstElement *ret;
    GstURIType type;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"Os|z:element_make_from_uri", kwlist, &py_type, &uri, &elementname))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_URI_TYPE, py_type, (gint *)&type))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_element_make_from_uri(type, uri, elementname);
    pyg_end_allow_threads;
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gst_util_set_object_arg(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "object", "name", "value", NULL };
    PyGObject *object;
    char *name, *value;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!ss:util_set_object_arg", kwlist, &PyGObject_Type, &object, &name, &value))
        return NULL;
    pyg_begin_allow_threads;
    gst_util_set_object_arg(G_OBJECT(object->obj), name, value);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_util_dump_mem(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "mem", "size", NULL };
    int mem_len;
    PyObject *py_size = NULL;
    guchar *mem;
    guint size = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s#O:util_dump_mem", kwlist, &mem, &mem_len, &py_size))
        return NULL;
    if (py_size) {
        if (PyLong_Check(py_size))
            size = PyLong_AsUnsignedLong(py_size);
        else if (PyInt_Check(py_size))
            size = PyInt_AsLong(py_size);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'size' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    pyg_begin_allow_threads;
    gst_util_dump_mem(mem, size);
    pyg_end_allow_threads;
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_util_gdouble_to_guint64(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "value", NULL };
    double value;
    guint64 ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"d:util_gdouble_to_guint64", kwlist, &value))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_util_gdouble_to_guint64(value);
    pyg_end_allow_threads;
    return PyLong_FromUnsignedLongLong(ret);
}

static PyObject *
_wrap_gst_util_guint64_to_gdouble(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "value", NULL };
    double ret;
    guint64 value;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"K:util_guint64_to_gdouble", kwlist, &value))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_util_guint64_to_gdouble(value);
    pyg_end_allow_threads;
    return PyFloat_FromDouble(ret);
}

static PyObject *
_wrap_gst_util_uint64_scale(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "val", "num", "denom", NULL };
    guint64 val, num, denom, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"KKK:util_uint64_scale", kwlist, &val, &num, &denom))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_util_uint64_scale(val, num, denom);
    pyg_end_allow_threads;
    return PyLong_FromUnsignedLongLong(ret);
}

static PyObject *
_wrap_gst_util_uint64_scale_int(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "val", "num", "denom", NULL };
    int num, denom;
    guint64 val, ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"Kii:util_uint64_scale_int", kwlist, &val, &num, &denom))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_util_uint64_scale_int(val, num, denom);
    pyg_end_allow_threads;
    return PyLong_FromUnsignedLongLong(ret);
}

static PyObject *
_wrap_gst_util_seqnum_next(PyObject *self)
{
    guint32 ret;

    pyg_begin_allow_threads;
    ret = gst_util_seqnum_next();
    pyg_end_allow_threads;
    return PyLong_FromUnsignedLong(ret);

}

static PyObject *
_wrap_gst_util_seqnum_compare(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "s1", "s2", NULL };
    int ret;
    unsigned long s1, s2;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"kk:util_seqnum_compare", kwlist, &s1, &s2))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_util_seqnum_compare(s1, s2);
    pyg_end_allow_threads;
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gst_element_state_get_name(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "state", NULL };
    PyObject *py_state = NULL;
    const gchar *ret;
    GstState state;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:element_state_get_name", kwlist, &py_state))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_STATE, py_state, (gint *)&state))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_element_state_get_name(state);
    pyg_end_allow_threads;
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gst_element_state_change_return_get_name(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "state_ret", NULL };
    PyObject *py_state_ret = NULL;
    const gchar *ret;
    GstStateChangeReturn state_ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:state_change_return_get_name", kwlist, &py_state_ret))
        return NULL;
    if (pyg_enum_get_value(GST_TYPE_STATE_CHANGE_RETURN, py_state_ret, (gint *)&state_ret))
        return NULL;
    pyg_begin_allow_threads;
    ret = gst_element_state_change_return_get_name(state_ret);
    pyg_end_allow_threads;
    if (ret)
        return PyString_FromString(ret);
    Py_INCREF(Py_None);
    return Py_None;
}

#line 112 "gstelement.override"
static PyObject *
_wrap_gst_element_link_many(PyObject *self, PyObject *args)
{
	PyGObject *element, *element2;
	int i, len;

	len = PyTuple_Size(args);
	if (len < 2)
	{
		PyErr_SetString(PyExc_TypeError, "gst.element_link_many requires at least two argument");
		return NULL;
	}
       

	for (i = 0; i < len; i++)
	{
		element = (PyGObject*)PyTuple_GetItem(args, i);
		if (!pygobject_check(element, &PyGstElement_Type))
		{
			PyErr_SetString(PyExc_TypeError, "argument must be a GstElement");
			return NULL;
		}
	}

	/* Mimic the real gst_element_link_many */
	element = (PyGObject*)PyTuple_GetItem(args, 0);
	element2 = (PyGObject*)PyTuple_GetItem(args, 1);

	i = 2;
	while (1) {
                gboolean res;

                pyg_begin_allow_threads;
		res = gst_element_link(GST_ELEMENT(element->obj),
                                      GST_ELEMENT(element2->obj));
                pyg_end_allow_threads;

                if (!res) 
                {
                    PyErr_Format(PyGstExc_LinkError,
				 "failed to link %s with %s",
				 GST_ELEMENT_NAME(element->obj),
				 GST_ELEMENT_NAME(element2->obj));
                    return NULL;
                }

		if (i >= len)
			break;
		
		element = element2;
		element2 = (PyGObject*)PyTuple_GetItem(args, i);
	
		i++;
	}

	Py_INCREF(Py_True);
	return Py_True;
}


#line 25920 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 314 "gstelement.override"
static PyObject *
_wrap_gst_element_unlink_many(PyObject *self, PyObject *args)
{
	PyGObject *element, *element2;
	int i, len;

	len = PyTuple_Size(args);
	if (len < 2)
	{
		PyErr_SetString(PyExc_TypeError, "gst.element_unlink_many requires at least two arguments");
		return NULL;
	}
       

	for (i = 0; i < len; i++)
	{
		element = (PyGObject*)PyTuple_GetItem(args, i);
		if (!pygobject_check(element, &PyGstElement_Type))
		{
			PyErr_SetString(PyExc_TypeError, "argument must be a GstElement");
			return NULL;
		}
	}

	/* Mimic the real gst_element_unlink_many */
	element = (PyGObject*)PyTuple_GetItem(args, 0);
	element2 = (PyGObject*)PyTuple_GetItem(args, 1);

	i = 2;
	while (1) {
                pyg_begin_allow_threads;
		gst_element_unlink(GST_ELEMENT(element->obj), GST_ELEMENT(element2->obj));
                pyg_end_allow_threads;

		if (i >= len)
			break;
		
		element = element2;
		element2 = (PyGObject*)PyTuple_GetItem(args, i);
	
		i++;
	}
	Py_INCREF(Py_None);
	return Py_None;
}
#line 25969 "..\\..\\Source\\gst-python\\gst\\gst.c"


#line 871 "..\\..\\Source\\gst-python\\gst\\gst.override"
static PyObject *
_wrap_GST_TIME_ARGS(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "time", NULL };
    PyObject *py_time = NULL;
    PyObject *string;
    gchar *ret;
    guint64 time;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:time_to_string", kwlist, &py_time))
        return NULL;
    time = PyInt_AsUnsignedLongLongMask(py_time);
    if (PyErr_Occurred ())
      return NULL;

    if (GST_CLOCK_TIME_IS_VALID (time))
	ret = g_strdup_printf("%"GST_TIME_FORMAT, GST_TIME_ARGS(time));
    else
	ret = g_strdup ("CLOCK_TIME_NONE");

    if (!ret) {
	Py_INCREF(Py_None);
	return Py_None;
    }

    if (!(string = PyString_FromString(ret))) {
	g_free(ret);
	return NULL;
    }
    g_free(ret);
    return string;
}
#line 26005 "..\\..\\Source\\gst-python\\gst\\gst.c"


const PyMethodDef pygst_functions[] = {
    { "log", (PyCFunction)_wrap_gst_log, METH_VARARGS,
      NULL },
    { "debug", (PyCFunction)_wrap_gst_debug, METH_VARARGS,
      NULL },
    { "info", (PyCFunction)_wrap_gst_info, METH_VARARGS,
      NULL },
    { "warning", (PyCFunction)_wrap_gst_warning, METH_VARARGS,
      NULL },
    { "error", (PyCFunction)_wrap_gst_error, METH_VARARGS,
      NULL },
    { "fixme", (PyCFunction)_wrap_gst_fixme, METH_VARARGS,
      NULL },
    { "memdump", (PyCFunction)_wrap_gst_memdump, METH_VARARGS,
      NULL },
    { "object_get_controller", (PyCFunction)_wrap_gst_object_get_controller, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "object_set_controller", (PyCFunction)_wrap_gst_object_set_controller, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "object_suggest_next_sync", (PyCFunction)_wrap_gst_object_suggest_next_sync, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "object_sync_values", (PyCFunction)_wrap_gst_object_sync_values, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "gst_object_set_control_source", (PyCFunction)_wrap_gst_object_set_control_source, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "gst_object_get_control_source", (PyCFunction)_wrap_gst_object_get_control_source, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "object_get_control_rate", (PyCFunction)_wrap_gst_object_get_control_rate, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "object_set_control_rate", (PyCFunction)_wrap_gst_object_set_control_rate, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "dp_init", (PyCFunction)_wrap_gst_dp_init, METH_NOARGS,
      NULL },
    { "dp_header_payload_length", (PyCFunction)_wrap_gst_dp_header_payload_length, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "dp_header_payload_type", (PyCFunction)_wrap_gst_dp_header_payload_type, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "dp_buffer_from_header", (PyCFunction)_wrap_gst_dp_buffer_from_header, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "dp_caps_from_packet", (PyCFunction)_wrap_gst_dp_caps_from_packet, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "dp_event_from_packet", (PyCFunction)_wrap_gst_dp_event_from_packet, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "dp_validate_header", (PyCFunction)_wrap_gst_dp_validate_header, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "dp_validate_payload", (PyCFunction)_wrap_gst_dp_validate_payload, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "dp_validate_packet", (PyCFunction)_wrap_gst_dp_validate_packet, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "type_find_helper", (PyCFunction)_wrap_gst_type_find_helper, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "type_find_helper_for_buffer", (PyCFunction)_wrap_gst_type_find_helper_for_buffer, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "type_find_helper_for_extension", (PyCFunction)_wrap_gst_type_find_helper_for_extension, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "version_string", (PyCFunction)_wrap_gst_version_string, METH_NOARGS,
      NULL },
    { "segtrap_is_enabled", (PyCFunction)_wrap_gst_segtrap_is_enabled, METH_NOARGS,
      NULL },
    { "segtrap_set_enabled", (PyCFunction)_wrap_gst_segtrap_set_enabled, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "registry_fork_is_enabled", (PyCFunction)_wrap_gst_registry_fork_is_enabled, METH_NOARGS,
      NULL },
    { "registry_fork_set_enabled", (PyCFunction)_wrap_gst_registry_fork_set_enabled, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "update_registry", (PyCFunction)_wrap_gst_update_registry, METH_NOARGS,
      NULL },
    { "get_gst_version", (PyCFunction)_wrap_gst_get_gst_version, METH_NOARGS,
      NULL },
    { "get_pygst_version", (PyCFunction)_wrap_gst_get_pygst_version, METH_NOARGS,
      NULL },
    { "buffer_new_and_alloc", (PyCFunction)_wrap_gst_buffer_new_and_alloc, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "buffer_try_new_and_alloc", (PyCFunction)_wrap_gst_buffer_try_new_and_alloc, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "caps_new_any", (PyCFunction)_wrap_gst_caps_new_any, METH_NOARGS,
      NULL },
    { "caps_from_string", (PyCFunction)_wrap_gst_caps_from_string, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "DEBUG_BIN_TO_DOT_FILE", (PyCFunction)_wrap_GST_DEBUG_BIN_TO_DOT_FILE, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "DEBUG_BIN_TO_DOT_FILE_WITH_TS", (PyCFunction)_wrap_GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "element_factory_find", (PyCFunction)_wrap_gst_element_factory_find, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "element_factory_make", (PyCFunction)_wrap_gst_element_factory_make, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "element_register", (PyCFunction)_wrap_gst_element_register, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "event_new_custom", (PyCFunction)_wrap_gst_event_new_custom, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "event_new_flush_start", (PyCFunction)_wrap_gst_event_new_flush_start, METH_NOARGS,
      NULL },
    { "event_new_flush_stop", (PyCFunction)_wrap_gst_event_new_flush_stop, METH_NOARGS,
      NULL },
    { "event_new_eos", (PyCFunction)_wrap_gst_event_new_eos, METH_NOARGS,
      NULL },
    { "event_new_new_segment", (PyCFunction)_wrap_gst_event_new_new_segment, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "event_new_new_segment_full", (PyCFunction)_wrap_gst_event_new_new_segment_full, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "event_new_tag", (PyCFunction)_wrap_gst_event_new_tag, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "event_new_buffer_size", (PyCFunction)_wrap_gst_event_new_buffer_size, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "event_new_qos", (PyCFunction)_wrap_gst_event_new_qos, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "event_new_seek", (PyCFunction)_wrap_gst_event_new_seek, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "event_new_navigation", (PyCFunction)_wrap_gst_event_new_navigation, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "event_new_latency", (PyCFunction)_wrap_gst_event_new_latency, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "format_register", (PyCFunction)_wrap_gst_format_register, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "format_get_by_nick", (PyCFunction)_wrap_gst_format_get_by_nick, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "format_iterate_definitions", (PyCFunction)_wrap_gst_format_iterate_definitions, METH_NOARGS,
      NULL },
    { "ghost_pad_new_notarget", (PyCFunction)_wrap_gst_ghost_pad_new_no_target, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "ghost_pad_new_from_template", (PyCFunction)_wrap_gst_ghost_pad_new_from_template, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "ghost_pad_new_no_target_from_template", (PyCFunction)_wrap_gst_ghost_pad_new_no_target_from_template, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "index_factory_find", (PyCFunction)_wrap_gst_index_factory_find, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "index_factory_make", (PyCFunction)_wrap_gst_index_factory_make, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "debug_log", (PyCFunction)_wrap_gst_debug_log, METH_VARARGS,
      NULL },
    { "debug_set_active", (PyCFunction)_wrap_gst_debug_set_active, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "debug_is_active", (PyCFunction)_wrap_gst_debug_is_active, METH_NOARGS,
      NULL },
    { "debug_set_colored", (PyCFunction)_wrap_gst_debug_set_colored, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "debug_is_colored", (PyCFunction)_wrap_gst_debug_is_colored, METH_NOARGS,
      NULL },
    { "debug_set_default_threshold", (PyCFunction)_wrap_gst_debug_set_default_threshold, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "debug_get_default_threshold", (PyCFunction)_wrap_gst_debug_get_default_threshold, METH_NOARGS,
      NULL },
    { "debug_set_threshold_for_name", (PyCFunction)_wrap_gst_debug_set_threshold_for_name, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "debug_unset_threshold_for_name", (PyCFunction)_wrap_gst_debug_unset_threshold_for_name, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "debug_construct_term_color", (PyCFunction)_wrap_gst_debug_construct_term_color, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "debug_construct_win_color", (PyCFunction)_wrap_gst_debug_construct_win_color, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "message_new_eos", (PyCFunction)_wrap_gst_message_new_eos, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "message_new_error", (PyCFunction)_wrap_gst_message_new_error, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "message_new_warning", (PyCFunction)_wrap_gst_message_new_warning, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "message_new_info", (PyCFunction)_wrap_gst_message_new_info, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "message_new_tag", (PyCFunction)_wrap_gst_message_new_tag, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "message_new_buffering", (PyCFunction)_wrap_gst_message_new_buffering, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "message_new_state_changed", (PyCFunction)_wrap_gst_message_new_state_changed, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "message_new_state_dirty", (PyCFunction)_wrap_gst_message_new_state_dirty, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "message_new_clock_provide", (PyCFunction)_wrap_gst_message_new_clock_provide, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "message_new_clock_lost", (PyCFunction)_wrap_gst_message_new_clock_lost, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "message_new_new_clock", (PyCFunction)_wrap_gst_message_new_new_clock, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "message_new_segment_start", (PyCFunction)_wrap_gst_message_new_segment_start, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "message_new_segment_done", (PyCFunction)_wrap_gst_message_new_segment_done, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "message_new_application", (PyCFunction)_wrap_gst_message_new_application, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "message_new_element", (PyCFunction)_wrap_gst_message_new_element, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "message_new_duration", (PyCFunction)_wrap_gst_message_new_duration, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "message_new_async_start", (PyCFunction)_wrap_gst_message_new_async_start, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "message_new_async_done", (PyCFunction)_wrap_gst_message_new_async_done, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "message_new_latency", (PyCFunction)_wrap_gst_message_new_latency, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "message_new_structure_change", (PyCFunction)_wrap_gst_message_new_structure_change, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "message_new_request_state", (PyCFunction)_wrap_gst_message_new_request_state, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "message_new_custom", (PyCFunction)_wrap_gst_message_new_custom, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "flow_get_name", (PyCFunction)_wrap_gst_flow_get_name, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "pad_new_from_static_template", (PyCFunction)_wrap_gst_pad_new_from_static_template, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "parse_bin_from_description", (PyCFunction)_wrap_gst_parse_bin_from_description, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "util_get_timestamp", (PyCFunction)_wrap_gst_util_get_timestamp, METH_NOARGS,
      NULL },
    { "parse_launch", (PyCFunction)_wrap_gst_parse_launch, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "plugin_load_file", (PyCFunction)_wrap_gst_plugin_load_file, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "plugin_load_by_name", (PyCFunction)_wrap_gst_plugin_load_by_name, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "query_type_register", (PyCFunction)_wrap_gst_query_type_register, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "query_type_get_by_nick", (PyCFunction)_wrap_gst_query_type_get_by_nick, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "query_type_iterate_definitions", (PyCFunction)_wrap_gst_query_type_iterate_definitions, METH_NOARGS,
      NULL },
    { "query_new_position", (PyCFunction)_wrap_gst_query_new_position, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "query_new_duration", (PyCFunction)_wrap_gst_query_new_duration, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "query_new_latency", (PyCFunction)_wrap_gst_query_new_latency, METH_NOARGS,
      NULL },
    { "query_new_convert", (PyCFunction)_wrap_gst_query_new_convert, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "query_new_segment", (PyCFunction)_wrap_gst_query_new_segment, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "query_new_application", (PyCFunction)_wrap_gst_query_new_application, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "query_new_seeking", (PyCFunction)_wrap_gst_query_new_seeking, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "query_new_formats", (PyCFunction)_wrap_gst_query_new_formats, METH_NOARGS,
      NULL },
    { "query_new_buffering", (PyCFunction)_wrap_gst_query_new_buffering, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "query_new_uri", (PyCFunction)_wrap_gst_query_new_uri, METH_NOARGS,
      NULL },
    { "registry_get_default", (PyCFunction)_wrap_gst_registry_get_default, METH_NOARGS,
      NULL },
    { "default_registry_check_feature_version", (PyCFunction)_wrap_gst_default_registry_check_feature_version, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "structure_from_string", (PyCFunction)_wrap_gst_structure_from_string, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "system_clock_obtain", (PyCFunction)_wrap_gst_system_clock_obtain, METH_NOARGS,
      NULL },
    { "tag_exists", (PyCFunction)_wrap_gst_tag_exists, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "tag_get_tag_type", (PyCFunction)_wrap_gst_tag_get_tag_type, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "tag_get_nick", (PyCFunction)_wrap_gst_tag_get_nick, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "tag_get_description", (PyCFunction)_wrap_gst_tag_get_description, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "tag_get_flag", (PyCFunction)_wrap_gst_tag_get_flag, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "tag_is_fixed", (PyCFunction)_wrap_gst_tag_is_fixed, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "alloc_trace_available", (PyCFunction)_wrap_gst_alloc_trace_available, METH_NOARGS,
      NULL },
    { "alloc_trace_live_all", (PyCFunction)_wrap_gst_alloc_trace_live_all, METH_NOARGS,
      NULL },
    { "alloc_trace_print_all", (PyCFunction)_wrap_gst_alloc_trace_print_all, METH_NOARGS,
      NULL },
    { "alloc_trace_print_live", (PyCFunction)_wrap_gst_alloc_trace_print_live, METH_NOARGS,
      NULL },
    { "alloc_trace_set_flags_all", (PyCFunction)_wrap_gst_alloc_trace_set_flags_all, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "type_find_new", (PyCFunction)_wrap_gst_type_find_new, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "type_find_register", (PyCFunction)_wrap_gst_type_find_register, METH_VARARGS,
      NULL },
    { "type_find_factory_get_list", (PyCFunction)_wrap_gst_type_find_factory_get_list, METH_NOARGS,
      NULL },
    { "uri_protocol_is_valid", (PyCFunction)_wrap_gst_uri_protocol_is_valid, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "uri_protocol_is_supported", (PyCFunction)_wrap_gst_uri_protocol_is_supported, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "uri_is_valid", (PyCFunction)_wrap_gst_uri_is_valid, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "uri_get_protocol", (PyCFunction)_wrap_gst_uri_get_protocol, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "uri_has_protocol", (PyCFunction)_wrap_gst_uri_has_protocol, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "uri_get_location", (PyCFunction)_wrap_gst_uri_get_location, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "uri_construct", (PyCFunction)_wrap_gst_uri_construct, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "element_make_from_uri", (PyCFunction)_wrap_gst_element_make_from_uri, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "util_set_object_arg", (PyCFunction)_wrap_gst_util_set_object_arg, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "util_dump_mem", (PyCFunction)_wrap_gst_util_dump_mem, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "util_gdouble_to_guint64", (PyCFunction)_wrap_gst_util_gdouble_to_guint64, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "util_guint64_to_gdouble", (PyCFunction)_wrap_gst_util_guint64_to_gdouble, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "util_uint64_scale", (PyCFunction)_wrap_gst_util_uint64_scale, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "util_uint64_scale_int", (PyCFunction)_wrap_gst_util_uint64_scale_int, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "util_seqnum_next", (PyCFunction)_wrap_gst_util_seqnum_next, METH_NOARGS,
      NULL },
    { "util_seqnum_compare", (PyCFunction)_wrap_gst_util_seqnum_compare, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "element_state_get_name", (PyCFunction)_wrap_gst_element_state_get_name, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "state_change_return_get_name", (PyCFunction)_wrap_gst_element_state_change_return_get_name, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "element_link_many", (PyCFunction)_wrap_gst_element_link_many, METH_VARARGS,
      NULL },
    { "element_unlink_many", (PyCFunction)_wrap_gst_element_unlink_many, METH_VARARGS,
      NULL },
    { "TIME_ARGS", (PyCFunction)_wrap_GST_TIME_ARGS, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};


/* ----------- enums and flags ----------- */

void
pygst_add_constants(PyObject *module, const gchar *strip_prefix)
{
  pyg_flags_add(module, "BinFlags", strip_prefix, GST_TYPE_BIN_FLAGS);
  pyg_flags_add(module, "BufferFlag", strip_prefix, GST_TYPE_BUFFER_FLAG);
  pyg_flags_add(module, "BufferCopyFlags", strip_prefix, GST_TYPE_BUFFER_COPY_FLAGS);
  pyg_flags_add(module, "BusFlags", strip_prefix, GST_TYPE_BUS_FLAGS);
  pyg_enum_add(module, "BusSyncReply", strip_prefix, GST_TYPE_BUS_SYNC_REPLY);
  pyg_flags_add(module, "CapsFlags", strip_prefix, GST_TYPE_CAPS_FLAGS);
  pyg_enum_add(module, "ClockReturn", strip_prefix, GST_TYPE_CLOCK_RETURN);
  pyg_enum_add(module, "ClockEntryType", strip_prefix, GST_TYPE_CLOCK_ENTRY_TYPE);
  pyg_flags_add(module, "ClockFlags", strip_prefix, GST_TYPE_CLOCK_FLAGS);
  pyg_flags_add(module, "DebugGraphDetails", strip_prefix, GST_TYPE_DEBUG_GRAPH_DETAILS);
  pyg_flags_add(module, "ElementFlags", strip_prefix, GST_TYPE_ELEMENT_FLAGS);
    PyModule_AddIntConstant(module, (char *) pyg_constant_strip_prefix("GST_PARAM_CONTROLLABLE", strip_prefix), GST_PARAM_CONTROLLABLE);
    PyModule_AddIntConstant(module, (char *) pyg_constant_strip_prefix("GST_PARAM_MUTABLE_READY", strip_prefix), GST_PARAM_MUTABLE_READY);
    PyModule_AddIntConstant(module, (char *) pyg_constant_strip_prefix("GST_PARAM_MUTABLE_PAUSED", strip_prefix), GST_PARAM_MUTABLE_PAUSED);
    PyModule_AddIntConstant(module, (char *) pyg_constant_strip_prefix("GST_PARAM_MUTABLE_PLAYING", strip_prefix), GST_PARAM_MUTABLE_PLAYING);
    PyModule_AddIntConstant(module, (char *) pyg_constant_strip_prefix("GST_PARAM_USER_SHIFT", strip_prefix), GST_PARAM_USER_SHIFT);
  pyg_enum_add(module, "CoreError", strip_prefix, GST_TYPE_CORE_ERROR);
  pyg_enum_add(module, "LibraryError", strip_prefix, GST_TYPE_LIBRARY_ERROR);
  pyg_enum_add(module, "ResourceError", strip_prefix, GST_TYPE_RESOURCE_ERROR);
  pyg_enum_add(module, "StreamError", strip_prefix, GST_TYPE_STREAM_ERROR);
  pyg_flags_add(module, "EventTypeFlags", strip_prefix, GST_TYPE_EVENT_TYPE_FLAGS);
  pyg_enum_add(module, "EventType", strip_prefix, GST_TYPE_EVENT_TYPE);
  pyg_enum_add(module, "SeekType", strip_prefix, GST_TYPE_SEEK_TYPE);
  pyg_flags_add(module, "SeekFlags", strip_prefix, GST_TYPE_SEEK_FLAGS);
  pyg_enum_add(module, "Format", strip_prefix, GST_TYPE_FORMAT);
  pyg_enum_add(module, "IndexCertainty", strip_prefix, GST_TYPE_INDEX_CERTAINTY);
  pyg_enum_add(module, "IndexEntryType", strip_prefix, GST_TYPE_INDEX_ENTRY_TYPE);
  pyg_enum_add(module, "IndexLookupMethod", strip_prefix, GST_TYPE_INDEX_LOOKUP_METHOD);
  pyg_flags_add(module, "AssocFlags", strip_prefix, GST_TYPE_ASSOC_FLAGS);
  pyg_enum_add(module, "IndexResolverMethod", strip_prefix, GST_TYPE_INDEX_RESOLVER_METHOD);
  pyg_flags_add(module, "IndexFlags", strip_prefix, GST_TYPE_INDEX_FLAGS);
  pyg_enum_add(module, "DebugLevel", strip_prefix, GST_TYPE_DEBUG_LEVEL);
  pyg_enum_add(module, "DebugColorFlags", strip_prefix, GST_TYPE_DEBUG_COLOR_FLAGS);
  pyg_enum_add(module, "IteratorResult", strip_prefix, GST_TYPE_ITERATOR_RESULT);
  pyg_enum_add(module, "IteratorItem", strip_prefix, GST_TYPE_ITERATOR_ITEM);
  pyg_flags_add(module, "MessageType", strip_prefix, GST_TYPE_MESSAGE_TYPE);
  pyg_enum_add(module, "StructureChangeType", strip_prefix, GST_TYPE_STRUCTURE_CHANGE_TYPE);
  pyg_flags_add(module, "MiniObjectFlags", strip_prefix, GST_TYPE_MINI_OBJECT_FLAGS);
  pyg_flags_add(module, "ObjectFlags", strip_prefix, GST_TYPE_OBJECT_FLAGS);
  pyg_enum_add(module, "PadLinkReturn", strip_prefix, GST_TYPE_PAD_LINK_RETURN);
  pyg_enum_add(module, "FlowReturn", strip_prefix, GST_TYPE_FLOW_RETURN);
  pyg_enum_add(module, "ActivateMode", strip_prefix, GST_TYPE_ACTIVATE_MODE);
  pyg_enum_add(module, "PadDirection", strip_prefix, GST_TYPE_PAD_DIRECTION);
  pyg_flags_add(module, "PadFlags", strip_prefix, GST_TYPE_PAD_FLAGS);
  pyg_enum_add(module, "PadPresence", strip_prefix, GST_TYPE_PAD_PRESENCE);
  pyg_flags_add(module, "PadTemplateFlags", strip_prefix, GST_TYPE_PAD_TEMPLATE_FLAGS);
  pyg_enum_add(module, "ParseError", strip_prefix, GST_TYPE_PARSE_ERROR);
  pyg_flags_add(module, "ParseFlags", strip_prefix, GST_TYPE_PARSE_FLAGS);
  pyg_flags_add(module, "PipelineFlags", strip_prefix, GST_TYPE_PIPELINE_FLAGS);
  pyg_enum_add(module, "PluginError", strip_prefix, GST_TYPE_PLUGIN_ERROR);
  pyg_flags_add(module, "PluginFlags", strip_prefix, GST_TYPE_PLUGIN_FLAGS);
  pyg_enum_add(module, "QueryType", strip_prefix, GST_TYPE_QUERY_TYPE);
  pyg_enum_add(module, "BufferingMode", strip_prefix, GST_TYPE_BUFFERING_MODE);
  pyg_enum_add(module, "TagMergeMode", strip_prefix, GST_TYPE_TAG_MERGE_MODE);
  pyg_enum_add(module, "TagFlag", strip_prefix, GST_TYPE_TAG_FLAG);
  pyg_enum_add(module, "TaskState", strip_prefix, GST_TYPE_TASK_STATE);
  pyg_flags_add(module, "AllocTraceFlags", strip_prefix, GST_TYPE_ALLOC_TRACE_FLAGS);
  pyg_enum_add(module, "TypeFindProbability", strip_prefix, GST_TYPE_TYPE_FIND_PROBABILITY);
  pyg_enum_add(module, "State", strip_prefix, GST_TYPE_STATE);
  pyg_enum_add(module, "StateChangeReturn", strip_prefix, GST_TYPE_STATE_CHANGE_RETURN);
  pyg_enum_add(module, "StateChange", strip_prefix, GST_TYPE_STATE_CHANGE);
  pyg_enum_add(module, "Rank", strip_prefix, GST_TYPE_RANK);
  pyg_enum_add(module, "URIType", strip_prefix, GST_TYPE_URI_TYPE);
  pyg_enum_add(module, "SearchMode", strip_prefix, GST_TYPE_SEARCH_MODE);
    PyModule_AddIntConstant(module, (char *) pyg_constant_strip_prefix("GST_INTERPOLATE_NONE", strip_prefix), GST_INTERPOLATE_NONE);
    PyModule_AddIntConstant(module, (char *) pyg_constant_strip_prefix("GST_INTERPOLATE_TRIGGER", strip_prefix), GST_INTERPOLATE_TRIGGER);
    PyModule_AddIntConstant(module, (char *) pyg_constant_strip_prefix("GST_INTERPOLATE_LINEAR", strip_prefix), GST_INTERPOLATE_LINEAR);
    PyModule_AddIntConstant(module, (char *) pyg_constant_strip_prefix("GST_INTERPOLATE_QUADRATIC", strip_prefix), GST_INTERPOLATE_QUADRATIC);
    PyModule_AddIntConstant(module, (char *) pyg_constant_strip_prefix("GST_INTERPOLATE_CUBIC", strip_prefix), GST_INTERPOLATE_CUBIC);
    PyModule_AddIntConstant(module, (char *) pyg_constant_strip_prefix("GST_INTERPOLATE_USER", strip_prefix), GST_INTERPOLATE_USER);
  pyg_enum_add(module, "LFOWaveform", strip_prefix, GST_TYPE_LFO_WAVEFORM);
  pyg_enum_add(module, "DPVersion", strip_prefix, GST_TYPE_DP_VERSION);
    PyModule_AddIntConstant(module, (char *) pyg_constant_strip_prefix("GST_DP_HEADER_FLAG_NONE", strip_prefix), GST_DP_HEADER_FLAG_NONE);
    PyModule_AddIntConstant(module, (char *) pyg_constant_strip_prefix("GST_DP_HEADER_FLAG_CRC_HEADER", strip_prefix), GST_DP_HEADER_FLAG_CRC_HEADER);
    PyModule_AddIntConstant(module, (char *) pyg_constant_strip_prefix("GST_DP_HEADER_FLAG_CRC_PAYLOAD", strip_prefix), GST_DP_HEADER_FLAG_CRC_PAYLOAD);
    PyModule_AddIntConstant(module, (char *) pyg_constant_strip_prefix("GST_DP_HEADER_FLAG_CRC", strip_prefix), GST_DP_HEADER_FLAG_CRC);
    PyModule_AddIntConstant(module, (char *) pyg_constant_strip_prefix("GST_DP_PAYLOAD_NONE", strip_prefix), GST_DP_PAYLOAD_NONE);
    PyModule_AddIntConstant(module, (char *) pyg_constant_strip_prefix("GST_DP_PAYLOAD_BUFFER", strip_prefix), GST_DP_PAYLOAD_BUFFER);
    PyModule_AddIntConstant(module, (char *) pyg_constant_strip_prefix("GST_DP_PAYLOAD_CAPS", strip_prefix), GST_DP_PAYLOAD_CAPS);
    PyModule_AddIntConstant(module, (char *) pyg_constant_strip_prefix("GST_DP_PAYLOAD_EVENT_NONE", strip_prefix), GST_DP_PAYLOAD_EVENT_NONE);

  if (PyErr_Occurred())
    PyErr_Print();
}

/* initialise stuff extension classes */
void
pygst_register_classes(PyObject *d)
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


#line 72 "gstcaps.override"
  structure_caps_map = g_hash_table_new (g_direct_hash, g_direct_equal);

#line 270 "..\\..\\Source\\gst-python\\gst\\gst.override"
{
    pyg_register_class_init (GST_TYPE_ELEMENT, _pygst_element_init);
    if (!pygst_value_init())
        return;
    gst_controller_init(NULL, NULL);
}

#line 26448 "..\\..\\Source\\gst-python\\gst\\gst.c"
    pyg_register_boxed(d, "Caps", GST_TYPE_CAPS, &PyGstCaps_Type);
    pyg_register_boxed(d, "Structure", GST_TYPE_STRUCTURE, &PyGstStructure_Type);
    pyg_register_boxed(d, "TagList", GST_TYPE_TAG_LIST, &PyGstTagList_Type);
    pyg_register_boxed(d, "GError", GST_TYPE_G_ERROR, &PyGError_Type);
    pyg_register_boxed(d, "Date", GST_TYPE_DATE, &PyGstDate_Type);
    pyg_register_boxed(d, "Segment", GST_TYPE_SEGMENT, &PyGstSegment_Type);
    pyg_register_boxed(d, "IndexEntry", GST_TYPE_INDEX_ENTRY, &PyGstIndexEntry_Type);
    pyg_register_pointer(d, "StaticPadTemplate", GST_TYPE_STATIC_PAD_TEMPLATE, &PyGstStaticPadTemplate_Type);
    pyg_register_pointer(d, "StaticCaps", GST_TYPE_STATIC_CAPS, &PyGstStaticCaps_Type);
    pyg_register_pointer(d, "TypeFind", GST_TYPE_TYPE_FIND, &PyGstTypeFind_Type);
    pyg_register_interface(d, "URIHandler", GST_TYPE_URI_HANDLER, &PyGstURIHandler_Type);
    pyg_register_interface_info(GST_TYPE_URI_HANDLER, &__GstURIHandler__iinfo);
    pyg_register_interface(d, "TagSetter", GST_TYPE_TAG_SETTER, &PyGstTagSetter_Type);
    pyg_register_interface(d, "ImplementsInterface", GST_TYPE_IMPLEMENTS_INTERFACE, &PyGstImplementsInterface_Type);
    pyg_register_interface_info(GST_TYPE_IMPLEMENTS_INTERFACE, &__GstImplementsInterface__iinfo);
    pyg_register_interface(d, "Preset", GST_TYPE_PRESET, &PyGstPreset_Type);
    pygobject_register_class(d, "GstObject", GST_TYPE_OBJECT, &PyGstObject_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_set_object_has_new_constructor(GST_TYPE_OBJECT);
    pygobject_register_class(d, "GstIndex", GST_TYPE_INDEX, &PyGstIndex_Type, Py_BuildValue("(O)", &PyGstObject_Type));
    pyg_set_object_has_new_constructor(GST_TYPE_INDEX);
    pyg_register_class_init(GST_TYPE_INDEX, __GstIndex_class_init);
    pygobject_register_class(d, "GstElement", GST_TYPE_ELEMENT, &PyGstElement_Type, Py_BuildValue("(O)", &PyGstObject_Type));
    pyg_set_object_has_new_constructor(GST_TYPE_ELEMENT);
    pyg_register_class_init(GST_TYPE_ELEMENT, __GstElement_class_init);
    pygobject_register_class(d, "GstBin", GST_TYPE_BIN, &PyGstBin_Type, Py_BuildValue("(O)", &PyGstElement_Type));
    pyg_set_object_has_new_constructor(GST_TYPE_BIN);
    pyg_register_class_init(GST_TYPE_BIN, __GstBin_class_init);
    pygobject_register_class(d, "GstClock", GST_TYPE_CLOCK, &PyGstClock_Type, Py_BuildValue("(O)", &PyGstObject_Type));
    pyg_set_object_has_new_constructor(GST_TYPE_CLOCK);
    pyg_register_class_init(GST_TYPE_CLOCK, __GstClock_class_init);
    pygobject_register_class(d, "GstBus", GST_TYPE_BUS, &PyGstBus_Type, Py_BuildValue("(O)", &PyGstObject_Type));
    pyg_set_object_has_new_constructor(GST_TYPE_BUS);
    pygobject_register_class(d, "GstPad", GST_TYPE_PAD, &PyGstPad_Type, Py_BuildValue("(O)", &PyGstObject_Type));
    pyg_set_object_has_new_constructor(GST_TYPE_PAD);
    pygobject_register_class(d, "GstGhostPad", GST_TYPE_GHOST_PAD, &PyGstGhostPad_Type, Py_BuildValue("(O)", &PyGstPad_Type));
    pygobject_register_class(d, "GstPadTemplate", GST_TYPE_PAD_TEMPLATE, &PyGstPadTemplate_Type, Py_BuildValue("(O)", &PyGstObject_Type));
    pygobject_register_class(d, "GstPipeline", GST_TYPE_PIPELINE, &PyGstPipeline_Type, Py_BuildValue("(O)", &PyGstBin_Type));
    pyg_set_object_has_new_constructor(GST_TYPE_PIPELINE);
    pygobject_register_class(d, "GstPluginFeature", GST_TYPE_PLUGIN_FEATURE, &PyGstPluginFeature_Type, Py_BuildValue("(O)", &PyGstObject_Type));
    pyg_set_object_has_new_constructor(GST_TYPE_PLUGIN_FEATURE);
    pygobject_register_class(d, "GstIndexFactory", GST_TYPE_INDEX_FACTORY, &PyGstIndexFactory_Type, Py_BuildValue("(O)", &PyGstPluginFeature_Type));
    pyg_set_object_has_new_constructor(GST_TYPE_INDEX_FACTORY);
    pygobject_register_class(d, "GstElementFactory", GST_TYPE_ELEMENT_FACTORY, &PyGstElementFactory_Type, Py_BuildValue("(O)", &PyGstPluginFeature_Type));
    pyg_set_object_has_new_constructor(GST_TYPE_ELEMENT_FACTORY);
    pygobject_register_class(d, "GstRegistry", GST_TYPE_REGISTRY, &PyGstRegistry_Type, Py_BuildValue("(O)", &PyGstObject_Type));
    pyg_set_object_has_new_constructor(GST_TYPE_REGISTRY);
    pygobject_register_class(d, "GstSystemClock", GST_TYPE_SYSTEM_CLOCK, &PyGstSystemClock_Type, Py_BuildValue("(O)", &PyGstClock_Type));
    pyg_set_object_has_new_constructor(GST_TYPE_SYSTEM_CLOCK);
    pygobject_register_class(d, "GstTask", GST_TYPE_TASK, &PyGstTask_Type, Py_BuildValue("(O)", &PyGstObject_Type));
    pyg_set_object_has_new_constructor(GST_TYPE_TASK);
    pygobject_register_class(d, "GstTypeFindFactory", GST_TYPE_TYPE_FIND_FACTORY, &PyGstTypeFindFactory_Type, Py_BuildValue("(O)", &PyGstPluginFeature_Type));
    pyg_set_object_has_new_constructor(GST_TYPE_TYPE_FIND_FACTORY);
    pygobject_register_class(d, "GstNetClientClock", GST_TYPE_NET_CLIENT_CLOCK, &PyGstNetClientClock_Type, Py_BuildValue("(O)", &PyGstSystemClock_Type));
    pygobject_register_class(d, "GstNetTimeProvider", GST_TYPE_NET_TIME_PROVIDER, &PyGstNetTimeProvider_Type, Py_BuildValue("(O)", &PyGstObject_Type));
    pygobject_register_class(d, "GstPlugin", GST_TYPE_PLUGIN, &PyGstPlugin_Type, Py_BuildValue("(O)", &PyGstObject_Type));
    pyg_set_object_has_new_constructor(GST_TYPE_PLUGIN);
    pygobject_register_class(d, "GstControlSource", GST_TYPE_CONTROL_SOURCE, &PyGstControlSource_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_set_object_has_new_constructor(GST_TYPE_CONTROL_SOURCE);
    pygobject_register_class(d, "GstController", GST_TYPE_CONTROLLER, &PyGstController_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pygobject_register_class(d, "GstInterpolationControlSource", GST_TYPE_INTERPOLATION_CONTROL_SOURCE, &PyGstInterpolationControlSource_Type, Py_BuildValue("(O)", &PyGstControlSource_Type));
    pyg_set_object_has_new_constructor(GST_TYPE_INTERPOLATION_CONTROL_SOURCE);
    pygobject_register_class(d, "GstLFOControlSource", GST_TYPE_LFO_CONTROL_SOURCE, &PyGstLFOControlSource_Type, Py_BuildValue("(O)", &PyGstControlSource_Type));
    pyg_set_object_has_new_constructor(GST_TYPE_LFO_CONTROL_SOURCE);
    pygobject_register_class(d, "GstAdapter", GST_TYPE_ADAPTER, &PyGstAdapter_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_set_object_has_new_constructor(GST_TYPE_ADAPTER);
    pygobject_register_class(d, "GstBaseSink", GST_TYPE_BASE_SINK, &PyGstBaseSink_Type, Py_BuildValue("(O)", &PyGstElement_Type));
    pyg_set_object_has_new_constructor(GST_TYPE_BASE_SINK);
    pyg_register_class_init(GST_TYPE_BASE_SINK, __GstBaseSink_class_init);
    pygobject_register_class(d, "GstBaseSrc", GST_TYPE_BASE_SRC, &PyGstBaseSrc_Type, Py_BuildValue("(O)", &PyGstElement_Type));
    pyg_set_object_has_new_constructor(GST_TYPE_BASE_SRC);
    pyg_register_class_init(GST_TYPE_BASE_SRC, __GstBaseSrc_class_init);
    pygobject_register_class(d, "GstBaseTransform", GST_TYPE_BASE_TRANSFORM, &PyGstBaseTransform_Type, Py_BuildValue("(O)", &PyGstElement_Type));
    pyg_set_object_has_new_constructor(GST_TYPE_BASE_TRANSFORM);
    pyg_register_class_init(GST_TYPE_BASE_TRANSFORM, __GstBaseTransform_class_init);
    pygobject_register_class(d, "GstCollectPads", GST_TYPE_COLLECT_PADS, &PyGstCollectPads_Type, Py_BuildValue("(O)", &PyGstObject_Type));
    pyg_set_object_has_new_constructor(GST_TYPE_COLLECT_PADS);
    pygobject_register_class(d, "GstDataQueue", GST_TYPE_DATA_QUEUE, &PyGstDataQueue_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pygstminiobject_register_class(d, "GstBuffer", GST_TYPE_BUFFER, &PyGstBuffer_Type, Py_BuildValue("(O)", &PyGstMiniObject_Type));
    pygstminiobject_register_class(d, "GstEvent", GST_TYPE_EVENT, &PyGstEvent_Type, Py_BuildValue("(O)", &PyGstMiniObject_Type));
    pygstminiobject_register_class(d, "GstMessage", GST_TYPE_MESSAGE, &PyGstMessage_Type, Py_BuildValue("(O)", &PyGstMiniObject_Type));
    pygstminiobject_register_class(d, "GstQuery", GST_TYPE_QUERY, &PyGstQuery_Type, Py_BuildValue("(O)", &PyGstMiniObject_Type));
}
