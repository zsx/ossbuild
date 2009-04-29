/* GStreamer
 * Copyright (C) 2005 Philippe Khalaf <burger@speedy.org>
 * Copyright (C) 2004 Tim Ringenbach <omarvo@hotmail.com>
 * Copyright (C) <1999> Erik Walthinsen <omega@cse.ogi.edu>
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
 *
 * Some code borrowed from the jp2topmn program of j2k, which is under the
 * following copyright:
 * Copyright (c) 2001-2002, David Janssens
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS `AS IS'
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gstj2kdec.h"
#include "gst/video/video.h"

/* elementfactory information */
GstElementDetails gst_jpeg2000dec_details = {
  "JPEG2000 image decoder",
  "Codec/Decoder/Image",
  "Decode images from JPEG2000 format",
  "Tim Ringenbach <marv_sf@users.sf.net>",
};

GST_DEBUG_CATEGORY(jpeg2000dec_debug);
#define GST_CAT_DEFAULT jpeg2000dec_debug

/* Jpeg2000Dec signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

enum
{
  ARG_0
  /* FILL ME */
};

static void gst_jpeg2000dec_base_init(gpointer g_class);
static void gst_jpeg2000dec_class_init(GstJpeg2000Dec *klass);
static void gst_jpeg2000dec_init(GstJpeg2000Dec *j2kdec);

static GstFlowReturn gst_jpeg2000dec_chain(GstPad *pad, GstBuffer *in);
static GstCaps *gst_jpeg2000dec_src_getcaps(GstPad *pad);
static gint calculate_loss(gint mask);
static gint calculate_shift(gint mask);

static GstElementClass *parent_class = NULL;

/*static guint gst_j2kdec_signals[LAST_SIGNAL] = { 0 }; */

GType
gst_jpeg2000dec_get_type(void)
{
  static GType jpeg2000dec_type = 0;

  if (!jpeg2000dec_type) {
    static const GTypeInfo jpeg2000dec_info = {
      sizeof (GstJpeg2000DecClass),
      gst_jpeg2000dec_base_init,
      NULL,
      (GClassInitFunc) gst_jpeg2000dec_class_init,
      NULL,
      NULL,
      sizeof (GstJpeg2000Dec),
      0,
      (GInstanceInitFunc) gst_jpeg2000dec_init,
    };

    jpeg2000dec_type =
        g_type_register_static(GST_TYPE_ELEMENT, "GstJpeg2000Dec", &jpeg2000dec_info,
        0);
  }
  return jpeg2000dec_type;
}

static GstStaticPadTemplate gst_jpeg2000dec_src_pad_template =
GST_STATIC_PAD_TEMPLATE("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("video/x-raw-rgb, "
        "bpp = (int) [16, 32], "
        "depth = (int) 24, "
        "endianness = (int) 4321, "
        "framerate = (double) [1.0, 30.0], "
        "red_mask = (int) 16711680, "
        "green_mask = (int) 65280, "
        "blue_mask = (int) 255, "
        "height = (int) [16, 4096], "
        "width = (int) [16, 4096]" 
    )
   );

static GstStaticPadTemplate gst_jpeg2000dec_sink_pad_template =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS("image/jp2, "
        "width = " GST_VIDEO_SIZE_RANGE ", "
        "height = " GST_VIDEO_SIZE_RANGE ", "
        "framerate = " GST_VIDEO_FPS_RANGE)
    );

static void
gst_jpeg2000dec_base_init(gpointer g_class)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS(g_class);

  gst_element_class_add_pad_template(element_class,
          gst_static_pad_template_get(&gst_jpeg2000dec_src_pad_template));
  gst_element_class_add_pad_template(element_class,
          gst_static_pad_template_get(&gst_jpeg2000dec_sink_pad_template));

  gst_element_class_set_details(element_class, &gst_jpeg2000dec_details);
}

static void
gst_jpeg2000dec_class_init(GstJpeg2000Dec *klass)
{
  GstElementClass *gstelement_class;
  GObjectClass *gobject_class;

  gstelement_class = (GstElementClass *) klass;
  gobject_class = (GObjectClass *) klass;

  parent_class = g_type_class_ref(GST_TYPE_ELEMENT);

  GST_DEBUG_CATEGORY_INIT(jpeg2000dec_debug, "jpeg2000dec", 0, "JPEG2000 decoder");
}

static void
gst_jpeg2000dec_init(GstJpeg2000Dec *j2kdec)
{
  GST_DEBUG("Initializing");
  /* create the sink and src pads */

  j2kdec->sinkpad = gst_pad_new_from_template(gst_static_pad_template_get(
              &gst_jpeg2000dec_sink_pad_template), "sink");
  gst_element_add_pad(GST_ELEMENT(j2kdec), j2kdec->sinkpad);
  gst_pad_set_chain_function(j2kdec->sinkpad, gst_jpeg2000dec_chain);


  j2kdec->srcpad = gst_pad_new_from_template(gst_static_pad_template_get(
              &gst_jpeg2000dec_src_pad_template), "src");
  gst_pad_set_getcaps_function (j2kdec->srcpad, gst_jpeg2000dec_src_getcaps); 
  gst_element_add_pad(GST_ELEMENT(j2kdec), j2kdec->srcpad);

  /* reset the initial video state */
  j2kdec->width = -1;
  j2kdec->height = -1;

  j2kdec->red_mask = 16711680;
  j2kdec->green_mask = 65280;
  j2kdec->blue_mask = 255;

  j2kdec->red_loss   = calculate_loss(j2kdec->red_mask);
  j2kdec->green_loss = calculate_loss(j2kdec->green_mask);
  j2kdec->blue_loss  = calculate_loss(j2kdec->blue_mask);

  j2kdec->red_shift   = calculate_shift(j2kdec->red_mask);
  j2kdec->green_shift = calculate_shift(j2kdec->green_mask);
  j2kdec->blue_shift  = calculate_shift(j2kdec->blue_mask);
}

static gint
calculate_loss(gint mask)
{
  gint bits = 0, i;

  for (i = 0; i < (sizeof(mask) * 8); i++)
    if (!(mask & 0x1))
      mask >>= 1;
    else
      break;

  for (; i < (sizeof(mask) * 8); i++)
    if (mask & 0x1) {
      bits++;
      mask >>= 1;
    } else {
      break;
    }

  return 8 - bits;
}

static gint
calculate_shift(gint mask)
{
  gint ret = 0, i;

  for (i = 0; i < (sizeof(mask) * 8); i++) {
    if (!(mask & 0x1)) {
      ret++;
      mask >>= 1;
    } else {
      break;
    }
  }

  return ret;
}

static GstCaps *
gst_jpeg2000dec_src_getcaps  (GstPad *pad)
{
  GstCaps *caps;

  GST_LOCK (pad);
  if (!(caps = GST_PAD_CAPS (pad)))
    caps = (GstCaps *) gst_pad_get_pad_template_caps (pad);
  caps = gst_caps_ref (caps);
  GST_UNLOCK (pad);

  return caps;
}

static int
ceildiv(int a, int b)
{
  return (a+b-1)/b;
}

static GstFlowReturn
gst_jpeg2000dec_chain(GstPad *pad, GstBuffer *buf)
{
  GstCaps *caps;
  GstJpeg2000Dec *j2kdec;
  guchar *data, *outdata;
  GstBuffer *outbuf;
  gint width, height, max, i, j;
  j2k_image_t *img;
  j2k_cp_t *cp;

  g_return_val_if_fail(pad != NULL, GST_FLOW_ERROR);
  g_return_val_if_fail(GST_IS_PAD(pad), GST_FLOW_ERROR);
  g_return_val_if_fail(buf != NULL, GST_FLOW_ERROR);

  j2kdec = GST_JPEG2000DEC(GST_OBJECT_PARENT(pad));

  if (!GST_PAD_IS_LINKED(j2kdec->srcpad)) {
    gst_buffer_unref(buf);
    return GST_FLOW_ERROR;
  }

  data = GST_BUFFER_DATA (buf);

  if (!j2k_decode(data, GST_BUFFER_SIZE(buf), &img, &cp)) {
      GST_DEBUG("Failed to decode image!");
      return GST_FLOW_ERROR;
  }

  GST_DEBUG("numcmps = %d", img->numcomps);
  if (img->numcomps==3 && img->comps[0].dx==img->comps[1].dx && img->comps[1].dx==img->comps[2].dx &&
          img->comps[0].dy==img->comps[1].dy && img->comps[1].dy==img->comps[2].dy &&
          img->comps[0].prec==img->comps[1].prec && img->comps[1].prec==img->comps[2].prec) {

      width = ceildiv(img->x1-img->x0, img->comps[0].dx);
      height = ceildiv(img->y1-img->y0, img->comps[0].dy);

      max = (1<<img->comps[0].prec)-1; /* What was this supposed to be and be for? */

      GST_DEBUG("width %d, height %d", width, height);

      outbuf = gst_buffer_new_and_alloc (width * height * 3);
      outdata = GST_BUFFER_DATA(outbuf);
      GST_BUFFER_TIMESTAMP(outbuf) = GST_BUFFER_TIMESTAMP(buf);

      int depth = 24;
      caps = gst_caps_new_simple ("video/x-raw-rgb",
              "bpp", G_TYPE_INT, depth,
              "depth", G_TYPE_INT, depth,
              "endianness", G_TYPE_INT, 4321,
              "framerate", G_TYPE_DOUBLE, 30.0,
              "red_mask", G_TYPE_INT, 16711680,
              "green_mask", G_TYPE_INT, 65280,
              "blue_mask", G_TYPE_INT, 255,
              "width", G_TYPE_INT, width,
              "height", G_TYPE_INT, height, NULL);
      gst_buffer_set_caps (outbuf, caps);

      switch (depth) {
          case 32:
              for (i = 0, j = 0; i < (width * height); i++) {
                  guint32 *tmp = (guint32 *) outdata;
                  tmp[i] = ((img->comps[0].data[i] >> j2kdec->red_loss) << j2kdec->red_shift)
                      | ((img->comps[1].data[i] >> j2kdec->green_loss) << j2kdec->green_shift)
                      | ((img->comps[2].data[i] >> j2kdec->blue_loss) << j2kdec->blue_shift);
              }
              break;
          case 24:
              for (i = 0, j = 0; i < (width * height); i++) {
                  /* this assumes 8:8:8 r:g:b and is probably wrong */
                  outdata[j++] = img->comps[0].data[i];
                  outdata[j++] = img->comps[1].data[i];
                  outdata[j++] = img->comps[2].data[i];
              }
              break;
          case 16:
              for (i = 0; i < (width * height); i++) {
                  ((guint16 *) outdata)[i] = (img->comps[0].data[i] >> j2kdec->red_loss) << j2kdec->red_shift;
                  ((guint16 *) outdata)[i] |= (img->comps[1].data[i] >> j2kdec->green_loss) << j2kdec->green_shift;
                  ((guint16 *) outdata)[i] |= (img->comps[2].data[i] >> j2kdec->blue_loss) << j2kdec->blue_shift;
              }
              break;
      }

      GST_DEBUG ("Pushing %d bytes", GST_BUFFER_SIZE(outbuf));
      gst_pad_push(j2kdec->srcpad, outbuf);
      j2k_release(img, cp);

  } else {
      GST_DEBUG("if failed!");
  }

  return GST_FLOW_OK;
}

gboolean
gst_jpeg2000dec_plugin_init (GstPlugin * plugin)
{
  return gst_element_register (plugin, "jpeg2000dec",
      GST_RANK_NONE, GST_TYPE_JPEG2000DEC);
}
