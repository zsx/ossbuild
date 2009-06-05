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

#include <gst/gst.h>
#ifdef USE_LIBOIL
#include <liboil/liboil.h>
#endif

#include "mp3tl-priv.h"
#include "decode.h"
#include "mp3-cos-tables.h"
#include "table-powtable.h"
#include "table-powtable-2.h"

#include "table-huffdec.h" 

GST_DEBUG_CATEGORY_EXTERN (flump3debug);
#define GST_CAT_DEFAULT flump3debug

#define HUFFBITS guint32 
#define HTSIZE  34
#define MXOFF   250

static const HUFFBITS dmask = (guint32)(1) << (sizeof (HUFFBITS) * 8 - 1);
static const guint hs = sizeof (HUFFBITS) * 8;

/************************* Layer III routines **********************/

gboolean
III_get_side_info (guint8 *data, III_side_info_t *si,
    frame_params *fr_ps)
{
  int ch, gr, i;
  int stereo = fr_ps->stereo;
  huffdec_bitbuf bb;

  h_setbuf (&bb, data, fr_ps->header.side_info_slots);

  if (fr_ps->header.version == MPEG_VERSION_1) {
    si->main_data_begin = h_getbits (&bb, 9);
    if (stereo == 1)
      si->private_bits = h_getbits (&bb, 5);
    else
      si->private_bits = h_getbits (&bb, 3);
      
    for (ch = 0; ch < stereo; ch++) {
      guint8 scfsi = (guint8) h_getbits (&bb, 4);
      si->scfsi[0][ch] = scfsi & 0x08;
      si->scfsi[1][ch] = scfsi & 0x04;
      si->scfsi[2][ch] = scfsi & 0x02;
      si->scfsi[3][ch] = scfsi & 0x01;
    }

    for (gr = 0; gr < 2; gr++) {
      for (ch = 0; ch < stereo; ch++) {
        gr_info_t *gi = &(si->gr[gr][ch]);
        
        gi->part2_3_length = h_getbits (&bb, 12);
        gi->big_values = h_getbits (&bb, 9);
        /* Add 116 to avoid doing it in the III_dequantize loop */
        gi->global_gain = h_getbits (&bb, 8) + 116;
        gi->scalefac_compress = h_getbits (&bb, 4);
        gi->window_switching_flag = h_get1bit (&bb);
        if (gi->window_switching_flag) {
          gi->block_type = h_getbits (&bb, 2);
          gi->mixed_block_flag = h_get1bit (&bb);
          gi->table_select[0] = h_getbits (&bb, 5);
          gi->table_select[1] = h_getbits (&bb, 5);
          for (i = 0; i < 3; i++)
            gi->subblock_gain[i] = h_getbits (&bb, 3);

          if (gi->block_type == 0) {
            g_warning ("Side info bad: block_type == 0 in split block.\n");
            return FALSE;
          } else if (gi->block_type == 2
              && gi->mixed_block_flag == 0) {
            gi->region0_count = 8;        /* MI 9; */
            gi->region1_count = 12;
          } else {
            gi->region0_count = 7;        /* MI 8; */
            gi->region1_count = 13;
          }
        } else {
          for (i = 0; i < 3; i++)
            gi->table_select[i] = h_getbits (&bb, 5);
          gi->region0_count = h_getbits (&bb, 4);
          gi->region1_count = h_getbits (&bb, 3);
          gi->block_type = 0;
        }
        gi->preflag = h_get1bit (&bb);
        /* Add 1 & multiply by 2 to avoid doing it in the III_dequantize loop */
        gi->scalefac_scale = 2 * (h_get1bit (&bb) + 1);
        gi->count1table_select = h_get1bit (&bb);
      }
    }
  } else {                      /* Layer 3 LSF */

    si->main_data_begin = h_getbits (&bb, 8);
    if (stereo == 1)
      si->private_bits = h_getbits (&bb, 1);
    else
      si->private_bits = h_getbits (&bb, 2);

    for (gr = 0; gr < 1; gr++) {
      for (ch = 0; ch < stereo; ch++) {
        gr_info_t *gi = &(si->gr[gr][ch]);
        
        gi->part2_3_length = h_getbits (&bb, 12);
        gi->big_values = h_getbits (&bb, 9);
        /* Add 116 to avoid doing it in the III_dequantize loop */
        gi->global_gain = h_getbits (&bb, 8) + 116;
        gi->scalefac_compress = h_getbits (&bb, 9);
        gi->window_switching_flag = h_get1bit (&bb);
        if (gi->window_switching_flag) {
          gi->block_type = h_getbits (&bb, 2);
          gi->mixed_block_flag = h_get1bit (&bb);
          gi->table_select[0] = h_getbits (&bb, 5);
          gi->table_select[1] = h_getbits (&bb, 5);
          for (i = 0; i < 3; i++)
            gi->subblock_gain[i] = h_getbits (&bb, 3);

          /* Set region_count parameters since they are 
           * implicit in this case. */
          if (gi->block_type == 0) {
            GST_WARNING ("Side info bad: block_type == 0 in split block.\n");
            return FALSE;
          } else if (gi->block_type == 2
              && gi->mixed_block_flag == 0) {
            gi->region0_count = 8;        /* MI 9; */
            gi->region1_count = 12;
          } else {
            gi->region0_count = 7;        /* MI 8; */
            gi->region1_count = 13;
          }
        } else {
          for (i = 0; i < 3; i++)
            gi->table_select[i] = h_getbits (&bb, 5);
          gi->region0_count = h_getbits (&bb, 4);
          gi->region1_count = h_getbits (&bb, 3);
          gi->block_type = 0;
        }

        gi->preflag = 0;
        /* Add 1 & multiply by 2 to avoid doing it in the III_dequantize loop */
        gi->scalefac_scale = 2 * (h_get1bit (&bb) + 1);
        gi->count1table_select = h_get1bit (&bb);
      }
    }
  }

  return TRUE;
}

const gint slen[2][16] = 
{ 
  {0, 0, 0, 0, 3, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4},
  {0, 1, 2, 3, 0, 1, 2, 3, 1, 2, 3, 1, 2, 3, 2, 3}
};

struct
{
  gint l[23];
  gint s[14];
} const sfBandIndex[] = 
{ 
  /* MPEG-1 */
  { 
    /* 44.1 khz */
    { 0, 4, 8, 12, 16, 20, 24, 30, 36, 44, 52, 62, 74, 90, 110, 134, 162, 196,
      238, 288, 342, 418, 576 }, 
    { 0, 4, 8, 12, 16, 22, 30, 40, 52, 66, 84, 106, 136, 192 }
  }, { 
    /* 48khz */
    { 0, 4, 8, 12, 16, 20, 24, 30, 36, 42, 50, 60, 72, 88, 106, 128, 156, 190,
      230, 276, 330, 384, 576 }, 
    { 0, 4, 8, 12, 16, 22, 28, 38, 50, 64, 80, 100, 126, 192 }
  }, { 
    /* 32khz */
    { 0, 4, 8, 12, 16, 20, 24, 30, 36, 44, 54, 66, 82, 102, 126, 156, 194, 240,
      296, 364, 448, 550, 576 }, 
    { 0, 4, 8, 12, 16, 22, 30, 42, 58, 78, 104, 138, 180, 192 }
  },
  /* MPEG-2 */
  { 
    /* 22.05 khz */
    { 0, 6, 12, 18, 24, 30, 36, 44, 54, 66, 80, 96, 116, 140, 168, 200, 238,
      284, 336, 396, 464, 522, 576 }, 
    { 0, 4, 8, 12, 18, 24, 32, 42, 56, 74, 100, 132, 174, 192 }
  }, { 
    /* 24khz */
    { 0, 6, 12, 18, 24, 30, 36, 44, 54, 66, 80, 96, 114, 136, 162, 194, 232, 
      278, 330, 394, 464, 540, 576 }, 
    { 0, 4, 8, 12, 18, 26, 36, 48, 62, 80, 104, 136, 180, 192 }
  }, 
  { 
    /* 16 khz */
    { 0, 6, 12, 18, 24, 30, 36, 44, 54, 66, 80, 96, 116, 140, 168, 200, 238, 
      284, 336, 396, 464, 522, 576 }, 
    { 0, 4, 8, 12, 18, 26, 36, 48, 62, 80, 104, 134, 174, 192 }
  }, 
  /* MPEG-2.5 */
  {
    /* 11025 */
    { 0, 6, 12, 18, 24, 30, 36, 44, 54, 66, 80, 96, 116, 140, 168, 200, 238, 
      284, 336, 396, 464, 522, 576 }, 
    { 0, 4, 8, 12, 18, 26, 36, 48, 62, 80, 104, 134, 174, 192 }
  }, 
  { 
    /* 12khz */
    { 0, 6, 12, 18, 24, 30, 36, 44, 54, 66, 80, 96, 116, 140, 168, 200, 238, 
      284, 336, 396, 464, 522, 576 }, 
    { 0, 4, 8, 12, 18, 26, 36, 48, 62, 80, 104, 134, 174, 192 }
  }, 
  { 
    /* 8khz */
    { 0, 12, 24, 36, 48, 60, 72, 88, 108, 132, 160, 192, 232, 280, 336, 400,
      476, 566, 568, 570, 572, 574, 576 },
    { 0, 8, 16, 24, 36, 52, 72, 96, 124, 160, 162, 164, 166, 192 }
  }
};
/* Offset into the sfBand table for each MPEG version */
guint sfb_offset[] = { 6, 0 /* invalid */, 3, 0 };

void III_get_scale_factors (III_scalefac_t *scalefac, III_side_info_t *si,
     int gr, int ch, mp3tl *tl)
{
  int sfb, window;
  gr_info_t *gr_info = &(si->gr[gr][ch]);
  huffdec_bitbuf *bb = &tl->c_impl.bb;
  gint slen0, slen1;

  slen0 = slen[0][gr_info->scalefac_compress];
  slen1 = slen[1][gr_info->scalefac_compress];
  if (gr_info->window_switching_flag && (gr_info->block_type == 2)) {
    if (gr_info->mixed_block_flag) {    /* MIXED *//* NEW - ag 11/25 */
      for (sfb = 0; sfb < 8; sfb++)
        (*scalefac)[ch].l[sfb] = h_getbits (bb, slen0);

      for (sfb = 3; sfb < 6; sfb++)
        for (window = 0; window < 3; window++)
          (*scalefac)[ch].s[window][sfb] = h_getbits (bb, slen0);

      for (/* sfb = 6 */; sfb < 12; sfb++)
        for (window = 0; window < 3; window++)
          (*scalefac)[ch].s[window][sfb] = h_getbits (bb, slen1);

      for (sfb = 12, window = 0; window < 3; window++)
        (*scalefac)[ch].s[window][sfb] = 0;
    } else {
      /* SHORT block */
      for (sfb = 0; sfb < 6; sfb++)
        for (window = 0; window < 3; window++)
          (*scalefac)[ch].s[window][sfb] =
              h_getbits (bb, slen0);
      for (/* sfb = 6 */; sfb < 12; sfb++)
        for (window = 0; window < 3; window++)
          (*scalefac)[ch].s[window][sfb] =
              h_getbits (bb, slen1);
                
      for (window = 0; window < 3; window++)
        (*scalefac)[ch].s[window][12] = 0;
    }
  } else {
    gint i;
    const gint l_sfbtable[5] = { 0, 6, 11, 16, 21 };
    /* LONG types 0,1,3 */
    if (gr == 0) {
      for (sfb = 0; sfb < 11; sfb++) {
        (*scalefac)[ch].l[sfb] = h_getbits (bb, slen0);
      }
      for (sfb = 11; sfb < 21; sfb++) {
        (*scalefac)[ch].l[sfb] = h_getbits (bb, slen1);
      }
    }
    else {
      for (i = 0; i < 2; i++) {
        if (si->scfsi[i][ch] == 0) {
          for (sfb = l_sfbtable[i]; sfb < l_sfbtable[i + 1]; sfb++) {
            (*scalefac)[ch].l[sfb] = h_getbits (bb, slen0);
          }
        }
      }
      for (/* i = 2 */; i < 4; i++) {
        if (si->scfsi[i][ch] == 0) {
          for (sfb = l_sfbtable[i]; sfb < l_sfbtable[i + 1]; sfb++) {
            (*scalefac)[ch].l[sfb] = h_getbits (bb, slen1);
          }
        }
      }
    }
    (*scalefac)[ch].l[21] = 0;
  }
}

/*** new MPEG2 stuff ***/

static const guint nr_of_sfb_block[6][3][4] =
    { 
      { {6,  5,  5, 5}, {9,  9,  9,  9}, {6,  9,  9,  9} },
      { {6,  5,  7, 3}, {9,  9,  12, 6}, {6,  9,  12, 6} },
      { {11, 10, 0, 0}, {18, 18, 0,  0}, {15, 18, 0,  0} },
      { {7,  7,  7, 0}, {12, 12, 12, 0}, {6,  15, 12, 0} },
      { {6,  6,  6, 3}, {12, 9,  9,  6}, {6,  12, 9,  6} },
      { {8,  8,  5, 0}, {15, 12, 9,  0}, {6,  18, 9,  0} }
};

void
III_get_LSF_scale_data (guint *scalefac_buffer, III_side_info_t *si,
    gint gr, gint ch, mp3tl *tl)
{
  short i, j, k;
  short blocktypenumber;
  short blocknumber = -1;

  gr_info_t *gr_info = &(si->gr[gr][ch]);
  guint scalefac_comp, int_scalefac_comp, new_slen[4];

  huffdec_bitbuf *bb = &tl->c_impl.bb;
  fr_header *hdr = &tl->fr_ps.header;

  scalefac_comp = gr_info->scalefac_compress;

  blocktypenumber = 0;
  if ((gr_info->block_type == 2) && (gr_info->mixed_block_flag == 0))
    blocktypenumber = 1;

  if ((gr_info->block_type == 2) && (gr_info->mixed_block_flag == 1))
    blocktypenumber = 2;

  if (!(((hdr->mode_ext == 1) || (hdr->mode_ext == 3)) && (ch == 1))) {
    if (scalefac_comp < 400) {
      new_slen[0] = (scalefac_comp >> 4) / 5;
      new_slen[1] = (scalefac_comp >> 4) % 5;
      new_slen[2] = (scalefac_comp % 16) >> 2;
      new_slen[3] = (scalefac_comp % 4);
      gr_info->preflag = 0;
      blocknumber = 0;
    }
    else if (scalefac_comp < 500) {
      new_slen[0] = ((scalefac_comp - 400) >> 2) / 5;
      new_slen[1] = ((scalefac_comp - 400) >> 2) % 5;
      new_slen[2] = (scalefac_comp - 400) % 4;
      new_slen[3] = 0;
      gr_info->preflag = 0;
      blocknumber = 1;
    }
    else if (scalefac_comp < 512) {
      new_slen[0] = (scalefac_comp - 500) / 3;
      new_slen[1] = (scalefac_comp - 500) % 3;
      new_slen[2] = 0;
      new_slen[3] = 0;
      gr_info->preflag = 1;
      blocknumber = 2;
    }
  }

  if ((((hdr->mode_ext == 1) || (hdr->mode_ext == 3)) && (ch == 1))) {
    /*   intensity_scale = scalefac_comp %2; */
    int_scalefac_comp = scalefac_comp >> 1;

    if (int_scalefac_comp < 180) {
      new_slen[0] = int_scalefac_comp / 36;
      new_slen[1] = (int_scalefac_comp % 36) / 6;
      new_slen[2] = (int_scalefac_comp % 36) % 6;
      new_slen[3] = 0;
      gr_info->preflag = 0;
      blocknumber = 3;
    }
    else if (int_scalefac_comp < 244) {
      new_slen[0] = ((int_scalefac_comp - 180) % 64) >> 4;
      new_slen[1] = ((int_scalefac_comp - 180) % 16) >> 2;
      new_slen[2] = (int_scalefac_comp - 180) % 4;
      new_slen[3] = 0;
      gr_info->preflag = 0;
      blocknumber = 4;
    }
    else if (int_scalefac_comp < 255) {
      new_slen[0] = (int_scalefac_comp - 244) / 3;
      new_slen[1] = (int_scalefac_comp - 244) % 3;
      new_slen[2] = 0;
      new_slen[3] = 0;
      gr_info->preflag = 0;
      blocknumber = 5;
    }
  }
  
  if (blocknumber < 0) {
    g_warning ("Invalid block number");
    return;
  }
  
  k = 0;
  for (i = 0; i < 4; i++) {
    guint slen = new_slen[i];
    if (slen == 0) {
      for (j = nr_of_sfb_block[blocknumber][blocktypenumber][i]; j > 0; j--) {
        scalefac_buffer[k] = 0;
        k++;
      }
    } else {
      for (j = nr_of_sfb_block[blocknumber][blocktypenumber][i]; j > 0; j--) {
        scalefac_buffer[k] = h_getbits (bb, slen);
        k++;
      }
    }
  }
  for (; k < 45; k++)
    scalefac_buffer[k] = 0;
}


void III_get_LSF_scale_factors (III_scalefac_t *scalefac, III_side_info_t *si,
     int gr, int ch, mp3tl *tl)
{
  int sfb, k = 0, window;
  gr_info_t *gr_info = &(si->gr[gr][ch]);
  guint *scalefac_buffer;

  scalefac_buffer = tl->c_impl.scalefac_buffer;
  III_get_LSF_scale_data (scalefac_buffer, si, gr, ch, tl);

  if (gr_info->window_switching_flag && (gr_info->block_type == 2)) {
    if (gr_info->mixed_block_flag) {    /* MIXED *//* NEW - ag 11/25 */
      for (sfb = 0; sfb < 8; sfb++) {
        (*scalefac)[ch].l[sfb] = scalefac_buffer[k];
        k++;
      }
      for (sfb = 3; sfb < 12; sfb++)
        for (window = 0; window < 3; window++) {
          (*scalefac)[ch].s[window][sfb] = scalefac_buffer[k];
          k++;
        }
      for (sfb = 12, window = 0; window < 3; window++)
        (*scalefac)[ch].s[window][sfb] = 0;
    } else {                    /* SHORT */
      for (sfb = 0; sfb < 12; sfb++)
        for (window = 0; window < 3; window++) {
          (*scalefac)[ch].s[window][sfb] = scalefac_buffer[k];
          k++;
        }
      for (sfb = 12, window = 0; window < 3; window++)
        (*scalefac)[ch].s[window][sfb] = 0;
    }
  } else {                      /* LONG types 0,1,3 */
    for (sfb = 0; sfb < 21; sfb++) {
      (*scalefac)[ch].l[sfb] = scalefac_buffer[k];
      k++;
    }
    (*scalefac)[ch].l[21] = 0;
  }
}

/* do the huffman-decoding 						*/
/* note! for counta,countb -the 4 bit value is returned in y, discard x */
static inline gboolean
huffman_decoder (huffdec_bitbuf *bb, 
    gint tnum, int *x, int *y, int *v, int *w)
{
  HUFFBITS level;
  guint point = 0;
  gboolean error = TRUE;
  const struct huffcodetab *h;

  g_return_val_if_fail (tnum >= 0 && tnum <= HTSIZE, FALSE);

  /* Grab a ptr to the huffman table to use */
  h = huff_tables + tnum;

  level = dmask;

  /* table 0 needs no bits */
  if (h->treelen == 0) {
    *x = *y = *v = *w = 0;
    return TRUE;
  }

  /* Lookup in Huffman table. */
  do {
    if (h->val[point][0] == 0) {        /*end of tree */      
      *x = h->val[point][1] >> 4;
      *y = h->val[point][1] & 0xf;

      error = FALSE;
      break;
    }
    if (h_get1bit (bb)) {
      while (h->val[point][1] >= MXOFF)
        point += h->val[point][1];
      point += h->val[point][1];
    } else {
      while (h->val[point][0] >= MXOFF)
        point += h->val[point][0];
      point += h->val[point][0];
    }
    level >>= 1;
  } while (level || (point < h->treelen));

  /* Check for error. */
  if (error) {
    /* set x and y to a medium value as a simple concealment */
    g_warning ("Illegal Huffman code in data.\n");
    *x = (h->xlen - 1) << 1;
    *y = (h->ylen - 1) << 1;
  }

  /* Process sign encodings for quadruples tables. */
  if (h->quad_table) {
    *v = (*y >> 3) & 1;
    *w = (*y >> 2) & 1;
    *x = (*y >> 1) & 1;
    *y = *y & 1;

    if (*v && (h_get1bit (bb) == 1))
        *v = -*v;
    if (*w && (h_get1bit (bb) == 1))
        *w = -*w;
    if (*x && (h_get1bit (bb) == 1))
        *x = -*x;
    if (*y && (h_get1bit (bb) == 1))
        *y = -*y;
  }
  /* Process sign and escape encodings for dual tables. */
  else {
    /* x and y are reversed in the test bitstream.
       Reverse x and y here to make test bitstream work. */

    if (h->linbits && ((h->xlen - 1) == *x))
        *x += h_getbits (bb, h->linbits);
    if (*x && (h_get1bit (bb) == 1))
        *x = -*x;

    if (h->linbits && ((h->ylen - 1) == *y))
        *y += h_getbits (bb, h->linbits);
    if (*y && (h_get1bit (bb) == 1))
        *y = -*y;
  }

  return !error;
}

gboolean
III_huffman_decode (gint is[SBLIMIT][SSLIMIT], III_side_info_t *si,
    gint ch, gint gr, gint part2_start, mp3tl *tl)
{
  guint i;
  int x, y;
  int v, w;
  gint h; /* Index of the huffman table to use */
  guint region1Start;
  guint region2Start;
  int sfreq;
  guint grBits;
  gr_info_t *gi = &(si->gr[gr][ch]);
  huffdec_bitbuf *bb = &tl->c_impl.bb;
  frame_params *fr_ps = &tl->fr_ps;

  /* Calculate index. */
  sfreq = sfb_offset[fr_ps->header.version] + fr_ps->header.srate_idx;
    
  /* Find region boundary for short block case. */
  if ((gi->window_switching_flag) &&
      (gi->block_type == 2)) {
    /* Region2. */
    region1Start = 36;          /* sfb[9/3]*3=36 */
    region2Start = 576;         /* No Region2 for short block case. */
  } else {                      /* Find region boundary for long block case. */
    region1Start = 
        sfBandIndex[sfreq].l[gi->region0_count + 1]; /* MI */
    region2Start = 
        sfBandIndex[sfreq].l[gi->region0_count + 
            gi->region1_count + 2]; /* MI */
  }

  /* Read bigvalues area. */
  /* i < SSLIMIT * SBLIMIT => gi->big_values < SSLIMIT * SBLIMIT/2 */
  for (i = 0; i < gi->big_values * 2; i += 2) {
    if (i < region1Start)
      h = gi->table_select[0];
    else if (i < region2Start)
      h = gi->table_select[1];
    else
      h = gi->table_select[2];

    if (!huffman_decoder (bb, h, &x, &y, &v, &w))
      return FALSE;
    is[i / SSLIMIT][i % SSLIMIT] = x;
    is[(i + 1) / SSLIMIT][(i + 1) % SSLIMIT] = y;
  }

  /* Read count1 area. */
  h = gi->count1table_select + 32;
  grBits = part2_start + gi->part2_3_length;

  while ((h_sstell (bb) < grBits) && (i + 3) < (SBLIMIT * SSLIMIT)) {
    if (!huffman_decoder (bb, h, &x, &y, &v, &w))
      return FALSE;

    is[i / SSLIMIT][i % SSLIMIT] = v;
    is[(i + 1) / SSLIMIT][(i + 1) % SSLIMIT] = w;
    is[(i + 2) / SSLIMIT][(i + 2) % SSLIMIT] = x;
    is[(i + 3) / SSLIMIT][(i + 3) % SSLIMIT] = y;
    i += 4;
  }

  if (h_sstell (bb) > grBits) {
    /* Didn't end exactly at the grBits boundary. Rewind one entry. */
    if (i >= 4)
      i -= 4;
    h_rewindNbits (bb, h_sstell (bb) - grBits);
  }

  /* Dismiss any stuffing Bits */
  if (h_sstell (bb) < grBits)
    h_flushbits (bb, grBits - h_sstell (bb));

  g_assert (i <= SSLIMIT * SBLIMIT);

  /* Zero out rest. */
  for (; i < SSLIMIT * SBLIMIT; i++)
    is[i / SSLIMIT][i % SSLIMIT] = 0;

  return TRUE;
}


const gint pretab[22] =
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 3, 3, 3, 2, 0 };

void
III_dequantize_sample (
     gint is[SBLIMIT][SSLIMIT],
     gfloat xr[SBLIMIT][SSLIMIT],
     III_scalefac_t *scalefac,
     gr_info_t *gr_info,
     gint ch,
     frame_params *fr_ps)
{
  int ss, sb, cb = 0, sfreq;

//   int stereo = fr_ps->stereo;
  int next_cb_boundary;
  int cb_begin = 0;
  int cb_width = 0;
  gint tmp;
  gint16 pow_factor;
  gboolean is_short_blk;

  /* Calculate index. */
  sfreq = sfb_offset[fr_ps->header.version] + fr_ps->header.srate_idx;

  /* choose correct scalefactor band per block type, initalize boundary */
  if (gr_info->window_switching_flag && (gr_info->block_type == 2)) {
    if (gr_info->mixed_block_flag) {
      next_cb_boundary = sfBandIndex[sfreq].l[1];       /* LONG blocks: 0,1,3 */
    } else {
      next_cb_boundary = sfBandIndex[sfreq].s[1] * 3;   /* pure SHORT block */
      cb_width = sfBandIndex[sfreq].s[1];
      cb_begin = 0;
    }
  } else {
    next_cb_boundary = sfBandIndex[sfreq].l[1]; /* LONG blocks: 0,1,3 */
  }

  /* apply formula per block type */
  for (sb = 0; sb < SBLIMIT; sb++) {
    gint sb_off = sb * 18;
    is_short_blk = gr_info->window_switching_flag && 
        (((gr_info->block_type == 2) && (gr_info->mixed_block_flag == 0)) || 
        ((gr_info->block_type == 2) && gr_info->mixed_block_flag && (sb >= 2)));

    for (ss = 0; ss < SSLIMIT; ss++) {
      if (sb_off + ss == next_cb_boundary) { /* Adjust critical band boundary */
        if (gr_info->window_switching_flag && (gr_info->block_type == 2)) {
          if (gr_info->mixed_block_flag) {
            if ((sb_off + ss) == sfBandIndex[sfreq].l[8]) {
              next_cb_boundary = sfBandIndex[sfreq].s[4] * 3;
              cb = 3;
              cb_width = sfBandIndex[sfreq].s[cb + 1] -
                  sfBandIndex[sfreq].s[cb];
              cb_begin = sfBandIndex[sfreq].s[cb] * 3;
            } else if ((sb_off + ss) < sfBandIndex[sfreq].l[8])
              next_cb_boundary = sfBandIndex[sfreq].l[(++cb) + 1];
            else {
              next_cb_boundary = sfBandIndex[sfreq].s[(++cb) + 1] * 3;
              cb_width = sfBandIndex[sfreq].s[cb + 1] -
                  sfBandIndex[sfreq].s[cb];
              cb_begin = sfBandIndex[sfreq].s[cb] * 3;
            }
          } else {
            next_cb_boundary = sfBandIndex[sfreq].s[(++cb) + 1] * 3;
            cb_width = sfBandIndex[sfreq].s[cb + 1] - sfBandIndex[sfreq].s[cb];
            cb_begin = sfBandIndex[sfreq].s[cb] * 3;
          }
        } else                  /* long blocks */
          next_cb_boundary = sfBandIndex[sfreq].l[(++cb) + 1];
      }

      /* Compute overall (global) scaling. */
      pow_factor = gr_info->global_gain;

      /* Do long/short dependent scaling operations. */
      if (is_short_blk) {
        pow_factor -=
            8 * gr_info->subblock_gain[((sb_off + ss) - cb_begin) / cb_width];
        pow_factor -=  gr_info->scalefac_scale *
            (*scalefac)[ch].s[(sb_off + ss - cb_begin) / cb_width][cb];
      } else {
        /* LONG block types 0,1,3 & 1st 2 subbands of switched blocks */
        pow_factor -= gr_info->scalefac_scale * 
            ((*scalefac)[ch].l[cb] + gr_info->preflag * pretab[cb]);
      }

#if 1
      /* g_assert (pow_factor >= 0 && pow_factor < 
          (sizeof (pow_2_table) / sizeof (pow_2_table[0]))); */
      xr[sb][ss] = pow_2_table[pow_factor];
#else
      /* Old method using powf */
      pow_factor -= 326;
      if (pow_factor >= (-140))
        xr[sb][ss] = powf (2.0, 0.25 * (pow_factor));
      else
        xr[sb][ss] = 0;
#endif

      /* Scale quantized value. */
      tmp = is[sb][ss];
      if (tmp >= 0) {
        xr[sb][ss] *= pow_43_table [tmp];
      }
      else {
        xr[sb][ss] *= -1.0f * pow_43_table [-tmp];
      }
    }
  }
}

void
III_reorder (gfloat xr[SBLIMIT][SSLIMIT], gfloat ro[SBLIMIT][SSLIMIT],
    gr_info_t *gr_info, frame_params *fr_ps)
{
  int sfreq;
  int sfb, sfb_start, sfb_lines;
  int sb, ss, window, freq, src_line, des_line;

  /* Calculate index. */
  sfreq = sfb_offset[fr_ps->header.version] + fr_ps->header.srate_idx;

  if (gr_info->window_switching_flag && (gr_info->block_type == 2)) {
    for (sb = 0; sb < SBLIMIT; sb++)
      for (ss = 0; ss < SSLIMIT; ss++)
        ro[sb][ss] = 0;

    if (gr_info->mixed_block_flag) {
      /* NO REORDER FOR LOW 2 SUBBANDS */
      for (sb = 0; sb < 2; sb++)
        for (ss = 0; ss < SSLIMIT; ss++) {
          ro[sb][ss] = xr[sb][ss];
        }
      /* REORDERING FOR REST SWITCHED SHORT */
      for (sfb = 3, sfb_start = sfBandIndex[sfreq].s[3],
          sfb_lines = sfBandIndex[sfreq].s[4] - sfb_start;
          sfb < 13; sfb++, sfb_start = sfBandIndex[sfreq].s[sfb],
          (sfb_lines = sfBandIndex[sfreq].s[sfb + 1] - sfb_start))
        for (window = 0; window < 3; window++)
          for (freq = 0; freq < sfb_lines; freq++) {
            src_line = sfb_start * 3 + window * sfb_lines + freq;
            des_line = (sfb_start * 3) + window + (freq * 3);
            ro[des_line / SSLIMIT][des_line % SSLIMIT] =
                xr[src_line / SSLIMIT][src_line % SSLIMIT];
          }
    } else {                    /* pure short */
      for (sfb = 0, sfb_start = 0, sfb_lines = sfBandIndex[sfreq].s[1];
          sfb < 13; sfb++, sfb_start = sfBandIndex[sfreq].s[sfb],
          (sfb_lines = sfBandIndex[sfreq].s[sfb + 1] - sfb_start))
        for (window = 0; window < 3; window++)
          for (freq = 0; freq < sfb_lines; freq++) {
            src_line = sfb_start * 3 + window * sfb_lines + freq;
            des_line = (sfb_start * 3) + window + (freq * 3);
            ro[des_line / SSLIMIT][des_line % SSLIMIT] =
                xr[src_line / SSLIMIT][src_line % SSLIMIT];
          }
    }
  } else {                      /*long blocks */
    for (sb = 0; sb < SBLIMIT; sb++)
      for (ss = 0; ss < SSLIMIT; ss++)
        ro[sb][ss] = xr[sb][ss];
  }
}

static void
III_i_stereo_k_values (gint is_pos, gfloat io, gint i, gfloat k[2][576])
{
  if (is_pos == 0) {
    k[0][i] = 1;
    k[1][i] = 1;
  } else if ((is_pos % 2) == 1) {
    k[0][i] = powf (io, ((is_pos + 1) / 2));
    k[1][i] = 1;
  } else {
    k[0][i] = 1;
    k[1][i] = powf (io, (is_pos / 2));
  }
}

void
III_stereo (gfloat xr[2][SBLIMIT][SSLIMIT], gfloat lr[2][SBLIMIT][SSLIMIT],
     III_scalefac_t *scalefac, gr_info_t *gr_info, frame_params *fr_ps)
{
  int sfreq;
  int stereo = fr_ps->stereo;
  int ms_stereo = (fr_ps->header.mode == MPG_MD_JOINT_STEREO) &&
      (fr_ps->header.mode_ext & 0x2);
  int i_stereo = (fr_ps->header.mode == MPG_MD_JOINT_STEREO) &&
      (fr_ps->header.mode_ext & 0x1);
  int sfb;
  int i, j, sb, ss;
  short is_pos[SBLIMIT * SSLIMIT];
  gfloat is_ratio[SBLIMIT * SSLIMIT];
  gfloat io;
  gfloat k[2][SBLIMIT * SSLIMIT];

  int lsf = (fr_ps->header.version != MPEG_VERSION_1);

  if ((gr_info->scalefac_compress % 2) == 1) {
    io = 0.707106781188f;
  } else {
    io = 0.840896415256f;
  }

  /* Calculate index. */
  sfreq = sfb_offset[fr_ps->header.version] + fr_ps->header.srate_idx;

  /* intialization */
  for (i = 0; i < SBLIMIT * SSLIMIT; i++)
    is_pos[i] = 7;

  if ((stereo == 2) && i_stereo) {
    if (gr_info->window_switching_flag && (gr_info->block_type == 2)) {
      if (gr_info->mixed_block_flag) {
        int max_sfb = 0;

        for (j = 0; j < 3; j++) {
          int sfbcnt;

          sfbcnt = 2;
          for (sfb = 12; sfb >= 3; sfb--) {
            int lines;

            lines = sfBandIndex[sfreq].s[sfb + 1] - sfBandIndex[sfreq].s[sfb];
            i = 3 * sfBandIndex[sfreq].s[sfb] + (j + 1) * lines - 1;
            while (lines > 0) {
              if (xr[1][i / SSLIMIT][i % SSLIMIT] != 0.0) {
                sfbcnt = sfb;
                sfb = -10;
                lines = -10;
              }
              lines--;
              i--;
            }
          }
          sfb = sfbcnt + 1;

          if (sfb > max_sfb)
            max_sfb = sfb;

          while (sfb < 12) {
            sb = sfBandIndex[sfreq].s[sfb + 1] - sfBandIndex[sfreq].s[sfb];
            i = 3 * sfBandIndex[sfreq].s[sfb] + j * sb;
            for (; sb > 0; sb--) {
              is_pos[i] = (*scalefac)[1].s[j][sfb];
              if (is_pos[i] != 7) {
                if (lsf) {
                  III_i_stereo_k_values (is_pos[i], io, i, k);
                } else {
                  is_ratio[i] = tanf (is_pos[i] * (PI / 12));
                }
              }
              i++;
            }
            sfb++;
          }

          sb = sfBandIndex[sfreq].s[12] - sfBandIndex[sfreq].s[11];
          sfb = 3 * sfBandIndex[sfreq].s[11] + j * sb;
          sb = sfBandIndex[sfreq].s[13] - sfBandIndex[sfreq].s[12];

          i = 3 * sfBandIndex[sfreq].s[11] + j * sb;
          for (; sb > 0; sb--) {
            is_pos[i] = is_pos[sfb];
            is_ratio[i] = is_ratio[sfb];
            k[0][i] = k[0][sfb];
            k[1][i] = k[1][sfb];
            i++;
          }
        }
        if (max_sfb <= 3) {
          i = 2;
          ss = 17;
          sb = -1;
          while (i >= 0) {
            if (xr[1][i][ss] != 0.0) {
              sb = i * 18 + ss;
              i = -1;
            } else {
              ss--;
              if (ss < 0) {
                i--;
                ss = 17;
              }
            }
          }
          i = 0;
          while (sfBandIndex[sfreq].l[i] <= sb)
            i++;
          sfb = i;
          i = sfBandIndex[sfreq].l[i];
          for (; sfb < 8; sfb++) {
            sb = sfBandIndex[sfreq].l[sfb + 1] - sfBandIndex[sfreq].l[sfb];
            for (; sb > 0; sb--) {
              is_pos[i] = (*scalefac)[1].l[sfb];
              if (is_pos[i] != 7) {
                if (lsf) {
                  III_i_stereo_k_values (is_pos[i], io, i, k);
                } else {
                  is_ratio[i] = tanf (is_pos[i] * (PI / 12));
                }
              }
              i++;
            }
          }
        }
      } else {
        for (j = 0; j < 3; j++) {
          int sfbcnt;

          sfbcnt = -1;
          for (sfb = 12; sfb >= 0; sfb--) {
            int lines;

            lines = sfBandIndex[sfreq].s[sfb + 1] - sfBandIndex[sfreq].s[sfb];
            i = 3 * sfBandIndex[sfreq].s[sfb] + (j + 1) * lines - 1;
            while (lines > 0) {
              if (xr[1][i / SSLIMIT][i % SSLIMIT] != 0.0) {
                sfbcnt = sfb;
                sfb = -10;
                lines = -10;
              }
              lines--;
              i--;
            }
          }
          sfb = sfbcnt + 1;
          while (sfb < 12) {
            sb = sfBandIndex[sfreq].s[sfb + 1] - sfBandIndex[sfreq].s[sfb];
            i = 3 * sfBandIndex[sfreq].s[sfb] + j * sb;
            for (; sb > 0; sb--) {
              is_pos[i] = (*scalefac)[1].s[j][sfb];
              if (is_pos[i] != 7) {
                if (lsf) {
                  III_i_stereo_k_values (is_pos[i], io, i, k);
                } else {
                  is_ratio[i] = tanf (is_pos[i] * (PI / 12));
                }
              }
              i++;
            }
            sfb++;
          }

          sb = sfBandIndex[sfreq].s[12] - sfBandIndex[sfreq].s[11];
          sfb = 3 * sfBandIndex[sfreq].s[11] + j * sb;
          sb = sfBandIndex[sfreq].s[13] - sfBandIndex[sfreq].s[12];

          i = 3 * sfBandIndex[sfreq].s[11] + j * sb;
          for (; sb > 0; sb--) {
            is_pos[i] = is_pos[sfb];
            is_ratio[i] = is_ratio[sfb];
            k[0][i] = k[0][sfb];
            k[1][i] = k[1][sfb];
            i++;
          }
        }
      }
    } else {
      i = 31;
      ss = 17;
      sb = 0;
      while (i >= 0) {
        if (xr[1][i][ss] != 0.0) {
          sb = i * 18 + ss;
          i = -1;
        } else {
          ss--;
          if (ss < 0) {
            i--;
            ss = 17;
          }
        }
      }
      i = 0;
      while (sfBandIndex[sfreq].l[i] <= sb)
        i++;
      sfb = i;
      i = sfBandIndex[sfreq].l[i];
      for (; sfb < 21; sfb++) {
        sb = sfBandIndex[sfreq].l[sfb + 1] - sfBandIndex[sfreq].l[sfb];
        for (; sb > 0; sb--) {
          is_pos[i] = (*scalefac)[1].l[sfb];
          if (is_pos[i] != 7) {
            if (lsf) {
              III_i_stereo_k_values (is_pos[i], io, i, k);
            } else {
              is_ratio[i] = tanf (is_pos[i] * (PI / 12));
            }
          }
          i++;
        }
      }
      sfb = sfBandIndex[sfreq].l[20];
      if (i > sfBandIndex[sfreq].l[21])
        sb = 576 - i;
      else
        sb = 576 - sfBandIndex[sfreq].l[21];

      for (;sb > 0; sb--) {
        is_pos[i] = is_pos[sfb];
        is_ratio[i] = is_ratio[sfb];
        k[0][i] = k[0][sfb];
        k[1][i] = k[1][sfb];
        i++;
      }
    }
  }

#if 0
  for (ch = 0; ch < 2; ch++)
    for (sb = 0; sb < SBLIMIT; sb++)
      for (ss = 0; ss < SSLIMIT; ss++)
        lr[ch][sb][ss] = 0;
#else
  memset (lr, 0, sizeof (gfloat) * 2 * SBLIMIT * SSLIMIT);
#endif

  if (stereo == 2)
    for (sb = 0; sb < SBLIMIT; sb++)
      for (ss = 0; ss < SSLIMIT; ss++) {
        i = (sb * 18) + ss;
        if (is_pos[i] == 7) {
          if (ms_stereo) {
            lr[0][sb][ss] = (gfloat)
                ((xr[0][sb][ss] + xr[1][sb][ss]) / (double) 1.41421356);
            lr[1][sb][ss] = (gfloat)
                ((xr[0][sb][ss] - xr[1][sb][ss]) / (double) 1.41421356);
          } else {
            lr[0][sb][ss] = xr[0][sb][ss];
            lr[1][sb][ss] = xr[1][sb][ss];
          }
        } else if (i_stereo) {
          if (lsf) {
            lr[0][sb][ss] = xr[0][sb][ss] * k[0][i];
            lr[1][sb][ss] = xr[0][sb][ss] * k[1][i];
          } else {
            lr[0][sb][ss] = xr[0][sb][ss] * (is_ratio[i] / (1 + is_ratio[i]));
            lr[1][sb][ss] = xr[0][sb][ss] * (1 / (1 + is_ratio[i]));
          }
        } else {
          g_warning ("Error in stereo processing\n");
        }
  } else /* mono , bypass xr[0][][] to lr[0][][] */
    for (sb = 0; sb < SBLIMIT; sb++)
      for (ss = 0; ss < SSLIMIT; ss++)
        lr[0][sb][ss] = xr[0][sb][ss];

}

static const gfloat Ci[8] =
    { -0.6f, -0.535f, -0.33f, -0.185f, -0.095f, -0.041f, -0.0142f, -0.0037f };

void
III_antialias (gfloat xr[SBLIMIT][SSLIMIT],
     gfloat hybridIn[SBLIMIT][SSLIMIT],
     gr_info_t *gr_info)
{
  /* Static shared computed constants */
  static int init = 0;
  static gfloat sca[8], scs[8];
  gfloat bu, bd;                /* upper and lower butterfly inputs */
  int ss, sb, sblim;
  const gfloat *ca = sca;
  const gfloat *cs = scs;

  if (!init) {
    int i;
    gfloat sq;

    for (i = 0; i < 8; i++) {
      sq = (gfloat) sqrt (1.0 + Ci[i] * Ci[i]);
      scs[i] = 1.0f / sq;
      sca[i] = Ci[i] / sq;
    }
    init = 1;
  }

  /* clear all inputs */
  for (sb = 0; sb < SBLIMIT; sb++)
    for (ss = 0; ss < SSLIMIT; ss++)
      hybridIn[sb][ss] = xr[sb][ss];

  if (gr_info->window_switching_flag && (gr_info->block_type == 2) &&
      !gr_info->mixed_block_flag)
    return;

  if (gr_info->window_switching_flag && gr_info->mixed_block_flag &&
      (gr_info->block_type == 2))
    sblim = 1;
  else
    sblim = SBLIMIT - 1;

  /* 31 alias-reduction operations between each pair of sub-bands */
  /* with 8 butterflies between each pair                         */

  for (sb = 0; sb < sblim; sb++)
    for (ss = 0; ss < 8; ss++) {
      bu = xr[sb][17 - ss];
      bd = xr[sb + 1][ss];
      hybridIn[sb][17 - ss] = (bu * cs[ss]) - (bd * ca[ss]);
      hybridIn[sb + 1][ss] = (bd * cs[ss]) + (bu * ca[ss]);
    }
}

static inline void
imdct_9pt (gfloat invec[9], gfloat outvec[9]);


#define ICOS24(i) (cos24_table[(i)])

#define COS18(i) (cos18_table[(i)])
#define ICOS36_A(i) (icos72_table[4*(i)+1])
#define ICOS72_A(i) (icos72_table[2*(i)])


/* Static shared computed constants */
static int mdct_inited = 0;
static gfloat mdct_swin[4][36];

/* Short (12 point) version of the IMDCT performed
   as 2 x 3-point IMDCT */
static inline void
inv_mdct_s (gfloat invec[6], gfloat outvec[12])
{
  int i;
  gfloat H[6], h[6], even_idct[3], odd_idct[3], *tmp;
  gfloat t0, t1, t2;
  /* sqrt (3) / 2.0 */
  const gfloat sqrt32 = 0.8660254037844385965883020617184229195117950439453125f;

  /* Preprocess the input to the two 3-point IDCT's */
  tmp = invec;
  for (i = 1; i < 6; i++) {
    H[i] = tmp[0] + tmp[1];
    tmp++;
  }

  /* 3-point IMDCT */
  t0 = H[4]/2.0f + invec[0];
  t1 = H[2] * sqrt32;
  even_idct[0] = t0 + t1;
  even_idct[1] = invec[0] - H[4];
  even_idct[2] = t0 - t1;
  /* END 3-point IMDCT */
  
  /* 3-point IMDCT */
  t2 = H[3] + H[5];

  t0 = (t2)/2.0f + H[1];
  t1 = (H[1] + H[3]) * sqrt32;
  odd_idct[0] = t0 + t1;
  odd_idct[1] = H[1] - t2;
  odd_idct[2] = t0 - t1;
  /* END 3-point IMDCT */
  
  /* Post-Twiddle */
  odd_idct[0] *= ICOS24(1);
  odd_idct[1] *= ICOS24(5);
  odd_idct[2] *= ICOS24(9);

  h[0] = (even_idct[0] + odd_idct[0]) * ICOS24(0);
  h[1] = (even_idct[1] + odd_idct[1]) * ICOS24(2);
  h[2] = (even_idct[2] + odd_idct[2]) * ICOS24(4); 

  h[3] = (even_idct[2] - odd_idct[2]) * ICOS24(6);
  h[4] = (even_idct[1] - odd_idct[1]) * ICOS24(8);
  h[5] = (even_idct[0] - odd_idct[0]) * ICOS24(10);

  /* Rearrange the 6 values from the IDCT to the output vector */
  outvec[0]  =  h[3];
  outvec[1]  =  h[4];
  outvec[2]  =  h[5];
  outvec[3]  = -h[5];
  outvec[4]  = -h[4];
  outvec[5]  = -h[3];
  outvec[6]  = -h[2];
  outvec[7]  = -h[1];
  outvec[8]  = -h[0];
  outvec[9]  = -h[0];
  outvec[10] = -h[1];
  outvec[11] = -h[2];  
}

static inline void
inv_mdct_l (gfloat invec[18], gfloat outvec[36])
{
  int i;
  gfloat H[17], h[18], even[9], odd[9], even_idct[9], odd_idct[9], *tmp;
  
#ifdef USE_LIBOIL
  oil_add_f32 (H, invec, invec + 1, 17);
#else
  for (i = 0; i < 17; i++)
    H[i] = invec[i] + invec[i + 1];
#endif

  even[0] = invec[0];
  odd[0] = H[0];
  tmp = H;
  for (i = 1; i < 9; i++) {
    even[i] = tmp[1];
    odd[i] = tmp[0] + tmp[2];
    tmp += 2;
  }

  imdct_9pt (even, even_idct);  
  imdct_9pt (odd, odd_idct);
  
  for (i = 0; i < 9; i++) {
    odd_idct[i] *= ICOS36_A(i);
    h[i] = (even_idct[i] + odd_idct[i]) * ICOS72_A (i);
  }
  for (/* i = 9 */; i < 18; i++) {
    h[i] = (even_idct[17-i] - odd_idct[17-i]) * ICOS72_A (i);
  }

  /* Rearrange the 18 values from the IDCT to the output vector */
  outvec[0]  =  h[9];
  outvec[1]  =  h[10];
  outvec[2]  =  h[11];
  outvec[3]  =  h[12];
  outvec[4]  =  h[13];
  outvec[5]  =  h[14];
  outvec[6]  =  h[15];
  outvec[7]  =  h[16];
  outvec[8]  =  h[17];
  
  outvec[9]  = -h[17];
  outvec[10] = -h[16];
  outvec[11] = -h[15];
  outvec[12] = -h[14];
  outvec[13] = -h[13];
  outvec[14] = -h[12];
  outvec[15] = -h[11];
  outvec[16] = -h[10];
  outvec[17] = -h[9];

  outvec[35] = outvec[18] = -h[8];
  outvec[34] = outvec[19] = -h[7];
  outvec[33] = outvec[20] = -h[6];
  outvec[32] = outvec[21] = -h[5];
  outvec[31] = outvec[22] = -h[4];
  outvec[30] = outvec[23] = -h[3];
  outvec[29] = outvec[24] = -h[2];
  outvec[28] = outvec[25] = -h[1];
  outvec[27] = outvec[26] = -h[0];
}

static inline void
imdct_9pt (gfloat invec[9], gfloat outvec[9])
{
  int i;
  gfloat even_idct[5], odd_idct[4];
  gfloat t0, t1, t2;

  /* BEGIN 5 Point IMDCT */
  t0 = invec[6] / 2.0f + invec[0];
  t1 = invec[0] - invec[6];
  t2 = invec[2] - invec[4] - invec[8];

  even_idct[0] = t0 + invec[2] * COS18 (2)
      + invec[4] * COS18 (4) + invec[8] * COS18 (8); 
      
  even_idct[1] = t2 / 2.0f + t1;
  even_idct[2] = t0 - invec[2] * COS18 (8) 
      - invec[4] * COS18 (2) + invec[8] * COS18 (4);
      
  even_idct[3] = t0 - invec[2] * COS18 (4) 
      + invec[4] * COS18 (8) - invec[8] * COS18 (2);
  
  even_idct[4] = t1 - t2;
  /* END 5 Point IMDCT */

  /* BEGIN 4 Point IMDCT */
  {
    gfloat odd1, odd2;
    odd1 = invec[1] + invec[3];
    odd2 = invec[3] + invec[5];
    t0 = (invec[5] + invec[7]) * 0.5f + invec[1];

    odd_idct[0] = t0 + odd1 * COS18 (2) + odd2 * COS18 (4);
    odd_idct[1] = (invec[1] - invec[5]) * 1.5f - invec[7];
    odd_idct[2] = t0 - odd1 * COS18 (8) - odd2 * COS18 (2);
    odd_idct[3] = t0 - odd1 * COS18 (4) + odd2 * COS18 (8);
  }
  /* END 4 Point IMDCT */

  /* Adjust for non power of 2 IDCT */
  odd_idct[0] +=  invec[7] * COS18 (8);
  odd_idct[1] -=  invec[7] * COS18 (6);
  odd_idct[2] +=  invec[7] * COS18 (4);
  odd_idct[3] -=  invec[7] * COS18 (2);
  
  /* Post-Twiddle */
  odd_idct[0] *= 0.5f / COS18 (1);
  odd_idct[1] *= 0.5f / COS18 (3);
  odd_idct[2] *= 0.5f / COS18 (5);
  odd_idct[3] *= 0.5f / COS18 (7);

  for (i = 0; i < 4; i++) {
    outvec[i] = even_idct[i] + odd_idct[i];
  }
  outvec[4] = even_idct[4];
  /* Mirror into the other half of the vector */
  for (i = 5; i < 9; i++) {
    outvec[i] = even_idct[8-i] - odd_idct[8-i];
  }
}

static void
inv_mdct (gfloat in[18], gfloat out[36], gint block_type)
{
  int i, m, p;

  const gfloat (*win)[36] = (const gfloat (*)[36])mdct_swin;

  if (block_type == 2) {
    gfloat tmp[12], tin[18], *tmpptr;
    for (i = 0; i < 36; i++) {
      out[i] = 0.0;
    }
    
    /* The short blocks input vector has to be re-arranged */
    tmpptr = tin;
    for (i = 0; i < 3; i++) {
      gfloat *v = &(in[i]); /* Input vector */
      for (m = 0; m < 6; m++) {
        tmpptr[m] = *v;
        v += 3;
      }
      tmpptr += 6;
    }
    
    for (i = 0; i < 18; i += 6) {
      tmpptr = &(out[i+6]);
      
      inv_mdct_s (&(tin[i]), tmp);
      
      /* The three short blocks must be windowed and overlapped added
       * with each other */
      for (p = 0; p < 12; p++) {
      	tmpptr[p] += tmp[p] * win[2][p];
      }
    } /* end for (i... */
  } else { /* block_type != 2 */
    inv_mdct_l (in, out);

    /* Window the imdct result */
#if defined(USE_LIBOIL)
    oil_multiply_f32 (out, out, win[block_type], 36);
#else
    for (i = 0; i < 36; i++)
      out[i] = out[i] * win[block_type][i];
#endif
  }
}

static void init_mdct ()
{
  gint i;

  if (G_UNLIKELY (mdct_inited))
    return;

  /* type 0 */
  for (i = 0; i < 36; i++)
    mdct_swin[0][i] = (gfloat) sin (PI / 36 * (i + 0.5));

  /* type 1 */
  for (i = 0; i < 18; i++)
    mdct_swin[1][i] = (gfloat) sin (PI / 36 * (i + 0.5));
  for (i = 18; i < 24; i++)
    mdct_swin[1][i] = 1.0;
  for (i = 24; i < 30; i++)
    mdct_swin[1][i] = (gfloat) sin (PI / 12 * (i + 0.5 - 18));
  for (i = 30; i < 36; i++)
    mdct_swin[1][i] = 0.0;

  /* type 3 */
  for (i = 0; i < 6; i++)
    mdct_swin[3][i] = 0.0;
  for (i = 6; i < 12; i++)
    mdct_swin[3][i] = (gfloat) sin (PI / 12 * (i + 0.5 - 6));
  for (i = 12; i < 18; i++)
    mdct_swin[3][i] = 1.0;
  for (i = 18; i < 36; i++)
    mdct_swin[3][i] = (gfloat) sin (PI / 36 * (i + 0.5));

  /* type 2 */
  for (i = 0; i < 12; i++)
    mdct_swin[2][i] = (gfloat) sin (PI / 12 * (i + 0.5));
  for (i = 12; i < 36; i++)
    mdct_swin[2][i] = 0.0;

  mdct_inited = 1;
}

void
init_hybrid (mp3cimpl_info *c_impl)
{
  int i, j, k;

  for (i = 0; i < 2; i++)
    for (j = 0; j < SBLIMIT; j++)
      for (k = 0; k < SSLIMIT; k++)
        c_impl->prevblck[i][j][k] = 0.0;
}

/* III_hybrid
 * Parameters:
 *   double fsIn[SSLIMIT]      - freq samples per subband in 
 *   double tsOut[SSLIMIT]     - time samples per subband out
 *   int sb, ch
 *   gr_info_t *gr_info
 *   frame_params *fr_ps
 */
static void
III_hybrid (gfloat fsIn[SSLIMIT], gfloat tsOut[SSLIMIT], int sb, int ch,
    gr_info_t *gr_info, mp3tl *tl)
{
  int ss;
  gfloat rawout[36];
  int bt;
  bt = (gr_info->window_switching_flag && gr_info->mixed_block_flag &&
      (sb < 2)) ? 0 : gr_info->block_type;

  inv_mdct (fsIn, rawout, bt);

  /* overlap addition */
#ifdef USE_LIBOIL
  oil_add_f32 (tsOut, rawout, (gfloat*) tl->c_impl.prevblck[ch][sb], SSLIMIT);
  oil_memcpy (tl->c_impl.prevblck[ch][sb], rawout + 18, 
      SSLIMIT *sizeof (gfloat));
#else
  for (ss = 0; ss < SSLIMIT; ss++) {
    tsOut[ss] = (gfloat) (rawout[ss] + tl->c_impl.prevblck[ch][sb][ss]);
    tl->c_impl.prevblck[ch][sb][ss] = rawout[ss + 18];
  }
#endif
}

/* Invert the odd frequencies for odd subbands in preparation for polyphase
 * filtering */
static void
III_frequency_inversion (gfloat hybridOut[SBLIMIT][SSLIMIT], 
    mp3tl *tl ATTR_UNUSED)
{
  guint ss, sb;

  for (ss = 1; ss < 18; ss += 2) {
    for (sb = 1; sb < SBLIMIT; sb += 2) {
      hybridOut[sb][ss] = -hybridOut[sb][ss];
    }
  }
}

Mp3TlRetcode
c_decode_mp3 (mp3tl *tl)
{
  III_scalefac_t III_scalefac;
  III_side_info_t III_side_info;
  huffdec_bitbuf *bb;
  guint gr, ch, sb;
  fr_header *hdr;
  guint8 side_info[32]; /* At most 32 bytes side info for MPEG-1 stereo */
  
  hdr = &tl->fr_ps.header;
  bb = &tl->c_impl.bb;

  /* Check enough side_info data available */
  if (bs_bits_avail (tl->bs) < hdr->side_info_slots * 8)
    return MP3TL_ERR_NEED_DATA;

  bs_getbytes (tl->bs, side_info, hdr->side_info_slots);
  if (!III_get_side_info (side_info, &III_side_info, &tl->fr_ps)) {
    GST_DEBUG ("Bad side info");
    return MP3TL_ERR_BAD_FRAME;
  }

  /* Check enough main_data available */
  if (bs_bits_avail (tl->bs) < hdr->main_slots * 8)
    return MP3TL_ERR_NEED_DATA;

  /* Check that we have enough main_data between the bit reservoir
   * and the incoming data */
  if (tl->c_impl.main_data_end - III_side_info.main_data_begin < 0) {
    /* Usually happens after a seek. We can't decode this frame */
    GST_LOG ("Not enough main data available to decode frame");
    return MP3TL_ERR_BAD_FRAME;
  }

  /* Copy the remaining main data in the bit reservoir to the start of the
   * huffman bit buffer, and then append the incoming bytes */
  memmove (tl->c_impl.hb_buf, tl->c_impl.hb_buf + 
     tl->c_impl.main_data_end - III_side_info.main_data_begin,
     III_side_info.main_data_begin);
  tl->c_impl.main_data_end = III_side_info.main_data_begin;

  /* And append the incoming bytes to the reservoir */
  bs_getbytes (tl->bs, tl->c_impl.hb_buf + tl->c_impl.main_data_end,
     hdr->main_slots);
  tl->c_impl.main_data_end += hdr->main_slots;

  /* And setup the huffman bitstream reader for this data */
  h_setbuf (bb, tl->c_impl.hb_buf, tl->c_impl.main_data_end);

#if 0
  g_print ("Mode %d mode_ext %d\n", hdr->mode, hdr->mode_ext);
  g_print ("Frame: main_data %d bytes. need %d prev bytes. Avail %d (of %d)\n", 
      hdr->main_slots, III_side_info.main_data_begin, h_bytes_avail (bb),
      III_side_info.main_data_begin + hdr->main_slots);
#endif

  /* Clear the scale factors to avoid problems with badly coded files
   * that try to reuse scalefactors from the first granule when they didn't
   * supply them. */
  memset (III_scalefac, 0, sizeof (III_scalefac_t));

  for (gr = 0; gr < tl->n_granules; gr++) {
    gfloat lr[2][SBLIMIT][SSLIMIT], ro[2][SBLIMIT][SSLIMIT];

    for (ch = 0; ch < hdr->channels; ch++) {
      gint is[SBLIMIT][SSLIMIT];        /* Quantized samples. */
      int part2_start;

      part2_start = h_sstell (bb);
      if (hdr->version == MPEG_VERSION_1) {
        III_get_scale_factors (&III_scalefac, &III_side_info, gr, ch,
            tl);
      } else {
        III_get_LSF_scale_factors (&III_scalefac,
            &III_side_info, gr, ch, tl);
      }

      if (III_side_info.gr[gr][ch].big_values > ((SBLIMIT*SSLIMIT)/2)) {
        GST_DEBUG ("Bad side info decoding frame: big_values");
        return MP3TL_ERR_BAD_FRAME;
      }

      if (!III_huffman_decode (is, &III_side_info, ch, gr, part2_start, tl)) {
        GST_DEBUG ("Failed to decode huffman info");
        return MP3TL_ERR_BAD_FRAME;
      }

      III_dequantize_sample (is, ro[ch], &III_scalefac,
          &(III_side_info.gr[gr][ch]), ch, &tl->fr_ps);

#if 0
      int i;
      for (sb = 0; sb < SBLIMIT; sb++) {
        g_print ("SB %02d: ", sb);
        for (i = 0; i < SSLIMIT; i++) {
          g_print ("%06f ", ro[ch][sb][i]);
        }
        g_print ("\n");
      }
#endif
    }

    III_stereo (ro, lr, &III_scalefac,
        &(III_side_info.gr[gr][0]), &tl->fr_ps);

    for (ch = 0; ch < hdr->channels; ch++) {
      gfloat re[SBLIMIT][SSLIMIT];
      gfloat hybridIn[SBLIMIT][SSLIMIT];    /* Hybrid filter input */
      gfloat hybridOut[SBLIMIT][SSLIMIT];   /* Hybrid filter out */
      gr_info_t *gi = &(III_side_info.gr[gr][ch]);
      
      III_reorder (lr[ch], re, gi, &tl->fr_ps);
      
      /* Antialias butterflies. */
      III_antialias (re, hybridIn, gi);
#if 0
      int i;
      g_print ("HybridIn\n");
      for (sb = 0; sb < SBLIMIT; sb++) {
        g_print ("SB %02d: ", sb);
        for (i = 0; i < SSLIMIT; i++) {
          g_print ("%06f ", hybridIn[sb][i]);
        }
        g_print ("\n");
      }
#endif

      for (sb = 0; sb < SBLIMIT; sb++) {
        /* Hybrid synthesis. */
        III_hybrid (hybridIn[sb], hybridOut[sb], sb, ch, gi, tl);
      }

      /* Frequency inversion for polyphase. Invert odd numbered indices */
      III_frequency_inversion (hybridOut, tl);

#if 0
      g_print ("HybridOut\n");
      for (sb = 0; sb < SBLIMIT; sb++) {
        g_print ("SB %02d: ", sb);
        for (i = 0; i < SSLIMIT; i++) {
          g_print ("%06f ", hybridOut[sb][i]);
        }
        g_print ("\n");
      }
#endif

      /* Polyphase synthesis */
      III_subband_synthesis (tl, &tl->fr_ps, hybridOut, ch, tl->pcm_sample[ch]);
#if 0
      if (ch == 0) {
        g_print ("synth\n");

        for (i = 0; i < SSLIMIT; i++) {
          g_print ("SS %02d: ", i);
          for (sb = 0; sb < SBLIMIT; sb++) {
            g_print ("%04d ", tl->pcm_sample[ch][sb][i]);
        }
        g_print ("\n");
      }
}
#endif

    }
    /* Output PCM sample points for one granule. */
    out_fifo (tl->pcm_sample, 18, &tl->fr_ps, tl->sample_buf, 
        &tl->sample_w, SAMPLE_BUF_SIZE);
  }
#if 0
  g_print ("After: %d bytes left\n", h_bytes_avail (bb));
#endif
  
  return MP3TL_ERR_OK;
}

gboolean mp3_c_init (mp3tl *tl)
{
#ifdef USE_LIBOIL
  oil_init();
#endif

  init_hybrid (&tl->c_impl);  
  init_mdct ();

  return TRUE;
}

void mp3_c_flush (mp3tl *tl)
{
  init_hybrid (&tl->c_impl);  
  tl->c_impl.main_data_end = 0;
}
