/*
 * Copyright <2005, 2006, 2007, 2008> Fluendo S.A.
 */
#ifndef __MP3TL_PRIV_H__
#define __MP3TL_PRIV_H__

#include <gst/gst.h>
#include "mp3tl.h"

#ifdef USE_IPP
#include "mp3-ipp.h"
#endif
#include "mp3-c.h"

typedef short PCM[2][SSLIMIT][SBLIMIT];
typedef unsigned int SAM[2][3][SBLIMIT];
typedef float FRA[2][3][SBLIMIT];

/* Size of the temp buffer for output samples, max 2 channels * 
 * sub-bands * samples-per-sub-band * 2 buffers
 */
#define SAMPLE_BUF_SIZE (4 * 2 * SBLIMIT * SSLIMIT)

struct mp3tl
{
  gboolean need_sync;
  gboolean need_header;
  gboolean at_eos;
  gboolean lost_sync;

  /* Bit stream to read the data from */
  Bit_stream_struc *bs;
  
  /* Layer number we're decoding, 0 if we don't know yet */
  guint8 stream_layer; 

  guint64 frame_num;

  /* Bits consumed from the stream so far */
  gint64 bits_used;

  /* Number of samples output so far */
  guint32 sample_frames;

  /* Total number of errors encountered */
  guint error_count;

  /* Sample size configure for. Always 16-bits, for now */
  guint sample_size;

  /* Frame decoding info */
  frame_params fr_ps;
  
  /* Time stamp associated with the buffer that contained the
   * sync_word for the current frame */
  GstClockTime frame_ts;

  /* Number of granules in this frame (1 or 2) */
  guint n_granules;
  /* CRC value read from the mpeg data */
  guint old_crc; 
  
  PCM pcm_sample;
  SAM sample;
  FRA fraction;

  /* Output samples circular buffer and read and write ptrs */
  gint16 *sample_buf;
  guint32 sample_w;
  
  char *reason; /* Reason why an error was returned, if known */
#ifdef USE_IPP
  mp3ipp_info ipp;
#endif
  mp3cimpl_info c_impl;

  /*Free format bitrate*/
  guint32 free_bitrate;

  /*Used for one time calculation of free bitrate*/
  gboolean free_first;
};

void mp3_SubBandSynthesis (mp3tl *tl ATTR_UNUSED,
    frame_params *fr_ps, float *bandPtr, gint channel, short *samples);
#endif
