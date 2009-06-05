/*
 * Copyright <2005, 2006, 2007, 2008> Fluendo S.A.
 */
#ifndef __MP3TL_H__
#define __MP3TL_H__

#include "common.h"
#include "bitstream.h"

typedef struct mp3tl mp3tl;
typedef enum {
  MP3TL_ERR_OK = 0,     /* Successful return code */
  MP3TL_ERR_NO_SYNC,    /* There was no sync word in the data buffer */
  MP3TL_ERR_NEED_DATA,  /* Not enough data in the buffer for the requested op */
  MP3TL_ERR_BAD_FRAME,  /* The frame data was corrupt and skipped */
  MP3TL_ERR_STREAM,     /* Encountered invalid data in the stream */
  MP3TL_ERR_UNSUPPORTED_STREAM, /* Encountered valid but unplayable data in 
                                 * the stream */
  MP3TL_ERR_PARAM,      /* Invalid parameter was passed in */
  MP3TL_ERR_UNKNOWN     /* Unspecified internal decoder error (bug) */
} Mp3TlRetcode;

typedef enum {
  MP3TL_MODE_16BIT = 0  /* Decoder mode to use */
} Mp3TlMode;

mp3tl *mp3tl_new (Bit_stream_struc *bs, Mp3TlMode mode);

void mp3tl_free (mp3tl *tl);

void mp3tl_set_eos (mp3tl *tl, gboolean more_data);
Mp3TlRetcode mp3tl_sync (mp3tl *tl);
Mp3TlRetcode mp3tl_decode_header (mp3tl *tl, const fr_header **ret_hdr);
Mp3TlRetcode mp3tl_skip_frame (mp3tl *tl, GstClockTime *buf_time);
Mp3TlRetcode mp3tl_decode_frame (mp3tl *tl, guint8 *samples, guint bufsize,
  GstClockTime *buf_time);
const char *mp3tl_get_err_reason (mp3tl *tl);
void mp3tl_flush (mp3tl *tl);
#endif
