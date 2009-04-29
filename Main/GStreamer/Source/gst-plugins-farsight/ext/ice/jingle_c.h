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

#ifndef JINGLE_C_H
#define JINGLE_C_H

#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif
    typedef void* socketclient_t;

    typedef void (*socket_read_cb_type) (gpointer creator, const gpointer data,
            guint len, const guint32 ip, const guint16 port);

    typedef struct {
        gpointer callback;
        gpointer data;
    } cb_info;

    typedef struct {
        GMutex *mutex;
        GArray *socket_read_cb_array;
    } callback_list;

    void connect_signal_socket_read (socketclient_t sockclient, gpointer
            callback, gpointer data);
    void disconnect_signal_socket_read (socketclient_t sockclient, gpointer
            callback);

    void socketclient_send_packet(socketclient_t sockclient, const gchar *data,
            guint len);
/*
    void socketclient_set_socket_read_cb(socketclient_t sockclient,
            socket_read_cb_type, gpointer data);
*/
#ifdef __cplusplus
}

#endif

#endif
