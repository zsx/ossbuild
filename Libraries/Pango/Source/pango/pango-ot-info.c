/* Pango
 * pango-ot-info.c: Store tables for OpenType
 *
 * Copyright (C) 2000 Red Hat Software
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "config.h"

#include "pango-ot-private.h"
#include "pango-impl-utils.h"
#include FT_MODULE_H

static void pango_ot_info_class_init (GObjectClass *object_class);
static void pango_ot_info_finalize   (GObject *object);

static GObjectClass *parent_class;

enum
{
  INFO_LOADED_GDEF = 1 << 0,
  INFO_LOADED_GSUB = 1 << 1,
  INFO_LOADED_GPOS = 1 << 2
};

GType
pango_ot_info_get_type (void)
{
  static GType object_type = 0;

  if (G_UNLIKELY (!object_type))
    {
      const GTypeInfo object_info =
      {
	sizeof (PangoOTInfoClass),
	(GBaseInitFunc) NULL,
	(GBaseFinalizeFunc) NULL,
	(GClassInitFunc)pango_ot_info_class_init,
	NULL,           /* class_finalize */
	NULL,           /* class_data */
	sizeof (PangoOTInfo),
	0,              /* n_preallocs */
	NULL,           /* init */
	NULL,           /* value_table */
      };

      object_type = g_type_register_static (G_TYPE_OBJECT,
					    I_("PangoOTInfo"),
					    &object_info, 0);
    }

  return object_type;
}

static void
pango_ot_info_class_init (GObjectClass *object_class)
{
  parent_class = g_type_class_peek_parent (object_class);

  object_class->finalize = pango_ot_info_finalize;
}

static void
pango_ot_info_finalize (GObject *object)
{
  PangoOTInfo *info = PANGO_OT_INFO (object);

  if (info->gdef)
    {
      HB_Done_GDEF_Table (info->gdef);
      info->gdef = NULL;
    }
  if (info->gsub)
    {
      HB_Done_GSUB_Table (info->gsub);
      info->gsub = NULL;
    }
  if (info->gpos)
    {
      HB_Done_GPOS_Table (info->gpos);
      info->gpos = NULL;
    }

  parent_class->finalize (object);
}

static void
pango_ot_info_finalizer (void *object)
{
  FT_Face face = object;
  PangoOTInfo *info = face->generic.data;

  info->face = NULL;
  g_object_unref (info);
}

/**
 * pango_ot_info_get:
 * @face: a <type>FT_Face</type>.
 *
 * Returns the #PangoOTInfo structure for the given FreeType font face.
 *
 * Return value: the #PangoOTInfo for @face. This object will have
 * the same lifetime as @face.
 *
 * Since: 1.2
 **/
PangoOTInfo *
pango_ot_info_get (FT_Face face)
{
  PangoOTInfo *info;

  if (face->generic.data)
    return face->generic.data;
  else
    {
      info = face->generic.data = g_object_new (PANGO_TYPE_OT_INFO, NULL);
      face->generic.finalizer = pango_ot_info_finalizer;

      info->face = face;
    }

  return info;
}

/* There must be be a better way to do this
 */
static gboolean
is_truetype (FT_Face face)
{
  return FT_IS_SFNT(face);
}

typedef struct _GlyphInfo GlyphInfo;

struct _GlyphInfo {
  HB_UShort glyph;
  HB_UShort class;
};

static int
compare_glyph_info (gconstpointer a,
		    gconstpointer b)
{
  const GlyphInfo *info_a = a;
  const GlyphInfo *info_b = b;

  return (info_a->glyph < info_b->glyph) ? -1 :
    (info_a->glyph == info_b->glyph) ? 0 : 1;
}

/* Make a guess at the appropriate class for a glyph given
 * a character code that maps to the glyph
 */
static gboolean
get_glyph_class (gunichar   charcode,
		 HB_UShort *class)
{
  /* For characters mapped into the Arabic Presentation forms, using properties
   * derived as we apply GSUB substitutions will be more reliable
   */
  if ((charcode >= 0xFB50 && charcode <= 0xFDFF) || /* Arabic Presentation Forms-A */
      (charcode >= 0xFE70 && charcode <= 0XFEFF))   /* Arabic Presentation Forms-B */
    return FALSE;

  switch ((int) g_unichar_type (charcode))
    {
    case G_UNICODE_COMBINING_MARK:
    case G_UNICODE_ENCLOSING_MARK:
    case G_UNICODE_NON_SPACING_MARK:
      *class = 3;		/* Mark glyph (non-spacing combining glyph) */
      return TRUE;
    case G_UNICODE_UNASSIGNED:
    case G_UNICODE_PRIVATE_USE:
      return FALSE;		/* Unknown, don't assign a class; classes get
				 * propagated during GSUB application */
    default:
      *class = 1;               /* Base glyph (single character, spacing glyph) */
      return TRUE;
    }
}

static gboolean
set_unicode_charmap (FT_Face face)
{
  int charmap;

  for (charmap = 0; charmap < face->num_charmaps; charmap++)
    if (face->charmaps[charmap]->encoding == ft_encoding_unicode)
      {
	HB_Error error = FT_Set_Charmap(face, face->charmaps[charmap]);
	return error == HB_Err_Ok;
      }

  return FALSE;
}

/* Synthesize a GDEF table using the font's charmap and the
 * Unicode property database. We'll fill in class definitions
 * for glyphs not in the charmap as we walk through the tables.
 */
static void
synthesize_class_def (PangoOTInfo *info)
{
  GArray *glyph_infos;
  HB_UShort *glyph_indices;
  HB_UShort *classes;
  HB_UInt charcode;
  HB_UInt glyph;
  unsigned int i, j;
  FT_CharMap old_charmap;

  old_charmap = info->face->charmap;

  if (!old_charmap || !old_charmap->encoding != ft_encoding_unicode)
    if (!set_unicode_charmap (info->face))
      return;

  glyph_infos = g_array_new (FALSE, FALSE, sizeof (GlyphInfo));

  /* Collect all the glyphs in the charmap, and guess
   * the appropriate classes for them
   */
  charcode = FT_Get_First_Char (info->face, &glyph);
  while (glyph != 0)
    {
      GlyphInfo glyph_info;

      if (glyph <= 65535)
	{
	  glyph_info.glyph = glyph;
	  if (get_glyph_class (charcode, &glyph_info.class))
	    g_array_append_val (glyph_infos, glyph_info);
	}

      charcode = FT_Get_Next_Char (info->face, charcode, &glyph);
    }

  /* Sort and remove duplicates
   */
  g_array_sort (glyph_infos, compare_glyph_info);

  glyph_indices = g_new (HB_UShort, glyph_infos->len);
  classes = g_new (HB_UShort, glyph_infos->len);

  for (i = 0, j = 0; i < glyph_infos->len; i++)
    {
      GlyphInfo *info = &g_array_index (glyph_infos, GlyphInfo, i);

      if (j == 0 || info->glyph != glyph_indices[j - 1])
	{
	  glyph_indices[j] = info->glyph;
	  classes[j] = info->class;

	  j++;
	}
    }

  g_array_free (glyph_infos, TRUE);

  HB_GDEF_Build_ClassDefinition (info->gdef, info->face->num_glyphs, j,
				 glyph_indices, classes);

  g_free (glyph_indices);
  g_free (classes);

  if (old_charmap && info->face->charmap != old_charmap)
    FT_Set_Charmap (info->face, old_charmap);
}

HB_GDEF
pango_ot_info_get_gdef (PangoOTInfo *info)
{
  g_return_val_if_fail (PANGO_IS_OT_INFO (info), NULL);

  if (!(info->loaded & INFO_LOADED_GDEF))
    {
      HB_Error error;

      info->loaded |= INFO_LOADED_GDEF;

      if (is_truetype (info->face))
	{
	  error = HB_Load_GDEF_Table (info->face, &info->gdef);

	  if (error && error != HB_Err_Not_Covered)
	    g_warning ("Error loading GDEF table 0x%04X", error);

	  if (!info->gdef)
	    error = HB_New_GDEF_Table (&info->gdef);

	  if (info->gdef && !info->gdef->GlyphClassDef.loaded)
	    synthesize_class_def (info);
	}
    }

  return info->gdef;
}

HB_GSUB
pango_ot_info_get_gsub (PangoOTInfo *info)
{
  g_return_val_if_fail (PANGO_IS_OT_INFO (info), NULL);

  if (!(info->loaded & INFO_LOADED_GSUB))
    {
      HB_Error error;
      HB_GDEF gdef = pango_ot_info_get_gdef (info);

      info->loaded |= INFO_LOADED_GSUB;

      if (is_truetype (info->face))
	{
	  error = HB_Load_GSUB_Table (info->face, &info->gsub, gdef);

	  if (error && error != HB_Err_Not_Covered)
	    g_warning ("Error loading GSUB table 0x%04X", error);
	}
    }

  return info->gsub;
}

HB_GPOS
pango_ot_info_get_gpos (PangoOTInfo *info)
{
  g_return_val_if_fail (PANGO_IS_OT_INFO (info), NULL);

  if (!(info->loaded & INFO_LOADED_GPOS))
    {
      HB_Error error;
      HB_GDEF gdef = pango_ot_info_get_gdef (info);

      info->loaded |= INFO_LOADED_GPOS;

      if (is_truetype (info->face))
	{
	  error = HB_Load_GPOS_Table (info->face, &info->gpos, gdef);

	  if (error && error != HB_Err_Not_Covered)
	    g_warning ("Error loading GPOS table 0x%04X", error);
	}
    }

  return info->gpos;
}

static gboolean
get_tables (PangoOTInfo      *info,
	    PangoOTTableType  table_type,
	    HB_ScriptList  **script_list,
	    HB_FeatureList **feature_list)
{
  if (table_type == PANGO_OT_TABLE_GSUB)
    {
      HB_GSUB gsub = pango_ot_info_get_gsub (info);

      if (!gsub)
	return FALSE;
      else
	{
	  if (script_list)
	    *script_list = &gsub->ScriptList;
	  if (feature_list)
	    *feature_list = &gsub->FeatureList;
	  return TRUE;
	}
    }
  else
    {
      HB_GPOS gpos = pango_ot_info_get_gpos (info);

      if (!gpos)
	return FALSE;
      else
	{
	  if (script_list)
	    *script_list = &gpos->ScriptList;
	  if (feature_list)
	    *feature_list = &gpos->FeatureList;
	  return TRUE;
	}
    }
}

/**
 * pango_ot_info_find_script:
 * @info: a #PangoOTInfo.
 * @table_type: the table type to obtain information about.
 * @script_tag: the tag of the script to find.
 * @script_index: location to store the index of the script, or %NULL.
 *
 * Finds the index of a script.  If not found, tries to find the 'DFLT'
 * and then 'dflt' scripts and return the index of that in @script_index.
 * If none of those is found either, %PANGO_OT_NO_SCRIPT is placed in
 * @script_index.
 *
 * All other functions taking an input script_index parameter know
 * how to handle %PANGO_OT_NO_SCRIPT, so one can ignore the return
 * value of this function completely and proceed, to enjoy the automatic
 * fallback to the 'DFLT'/'dflt' script.
 *
 * Return value: %TRUE if the script was found.
 **/
gboolean
pango_ot_info_find_script (PangoOTInfo      *info,
			   PangoOTTableType  table_type,
			   PangoOTTag        script_tag,
			   guint            *script_index)
{
  HB_ScriptList *script_list;
  int i;

  if (script_index)
    *script_index = PANGO_OT_NO_SCRIPT;

  g_return_val_if_fail (PANGO_IS_OT_INFO (info), FALSE);

  if (!get_tables (info, table_type, &script_list, NULL))
    return FALSE;

  for (i=0; i < script_list->ScriptCount; i++)
    {
      if (script_list->ScriptRecord[i].ScriptTag == script_tag)
	{
	  if (script_index)
	    *script_index = i;

	  return TRUE;
	}
    }

  /* try finding 'DFLT' */
  script_tag = PANGO_OT_TAG_DEFAULT_SCRIPT;

  for (i=0; i < script_list->ScriptCount; i++)
    {
      if (script_list->ScriptRecord[i].ScriptTag == script_tag)
	{
	  if (script_index)
	    *script_index = i;

	  return FALSE;
	}
    }

  /* try with 'dflt'; MS site has had typos and many fonts use it now :( */
  script_tag = PANGO_OT_TAG_MAKE ('d', 'f', 'l', 't');

  for (i=0; i < script_list->ScriptCount; i++)
    {
      if (script_list->ScriptRecord[i].ScriptTag == script_tag)
	{
	  if (script_index)
	    *script_index = i;

	  return FALSE;
	}
    }

  return FALSE;
}

/**
 * pango_ot_info_find_language:
 * @info: a #PangoOTInfo.
 * @table_type: the table type to obtain information about.
 * @script_index: the index of the script whose languages are searched.
 * @language_tag: the tag of the language to find.
 * @language_index: location to store the index of the language, or %NULL.
 * @required_feature_index: location to store the required feature index of
 *    the language, or %NULL.
 *
 * Finds the index of a language and its required feature index.
 * If the language is not found, sets @language_index to
 * PANGO_OT_DEFAULT_LANGUAGE and the required feature of the default language
 * system is returned in required_feature_index.  For best compatibility with
 * some fonts, also searches the language system tag 'dflt' before falling
 * back to the default language system, but that is transparent to the user.
 * The user can simply ignore the return value of this function to
 * automatically fall back to the default language system.
 *
 * Return value: %TRUE if the language was found.
 **/
gboolean
pango_ot_info_find_language (PangoOTInfo      *info,
			     PangoOTTableType  table_type,
			     guint             script_index,
			     PangoOTTag        language_tag,
			     guint            *language_index,
			     guint            *required_feature_index)
{
  HB_ScriptList *script_list;
  HB_ScriptTable *script;
  int i;

  if (language_index)
    *language_index = PANGO_OT_DEFAULT_LANGUAGE;
  if (required_feature_index)
    *required_feature_index = PANGO_OT_NO_FEATURE;

  g_return_val_if_fail (PANGO_IS_OT_INFO (info), FALSE);

  if (script_index == PANGO_OT_NO_SCRIPT)
    return FALSE;

  if (!get_tables (info, table_type, &script_list, NULL))
    return FALSE;

  g_return_val_if_fail (script_index < script_list->ScriptCount, FALSE);
  script = &script_list->ScriptRecord[script_index].Script;

  for (i = 0; i < script->LangSysCount; i++)
    {
      if (script->LangSysRecord[i].LangSysTag == language_tag)
	{
	  if (language_index)
	    *language_index = i;
	  if (required_feature_index)
	    *required_feature_index = script->LangSysRecord[i].LangSys.ReqFeatureIndex;
	  return TRUE;
	}
    }

  /* try with 'dflt'; MS site has had typos and many fonts use it now :( */
  language_tag = PANGO_OT_TAG_MAKE ('d', 'f', 'l', 't');

  for (i = 0; i < script->LangSysCount; i++)
    {
      if (script->LangSysRecord[i].LangSysTag == language_tag)
	{
	  if (language_index)
	    *language_index = i;
	  if (required_feature_index)
	    *required_feature_index = script->LangSysRecord[i].LangSys.ReqFeatureIndex;
	  return FALSE;
	}
    }

  /* DefaultLangSys */
  if (language_index)
    *language_index = PANGO_OT_DEFAULT_LANGUAGE;
  if (required_feature_index)
    *required_feature_index = script->DefaultLangSys.ReqFeatureIndex;

  return FALSE;
}

/**
 * pango_ot_info_find_feature:
 * @info: a #PangoOTInfo.
 * @table_type: the table type to obtain information about.
 * @feature_tag: the tag of the feature to find.
 * @script_index: the index of the script.
 * @language_index: the index of the language whose features are searched,
 *     or %PANGO_OT_DEFAULT_LANGUAGE to use the default language of the script.
 * @feature_index: location to store the index of the feature, or %NULL.
 *
 * Finds the index of a feature.  If the feature is not found, sets
 * @feature_index to PANGO_OT_NO_FEATURE, which is safe to pass to
 * pango_ot_ruleset_add_feature() and similar functions.
 *
 * In the future, this may set @feature_index to an special value that if used
 * in pango_ot_ruleset_add_feature() will ask Pango to synthesize the
 * requested feature based on Unicode properties and data.  However, this
 * function will still return %FALSE in those cases.  So, users may want to
 * ignore the return value of this function in certain cases.
 *
 * Return value: %TRUE if the feature was found.
 **/
gboolean
pango_ot_info_find_feature  (PangoOTInfo      *info,
			     PangoOTTableType  table_type,
			     PangoOTTag        feature_tag,
			     guint             script_index,
			     guint             language_index,
			     guint            *feature_index)
{
  HB_ScriptList *script_list;
  HB_FeatureList *feature_list;
  HB_ScriptTable *script;
  HB_LangSys *lang_sys;

  int i;

  if (feature_index)
    *feature_index = PANGO_OT_NO_FEATURE;

  g_return_val_if_fail (PANGO_IS_OT_INFO (info), FALSE);

  if (script_index == PANGO_OT_NO_SCRIPT)
    return FALSE;

  if (!get_tables (info, table_type, &script_list, &feature_list))
    return FALSE;

  g_return_val_if_fail (script_index < script_list->ScriptCount, FALSE);
  script = &script_list->ScriptRecord[script_index].Script;

  if (language_index == PANGO_OT_DEFAULT_LANGUAGE)
    lang_sys = &script->DefaultLangSys;
  else
    {
      g_return_val_if_fail (language_index < script->LangSysCount, FALSE);
      lang_sys = &script->LangSysRecord[language_index].LangSys;
    }

  for (i = 0; i < lang_sys->FeatureCount; i++)
    {
      HB_UShort index = lang_sys->FeatureIndex[i];

      if (index < feature_list->FeatureCount &&
	  feature_list->FeatureRecord[index].FeatureTag == feature_tag)
	{
	  if (feature_index)
	    *feature_index = index;

	  return TRUE;
	}
    }

  return FALSE;
}

/**
 * pango_ot_info_list_scripts:
 * @info: a #PangoOTInfo.
 * @table_type: the table type to obtain information about.
 *
 * Obtains the list of available scripts.
 *
 * Return value: a newly-allocated zero-terminated array containing the tags of the
 *   available scripts.  Should be freed using g_free().
 **/
PangoOTTag *
pango_ot_info_list_scripts (PangoOTInfo      *info,
			    PangoOTTableType  table_type)
{
  PangoOTTag *result;
  HB_ScriptList *script_list;
  int i;

  g_return_val_if_fail (PANGO_IS_OT_INFO (info), NULL);

  if (!get_tables (info, table_type, &script_list, NULL))
    return NULL;

  result = g_new (PangoOTTag, script_list->ScriptCount + 1);

  for (i=0; i < script_list->ScriptCount; i++)
    result[i] = script_list->ScriptRecord[i].ScriptTag;

  result[i] = 0;

  return result;
}

/**
 * pango_ot_info_list_languages:
 * @info: a #PangoOTInfo.
 * @table_type: the table type to obtain information about.
 * @script_index: the index of the script to list languages for.
 * @language_tag: unused parameter.
 *
 * Obtains the list of available languages for a given script.
 *
 * Return value: a newly-allocated zero-terminated array containing the tags of the
 *   available languages.  Should be freed using g_free().
 **/
PangoOTTag *
pango_ot_info_list_languages (PangoOTInfo      *info,
			      PangoOTTableType  table_type,
			      guint             script_index,
			      PangoOTTag        language_tag G_GNUC_UNUSED)
{
  PangoOTTag *result;
  HB_ScriptList *script_list;
  HB_ScriptTable *script;
  int i;

  g_return_val_if_fail (PANGO_IS_OT_INFO (info), NULL);

  if (script_index == PANGO_OT_NO_SCRIPT)
    {
      result = g_new (PangoOTTag, 0+1);
      result[0] = 0;
      return result;
    }

  if (!get_tables (info, table_type, &script_list, NULL))
    return NULL;

  g_return_val_if_fail (script_index < script_list->ScriptCount, NULL);
  script = &script_list->ScriptRecord[script_index].Script;

  result = g_new (PangoOTTag, script->LangSysCount + 1);

  for (i = 0; i < script->LangSysCount; i++)
    result[i] = script->LangSysRecord[i].LangSysTag;

  result[i] = 0;

  return result;
}

/**
 * pango_ot_info_list_features:
 * @info: a #PangoOTInfo.
 * @table_type: the table type to obtain information about.
 * @tag: unused parameter.
 * @script_index: the index of the script to obtain information about.
 * @language_index: the index of the language to list features for, or
 *     %PANGO_OT_DEFAULT_LANGUAGE, to list features for the default
 *     language of the script.
 *
 * Obtains the list of features for the given language of the given script.
 *
 * Return value: a newly-allocated zero-terminated array containing the tags of the
 * available features.  Should be freed using g_free().
 **/
PangoOTTag *
pango_ot_info_list_features  (PangoOTInfo      *info,
			      PangoOTTableType  table_type,
			      PangoOTTag        tag G_GNUC_UNUSED,
			      guint             script_index,
			      guint             language_index)
{
  PangoOTTag *result;

  HB_ScriptList *script_list;
  HB_FeatureList *feature_list;
  HB_ScriptTable *script;
  HB_LangSys *lang_sys;

  int i, j;

  g_return_val_if_fail (PANGO_IS_OT_INFO (info), NULL);

  if (script_index == PANGO_OT_NO_SCRIPT)
    {
      result = g_new (PangoOTTag, 0+1);
      result[0] = 0;
      return result;
    }

  if (!get_tables (info, table_type, &script_list, &feature_list))
    return NULL;

  g_return_val_if_fail (script_index < script_list->ScriptCount, NULL);
  script = &script_list->ScriptRecord[script_index].Script;

  if (language_index == PANGO_OT_DEFAULT_LANGUAGE)
    lang_sys = &script->DefaultLangSys;
  else
    {
      g_return_val_if_fail (language_index < script->LangSysCount, NULL);
      lang_sys = &script->LangSysRecord[language_index].LangSys;
    }

  result = g_new (PangoOTTag, lang_sys->FeatureCount + 1);

  j = 0;
  for (i = 0; i < lang_sys->FeatureCount; i++)
    {
      HB_UShort index = lang_sys->FeatureIndex[i];

      if (index < feature_list->FeatureCount)
	result[j++] = feature_list->FeatureRecord[index].FeatureTag;
    }

  result[j] = 0;

  return result;
}


