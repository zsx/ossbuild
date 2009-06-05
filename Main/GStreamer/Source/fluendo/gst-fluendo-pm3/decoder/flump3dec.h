/*
 * Copyright <2005, 2006, 2007, 2008> Fluendo S.A.
 */
 
#ifndef __FLUMP3DEC_H__
#define __FLUMP3DEC_H__

#include <gst/gst.h>
#include "mp3tl.h"

G_BEGIN_DECLS

#define FLUMP3DEC_TYPE \
  (flump3dec_get_type())
#define FLUMP3DEC(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),FLUMP3DEC_TYPE,FluMp3Dec))
#define FLUMP3DEC_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),FLUMP3DEC_TYPE,GstPluginTemplate))
#define IS_FLUMP3DEC(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),FLUMP3DEC_TYPE))
#define IS_FLUMP3DEC_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),FLUMP3DEC_TYPE))

typedef struct FluMp3Dec FluMp3Dec;
typedef struct FluMp3DecClass FluMp3DecClass;

struct FluMp3Dec
{
  GstElement element;

  GstPad *sinkpad, *srcpad;

  Bit_stream_struc *bs;
  mp3tl *dec;

  guint rate;
  guint channels;
  guint bytes_per_sample;
  gboolean need_discont;

  GstClockTime next_ts;
  GstClockTime last_dec_ts;

  /* VBR tracking */
  guint   avg_bitrate;
  guint64 bitrate_sum;
  guint   frame_count;

  guint   last_posted_bitrate;

  gboolean bad;
  GstBuffer *pending_frame;

  /* Xing header info */
  guint32 xing_flags;
  guint32 xing_frames;
  GstClockTime xing_total_time;
  guint32 xing_bytes;
  guchar xing_seek_table[100];
  guint32 xing_vbr_scale;
};

struct FluMp3DecClass
{
  GstElementClass parent;
};

GType flump3dec_get_type (void);

G_END_DECLS

#endif
