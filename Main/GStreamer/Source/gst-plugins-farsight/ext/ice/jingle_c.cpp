/*
 * Farsight Voice+Video library
 *  Copyright 2006 Collabora Ltd, 
 *  Copyright 2006 Nokia Corporation
 *   @author: Philippe Khalaf <philippe.khalaf@collabora.co.uk>.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "jingle_c.h"

#include <string.h>

#include <glib.h>

#include <talk/base/thread.h>
#include <talk/base/physicalsocketserver.h>
#include <talk/base/network.h>
#include <talk/base/socketaddress.h>
#include <talk/p2p/base/socketmanager.h>
#include <talk/p2p/base/helpers.h>
#include <talk/p2p/client/basicportallocator.h>
#include <talk/p2p/client/socketclient.h>

class SignalListener2 : public sigslot::has_slots<>
{
    private:
        callback_list cb_list;
        SocketClient *socketclient;

    public:
        SignalListener2(SocketClient *sc) { memset(&cb_list, 0, sizeof(callback_list)); socketclient = sc; };
        callback_list *getCbList() { return &cb_list; };
        void OnSocketRead(P2PSocket *socket, const char *data, size_t len);
};

void SignalListener2::OnSocketRead(P2PSocket *socket, const char *data, size_t len)
{
    guint i;
    g_mutex_lock (socketclient->sigl2->getCbList()->mutex);
    for (i = 0; i < socketclient->sigl2->getCbList()->socket_read_cb_array->len;
            i++)
    {
        cb_info *cb;
        cb = &g_array_index
            (socketclient->sigl2->getCbList()->socket_read_cb_array, 
             cb_info, i);
        socket_read_cb_type cb_ptr = (socket_read_cb_type)cb->callback;
        //g_message("called socketread cb");

        if (!socket->best_connection())
        {
            g_warning ("received %" G_GSIZE_FORMAT " bytes "
                "but no best_connection, ignoring", len);
            goto finally;
        }

        cb_ptr (cb->data, (gpointer)data, len,
            socket->best_connection()->remote_candidate().address().ip(),
            socket->best_connection()->remote_candidate().address().port());
    }

finally:
    g_mutex_unlock (socketclient->sigl2->getCbList()->mutex);
}

static void init_callback_list (SocketClient *sc)
{
    if (!sc->sigl2)
        sc->sigl2 = new SignalListener2(sc);
    if (!sc->sigl2->getCbList()->mutex)
        sc->sigl2->getCbList()->mutex = g_mutex_new();

    if (!sc->sigl2->getCbList()->socket_read_cb_array)
    {
        sc->sigl2->getCbList()->socket_read_cb_array =
            g_array_new(FALSE, TRUE, sizeof(cb_info));
        if (sc->getSocket())
        {
            g_message("Connected to ReadPacket");
            sc->getSocket()->SignalReadPacket.connect(sc->sigl2,
                    &SignalListener2::OnSocketRead);
        }
        else
        {
            g_message ("No socket created yet! SocketRead not connect");
        }
    }
}

void connect_signal_socket_read (socketclient_t sockclient, gpointer callback,
        gpointer data)
{
    SocketClient *sc =
        reinterpret_cast<SocketClient*>(sockclient);
    if (!sc->sigl2)
    {
        init_callback_list(sc);

        if (!sc->getSocket())
        {
            g_message ("No socket created yet! Callback not connected");
            return;
        }
    }

    cb_info new_cb;
    new_cb.callback = callback;
    new_cb.data = data;

    g_message("mutex is %p", sc->sigl2->getCbList()->mutex);
    g_mutex_lock (sc->sigl2->getCbList()->mutex);
    g_array_append_val (sc->sigl2->getCbList()->socket_read_cb_array, new_cb);
    g_mutex_unlock (sc->sigl2->getCbList()->mutex);
}

void disconnect_signal_socket_read (socketclient_t sockclient, gpointer
        callback)
{
    SocketClient *sc =
        reinterpret_cast<SocketClient*>(sockclient);
    gint i;

    if (sc->sigl2 == NULL)
      return;

    //sc->getSocket()->SignalReadPacket.disconnect(sc->sigl2);
    g_mutex_lock (sc->sigl2->getCbList()->mutex);
    for (i = 0; i < sc->sigl2->getCbList()->socket_read_cb_array->len; i++)
    {
        cb_info *cb;
        cb = &g_array_index
            (sc->sigl2->getCbList()->socket_read_cb_array, 
             cb_info, i);
        if (cb->callback == callback)
            g_array_remove_index (sc->sigl2->getCbList()->socket_read_cb_array,
                    i);
    }
    g_mutex_unlock (sc->sigl2->getCbList()->mutex);
}

void socketclient_send_packet(socketclient_t sockclient, const gchar *data,
        guint len)
{
    SocketClient *sc = 
        reinterpret_cast<SocketClient*>(sockclient);

    //g_message("sending from %p", sockclient);

    sc->getSocket()->Send(data, len);
}

/*
void socketclient_set_socket_read_cb(socketclient_t sockclient,
        socket_read_cb_type_ cb, gpointer data)
{
    SocketClient *sc = 
        reinterpret_cast<SocketClient*>(sockclient);

    sc->SetSocketReadCb(cb, data);
}*/
