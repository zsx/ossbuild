/* Copyright 2006, 2007, 2008 Fluendo S.A. 
 * Authors: Jan Schmidt <jan@fluendo.com>
 *          Kapil Agrawal <kapil@fluendo.com>
 *          Julien Moutte <julien@fluendo.com>
 * See the COPYING file in the top-level directory.
 */
 
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "flumpegtsmux_h264.h"
#include <string.h>

GST_DEBUG_CATEGORY_EXTERN (flutsmux_debug);
#define GST_CAT_DEFAULT flutsmux_debug
 
GstBuffer * flutsmux_prepare_h264 (GstBuffer * buf, FluTsPadData * data,
    FluTsMux * mux)
{
  guint8 nal_length_size = 0;
  guint8 startcode [4] = { 0x00, 0x00, 0x00, 0x01 };
  GstBuffer * out_buf = gst_buffer_new_and_alloc (GST_BUFFER_SIZE (buf) * 2);
  gint offset = 4, i = 0, nb_sps = 0, nb_pps = 0;
  gsize out_offset = 0, in_offset = 0;
  
  GST_DEBUG_OBJECT (mux, "Preparing H264 buffer for output");
  
  /* We want the same metadata */
  gst_buffer_copy_metadata (out_buf, buf, GST_BUFFER_COPY_ALL);
  
  /* Get NAL length size */
  nal_length_size = 
      (GST_READ_UINT8 (GST_BUFFER_DATA (data->codec_data) + offset) & 0x03) + 1;
  GST_LOG_OBJECT (mux, "NAL length will be coded on %u bytes", nal_length_size);
  offset++;
  
  /* Generate SPS */
  nb_sps =  GST_READ_UINT8 (GST_BUFFER_DATA (data->codec_data) + offset) & 0x1f;
  GST_DEBUG_OBJECT (mux, "we have %d Sequence Parameter Set", nb_sps);
  offset++;
  
  /* For each SPS */
  for (i = 0; i < nb_sps; i++) {
    guint16 sps_size =
        GST_READ_UINT16_BE (GST_BUFFER_DATA (data->codec_data) + offset);
      
    GST_LOG_OBJECT (mux, "Sequence Parameter Set is %d bytes", sps_size);
      
    /* Jump over SPS size */
    offset += 2;
      
    /* Fake a start code */
    memcpy (GST_BUFFER_DATA (out_buf) + out_offset, startcode, 4);
    out_offset += 4;
    /* Now push the SPS */
    memcpy (GST_BUFFER_DATA (out_buf) + out_offset,
        GST_BUFFER_DATA (data->codec_data) + offset, sps_size);

    out_offset += sps_size;
    offset += sps_size;
  }
  
  nb_pps = GST_READ_UINT8 (GST_BUFFER_DATA (data->codec_data) + offset);
  GST_LOG_OBJECT (mux, "we have %d Picture Parameter Set", nb_sps);
  offset++;
  
  /* For each PPS */
  for (i = 0; i < nb_pps; i++) {
    gint pps_size =
        GST_READ_UINT16_BE (GST_BUFFER_DATA (data->codec_data) + offset);
    
    GST_LOG_OBJECT (mux, "Picture Parameter Set is %d bytes", pps_size);
    
    /* Jump over PPS size */
    offset += 2;
    
    /* Fake a start code */
    memcpy (GST_BUFFER_DATA (out_buf) + out_offset, startcode, 4);
    out_offset += 4;
    /* Now push the PPS */
    memcpy (GST_BUFFER_DATA (out_buf) + out_offset,
        GST_BUFFER_DATA (data->codec_data) + offset, pps_size);
    
    out_offset += pps_size;
    offset += pps_size;
  }
  
  while (in_offset < GST_BUFFER_SIZE (buf) &&
      out_offset < GST_BUFFER_SIZE (out_buf) - 4) {
    guint32 nal_size = 0;
    
    switch (nal_length_size) {
      case 1:
        nal_size = GST_READ_UINT8 (GST_BUFFER_DATA (buf) + in_offset);
        break;
      case 2:
        nal_size = GST_READ_UINT16_BE (GST_BUFFER_DATA (buf) + in_offset);
        break;
      case 4:
        nal_size = GST_READ_UINT32_BE (GST_BUFFER_DATA (buf) + in_offset);
        break;
      default:
        GST_WARNING_OBJECT (mux, "unsupported NAL length size %u",
            nal_length_size);
    }
    in_offset += nal_length_size;
    
    /* Generate an Elementary stream buffer by inserting a startcode */
    memcpy (GST_BUFFER_DATA (out_buf) + out_offset, startcode, 4);
    out_offset += 4;
    memcpy (GST_BUFFER_DATA (out_buf) + out_offset,
        GST_BUFFER_DATA (buf) + in_offset,
        MIN (nal_size, GST_BUFFER_SIZE (out_buf) - out_offset));
    in_offset += nal_size;
    out_offset += nal_size;
  }
  
  GST_BUFFER_SIZE (out_buf) = out_offset;
    
  return out_buf;
}
