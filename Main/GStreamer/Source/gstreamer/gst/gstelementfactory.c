/* GStreamer
 * Copyright (C) 1999,2000 Erik Walthinsen <omega@cse.ogi.edu>
 *                    2000 Wim Taymans <wtay@chello.be>
 *                    2003 Benjamin Otte <in7y118@public.uni-hamburg.de>
 *
 * gstelementfactory.c: GstElementFactory object, support routines
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/**
 * SECTION:gstelementfactory
 * @short_description: Create GstElements from a factory
 * @see_also: #GstElement, #GstPlugin, #GstPluginFeature, #GstPadTemplate.
 *
 * #GstElementFactory is used to create instances of elements. A
 * GstElementfactory can be added to a #GstPlugin as it is also a
 * #GstPluginFeature.
 *
 * Use the gst_element_factory_find() and gst_element_factory_create()
 * functions to create element instances or use gst_element_factory_make() as a
 * convenient shortcut.
 *
 * The following code example shows you how to create a GstFileSrc element.
 *
 * <example>
 * <title>Using an element factory</title>
 * <programlisting language="c">
 *   #include &lt;gst/gst.h&gt;
 *   
 *   GstElement *src;
 *   GstElementFactory *srcfactory;
 *   
 *   gst_init (&amp;argc, &amp;argv);
 *   
 *   srcfactory = gst_element_factory_find ("filesrc");
 *   g_return_if_fail (srcfactory != NULL);
 *   src = gst_element_factory_create (srcfactory, "src");
 *   g_return_if_fail (src != NULL);
 *   ...
 * </programlisting>
 * </example>
 *
 * Last reviewed on 2005-11-23 (0.9.5)
 */

#include "gst_private.h"

#include "gstelement.h"
#include "gstinfo.h"
#include "gsturi.h"
#include "gstregistry.h"

#include "glib-compat-private.h"

GST_DEBUG_CATEGORY_STATIC (element_factory_debug);
#define GST_CAT_DEFAULT element_factory_debug

static void gst_element_factory_class_init (GstElementFactoryClass * klass);
static void gst_element_factory_init (GstElementFactory * factory);
static void gst_element_factory_finalize (GObject * object);
void __gst_element_details_clear (GstElementDetails * dp);
static void gst_element_factory_cleanup (GstElementFactory * factory);

static GstPluginFeatureClass *parent_class = NULL;

/* static guint gst_element_factory_signals[LAST_SIGNAL] = { 0 }; */

#define _do_init \
{ \
  GST_DEBUG_CATEGORY_INIT (element_factory_debug, "GST_ELEMENT_FACTORY", \
      GST_DEBUG_BOLD | GST_DEBUG_FG_WHITE | GST_DEBUG_BG_RED, \
      "element factories keep information about installed elements"); \
}

G_DEFINE_TYPE_WITH_CODE (GstElementFactory, gst_element_factory,
    GST_TYPE_PLUGIN_FEATURE, _do_init);

static void
gst_element_factory_class_init (GstElementFactoryClass * klass)
{
  GObjectClass *gobject_class = (GObjectClass *) klass;

  parent_class = g_type_class_peek_parent (klass);

  gobject_class->finalize = GST_DEBUG_FUNCPTR (gst_element_factory_finalize);
}

static void
gst_element_factory_init (GstElementFactory * factory)
{
  factory->staticpadtemplates = NULL;
  factory->numpadtemplates = 0;

  factory->uri_type = GST_URI_UNKNOWN;
  factory->uri_protocols = NULL;

  factory->interfaces = NULL;
}

static void
gst_element_factory_finalize (GObject * object)
{
  GstElementFactory *factory = GST_ELEMENT_FACTORY (object);

  gst_element_factory_cleanup (factory);
  G_OBJECT_CLASS (parent_class)->finalize (object);
}

/**
 * gst_element_factory_find:
 * @name: name of factory to find
 *
 * Search for an element factory of the given name. Refs the returned
 * element factory; caller is responsible for unreffing.
 *
 * Returns: #GstElementFactory if found, NULL otherwise
 */
GstElementFactory *
gst_element_factory_find (const gchar * name)
{
  GstPluginFeature *feature;

  g_return_val_if_fail (name != NULL, NULL);

  feature = gst_registry_find_feature (gst_registry_get_default (), name,
      GST_TYPE_ELEMENT_FACTORY);
  if (feature)
    return GST_ELEMENT_FACTORY (feature);

  /* this isn't an error, for instance when you query if an element factory is
   * present */
  GST_LOG ("no such element factory \"%s\"", name);
  return NULL;
}

void
__gst_element_details_clear (GstElementDetails * dp)
{
  g_free (dp->longname);
  g_free (dp->klass);
  g_free (dp->description);
  g_free (dp->author);
  memset (dp, 0, sizeof (GstElementDetails));
}

#define VALIDATE_SET(__dest, __src, __entry)                            \
G_STMT_START {                                                          \
  if (g_utf8_validate (__src->__entry, -1, NULL)) {                     \
    __dest->__entry = g_strdup (__src->__entry);                        \
  } else {                                                              \
    g_warning ("Invalid UTF-8 in " G_STRINGIFY (__entry) ": %s",        \
        __src->__entry);                                                \
    __dest->__entry = g_strdup ("[ERROR: invalid UTF-8]");              \
  }                                                                     \
} G_STMT_END

void
__gst_element_details_set (GstElementDetails * dest,
    const GstElementDetails * src)
{
  VALIDATE_SET (dest, src, longname);
  VALIDATE_SET (dest, src, klass);
  VALIDATE_SET (dest, src, description);
  VALIDATE_SET (dest, src, author);
}

void
__gst_element_details_copy (GstElementDetails * dest,
    const GstElementDetails * src)
{
  __gst_element_details_clear (dest);
  __gst_element_details_set (dest, src);
}

static void
gst_element_factory_cleanup (GstElementFactory * factory)
{
  GList *item;

  __gst_element_details_clear (&factory->details);
  if (factory->type) {
    g_type_class_unref (g_type_class_peek (factory->type));
    factory->type = 0;
  }

  for (item = factory->staticpadtemplates; item; item = item->next) {
    GstStaticPadTemplate *templ = item->data;
    GstCaps *caps = (GstCaps *) & (templ->static_caps);

    g_free ((gchar *) templ->static_caps.string);

    /* FIXME: this is not threadsafe */
    if (caps->refcount == 1) {
      GstStructure *structure;
      guint i;

      for (i = 0; i < caps->structs->len; i++) {
        structure = (GstStructure *) gst_caps_get_structure (caps, i);
        gst_structure_set_parent_refcount (structure, NULL);
        gst_structure_free (structure);
      }
      g_ptr_array_free (caps->structs, TRUE);
      caps->refcount = 0;
    }
    g_free (templ);
  }
  g_list_free (factory->staticpadtemplates);
  factory->staticpadtemplates = NULL;
  factory->numpadtemplates = 0;
  factory->uri_type = GST_URI_UNKNOWN;
  if (factory->uri_protocols) {
    g_strfreev (factory->uri_protocols);
    factory->uri_protocols = NULL;
  }

  g_list_foreach (factory->interfaces, (GFunc) g_free, NULL);
  g_list_free (factory->interfaces);
  factory->interfaces = NULL;
}

/**
 * gst_element_register:
 * @plugin: #GstPlugin to register the element with, or NULL for a static
 * element (note that passing NULL only works in GStreamer 0.10.13 and later)
 * @name: name of elements of this type
 * @rank: rank of element (higher rank means more importance when autoplugging)
 * @type: GType of element to register
 *
 * Create a new elementfactory capable of instantiating objects of the
 * @type and add the factory to @plugin.
 *
 * Returns: TRUE, if the registering succeeded, FALSE on error
 */
gboolean
gst_element_register (GstPlugin * plugin, const gchar * name, guint rank,
    GType type)
{
  GstElementFactory *factory;
  GType *interfaces;
  guint n_interfaces, i;
  GstElementClass *klass;
  GList *item;

  g_return_val_if_fail (name != NULL, FALSE);
  g_return_val_if_fail (g_type_is_a (type, GST_TYPE_ELEMENT), FALSE);

  factory = GST_ELEMENT_FACTORY (g_object_new (GST_TYPE_ELEMENT_FACTORY, NULL));
  gst_plugin_feature_set_name (GST_PLUGIN_FEATURE (factory), name);
  GST_LOG_OBJECT (factory, "Created new elementfactory for type %s",
      g_type_name (type));

  klass = GST_ELEMENT_CLASS (g_type_class_ref (type));
  if ((klass->details.longname == NULL) ||
      (klass->details.klass == NULL) || (klass->details.author == NULL))
    goto detailserror;

  factory->type = type;
  __gst_element_details_copy (&factory->details, &klass->details);
  for (item = klass->padtemplates; item; item = item->next) {
    GstPadTemplate *templ = item->data;
    GstStaticPadTemplate *newt;

    newt = g_new0 (GstStaticPadTemplate, 1);
    newt->name_template = g_intern_string (templ->name_template);
    newt->direction = templ->direction;
    newt->presence = templ->presence;
    newt->static_caps.string = gst_caps_to_string (templ->caps);
    factory->staticpadtemplates =
        g_list_append (factory->staticpadtemplates, newt);
  }
  factory->numpadtemplates = klass->numpadtemplates;
  klass->elementfactory = factory;

  /* special stuff for URI handling */
  if (g_type_is_a (type, GST_TYPE_URI_HANDLER)) {
    GstURIHandlerInterface *iface = (GstURIHandlerInterface *)
        g_type_interface_peek (klass, GST_TYPE_URI_HANDLER);

    if (!iface || (!iface->get_type && !iface->get_type_full) ||
        (!iface->get_protocols && !iface->get_protocols_full))
      goto urierror;
    if (iface->get_type)
      factory->uri_type = iface->get_type ();
    else if (iface->get_type_full)
      factory->uri_type = iface->get_type_full (factory->type);
    if (!GST_URI_TYPE_IS_VALID (factory->uri_type))
      goto urierror;
    if (iface->get_protocols)
      factory->uri_protocols = g_strdupv (iface->get_protocols ());
    else if (iface->get_protocols_full)
      factory->uri_protocols = iface->get_protocols_full (factory->type);
    if (!factory->uri_protocols)
      goto urierror;
  }

  interfaces = g_type_interfaces (type, &n_interfaces);
  for (i = 0; i < n_interfaces; i++) {
    __gst_element_factory_add_interface (factory, g_type_name (interfaces[i]));
  }
  g_free (interfaces);

  if (plugin && plugin->desc.name) {
    GST_PLUGIN_FEATURE (factory)->plugin_name = plugin->desc.name;
  } else {
    GST_PLUGIN_FEATURE (factory)->plugin_name = "NULL";
  }
  gst_plugin_feature_set_rank (GST_PLUGIN_FEATURE (factory), rank);
  GST_PLUGIN_FEATURE (factory)->loaded = TRUE;

  gst_registry_add_feature (gst_registry_get_default (),
      GST_PLUGIN_FEATURE (factory));

  return TRUE;

  /* ERRORS */
urierror:
  {
    GST_WARNING_OBJECT (factory, "error with uri handler!");
    gst_element_factory_cleanup (factory);
    return FALSE;
  }

detailserror:
  {
    GST_WARNING_OBJECT (factory,
        "The GstElementDetails don't seem to have been set properly");
    gst_element_factory_cleanup (factory);
    return FALSE;
  }
}

/**
 * gst_element_factory_create:
 * @factory: factory to instantiate
 * @name: name of new element
 *
 * Create a new element of the type defined by the given elementfactory.
 * It will be given the name supplied, since all elements require a name as
 * their first argument.
 *
 * Returns: new #GstElement or NULL if the element couldn't be created
 */
GstElement *
gst_element_factory_create (GstElementFactory * factory, const gchar * name)
{
  GstElement *element;
  GstElementClass *oclass;
  GstElementFactory *newfactory;

  g_return_val_if_fail (factory != NULL, NULL);

  newfactory =
      GST_ELEMENT_FACTORY (gst_plugin_feature_load (GST_PLUGIN_FEATURE
          (factory)));

  if (newfactory == NULL)
    goto load_failed;

  factory = newfactory;

  if (name)
    GST_INFO ("creating element \"%s\" named \"%s\"",
        GST_PLUGIN_FEATURE_NAME (factory), GST_STR_NULL (name));
  else
    GST_INFO ("creating element \"%s\"", GST_PLUGIN_FEATURE_NAME (factory));

  if (factory->type == 0)
    goto no_type;

  /* create an instance of the element, cast so we don't assert on NULL */
  element = GST_ELEMENT_CAST (g_object_new (factory->type, NULL));
  if (G_UNLIKELY (element == NULL))
    goto no_element;

  /* fill in the pointer to the factory in the element class. The
   * class will not be unreffed currently. 
   * FIXME: This isn't safe and may leak a refcount on the factory if 2 threads
   * create the first instance of an element at the same moment */
  oclass = GST_ELEMENT_GET_CLASS (element);
  if (G_UNLIKELY (oclass->elementfactory == NULL))
    oclass->elementfactory = factory;
  else
    gst_object_unref (factory);

  if (name)
    gst_object_set_name (GST_OBJECT (element), name);

  GST_DEBUG ("created element \"%s\"", GST_PLUGIN_FEATURE_NAME (factory));

  return element;

  /* ERRORS */
load_failed:
  {
    GST_WARNING_OBJECT (factory, "loading plugin returned NULL!");
    return NULL;
  }
no_type:
  {
    GST_WARNING_OBJECT (factory, "factory has no type");
    gst_object_unref (factory);
    return NULL;
  }
no_element:
  {
    GST_WARNING_OBJECT (factory, "could not create element");
    gst_object_unref (factory);
    return NULL;
  }
}

/**
 * gst_element_factory_make:
 * @factoryname: a named factory to instantiate
 * @name: name of new element
 *
 * Create a new element of the type defined by the given element factory.
 * If name is NULL, then the element will receive a guaranteed unique name,
 * consisting of the element factory name and a number.
 * If name is given, it will be given the name supplied.
 *
 * Returns: new #GstElement or NULL if unable to create element
 */
GstElement *
gst_element_factory_make (const gchar * factoryname, const gchar * name)
{
  GstElementFactory *factory;
  GstElement *element;

  g_return_val_if_fail (factoryname != NULL, NULL);

  GST_LOG ("gstelementfactory: make \"%s\" \"%s\"",
      factoryname, GST_STR_NULL (name));

  factory = gst_element_factory_find (factoryname);
  if (factory == NULL)
    goto no_factory;

  GST_LOG_OBJECT (factory, "found factory %p", factory);
  element = gst_element_factory_create (factory, name);
  if (element == NULL)
    goto create_failed;

  gst_object_unref (factory);
  return element;

  /* ERRORS */
no_factory:
  {
    GST_INFO ("no such element factory \"%s\"!", factoryname);
    return NULL;
  }
create_failed:
  {
    GST_INFO_OBJECT (factory, "couldn't create instance!");
    gst_object_unref (factory);
    return NULL;
  }
}

void
__gst_element_factory_add_static_pad_template (GstElementFactory * factory,
    GstStaticPadTemplate * templ)
{
  g_return_if_fail (factory != NULL);
  g_return_if_fail (templ != NULL);

  factory->staticpadtemplates =
      g_list_append (factory->staticpadtemplates, templ);
  factory->numpadtemplates++;
}

/**
 * gst_element_factory_get_element_type:
 * @factory: factory to get managed #GType from
 *
 * Get the #GType for elements managed by this factory. The type can
 * only be retrieved if the element factory is loaded, which can be
 * assured with gst_plugin_feature_load().
 *
 * Returns: the #GType for elements managed by this factory or 0 if
 * the factory is not loaded.
 */
GType
gst_element_factory_get_element_type (GstElementFactory * factory)
{
  g_return_val_if_fail (GST_IS_ELEMENT_FACTORY (factory), 0);

  return factory->type;
}

/**
 * gst_element_factory_get_longname:
 * @factory: a #GstElementFactory
 *
 * Gets the longname for this factory
 *
 * Returns: the longname
 */
G_CONST_RETURN gchar *
gst_element_factory_get_longname (GstElementFactory * factory)
{
  g_return_val_if_fail (GST_IS_ELEMENT_FACTORY (factory), NULL);

  return factory->details.longname;
}

/**
 * gst_element_factory_get_klass:
 * @factory: a #GstElementFactory
 *
 * Gets the class for this factory.
 *
 * Returns: the class
 */
G_CONST_RETURN gchar *
gst_element_factory_get_klass (GstElementFactory * factory)
{
  g_return_val_if_fail (GST_IS_ELEMENT_FACTORY (factory), NULL);

  return factory->details.klass;
}

/**
 * gst_element_factory_get_description:
 * @factory: a #GstElementFactory
 *
 * Gets the description for this factory.
 *
 * Returns: the description
 */
G_CONST_RETURN gchar *
gst_element_factory_get_description (GstElementFactory * factory)
{
  g_return_val_if_fail (GST_IS_ELEMENT_FACTORY (factory), NULL);

  return factory->details.description;
}

/**
 * gst_element_factory_get_author:
 * @factory: a #GstElementFactory
 *
 * Gets the author for this factory.
 *
 * Returns: the author
 */
G_CONST_RETURN gchar *
gst_element_factory_get_author (GstElementFactory * factory)
{
  g_return_val_if_fail (GST_IS_ELEMENT_FACTORY (factory), NULL);

  return factory->details.author;
}

/**
 * gst_element_factory_get_num_pad_templates:
 * @factory: a #GstElementFactory
 *
 * Gets the number of pad_templates in this factory.
 *
 * Returns: the number of pad_templates
 */
guint
gst_element_factory_get_num_pad_templates (GstElementFactory * factory)
{
  g_return_val_if_fail (GST_IS_ELEMENT_FACTORY (factory), 0);

  return factory->numpadtemplates;
}

/**
 * __gst_element_factory_add_interface:
 * @elementfactory: The elementfactory to add the interface to
 * @interfacename: Name of the interface
 *
 * Adds the given interfacename to the list of implemented interfaces of the
 * element.
 */
void
__gst_element_factory_add_interface (GstElementFactory * elementfactory,
    const gchar * interfacename)
{
  g_return_if_fail (GST_IS_ELEMENT_FACTORY (elementfactory));
  g_return_if_fail (interfacename != NULL);
  g_return_if_fail (interfacename[0] != '\0');  /* no empty string */

  elementfactory->interfaces =
      g_list_prepend (elementfactory->interfaces, g_strdup (interfacename));
}

/**
 * gst_element_factory_get_static_pad_templates:
 * @factory: a #GstElementFactory
 *
 * Gets the #GList of #GstStaticPadTemplate for this factory.
 *
 * Returns: the padtemplates
 */
G_CONST_RETURN GList *
gst_element_factory_get_static_pad_templates (GstElementFactory * factory)
{
  g_return_val_if_fail (GST_IS_ELEMENT_FACTORY (factory), NULL);

  return factory->staticpadtemplates;
}

/**
 * gst_element_factory_get_uri_type:
 * @factory: a #GstElementFactory
 *
 * Gets the type of URIs the element supports or #GST_URI_UNKNOWN if none.
 *
 * Returns: type of URIs this element supports
 */
gint
gst_element_factory_get_uri_type (GstElementFactory * factory)
{
  g_return_val_if_fail (GST_IS_ELEMENT_FACTORY (factory), GST_URI_UNKNOWN);

  return factory->uri_type;
}

/**
 * gst_element_factory_get_uri_protocols:
 * @factory: a #GstElementFactory
 *
 * Gets a NULL-terminated array of protocols this element supports or NULL if
 * no protocols are supported. You may not change the contents of the returned
 * array, as it is still owned by the element factory. Use g_strdupv() to
 * make a copy of the protocol string array if you need to.
 *
 * Returns: the supported protocols or NULL
 */
gchar **
gst_element_factory_get_uri_protocols (GstElementFactory * factory)
{
  g_return_val_if_fail (GST_IS_ELEMENT_FACTORY (factory), NULL);

  return factory->uri_protocols;
}

/**
 * gst_element_factory_has_interface:
 * @factory: a #GstElementFactory
 * @interfacename: an interface name
 *
 * Check if @factory implements the interface with name @interfacename.
 *
 * Returns: #TRUE when @factory implement the interface.
 *
 * Since: 0.10.14
 */
gboolean
gst_element_factory_has_interface (GstElementFactory * factory,
    const gchar * interfacename)
{
  GList *walk;

  g_return_val_if_fail (GST_IS_ELEMENT_FACTORY (factory), FALSE);

  for (walk = factory->interfaces; walk; walk = g_list_next (walk)) {
    gchar *iname = (gchar *) walk->data;

    if (!strcmp (iname, interfacename))
      return TRUE;
  }
  return FALSE;
}
