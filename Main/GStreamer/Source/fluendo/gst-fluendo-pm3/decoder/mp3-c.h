/*
 * Copyright <2005, 2006, 2007, 2008> Fluendo S.A.
 */
 /*********************************************************************
 * Adapted from dist10 reference code and used under the license therein:
 * ISO MPEG Audio Subgroup Software Simulation Group (1996)
 * ISO 13818-3 MPEG-2 Audio Decoder - Lower Sampling Frequency Extension
 **********************************************************************/
#ifndef __MP3TL_C_H__
#define __MP3TL_C_H__

typedef struct mp3cimpl_info mp3cimpl_info;

struct mp3cimpl_info {
  huffdec_bitbuf bb;          /* huffman decoder bit buffer */
  guint8 hb_buf[HDBB_BUFSIZE]; /* Huffman decoder work buffer */
  guint main_data_end; /* Number of bytes in the bit reservoir at the 
                        * end of the last frame */

  /* Hybrid */
  gdouble prevblck[2][SBLIMIT][SSLIMIT];

  /* scale data */
  guint scalefac_buffer[54];
};

void III_subband_synthesis (mp3tl *tl, frame_params *fr_ps, 
   gfloat hybridOut[SBLIMIT][SSLIMIT], gint channel, 
   short samples[SSLIMIT][SBLIMIT]);

gboolean mp3_c_init (mp3tl *tl);
void mp3_c_flush (mp3tl *tl);

Mp3TlRetcode c_decode_mp3 (mp3tl *tl);

#endif
