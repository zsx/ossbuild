/*
 * Copyright <2005, 2006, 2007, 2008> Fluendo S.A.
 */
 /*********************************************************************
 * Adapted from dist10 reference code and used under the license therein:
 * ISO MPEG Audio Subgroup Software Simulation Group (1996)
 * ISO 13818-3 MPEG-2 Audio Decoder - Lower Sampling Frequency Extension
 **********************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/***********************************************************************
*
*  Global Include Files
*
***********************************************************************/

#include "common.h"

#include <string.h>      /* 1995-07-11 shn */
#include <ctype.h>
#include <stdlib.h>
#include <math.h>

#include "table-bitalloc.h"

/***********************************************************************
*
*  Global Variable Definitions
*
***********************************************************************/

/* Sample rates table, index by MPEG version, samplerate index */
const gint s_rates[4][4] = 
{ 
  { 11025, 12000, 8000, 0},  /* MPEG_VERSION_2_5 */
  { 0, 0, 0, 0 },            /* Invalid MPEG version */
  { 22050, 24000, 16000, 0}, /* MPEG_VERSION_2 */
  { 44100, 48000, 32000, 0}  /* MPEG_VERSION_1 */
};

/* MPEG version 1 bitrates. indexed by layer, bitrate index */
const gint bitrates_v1[3][15] = 
{
  {0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448},
  {0, 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384},
  {0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320}
};

/* MPEG version 2 (LSF) and 2.5 bitrates. indexed by layer, bitrate index */
const gint bitrates_v2[3][15] = 
{
  {0, 32, 48, 56, 64, 80, 96, 112, 128, 144, 160, 176, 192, 224, 256},
  {0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160},
  {0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160}
};

/***********************************************************************
*
*  Global Function Definitions
*
***********************************************************************/

/***********************************************************************
 * Use the header information to select the subband allocation table
 **********************************************************************/
void
II_pick_table (frame_params *fr_ps)
{
  int table, ver, lay, bsp, br_per_ch, sfrq;

  ver = fr_ps->header.version;
  lay = fr_ps->header.layer - 1;
  bsp = fr_ps->header.bitrate_idx;

  /* decision rules refer to per-channel bitrates (kbits/sec/chan) */
  if (ver == MPEG_VERSION_1) {
    br_per_ch = bitrates_v1[lay][bsp] / fr_ps->stereo;

    sfrq = s_rates[ver][fr_ps->header.srate_idx];

    /* MPEG-1 */
    if ((sfrq == 48000 && br_per_ch >= 56) || 
        (br_per_ch >= 56 && br_per_ch <= 80))
      table = 0;
    else if (sfrq != 48000 && br_per_ch >= 96)
      table = 1;
    else if (sfrq != 32000 && br_per_ch <= 48)
      table = 2;
    else
      table = 3;

  } else {
    br_per_ch = bitrates_v2[lay][bsp] / fr_ps->stereo;

    /* MPEG-2 LSF */
    table = 4;
  }

  fr_ps->sblimit = ba_tables[table].sub_bands;
  fr_ps->alloc = &ba_tables[table].alloc;
}

static int
js_bound (gint lay, gint m_ext)
{
  /* layer + mode_ext -> jsbound */
  static const int jsb_table[3][4] = { 
      {4, 8, 12, 16}, {4, 8, 12, 16}, {0, 4, 8, 16}
  };  

  if (lay < 1 || lay > 3 || m_ext < 0 || m_ext > 3) {
    g_warning ("js_bound bad layer/modext (%d/%d)\n", lay, m_ext);
    return 0;
  }
  return (jsb_table[lay - 1][m_ext]);
}

void
hdr_to_frps (frame_params *fr_ps)
{
  fr_header *hdr = &fr_ps->header;

  fr_ps->actual_mode = hdr->mode;
  fr_ps->stereo = (hdr->mode == MPG_MD_MONO) ? 1 : 2;
  fr_ps->sblimit = SBLIMIT;

  if (hdr->mode == MPG_MD_JOINT_STEREO)
    fr_ps->jsbound = js_bound (hdr->layer, hdr->mode_ext);
  else
    fr_ps->jsbound = fr_ps->sblimit;
}

/*****************************************************************************
*
*  CRC error protection package
*
*****************************************************************************/
void
I_CRC_calc (const frame_params *fr_ps, guint bit_alloc[2][SBLIMIT],
    guint *crc)
{
  gint i, k;
  const fr_header *hdr = &fr_ps->header;
  const gint stereo = fr_ps->stereo;
  const gint jsbound = fr_ps->jsbound;

  *crc = 0xffff;                /* changed from '0' 92-08-11 shn */
  update_CRC (hdr->bitrate_idx, 4, crc);
  update_CRC (hdr->srate_idx, 2, crc);
  update_CRC (hdr->padding, 1, crc);
  update_CRC (hdr->extension, 1, crc);
  update_CRC (hdr->mode, 2, crc);
  update_CRC (hdr->mode_ext, 2, crc);
  update_CRC (hdr->copyright, 1, crc);
  update_CRC (hdr->original, 1, crc);
  update_CRC (hdr->emphasis, 2, crc);

  for (i = 0; i < SBLIMIT; i++)
    for (k = 0; k < ((i < jsbound) ? stereo : 1); k++)
      update_CRC (bit_alloc[k][i], 4, crc);
}

void
II_CRC_calc (const frame_params *fr_ps, guint bit_alloc[2][SBLIMIT],
    guint scfsi[2][SBLIMIT], guint *crc)
{
  gint i, k;
  const fr_header *hdr = &fr_ps->header;
  const gint stereo = fr_ps->stereo;
  const gint sblimit = fr_ps->sblimit;
  const gint jsbound = fr_ps->jsbound;
  const al_table *alloc = fr_ps->alloc;

  *crc = 0xffff; /* changed from '0' 92-08-11 shn */

  update_CRC (hdr->bitrate_idx, 4, crc);
  update_CRC (hdr->srate_idx, 2, crc);
  update_CRC (hdr->padding, 1, crc);
  update_CRC (hdr->extension, 1, crc);
  update_CRC (hdr->mode, 2, crc);
  update_CRC (hdr->mode_ext, 2, crc);
  update_CRC (hdr->copyright, 1, crc);
  update_CRC (hdr->original, 1, crc);
  update_CRC (hdr->emphasis, 2, crc);

  for (i = 0; i < sblimit; i++)
    for (k = 0; k < ((i < jsbound) ? stereo : 1); k++)
      update_CRC (bit_alloc[k][i], (*alloc)[i][0].bits, crc);

  for (i = 0; i < sblimit; i++)
    for (k = 0; k < stereo; k++)
      if (bit_alloc[k][i])
        update_CRC (scfsi[k][i], 2, crc);
}

void
update_CRC (const guint data, const guint length, guint *crc)
{
  unsigned int masking, carry;

  masking = 1 << length;

  while ((masking >>= 1)) {
    carry = *crc & 0x8000;
    *crc <<= 1;
    if (!carry ^ !(data & masking))
      *crc ^= CRC16_POLYNOMIAL;
  }
  *crc &= 0xffff;
}

/*****************************************************************************
*
*  End of CRC error protection package
*
*****************************************************************************/

