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
 * SECTION:gstrtspdefs
 * @short_description: common RTSP defines
 * @see_also: gstrtspurl, gstrtspconnection
 *  
 * <refsect2>
 * <para>
 * Provides common defines for the RTSP library. 
 * </para>
 * </refsect2>
 *  
 * Last reviewed on 2007-07-24 (0.10.14)
 */

#include <errno.h>

extern int h_errno;

#include "gstrtspdefs.h"

#ifdef G_OS_WIN32
#include <winsock2.h>
#else
#include <netdb.h>
#endif

static const gchar *rtsp_results[] = {
  "OK",
  /* errors */
  "Generic error",
  "Invalid parameter specified",
  "Operation interrupted",
  "Out of memory",
  "Cannot resolve host",
  "Function not implemented",
  "System error: %s",
  "Parse error",
  "Error on WSAStartup",
  "Windows sockets are not version 0x202",
  "Received end-of-file",
  "Network error: %s",
  "Host is not a valid IP address",
  "Timeout while waiting for server response",
  "Tunnel GET request received",
  "Tunnel POST request received",
  "Unknown error (%d)",
  NULL
};

static const gchar *rtsp_methods[] = {
  "DESCRIBE",
  "ANNOUNCE",
  "GET_PARAMETER",
  "OPTIONS",
  "PAUSE",
  "PLAY",
  "RECORD",
  "REDIRECT",
  "SETUP",
  "SET_PARAMETER",
  "TEARDOWN",
  NULL
};

static const gchar *rtsp_headers[] = {
  "Accept",                     /* Accept               R      opt.      entity */
  "Accept-Encoding",            /* Accept-Encoding      R      opt.      entity */
  "Accept-Language",            /* Accept-Language      R      opt.      all */
  "Allow",                      /* Allow                r      opt.      all */
  "Authorization",              /* Authorization        R      opt.      all */
  "Bandwidth",                  /* Bandwidth            R      opt.      all */
  "Blocksize",                  /* Blocksize            R      opt.      all but OPTIONS, TEARDOWN */
  "Cache-Control",              /* Cache-Control        g      opt.      SETUP */
  "Conference",                 /* Conference           R      opt.      SETUP */
  "Connection",                 /* Connection           g      req.      all */
  "Content-Base",               /* Content-Base         e      opt.      entity */
  "Content-Encoding",           /* Content-Encoding     e      req.      SET_PARAMETER, DESCRIBE, ANNOUNCE */
  "Content-Language",           /* Content-Language     e      req.      DESCRIBE, ANNOUNCE */
  "Content-Length",             /* Content-Length       e      req.      SET_PARAMETER, ANNOUNCE, entity */
  "Content-Location",           /* Content-Location     e      opt.      entity */
  "Content-Type",               /* Content-Type         e      req.      SET_PARAMETER, ANNOUNCE, entity */
  "CSeq",                       /* CSeq                 g      req.      all */
  "Date",                       /* Date                 g      opt.      all */
  "Expires",                    /* Expires              e      opt.      DESCRIBE, ANNOUNCE */
  "From",                       /* From                 R      opt.      all */
  "If-Modified-Since",          /* If-Modified-Since    R      opt.      DESCRIBE, SETUP */
  "Last-Modified",              /* Last-Modified        e      opt.      entity */
  "Proxy-Authenticate",         /* Proxy-Authenticate */
  "Proxy-Require",              /* Proxy-Require        R      req.      all */
  "Public",                     /* Public               r      opt.      all */
  "Range",                      /* Range                Rr     opt.      PLAY, PAUSE, RECORD */
  "Referer",                    /* Referer              R      opt.      all */
  "Require",                    /* Require              R      req.      all */
  "Retry-After",                /* Retry-After          r      opt.      all */
  "RTP-Info",                   /* RTP-Info             r      req.      PLAY */
  "Scale",                      /* Scale                Rr     opt.      PLAY, RECORD */
  "Session",                    /* Session              Rr     req.      all but SETUP, OPTIONS */
  "Server",                     /* Server               r      opt.      all */
  "Speed",                      /* Speed                Rr     opt.      PLAY */
  "Transport",                  /* Transport            Rr     req.      SETUP */
  "Unsupported",                /* Unsupported          r      req.      all */
  "User-Agent",                 /* User-Agent           R      opt.      all */
  "Via",                        /* Via                  g      opt.      all */
  "WWW-Authenticate",           /* WWW-Authenticate     r      opt.      all */

  /* Real extensions */
  "ClientChallenge",            /* ClientChallenge */
  "RealChallenge1",             /* RealChallenge1 */
  "RealChallenge2",             /* RealChallenge2 */
  "RealChallenge3",             /* RealChallenge3 */
  "Subscribe",                  /* Subscribe */
  "Alert",                      /* Alert */
  "ClientID",                   /* ClientID */
  "CompanyID",                  /* CompanyID */
  "GUID",                       /* GUID */
  "RegionData",                 /* RegionData */
  "SupportsMaximumASMBandwidth",        /* SupportsMaximumASMBandwidth */
  "Language",                   /* Language */
  "PlayerStarttime",            /* PlayerStarttime */

  "Location",                   /* Location */
  "ETag",                       /* ETag */
  "If-Match",                   /* If-Match */

  /* WM extensions [MS-RTSP] */
  "Accept-Charset",             /* Accept-Charset */
  "Supported",                  /* Supported */
  "Vary",                       /* Vary */
  "X-Accelerate-Streaming",     /* X-Accelerate-Streaming */
  "X-Accept-Authentication",    /* X-Accept-Authentication */
  "X-Accept-Proxy-Authentication",      /* X-Accept-Proxy-Authentication */
  "X-Broadcast-Id",             /* X-Broadcast-Id */
  "X-Burst-Streaming",          /* X-Burst-Streaming */
  "X-Notice",                   /* X-Notice */
  "X-Player-Lag-Time",          /* X-Player-Lag-Time */
  "X-Playlist",                 /* X-Playlist */
  "X-Playlist-Change-Notice",   /* X-Playlist-Change-Notice */
  "X-Playlist-Gen-Id",          /* X-Playlist-Gen-Id */
  "X-Playlist-Seek-Id",         /* X-Playlist-Seek-Id */
  "X-Proxy-Client-Agent",       /* X-Proxy-Client-Agent */
  "X-Proxy-Client-Verb",        /* X-Proxy-Client-Verb */
  "X-Receding-PlaylistChange",  /* X-Receding-PlaylistChange */
  "X-RTP-Info",                 /* X-RTP-Info */
  "X-StartupProfile",           /* X-StartupProfile */
  "Timestamp",                  /* Timestamp */

  NULL
};

#define DEF_STATUS(c, t) \
  g_hash_table_insert (statuses, GUINT_TO_POINTER(c), t)

static GHashTable *
rtsp_init_status (void)
{
  GHashTable *statuses = g_hash_table_new (NULL, NULL);

  DEF_STATUS (GST_RTSP_STS_CONTINUE, "Continue");
  DEF_STATUS (GST_RTSP_STS_OK, "OK");
  DEF_STATUS (GST_RTSP_STS_CREATED, "Created");
  DEF_STATUS (GST_RTSP_STS_LOW_ON_STORAGE, "Low on Storage Space");
  DEF_STATUS (GST_RTSP_STS_MULTIPLE_CHOICES, "Multiple Choices");
  DEF_STATUS (GST_RTSP_STS_MOVED_PERMANENTLY, "Moved Permanently");
  DEF_STATUS (GST_RTSP_STS_MOVE_TEMPORARILY, "Move Temporarily");
  DEF_STATUS (GST_RTSP_STS_SEE_OTHER, "See Other");
  DEF_STATUS (GST_RTSP_STS_NOT_MODIFIED, "Not Modified");
  DEF_STATUS (GST_RTSP_STS_USE_PROXY, "Use Proxy");
  DEF_STATUS (GST_RTSP_STS_BAD_REQUEST, "Bad Request");
  DEF_STATUS (GST_RTSP_STS_UNAUTHORIZED, "Unauthorized");
  DEF_STATUS (GST_RTSP_STS_PAYMENT_REQUIRED, "Payment Required");
  DEF_STATUS (GST_RTSP_STS_FORBIDDEN, "Forbidden");
  DEF_STATUS (GST_RTSP_STS_NOT_FOUND, "Not Found");
  DEF_STATUS (GST_RTSP_STS_METHOD_NOT_ALLOWED, "Method Not Allowed");
  DEF_STATUS (GST_RTSP_STS_NOT_ACCEPTABLE, "Not Acceptable");
  DEF_STATUS (GST_RTSP_STS_PROXY_AUTH_REQUIRED,
      "Proxy Authentication Required");
  DEF_STATUS (GST_RTSP_STS_REQUEST_TIMEOUT, "Request Time-out");
  DEF_STATUS (GST_RTSP_STS_GONE, "Gone");
  DEF_STATUS (GST_RTSP_STS_LENGTH_REQUIRED, "Length Required");
  DEF_STATUS (GST_RTSP_STS_PRECONDITION_FAILED, "Precondition Failed");
  DEF_STATUS (GST_RTSP_STS_REQUEST_ENTITY_TOO_LARGE,
      "Request Entity Too Large");
  DEF_STATUS (GST_RTSP_STS_REQUEST_URI_TOO_LARGE, "Request-URI Too Large");
  DEF_STATUS (GST_RTSP_STS_UNSUPPORTED_MEDIA_TYPE, "Unsupported Media Type");
  DEF_STATUS (GST_RTSP_STS_PARAMETER_NOT_UNDERSTOOD,
      "Parameter Not Understood");
  DEF_STATUS (GST_RTSP_STS_CONFERENCE_NOT_FOUND, "Conference Not Found");
  DEF_STATUS (GST_RTSP_STS_NOT_ENOUGH_BANDWIDTH, "Not Enough Bandwidth");
  DEF_STATUS (GST_RTSP_STS_SESSION_NOT_FOUND, "Session Not Found");
  DEF_STATUS (GST_RTSP_STS_METHOD_NOT_VALID_IN_THIS_STATE,
      "Method Not Valid in This State");
  DEF_STATUS (GST_RTSP_STS_HEADER_FIELD_NOT_VALID_FOR_RESOURCE,
      "Header Field Not Valid for Resource");
  DEF_STATUS (GST_RTSP_STS_INVALID_RANGE, "Invalid Range");
  DEF_STATUS (GST_RTSP_STS_PARAMETER_IS_READONLY, "Parameter Is Read-Only");
  DEF_STATUS (GST_RTSP_STS_AGGREGATE_OPERATION_NOT_ALLOWED,
      "Aggregate operation not allowed");
  DEF_STATUS (GST_RTSP_STS_ONLY_AGGREGATE_OPERATION_ALLOWED,
      "Only aggregate operation allowed");
  DEF_STATUS (GST_RTSP_STS_UNSUPPORTED_TRANSPORT, "Unsupported transport");
  DEF_STATUS (GST_RTSP_STS_DESTINATION_UNREACHABLE, "Destination unreachable");
  DEF_STATUS (GST_RTSP_STS_INTERNAL_SERVER_ERROR, "Internal Server Error");
  DEF_STATUS (GST_RTSP_STS_NOT_IMPLEMENTED, "Not Implemented");
  DEF_STATUS (GST_RTSP_STS_BAD_GATEWAY, "Bad Gateway");
  DEF_STATUS (GST_RTSP_STS_SERVICE_UNAVAILABLE, "Service Unavailable");
  DEF_STATUS (GST_RTSP_STS_GATEWAY_TIMEOUT, "Gateway Time-out");
  DEF_STATUS (GST_RTSP_STS_RTSP_VERSION_NOT_SUPPORTED,
      "RTSP Version not supported");
  DEF_STATUS (GST_RTSP_STS_OPTION_NOT_SUPPORTED, "Option not supported");

  return statuses;
}

/**
 * gst_rtsp_strresult:
 * @result: a #GstRTSPResult
 *
 * Convert @result in a human readable string.
 *
 * Returns: a newly allocated string. g_free() after usage.
 */
gchar *
gst_rtsp_strresult (GstRTSPResult result)
{
  gint idx;
  gchar *res;

  idx = ABS (result);
  idx = CLAMP (idx, 0, -GST_RTSP_ELAST);

  switch (idx) {
#ifdef G_OS_WIN32
    case -GST_RTSP_ESYS:
    case -GST_RTSP_ENET:
    {
      gchar *msg = g_win32_error_message (WSAGetLastError ());
      res = g_strdup_printf (rtsp_results[idx], msg);
      g_free (msg);
      break;
    }
#else
    case -GST_RTSP_ESYS:
      res = g_strdup_printf (rtsp_results[idx], g_strerror (errno));
      break;
    case -GST_RTSP_ENET:
      res = g_strdup_printf (rtsp_results[idx], hstrerror (h_errno));
#endif
      break;
    case -GST_RTSP_ELAST:
      res = g_strdup_printf (rtsp_results[idx], result);
      break;
    default:
      res = g_strdup (rtsp_results[idx]);
      break;
  }
  return res;
}

/**
 * gst_rtsp_method_as_text:
 * @method: a #GstRTSPMethod
 *
 * Convert @method to a string.
 *
 * Returns: a string representation of @method.
 */
const gchar *
gst_rtsp_method_as_text (GstRTSPMethod method)
{
  gint i;

  if (method == GST_RTSP_INVALID)
    return NULL;

  i = 0;
  while ((method & 1) == 0) {
    i++;
    method >>= 1;
  }
  return rtsp_methods[i];
}

/**
 * gst_rtsp_version_as_text:
 * @version: a #GstRTSPVersion
 *
 * Convert @version to a string.
 *
 * Returns: a string representation of @version.
 */
const gchar *
gst_rtsp_version_as_text (GstRTSPVersion version)
{
  switch (version) {
    case GST_RTSP_VERSION_1_0:
      return "1.0";

    default:
      return "0.0";
  }
}

/**
 * gst_rtsp_header_as_text:
 * @field: a #GstRTSPHeaderField
 *
 * Convert @field to a string.
 *
 * Returns: a string representation of @field.
 */
const gchar *
gst_rtsp_header_as_text (GstRTSPHeaderField field)
{
  if (field == GST_RTSP_HDR_INVALID)
    return NULL;
  else
    return rtsp_headers[field - 1];
}

/**
 * gst_rtsp_status_as_text:
 * @code: a #GstRTSPStatusCode
 *
 * Convert @code to a string.
 *
 * Returns: a string representation of @code.
 */
const gchar *
gst_rtsp_status_as_text (GstRTSPStatusCode code)
{
  static GHashTable *statuses;

  if (G_UNLIKELY (statuses == NULL))
    statuses = rtsp_init_status ();

  return g_hash_table_lookup (statuses, GUINT_TO_POINTER (code));
}

/**
 * gst_rtsp_find_header_field:
 * @header: a header string
 *
 * Convert @header to a #GstRTSPHeaderField.
 *
 * Returns: a #GstRTSPHeaderField for @header or #GST_RTSP_HDR_INVALID if the
 * header field is unknown.
 */
GstRTSPHeaderField
gst_rtsp_find_header_field (const gchar * header)
{
  gint idx;

  for (idx = 0; rtsp_headers[idx]; idx++) {
    if (g_ascii_strcasecmp (rtsp_headers[idx], header) == 0) {
      return idx + 1;
    }
  }
  return GST_RTSP_HDR_INVALID;
}

/**
 * gst_rtsp_find_method:
 * @method: a method
 *
 * Convert @method to a #GstRTSPMethod.
 *
 * Returns: a #GstRTSPMethod for @method or #GST_RTSP_INVALID if the
 * method is unknown.
 */
GstRTSPMethod
gst_rtsp_find_method (const gchar * method)
{
  gint idx;

  for (idx = 0; rtsp_methods[idx]; idx++) {
    if (g_ascii_strcasecmp (rtsp_methods[idx], method) == 0) {
      return (1 << idx);
    }
  }
  return GST_RTSP_INVALID;
}

/**
 * gst_rtsp_options_as_text:
 * @options: one or more #GstRTSPMethod
 *
 * Convert @options to a string.
 *
 * Returns: a new string of @options. g_free() after usage.
 *
 * Since: 0.10.23
 */
gchar *
gst_rtsp_options_as_text (GstRTSPMethod options)
{
  GString *str;

  str = g_string_new ("");

  if (options & GST_RTSP_OPTIONS)
    g_string_append (str, "OPTIONS, ");
  if (options & GST_RTSP_DESCRIBE)
    g_string_append (str, "DESCRIBE, ");
  if (options & GST_RTSP_ANNOUNCE)
    g_string_append (str, "ANNOUNCE, ");
  if (options & GST_RTSP_GET_PARAMETER)
    g_string_append (str, "GET_PARAMETER, ");
  if (options & GST_RTSP_PAUSE)
    g_string_append (str, "PAUSE, ");
  if (options & GST_RTSP_PLAY)
    g_string_append (str, "PLAY, ");
  if (options & GST_RTSP_RECORD)
    g_string_append (str, "RECORD, ");
  if (options & GST_RTSP_REDIRECT)
    g_string_append (str, "REDIRECT, ");
  if (options & GST_RTSP_SETUP)
    g_string_append (str, "SETUP, ");
  if (options & GST_RTSP_SET_PARAMETER)
    g_string_append (str, "SET_PARAMETER, ");
  if (options & GST_RTSP_TEARDOWN)
    g_string_append (str, "TEARDOWN, ");

  /* remove trailing ", " if there is one */
  if (str->len > 2)
    str = g_string_truncate (str, str->len - 2);

  return g_string_free (str, FALSE);
}
