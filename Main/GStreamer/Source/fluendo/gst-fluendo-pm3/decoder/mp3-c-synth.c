/*
 * Copyright <2005, 2006, 2007, 2008> Fluendo S.A.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>
#include "mp3tl-priv.h"
#include "table-dewindow.h"
#include "mp3-cos-tables.h"

#ifdef USE_LIBOIL
#include <liboil/liboil.h>
#endif

#define OPT_SYNTH

void III_subband_synthesis (mp3tl *tl, frame_params *fr_ps, 
   gfloat hybridOut[SBLIMIT][SSLIMIT], gint channel, 
   short samples[SSLIMIT][SBLIMIT])
{
  gint ss, sb;
  gfloat polyPhaseIn[SBLIMIT];  /* PolyPhase Input. */

  for (ss = 0; ss < 18; ss++) {
    /* Each of the 32 subbands has 18 samples. On each iteration, we take
      * one sample from each subband, (32 samples), and use a 32 point DCT 
      * to perform matrixing, and copy the result into the synthesis 
      * buffer fifo. */
    for (sb = 0; sb < SBLIMIT; sb++) {
      polyPhaseIn[sb] = hybridOut[sb][ss];
    } 
 
    mp3_SubBandSynthesis (tl, fr_ps, polyPhaseIn, channel, 
        &(tl->pcm_sample[channel][ss][0]));
  }
}

#ifndef USE_IPP

#ifdef OPT_SYNTH
static void III_polyphase_matrix (gfloat in[SBLIMIT], gfloat out[2 * SBLIMIT]);
static void MPG_DCT_32 (gfloat in[32], gfloat out[32]);
static void MPG_DCT_16 (gfloat in[16], gfloat out[16]);
static void MPG_DCT_8 (gfloat in[8], gfloat out[8]);

void
mp3_SubBandSynthesis (mp3tl *tl ATTR_UNUSED, frame_params *fr_ps,
    float *polyPhaseIn, gint channel, short *samples)
{
  gint i, j;
  gint buf_offset;
  gfloat u_vec[HAN_SIZE];
  gfloat *cur_synbuf;
  gfloat *u_vec0; 

  buf_offset = fr_ps->bufOffset[channel];
  cur_synbuf = fr_ps->synbuf[channel];

  /* Shift down 64 samples in the fifo, which should always leave room */
  /* The maximum size of the synth buf is ((2 * HAN_SIZE) - 1) = 1023 */
  buf_offset = (buf_offset - 64) & 1023;
  /* g_assert (buf_offset <= (2 * HAN_SIZE) - 64); */

  III_polyphase_matrix (polyPhaseIn, cur_synbuf + buf_offset); 
  
  /* Build the U vector and perform dewindowing */
#if 1
  for (i = 0; i < HAN_SIZE; i += 64) {
    gint k = i * 2;
    for (j = 0; j < SBLIMIT; j++) {
      u_vec[i + j] = cur_synbuf[(buf_offset + k + j) & 1023];
      u_vec[i + j + 32] = cur_synbuf[(buf_offset + k + j + 96) & 1023];
    }
  }
#else
  {
  gint cur_offset;
  for (i = 0; i < (1024 - buf_offset - 96 - 32) / 2; i += 64) {
    cur_offset = buf_offset + i + i;
    u_vec0 = u_vec + i;
    for (j = 0; j < SBLIMIT; j++) {
      u_vec0[j] = cur_synbuf[cur_offset + j];
      u_vec0[j + 32] = cur_synbuf[cur_offset + j + 96];
    }
  }
  for (; i < (1024 - buf_offset - 32) / 2; i += 64) {
    cur_offset = buf_offset + i + i;
    u_vec0 = u_vec + i;
    for (j = 0; j < SBLIMIT; j++) {
      u_vec0[j] = cur_synbuf[(cur_offset + j) & 1023];
      u_vec0[j + 32] = cur_synbuf[(cur_offset + j + 96) & 1023];
    }
  }
  for (; i < HAN_SIZE; i += 64) {
    cur_offset = buf_offset + i + i - 1024;
    u_vec0 = u_vec + i;
    for (j = 0; j < SBLIMIT; j++) {
      u_vec0[j] = cur_synbuf[cur_offset + j];
      u_vec0[j + 32] = cur_synbuf[cur_offset + j + 96];
    }
  }
  }
#endif

  /* dewindowing */
#ifdef USE_LIBOIL
  oil_multiply_f32 (u_vec, u_vec, dewindow, HAN_SIZE);
#else
  for (i = 0; i < HAN_SIZE; i++)
    u_vec[i] *= dewindow[i];
#endif

  /* Now calculate and output 32 samples */
  for (i = 0; i < 32; i++) {
    gfloat sum = 0.0;
#if 1
    u_vec0 = u_vec + i;
    for (j = 0; j < HAN_SIZE; j += 128) {
      sum += u_vec0[j]; 
      sum += u_vec0[j + 32]; 
      sum += u_vec0[j + 64]; 
      sum += u_vec0[j + 96]; 
    }
#else
    for (j = 0; j < HAN_SIZE; j += 32)
      sum += u_vec[j + i]; 
#endif

    if (sum > 0) {
      sum = sum * SCALE + 0.5f;
      if (sum < (SCALE-1)) {
        samples[i] = (short) (sum);
      } else {
        samples[i] = (short) (SCALE - 1);
      }
    } else {
      sum = sum * SCALE - 0.5f;
      if (sum > -SCALE) {
        samples[i] = (short) (sum);
      } else {
        samples[i] = (short) (-SCALE);
      }
    }
  }

  fr_ps->bufOffset[channel] = buf_offset;
}

/* Synthesis matrixing variant which uses a 32 point DCT to compute 
 * the matrixing */
static void
III_polyphase_matrix (gfloat in[SBLIMIT], gfloat out[2 * SBLIMIT])
{
  int i;
  gfloat tmp[32];

#if 0
  g_print ("Matrixing, input: ");
  for (i = 0; i < 32; i++)
    g_print ("%f ", in[i]);
  g_print ("\n");
#endif

  /* DCT part */
  MPG_DCT_32 (in, tmp);

  /* Re-arrange results into the output.
   * FIXME: We can avoid this copying by using the results
   * more intelligently later in the processing and computing only
   * as needed. */
  for (i = 0; i < 16; i++) {
    out[i] = tmp[i+16];
  }
  out[16] = 0.0;

  for (i = 17; i < 48; i++) {
    out[i] = -tmp[48-i];
  }
  for (i = 48; i < 64; i++) {
    out[i] = -tmp[i-48];
  }

#if 0
  g_print ("output: ");
  for (i = 0; i < 64; i++)
    g_print ("%f ", out[i]);
  g_print ("\n"); 
#endif
}

#define INV_SQRT_2 (7.071067811865474617150084668537e-01)

static void
MPG_DCT_32 (gfloat in[32], gfloat out[32])
{
  int i;
  gfloat even_in[16], even_out[16], odd_in[16], odd_out[16];
  
  for (i = 0; i < 16; i++) {
    even_in[i] = in[i] + in[31-i];
    odd_in[i] = (in[i] - in[31-i]) * synth_cos64_table[2*i];
  }

  MPG_DCT_16 (even_in, even_out); 
  MPG_DCT_16 (odd_in, odd_out);

  for (i = 0; i < 15; i++) {
    out[2*i] = even_out[i];     
    out[2*i+1] = odd_out[i] + odd_out[i+1];
  }
  out[30] = even_out[15];
  out[31] = odd_out[15];  
}

static void
MPG_DCT_16 (gfloat in[16], gfloat out[16])
{
  int i;
  gfloat even_in[8], even_out[8], odd_in[8], odd_out[8];
  
  for (i = 0; i < 8; i++) {
    even_in[i] = in[i] + in[15-i];
    odd_in[i] = (in[i] - in[15-i]) * synth_cos64_table[4*i+1];
#if 0
    g_print ("i = %d, index %d calc %.06f table %.06f\n", i, 4*i+1, 
       (1.0 / (2.0 * cos ((2*i+1) * (M_PI / (32))))), synth_cos64_table[4*i+1]);
#endif
  }

  MPG_DCT_8 (even_in, even_out);
  MPG_DCT_8 (odd_in, odd_out);

  for (i = 0; i < 8; i++) {
    out[2*i] = even_out[i];     
  }

  for (i = 0; i < 7; i++) {
    out[2*i+1] = odd_out[i] + odd_out[i+1];
  }
  out[15] = odd_out[7];
}

static void
MPG_DCT_8 (gfloat in[8], gfloat out[8])
{
  gfloat even_in[4];
  gfloat odd_in[4], odd_out[4];
  gfloat tmp[6];

  /* Even indices */
  even_in[0] = in[0] + in[7];
  even_in[1] = in[3] + in[4];
  even_in[2] = in[1] + in[6];
  even_in[3] = in[2] + in[5];

  tmp[0] = even_in[0] + even_in[1];
  tmp[1] = even_in[2] + even_in[3];
  tmp[2] = (even_in[0] - even_in[1]) * synth_cos64_table[7];
  tmp[3] = (even_in[2] - even_in[3]) * synth_cos64_table[23];
  tmp[4] = (gfloat) ((tmp[2] - tmp[3]) * INV_SQRT_2);

  out[0] = tmp[0] + tmp[1];
  out[2] = tmp[2] + tmp[3] + tmp[4];
  out[4] = (gfloat) ((tmp [0] - tmp[1]) * INV_SQRT_2);
  out[6] = tmp[4];

  /* Odd indices */
  odd_in[0] = (in[0] - in[7]) * synth_cos64_table [3];
  odd_in[1] = (in[1] - in[6]) * synth_cos64_table [11];
  odd_in[2] = (in[2] - in[5]) * synth_cos64_table [19];
  odd_in[3] = (in[3] - in[4]) * synth_cos64_table [27];

  tmp[0] = odd_in[0] + odd_in[3];
  tmp[1] = odd_in[1] + odd_in[2];
  tmp[2] = (odd_in[0] - odd_in[3]) * synth_cos64_table[7];
  tmp[3] = (odd_in[1] - odd_in[2]) * synth_cos64_table[23];
  tmp[4] = tmp[2] + tmp[3];
  tmp[5] = (gfloat) ((tmp[2] - tmp[3]) * INV_SQRT_2);

  odd_out[0] = tmp[0] + tmp[1];
  odd_out[1] = tmp[4] + tmp[5];
  odd_out[2] = (gfloat) ((tmp[0] - tmp[1]) * INV_SQRT_2);
  odd_out[3] = tmp[5];

  out[1] = odd_out[0] + odd_out[1];
  out[3] = odd_out[1] + odd_out[2];
  out[5] = odd_out[2] + odd_out[3];
  out[7] = odd_out[3];
}

#else
/* Original sub band synthesis function, for reference */
void
mp3_SubBandSynthesis (mp3tl *tl ATTR_UNUSED, frame_params *fr_ps,
    float *bandPtr, gint channel, short *samples)
{
  int i, j, k;
  float *bufOffsetPtr, sum;
  gint    *bufOffset;
  float *synbuf = fr_ps->synbuf[channel];

  bufOffset = fr_ps->bufOffset;

  bufOffset[channel] = (bufOffset[channel] - 64) & 0x3ff;
  /* g_print ("bufOffset[%d] = %d\n", channel, bufOffset[channel]); */

  bufOffsetPtr = synbuf + bufOffset[channel];

  for (i = 0; i < 64; i++) {
    float* f_row = fr_ps->filter[i];
    sum = 0;
    for (k = 0; k < 32; k++)
      sum += bandPtr[k] * f_row[k];
    bufOffsetPtr[i] = sum;
  }

  /*  S(i,j) = D(j+32i) * U(j+32i+((i+1)>>1)*64)  */
  /*  samples(i,j) = MWindow(j+32i) * bufPtr(j+32i+((i+1)>>1)*64)  */
  for (j = 0; j < 32; j++) {
    sum = 0;
    sum += dewindow[j] * synbuf[(j + bufOffset[channel]) & 0x3ff];
    for (i = 64; i < 512; i += 64) {
      k = j + i;
      sum += dewindow[k] * synbuf[(k + i + bufOffset[channel]) & 0x3ff];
      sum += dewindow[k - 32] * synbuf[(k - 32 + i + bufOffset[channel]) & 0x3ff];
    }

    /* Casting truncates towards zero for both positive and negative numbers,
       the result is cross-over distortion,  1995-07-12 shn */
    if (sum > 0) {
      sum = sum * SCALE + 0.5;
      if (sum < (SCALE-1)) {
        samples[j] = (short) (sum);
      } else {
        samples[j] = (short) (SCALE - 1);
      }
    } else {
      sum = sum * SCALE - 0.5;
      if (sum > -SCALE) {
        samples[j] = (short) (sum);
      } else {
        samples[j] = (short) (-SCALE);
      }
    }
  }
}

#endif
#endif
