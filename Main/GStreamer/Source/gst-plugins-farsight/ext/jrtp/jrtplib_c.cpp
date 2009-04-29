/* GStreamer
 * Copyright (C) 2005 Philippe Khalaf <burger@speedy.org>
 * Copyright (C) 2004 Anatoly Yakovenko <aeyakovenko@yahoo.com>
 *
 * jrtplib_c.cpp: 
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

#include "jrtplib_c.h"

//jrtplib
#include <jrtplib3/rtpsession.h>
#include <jrtplib3/rtpsessionparams.h>
#include <jrtplib3/rtpfaketransmitter.h>
#include <jrtplib3/rtpipv4address.h>
#include <jrtplib3/rtppacket.h>

#include <gst/rtp/gstrtpbuffer.h>

#include <string>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <sys/poll.h>

#include <arpa/inet.h>

#include <errno.h>

#include <gst/gst.h>

GST_DEBUG_CATEGORY (rtpbin_debug);
#define GST_CAT_DEFAULT (rtpbin_debug)

// Override RTPSession to use a UserDefinedProto
class MyRTPSession:public RTPSession
{
    public:
    MyRTPSession():RTPSession() { };
    void setRTPSendSrc(GstPad *pad) { rtpsend_rtp_src = pad; };
    GstPad *getRTPSendSrc() { return rtpsend_rtp_src; };
    void setRTCPSendSrc(GstPad *pad) { rtpsend_rtcp_src = pad; };
    GstPad *getRTCPSendSrc() { return rtpsend_rtcp_src; };

    protected:
    RTPTransmitter *NewUserDefinedTransmitter()
    {
        RTPTransmitter *new_trans = new RTPFakeTransmitter(GetMemoryManager());
        return new_trans;
    }

    private:
    GstPad *rtpsend_rtp_src;
    GstPad *rtpsend_rtcp_src;
};


int checkerror(int rtperr)
{
    if (rtperr < 0)
    {
        std::string errstr = RTPGetErrorString(rtperr);
        GST_ERROR("%d %s", rtperr, errstr.c_str());
    }
    return rtperr;
}

// this callback will be called by jrtps transmitter when packets have been
// processed and are ready to be sent out
void push_packet_on_pad(void* sess, guint8* data, guint16 len, guint32 ip, guint16 port, gboolean rtp)
{
    MyRTPSession* s = reinterpret_cast<MyRTPSession*>(sess);

    GstNetBuffer *out_buf;

    // TODO
    // TEMP WORKAROUND
    // ADD JRTPLIB OPTION TO STOP RTCP IF DOSENT EXIST YET
    if (!rtp && ! s->getRTCPSendSrc())
        return;

    out_buf = gst_netbuffer_new ();
    GST_BUFFER_DATA (out_buf) = (guint8 *)data;
    GST_BUFFER_SIZE (out_buf) = len;
    gst_netaddress_set_ip4_address (&out_buf->to, ip, port);
    GST_DEBUG("2. Outgoing RTP/RTCP packet back from jrtplib, sending RTP/RTCP packet %p %d %p", 
            GST_BUFFER_DATA(out_buf), GST_BUFFER_SIZE(out_buf),
            s->getRTPSendSrc());

    // push data
    gst_pad_push(rtp?s->getRTPSendSrc():s->getRTCPSendSrc(),
            GST_BUFFER(out_buf));
}

//creates a session with a default pt mark and time inc
jrtpsession_t jrtpsession_init()
{
    GST_DEBUG_CATEGORY_INIT (rtpbin_debug, "rtpbin", 0, "RTP Session");
    GST_INFO("Initialising RTP Session");

    MyRTPSession* s = new MyRTPSession();
    return s;
}

rtpgstv4transmissionparams_t
jrtpsession_create(jrtpsession_t sess, gint clockrate)
{
    GST_INFO("Creating RTP Session");
    MyRTPSession* s = reinterpret_cast<MyRTPSession*>(sess);

    GST_DEBUG("got clockrate in create %d", clockrate );
    RTPSessionParams sessionparams;
    sessionparams.SetOwnTimestampUnit(1 / clockrate);
    sessionparams.SetUsePollThread(0); 
    sessionparams.SetAcceptOwnPackets(true);

    RTPFakeTransmissionParams *transparams = new RTPFakeTransmissionParams;

    transparams->SetPacketReadyCB(&push_packet_on_pad);
    transparams->SetPacketReadyCBData(s);

    checkerror(s->Create(sessionparams, transparams,
          RTPTransmitter::UserDefinedProto));
    checkerror(s->SetMaximumPacketSize(RTP_MAXIMUMPACKETSIZE));
    checkerror(s->SetReceiveMode(RTPTransmitter::AcceptAll));

    return transparams;
}

void jrtpsession_setport(jrtpsession_t sess, guint port)
{
    MyRTPSession* s = reinterpret_cast<MyRTPSession*>(sess);

    RTPFakeTransmissionInfo *transinfo = (RTPFakeTransmissionInfo *)s->GetTransmissionInfo();

    RTPFakeTransmissionParams *p = transinfo->GetTransParams();

    if (port % 2 != 0)
    {
        GST_DEBUG ("Port has to be even! I'm taking port %d instead", port-1);
        p->SetPortbase(port-1);
    }
    p->SetPortbase(port);
    delete transinfo;
}


void jrtpsession_setpads(jrtpsession_t sess, GstPad *rtpsrc, GstPad *rtcpsrc)
{
    MyRTPSession* s = reinterpret_cast<MyRTPSession*>(sess);
    s->setRTPSendSrc(rtpsrc);
    s->setRTCPSendSrc(rtcpsrc);
}

int jrtpsession_setcurrentdata(jrtpsession_t sess, GstNetBuffer *buf, gint type)
{
    MyRTPSession* s = reinterpret_cast<MyRTPSession*>(sess);

    RTPFakeTransmissionInfo *transinfo = (RTPFakeTransmissionInfo *)s->GetTransmissionInfo();

    RTPFakeTransmissionParams *p = transinfo->GetTransParams();
    guint32 addr;
    guint16 port;

    if (!GST_IS_NETBUFFER (buf))
    {
        GST_DEBUG("Buffer is not a netbuffer!");
        return 0;
    }
    gst_netaddress_get_ip4_address (&buf->from, &addr, &port);

    if(p->GetCurrentData() != NULL)
    {
        GST_DEBUG("Data ptr in transparams not NULL! Overwriting!");
    }
    p->SetCurrentData(GST_BUFFER_DATA(buf));
    p->SetCurrentDataLen(GST_BUFFER_SIZE(buf));
    p->SetCurrentDataAddr(g_ntohl(addr));
    p->SetCurrentDataPort(g_ntohs(port));
    p->SetCurrentDataType(type);
    GST_DEBUG("Current data set to RTPsession, ready to be polled %p %d %d %d", 
            p->GetCurrentData(), p->GetCurrentDataLen(), p->GetCurrentDataAddr(),
            p->GetCurrentDataPort());

    delete transinfo;

    return 1;
}

int jrtpsession_setdefaultpt(jrtpsession_t sess, guchar pt)
{
    MyRTPSession* s = reinterpret_cast<MyRTPSession*>(sess);
    GST_DEBUG("Setting default payload type to %d", pt);
    return checkerror(s->SetDefaultPayloadType(pt));
}

int jrtpsession_setdefaultmark(jrtpsession_t sess, gboolean mark)
{
    MyRTPSession* s = reinterpret_cast<MyRTPSession*>(sess);
    GST_DEBUG("Setting default mark to %d", mark);
    return checkerror(s->SetDefaultMark(mark));
}

int jrtpsession_setdefaultinc(jrtpsession_t sess, guint inc)
{
    MyRTPSession* s = reinterpret_cast<MyRTPSession*>(sess);
    GST_DEBUG("Setting default timestamp increment to %d", inc);
    return checkerror(s->SetDefaultTimestampIncrement(inc));
}

int jrtpsession_settimestampunit(jrtpsession_t sess, gdouble unit)
{
    MyRTPSession* s = reinterpret_cast<MyRTPSession*>(sess);
    GST_DEBUG("Setting timestamp unit");
    return checkerror(s->SetTimestampUnit(unit));
}

void jrtpsession_setdestinationaddrs(jrtpsession_t sess, gchar *ip, guint16 port)
{
    MyRTPSession* s = reinterpret_cast<MyRTPSession*>(sess);
 
    unsigned long intIP = inet_addr(ip);
    if(intIP != INADDR_NONE)
    {
        GST_DEBUG("setting destination addr %s : %d", ip, port);
        intIP = ntohl(intIP);
        RTPIPv4Address addr(intIP, port);
        checkerror(s->AddDestination(addr));
    } else {
        GST_DEBUG("Error converting IP to integer %s", ip);
    }
}

/*int jrtpsession_setdestinations(jrtpsession_t sess, unsigned long* remoteip, 
  int* remoteportbase, unsigned size) 
  {
  MyRTPSession* s = reinterpret_cast<MyRTPSession*>(sess);
  s->ClearDestinations();
  for(unsigned i=0; i< size; i++)
  {
  if(int z = s->AddDestination(remoteip[i], remoteportbase[i]) < 0)
  return z;
  }
  return 0;
  }*/

int jrtpsession_setacceptsourceaddr(jrtpsession_t sess, gchar* c_addr)
{

    MyRTPSession* s = reinterpret_cast<MyRTPSession*>(sess);
    s->ClearAcceptList();
    int port(-1);
    std::string ip(c_addr);
    size_t collon = ip.find(":");

    if(collon != std::string::npos)
    {
        std::string s_port = ip.substr(collon+1, ip.size() - (collon + 1));
        port = atoi(s_port.c_str());
        ip = ip.substr(0, collon);
    }

    if(port < 0 || port > 65535)
    {
        unsigned long intIP = inet_addr(ip.c_str());
        if(intIP != INADDR_NONE)
        {
            GST_DEBUG("setting accept addr %s", ip.c_str());
            intIP = ntohl(intIP);
            RTPIPv4Address addr(intIP, 0);
            checkerror(s->AddToAcceptList(addr));
        } else {
            GST_DEBUG("Error converting IP to integer %s", ip.c_str());
        }
    }
    else 
    {
        unsigned long intIP = inet_addr(ip.c_str());
        if(intIP != INADDR_NONE)
        {
            GST_DEBUG("setting accept addr %s : %d", ip.c_str(), port);
            intIP = ntohl(intIP);
            RTPIPv4Address addr(intIP, port);
            checkerror(s->AddToAcceptList(addr));
        } else {
            GST_DEBUG("Error converting IP to integer %s", ip.c_str());
        }
    }

    return 0;
}

/*int jrtpsession_setacceptsources(jrtpsession_t sess, unsigned long* remoteip,
  int* remoteportbase, unsigned size)
  {
  MyRTPSession* s = reinterpret_cast<MyRTPSession*>(sess);
  s->ClearAcceptList();
  for(unsigned i=0; i< size; i++)
  {
  if(remoteportbase[i] < 0)
  {
  if(int z = s->AddToAcceptList(remoteip[i], true, remoteportbase[i]) < 0)
  return z;
  }
  else
  {
  if(int z = s->AddToAcceptList(remoteip[i], false, remoteportbase[i]) < 0)
  return z;
  }
  }
  return 0;
  }*/

void jrtpsession_destroy(jrtpsession_t sess)
{
    MyRTPSession* s = reinterpret_cast<MyRTPSession*>(sess);
    s->Destroy();
    delete s;
}

int jrtpsession_incrementtimestamp(jrtpsession_t sess, unsigned int timeinc)
{
    MyRTPSession* s = reinterpret_cast<MyRTPSession*>(sess);
    GST_DEBUG("incrementing timestamp by %u", timeinc);
    return checkerror(s->IncrementTimestamp(timeinc));
}

int jrtpsession_sendpacket(jrtpsession_t sess, guint8 * data, guint len, 
        unsigned char pt, unsigned short mark, unsigned int timeinc)
{
    MyRTPSession* s = reinterpret_cast<MyRTPSession*>(sess);
    GST_DEBUG("sending packet data length: %d mark : %d timeinc : %u", len, mark, timeinc);
    return checkerror(s->SendPacket((void*)data, len, pt, mark, timeinc));
}



int jrtpsession_sendpacket_default(jrtpsession_t sess, guint8* data, guint len)
{
    MyRTPSession* s = reinterpret_cast<MyRTPSession*>(sess);
    //GST_DEBUG("packet with defaults info data length: %d", len);
    return checkerror(s->SendPacket((void*)data, len));
}

int jrtpsession_poll(jrtpsession_t sess)
{
    MyRTPSession* s = reinterpret_cast<MyRTPSession*>(sess);

    GST_DEBUG("Polling");
    return checkerror(s->Poll());
} 

GstBuffer* jrtpsession_getpacket(jrtpsession_t sess)
{
    MyRTPSession* s = reinterpret_cast<MyRTPSession*>(sess);

    guint size = 0;
    static int tot=1;
    GstBuffer* rtpbuf = NULL;

    GST_DEBUG("Getting packet");
    if(s->GotoFirstSourceWithData())
    {
        GST_DEBUG("There is a source");
        RTPPacket* packet = NULL;
        packet = s->GetNextPacket();
        if(packet)
        {
            if(packet->GetPayloadLength() > 0)
            {
                size = packet->GetPacketLength(); 
                if(size > RTP_MAXIMUMPACKETSIZE)
                {
                    GST_DEBUG("packet is to big %d", size);
                    return NULL;
                }
                else
                {
                    rtpbuf = gst_rtp_buffer_new_copy_data (packet->GetPacketData(),(unsigned long) size);
                    GST_DEBUG("%d got packet %d, timestamp %d mark %d plsize %d", 
                            tot, size, packet->GetTimestamp(), packet->HasMarker(), packet->GetPayloadLength());
                    GST_DEBUG("%d got packet %d, timestamp %d mark %d plsize %d", 
                            tot, gst_rtp_buffer_get_packet_len (rtpbuf),
                            gst_rtp_buffer_get_timestamp (rtpbuf),
                            gst_rtp_buffer_get_marker (rtpbuf),
                            gst_rtp_buffer_get_payload_len (rtpbuf));
                    tot++;
                    GST_DEBUG("memcpied packet successfully");
                }
            }
            delete packet;
        }
        /*else
          {
          GST_DEBUG("get next packet returned NULL");
          if(s->GetCurrentSourceInfo() != NULL)
          {
          s->GetCurrentSourceInfo()->FlushPackets();
          }
          }*/
    } else {
        GST_DEBUG("No source with data available");
    }

    return rtpbuf;
}
