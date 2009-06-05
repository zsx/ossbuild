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

#include <stdlib.h>

#include "common.h"
#include "decode.h"

/* Constant declarations */
static const gdouble multiple[64] = {
  2.00000000000000, 1.58740105196820, 1.25992104989487,
  1.00000000000000, 0.79370052598410, 0.62996052494744, 0.50000000000000,
  0.39685026299205, 0.31498026247372, 0.25000000000000, 0.19842513149602,
  0.15749013123686, 0.12500000000000, 0.09921256574801, 0.07874506561843,
  0.06250000000000, 0.04960628287401, 0.03937253280921, 0.03125000000000,
  0.02480314143700, 0.01968626640461, 0.01562500000000, 0.01240157071850,
  0.00984313320230, 0.00781250000000, 0.00620078535925, 0.00492156660115,
  0.00390625000000, 0.00310039267963, 0.00246078330058, 0.00195312500000,
  0.00155019633981, 0.00123039165029, 0.00097656250000, 0.00077509816991,
  0.00061519582514, 0.00048828125000, 0.00038754908495, 0.00030759791257,
  0.00024414062500, 0.00019377454248, 0.00015379895629, 0.00012207031250,
  0.00009688727124, 0.00007689947814, 0.00006103515625, 0.00004844363562,
  0.00003844973907, 0.00003051757813, 0.00002422181781, 0.00001922486954,
  0.00001525878906, 0.00001211090890, 0.00000961243477, 0.00000762939453,
  0.00000605545445, 0.00000480621738, 0.00000381469727, 0.00000302772723,
  0.00000240310869, 0.00000190734863, 0.00000151386361, 0.00000120155435,
  1E-20
};

/*************************************************************
 *
 * This module parses the starting 21 bits of the header
 *
 * **********************************************************/

gboolean
read_main_header (Bit_stream_struc *bs, fr_header *hdr)
{
  if (bs_bits_avail (bs) < 20) {
    return FALSE;
  }

  /* Read 2 bits as version, since we're doing the MPEG2.5 thing
   * of an 11 bit sync word and 2 bit version */
  hdr->version          = bs_getbits (bs, 2);
  hdr->layer            = 4 - bs_getbits (bs, 2);

  /* error_protection TRUE indicates there is a CRC */
  hdr->error_protection = !bs_get1bit (bs); 
  hdr->bitrate_idx      = bs_getbits (bs, 4);
  hdr->srate_idx        = bs_getbits (bs, 2);
  hdr->padding          = bs_get1bit (bs);
  hdr->extension        = bs_get1bit (bs);
  hdr->mode             = bs_getbits (bs, 2);
  hdr->mode_ext         = bs_getbits (bs, 2);

  hdr->copyright        = bs_get1bit (bs);
  hdr->original         = bs_get1bit (bs);
  hdr->emphasis         = bs_getbits (bs, 2);

  return TRUE;
}

/***************************************************************
 *
 * This module contains the core of the decoder ie all the
 * computational routines. (Layer I and II only)
 * Functions are common to both layer unless
 * otherwise specified.
 *
 ***************************************************************/

/************ Layer I, Layer II & Layer III ******************/
gboolean
read_header (mp3tl *tl, fr_header *hdr)
{
  Bit_stream_struc *bs = tl->bs;
 
  if (!read_main_header (bs, hdr))
    return FALSE;

  switch (hdr->layer) {
    case 1: 
      hdr->bits_per_slot = 32;
      hdr->frame_samples = 384;
      break;
    case 2: 
      hdr->bits_per_slot = 8;
      hdr->frame_samples = 1152;
      break;
    case 3: 
      hdr->bits_per_slot = 8;
      switch (hdr->version) {
        case MPEG_VERSION_1:
          hdr->frame_samples = 1152;
          break;
        case MPEG_VERSION_2:
        case MPEG_VERSION_2_5:
          hdr->frame_samples = 576;
          break;
        default:
          return FALSE;
      }
      break;
    default:
      /* Layer must be 1, 2 or 3 */
      return FALSE;
  }

  /* Sample rate index cannot be 0x03 (reserved value) */
  /* Bitrate index cannot be 0x0f (forbidden) */
  if (hdr->srate_idx == 0x03 || 
      hdr->bitrate_idx == 0x0f) {
    return FALSE;
  }

  hdr->channels = (hdr->mode == MPG_MD_MONO) ? 1 : 2;
  hdr->sample_rate = s_rates[hdr->version][hdr->srate_idx];
  hdr->bitrate = 0;
  /*Free format as bitrate index is 0 */
  if (hdr->bitrate_idx == 0 ){
    /*Calculate Only for the first free format frame since the stream 
     * is of constant bitrate */
    if (tl->free_first){
      Bit_stream_struc org_bs;
      GstClockTime next_ts;
      fr_header hdr1;
      guint16 N;
      /*copy the orignal bitsream structure */ 
      memcpy (&org_bs, bs, sizeof (Bit_stream_struc));
  
      /*Seek to next mp3 sync word and loop till there is data in the bitstream buffer*/
      while (bs_seek_sync (bs, &next_ts)){ 
        if (!read_main_header (bs, &hdr1))
          return FALSE;
  
        /*Calculates this frame sample rate */
        hdr1.sample_rate = s_rates[hdr1.version][hdr1.srate_idx];

        /*Checks if the original and forwarded frames layer and sample rate are same
         *if yes then calculate free format bitrate else seek to next frame*/
        if (hdr->layer == hdr1.layer && hdr->sample_rate == hdr1.sample_rate){
          /*Calculates distance between 2 valid frames*/
          N = bs->read.cur_used - org_bs.read.cur_used;
          /*Copies back the original bitsream to main bs structure*/
          memcpy (bs, &org_bs, sizeof (Bit_stream_struc));

          /*Free format bitrate in kbps that will be used for future reference*/
          tl->free_bitrate = (hdr->sample_rate * (N -  hdr->padding + 1) / 72 ) / 1000;
          hdr->bitrate = tl->free_bitrate * 1000;
          tl->free_first = FALSE;
          break; 
        }
      }
    } 
    else
      /*for all frames copy the same free format bitrate as the stream is cbr*/
      hdr->bitrate = tl->free_bitrate * 1000;
  }
  else if (hdr->version == MPEG_VERSION_1) 
    hdr->bitrate  = bitrates_v1[hdr->layer - 1][hdr->bitrate_idx] * 1000;
  else
    hdr->bitrate  = bitrates_v2[hdr->layer - 1][hdr->bitrate_idx] * 1000;
  
  if (hdr->sample_rate == 0 || hdr->bitrate == 0) {
    return FALSE;
  }

  /* Magic formula for calculating the size of a frame based on
   * the duration of the frame and the bitrate */
  hdr->frame_slots = (hdr->frame_samples / hdr->bits_per_slot)
                        * hdr->bitrate / hdr->sample_rate + hdr->padding;

  /* Number of bits we need for decode is frame_slots * slot_size */
  hdr->frame_bits = hdr->frame_slots * hdr->bits_per_slot;
  if (hdr->frame_bits <= 32) {
    return FALSE; /* Invalid header */
  }  

  return TRUE;
}

#define MPEG1_STEREO_SI_SLOTS 32
#define MPEG1_MONO_SI_SLOTS 17
#define MPEG2_LSF_STEREO_SI_SLOTS 17
#define MPEG2_LSF_MONO_SI_SLOTS 9

#define SAMPLE_RATES 3
#define BIT_RATES 15

const gint MPEG1_slot_table[SAMPLE_RATES][BIT_RATES] = {
    { 0,104,130,156,182,208,261,313,365,417,522,626,731,835,1044 },
    { 0,96,120,144,168,192,240,288,336,384,480,576,672,768,960 },
    { 0,144,180,216,252,288,360,432,504,576,720,864,1008,1152,1440 }
};

const gint MPEG2_LSF_slot_table[SAMPLE_RATES][BIT_RATES] = {
    { 0,26,52,78,104,130,156,182,208,261,313,365,417,470,522 },
{ 0,24,48,72,96,120,144,168,192,240,288,336,384,432,480 },
{ 0,36,72,108,144,180,216,252,288,360,432,504,576,648,720 }
};

/* For layer 3 only - the number of slots for main data of 
 * current frame. In Layer 3, 1 slot = 1 byte */
gboolean
set_hdr_data_slots (fr_header *hdr)
{
  int nSlots;

  if (hdr->layer != 3) {
    hdr->side_info_slots = 0;
    hdr->main_slots = 0;
    return TRUE;
  }

  nSlots = hdr->frame_slots - hdr->padding;

#if 0
  if (hdr->version == MPEG_VERSION_1) {
    g_print ("Calced %d main slots, table says %d\n", nSlots, 
       MPEG1_slot_table[hdr->srate_idx][hdr->bitrate_idx]);
  } else {
    g_print ("Calced %d main slots, table says %d\n", nSlots, 
       MPEG2_LSF_slot_table[hdr->srate_idx][hdr->bitrate_idx]);
  }
#endif

  if (hdr->version == MPEG_VERSION_1) {
    // nSlots = MPEG1_slot_table[hdr->srate_idx][hdr->bitrate_idx];

    if (hdr->channels == 1)
      hdr->side_info_slots = MPEG1_MONO_SI_SLOTS;
    else
      hdr->side_info_slots = MPEG1_STEREO_SI_SLOTS;
  } else {
    // nSlots = MPEG2_LSF_slot_table[hdr->srate_idx][hdr->bitrate_idx];

    if (hdr->channels == 1)
      hdr->side_info_slots = MPEG2_LSF_MONO_SI_SLOTS;
    else
      hdr->side_info_slots = MPEG2_LSF_STEREO_SI_SLOTS;
  }
  nSlots -= hdr->side_info_slots;

  if (hdr->padding)
    nSlots++;

  nSlots -= 4;
  if (hdr->error_protection)
    nSlots -= 2;

  if (nSlots < 0)
    return FALSE;

  hdr->main_slots = nSlots;

  return TRUE;
}

/*******************************************************************
 *
 * The bit allocation information is decoded. Layer I
 * has 4 bit per subband whereas Layer II is Ws and bit rate
 * dependent.
 *
 ********************************************************************/

/**************************** Layer II *************/
void
II_decode_bitalloc (Bit_stream_struc * bs, guint32 bit_alloc[2][SBLIMIT],
    frame_params * fr_ps)
{
  int sb, ch;
  int stereo = fr_ps->stereo;
  int sblimit = fr_ps->sblimit;
  int jsbound = fr_ps->jsbound;
  const al_table *alloc = fr_ps->alloc;

  for (sb = 0; sb < jsbound; sb++)
    for (ch = 0; ch < stereo; ch++) {
      bit_alloc[ch][sb] = (char) bs_getbits (bs, (*alloc)[sb][0].bits);
    }

  for (sb = jsbound; sb < sblimit; sb++) {
    /* expand to 2 channels */
    bit_alloc[0][sb] = bit_alloc[1][sb] = bs_getbits (bs, (*alloc)[sb][0].bits);
  }

  /* Zero the rest of the array */
  for (sb = sblimit; sb < SBLIMIT; sb++)
    for (ch = 0; ch < stereo; ch++)
      bit_alloc[ch][sb] = 0;
}

/**************************** Layer I *************/

void
I_decode_bitalloc (Bit_stream_struc * bs, guint32 bit_alloc[2][SBLIMIT],
    frame_params * fr_ps)
{
  int i, j;
  int stereo = fr_ps->stereo;

//  int sblimit = fr_ps->sblimit;
  int jsbound = fr_ps->jsbound;

  for (i = 0; i < jsbound; i++)
    for (j = 0; j < stereo; j++) {
      bit_alloc[j][i] = bs_getbits (bs, 4);
    }

  for (i = jsbound; i < SBLIMIT; i++) {
    guint32 b = bs_getbits (bs, 4);

    for (j = 0; j < stereo; j++)
      bit_alloc[j][i] = b;
  }
}

/*****************************************************************
 *
 * The following two functions implement the layer I and II
 * format of scale factor extraction. Layer I involves reading
 * 6 bit per subband as scale factor. Layer II requires reading
 * first the scfsi which in turn indicate the number of scale factors
 * transmitted.
 *    Layer I : I_decode_scale
 *   Layer II : II_decode_scale
 *
 ****************************************************************/

/************************** Layer I stuff ************************/
void
I_decode_scale (Bit_stream_struc * bs, guint32 bit_alloc[2][SBLIMIT],
    guint32 scale_index[2][3][SBLIMIT], frame_params * fr_ps)
{
  int i, j;
  int stereo = fr_ps->stereo;

//  int sblimit = fr_ps->sblimit;

  for (i = 0; i < SBLIMIT; i++)
    for (j = 0; j < stereo; j++) {
      if (!bit_alloc[j][i])
        scale_index[j][0][i] = SCALE_RANGE - 1;
      else {
        /* 6 bit per scale factor */
        scale_index[j][0][i] = bs_getbits (bs, 6);
      }
    }
}

/*************************** Layer II stuff ***************************/

void
II_decode_scale (Bit_stream_struc *bs, 
    guint scfsi[2][SBLIMIT], 
    guint bit_alloc[2][SBLIMIT], 
    guint scale_index[2][3][SBLIMIT], 
    frame_params *fr_ps)
{
  int sb, ch;
  int stereo = fr_ps->stereo;
  int sblimit = fr_ps->sblimit;

  for (sb = 0; sb < sblimit; sb++)
    for (ch = 0; ch < stereo; ch++)        /* 2 bit scfsi */
      if (bit_alloc[ch][sb]) {
        scfsi[ch][sb] = bs_getbits (bs, 2);
      }

  for (sb = sblimit; sb < SBLIMIT; sb++)
    for (ch = 0; ch < stereo; ch++)
      scfsi[ch][sb] = 0;

  for (sb = 0; sb < sblimit; sb++) {
    for (ch = 0; ch < stereo; ch++) {
      if (bit_alloc[ch][sb]) {
        switch (scfsi[ch][sb]) {
            /* all three scale factors transmitted */
          case 0:
            scale_index[ch][0][sb] = bs_getbits (bs, 6);
            scale_index[ch][1][sb] = bs_getbits (bs, 6);
            scale_index[ch][2][sb] = bs_getbits (bs, 6);
            break;
            /* scale factor 1 & 3 transmitted */
          case 1:
            scale_index[ch][0][sb] = 
		scale_index[ch][1][sb] = bs_getbits (bs, 6);
            scale_index[ch][2][sb] = bs_getbits (bs, 6);
            break;
            /* scale factor 1 & 2 transmitted */
          case 3:
            scale_index[ch][0][sb] = bs_getbits (bs, 6);
            scale_index[ch][1][sb] = 
		scale_index[ch][2][sb] = bs_getbits (bs, 6);
            break;
            /* only one scale factor transmitted */
          case 2:
            scale_index[ch][0][sb] =
                scale_index[ch][1][sb] =
                scale_index[ch][2][sb] = bs_getbits (bs, 6);
            break;
          default:
            break;
        }
      } else {
        scale_index[ch][0][sb] = 
	    scale_index[ch][1][sb] =
            scale_index[ch][2][sb] = SCALE_RANGE - 1;
      }
    }
  }
  for (sb = sblimit; sb < SBLIMIT; sb++) {
    for (ch = 0; ch < stereo; ch++) {
      scale_index[ch][0][sb] = 
	  scale_index[ch][1][sb] =
          scale_index[ch][2][sb] = SCALE_RANGE - 1;
    }
  }
}

/**************************************************************
 *
 *   The following two routines take care of reading the
 * compressed sample from the bit stream for both layer 1 and
 * layer 2. For layer 1, read the number of bits as indicated
 * by the bit_alloc information. For layer 2, if grouping is
 * indicated for a particular subband, then the sample size has
 * to be read from the bits_group and the merged samples has
 * to be decompose into the three distinct samples. Otherwise,
 * it is the same for as layer one.
 *
 **************************************************************/

/******************************* Layer I stuff ******************/

void
I_buffer_sample (
    Bit_stream_struc *bs,
    guint sample[2][3][SBLIMIT],
    guint bit_alloc[2][SBLIMIT],
    frame_params *fr_ps)
{
  int i, j, k;
  int stereo = fr_ps->stereo;

//  int sblimit = fr_ps->sblimit;
  int jsbound = fr_ps->jsbound;
  unsigned int s;

  for (i = 0; i < jsbound; i++) {
    for (j = 0; j < stereo; j++) {
      k = bit_alloc[j][i];
      if (k == 0)
        sample[j][0][i] = 0;
      else
        sample[j][0][i] = bs_getbits (bs, k + 1);
    }
  }
  for (i = jsbound; i < SBLIMIT; i++) {
    k = bit_alloc[0][i];
    if (k == 0)
      s = 0;
    else
      s = bs_getbits (bs, k + 1);

    for (j = 0; j < stereo; j++)
      sample[j][0][i] = s;
  }
}

/*************************** Layer II stuff ************************/

void
II_buffer_sample (Bit_stream_struc * bs, guint sample[2][3][SBLIMIT],
    guint bit_alloc[2][SBLIMIT], frame_params * fr_ps)
{
  int sb, ch, k;
  int stereo = fr_ps->stereo;
  int sblimit = fr_ps->sblimit;
  int jsbound = fr_ps->jsbound;
  const al_table *alloc = fr_ps->alloc;

  for (sb = 0; sb < sblimit; sb++) {
    for (ch = 0; ch < ((sb < jsbound) ? stereo : 1); ch++) {
      guint allocation = bit_alloc [ch][sb];
      if (allocation) {
        /* check for grouping in subband */
        if (alloc[0][sb][allocation].group == 3) {
          k = alloc[0][sb][allocation].bits;
          sample[ch][0][sb] = bs_getbits (bs, k);
          sample[ch][1][sb] = bs_getbits (bs, k);
          sample[ch][2][sb] = bs_getbits (bs, k);
        } else {                /* bit_alloc = 3, 5, 9 */
          unsigned int nlevels, c = 0;

          nlevels = alloc[0][sb][allocation].steps;
          k = alloc[0][sb][allocation].bits;
          c = bs_getbits (bs, k);
          for (k = 0; k < 3; k++) {
            sample[ch][k][sb] = c % nlevels;
            c /= nlevels;
          }
        }
      } else {                  /* for no sample transmitted */
        sample[ch][0][sb] = 0;
        sample[ch][1][sb] = 0;
        sample[ch][2][sb] = 0;
      }
      if (stereo == 2 && sb >= jsbound) { /* joint stereo : copy L to R */
          sample[1][0][sb] = sample[0][0][sb];
          sample[1][1][sb] = sample[0][1][sb];
          sample[1][2][sb] = sample[0][2][sb];
      }
    }
  }
  for (sb = sblimit; sb < SBLIMIT; sb++)
    for (ch = 0; ch < stereo; ch++) {
      sample[ch][0][sb] = 0;
      sample[ch][1][sb] = 0;
      sample[ch][2][sb] = 0;
    }
}

/**************************************************************
 *
 *   Restore the compressed sample to a factional number.
 *   first complement the MSB of the sample
 *    for layer I :
 *    Use s = (s' + 2^(-nb+1) ) * 2^nb / (2^nb-1)
 *   for Layer II :
 *   Use the formula s = s' * c + d
 *
 **************************************************************/
static const gfloat c[17] = 
{ 1.33333333333f, 1.60000000000f, 1.14285714286f,
  1.77777777777f, 1.06666666666f, 1.03225806452f,
  1.01587301587f, 1.00787401575f, 1.00392156863f,
  1.00195694716f, 1.00097751711f, 1.00048851979f,
  1.00024420024f, 1.00012208522f, 1.00006103888f,
  1.00003051851f, 1.00001525902f
};

static const gfloat d[17] = 
{ 0.500000000f, 0.500000000f, 0.250000000f, 0.500000000f,
  0.125000000f, 0.062500000f, 0.031250000f, 0.015625000f,
  0.007812500f, 0.003906250f, 0.001953125f, 0.0009765625f,
  0.00048828125f, 0.00024414063f, 0.00012207031f,
  0.00006103516f, 0.00003051758f
};

/************************** Layer II stuff ************************/

void
II_dequant_and_scale_sample (
     guint sample[2][3][SBLIMIT], guint bit_alloc[2][SBLIMIT],
     float fraction[2][3][SBLIMIT],
     guint scale_index[2][3][SBLIMIT], int scale_block,
     frame_params *fr_ps)
{
  int sb, gr, ch, x;
  int stereo = fr_ps->stereo;
  int sblimit = fr_ps->sblimit;
  const al_table *alloc = fr_ps->alloc;

  for (sb = 0; sb < sblimit; sb++) {
    for (ch = 0; ch < stereo; ch++) {
      guint allocation = bit_alloc[ch][sb];
      
      if (allocation != 0) {
        gfloat scale_val, val;
        gfloat c_quant, d_quant;

        c_quant = c[alloc[0][sb][allocation].quant];
        d_quant = d[alloc[0][sb][allocation].quant];
        scale_val = (gfloat) multiple[scale_index[ch][scale_block][sb]];

        for (gr = 0; gr < 3; gr++) {
          /* locate MSB in the sample */
          x = 0;
          while ((1UL << x) < (*alloc)[sb][allocation].steps)
            x++;

          /* MSB inversion */
          if (((sample[ch][gr][sb] >> (x - 1)) & 1) == 1)
            val = 0.0;
          else
            val = -1.0;

          /* Form a 2's complement sample */
          val += (gfloat) ((double) (sample[ch][gr][sb] & ((1 << (x - 1)) - 1)) 
	      / (double) (1L << (x - 1)));

          /* Dequantize the sample */
          val += d_quant;
          val *= c_quant;

	  /* And scale */
	  val *= scale_val;
          fraction[ch][gr][sb] = val;
	}
      } else {
        fraction[ch][0][sb] = 0.0;
        fraction[ch][1][sb] = 0.0;
        fraction[ch][2][sb] = 0.0;
      }
    }
  }

  for (sb = sblimit; sb < SBLIMIT; sb++)
    for (ch = 0; ch < stereo; ch++) {
        fraction[ch][0][sb] = 0.0;
        fraction[ch][1][sb] = 0.0;
        fraction[ch][2][sb] = 0.0;
    }
}

/***************************** Layer I stuff ***********************/

void I_dequant_and_scale_sample (
    guint sample[2][3][SBLIMIT],
    float fraction[2][3][SBLIMIT],
    guint bit_alloc[2][SBLIMIT],
    guint scale_index[2][3][SBLIMIT],
    frame_params *fr_ps)
{
  int sb, ch;
  guint nb;
  int stereo = fr_ps->stereo;

  for (sb = 0; sb < SBLIMIT; sb++) {
    for (ch = 0; ch < stereo; ch++) {
      guint allocation = bit_alloc[ch][sb];

      if (allocation != 0) {
        gdouble val;

        nb = allocation + 1;

        if (((sample[ch][0][sb] >> allocation) & 1) != 0)
          val = 0.0;
        else
          val = -1.0;

        val += (double) (sample[ch][0][sb] & ((1 << allocation) - 1)) / 
	       (double) (1L << (nb - 1));

        val =
            (double) (val + 1.0 / (1L << allocation)) *
            (double) (1L << nb) / ((1L << nb) - 1);

        val *= multiple[scale_index[ch][0][sb]];

        fraction[ch][0][sb] = (gfloat) val;
      } else
        fraction[ch][0][sb] = 0.0;
    }
  }
}

/*****************************************************************
 *
 * The following are the subband synthesis routines. They apply
 * to both layer I and layer II stereo or mono. The user has to
 * decide what parameters are to be passed to the routines.
 *
 ***************************************************************/

/*************************************************************
 *
 *   Pass the subband sample through the synthesis window
 *
 **************************************************************/

/* create in synthesis filter */
void
init_syn_filter (frame_params *fr_ps)
{
  register int i, k;
  gfloat (*filter)[32];

  filter = fr_ps->filter;

  for (i = 0; i < 64; i++)
    for (k = 0; k < 32; k++) {
      if ((filter[i][k] =
              1e9f * cosf (((PI64 * i + PI4) * (2 * k + 1)))) >= 0)
        modff (filter[i][k] + 0.5, &filter[i][k]);
      else
        modff (filter[i][k] - 0.5, &filter[i][k]);
      filter[i][k] *= 1e-9f;
    }

  for (i = 0; i < 2; i++)
    fr_ps->bufOffset[i] = 64;
}

/***************************************************************
 *
 *   Window the restored sample
 *
 ***************************************************************/

/* Write output samples into the outBuf, incrementing psamples for each 
 * sample, wrapping at bufSize */
void
out_fifo (short pcm_sample[2][SSLIMIT][SBLIMIT], int num, 
  frame_params *fr_ps, gint16 *outBuf, guint32 *psamples, guint32 bufSize)
{
  int i, j, l;
  int stereo = fr_ps->stereo;

  for (i = 0; i < num; i++) {
    for (j = 0; j < SBLIMIT; j++) {
      for (l = 0; l < stereo; l++) {
        outBuf[*psamples] = pcm_sample[l][i][j];
        (*psamples) ++;
        (*psamples) %= bufSize;
      }
    }
  }
}

void
buffer_CRC (Bit_stream_struc * bs, guint * old_crc)
{
  *old_crc = bs_getbits (bs, 16);
}

void
recover_CRC_error (short pcm_sample[2][SSLIMIT][SBLIMIT], int error_count, 
    frame_params *fr_ps, gint16 *outBuf, guint32 *psamples, guint32 bufSize)
{
  int stereo = fr_ps->stereo;
  int num, i;
  int samplesPerFrame, samplesPerSlot;
  fr_header *hdr = &fr_ps->header;

  num = 3;
  if (hdr->layer == 1)
    num = 1;

  samplesPerSlot = SBLIMIT * num * stereo;
  samplesPerFrame = samplesPerSlot * 32;

  if (error_count == 1) {       /* replicate previous error_free frame */
    /* flush out fifo */
    out_fifo (pcm_sample, num, fr_ps, outBuf, psamples, bufSize);
    /* go back to the beginning of the previous frame */
    #if 0
    offset = sizeof (short int) * samplesPerFrame;
    fseek (outFile, -offset, SEEK_CUR);
    done = 0;
    for (i = 0; i < SCALE_BLOCK; i++) {
      fread (pcm_sample, 2, samplesPerSlot, outFile);
      out_fifo (pcm_sample, num, fr_ps, done, outFile, psampFrames);
    }
    #endif
    g_assert ("FIXME - implement previous frame replication\n");

  } else { 
    short *temp;

    /* mute the frame */
    temp = (short *) pcm_sample;
    for (i = 0; i < 2 * 3 * SBLIMIT; i++)
      *temp++ = MUTE;           /* MUTE value is in decoder.h */
    for (i = 0; i < SCALE_BLOCK; i++)
      out_fifo (pcm_sample, num, fr_ps, outBuf, psamples, bufSize);
  }
}
