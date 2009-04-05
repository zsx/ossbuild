/*
 * mpegtsparse.c - 
 * Copyright (C) 2007 Alessandro Decina
 * 
 * Authors:
 *   Alessandro Decina <alessandro@nnva.org>
 *   Zaheer Abbas Merali <zaheerabbas at merali dot org>
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>

#include "mpegtsparse.h"
#include "gstmpegdesc.h"

/* latency in mseconds */
#define TS_LATENCY 700

#define TABLE_ID_UNSET 0xFF

GST_DEBUG_CATEGORY_STATIC (mpegts_parse_debug);
#define GST_CAT_DEFAULT mpegts_parse_debug

typedef struct _MpegTSParsePad MpegTSParsePad;

typedef struct
{
  guint16 pid;
  guint8 stream_type;
} MpegTSParseStream;

typedef struct
{
  gint program_number;
  guint16 pmt_pid;
  guint16 pcr_pid;
  GstStructure *pmt_info;
  GHashTable *streams;
  gint patcount;
  gint selected;
  gboolean active;
  MpegTSParsePad *tspad;
} MpegTSParseProgram;

struct _MpegTSParsePad
{
  GstPad *pad;

  /* the program number that the peer wants on this pad */
  gint program_number;
  MpegTSParseProgram *program;

  /* set to FALSE before a push and TRUE after */
  gboolean pushed;

  /* the return of the latest push */
  GstFlowReturn flow_return;
};

static GstElementDetails mpegts_parse_details =
GST_ELEMENT_DETAILS ("MPEG transport stream parser",
    "Codec/Parser",
    "Parses MPEG2 transport streams",
    "Alessandro Decina <alessandro@nnva.org>\n"
    "Zaheer Abbas Merali <zaheerabbas at merali dot org>");

static GstStaticPadTemplate sink_template = GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("video/mpegts, " "systemstream = (boolean) true ")
    );

static GstStaticPadTemplate src_template =
GST_STATIC_PAD_TEMPLATE ("src%d", GST_PAD_SRC,
    GST_PAD_REQUEST,
    GST_STATIC_CAPS ("video/mpegts, " "systemstream = (boolean) true ")
    );

static GstStaticPadTemplate program_template =
GST_STATIC_PAD_TEMPLATE ("program_%d", GST_PAD_SRC,
    GST_PAD_SOMETIMES,
    GST_STATIC_CAPS ("video/mpegts, " "systemstream = (boolean) true ")
    );

enum
{
  ARG_0,
  PROP_PROGRAM_NUMBERS,
  /* FILL ME */
};

static void mpegts_parse_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void mpegts_parse_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);
static void mpegts_parse_dispose (GObject * object);
static void mpegts_parse_finalize (GObject * object);

static MpegTSParsePad *mpegts_parse_create_tspad (MpegTSParse * parse,
    const gchar * name);
static void mpegts_parse_destroy_tspad (MpegTSParse * parse,
    MpegTSParsePad * tspad);
static GstPad *mpegts_parse_activate_program (MpegTSParse * parse,
    MpegTSParseProgram * program);
static void mpegts_parse_free_program (MpegTSParseProgram * program);
static void mpegts_parse_free_stream (MpegTSParseStream * ptream);
static void mpegts_parse_reset_selected_programs (MpegTSParse * parse,
    gchar * programs);

static void mpegts_parse_pad_removed (GstElement * element, GstPad * pad);
static GstPad *mpegts_parse_request_new_pad (GstElement * element,
    GstPadTemplate * templ, const gchar * name);
static void mpegts_parse_release_pad (GstElement * element, GstPad * pad);
static GstFlowReturn mpegts_parse_chain (GstPad * pad, GstBuffer * buf);
static gboolean mpegts_parse_sink_event (GstPad * pad, GstEvent * event);
static GstStateChangeReturn mpegts_parse_change_state (GstElement * element,
    GstStateChange transition);
static gboolean mpegts_parse_src_pad_query (GstPad * pad, GstQuery * query);

GST_BOILERPLATE (MpegTSParse, mpegts_parse, GstElement, GST_TYPE_ELEMENT);

static const guint32 crc_tab[256] = {
  0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9, 0x130476dc, 0x17c56b6b,
  0x1a864db2, 0x1e475005, 0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
  0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd, 0x4c11db70, 0x48d0c6c7,
  0x4593e01e, 0x4152fda9, 0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
  0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011, 0x791d4014, 0x7ddc5da3,
  0x709f7b7a, 0x745e66cd, 0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
  0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5, 0xbe2b5b58, 0xbaea46ef,
  0xb7a96036, 0xb3687d81, 0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
  0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49, 0xc7361b4c, 0xc3f706fb,
  0xceb42022, 0xca753d95, 0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
  0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d, 0x34867077, 0x30476dc0,
  0x3d044b19, 0x39c556ae, 0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
  0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16, 0x018aeb13, 0x054bf6a4,
  0x0808d07d, 0x0cc9cdca, 0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
  0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02, 0x5e9f46bf, 0x5a5e5b08,
  0x571d7dd1, 0x53dc6066, 0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
  0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e, 0xbfa1b04b, 0xbb60adfc,
  0xb6238b25, 0xb2e29692, 0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
  0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a, 0xe0b41de7, 0xe4750050,
  0xe9362689, 0xedf73b3e, 0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
  0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686, 0xd5b88683, 0xd1799b34,
  0xdc3abded, 0xd8fba05a, 0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
  0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb, 0x4f040d56, 0x4bc510e1,
  0x46863638, 0x42472b8f, 0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
  0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47, 0x36194d42, 0x32d850f5,
  0x3f9b762c, 0x3b5a6b9b, 0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
  0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623, 0xf12f560e, 0xf5ee4bb9,
  0xf8ad6d60, 0xfc6c70d7, 0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
  0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f, 0xc423cd6a, 0xc0e2d0dd,
  0xcda1f604, 0xc960ebb3, 0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
  0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b, 0x9b3660c6, 0x9ff77d71,
  0x92b45ba8, 0x9675461f, 0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
  0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640, 0x4e8ee645, 0x4a4ffbf2,
  0x470cdd2b, 0x43cdc09c, 0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
  0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24, 0x119b4be9, 0x155a565e,
  0x18197087, 0x1cd86d30, 0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
  0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088, 0x2497d08d, 0x2056cd3a,
  0x2d15ebe3, 0x29d4f654, 0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
  0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c, 0xe3a1cbc1, 0xe760d676,
  0xea23f0af, 0xeee2ed18, 0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
  0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0, 0x9abc8bd5, 0x9e7d9662,
  0x933eb0bb, 0x97ffad0c, 0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
  0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4
};

/* relicenced to LGPL from fluendo ts demuxer */
static guint32
mpegts_parse_calc_crc32 (guint8 * data, guint datalen)
{
  gint i;
  guint32 crc = 0xffffffff;

  for (i = 0; i < datalen; i++) {
    crc = (crc << 8) ^ crc_tab[((crc >> 24) ^ *data++) & 0xff];
  }
  return crc;
}

static void
mpegts_parse_base_init (gpointer klass)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&sink_template));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&src_template));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&program_template));

  gst_element_class_set_details (element_class, &mpegts_parse_details);
}

static void
mpegts_parse_class_init (MpegTSParseClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *element_class;

  element_class = GST_ELEMENT_CLASS (klass);
  element_class->pad_removed = mpegts_parse_pad_removed;
  element_class->request_new_pad = mpegts_parse_request_new_pad;
  element_class->release_pad = mpegts_parse_release_pad;
  element_class->change_state = mpegts_parse_change_state;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->set_property = mpegts_parse_set_property;
  gobject_class->get_property = mpegts_parse_get_property;
  gobject_class->dispose = mpegts_parse_dispose;
  gobject_class->finalize = mpegts_parse_finalize;

  g_object_class_install_property (gobject_class, PROP_PROGRAM_NUMBERS,
      g_param_spec_string ("program-numbers",
          "Program Numbers",
          "Colon separated list of programs", "", G_PARAM_READWRITE));
}

static gboolean
foreach_psi_pid_remove (gpointer key, gpointer value, gpointer data)
{
  return TRUE;
}

static void
mpegts_parse_reset (MpegTSParse * parse)
{
  mpegts_packetizer_clear (parse->packetizer);
  g_hash_table_foreach_remove (parse->psi_pids, foreach_psi_pid_remove, NULL);

  /* PAT */
  g_hash_table_insert (parse->psi_pids,
      GINT_TO_POINTER (0), GINT_TO_POINTER (1));
  if (parse->pat != NULL)
    gst_structure_free (parse->pat);
  parse->pat = NULL;
  /* pmt pids will be added and removed dynamically */

}

static void
mpegts_parse_init (MpegTSParse * parse, MpegTSParseClass * klass)
{
  parse->sinkpad = gst_pad_new_from_static_template (&sink_template, "sink");
  gst_pad_set_chain_function (parse->sinkpad, mpegts_parse_chain);
  gst_pad_set_event_function (parse->sinkpad, mpegts_parse_sink_event);
  gst_element_add_pad (GST_ELEMENT (parse), parse->sinkpad);

  parse->disposed = FALSE;
  parse->packetizer = mpegts_packetizer_new ();
  parse->program_numbers = g_strdup ("");
  parse->pads_to_add = NULL;
  parse->programs = g_hash_table_new_full (g_direct_hash, g_direct_equal,
      NULL, (GDestroyNotify) mpegts_parse_free_program);
  parse->psi_pids = g_hash_table_new (g_direct_hash, g_direct_equal);
  mpegts_parse_reset (parse);
}

static void
mpegts_parse_dispose (GObject * object)
{
  MpegTSParse *parse = GST_MPEGTS_PARSE (object);

  if (!parse->disposed) {
    g_object_unref (parse->packetizer);
    parse->disposed = TRUE;
  }

  if (G_OBJECT_CLASS (parent_class)->dispose)
    G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
mpegts_parse_finalize (GObject * object)
{
  MpegTSParse *parse = GST_MPEGTS_PARSE (object);

  g_free (parse->program_numbers);
  if (parse->pat) {
    gst_structure_free (parse->pat);
    parse->pat = NULL;
  }
  g_hash_table_destroy (parse->programs);
  g_hash_table_destroy (parse->psi_pids);

  if (G_OBJECT_CLASS (parent_class)->finalize)
    G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
mpegts_parse_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  MpegTSParse *parse = GST_MPEGTS_PARSE (object);

  switch (prop_id) {
    case PROP_PROGRAM_NUMBERS:
      mpegts_parse_reset_selected_programs (parse, g_value_dup_string (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
mpegts_parse_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  MpegTSParse *parse = GST_MPEGTS_PARSE (object);

  switch (prop_id) {
    case PROP_PROGRAM_NUMBERS:
      g_value_set_string (value, parse->program_numbers);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static MpegTSParseProgram *
mpegts_parse_add_program (MpegTSParse * parse,
    gint program_number, guint16 pmt_pid)
{
  MpegTSParseProgram *program;

  program = g_new0 (MpegTSParseProgram, 1);
  program->program_number = program_number;
  program->pmt_pid = pmt_pid;
  program->pcr_pid = G_MAXUINT16;
  program->streams = g_hash_table_new_full (g_direct_hash, g_direct_equal,
      NULL, (GDestroyNotify) mpegts_parse_free_stream);
  program->patcount = 1;
  program->selected = 0;
  program->active = FALSE;

  g_hash_table_insert (parse->programs,
      GINT_TO_POINTER (program_number), program);

  return program;
}

static MpegTSParseProgram *
mpegts_parse_get_program (MpegTSParse * parse, gint program_number)
{
  MpegTSParseProgram *program;

  program = (MpegTSParseProgram *) g_hash_table_lookup (parse->programs,
      GINT_TO_POINTER ((gint) program_number));

  return program;
}

static GstPad *
mpegts_parse_activate_program (MpegTSParse * parse,
    MpegTSParseProgram * program)
{
  MpegTSParsePad *tspad;
  gchar *pad_name;

  pad_name = g_strdup_printf ("program_%d", program->program_number);

  tspad = mpegts_parse_create_tspad (parse, pad_name);
  tspad->program_number = program->program_number;
  tspad->program = program;
  program->tspad = tspad;
  g_free (pad_name);
  gst_pad_set_active (tspad->pad, TRUE);
  program->active = TRUE;

  return tspad->pad;
}

static GstPad *
mpegts_parse_deactivate_program (MpegTSParse * parse,
    MpegTSParseProgram * program)
{
  MpegTSParsePad *tspad;

  tspad = program->tspad;
  gst_pad_set_active (tspad->pad, FALSE);
  program->active = FALSE;

  /* tspad will be destroyed in GstElementClass::pad_removed */

  return tspad->pad;
}

static void
mpegts_parse_free_program (MpegTSParseProgram * program)
{
  if (program->pmt_info)
    gst_structure_free (program->pmt_info);

  g_hash_table_destroy (program->streams);

  g_free (program);
}

static void
mpegts_parse_remove_program (MpegTSParse * parse, gint program_number)
{
  g_hash_table_remove (parse->programs, GINT_TO_POINTER (program_number));
}

static void
mpegts_parse_sync_program_pads (MpegTSParse * parse,
    GList * to_add, GList * to_remove)
{
  GList *walk;

  for (walk = to_remove; walk; walk = walk->next)
    gst_element_remove_pad (GST_ELEMENT (parse), GST_PAD (walk->data));

  for (walk = to_add; walk; walk = walk->next)
    gst_element_add_pad (GST_ELEMENT (parse), GST_PAD (walk->data));

  if (to_add)
    g_list_free (to_add);

  if (to_remove)
    g_list_free (to_remove);
}


static MpegTSParseStream *
mpegts_parse_program_add_stream (MpegTSParse * parse,
    MpegTSParseProgram * program, guint16 pid, guint8 stream_type)
{
  MpegTSParseStream *stream;

  stream = g_new0 (MpegTSParseStream, 1);
  stream->pid = pid;
  stream->stream_type = stream_type;

  g_hash_table_insert (program->streams, GINT_TO_POINTER ((gint) pid), stream);

  return stream;
}

static void
foreach_program_activate_or_deactivate (gpointer key, gpointer value,
    gpointer data)
{
  MpegTSParse *parse = GST_MPEGTS_PARSE (data);
  MpegTSParseProgram *program = (MpegTSParseProgram *) value;

  /* at this point selected programs have program->selected == 2,
   * unselected programs thay may have to be deactivated have selected == 1 and
   * unselected inactive programs have selected == 0 */

  switch (--program->selected) {
    case 1:
      /* selected */
      if (!program->active && program->pmt_pid != G_MAXUINT16)
        parse->pads_to_add = g_list_append (parse->pads_to_add,
            mpegts_parse_activate_program (parse, program));
      break;
    case 0:
      /* unselected */
      if (program->active)
        parse->pads_to_remove = g_list_append (parse->pads_to_remove,
            mpegts_parse_deactivate_program (parse, program));
      break;
    case -1:
      /* was already unselected */
      program->selected = 0;
      break;
    default:
      g_return_if_reached ();
  }
}

static void
mpegts_parse_reset_selected_programs (MpegTSParse * parse,
    gchar * program_numbers)
{
  GList *pads_to_add = NULL;
  GList *pads_to_remove = NULL;

  GST_OBJECT_LOCK (parse);
  if (parse->program_numbers)
    g_free (parse->program_numbers);

  parse->program_numbers = program_numbers;

  if (*parse->program_numbers != '\0') {
    gint program_number;
    MpegTSParseProgram *program;
    gchar **progs, **walk;

    progs = g_strsplit (parse->program_numbers, ":", 0);

    walk = progs;
    while (*walk != NULL) {
      program_number = strtol (*walk, NULL, 0);
      program = mpegts_parse_get_program (parse, program_number);
      if (program == NULL)
        /* create the program, it will get activated once we get a PMT for it */
        program = mpegts_parse_add_program (parse, program_number, G_MAXUINT16);

      program->selected = 2;
      ++walk;
    }
    g_strfreev (progs);
  }

  g_hash_table_foreach (parse->programs,
      foreach_program_activate_or_deactivate, parse);

  pads_to_add = parse->pads_to_add;
  parse->pads_to_add = NULL;
  pads_to_remove = parse->pads_to_remove;
  parse->pads_to_remove = NULL;
  GST_OBJECT_UNLOCK (parse);

  mpegts_parse_sync_program_pads (parse, pads_to_add, pads_to_remove);
}

static void
mpegts_parse_free_stream (MpegTSParseStream * stream)
{
  g_free (stream);
}

static void
mpegts_parse_program_remove_stream (MpegTSParse * parse,
    MpegTSParseProgram * program, guint16 pid)
{
  g_hash_table_remove (program->streams, GINT_TO_POINTER ((gint) pid));
}

static MpegTSParsePad *
mpegts_parse_create_tspad (MpegTSParse * parse, const gchar * pad_name)
{
  GstPad *pad;
  MpegTSParsePad *tspad;

  pad = gst_pad_new_from_static_template (&program_template, pad_name);
  gst_pad_set_query_function (pad,
      GST_DEBUG_FUNCPTR (mpegts_parse_src_pad_query));

  /* create our wrapper */
  tspad = g_new0 (MpegTSParsePad, 1);
  tspad->pad = pad;
  tspad->program_number = -1;
  tspad->program = NULL;
  tspad->pushed = FALSE;
  tspad->flow_return = GST_FLOW_NOT_LINKED;
  gst_pad_set_element_private (pad, tspad);

  return tspad;
}

static void
mpegts_parse_destroy_tspad (MpegTSParse * parse, MpegTSParsePad * tspad)
{
  /* free the wrapper */
  g_free (tspad);
}

static void
mpegts_parse_pad_removed (GstElement * element, GstPad * pad)
{
  MpegTSParsePad *tspad;
  MpegTSParse *parse = GST_MPEGTS_PARSE (element);

  if (gst_pad_get_direction (pad) == GST_PAD_SINK)
    return;

  tspad = (MpegTSParsePad *) gst_pad_get_element_private (pad);
  mpegts_parse_destroy_tspad (parse, tspad);

  if (GST_ELEMENT_CLASS (parent_class)->pad_removed)
    GST_ELEMENT_CLASS (parent_class)->pad_removed (element, pad);
}

static GstPad *
mpegts_parse_request_new_pad (GstElement * element, GstPadTemplate * template,
    const gchar * unused)
{
  MpegTSParse *parse;
  gchar *name;
  GstPad *pad;

  g_return_val_if_fail (template != NULL, NULL);
  g_return_val_if_fail (GST_IS_MPEGTS_PARSE (element), NULL);

  parse = GST_MPEGTS_PARSE (element);

  GST_OBJECT_LOCK (element);
  name = g_strdup_printf ("src%d", parse->req_pads++);
  GST_OBJECT_UNLOCK (element);

  pad = mpegts_parse_create_tspad (parse, name)->pad;
  gst_pad_set_active (pad, TRUE);
  gst_element_add_pad (element, pad);
  g_free (name);

  return pad;
}

static void
mpegts_parse_release_pad (GstElement * element, GstPad * pad)
{
  g_return_if_fail (GST_IS_MPEGTS_PARSE (element));

  gst_pad_set_active (pad, FALSE);
  /* we do the cleanup in GstElement::pad-removed */
  gst_element_remove_pad (element, pad);
}

static GstFlowReturn
mpegts_parse_tspad_push_section (MpegTSParse * parse, MpegTSParsePad * tspad,
    MpegTSPacketizerSection * section, GstBuffer * buffer)
{
  GstFlowReturn ret = GST_FLOW_NOT_LINKED;
  gboolean to_push = TRUE;

  if (tspad->program_number != -1) {
    if (tspad->program) {
      /* we push all sections to all pads except PMTs which we
       * only push to pads meant to receive that program number */
      if (section->table_id == 0x02) {
        /* PMT */
        if (section->subtable_extension != tspad->program_number)
          to_push = FALSE;
      }
    } else {
      /* there's a program filter on the pad but the PMT for the program has not
       * been parsed yet, ignore the pad until we get a PMT */
      to_push = FALSE;
      ret = GST_FLOW_OK;
    }
  }
  GST_DEBUG_OBJECT (parse,
      "pushing section: %d program number: %d table_id: %d", to_push,
      tspad->program_number, section->table_id);
  if (to_push) {
    ret = gst_pad_push (tspad->pad, buffer);
  } else {
    gst_buffer_unref (buffer);
    if (gst_pad_is_linked (tspad->pad))
      ret = GST_FLOW_OK;
  }

  return ret;
}

static GstFlowReturn
mpegts_parse_tspad_push (MpegTSParse * parse, MpegTSParsePad * tspad,
    guint16 pid, GstBuffer * buffer)
{
  GstFlowReturn ret = GST_FLOW_NOT_LINKED;
  GHashTable *pad_pids = NULL;

  if (tspad->program_number != -1) {
    if (tspad->program) {
      pad_pids = tspad->program->streams;
    } else {
      /* there's a program filter on the pad but the PMT for the program has not
       * been parsed yet, ignore the pad until we get a PMT */
      gst_buffer_unref (buffer);
      ret = GST_FLOW_OK;
      goto out;
    }
  }

  if (pad_pids == NULL ||
      g_hash_table_lookup (pad_pids, GINT_TO_POINTER ((gint) pid)) != NULL) {
    /* push if there's no filter or if the pid is in the filter */
    ret = gst_pad_push (tspad->pad, buffer);
  } else {
    gst_buffer_unref (buffer);
    if (gst_pad_is_linked (tspad->pad))
      ret = GST_FLOW_OK;
  }

out:
  return ret;
}

static void
pad_clear_for_push (GstPad * pad, MpegTSParse * parse)
{
  MpegTSParsePad *tspad = (MpegTSParsePad *) gst_pad_get_element_private (pad);

  tspad->flow_return = GST_FLOW_NOT_LINKED;
  tspad->pushed = FALSE;
}

static GstFlowReturn
mpegts_parse_push (MpegTSParse * parse, MpegTSPacketizerPacket * packet,
    MpegTSPacketizerSection * section)
{
  GstIterator *iterator;
  gboolean done = FALSE;
  gpointer pad = NULL;
  MpegTSParsePad *tspad;
  guint16 pid;
  GstBuffer *buffer;
  GstFlowReturn ret;
  GstCaps *caps;

  pid = packet->pid;
  buffer = packet->buffer;
  /* we have the same caps on all the src pads */
  caps = gst_static_pad_template_get_caps (&src_template);
  gst_buffer_set_caps (buffer, caps);
  gst_caps_unref (caps);

  GST_OBJECT_LOCK (parse);
  /* clear tspad->pushed on pads */
  g_list_foreach (GST_ELEMENT_CAST (parse)->srcpads,
      (GFunc) pad_clear_for_push, parse);
  if (GST_ELEMENT_CAST (parse)->srcpads)
    ret = GST_FLOW_NOT_LINKED;
  else
    ret = GST_FLOW_OK;
  GST_OBJECT_UNLOCK (parse);

  iterator = gst_element_iterate_src_pads (GST_ELEMENT_CAST (parse));
  while (!done) {
    switch (gst_iterator_next (iterator, &pad)) {
      case GST_ITERATOR_OK:
        tspad = gst_pad_get_element_private (GST_PAD (pad));

        /* make sure to push only once if the iterator resyncs */
        if (!tspad->pushed) {
          /* ref the buffer as gst_pad_push takes a ref but we want to reuse the
           * same buffer for next pushes */
          gst_buffer_ref (buffer);
          if (section) {
            tspad->flow_return =
                mpegts_parse_tspad_push_section (parse, tspad, section, buffer);
          } else {
            tspad->flow_return =
                mpegts_parse_tspad_push (parse, tspad, pid, buffer);
          }
          tspad->pushed = TRUE;

          if (GST_FLOW_IS_FATAL (tspad->flow_return)) {
            /* return the error upstream */
            ret = tspad->flow_return;
            done = TRUE;
          }
        }

        if (ret == GST_FLOW_NOT_LINKED)
          ret = tspad->flow_return;

        /* the iterator refs the pad */
        g_object_unref (GST_PAD (pad));
        break;
      case GST_ITERATOR_RESYNC:
        gst_iterator_resync (iterator);
        break;
      case GST_ITERATOR_DONE:
        done = TRUE;
        break;
      default:
        g_warning ("this should not be reached");
    }
  }

  gst_iterator_free (iterator);

  gst_buffer_unref (buffer);
  packet->buffer = NULL;

  return ret;
}

static gboolean
mpegts_parse_is_psi (MpegTSParse * parse, MpegTSPacketizerPacket * packet)
{
  gboolean retval = FALSE;
  guint8 table_id;
  int i;
  guint8 si_tables[] = { 0x00, 0x01, 0x02, 0x03, 0x40, 0x41, 0x42, 0x46, 0x4A,
    0x4E, 0x4F, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
    0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65,
    0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70, 0x71,
    0x72, 0x73, 0x7E, 0x7F, TABLE_ID_UNSET
  };
  if (g_hash_table_lookup (parse->psi_pids,
          GINT_TO_POINTER ((gint) packet->pid)) != NULL)
    retval = TRUE;
  if (!retval) {
    if (packet->payload_unit_start_indicator) {
      table_id = *(packet->data);
      i = 0;
      while (si_tables[i] != TABLE_ID_UNSET) {
        if (si_tables[i] == table_id) {
          GST_DEBUG_OBJECT (parse, "Packet has table id 0x%x", table_id);
          retval = TRUE;
          break;
        }
        i++;
      }
    } else {
      MpegTSPacketizerStream *stream = (MpegTSPacketizerStream *)
          g_hash_table_lookup (parse->packetizer->streams,
          GINT_TO_POINTER ((gint) packet->pid));

      if (stream) {
        i = 0;
        GST_DEBUG_OBJECT (parse, "section table id: 0x%x",
            stream->section_table_id);
        while (si_tables[i] != TABLE_ID_UNSET) {
          if (si_tables[i] == stream->section_table_id) {
            retval = TRUE;
            break;
          }
          i++;
        }
      }
    }
  }
  GST_DEBUG_OBJECT (parse, "Packet of pid 0x%x is psi: %d", packet->pid,
      retval);
  return retval;
}

static void
mpegts_parse_apply_pat (MpegTSParse * parse, GstStructure * pat_info)
{
  const GValue *value;
  GstStructure *old_pat;
  GstStructure *program_info;
  guint program_number;
  guint pid;
  MpegTSParseProgram *program;
  gint i;
  GList *pads_to_add = NULL;
  GList *pads_to_remove = NULL;
  const GValue *programs;
  gchar *dbg;

  old_pat = parse->pat;
  parse->pat = gst_structure_copy (pat_info);

  dbg = gst_structure_to_string (pat_info);
  GST_INFO_OBJECT (parse, "PAT %s", dbg);
  g_free (dbg);

  gst_element_post_message (GST_ELEMENT_CAST (parse),
      gst_message_new_element (GST_OBJECT (parse),
          gst_structure_copy (pat_info)));

  GST_OBJECT_LOCK (parse);
  programs = gst_structure_get_value (pat_info, "programs");
  /* activate the new table */
  for (i = 0; i < gst_value_list_get_size (programs); ++i) {
    value = gst_value_list_get_value (programs, i);

    program_info = g_value_get_boxed (value);
    gst_structure_get_uint (program_info, "program-number", &program_number);
    gst_structure_get_uint (program_info, "pid", &pid);

    program = mpegts_parse_get_program (parse, program_number);
    if (program) {
      if (program->pmt_pid != pid) {
        if (program->pmt_pid != G_MAXUINT16) {
          /* pmt pid changed */
          g_hash_table_remove (parse->psi_pids,
              GINT_TO_POINTER ((gint) program->pmt_pid));
        }

        program->pmt_pid = pid;
        g_hash_table_insert (parse->psi_pids,
            GINT_TO_POINTER ((gint) pid), GINT_TO_POINTER (1));
      }

      program->patcount += 1;
    } else {
      g_hash_table_insert (parse->psi_pids,
          GINT_TO_POINTER ((gint) pid), GINT_TO_POINTER (1));
      program = mpegts_parse_add_program (parse, program_number, pid);
    }

    if (program->selected && !program->active)
      parse->pads_to_add = g_list_append (parse->pads_to_add,
          mpegts_parse_activate_program (parse, program));
  }

  if (old_pat) {
    /* deactivate the old table */

    programs = gst_structure_get_value (old_pat, "programs");
    for (i = 0; i < gst_value_list_get_size (programs); ++i) {
      value = gst_value_list_get_value (programs, i);

      program_info = g_value_get_boxed (value);
      gst_structure_get_uint (program_info, "program-number", &program_number);
      gst_structure_get_uint (program_info, "pid", &pid);

      program = mpegts_parse_get_program (parse, program_number);
      if (program == NULL) {
        GST_DEBUG_OBJECT (parse, "broken PAT, duplicated entry for program %d",
            program_number);
        continue;
      }

      if (--program->patcount > 0)
        /* the program has been referenced by the new pat, keep it */
        continue;

      {
        gchar *dbg = gst_structure_to_string (program_info);

        GST_INFO_OBJECT (parse, "PAT removing program %s", dbg);
        g_free (dbg);
      }

      if (program->active)
        parse->pads_to_remove = g_list_append (parse->pads_to_remove,
            mpegts_parse_deactivate_program (parse, program));

      mpegts_parse_remove_program (parse, program_number);
      g_hash_table_remove (parse->psi_pids, GINT_TO_POINTER ((gint) pid));
    }

    gst_structure_free (old_pat);
  }

  pads_to_add = parse->pads_to_add;
  parse->pads_to_add = NULL;
  pads_to_remove = parse->pads_to_remove;
  parse->pads_to_remove = NULL;
  GST_OBJECT_UNLOCK (parse);

  mpegts_parse_sync_program_pads (parse, pads_to_add, pads_to_remove);
}

static void
mpegts_parse_apply_pmt (MpegTSParse * parse,
    guint16 pmt_pid, GstStructure * pmt_info)
{
  MpegTSParseProgram *program;
  guint program_number;
  guint pcr_pid;
  guint pid;
  guint stream_type;
  GstStructure *stream;
  gint i;
  const GValue *old_streams;
  const GValue *new_streams;
  const GValue *value;

  gst_structure_get_uint (pmt_info, "program-number", &program_number);
  gst_structure_get_uint (pmt_info, "pcr-pid", &pcr_pid);
  new_streams = gst_structure_get_value (pmt_info, "streams");

  GST_OBJECT_LOCK (parse);
  program = mpegts_parse_get_program (parse, program_number);
  if (program) {
    if (program->pmt_info) {
      /* deactivate old pmt */
      old_streams = gst_structure_get_value (program->pmt_info, "streams");

      for (i = 0; i < gst_value_list_get_size (old_streams); ++i) {
        value = gst_value_list_get_value (old_streams, i);
        stream = g_value_get_boxed (value);
        gst_structure_get_uint (stream, "pid", &pid);
        gst_structure_get_uint (stream, "stream-type", &stream_type);
        mpegts_parse_program_remove_stream (parse, program, (guint16) pid);
      }

      /* remove pcr stream */
      mpegts_parse_program_remove_stream (parse, program, program->pcr_pid);

      gst_structure_free (program->pmt_info);
      program->pmt_info = NULL;
    }
  } else {
    /* no PAT?? */
    g_hash_table_insert (parse->psi_pids,
        GINT_TO_POINTER ((gint) pmt_pid), GINT_TO_POINTER (1));
    program = mpegts_parse_add_program (parse, program_number, pid);
  }

  /* activate new pmt */
  program->pmt_info = gst_structure_copy (pmt_info);
  program->pmt_pid = pmt_pid;
  program->pcr_pid = pcr_pid;
  mpegts_parse_program_add_stream (parse, program, (guint16) pcr_pid, -1);

  for (i = 0; i < gst_value_list_get_size (new_streams); ++i) {
    value = gst_value_list_get_value (new_streams, i);
    stream = g_value_get_boxed (value);

    gst_structure_get_uint (stream, "pid", &pid);
    gst_structure_get_uint (stream, "stream-type", &stream_type);
    mpegts_parse_program_add_stream (parse, program,
        (guint16) pid, (guint8) stream_type);
  }
  GST_OBJECT_UNLOCK (parse);

  {
    gchar *dbg = gst_structure_to_string (pmt_info);

    GST_DEBUG_OBJECT (parse, "new pmt %s", dbg);
    g_free (dbg);
  }

  gst_element_post_message (GST_ELEMENT_CAST (parse),
      gst_message_new_element (GST_OBJECT (parse),
          gst_structure_copy (pmt_info)));
}

static void
mpegts_parse_apply_nit (MpegTSParse * parse,
    guint16 pmt_pid, GstStructure * nit_info)
{
  gst_element_post_message (GST_ELEMENT_CAST (parse),
      gst_message_new_element (GST_OBJECT (parse),
          gst_structure_copy (nit_info)));
}

static void
mpegts_parse_apply_sdt (MpegTSParse * parse,
    guint16 pmt_pid, GstStructure * sdt_info)
{
  gst_element_post_message (GST_ELEMENT_CAST (parse),
      gst_message_new_element (GST_OBJECT (parse),
          gst_structure_copy (sdt_info)));
}

static void
mpegts_parse_apply_eit (MpegTSParse * parse,
    guint16 pmt_pid, GstStructure * eit_info)
{
  gst_element_post_message (GST_ELEMENT_CAST (parse),
      gst_message_new_element (GST_OBJECT (parse),
          gst_structure_copy (eit_info)));
}

static gboolean
mpegts_parse_handle_psi (MpegTSParse * parse, MpegTSPacketizerSection * section)
{
  gboolean res = TRUE;
  GstStructure *structure = NULL;

  if (mpegts_parse_calc_crc32 (GST_BUFFER_DATA (section->buffer),
          GST_BUFFER_SIZE (section->buffer)) != 0) {
    GST_WARNING_OBJECT (parse, "bad crc in psi pid 0x%x", section->pid);
    return FALSE;
  }

  switch (section->table_id) {
    case 0x00:
      /* PAT */
      structure = mpegts_packetizer_parse_pat (parse->packetizer, section);
      if (structure)
        mpegts_parse_apply_pat (parse, structure);
      else
        res = FALSE;

      break;
    case 0x02:
      structure = mpegts_packetizer_parse_pmt (parse->packetizer, section);
      if (structure)
        mpegts_parse_apply_pmt (parse, section->pid, structure);
      else
        res = FALSE;

      break;
    case 0x40:
      /* NIT, actual network */
    case 0x41:
      /* NIT, other network */
      structure = mpegts_packetizer_parse_nit (parse->packetizer, section);
      if (structure)
        mpegts_parse_apply_nit (parse, section->pid, structure);
      else
        res = FALSE;

      break;
    case 0x42:
    case 0x46:
      structure = mpegts_packetizer_parse_sdt (parse->packetizer, section);
      if (structure)
        mpegts_parse_apply_sdt (parse, section->pid, structure);
      else
        res = FALSE;
      break;
    case 0x4E:
    case 0x4F:
      /* EIT, present/following */
    case 0x50:
    case 0x51:
    case 0x52:
    case 0x53:
    case 0x54:
    case 0x55:
    case 0x56:
    case 0x57:
    case 0x58:
    case 0x59:
    case 0x5A:
    case 0x5B:
    case 0x5C:
    case 0x5D:
    case 0x5E:
    case 0x5F:
    case 0x60:
    case 0x61:
    case 0x62:
    case 0x63:
    case 0x64:
    case 0x65:
    case 0x66:
    case 0x67:
    case 0x68:
    case 0x69:
    case 0x6A:
    case 0x6B:
    case 0x6C:
    case 0x6D:
    case 0x6E:
    case 0x6F:
      /* EIT, schedule */
      structure = mpegts_packetizer_parse_eit (parse->packetizer, section);
      if (structure)
        mpegts_parse_apply_eit (parse, section->pid, structure);
      else
        res = FALSE;
      break;
    default:
      break;
  }

  if (structure)
    gst_structure_free (structure);

  return res;
}

static gboolean
mpegts_parse_sink_event (GstPad * pad, GstEvent * event)
{
  gboolean res;
  MpegTSParse *parse =
      GST_MPEGTS_PARSE (gst_object_get_parent (GST_OBJECT (pad)));

  switch (GST_EVENT_TYPE (event)) {
    case GST_EVENT_FLUSH_STOP:
      mpegts_packetizer_clear (parse->packetizer);
      res = gst_pad_event_default (pad, event);
      break;
    default:
      res = gst_pad_event_default (pad, event);
  }

  gst_object_unref (parse);
  return res;
}

static GstFlowReturn
mpegts_parse_chain (GstPad * pad, GstBuffer * buf)
{
  GstFlowReturn res = GST_FLOW_OK;
  MpegTSParse *parse;
  gboolean parsed;
  MpegTSPacketizer *packetizer;
  MpegTSPacketizerPacket packet;

  parse = GST_MPEGTS_PARSE (gst_object_get_parent (GST_OBJECT (pad)));
  packetizer = parse->packetizer;

  mpegts_packetizer_push (parse->packetizer, buf);
  while (mpegts_packetizer_has_packets (parse->packetizer) &&
      !GST_FLOW_IS_FATAL (res)) {
    /* get the next packet */
    parsed = mpegts_packetizer_next_packet (packetizer, &packet);
    if (!parsed)
      /* bad header, skip the packet */
      goto next;

    /* parse PSI data */
    if (packet.payload != NULL && mpegts_parse_is_psi (parse, &packet)) {
      MpegTSPacketizerSection section;

      parsed = mpegts_packetizer_push_section (packetizer, &packet, &section);
      if (!parsed)
        /* bad section data */
        goto next;

      if (section.complete) {
        /* section complete */
        parsed = mpegts_parse_handle_psi (parse, &section);
        gst_buffer_unref (section.buffer);

        if (!parsed)
          /* bad PSI table */
          goto next;
      }
      /* we need to push section packet downstream */
      res = mpegts_parse_push (parse, &packet, &section);

    } else {
      /* push the packet downstream */
      res = mpegts_parse_push (parse, &packet, NULL);
    }

  next:
    mpegts_packetizer_clear_packet (parse->packetizer, &packet);
  }

  gst_object_unref (parse);
  return res;
}

static GstStateChangeReturn
mpegts_parse_change_state (GstElement * element, GstStateChange transition)
{
  MpegTSParse *parse;
  GstStateChangeReturn ret;

  parse = GST_MPEGTS_PARSE (element);
  ret = GST_ELEMENT_CLASS (parent_class)->change_state (element, transition);

  switch (transition) {
    case GST_STATE_CHANGE_PAUSED_TO_READY:
      mpegts_parse_reset (parse);
      break;
    default:
      break;
  }

  return ret;
}

static gboolean
mpegts_parse_src_pad_query (GstPad * pad, GstQuery * query)
{
  MpegTSParse *parse = GST_MPEGTS_PARSE (gst_pad_get_parent (pad));
  gboolean res;

  switch (GST_QUERY_TYPE (query)) {
    case GST_QUERY_LATENCY:
    {
      if ((res = gst_pad_peer_query (parse->sinkpad, query))) {
        gboolean is_live;
        GstClockTime min_latency, max_latency;

        gst_query_parse_latency (query, &is_live, &min_latency, &max_latency);
        if (is_live) {
          min_latency += TS_LATENCY * GST_MSECOND;
          if (max_latency != GST_CLOCK_TIME_NONE)
            max_latency += TS_LATENCY * GST_MSECOND;
        }

        gst_query_set_latency (query, is_live, min_latency, max_latency);
      }

      break;
    }
    default:
      res = gst_pad_query_default (pad, query);
  }

  return res;
}

gboolean
gst_mpegtsparse_plugin_init (GstPlugin * plugin)
{
  GST_DEBUG_CATEGORY_INIT (mpegts_parse_debug, "mpegtsparse", 0,
      "MPEG transport stream parser");

  mpegts_packetizer_init_debug ();
  gst_mpegtsdesc_init_debug ();

  return gst_element_register (plugin, "mpegtsparse",
      GST_RANK_NONE, GST_TYPE_MPEGTS_PARSE);
}
