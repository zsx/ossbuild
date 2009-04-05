/* GStreamer
 * Copyright (C) <2005,2006> Wim Taymans <wim@fluendo.com>
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
/*
 * Unless otherwise indicated, Source Code is licensed under MIT license.
 * See further explanation attached in License Statement (distributed in the file
 * LICENSE).
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * SECTION:gstrtsprange
 * @short_description: dealing with time ranges
 *  
 * <refsect2>
 * <para>
 * Provides helper functions to deal with time ranges.
 * </para>
 * </refsect2>
 *  
 * Last reviewed on 2007-07-25 (0.10.14)
 */


#include <stdio.h>
#include <string.h>

#include "gstrtsprange.h"

/* npt-time     =   "now" | npt-sec | npt-hhmmss
 * npt-sec      =   1*DIGIT [ "." *DIGIT ]
 * npt-hhmmss   =   npt-hh ":" npt-mm ":" npt-ss [ "." *DIGIT ]
 * npt-hh       =   1*DIGIT     ; any positive number
 * npt-mm       =   1*2DIGIT    ; 0-59
 * npt-ss       =   1*2DIGIT    ; 0-59
 */
static GstRTSPResult
parse_npt_time (const gchar * str, GstRTSPTime * time)
{
  if (strcmp (str, "now") == 0) {
    time->type = GST_RTSP_TIME_NOW;
  } else if (str[0] == '\0') {
    time->type = GST_RTSP_TIME_END;
  } else if (strstr (str, ":")) {
    gfloat seconds;
    gint hours, mins;

    sscanf (str, "%2d:%2d:%f", &hours, &mins, &seconds);

    time->type = GST_RTSP_TIME_SECONDS;
    time->seconds = ((hours * 60) + mins) * 60 + seconds;
  } else {
    gfloat seconds;

    sscanf (str, "%f", &seconds);

    time->type = GST_RTSP_TIME_SECONDS;
    time->seconds = seconds;
  }
  return GST_RTSP_OK;
}

/* npt-range    =   ( npt-time "-" [ npt-time ] ) | ( "-" npt-time )
 */
static GstRTSPResult
parse_npt_range (const gchar * str, GstRTSPTimeRange * range)
{
  GstRTSPResult res;
  gchar *p;

  range->unit = GST_RTSP_RANGE_NPT;

  /* find '-' separator */
  p = strstr (str, "-");
  if (p == NULL)
    return GST_RTSP_EINVAL;

  if ((res = parse_npt_time (str, &range->min)) != GST_RTSP_OK)
    goto done;

  res = parse_npt_time (p + 1, &range->max);

done:
  return res;
}

static GstRTSPResult
parse_clock_range (const gchar * str, GstRTSPTimeRange * range)
{
  return GST_RTSP_ENOTIMPL;
}

static GstRTSPResult
parse_smpte_range (const gchar * str, GstRTSPTimeRange * range)
{
  return GST_RTSP_ENOTIMPL;
}

/**
 * gst_rtsp_range_parse:
 * @rangestr: a range string to parse
 * @range: location to hold the #GstRTSPTimeRange result
 *
 * Parse @rangestr to a #GstRTSPTimeRange.
 *
 * Returns: #GST_RTSP_OK on success.
 */
GstRTSPResult
gst_rtsp_range_parse (const gchar * rangestr, GstRTSPTimeRange ** range)
{
  GstRTSPResult ret;
  GstRTSPTimeRange *res;
  gchar *p;

  g_return_val_if_fail (rangestr != NULL, GST_RTSP_EINVAL);
  g_return_val_if_fail (range != NULL, GST_RTSP_EINVAL);

  res = g_new0 (GstRTSPTimeRange, 1);

  p = (gchar *) rangestr;
  /* first figure out the units of the range */
  if (g_str_has_prefix (p, "npt=")) {
    ret = parse_npt_range (p + 4, res);
  } else if (g_str_has_prefix (p, "clock=")) {
    ret = parse_clock_range (p + 6, res);
  } else if (g_str_has_prefix (p, "smpte=")) {
    res->unit = GST_RTSP_RANGE_SMPTE;
    ret = parse_smpte_range (p + 6, res);
  } else if (g_str_has_prefix (p, "smpte-30-drop=")) {
    res->unit = GST_RTSP_RANGE_SMPTE_30_DROP;
    ret = parse_smpte_range (p + 14, res);
  } else if (g_str_has_prefix (p, "smpte-25=")) {
    res->unit = GST_RTSP_RANGE_SMPTE_25;
    ret = parse_smpte_range (p + 9, res);
  } else
    goto invalid;

  if (ret == GST_RTSP_OK)
    *range = res;

  return ret;

  /* ERRORS */
invalid:
  {
    gst_rtsp_range_free (res);
    return GST_RTSP_EINVAL;
  }
}

/**
 * gst_rtsp_range_free:
 * @range: a #GstRTSPTimeRange
 *
 * Free the memory alocated by @range.
 */
void
gst_rtsp_range_free (GstRTSPTimeRange * range)
{
  if (range == NULL)
    return;

  g_free (range);
}
