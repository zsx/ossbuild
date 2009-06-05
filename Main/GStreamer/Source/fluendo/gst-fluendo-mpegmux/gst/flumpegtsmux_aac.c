/* Copyright 2006, 2007, 2008 Fluendo S.A. 
 * Authors: Jan Schmidt <jan@fluendo.com>
 *          Kapil Agrawal <kapil@fluendo.com>
 *          Julien Moutte <julien@fluendo.com>
 * See the COPYING file in the top-level directory.
 */
 
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "flumpegtsmux_aac.h"
#include <string.h>

GST_DEBUG_CATEGORY_EXTERN (flutsmux_debug);
#define GST_CAT_DEFAULT flutsmux_debug
 
GstBuffer * flutsmux_prepare_aac (GstBuffer * buf, FluTsPadData * data,
    FluTsMux * mux)
{
  guint8 * adts_header = g_malloc0 (7);
  GstBuffer * out_buf = gst_buffer_new_and_alloc (GST_BUFFER_SIZE (buf) + 7);
  gsize out_offset = 0;
  guint8 rate_idx = 0, channels = 0, obj_type = 0;
  
  GST_DEBUG_OBJECT (mux, "Preparing AAC buffer for output");
  
  /* We want the same metadata */
  gst_buffer_copy_metadata (out_buf, buf, GST_BUFFER_COPY_ALL);
  
  /* Generate ADTS header */
  obj_type = (GST_READ_UINT8 (GST_BUFFER_DATA (data->codec_data)) & 0xC) >> 2;
  obj_type++;
  rate_idx = (GST_READ_UINT8 (GST_BUFFER_DATA (data->codec_data)) & 0x3) << 1;
  rate_idx |=
      (GST_READ_UINT8 (GST_BUFFER_DATA (data->codec_data) + 1) & 0x80) >> 7;
  channels = 
      (GST_READ_UINT8 (GST_BUFFER_DATA (data->codec_data) + 1) & 0x78) >> 3;
  GST_DEBUG_OBJECT (mux, "Rate index %u, channels %u, object type %u", rate_idx,
      channels, obj_type);
  /* Sync point over a full byte */
  adts_header[0] = 0xFF;
  /* Sync point continued over first 4 bits + static 4 bits
   * (ID, layer, protection)*/
  adts_header[1] = 0xF1;
  /* Object type over first 2 bits */
  adts_header[2] = obj_type << 6;
  /* rate index over next 4 bits */
  adts_header[2] |= (rate_idx << 2);
  /* channels over last 2 bits */
  adts_header[2] |= (channels & 0x4) >> 2;
  /* channels continued over next 2 bits + 4 bits at zero */
  adts_header[3] = (channels & 0x3) << 6;
  /* frame size over last 2 bits */
  adts_header[3] |= (GST_BUFFER_SIZE (out_buf) & 0x1800) >> 11;
  /* frame size continued over full byte */
  adts_header[4] = (GST_BUFFER_SIZE (out_buf) & 0x1FF8) >> 3;
  /* frame size continued first 3 bits */ 
  adts_header[5] = (GST_BUFFER_SIZE (out_buf) & 0x7) << 5;
  /* buffer fullness (0x7FF for VBR) over 5 last bits*/
  adts_header[5] |= 0x1F;
  /* buffer fullness (0x7FF for VBR) continued over 6 first bits + 2 zeros for
   * number of raw data blocks */
  adts_header[6] = 0xFC;
  
  /* Insert ADTS header */
  memcpy (GST_BUFFER_DATA (out_buf) + out_offset, adts_header, 7);
  g_free (adts_header);
  out_offset += 7;
  
  /* Now copy complete frame */
  memcpy (GST_BUFFER_DATA (out_buf) + out_offset, GST_BUFFER_DATA (buf),
      GST_BUFFER_SIZE (buf));
  
  return out_buf;
}
