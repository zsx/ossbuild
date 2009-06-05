/*
 * Copyright <2005, 2006, 2007, 2008> Fluendo S.A.
 */
 /*********************************************************************
 * Adapted from dist10 reference code and used under the license therein:
 * ISO MPEG Audio Subgroup Software Simulation Group (1996)
 * ISO 13818-3 MPEG-2 Audio Decoder - Lower Sampling Frequency Extension
 **********************************************************************/

#ifndef __DECODER_H__
#define __DECODER_H__

/***********************************************************************
*
*  Decoder Include Files
*
***********************************************************************/
#include "mp3tl-priv.h"

/***********************************************************************
*
*  Decoder Definitions
*
***********************************************************************/

#define   MUTE              0

/***********************************************************************
*
*  Decoder Function Prototype Declarations
*
***********************************************************************/

/* The following functions are in the file "decode.c" */

gboolean   read_header (mp3tl *tl, fr_header *hdr);
gboolean   set_hdr_data_slots (fr_header *hdr);

void I_decode_bitalloc (Bit_stream_struc *bs, guint bit_alloc[2][SBLIMIT],
    frame_params *fr_ps);
void I_decode_scale (Bit_stream_struc*, guint scfsi[2][SBLIMIT],
    guint scale_index[2][3][SBLIMIT], frame_params *fr_ps);

void II_decode_bitalloc (Bit_stream_struc *bs, guint bit_alloc[2][SBLIMIT],
    frame_params *fr_ps);
void II_decode_scale (Bit_stream_struc *bs, guint scfsi[2][SBLIMIT],
    guint bit_alloc[2][SBLIMIT], guint scale_index[2][3][SBLIMIT],
    frame_params *fr_ps);

void I_buffer_sample(Bit_stream_struc *bs, guint sample[2][3][SBLIMIT],
    guint bit_alloc[2][SBLIMIT], frame_params *fr_ps);
void II_buffer_sample(Bit_stream_struc *bs, guint sample[2][3][SBLIMIT],
    guint bit_alloc[2][SBLIMIT], frame_params * fr_ps);

void I_dequant_and_scale_sample (guint sample[2][3][SBLIMIT],
    float fraction[2][3][SBLIMIT], guint bit_alloc[2][SBLIMIT],
    guint scale_index[2][3][SBLIMIT], frame_params *fr_ps);

void II_dequant_and_scale_sample (guint sample[2][3][SBLIMIT], 
    guint bit_alloc[2][SBLIMIT], float fraction[2][3][SBLIMIT],
    guint scale_index[2][3][SBLIMIT], int scale_block,
    frame_params *fr_ps);

void   init_syn_filter (frame_params *fr_ps);

void out_fifo (short pcm_sample[2][SSLIMIT][SBLIMIT], int num, 
  frame_params *fr_ps, gint16 *outBuf, guint32 *psamples, guint32 bufSize);

void buffer_CRC (Bit_stream_struc*, unsigned int*);
void recover_CRC_error (short pcm_sample[2][SSLIMIT][SBLIMIT], int error_count, 
    frame_params *fr_ps, gint16 *outBuf, guint32 *psamples, guint32 bufSize);
#endif
