/* GStreamer
 * Copyright (C) 2005 Wim Taymans <wim at fluendo dot com>
 *
 * audioconvert.c: Convert audio to different audio formats automatically
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <math.h>
#include <string.h>

#include "gstchannelmix.h"
#include "gstaudioquantize.h"
#include "audioconvert.h"
#include "gst/floatcast/floatcast.h"

/* sign bit in the intermediate format */
#define SIGNED  (1U<<31)

/*** 
 * unpack code
 */
#define MAKE_UNPACK_FUNC_NAME(name)                                     \
audio_convert_unpack_##name

/* unpack from integer to signed integer 32 */
#define MAKE_UNPACK_FUNC_II(name, stride, sign, READ_FUNC)              \
static void                                                             \
MAKE_UNPACK_FUNC_NAME (name) (guint8 *src, gint32 *dst,                 \
        gint scale, gint count)                                         \
{                                                                       \
  for (;count; count--) {                                               \
    *dst++ = (((gint32) READ_FUNC (src)) << scale) ^ (sign);            \
    src+=stride;                                                        \
  }                                                                     \
}

/* unpack from float to signed integer 32 */
#define MAKE_UNPACK_FUNC_FI(name, type, READ_FUNC)                            \
static void                                                                   \
MAKE_UNPACK_FUNC_NAME (name) (type * src, gint32 * dst, gint s, gint count)   \
{                                                                             \
  gdouble temp;                                                               \
                                                                              \
  for (; count; count--) {                                                    \
    /* blow up to 32 bit */                                                   \
    temp = floor ((READ_FUNC (*src++) * 2147483647.0) + 0.5);                 \
    *dst++ = (gint32) CLAMP (temp, G_MININT32, G_MAXINT32);                   \
  }                                                                           \
}

/* unpack from float to float 64 (double) */
#define MAKE_UNPACK_FUNC_FF(name, type, FUNC)                                 \
static void                                                                   \
MAKE_UNPACK_FUNC_NAME (name) (type * src, gdouble * dst, gint s,              \
    gint count)                                                               \
{                                                                             \
  for (; count; count--)                                                      \
    *dst++ = (gdouble) FUNC (*src++);                                         \
}

/* unpack from int to float 64 (double) */
#define MAKE_UNPACK_FUNC_IF(name, stride, sign, READ_FUNC)                    \
static void                                                                   \
MAKE_UNPACK_FUNC_NAME (name) (guint8 * src, gdouble * dst, gint scale,        \
    gint count)                                                               \
{                                                                             \
  gdouble tmp;                                                                \
  for (; count; count--) {                                                    \
    tmp = (gdouble) ((((gint32) READ_FUNC (src)) << scale) ^ (sign));         \
    *dst++ = tmp * (1.0 / 2147483647.0);                                      \
    src += stride;                                                            \
  }                                                                           \
}

#define READ8(p)          GST_READ_UINT8(p)
#define READ16_FROM_LE(p) GST_READ_UINT16_LE (p)
#define READ16_FROM_BE(p) GST_READ_UINT16_BE (p)
#define READ24_FROM_LE(p) (p[0] | (p[1] << 8) | (p[2] << 16))
#define READ24_FROM_BE(p) (p[2] | (p[1] << 8) | (p[0] << 16))
#define READ32_FROM_LE(p) GST_READ_UINT32_LE (p)
#define READ32_FROM_BE(p) GST_READ_UINT32_BE (p)

MAKE_UNPACK_FUNC_II (u8, 1, SIGNED, READ8);
MAKE_UNPACK_FUNC_II (s8, 1, 0, READ8);
MAKE_UNPACK_FUNC_II (u16_le, 2, SIGNED, READ16_FROM_LE);
MAKE_UNPACK_FUNC_II (s16_le, 2, 0, READ16_FROM_LE);
MAKE_UNPACK_FUNC_II (u16_be, 2, SIGNED, READ16_FROM_BE);
MAKE_UNPACK_FUNC_II (s16_be, 2, 0, READ16_FROM_BE);
MAKE_UNPACK_FUNC_II (u24_le, 3, SIGNED, READ24_FROM_LE);
MAKE_UNPACK_FUNC_II (s24_le, 3, 0, READ24_FROM_LE);
MAKE_UNPACK_FUNC_II (u24_be, 3, SIGNED, READ24_FROM_BE);
MAKE_UNPACK_FUNC_II (s24_be, 3, 0, READ24_FROM_BE);
MAKE_UNPACK_FUNC_II (u32_le, 4, SIGNED, READ32_FROM_LE);
MAKE_UNPACK_FUNC_II (s32_le, 4, 0, READ32_FROM_LE);
MAKE_UNPACK_FUNC_II (u32_be, 4, SIGNED, READ32_FROM_BE);
MAKE_UNPACK_FUNC_II (s32_be, 4, 0, READ32_FROM_BE);
MAKE_UNPACK_FUNC_FI (float_le, gfloat, GFLOAT_FROM_LE);
MAKE_UNPACK_FUNC_FI (float_be, gfloat, GFLOAT_FROM_BE);
MAKE_UNPACK_FUNC_FI (double_le, gdouble, GDOUBLE_FROM_LE);
MAKE_UNPACK_FUNC_FI (double_be, gdouble, GDOUBLE_FROM_BE);
MAKE_UNPACK_FUNC_FF (float_hq_le, gfloat, GFLOAT_FROM_LE);
MAKE_UNPACK_FUNC_FF (float_hq_be, gfloat, GFLOAT_FROM_BE);
MAKE_UNPACK_FUNC_FF (double_hq_le, gdouble, GDOUBLE_FROM_LE);
MAKE_UNPACK_FUNC_FF (double_hq_be, gdouble, GDOUBLE_FROM_BE);
MAKE_UNPACK_FUNC_IF (u8_float, 1, SIGNED, READ8);
MAKE_UNPACK_FUNC_IF (s8_float, 1, 0, READ8);
MAKE_UNPACK_FUNC_IF (u16_le_float, 2, SIGNED, READ16_FROM_LE);
MAKE_UNPACK_FUNC_IF (s16_le_float, 2, 0, READ16_FROM_LE);
MAKE_UNPACK_FUNC_IF (u16_be_float, 2, SIGNED, READ16_FROM_BE);
MAKE_UNPACK_FUNC_IF (s16_be_float, 2, 0, READ16_FROM_BE);
MAKE_UNPACK_FUNC_IF (u24_le_float, 3, SIGNED, READ24_FROM_LE);
MAKE_UNPACK_FUNC_IF (s24_le_float, 3, 0, READ24_FROM_LE);
MAKE_UNPACK_FUNC_IF (u24_be_float, 3, SIGNED, READ24_FROM_BE);
MAKE_UNPACK_FUNC_IF (s24_be_float, 3, 0, READ24_FROM_BE);
MAKE_UNPACK_FUNC_IF (u32_le_float, 4, SIGNED, READ32_FROM_LE);
MAKE_UNPACK_FUNC_IF (s32_le_float, 4, 0, READ32_FROM_LE);
MAKE_UNPACK_FUNC_IF (u32_be_float, 4, SIGNED, READ32_FROM_BE);
MAKE_UNPACK_FUNC_IF (s32_be_float, 4, 0, READ32_FROM_BE);

/* One of the double_hq_* functions generated above is ineffecient, but it's
 * never used anyway.  The same is true for one of the s32_* functions. */

/*** 
 * packing code
 */
#define MAKE_PACK_FUNC_NAME(name)                                       \
audio_convert_pack_##name

/*
 * These functions convert the signed 32 bit integers to the
 * target format. For this to work the following steps are done:
 *
 * 1) If the output format is unsigned we will XOR the sign bit. This
 *    will do the same as if we add 1<<31.
 * 2) Afterwards we shift to the target depth. It's necessary to left-shift
 *    on signed values here to get arithmetical shifting.
 * 3) This is then written into our target array by the corresponding write
 *    function for the target width.
 */

/* pack from signed integer 32 to integer */
#define MAKE_PACK_FUNC_II(name, stride, sign, WRITE_FUNC)               \
static void                                                             \
MAKE_PACK_FUNC_NAME (name) (gint32 *src, guint8 * dst,                  \
        gint scale, gint count)                                         \
{                                                                       \
  gint32 tmp;                                                           \
  for (;count; count--) {                                               \
    tmp = (*src++ ^ (sign)) >> scale;                                   \
    WRITE_FUNC (dst, tmp);                                              \
    dst += stride;                                                      \
  }                                                                     \
}

/* pack from signed integer 32 to float */
#define MAKE_PACK_FUNC_IF(name, type, FUNC)                             \
static void                                                             \
MAKE_PACK_FUNC_NAME (name) (gint32 * src, type * dst, gint scale,       \
    gint count)                                                         \
{                                                                       \
  for (; count; count--)                                                \
    *dst++ = FUNC ((type) ((*src++) * (1.0 / 2147483647.0)));           \
}

/* pack from float 64 (double) to float */
#define MAKE_PACK_FUNC_FF(name, type, FUNC)                             \
static void                                                             \
MAKE_PACK_FUNC_NAME (name) (gdouble * src, type * dst, gint s,          \
    gint count)                                                         \
{                                                                       \
  for (; count; count--)                                                \
    *dst++ = FUNC ((type) (*src++));                                    \
}

/* pack from float 64 (double) to signed int.
 * the floats are already in the correct range. Only a cast is needed.
 */
#define MAKE_PACK_FUNC_FI_S(name, stride, WRITE_FUNC)                   \
static void                                                             \
MAKE_PACK_FUNC_NAME (name) (gdouble * src, guint8 * dst, gint scale,    \
    gint count)                                                         \
{                                                                       \
  gint32 tmp;                                                           \
  for (; count; count--) {                                              \
    tmp = (gint32) (*src);                                              \
    WRITE_FUNC (dst, tmp);                                              \
    src++;                                                              \
    dst += stride;                                                      \
  }                                                                     \
}

/* pack from float 64 (double) to unsigned int.
 * the floats are already in the correct range. Only a cast is needed
 * and an addition of 2^(target_depth-1) to get in the correct unsigned
 * range. */
#define MAKE_PACK_FUNC_FI_U(name, stride, WRITE_FUNC)                   \
static void                                                             \
MAKE_PACK_FUNC_NAME (name) (gdouble * src, guint8 * dst, gint scale,    \
    gint count)                                                         \
{                                                                       \
  guint32 tmp;                                                          \
  gdouble limit = (1U<<(32-scale-1));                                   \
  for (; count; count--) {                                              \
    tmp = (guint32) (*src + limit);                                     \
    WRITE_FUNC (dst, tmp);                                              \
    src++;                                                              \
    dst += stride;                                                      \
  }                                                                     \
}

#define WRITE8(p, v)       GST_WRITE_UINT8 (p, v)
#define WRITE16_TO_LE(p,v) GST_WRITE_UINT16_LE (p, (guint16)(v))
#define WRITE16_TO_BE(p,v) GST_WRITE_UINT16_BE (p, (guint16)(v))
#define WRITE24_TO_LE(p,v) p[0] = v & 0xff; p[1] = (v >> 8) & 0xff; p[2] = (v >> 16) & 0xff
#define WRITE24_TO_BE(p,v) p[2] = v & 0xff; p[1] = (v >> 8) & 0xff; p[0] = (v >> 16) & 0xff
#define WRITE32_TO_LE(p,v) GST_WRITE_UINT32_LE (p, (guint32)(v))
#define WRITE32_TO_BE(p,v) GST_WRITE_UINT32_BE (p, (guint32)(v))

MAKE_PACK_FUNC_II (u8, 1, SIGNED, WRITE8);
MAKE_PACK_FUNC_II (s8, 1, 0, WRITE8);
MAKE_PACK_FUNC_II (u16_le, 2, SIGNED, WRITE16_TO_LE);
MAKE_PACK_FUNC_II (s16_le, 2, 0, WRITE16_TO_LE);
MAKE_PACK_FUNC_II (u16_be, 2, SIGNED, WRITE16_TO_BE);
MAKE_PACK_FUNC_II (s16_be, 2, 0, WRITE16_TO_BE);
MAKE_PACK_FUNC_II (u24_le, 3, SIGNED, WRITE24_TO_LE);
MAKE_PACK_FUNC_II (s24_le, 3, 0, WRITE24_TO_LE);
MAKE_PACK_FUNC_II (u24_be, 3, SIGNED, WRITE24_TO_BE);
MAKE_PACK_FUNC_II (s24_be, 3, 0, WRITE24_TO_BE);
MAKE_PACK_FUNC_II (u32_le, 4, SIGNED, WRITE32_TO_LE);
MAKE_PACK_FUNC_II (s32_le, 4, 0, WRITE32_TO_LE);
MAKE_PACK_FUNC_II (u32_be, 4, SIGNED, WRITE32_TO_BE);
MAKE_PACK_FUNC_II (s32_be, 4, 0, WRITE32_TO_BE);
MAKE_PACK_FUNC_IF (float_le, gfloat, GFLOAT_TO_LE);
MAKE_PACK_FUNC_IF (float_be, gfloat, GFLOAT_TO_BE);
MAKE_PACK_FUNC_IF (double_le, gdouble, GDOUBLE_TO_LE);
MAKE_PACK_FUNC_IF (double_be, gdouble, GDOUBLE_TO_BE);
MAKE_PACK_FUNC_FF (float_hq_le, gfloat, GFLOAT_TO_LE);
MAKE_PACK_FUNC_FF (float_hq_be, gfloat, GFLOAT_TO_BE);
MAKE_PACK_FUNC_FI_U (u8_float, 1, WRITE8);
MAKE_PACK_FUNC_FI_S (s8_float, 1, WRITE8);
MAKE_PACK_FUNC_FI_U (u16_le_float, 2, WRITE16_TO_LE);
MAKE_PACK_FUNC_FI_S (s16_le_float, 2, WRITE16_TO_LE);
MAKE_PACK_FUNC_FI_U (u16_be_float, 2, WRITE16_TO_BE);
MAKE_PACK_FUNC_FI_S (s16_be_float, 2, WRITE16_TO_BE);
MAKE_PACK_FUNC_FI_U (u24_le_float, 3, WRITE24_TO_LE);
MAKE_PACK_FUNC_FI_S (s24_le_float, 3, WRITE24_TO_LE);
MAKE_PACK_FUNC_FI_U (u24_be_float, 3, WRITE24_TO_BE);
MAKE_PACK_FUNC_FI_S (s24_be_float, 3, WRITE24_TO_BE);
MAKE_PACK_FUNC_FI_U (u32_le_float, 4, WRITE32_TO_LE);
MAKE_PACK_FUNC_FI_S (s32_le_float, 4, WRITE32_TO_LE);
MAKE_PACK_FUNC_FI_U (u32_be_float, 4, WRITE32_TO_BE);
MAKE_PACK_FUNC_FI_S (s32_be_float, 4, WRITE32_TO_BE);

/* For double_hq, packing and unpacking is the same, so we reuse the unpacking
 * functions here. */
#define audio_convert_pack_double_hq_le MAKE_UNPACK_FUNC_NAME (double_hq_le)
#define audio_convert_pack_double_hq_be MAKE_UNPACK_FUNC_NAME (double_hq_be)

static AudioConvertUnpack unpack_funcs[] = {
  (AudioConvertUnpack) MAKE_UNPACK_FUNC_NAME (u8),
  (AudioConvertUnpack) MAKE_UNPACK_FUNC_NAME (s8),
  (AudioConvertUnpack) MAKE_UNPACK_FUNC_NAME (u8),
  (AudioConvertUnpack) MAKE_UNPACK_FUNC_NAME (s8),
  (AudioConvertUnpack) MAKE_UNPACK_FUNC_NAME (u16_le),
  (AudioConvertUnpack) MAKE_UNPACK_FUNC_NAME (s16_le),
  (AudioConvertUnpack) MAKE_UNPACK_FUNC_NAME (u16_be),
  (AudioConvertUnpack) MAKE_UNPACK_FUNC_NAME (s16_be),
  (AudioConvertUnpack) MAKE_UNPACK_FUNC_NAME (u24_le),
  (AudioConvertUnpack) MAKE_UNPACK_FUNC_NAME (s24_le),
  (AudioConvertUnpack) MAKE_UNPACK_FUNC_NAME (u24_be),
  (AudioConvertUnpack) MAKE_UNPACK_FUNC_NAME (s24_be),
  (AudioConvertUnpack) MAKE_UNPACK_FUNC_NAME (u32_le),
  (AudioConvertUnpack) MAKE_UNPACK_FUNC_NAME (s32_le),
  (AudioConvertUnpack) MAKE_UNPACK_FUNC_NAME (u32_be),
  (AudioConvertUnpack) MAKE_UNPACK_FUNC_NAME (s32_be),
  (AudioConvertUnpack) MAKE_UNPACK_FUNC_NAME (float_le),
  (AudioConvertUnpack) MAKE_UNPACK_FUNC_NAME (float_be),
  (AudioConvertUnpack) MAKE_UNPACK_FUNC_NAME (double_le),
  (AudioConvertUnpack) MAKE_UNPACK_FUNC_NAME (double_be),
  (AudioConvertUnpack) MAKE_UNPACK_FUNC_NAME (float_hq_le),
  (AudioConvertUnpack) MAKE_UNPACK_FUNC_NAME (float_hq_be),
  (AudioConvertUnpack) MAKE_UNPACK_FUNC_NAME (double_hq_le),
  (AudioConvertUnpack) MAKE_UNPACK_FUNC_NAME (double_hq_be),
  (AudioConvertUnpack) MAKE_UNPACK_FUNC_NAME (u8_float),
  (AudioConvertUnpack) MAKE_UNPACK_FUNC_NAME (s8_float),
  (AudioConvertUnpack) MAKE_UNPACK_FUNC_NAME (u8_float),
  (AudioConvertUnpack) MAKE_UNPACK_FUNC_NAME (s8_float),
  (AudioConvertUnpack) MAKE_UNPACK_FUNC_NAME (u16_le_float),
  (AudioConvertUnpack) MAKE_UNPACK_FUNC_NAME (s16_le_float),
  (AudioConvertUnpack) MAKE_UNPACK_FUNC_NAME (u16_be_float),
  (AudioConvertUnpack) MAKE_UNPACK_FUNC_NAME (s16_be_float),
  (AudioConvertUnpack) MAKE_UNPACK_FUNC_NAME (u24_le_float),
  (AudioConvertUnpack) MAKE_UNPACK_FUNC_NAME (s24_le_float),
  (AudioConvertUnpack) MAKE_UNPACK_FUNC_NAME (u24_be_float),
  (AudioConvertUnpack) MAKE_UNPACK_FUNC_NAME (s24_be_float),
  (AudioConvertUnpack) MAKE_UNPACK_FUNC_NAME (u32_le_float),
  (AudioConvertUnpack) MAKE_UNPACK_FUNC_NAME (s32_le_float),
  (AudioConvertUnpack) MAKE_UNPACK_FUNC_NAME (u32_be_float),
  (AudioConvertUnpack) MAKE_UNPACK_FUNC_NAME (s32_be_float),
};

static AudioConvertPack pack_funcs[] = {
  (AudioConvertPack) MAKE_PACK_FUNC_NAME (u8),
  (AudioConvertPack) MAKE_PACK_FUNC_NAME (s8),
  (AudioConvertPack) MAKE_PACK_FUNC_NAME (u8),
  (AudioConvertPack) MAKE_PACK_FUNC_NAME (s8),
  (AudioConvertPack) MAKE_PACK_FUNC_NAME (u16_le),
  (AudioConvertPack) MAKE_PACK_FUNC_NAME (s16_le),
  (AudioConvertPack) MAKE_PACK_FUNC_NAME (u16_be),
  (AudioConvertPack) MAKE_PACK_FUNC_NAME (s16_be),
  (AudioConvertPack) MAKE_PACK_FUNC_NAME (u24_le),
  (AudioConvertPack) MAKE_PACK_FUNC_NAME (s24_le),
  (AudioConvertPack) MAKE_PACK_FUNC_NAME (u24_be),
  (AudioConvertPack) MAKE_PACK_FUNC_NAME (s24_be),
  (AudioConvertPack) MAKE_PACK_FUNC_NAME (u32_le),
  (AudioConvertPack) MAKE_PACK_FUNC_NAME (s32_le),
  (AudioConvertPack) MAKE_PACK_FUNC_NAME (u32_be),
  (AudioConvertPack) MAKE_PACK_FUNC_NAME (s32_be),
  (AudioConvertPack) MAKE_PACK_FUNC_NAME (float_le),
  (AudioConvertPack) MAKE_PACK_FUNC_NAME (float_be),
  (AudioConvertPack) MAKE_PACK_FUNC_NAME (double_le),
  (AudioConvertPack) MAKE_PACK_FUNC_NAME (double_be),
  (AudioConvertPack) MAKE_PACK_FUNC_NAME (float_hq_le),
  (AudioConvertPack) MAKE_PACK_FUNC_NAME (float_hq_be),
  (AudioConvertPack) MAKE_PACK_FUNC_NAME (double_hq_le),
  (AudioConvertPack) MAKE_PACK_FUNC_NAME (double_hq_be),
  (AudioConvertPack) MAKE_PACK_FUNC_NAME (u8_float),
  (AudioConvertPack) MAKE_PACK_FUNC_NAME (s8_float),
  (AudioConvertPack) MAKE_PACK_FUNC_NAME (u8_float),
  (AudioConvertPack) MAKE_PACK_FUNC_NAME (s8_float),
  (AudioConvertPack) MAKE_PACK_FUNC_NAME (u16_le_float),
  (AudioConvertPack) MAKE_PACK_FUNC_NAME (s16_le_float),
  (AudioConvertPack) MAKE_PACK_FUNC_NAME (u16_be_float),
  (AudioConvertPack) MAKE_PACK_FUNC_NAME (s16_be_float),
  (AudioConvertPack) MAKE_PACK_FUNC_NAME (u24_le_float),
  (AudioConvertPack) MAKE_PACK_FUNC_NAME (s24_le_float),
  (AudioConvertPack) MAKE_PACK_FUNC_NAME (u24_be_float),
  (AudioConvertPack) MAKE_PACK_FUNC_NAME (s24_be_float),
  (AudioConvertPack) MAKE_PACK_FUNC_NAME (u32_le_float),
  (AudioConvertPack) MAKE_PACK_FUNC_NAME (s32_le_float),
  (AudioConvertPack) MAKE_PACK_FUNC_NAME (u32_be_float),
  (AudioConvertPack) MAKE_PACK_FUNC_NAME (s32_be_float),
};

#define DOUBLE_INTERMEDIATE_FORMAT(ctx)                                 \
    ((!ctx->in.is_int && !ctx->out.is_int) || (ctx->ns != NOISE_SHAPING_NONE))

static gint
audio_convert_get_func_index (AudioConvertCtx * ctx, AudioConvertFmt * fmt)
{
  gint index = 0;

  if (fmt->is_int) {
    index += (fmt->width / 8 - 1) * 4;
    index += fmt->endianness == G_LITTLE_ENDIAN ? 0 : 2;
    index += fmt->sign ? 1 : 0;
    index += (ctx->ns == NOISE_SHAPING_NONE) ? 0 : 24;
  } else {
    /* this is float/double */
    index = 16;
    index += (fmt->width == 32) ? 0 : 2;
    index += (fmt->endianness == G_LITTLE_ENDIAN) ? 0 : 1;
    index += (DOUBLE_INTERMEDIATE_FORMAT (ctx)) ? 4 : 0;
  }

  return index;
}

static inline gboolean
check_default (AudioConvertCtx * ctx, AudioConvertFmt * fmt)
{
  if (!DOUBLE_INTERMEDIATE_FORMAT (ctx)) {
    return (fmt->width == 32 && fmt->depth == 32 &&
        fmt->endianness == G_BYTE_ORDER && fmt->sign == TRUE);
  } else {
    return (fmt->width == 64 && fmt->endianness == G_BYTE_ORDER);
  }
}

gboolean
audio_convert_clean_fmt (AudioConvertFmt * fmt)
{
  g_return_val_if_fail (fmt != NULL, FALSE);

  g_free (fmt->pos);
  fmt->pos = NULL;

  return TRUE;
}


gboolean
audio_convert_prepare_context (AudioConvertCtx * ctx, AudioConvertFmt * in,
    AudioConvertFmt * out, GstAudioConvertDithering dither,
    GstAudioConvertNoiseShaping ns)
{
  gint idx_in, idx_out;

  g_return_val_if_fail (ctx != NULL, FALSE);
  g_return_val_if_fail (in != NULL, FALSE);
  g_return_val_if_fail (out != NULL, FALSE);

  /* first clean the existing context */
  audio_convert_clean_context (ctx);

  g_return_val_if_fail (in->unpositioned_layout == out->unpositioned_layout,
      FALSE);

  ctx->in = *in;
  ctx->out = *out;

  /* Don't dither or apply noise shaping if out depth is bigger than 20 bits
   * as DA converters only can do a SNR up to 20 bits in reality.
   * Also don't dither or apply noise shaping if target depth is larger than
   * source depth. */
  if (ctx->out.depth <= 20 && (!ctx->in.is_int
          || ctx->in.depth >= ctx->out.depth)) {
    ctx->dither = dither;
    ctx->ns = ns;
  } else {
    ctx->dither = DITHER_NONE;
    ctx->ns = NOISE_SHAPING_NONE;
  }

  /* Use simple error feedback when output sample rate is smaller than
   * 32000 as the other methods might move the noise to audible ranges */
  if (ctx->ns > NOISE_SHAPING_ERROR_FEEDBACK && ctx->out.rate < 32000)
    ctx->ns = NOISE_SHAPING_ERROR_FEEDBACK;

  gst_channel_mix_setup_matrix (ctx);

  idx_in = audio_convert_get_func_index (ctx, in);
  ctx->unpack = unpack_funcs[idx_in];

  idx_out = audio_convert_get_func_index (ctx, out);
  ctx->pack = pack_funcs[idx_out];

  /* if both formats are float/double or we use noise shaping use double as
   * intermediate format and and switch mixing */
  if (!DOUBLE_INTERMEDIATE_FORMAT (ctx)) {
    GST_INFO ("use int mixing");
    ctx->channel_mix = (AudioConvertMix) gst_channel_mix_mix_int;
  } else {
    GST_INFO ("use float mixing");
    ctx->channel_mix = (AudioConvertMix) gst_channel_mix_mix_float;
  }
  GST_INFO ("unitsizes: %d -> %d", in->unit_size, out->unit_size);

  /* check if input is in default format */
  ctx->in_default = check_default (ctx, in);
  /* check if channel mixer is passthrough */
  ctx->mix_passthrough = gst_channel_mix_passthrough (ctx);
  /* check if output is in default format */
  ctx->out_default = check_default (ctx, out);

  GST_INFO ("in default %d, mix passthrough %d, out default %d",
      ctx->in_default, ctx->mix_passthrough, ctx->out_default);

  ctx->in_scale = (in->is_int) ? (32 - in->depth) : 0;
  ctx->out_scale = (out->is_int) ? (32 - out->depth) : 0;

  gst_audio_quantize_setup (ctx);

  return TRUE;
}

gboolean
audio_convert_clean_context (AudioConvertCtx * ctx)
{
  g_return_val_if_fail (ctx != NULL, FALSE);

  gst_audio_quantize_free (ctx);
  audio_convert_clean_fmt (&ctx->in);
  audio_convert_clean_fmt (&ctx->out);
  gst_channel_mix_unset_matrix (ctx);

  g_free (ctx->tmpbuf);
  ctx->tmpbuf = NULL;
  ctx->tmpbufsize = 0;

  return TRUE;
}

gboolean
audio_convert_get_sizes (AudioConvertCtx * ctx, gint samples, gint * srcsize,
    gint * dstsize)
{
  g_return_val_if_fail (ctx != NULL, FALSE);

  if (srcsize)
    *srcsize = samples * ctx->in.unit_size;
  if (dstsize)
    *dstsize = samples * ctx->out.unit_size;

  return TRUE;
}

gboolean
audio_convert_convert (AudioConvertCtx * ctx, gpointer src,
    gpointer dst, gint samples, gboolean src_writable)
{
  guint insize, outsize, size;
  gpointer outbuf, tmpbuf;
  guint intemp = 0, outtemp = 0, biggest;

  g_return_val_if_fail (ctx != NULL, FALSE);
  g_return_val_if_fail (src != NULL, FALSE);
  g_return_val_if_fail (dst != NULL, FALSE);
  g_return_val_if_fail (samples >= 0, FALSE);

  if (samples == 0)
    return TRUE;

  insize = ctx->in.unit_size * samples;
  outsize = ctx->out.unit_size * samples;

  /* find biggest temp buffer size */
  size = (DOUBLE_INTERMEDIATE_FORMAT (ctx)) ? sizeof (gdouble)
      : sizeof (gint32);

  if (!ctx->in_default)
    intemp = gst_util_uint64_scale (insize, size * 8, ctx->in.width);
  if (!ctx->mix_passthrough || !ctx->out_default)
    outtemp = gst_util_uint64_scale (outsize, size * 8, ctx->out.width);
  biggest = MAX (intemp, outtemp);

  /* see if one of the buffers can be used as temp */
  if ((outsize >= biggest) && (ctx->out.unit_size <= size))
    tmpbuf = dst;
  else if ((insize >= biggest) && src_writable && (ctx->in.unit_size >= size))
    tmpbuf = src;
  else {
    if (biggest > ctx->tmpbufsize) {
      ctx->tmpbuf = g_realloc (ctx->tmpbuf, biggest);
      ctx->tmpbufsize = biggest;
    }
    tmpbuf = ctx->tmpbuf;
  }

  /* start conversion */
  if (!ctx->in_default) {
    /* check if final conversion */
    if (!(ctx->out_default && ctx->mix_passthrough))
      outbuf = tmpbuf;
    else
      outbuf = dst;

    /* unpack to default format */
    ctx->unpack (src, outbuf, ctx->in_scale, samples * ctx->in.channels);

    src = outbuf;
  }

  if (!ctx->mix_passthrough) {
    /* check if final conversion */
    if (!ctx->out_default)
      outbuf = tmpbuf;
    else
      outbuf = dst;

    /* convert channels */
    ctx->channel_mix (ctx, src, outbuf, samples);

    src = outbuf;
  }

  /* we only need to quantize if output format is int */
  if (ctx->out.is_int) {
    if (ctx->out_default)
      outbuf = dst;
    else
      outbuf = tmpbuf;
    ctx->quantize (ctx, src, outbuf, samples);
  }

  if (!ctx->out_default) {
    /* pack default format into dst */
    ctx->pack (src, dst, ctx->out_scale, samples * ctx->out.channels);
  }

  return TRUE;
}
