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
 * Some code borrowed from the pmn2jp2 program of j2k, which is under the
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
#include <string.h>
#include <math.h>

#include "gstj2kenc.h"
#include "gst/video/video.h"

/* elementfactory information */
GstElementDetails gst_jpeg2000enc_details = {
  "JPEG2000 image encoder",
  "Codec/Encoder/Image",
  "Encode images to JPEG2000 format",
  "Tim Ringenbach <omarvo@hotmail.com>",
};

GST_DEBUG_CATEGORY(jpeg2000enc_debug);
#define GST_CAT_DEFAULT jpeg2000enc_debug

/* Jpeg2000Enc signals and args */
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

static void gst_jpeg2000enc_base_init(gpointer g_class);
static void gst_jpeg2000enc_class_init(GstJpeg2000Enc *klass);
static void gst_jpeg2000enc_init(GstJpeg2000Enc *jpegenc);

static GstFlowReturn gst_jpeg2000enc_chain(GstPad *pad, GstBuffer *in);
static GstPadLinkReturn gst_jpeg2000enc_sink_setcaps(GstPad * pad, GstCaps * caps);

static GstElementClass *parent_class = NULL;

/*static guint gst_jpegenc_signals[LAST_SIGNAL] = { 0 }; */

GType
gst_jpeg2000enc_get_type(void)
{
  static GType jpeg2000enc_type = 0;

  if (!jpeg2000enc_type) {
    static const GTypeInfo jpeg2000enc_info = {
      sizeof (GstJpeg2000EncClass),
      gst_jpeg2000enc_base_init,
      NULL,
      (GClassInitFunc) gst_jpeg2000enc_class_init,
      NULL,
      NULL,
      sizeof (GstJpeg2000Enc),
      0,
      (GInstanceInitFunc) gst_jpeg2000enc_init,
    };

    jpeg2000enc_type =
        g_type_register_static(GST_TYPE_ELEMENT, "GstJpeg2000Enc", &jpeg2000enc_info,
        0);
  }
  return jpeg2000enc_type;
}


static GstStaticPadTemplate gst_jpeg2000enc_src_pad_template =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS("image/jp2, "
        "width = " GST_VIDEO_SIZE_RANGE ", "
        "height = " GST_VIDEO_SIZE_RANGE ", "
        "framerate = " GST_VIDEO_FPS_RANGE)
    );

static GstStaticPadTemplate gst_jpeg2000enc_sink_pad_template =
GST_STATIC_PAD_TEMPLATE("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS(GST_VIDEO_CAPS_RGBx)
    );

static void
gst_jpeg2000enc_base_init(gpointer g_class)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS(g_class);

  gst_element_class_add_pad_template(element_class,
                                     gst_static_pad_template_get(&gst_jpeg2000enc_src_pad_template));
  gst_element_class_add_pad_template(element_class,
                                     gst_static_pad_template_get(&gst_jpeg2000enc_sink_pad_template));
  gst_element_class_set_details(element_class, &gst_jpeg2000enc_details);
}

static void
gst_jpeg2000enc_class_init(GstJpeg2000Enc *klass)
{
  GstElementClass *gstelement_class;

  gstelement_class = (GstElementClass *) klass;

  parent_class = g_type_class_ref(GST_TYPE_ELEMENT);

  GST_DEBUG_CATEGORY_INIT(jpeg2000enc_debug, "jpeg2000enc", 0, "JPEG2000 encoder");
}

static void
gst_jpeg2000enc_init(GstJpeg2000Enc *j2kenc)
{
  GST_DEBUG("gst_jpeg2000enc_init: initializing");
  /* create the sink and src pads */

  j2kenc->sinkpad = gst_pad_new_from_template(gst_static_pad_template_get(
                                              &gst_jpeg2000enc_sink_pad_template), "sink");
  gst_element_add_pad(GST_ELEMENT(j2kenc), j2kenc->sinkpad);
  gst_pad_set_chain_function(j2kenc->sinkpad, gst_jpeg2000enc_chain);
  gst_pad_set_setcaps_function (j2kenc->sinkpad, gst_jpeg2000enc_sink_setcaps); 

  j2kenc->srcpad =
    gst_pad_new_from_template(gst_static_pad_template_get(&gst_jpeg2000enc_src_pad_template), "src");
  gst_element_add_pad(GST_ELEMENT(j2kenc), j2kenc->srcpad);

  /* reset the initial video state */
  j2kenc->width = -1;
  j2kenc->height = -1;
}

static GstPadLinkReturn gst_jpeg2000enc_sink_setcaps(GstPad * pad, GstCaps * caps)
{
  GstJpeg2000Enc *jpegenc = GST_JPEG2000ENC(gst_pad_get_parent(pad));
  GstStructure *structure;
  GstCaps *srccaps;
  GstPadLinkReturn ret;
  
  GST_DEBUG("caps %" GST_PTR_FORMAT, caps);

  structure = gst_caps_get_structure(caps, 0);

  ret = gst_structure_get_double(structure, "framerate", &jpegenc->fps);
  ret &= gst_structure_get_int(structure, "width", &jpegenc->width);
  ret &= gst_structure_get_int(structure, "height", &jpegenc->height);

  ret &=  gst_structure_get_int(structure, "bpp", &jpegenc->bpp);
  ret &= gst_structure_get_int(structure, "depth", &jpegenc->depth);
  ret &= gst_structure_get_int(structure, "endianness", &jpegenc->endianness);
  ret &= gst_structure_get_int(structure, "red_mask", &jpegenc->red_mask);
  ret &= gst_structure_get_int(structure, "green_mask", &jpegenc->green_mask);
  ret &= gst_structure_get_int(structure, "blue_mask", &jpegenc->blue_mask);

  GST_DEBUG("Framerate %d %d %d %d", jpegenc->fps, jpegenc->width, jpegenc->height, jpegenc->width);

  if (!ret)
  {
    gst_object_unref(jpegenc);
    return FALSE;
  }
  srccaps = gst_caps_new_simple("image/jp2",
      /* next three are for all video types, previous ones just for x-raw-rgb */
      "width", G_TYPE_INT, jpegenc->width,
      "height", G_TYPE_INT, jpegenc->height,
      "framerate", G_TYPE_DOUBLE, jpegenc->fps, NULL);

  /* at this point, we're pretty sure that this will be the output
   * format, so we'll set it. */
  gst_pad_set_caps(jpegenc->srcpad, srccaps);
  gst_object_unref(jpegenc);
  
  return TRUE;
}

static double dwt_norms_97[4][10]={
    {1.000, 1.965, 4.177, 8.403, 16.90, 33.84, 67.69, 135.3, 270.6, 540.9},
    {2.022, 3.989, 8.355, 17.04, 34.27, 68.63, 137.3, 274.6, 549.0},
    {2.022, 3.989, 8.355, 17.04, 34.27, 68.63, 137.3, 274.6, 549.0},
    {2.080, 3.865, 8.307, 17.18, 34.71, 69.59, 139.3, 278.6, 557.2}
};

static int floorlog2(int a) {
    int l;
    for (l=0; a>1; l++) {
        a>>=1;
    }
    return l;
}

static void encode_stepsize(int stepsize, int numbps, int *expn, int *mant) {
    int p, n;
    p=floorlog2(stepsize)-13;
    n=11-floorlog2(stepsize);
    *mant=(n<0?stepsize>>-n:stepsize<<n)&0x7ff;
    *expn=numbps-p;
}

static void calc_explicit_stepsizes(j2k_tccp_t *tccp, int prec) {
    int numbands, bandno;
    numbands=3*tccp->numresolutions-2;
    for (bandno=0; bandno<numbands; bandno++) {
        double stepsize;

        int resno, level, orient, gain;
        resno=bandno==0?0:(bandno-1)/3+1;
        orient=bandno==0?0:(bandno-1)%3+1;
        level=tccp->numresolutions-1-resno;
        gain=tccp->qmfbid==0?0:(orient==0?0:(orient==1||orient==2?1:2));
        if (tccp->qntsty==J2K_CCP_QNTSTY_NOQNT) {
            stepsize=1.0;
        } else {
            double norm=dwt_norms_97[orient][level];
            stepsize=(1<<(gain+1))/norm;
        }
        encode_stepsize((int)floor(stepsize*8192.0), prec+gain, &tccp->stepsizes[bandno].expn, &tccp->stepsizes[bandno].mant);
    }
}

static GstFlowReturn gst_jpeg2000enc_chain(GstPad *pad, GstBuffer *buf)
{
  GstJpeg2000Enc *jpegenc;
  guchar *data, *outdata;
  gulong size, outsize;
  GstBuffer *outbuf;
  gint width, height, i, j;
  j2k_image_t img;
  j2k_cp_t cp;
  j2k_tcp_t *tcp;
  j2k_tccp_t *tccp;
  gulong ret;
  gboolean ir = FALSE;

  g_return_val_if_fail(pad != NULL, GST_FLOW_ERROR);
  g_return_val_if_fail(GST_IS_PAD(pad), GST_FLOW_ERROR);
  g_return_val_if_fail(buf != NULL, GST_FLOW_ERROR);

  jpegenc = GST_JPEG2000ENC(GST_OBJECT_PARENT(pad));

  if (!GST_PAD_IS_LINKED(jpegenc->srcpad)) {
    gst_buffer_unref(buf);
    return GST_FLOW_ERROR;
  }

  width = jpegenc->width;
  height = jpegenc->height;

  data = (guchar *) GST_BUFFER_DATA(buf);
  size = GST_BUFFER_SIZE(buf);
  GST_DEBUG("gst_jpeg2000enc_chain: got buffer of %ld bytes in '%s'", size,
            GST_OBJECT_NAME(jpegenc));

  GST_DEBUG("gst_jpeg2000enc_chain: starting compression");

  memset(&cp, 0, sizeof(j2k_cp_t));
  memset(&img,0, sizeof(j2k_image_t));

  cp.tx0 = 0; cp.ty0 = 0;
  cp.tw = 1; cp.th = 1;
  cp.tcps = (j2k_tcp_t*)g_malloc(sizeof(j2k_tcp_t));
  tcp = &cp.tcps[0];

  tcp->numlayers = 1;

  tcp->rates[0] = 10000; /* make this adjustable? */


  img.x0 = 0; img.y0 = 0; img.x1 = width; img.y1 = height;
  img.numcomps = 3;
  img.comps = (j2k_comp_t*)g_malloc(img.numcomps*sizeof(j2k_comp_t));

  for (i = 0; i < img.numcomps; i++) {
    img.comps[i].data = (int*)g_malloc(width*height*sizeof(int));
    img.comps[i].prec = 8;
    img.comps[i].sgnd = 0;
    img.comps[i].dx = 1;
    img.comps[i].dy = 1;
  }

  i = 0; j = 0;
  while ((i < width * height) && (j < size)) {
    img.comps[0].data[i] = data[j++];
    img.comps[1].data[i] = data[j++];
    img.comps[2].data[i] = data[j++];
    i++; j++;
  }

  cp.tdx = img.x1 - img.x0;
  cp.tdy = img.y1 - img.y0;

  tcp->csty=0;
  tcp->prg=0;
  tcp->mct = img.numcomps==3?1:0;
  tcp->tccps = (j2k_tccp_t*)g_malloc(img.numcomps*sizeof(j2k_tccp_t));

  for (i = 0; i < img.numcomps; i++) {
    tccp = &tcp->tccps[i];
    tccp->csty = 0;
    /* tccp->numresolutions = 6; */
    tccp->numresolutions = 1;
    tccp->cblkw = 6;
    tccp->cblkh = 6;
    tccp->cblksty = 0;

    tccp->qmfbid = ir?0:1;
    tccp->qntsty = ir?J2K_CCP_QNTSTY_SEQNT:J2K_CCP_QNTSTY_NOQNT;
    tccp->numgbits = 2;
    tccp->roishift = 0;
    calc_explicit_stepsizes(tccp, img.comps[i].prec);
  }

  outbuf = gst_buffer_new();
  outsize = GST_BUFFER_SIZE(outbuf) = tcp->rates[tcp->numlayers - 1] + 2;
  outdata = GST_BUFFER_DATA(outbuf) = g_malloc(outsize);
  GST_BUFFER_TIMESTAMP(outbuf) = GST_BUFFER_TIMESTAMP(buf);

  ret = j2k_encode(&img, &cp, outdata, outsize);
  if (ret == 0) {
        GST_DEBUG("failed to encode image\n");
        return GST_FLOW_ERROR;
  }

  GST_BUFFER_SIZE(outbuf) = ret;

  g_free(tcp->tccps);
  for (i = 0; i < img.numcomps; i++)
    g_free(img.comps[i].data);
  g_free(img.comps);
  g_free(cp.tcps);


  GST_DEBUG("gst_jpegenc_chain: sending buffer");
  gst_pad_push(jpegenc->srcpad, outbuf);

  gst_buffer_unref(buf);
  return GST_FLOW_OK;
}

gboolean
gst_jpeg2000enc_plugin_init (GstPlugin * plugin)
{
  return gst_element_register (plugin, "jpeg2000enc",
      GST_RANK_NONE, GST_TYPE_JPEG2000ENC);
}
