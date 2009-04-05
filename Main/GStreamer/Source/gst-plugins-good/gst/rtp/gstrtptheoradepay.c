/* GStreamer
 * Copyright (C) <2006> Wim Taymans <wim.taymans@gmail.com>
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
#  include "config.h"
#endif

#include <gst/tag/tag.h>
#include <gst/rtp/gstrtpbuffer.h>

#include <string.h>
#include "gstrtptheoradepay.h"

GST_DEBUG_CATEGORY_STATIC (rtptheoradepay_debug);
#define GST_CAT_DEFAULT (rtptheoradepay_debug)

/* elementfactory information */
static const GstElementDetails gst_rtp_theora_depay_details =
GST_ELEMENT_DETAILS ("RTP Theora depayloader",
    "Codec/Depayloader/Network",
    "Extracts Theora video from RTP packets (draft-01 of RFC XXXX)",
    "Wim Taymans <wim.taymans@gmail.com>");

static GstStaticPadTemplate gst_rtp_theora_depay_sink_template =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("application/x-rtp, "
        "media = (string) \"video\", "
        "payload = (int) " GST_RTP_PAYLOAD_DYNAMIC_STRING ", "
        "clock-rate = (int) 90000, " "encoding-name = (string) \"THEORA\","
        /* only support inline delivery */
        "delivery-method = (string) \"inline\""
        /* All required parameters 
         *
         * "sampling = (string) { "YCbCr-4:2:0", "YCbCr-4:2:2", "YCbCr-4:4:4" } "
         * "width = (string) [1, 1048561] (multiples of 16) "
         * "height = (string) [1, 1048561] (multiples of 16) "
         * "delivery-method = (string) { inline, in_band, out_band/<specific_name> } " 
         * "configuration = (string) ANY" 
         */
        /* All optional parameters
         *
         * "configuration-uri =" 
         */
    )
    );

static GstStaticPadTemplate gst_rtp_theora_depay_src_template =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("video/x-theora")
    );

GST_BOILERPLATE (GstRtpTheoraDepay, gst_rtp_theora_depay, GstBaseRTPDepayload,
    GST_TYPE_BASE_RTP_DEPAYLOAD);

static gboolean gst_rtp_theora_depay_setcaps (GstBaseRTPDepayload * depayload,
    GstCaps * caps);
static GstBuffer *gst_rtp_theora_depay_process (GstBaseRTPDepayload * depayload,
    GstBuffer * buf);

static void gst_rtp_theora_depay_finalize (GObject * object);

static GstStateChangeReturn gst_rtp_theora_depay_change_state (GstElement *
    element, GstStateChange transition);


static void
gst_rtp_theora_depay_base_init (gpointer klass)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_rtp_theora_depay_sink_template));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_rtp_theora_depay_src_template));

  gst_element_class_set_details (element_class, &gst_rtp_theora_depay_details);
}

static void
gst_rtp_theora_depay_class_init (GstRtpTheoraDepayClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;
  GstBaseRTPDepayloadClass *gstbasertpdepayload_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;
  gstbasertpdepayload_class = (GstBaseRTPDepayloadClass *) klass;

  gobject_class->finalize = gst_rtp_theora_depay_finalize;

  gstelement_class->change_state = gst_rtp_theora_depay_change_state;

  gstbasertpdepayload_class->process = gst_rtp_theora_depay_process;
  gstbasertpdepayload_class->set_caps = gst_rtp_theora_depay_setcaps;

  GST_DEBUG_CATEGORY_INIT (rtptheoradepay_debug, "rtptheoradepay", 0,
      "Theora RTP Depayloader");
}

static void
gst_rtp_theora_depay_init (GstRtpTheoraDepay * rtptheoradepay,
    GstRtpTheoraDepayClass * klass)
{
  rtptheoradepay->adapter = gst_adapter_new ();
}
static void
gst_rtp_theora_depay_finalize (GObject * object)
{
  GstRtpTheoraDepay *rtptheoradepay = GST_RTP_THEORA_DEPAY (object);

  g_object_unref (rtptheoradepay->adapter);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static const guint8 a2bin[256] = {
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
  52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
  64, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
  15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
  64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
  41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
};

static guint
decode_base64 (const gchar * in, guint8 * out)
{
  guint8 v1, v2;
  guint len = 0;

  v1 = a2bin[(gint) * in];
  while (v1 <= 63) {
    /* read 4 bytes, write 3 bytes, invalid base64 are zeroes */
    v2 = a2bin[(gint) * ++in];
    *out++ = (v1 << 2) | ((v2 & 0x3f) >> 4);
    v1 = (v2 > 63 ? 64 : a2bin[(gint) * ++in]);
    *out++ = (v2 << 4) | ((v1 & 0x3f) >> 2);
    v2 = (v1 > 63 ? 64 : a2bin[(gint) * ++in]);
    *out++ = (v1 << 6) | (v2 & 0x3f);
    v1 = (v2 > 63 ? 64 : a2bin[(gint) * ++in]);
    len += 3;
  }
  /* move to '\0' */
  while (*in != '\0')
    in++;

  /* subtract padding */
  while (len > 0 && *--in == '=')
    len--;

  return len;
}

static gboolean
gst_rtp_theora_depay_parse_configuration (GstRtpTheoraDepay * rtptheoradepay,
    const gchar * configuration)
{
  GstBuffer *buf;
  guint32 num_headers;
  guint8 *data;
  guint size;
  gint i, j;

  /* deserialize base64 to buffer */
  size = strlen (configuration);
  GST_DEBUG_OBJECT (rtptheoradepay, "base64 config size %u", size);

  data = g_malloc (size);
  size = decode_base64 (configuration, data);

  GST_DEBUG_OBJECT (rtptheoradepay, "config size %u", size);

  /* +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   * |                     Number of packed headers                  |
   * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   * |                          Packed header                        |
   * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   * |                          Packed header                        |
   * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   * |                          ....                                 |
   * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   */
  if (size < 4)
    goto too_small;

  num_headers = GST_READ_UINT32_BE (data);
  size -= 4;
  data += 4;

  GST_DEBUG_OBJECT (rtptheoradepay, "have %u headers", num_headers);

  /*  0                   1                   2                   3
   *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   * |                   Ident                       | length       ..
   * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   * ..              | n. of headers |    length1    |    length2   ..
   * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   * ..              |             Identification Header            ..
   * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   * .................................................................
   * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   * ..              |         Comment Header                       ..
   * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   * .................................................................
   * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   * ..                        Comment Header                        |
   * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   * |                          Setup Header                        ..
   * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   * .................................................................
   * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   * ..                         Setup Header                         |
   * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   */
  for (i = 0; i < num_headers; i++) {
    guint32 ident;
    guint16 length;
    guint8 n_headers, b;
    GstRtpTheoraConfig *conf;
    guint *h_sizes;

    if (size < 6)
      goto too_small;

    ident = (data[0] << 16) | (data[1] << 8) | data[2];
    length = (data[3] << 8) | data[4];
    n_headers = data[5];
    size -= 6;
    data += 6;

    GST_DEBUG_OBJECT (rtptheoradepay,
        "header %d, ident 0x%08x, length %u, left %u", i, ident, length, size);

    if (size < length)
      goto too_small;

    /* read header sizes we read 2 sizes, the third size (for which we allocate
     * space) must be derived from the total packed header length. */
    h_sizes = g_newa (guint, n_headers + 1);
    for (j = 0; j < n_headers; j++) {
      guint h_size;

      h_size = 0;
      do {
        if (size < 1)
          goto too_small;
        b = *data++;
        size--;
        h_size = (h_size << 7) | (b & 0x7f);
      } while (b & 0x80);
      GST_DEBUG_OBJECT (rtptheoradepay, "headers %d: size: %u", j, h_size);
      h_sizes[j] = h_size;
      length -= h_size;
    }
    /* last header length is the remaining space */
    GST_DEBUG_OBJECT (rtptheoradepay, "last header size: %u", length);
    h_sizes[j] = length;

    GST_DEBUG_OBJECT (rtptheoradepay, "preparing headers");
    conf = g_new0 (GstRtpTheoraConfig, 1);
    conf->ident = ident;

    for (j = 0; j <= n_headers; j++) {
      guint h_size;

      h_size = h_sizes[j];
      if (size < h_size)
        goto too_small;

      GST_DEBUG_OBJECT (rtptheoradepay, "reading header %d, size %u", j,
          h_size);

      buf = gst_buffer_new_and_alloc (h_size);
      memcpy (GST_BUFFER_DATA (buf), data, h_size);
      conf->headers = g_list_append (conf->headers, buf);
      data += h_size;
      size -= h_size;
    }
    rtptheoradepay->configs = g_list_append (rtptheoradepay->configs, conf);
  }
  return TRUE;

  /* ERRORS */
too_small:
  {
    GST_DEBUG_OBJECT (rtptheoradepay, "configuration too small");
    return FALSE;
  }
}

static gboolean
gst_rtp_theora_depay_setcaps (GstBaseRTPDepayload * depayload, GstCaps * caps)
{
  GstStructure *structure;
  GstRtpTheoraDepay *rtptheoradepay;
  GstCaps *srccaps;
  const gchar *delivery_method;
  const gchar *configuration;
  gboolean res;

  rtptheoradepay = GST_RTP_THEORA_DEPAY (depayload);

  structure = gst_caps_get_structure (caps, 0);

  /* see how the configuration parameters will be transmitted */
  delivery_method = gst_structure_get_string (structure, "delivery-method");
  if (delivery_method == NULL)
    goto no_delivery_method;

  if (!g_ascii_strcasecmp (delivery_method, "inline")) {
    /* configure string is in the caps */
  } else if (!g_ascii_strcasecmp (delivery_method, "in_band")) {
    /* headers will (also) be transmitted in the RTP packets */
    goto unsupported_delivery_method;
  } else if (g_str_has_prefix (delivery_method, "out_band/")) {
    /* some other method of header delivery. */
    goto unsupported_delivery_method;
  } else
    goto unsupported_delivery_method;

  /* read and parse configuration string */
  configuration = gst_structure_get_string (structure, "configuration");
  if (configuration == NULL)
    goto no_configuration;

  if (!gst_rtp_theora_depay_parse_configuration (rtptheoradepay, configuration))
    goto invalid_configuration;

  /* set caps on pad and on header */
  srccaps = gst_caps_new_simple ("video/x-theora", NULL);
  res = gst_pad_set_caps (depayload->srcpad, srccaps);
  gst_caps_unref (srccaps);

  /* Clock rate is always 90000 according to draft-barbato-avt-rtp-theora-01 */
  depayload->clock_rate = 90000;

  return res;

  /* ERRORS */
unsupported_delivery_method:
  {
    GST_ERROR_OBJECT (rtptheoradepay,
        "unsupported delivery-method \"%s\" specified", delivery_method);
    return FALSE;
  }
no_delivery_method:
  {
    GST_ERROR_OBJECT (rtptheoradepay, "no delivery-method specified");
    return FALSE;
  }
no_configuration:
  {
    GST_ERROR_OBJECT (rtptheoradepay, "no configuration specified");
    return FALSE;
  }
invalid_configuration:
  {
    GST_ERROR_OBJECT (rtptheoradepay, "invalid configuration specified");
    return FALSE;
  }
}

static gboolean
gst_rtp_theora_depay_switch_codebook (GstRtpTheoraDepay * rtptheoradepay,
    guint32 ident)
{
  GList *walk;
  gboolean res = FALSE;

  for (walk = rtptheoradepay->configs; walk; walk = g_list_next (walk)) {
    GstRtpTheoraConfig *conf = (GstRtpTheoraConfig *) walk->data;

    if (conf->ident == ident) {
      GList *headers;

      /* FIXME, remove pads, create new pad.. */

      /* push out all the headers */
      for (headers = conf->headers; headers; headers = g_list_next (headers)) {
        GstBuffer *header = GST_BUFFER_CAST (headers->data);

        gst_buffer_ref (header);
        gst_base_rtp_depayload_push (GST_BASE_RTP_DEPAYLOAD (rtptheoradepay),
            header);
      }
      /* remember the current config */
      rtptheoradepay->config = conf;
      res = TRUE;
    }
  }
  if (!res) {
    /* we don't know about the headers, figure out an alternative method for
     * getting the codebooks. FIXME, fail for now. */
  }
  return res;
}

static GstBuffer *
gst_rtp_theora_depay_process (GstBaseRTPDepayload * depayload, GstBuffer * buf)
{
  GstRtpTheoraDepay *rtptheoradepay;
  GstBuffer *outbuf;
  GstFlowReturn ret;
  gint payload_len;
  guint8 *payload, *to_free = NULL;
  guint32 timestamp;
  guint32 header, ident;
  guint8 F, TDT, packets;
  gboolean free_payload;

  rtptheoradepay = GST_RTP_THEORA_DEPAY (depayload);

  payload_len = gst_rtp_buffer_get_payload_len (buf);

  GST_DEBUG_OBJECT (depayload, "got RTP packet of size %d", payload_len);

  /* we need at least 4 bytes for the packet header */
  if (G_UNLIKELY (payload_len < 4))
    goto packet_short;

  payload = gst_rtp_buffer_get_payload (buf);
  free_payload = FALSE;

  header = GST_READ_UINT32_BE (payload);
  /*
   *  0                   1                   2                   3
   *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   * |                     Ident                     | F |TDT|# pkts.|
   * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   *
   * F: Fragment type (0=none, 1=start, 2=cont, 3=end)
   * TDT: Theora data type (0=theora, 1=config, 2=comment, 3=reserved)
   * pkts: number of packets.
   */
  TDT = (header & 0x30) >> 4;
  if (G_UNLIKELY (TDT == 3))
    goto ignore_reserved;

  ident = (header >> 8) & 0xffffff;
  F = (header & 0xc0) >> 6;
  packets = (header & 0xf);

  GST_DEBUG_OBJECT (depayload, "ident: 0x%08x, F: %d, TDT: %d, packets: %d",
      ident, F, TDT, packets);

  if (TDT == 0) {
    gboolean do_switch = FALSE;

    /* we have a raw payload, find the codebook for the ident */
    if (!rtptheoradepay->config) {
      /* we don't have an active codebook, find the codebook and
       * activate it */
      do_switch = TRUE;
    } else if (rtptheoradepay->config->ident != ident) {
      /* codebook changed */
      do_switch = TRUE;
    }
    if (do_switch) {
      if (!gst_rtp_theora_depay_switch_codebook (rtptheoradepay, ident))
        goto switch_failed;
    }
  }

  /* skip header */
  payload += 4;
  payload_len -= 4;

  /* fragmented packets, assemble */
  if (F != 0) {
    GstBuffer *vdata;
    guint headerskip;

    if (F == 1) {
      /* if we start a packet, clear adapter and start assembling. */
      gst_adapter_clear (rtptheoradepay->adapter);
      GST_DEBUG_OBJECT (depayload, "start assemble");
      rtptheoradepay->assembling = TRUE;
    }

    if (!rtptheoradepay->assembling)
      goto no_output;

    /* first assembled packet, reuse 2 bytes to store the length */
    headerskip = (F == 1 ? 4 : 6);
    /* skip header and length. */
    vdata = gst_rtp_buffer_get_payload_subbuffer (buf, headerskip, -1);

    GST_DEBUG_OBJECT (depayload, "assemble theora packet");
    gst_adapter_push (rtptheoradepay->adapter, vdata);

    /* packet is not complete, we are done */
    if (F != 3)
      goto no_output;

    /* construct assembled buffer */
    payload_len = gst_adapter_available (rtptheoradepay->adapter);
    payload = gst_adapter_take (rtptheoradepay->adapter, payload_len);
    /* fix the length */
    payload[0] = ((payload_len - 2) >> 8) & 0xff;
    payload[1] = (payload_len - 2) & 0xff;
    to_free = payload;
  }

  GST_DEBUG_OBJECT (depayload, "assemble done");

  /* we not assembling anymore now */
  rtptheoradepay->assembling = FALSE;
  gst_adapter_clear (rtptheoradepay->adapter);

  /* payload now points to a length with that many theora data bytes.
   * Iterate over the packets and send them out.
   *
   *  0                   1                   2                   3
   *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   * |             length            |          theora data         ..
   * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   * ..                        theora data                           |
   * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   * |            length             |   next theora packet data    ..
   * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   * ..                        theora data                           |
   * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+*
   */
  timestamp = gst_rtp_buffer_get_timestamp (buf);

  while (payload_len > 2) {
    guint16 length;

    length = GST_READ_UINT16_BE (payload);
    payload += 2;
    payload_len -= 2;

    GST_DEBUG_OBJECT (depayload, "read length %u, avail: %d", length,
        payload_len);

    /* skip packet if something odd happens */
    if (G_UNLIKELY (length > payload_len))
      goto length_short;

    /* create buffer for packet */
    if (G_UNLIKELY (to_free)) {
      outbuf = gst_buffer_new ();
      GST_BUFFER_DATA (outbuf) = payload;
      GST_BUFFER_MALLOCDATA (outbuf) = to_free;
      GST_BUFFER_SIZE (outbuf) = length;
      to_free = NULL;
    } else {
      outbuf = gst_buffer_new_and_alloc (length);
      memcpy (GST_BUFFER_DATA (outbuf), payload, length);
    }

    payload += length;
    payload_len -= length;

    if (timestamp != -1)
      /* push with timestamp of the last packet, which is the same timestamp that
       * should apply to the first assembled packet. */
      ret = gst_base_rtp_depayload_push_ts (depayload, timestamp, outbuf);
    else
      ret = gst_base_rtp_depayload_push (depayload, outbuf);

    if (ret != GST_FLOW_OK)
      break;

    /* make sure we don't set a timestamp on next buffers */
    timestamp = -1;
  }

  g_free (to_free);

  return NULL;

no_output:
  {
    return NULL;
  }
  /* ERORRS */
switch_failed:
  {
    GST_ELEMENT_ERROR (rtptheoradepay, STREAM, DECODE,
        (NULL), ("Could not switch codebooks"));
    return NULL;
  }
packet_short:
  {
    GST_ELEMENT_WARNING (rtptheoradepay, STREAM, DECODE,
        (NULL), ("Packet was too short (%d < 4)", payload_len));
    return NULL;
  }
ignore_reserved:
  {
    GST_WARNING_OBJECT (rtptheoradepay, "reserved TDT ignored");
    return NULL;
  }
length_short:
  {
    GST_ELEMENT_WARNING (rtptheoradepay, STREAM, DECODE,
        (NULL), ("Packet contains invalid data"));
    return NULL;
  }
}

static GstStateChangeReturn
gst_rtp_theora_depay_change_state (GstElement * element,
    GstStateChange transition)
{
  GstRtpTheoraDepay *rtptheoradepay;
  GstStateChangeReturn ret;

  rtptheoradepay = GST_RTP_THEORA_DEPAY (element);

  switch (transition) {
    case GST_STATE_CHANGE_NULL_TO_READY:
      break;
    case GST_STATE_CHANGE_READY_TO_PAUSED:
      break;
    default:
      break;
  }

  ret = GST_ELEMENT_CLASS (parent_class)->change_state (element, transition);

  switch (transition) {
    case GST_STATE_CHANGE_READY_TO_NULL:
      break;
    default:
      break;
  }
  return ret;
}

gboolean
gst_rtp_theora_depay_plugin_init (GstPlugin * plugin)
{
  return gst_element_register (plugin, "rtptheoradepay",
      GST_RANK_MARGINAL, GST_TYPE_RTP_THEORA_DEPAY);
}
