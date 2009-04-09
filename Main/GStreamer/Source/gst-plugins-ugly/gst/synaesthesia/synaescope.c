/* synaescope.cpp
 * Copyright (C) 1999,2002 Richard Boulton <richard@tartarus.org>
 *
 * Much code copied from Synaesthesia - a program to display sound
 * graphically, by Paul Francis Harrison <pfh@yoyo.cc.monash.edu.au>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "synaescope.h"

#include <pthread.h>
#include <dirent.h>
#include <sys/stat.h>
#ifndef _MSC_VER
  #include <sys/time.h>
  #include <time.h>
#endif
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#ifndef _MSC_VER
  #include <unistd.h>
#endif
#include <string.h>
#include <assert.h>

#ifdef G_OS_WIN32
#ifndef M_PI
#define M_PI  3.14159265358979323846
#endif
#endif

#define SCOPE_BG_RED 0
#define SCOPE_BG_GREEN 0
#define SCOPE_BG_BLUE 0

#define FFT_BUFFER_SIZE_LOG 9
#define FFT_BUFFER_SIZE (1 << FFT_BUFFER_SIZE_LOG)

#define syn_width 320
#define syn_height 200
#define brightMin 200
#define brightMax 2000
#define brightDec 10
#define brightInc 6
#define brTotTargetLow 5000
#define brTotTargetHigh 15000

#define BOUND(x) ((x) > 255 ? 255 : (x))
#define PEAKIFY(x) BOUND((x) - (x)*(255-(x))/255/2)

/* Instance data */
struct syn_instance
{
  int autobrightness;           /* Whether to use automatic brightness adjust */
  unsigned int brightFactor;
  unsigned char output[syn_width * syn_height * 2];
  guint32 display[syn_width * syn_height];
  gint16 pcmt_l[FFT_BUFFER_SIZE];
  gint16 pcmt_r[FFT_BUFFER_SIZE];
  gint16 pcm_l[FFT_BUFFER_SIZE];
  gint16 pcm_r[FFT_BUFFER_SIZE];
  double fftout_l[FFT_BUFFER_SIZE];
  double fftout_r[FFT_BUFFER_SIZE];
  double corr_l[FFT_BUFFER_SIZE];
  double corr_r[FFT_BUFFER_SIZE];
  int clarity[FFT_BUFFER_SIZE]; /* Surround sound */
};

/* Shared lookup tables */
static double fftmult[FFT_BUFFER_SIZE / 2 + 1];
static double cosTable[FFT_BUFFER_SIZE];
static double negSinTable[FFT_BUFFER_SIZE];
static int bitReverse[FFT_BUFFER_SIZE];
static int scaleDown[256];
static guint32 colEq[256];

static void synaes_fft (double *x, double *y);
static void synaescope_coreGo (syn_instance * si);

static inline void
addPixel (unsigned char *output, int x, int y, int br1, int br2)
{
  unsigned char *p;

  if (x < 0 || x >= syn_width || y < 0 || y >= syn_height)
    return;

  p = output + x * 2 + y * syn_width * 2;
  if (p[0] < 255 - br1)
    p[0] += br1;
  else
    p[0] = 255;
  if (p[1] < 255 - br2)
    p[1] += br2;
  else
    p[1] = 255;
}

static inline void
addPixelFast (unsigned char *p, int br1, int br2)
{
  if (p[0] < 255 - br1)
    p[0] += br1;
  else
    p[0] = 255;
  if (p[1] < 255 - br2)
    p[1] += br2;
  else
    p[1] = 255;
}

static void
synaescope_coreGo (syn_instance * si)
{
  int i, j;
  register guint32 *ptr;
  register guint32 *end;
  int heightFactor;
  int actualHeight;
  int heightAdd;
  double brightFactor2;
  long int brtot;

  memcpy (si->pcm_l, si->pcmt_l, sizeof (si->pcm_l));
  memcpy (si->pcm_r, si->pcmt_r, sizeof (si->pcm_r));

  for (i = 0; i < FFT_BUFFER_SIZE; i++) {
    si->fftout_l[i] = si->pcm_l[i];
    si->fftout_r[i] = si->pcm_r[i];
  }

  synaes_fft (si->fftout_l, si->fftout_r);

  for (i = 0 + 1; i < FFT_BUFFER_SIZE; i++) {
    double x1 = si->fftout_l[bitReverse[i]];
    double y1 = si->fftout_r[bitReverse[i]];
    double x2 = si->fftout_l[bitReverse[FFT_BUFFER_SIZE - i]];
    double y2 = si->fftout_r[bitReverse[FFT_BUFFER_SIZE - i]];
    double aa, bb;

    si->corr_l[i] = sqrt (aa = (x1 + x2) * (x1 + x2) + (y1 - y2) * (y1 - y2));
    si->corr_r[i] = sqrt (bb = (x1 - x2) * (x1 - x2) + (y1 + y2) * (y1 + y2));
    si->clarity[i] = (int) (
        ((x1 + x2) * (x1 - x2) + (y1 + y2) * (y1 - y2)) / (aa + bb) * 256);
  }

  /* Asger Alstrupt's optimized 32 bit fade */
  /* (alstrup@diku.dk) */
  ptr = (guint32 *) si->output;
  end = (guint32 *) (si->output + syn_width * syn_height * 2);
  do {
    /*Bytewize version was: *(ptr++) -= *ptr+(*ptr>>1)>>4; */
    if (*ptr) {
      if (*ptr & 0xf0f0f0f0) {
        *ptr = *ptr - ((*ptr & 0xf0f0f0f0) >> 4) - ((*ptr & 0xe0e0e0e0) >> 5);
      } else {
        *ptr = (*ptr * 14 >> 4) & 0x0f0f0f0f;
        /*Should be 29/32 to be consistent. Who cares. This is totally */
        /* hacked anyway.  */
        /*unsigned char *subptr = (unsigned char*)(ptr++); */
        /*subptr[0] = (int)subptr[0] * 29 / 32; */
        /*subptr[1] = (int)subptr[0] * 29 / 32; */
        /*subptr[2] = (int)subptr[0] * 29 / 32; */
        /*subptr[3] = (int)subptr[0] * 29 / 32; */
      }
    }
    ptr++;
  } while (ptr < end);

  heightFactor = FFT_BUFFER_SIZE / 2 / syn_height + 1;
  actualHeight = FFT_BUFFER_SIZE / 2 / heightFactor;
  heightAdd = (syn_height + actualHeight) >> 1;

  /* Correct for window size */
  brightFactor2 = (si->brightFactor / 65536.0 / FFT_BUFFER_SIZE) *
      sqrt (actualHeight * syn_width / (320.0 * 200.0));

  brtot = 0;
  for (i = 1; i < FFT_BUFFER_SIZE / 2; i++) {
    /*int h = (int)( corr_r[i]*280 / (corr_l[i]+corr_r[i]+0.0001)+20 ); */
    if (si->corr_l[i] > 0 || si->corr_r[i] > 0) {
      int h =
          (int) (si->corr_r[i] * syn_width / (si->corr_l[i] + si->corr_r[i]));

/*      int h = (int)( syn_width - 1 ); */
      int br1, br2, br =
          (int) ((si->corr_l[i] + si->corr_r[i]) * i * brightFactor2);
      int px = h, py = heightAdd - i / heightFactor;

      brtot += br;
      br1 = br * (si->clarity[i] + 128) >> 8;
      br2 = br * (128 - si->clarity[i]) >> 8;
      if (br1 < 0)
        br1 = 0;
      else if (br1 > 255)
        br1 = 255;
      if (br2 < 0)
        br2 = 0;
      else if (br2 > 255)
        br2 = 255;
      /*unsigned char *p = output+ h*2+(164-((i<<8)>>FFT_BUFFER_SIZE_LOG))*(syn_width*2);  */

      if (px < 30 || py < 30 || px > syn_width - 30 || py > syn_height - 30) {
        addPixel (si->output, px, py, br1, br2);
        for (j = 1; br1 > 0 || br2 > 0;
            j++, br1 = scaleDown[br1], br2 = scaleDown[br2]) {
          addPixel (si->output, px + j, py, br1, br2);
          addPixel (si->output, px, py + j, br1, br2);
          addPixel (si->output, px - j, py, br1, br2);
          addPixel (si->output, px, py - j, br1, br2);
        }
      } else {
        unsigned char *p = si->output + px * 2 + py * syn_width * 2, *p1 =
            p, *p2 = p, *p3 = p, *p4 = p;
        addPixelFast (p, br1, br2);
        for (; br1 > 0 || br2 > 0; br1 = scaleDown[br1], br2 = scaleDown[br2]) {
          p1 += 2;
          addPixelFast (p1, br1, br2);
          p2 -= 2;
          addPixelFast (p2, br1, br2);
          p3 += syn_width * 2;
          addPixelFast (p3, br1, br2);
          p4 -= syn_width * 2;
          addPixelFast (p4, br1, br2);
        }
      }
    }
  }

  /* Apply autoscaling: makes quiet bits brighter, and loud bits
   * darker, but still keeps loud bits brighter than quiet bits. */
  if (brtot != 0 && si->autobrightness) {
    long int brTotTarget = brTotTargetHigh;

    if (brightMax != brightMin) {
      brTotTarget -= ((brTotTargetHigh - brTotTargetLow) *
          (si->brightFactor - brightMin)) / (brightMax - brightMin);
    }
    if (brtot < brTotTarget) {
      si->brightFactor += brightInc;
      if (si->brightFactor > brightMax)
        si->brightFactor = brightMax;
    } else {
      si->brightFactor -= brightDec;
      if (si->brightFactor < brightMin)
        si->brightFactor = brightMin;
    }
    /* printf("brtot: %ld\tbrightFactor: %d\tbrTotTarget: %d\n",
       brtot, brightFactor, brTotTarget); */
  }
}


static void
synaescope32 (syn_instance * si)
{
  unsigned char *outptr;
  int i;

  synaescope_coreGo (si);

  outptr = si->output;
  for (i = 0; i < syn_width * syn_height; i++) {
    si->display[i] = colEq[(outptr[0] >> 4) + (outptr[1] & 0xf0)];
    outptr += 2;
  }
}


static int
bitReverser (int i)
{
  int sum = 0;
  int j;

  for (j = 0; j < FFT_BUFFER_SIZE_LOG; j++) {
    sum = (i & 1) + sum * 2;
    i >>= 1;
  }

  return sum;
}

static void
synaes_fft (double *x, double *y)
{
  int n2 = FFT_BUFFER_SIZE;
  int n1;
  int twoToTheK;
  int j;

  for (twoToTheK = 1; twoToTheK < FFT_BUFFER_SIZE; twoToTheK *= 2) {
    n1 = n2;
    n2 /= 2;
    for (j = 0; j < n2; j++) {
      double c = cosTable[j * twoToTheK & (FFT_BUFFER_SIZE - 1)];
      double s = negSinTable[j * twoToTheK & (FFT_BUFFER_SIZE - 1)];
      int i;

      for (i = j; i < FFT_BUFFER_SIZE; i += n1) {
        int l = i + n2;
        double xt = x[i] - x[l];
        double yt = y[i] - y[l];

        x[i] = (x[i] + x[l]);
        y[i] = (y[i] + y[l]);
        x[l] = xt * c - yt * s;
        y[l] = xt * s + yt * c;
      }
    }
  }
}

static void
synaescope_set_data (syn_instance * si, gint16 data[2][512])
{
  int i;
  gint16 *newset_l = si->pcmt_l;
  gint16 *newset_r = si->pcmt_r;

  for (i = 0; i < FFT_BUFFER_SIZE; i++) {
    newset_l[i] = data[0][i];
    newset_r[i] = data[1][i];
  }
}


guint32 *
synaesthesia_update (syn_instance * si, gint16 data[2][512])
{
  synaescope_set_data (si, data);
  synaescope32 (si);
  return si->display;
}

void
synaesthesia_init ()
{
  static int inited = 0;
  int i;

  if (inited)
    return;

  for (i = 0; i <= FFT_BUFFER_SIZE / 2 + 1; i++) {
    double mult = (double) 128 / ((FFT_BUFFER_SIZE * 16384) ^ 2);

    /* Result now guaranteed (well, almost) to be in range 0..128 */

    /* Low values represent more frequencies, and thus get more */
    /* intensity - this helps correct for that. */
    mult *= log (i + 1) / log (2);

    mult *= 3;                  /* Adhoc parameter, looks about right for me. */

    fftmult[i] = mult;
  }

  for (i = 0; i < FFT_BUFFER_SIZE; i++) {
    negSinTable[i] = -sin (M_PI * 2 / FFT_BUFFER_SIZE * i);
    cosTable[i] = cos (M_PI * 2 / FFT_BUFFER_SIZE * i);
    bitReverse[i] = bitReverser (i);
  }

  for (i = 0; i < 256; i++)
    scaleDown[i] = i * 200 >> 8;

  for (i = 0; i < 256; i++) {
    int red = PEAKIFY ((i & 15 * 16));
    int green = PEAKIFY ((i & 15) * 16 + (i & 15 * 16) / 4);
    int blue = PEAKIFY ((i & 15) * 16);

    colEq[i] = (red << 16) + (green << 8) + blue;
  }

  inited = 1;
}

syn_instance *
synaesthesia_new (guint32 resx, guint32 resy)
{
  syn_instance *si;

  si = g_try_new0 (syn_instance, 1);
  if (si == NULL)
    return NULL;

  si->autobrightness = 1;       /* Whether to use automatic brightness adjust */
  si->brightFactor = 400;

  /* FIXME: Make the width and height configurable? */

  return si;
}

void
synaesthesia_close (syn_instance * si)
{
  g_return_if_fail (si != NULL);

  g_free (si);
}
