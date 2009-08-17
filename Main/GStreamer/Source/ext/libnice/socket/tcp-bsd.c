/*
 * This file is part of the Nice GLib ICE library.
 *
 * (C) 2006-2008 Collabora Ltd.
 *  Contact: Dafydd Harries
 *  Contact: Olivier Crete
 * (C) 2006, 2007 Nokia Corporation. All rights reserved.
 *  Contact: Kai Vehmanen
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is the Nice GLib ICE library.
 *
 * The Initial Developers of the Original Code are Collabora Ltd and Nokia
 * Corporation. All Rights Reserved.
 *
 * Contributors:
 *   Dafydd Harries, Collabora Ltd.
 *   Olivier Crete, Collabora Ltd.
 *   Rémi Denis-Courmont, Nokia
 *   Kai Vehmanen
 *
 * Alternatively, the contents of this file may be used under the terms of the
 * the GNU Lesser General Public License Version 2.1 (the "LGPL"), in which
 * case the provisions of LGPL are applicable instead of those above. If you
 * wish to allow use of your version of this file only under the terms of the
 * LGPL and not to allow others to use your version of this file under the
 * MPL, indicate your decision by deleting the provisions above and replace
 * them with the notice and other provisions required by the LGPL. If you do
 * not delete the provisions above, a recipient may use your version of this
 * file under either the MPL or the LGPL.
 */

/*
 * Implementation of TCP relay socket interface using TCP Berkeley sockets. (See
 * http://en.wikipedia.org/wiki/Berkeley_sockets.)
 */
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "tcp-bsd.h"
#include "agent-priv.h"

#include <string.h>
#include <errno.h>
#include <fcntl.h>

#ifndef G_OS_WIN32
#include <unistd.h>
#endif

typedef struct {
  NiceAgent *agent;
  NiceAddress server_addr;
  GQueue send_queue;
  GMainContext *context;
  GIOChannel *io_channel;
  GSource *io_source;
} TcpPriv;

struct to_be_sent {
  guint length;
  gchar *buf;
  gboolean can_drop;
};

#define MAX_QUEUE_LENGTH 20

static void socket_close (NiceSocket *sock);
static gint socket_recv (NiceSocket *sock, NiceAddress *from,
    guint len, gchar *buf);
static gboolean socket_send (NiceSocket *sock, const NiceAddress *to,
    guint len, const gchar *buf);
static gboolean socket_is_reliable (NiceSocket *sock);


static void add_to_be_sent (NiceSocket *sock, const gchar *buf, guint len,
    gboolean head);
static void free_to_be_sent (struct to_be_sent *tbs);
static gboolean socket_send_more (GIOChannel *source, GIOCondition condition,
    gpointer data);

NiceSocket *
nice_tcp_bsd_socket_new (NiceAgent *agent, GMainContext *ctx, NiceAddress *addr)
{
  int sockfd = -1;
  int ret;
  struct sockaddr_storage name;
  guint name_len = sizeof (name);
  NiceSocket *sock = g_slice_new0 (NiceSocket);
  TcpPriv *priv;

  if (addr != NULL) {
    nice_address_copy_to_sockaddr(addr, (struct sockaddr *)&name);
  } else {
    memset (&name, 0, sizeof (name));
    name.ss_family = AF_UNSPEC;
  }

  if ((sockfd == -1) &&
      ((name.ss_family == AF_UNSPEC) ||
          (name.ss_family == AF_INET))) {
    sockfd = socket (PF_INET, SOCK_STREAM, 0);
    name.ss_family = AF_INET;
#ifdef HAVE_SA_LEN
    name.ss_len = sizeof (struct sockaddr_in);
#endif
  }

  if (sockfd == -1) {
    g_slice_free (NiceSocket, sock);
    return NULL;
  }

#ifdef FD_CLOEXEC
  fcntl (sockfd, F_SETFD, fcntl (sockfd, F_GETFD) | FD_CLOEXEC);
#endif
#ifdef O_NONBLOCK
  fcntl (sockfd, F_SETFL, fcntl (sockfd, F_GETFL) | O_NONBLOCK);
#endif

  name_len = name.ss_family == AF_INET? sizeof (struct sockaddr_in) :
      sizeof(struct sockaddr_in6);
  ret = connect (sockfd, (const struct sockaddr *)&name, name_len);

#ifdef G_OS_WIN32
  if (ret < 0 && WSAGetLastError () != WSAEINPROGRESS) {
    closesocket (sockfd);
#else
  if (ret < 0 && errno != EINPROGRESS) {
    close (sockfd);
#endif
    g_slice_free (NiceSocket, sock);
    return NULL;
  }

  name_len = name.ss_family == AF_INET? sizeof (struct sockaddr_in) :
      sizeof(struct sockaddr_in6);
  if (getsockname (sockfd, (struct sockaddr *) &name, &name_len) < 0) {
    g_slice_free (NiceSocket, sock);
#ifdef G_OS_WIN32
    closesocket(sockfd);
#else
    close (sockfd);
#endif
    return NULL;
  }

  nice_address_set_from_sockaddr (&sock->addr, (struct sockaddr *)&name);

  sock->priv = priv = g_slice_new0 (TcpPriv);

  priv->agent = agent;
  priv->context = ctx;
  priv->server_addr = *addr;

  sock->fileno = sockfd;
  sock->send = socket_send;
  sock->recv = socket_recv;
  sock->is_reliable = socket_is_reliable;
  sock->close = socket_close;

  return sock;
}


static void
socket_close (NiceSocket *sock)
{
  TcpPriv *priv = sock->priv;

#ifdef G_OS_WIN32
  closesocket(sock->fileno);
#else
  close (sock->fileno);
#endif
  if (priv->io_source) {
    g_source_destroy (priv->io_source);
    g_source_unref (priv->io_source);
  }
  if (priv->io_channel)
    g_io_channel_unref (priv->io_channel);
  g_queue_foreach (&priv->send_queue, (GFunc) free_to_be_sent, NULL);
  g_queue_clear (&priv->send_queue);

  g_slice_free(TcpPriv, sock->priv);
}

static gint
socket_recv (NiceSocket *sock, NiceAddress *from, guint len, gchar *buf)
{
  TcpPriv *priv = sock->priv;
  int ret;

  ret = recv (sock->fileno, buf, len, 0);

  /* recv returns 0 when the peer performed a shutdown.. we must return -1 here
   * so that the agent destroys the g_source */
  if (ret == 0)
    return -1;

  if (ret < 0) {
#ifdef G_OS_WIN32
    if (WSAGetLastError () == WSAEWOULDBLOCK)
#else
    if (errno == EAGAIN)
#endif
      return 0;
    else
      return ret;
  }

  if (from)
    *from = priv->server_addr;
  return ret;
}

/* Data sent to this function must be a single entity because buffers can be
 * dropped if the bandwidth isn't fast enough. So do not send a message in
 * multiple chunks. */
static gboolean
socket_send (NiceSocket *sock, const NiceAddress *to,
    guint len, const gchar *buf)
{
  TcpPriv *priv = sock->priv;
  int ret;

  /* First try to send the data, don't send it later if it can be sent now
     this way we avoid allocating memory on every send */
  if (g_queue_is_empty (&priv->send_queue)) {
    ret = send (sock->fileno, buf, len, 0);

    if (ret < 0) {
#ifdef G_OS_WIN32
      if (WSAGetLastError () == WSAEWOULDBLOCK) {
#else
      if (errno == EAGAIN) {
#endif
        add_to_be_sent (sock, buf, len, FALSE);
        return TRUE;
      } else {
        return FALSE;
      }
    } else if ((guint)ret < len) {
      add_to_be_sent (sock, buf + ret, len - ret, TRUE);
      return TRUE;
    }
  } else {
    if (g_queue_get_length(&priv->send_queue) >= MAX_QUEUE_LENGTH) {
      int peek_idx = 0;
      struct to_be_sent *tbs = NULL;
      while ((tbs = g_queue_peek_nth (&priv->send_queue, peek_idx)) != NULL) {
        if (tbs->can_drop) {
          tbs = g_queue_pop_nth (&priv->send_queue, peek_idx);
          g_free (tbs->buf);
          g_slice_free (struct to_be_sent, tbs);
          break;
        } else {
          peek_idx++;
        }
      }
    }
    add_to_be_sent (sock, buf, len, FALSE);
  }

  return TRUE;
}

static gboolean
socket_is_reliable (NiceSocket *sock)
{
  return TRUE;
}


/*
 * Returns:
 * -1 = error
 * 0 = have more to send
 * 1 = sent everything
 */

static gboolean
socket_send_more (
  GIOChannel *source,
  GIOCondition condition,
  gpointer data)
{
  NiceSocket *sock = (NiceSocket *) data;
  TcpPriv *priv = sock->priv;
  struct to_be_sent *tbs = NULL;

  agent_lock ();

  if (g_source_is_destroyed (g_main_current_source ())) {
    nice_debug ("Source was destroyed. "
        "Avoided race condition in tcp-bsd.c:socket_send_more");
    agent_unlock ();
    return FALSE;
  }

  while ((tbs = g_queue_pop_head (&priv->send_queue)) != NULL) {
    int ret;

    ret = send (sock->fileno, tbs->buf, tbs->length, 0);

    if (ret < 0) {
#ifdef G_OS_WIN32
      if (WSAGetLastError () == WSAEWOULDBLOCK) {
#else
      if (errno == EAGAIN) {
#endif
        add_to_be_sent (sock, tbs->buf, tbs->length, TRUE);
        g_free (tbs->buf);
        g_slice_free (struct to_be_sent, tbs);
        break;
      }
    } else if (ret < (int) tbs->length) {
      add_to_be_sent (sock, tbs->buf + ret, tbs->length - ret, TRUE);
      g_free (tbs->buf);
      g_slice_free (struct to_be_sent, tbs);
      break;
    }

    g_free (tbs->buf);
    g_slice_free (struct to_be_sent, tbs);
  }

  if (g_queue_is_empty (&priv->send_queue)) {
    g_io_channel_unref (priv->io_channel);
    priv->io_channel = NULL;
    g_source_destroy (priv->io_source);
    g_source_unref (priv->io_source);
    priv->io_source = NULL;

    agent_unlock ();
    return FALSE;
  }

  agent_unlock ();
  return TRUE;
}


static void
add_to_be_sent (NiceSocket *sock, const gchar *buf, guint len, gboolean head)
{
  TcpPriv *priv = sock->priv;
  struct to_be_sent *tbs = NULL;

  if (len <= 0)
    return;

  tbs = g_slice_new0 (struct to_be_sent);
  tbs->buf = g_memdup (buf, len);
  tbs->length = len;
  tbs->can_drop = !head;
  if (head)
    g_queue_push_head (&priv->send_queue, tbs);
  else
    g_queue_push_tail (&priv->send_queue, tbs);

  if (priv->io_channel == NULL) {
    priv->io_channel = g_io_channel_unix_new (sock->fileno);
    priv->io_source = g_io_create_watch (priv->io_channel, G_IO_OUT);
    g_source_set_callback (priv->io_source, (GSourceFunc) socket_send_more,
        sock, NULL);
    g_source_attach (priv->io_source, priv->context);
  }
}



static void
free_to_be_sent (struct to_be_sent *tbs)
{
  g_free (tbs->buf);
  g_slice_free (struct to_be_sent, tbs);
}

