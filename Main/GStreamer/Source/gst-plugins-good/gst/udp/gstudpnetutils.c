/* GStreamer UDP network utility functions
 * Copyright (C) 2006 Tim-Philipp Müller <tim centricular net>
 * Copyright (C) 2006 Joni Valtanen <joni.valtanen@movial.fi>
 * Copyright (C) 2009 Jarkko Palviainen <jarkko.palviainen@sesca.com>
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

#include <errno.h>
#include <stdio.h>
#include <memory.h>

#include "gstudpnetutils.h"

/* EAI_ADDRFAMILY was obsoleted in BSD at some point */
#ifndef EAI_ADDRFAMILY
#define EAI_ADDRFAMILY 1
#endif

#ifdef G_OS_WIN32

gboolean
gst_udp_net_utils_win32_wsa_startup (GstObject * obj)
{
  WSADATA w;
  int error;

  error = WSAStartup (0x0202, &w);

  if (error) {
    GST_WARNING_OBJECT (obj, "WSAStartup error: %d", error);
    return FALSE;
  }

  if (w.wVersion != 0x0202) {
    WSACleanup ();
    GST_WARNING_OBJECT (obj, "Winsock version wrong : 0x%x", w.wVersion);
    return FALSE;
  }

  return TRUE;
}

#endif

int
gst_udp_get_sockaddr_length (struct sockaddr_storage *addr)
{
  /* MacOS is picky about passing precisely the correct length,
   * so we calculate it here for the given socket type.
   */
  switch (addr->ss_family) {
    case AF_INET:
      return sizeof (struct sockaddr_in);
    case AF_INET6:
      return sizeof (struct sockaddr_in6);
    default:
      /* don't know, Screw MacOS and use the full length */
      return sizeof (*addr);
  }
}

int
gst_udp_get_addr (const char *hostname, int port, struct sockaddr_storage *addr)
{
  struct addrinfo hints, *res = NULL, *nres;
  char service[NI_MAXSERV];
  int ret;

  memset (&hints, 0, sizeof (hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;
  g_snprintf (service, sizeof (service) - 1, "%d", port);
  service[sizeof (service) - 1] = '\0';

  if ((ret = getaddrinfo (hostname, (port == -1) ? NULL : service, &hints,
              &res)) < 0) {
    goto beach;
  }

  nres = res;
  while (nres) {
    if (nres->ai_family == AF_INET || nres->ai_family == AF_INET6)
      break;
    nres = nres->ai_next;
  }

  if (nres) {
    memcpy (addr, nres->ai_addr, nres->ai_addrlen);
  } else {
    ret = EAI_ADDRFAMILY;
  }

  freeaddrinfo (res);
beach:
  return ret;
}

int
gst_udp_set_loop (int sockfd, gboolean loop)
{
  socklen_t socklen;
  struct sockaddr_storage addr;
  int ret = -1;
  int l = (loop == FALSE) ? 0 : 1;

  socklen = sizeof (addr);
  if ((ret = getsockname (sockfd, (struct sockaddr *) &addr, &socklen)) < 0) {
    return ret;
  }

  switch (addr.ss_family) {
    case AF_INET:
    {
      ret = setsockopt (sockfd, IPPROTO_IP, IP_MULTICAST_LOOP, &l, sizeof (l));
      if (ret < 0)
        return ret;

      break;
    }
    case AF_INET6:
    {
      ret =
          setsockopt (sockfd, IPPROTO_IPV6, IPV6_MULTICAST_LOOP, &l,
          sizeof (l));
      if (ret < 0)
        return ret;

      break;
    }
    default:
#ifdef G_OS_WIN32
      WSASetLastError (WSAEAFNOSUPPORT);
#else
      errno = EAFNOSUPPORT;
#endif
  }

  return ret;
}

int
gst_udp_set_ttl (int sockfd, int ttl, gboolean is_multicast)
{
  socklen_t socklen;
  struct sockaddr_storage addr;
  int optname = -1;
  int ret = -1;

  socklen = sizeof (addr);
  if ((ret = getsockname (sockfd, (struct sockaddr *) &addr, &socklen)) < 0) {
    return ret;
  }

  switch (addr.ss_family) {
    case AF_INET:
    {
      optname = (is_multicast == TRUE) ? IP_MULTICAST_TTL : IP_TTL;
      ret = setsockopt (sockfd, IPPROTO_IP, optname, &ttl, sizeof (ttl));
      if (ret < 0)
        return ret;
      break;
    }
    case AF_INET6:
    {
      optname =
          (is_multicast == TRUE) ? IPV6_MULTICAST_HOPS : IPV6_UNICAST_HOPS;
      ret = setsockopt (sockfd, IPPROTO_IPV6, optname, &ttl, sizeof (ttl));
      if (ret < 0)
        return ret;

      /* When using IPV4 address with IPV6 socket, both TTL values
         must be set in order to actually use the given value.
         Has no effect when IPV6 address is used. */
      optname = (is_multicast == TRUE) ? IP_MULTICAST_TTL : IP_TTL;
      ret = setsockopt (sockfd, IPPROTO_IP, optname, &ttl, sizeof (ttl));
      if (ret < 0)
        return ret;
      break;
    }
    default:
#ifdef G_OS_WIN32
      WSASetLastError (WSAEAFNOSUPPORT);
#else
      errno = EAFNOSUPPORT;
#endif
  }
  return ret;
}

/* FIXME: Add interface selection for windows hosts.  */
int
gst_udp_join_group (int sockfd, struct sockaddr_storage *addr, gchar * iface)
{
  int ret = -1;

  switch (addr->ss_family) {
    case AF_INET:
    {
#ifdef HAVE_IP_MREQN
      struct ip_mreqn mreq4;
#else
      struct ip_mreq mreq4;
#endif

      memset (&mreq4, 0, sizeof (mreq4));
      mreq4.imr_multiaddr.s_addr =
          ((struct sockaddr_in *) addr)->sin_addr.s_addr;
#ifdef HAVE_IP_MREQN
      if (iface)
        mreq4.imr_ifindex = if_nametoindex (iface);
      else
        mreq4.imr_ifindex = 0;  /* Pick any.  */
#else
      mreq4.imr_interface.s_addr = INADDR_ANY;
#endif

      if ((ret =
              setsockopt (sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                  (const void *) &mreq4, sizeof (mreq4))) < 0)
        return ret;

      break;
    }
    case AF_INET6:
    {
      struct ipv6_mreq mreq6;

      memset (&mreq6, 0, sizeof (mreq6));
      memcpy (&mreq6.ipv6mr_multiaddr,
          &(((struct sockaddr_in6 *) addr)->sin6_addr),
          sizeof (struct in6_addr));
      mreq6.ipv6mr_interface = 0;
#if !defined(G_OS_WIN32)
      if (iface)
        mreq6.ipv6mr_interface = if_nametoindex (iface);
#endif

      if ((ret =
              setsockopt (sockfd, IPPROTO_IPV6, IPV6_JOIN_GROUP,
                  (const void *) &mreq6, sizeof (mreq6))) < 0)
        return ret;

      break;
    }
    default:
#ifdef G_OS_WIN32
      WSASetLastError (WSAEAFNOSUPPORT);
#else
      errno = EAFNOSUPPORT;
#endif
  }
  return ret;
}

int
gst_udp_leave_group (int sockfd, struct sockaddr_storage *addr)
{
  int ret = -1;

  switch (addr->ss_family) {
    case AF_INET:
    {
      struct ip_mreq mreq4;

      memset (&mreq4, 0, sizeof (mreq4));
      mreq4.imr_multiaddr.s_addr =
          ((struct sockaddr_in *) addr)->sin_addr.s_addr;
      mreq4.imr_interface.s_addr = INADDR_ANY;

      if ((ret =
              setsockopt (sockfd, IPPROTO_IP, IP_DROP_MEMBERSHIP,
                  (const void *) &mreq4, sizeof (mreq4))) < 0)
        return ret;
    }
      break;

    case AF_INET6:
    {
      struct ipv6_mreq mreq6;

      memset (&mreq6, 0, sizeof (mreq6));
      memcpy (&mreq6.ipv6mr_multiaddr,
          &(((struct sockaddr_in6 *) addr)->sin6_addr),
          sizeof (struct in6_addr));
      mreq6.ipv6mr_interface = 0;

      if ((ret =
              setsockopt (sockfd, IPPROTO_IPV6, IPV6_LEAVE_GROUP,
                  (const void *) &mreq6, sizeof (mreq6))) < 0)
        return ret;
    }
      break;

    default:
#ifdef G_OS_WIN32
      WSASetLastError (WSAEAFNOSUPPORT);
#else
      errno = EAFNOSUPPORT;
#endif
  }

  return ret;
}

int
gst_udp_is_multicast (struct sockaddr_storage *addr)
{
  int ret = -1;

  switch (addr->ss_family) {
    case AF_INET:
    {
      struct sockaddr_in *addr4 = (struct sockaddr_in *) addr;

      ret = IN_MULTICAST (g_ntohl (addr4->sin_addr.s_addr));
    }
      break;

    case AF_INET6:
    {
      struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *) addr;

      ret = IN6_IS_ADDR_MULTICAST (&addr6->sin6_addr);
    }
      break;

    default:
#ifdef G_OS_WIN32
      WSASetLastError (WSAEAFNOSUPPORT);
#else
      errno = EAFNOSUPPORT;
#endif
  }

  return ret;
}
