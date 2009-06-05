/* Copyright 2006, 2007, 2008 Fluendo S.A. 
 * Authors: Jan Schmidt <jan@fluendo.com>
 *          Kapil Agrawal <kapil@fluendo.com>
 *          Julien Moutte <julien@fluendo.com>
 * See the COPYING file in the top-level directory.
 */
 
#ifndef __FLUTSMUX_H__
#define __FLUTSMUX_H__

#include <gst/gst.h>
#include <gst/base/gstcollectpads.h>
#include <gst/base/gstadapter.h>

G_BEGIN_DECLS

#include <tsmux/tsmux.h>

#define FLU_TYPE_TSMUX  (flutsmux_get_type())
#define FLU_TSMUX(obj)  (G_TYPE_CHECK_INSTANCE_CAST((obj), FLU_TYPE_TSMUX, FluTsMux))

typedef struct FluTsMux FluTsMux;
typedef struct FluTsMuxClass FluTsMuxClass;
typedef struct FluTsPadData FluTsPadData;

typedef GstBuffer * (*FluTsPadDataPrepareFunction) (GstBuffer * buf,
    FluTsPadData * data, FluTsMux * mux);

struct FluTsMux {
  GstElement parent;

  GstPad *srcpad;

  GstCollectPads *collect;

  TsMux *tsmux;
  TsMuxProgram *program;

  gboolean first;
  TsMuxStream *pcr_stream;
  GstFlowReturn last_flow_ret;
  GstAdapter *adapter;
  gint64 previous_pcr;
  gboolean m2ts_mode;
  gboolean first_pcr;
  
  GstClockTime last_ts;
};

struct FluTsMuxClass  {
  GstElementClass parent_class;
};

struct FluTsPadData {
  GstCollectData collect; /* Parent */

  gint pid;
  TsMuxStream *stream;

  GstBuffer *queued_buf; /* Currently pulled buffer */
  GstClockTime cur_ts; /* Adjusted TS for the pulled buffer */
  GstClockTime last_ts; /* Most recent valid TS for this stream */

  GstBuffer * codec_data; /* Optional codec data available in the caps */

  FluTsPadDataPrepareFunction prepare_func; /* Handler to prepare input data */

  gboolean eos;
};

GType flutsmux_get_type (void);

#define CLOCK_BASE 9LL
#define CLOCK_FREQ (CLOCK_BASE * 10000)

#define MPEGTIME_TO_GSTTIME(time) (gst_util_uint64_scale ((time), \
                        GST_MSECOND/10, CLOCK_BASE))
#define GSTTIME_TO_MPEGTIME(time) (gst_util_uint64_scale ((time), \
                        CLOCK_BASE, GST_MSECOND/10))

#define NORMAL_TS_PACKET_LENGTH 188
#define M2TS_PACKET_LENGTH      192
#define STANDARD_TIME_CLOCK     27000000
/*33 bits as 1 ie 0x1ffffffff*/
#define TWO_POW_33_MINUS1     ((0xffffffff * 2) - 1) 
G_END_DECLS

#endif
