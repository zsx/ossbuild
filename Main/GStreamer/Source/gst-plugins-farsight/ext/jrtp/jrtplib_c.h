/* GStreamer
 * Copyright (C) 2004 Anatoly Yakovenko <aeyakovenko@yahoo.com>
 * Copyright (C) 2005 Philippe Khalaf <burger@speedy.org>
 *
 * jrtplib_c.h: 
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

#ifndef JRTPLIB_C_H
#define JRTPLIB_C_H

#define RTP_MAXIMUMPACKETSIZE 65535
//#define TIMESTAMP_UNIT 1.0 / 1000.0

#include <gst/gst.h>
#include <gst/netbuffer/gstnetbuffer.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef void* jrtpsession_t;
    typedef void* rtpgstv4transmissionparams_t; 

    jrtpsession_t jrtpsession_init();

    //creates a session with a default pt mark and time inc
    rtpgstv4transmissionparams_t jrtpsession_create(jrtpsession_t sess, gint clockrate);
    void jrtpsession_setport(jrtpsession_t sess, guint port);
    void jrtpsession_setpads(jrtpsession_t sess, GstPad *rtpsrc, GstPad *rtcpsrc);
    int jrtpsession_setcurrentdata(jrtpsession_t sess, GstNetBuffer *buf, gint type);
    int jrtpsession_setdefaultpt(jrtpsession_t sess, guchar pt); 
    int jrtpsession_setdefaultmark(jrtpsession_t sess, gboolean mark);
    int jrtpsession_setdefaultinc(jrtpsession_t sess, guint inc); 
    int jrtpsession_settimestampunit(jrtpsession_t sess, gdouble unit);

    //destroys the session
    void jrtpsession_destroy(jrtpsession_t sess);

    //addr:port;addr;addr:port...
    void jrtpsession_setdestinationaddrs(jrtpsession_t sess, gchar *ip, guint16 port);

    //set the destination array
    int jrtpsession_setdestinations(jrtpsession_t sess, unsigned long* remoteip, 
            int* remoteportbase, guint size); 

    //addr or addr:port
    int jrtpsession_setacceptsourceaddr(jrtpsession_t sess, gchar* addr);

    //if port < 0 then it will accept from any port
    int jrtpsession_setacceptsources(jrtpsession_t sess, unsigned long* remoteip,
            int* remoteportbase, guint size);

    //increment the timestamp
    int jrtpsession_incrementtimestamp(jrtpsession_t sess,
            unsigned int timeinc);

    //send a packet
    int jrtpsession_sendpacket(jrtpsession_t sess, guint8* data, guint len, 
            guchar pt, gushort mark, guint timeinc);

    // send a packet with default values
    int jrtpsession_sendpacket_default(jrtpsession_t sess, guint8* data, guint len); 

    //polls the library for data
    int jrtpsession_poll(jrtpsession_t sess);

    GstBuffer* jrtpsession_getpacket(jrtpsession_t sess);

#ifdef __cplusplus
}
#endif

#endif
