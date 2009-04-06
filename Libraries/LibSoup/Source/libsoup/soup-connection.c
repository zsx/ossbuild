/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-connection.c: A single HTTP/HTTPS connection
 *
 * Copyright (C) 2000-2003, Ximian, Inc.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>

#include <fcntl.h>
#include <sys/types.h>

#include "soup-address.h"
#include "soup-connection.h"
#include "soup-marshal.h"
#include "soup-message.h"
#include "soup-message-private.h"
#include "soup-misc.h"
#include "soup-socket.h"
#include "soup-ssl.h"
#include "soup-uri.h"

typedef enum {
	SOUP_CONNECTION_MODE_DIRECT,
	SOUP_CONNECTION_MODE_PROXY,
	SOUP_CONNECTION_MODE_TUNNEL
} SoupConnectionMode;

typedef struct {
	SoupSocket  *socket;

	/* proxy_addr is the address of the proxy server we are
	 * connected to, if any. server_addr is the address of the
	 * origin server. conn_addr is the uri of the host we are
	 * actually directly connected to, which will be proxy_addr if
	 * there's a proxy and server_addr if not.
	 */
	SoupAddress *proxy_addr, *server_addr, *conn_addr;
	gpointer     ssl_creds;

	SoupConnectionMode  mode;

	GMainContext      *async_context;

	SoupMessage *cur_req;
	time_t       last_used;
	gboolean     connected, in_use;
	guint        io_timeout, idle_timeout;
	GSource     *idle_timeout_src;
} SoupConnectionPrivate;
#define SOUP_CONNECTION_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), SOUP_TYPE_CONNECTION, SoupConnectionPrivate))

G_DEFINE_TYPE (SoupConnection, soup_connection, G_TYPE_OBJECT)

enum {
	CONNECT_RESULT,
	DISCONNECTED,
	REQUEST_STARTED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

enum {
	PROP_0,

	PROP_SERVER_ADDRESS,
	PROP_PROXY_ADDRESS,
	PROP_SSL_CREDS,
	PROP_ASYNC_CONTEXT,
	PROP_TIMEOUT,
	PROP_IDLE_TIMEOUT,

	LAST_PROP
};

static void set_property (GObject *object, guint prop_id,
			  const GValue *value, GParamSpec *pspec);
static void get_property (GObject *object, guint prop_id,
			  GValue *value, GParamSpec *pspec);

static void stop_idle_timer (SoupConnectionPrivate *priv);
static void send_request (SoupConnection *conn, SoupMessage *req);
static void clear_current_request (SoupConnection *conn);

static void
soup_connection_init (SoupConnection *conn)
{
	;
}

static void
finalize (GObject *object)
{
	SoupConnectionPrivate *priv = SOUP_CONNECTION_GET_PRIVATE (object);

	if (priv->proxy_addr)
		g_object_unref (priv->proxy_addr);
	if (priv->server_addr)
		g_object_unref (priv->server_addr);

	if (priv->async_context)
		g_main_context_unref (priv->async_context);

	G_OBJECT_CLASS (soup_connection_parent_class)->finalize (object);
}

static void
dispose (GObject *object)
{
	SoupConnection *conn = SOUP_CONNECTION (object);
	SoupConnectionPrivate *priv = SOUP_CONNECTION_GET_PRIVATE (conn);

	stop_idle_timer (priv);
	/* Make sure clear_current_request doesn't re-establish the timeout */
	priv->idle_timeout = 0;

	clear_current_request (conn);
	soup_connection_disconnect (conn);

	G_OBJECT_CLASS (soup_connection_parent_class)->dispose (object);
}

static void
soup_connection_class_init (SoupConnectionClass *connection_class)
{
	GObjectClass *object_class = G_OBJECT_CLASS (connection_class);

	g_type_class_add_private (connection_class, sizeof (SoupConnectionPrivate));

	/* virtual method definition */
	connection_class->send_request = send_request;

	/* virtual method override */
	object_class->dispose = dispose;
	object_class->finalize = finalize;
	object_class->set_property = set_property;
	object_class->get_property = get_property;

	/* signals */

	signals[CONNECT_RESULT] =
		g_signal_new ("connect_result",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_FIRST,
			      G_STRUCT_OFFSET (SoupConnectionClass, connect_result),
			      NULL, NULL,
			      soup_marshal_NONE__INT,
			      G_TYPE_NONE, 1,
			      G_TYPE_INT);
	signals[DISCONNECTED] =
		g_signal_new ("disconnected",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_FIRST,
			      G_STRUCT_OFFSET (SoupConnectionClass, disconnected),
			      NULL, NULL,
			      soup_marshal_NONE__NONE,
			      G_TYPE_NONE, 0);
	signals[REQUEST_STARTED] =
		g_signal_new ("request-started",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_FIRST,
			      G_STRUCT_OFFSET (SoupConnectionClass, request_started),
			      NULL, NULL,
			      soup_marshal_NONE__OBJECT,
			      G_TYPE_NONE, 1,
			      SOUP_TYPE_MESSAGE);

	/* properties */
	g_object_class_install_property (
		object_class, PROP_SERVER_ADDRESS,
		g_param_spec_object (SOUP_CONNECTION_SERVER_ADDRESS,
				     "Server address",
				     "The address of the HTTP origin server for this connection",
				     SOUP_TYPE_ADDRESS,
				     G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
	g_object_class_install_property (
		object_class, PROP_PROXY_ADDRESS,
		g_param_spec_object (SOUP_CONNECTION_PROXY_ADDRESS,
				     "Proxy address",
				     "The address of the HTTP Proxy to use for this connection",
				     SOUP_TYPE_ADDRESS,
				     G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
	g_object_class_install_property (
		object_class, PROP_SSL_CREDS,
		g_param_spec_pointer (SOUP_CONNECTION_SSL_CREDENTIALS,
				      "SSL credentials",
				      "Opaque SSL credentials for this connection",
				      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
	g_object_class_install_property (
		object_class, PROP_ASYNC_CONTEXT,
		g_param_spec_pointer (SOUP_CONNECTION_ASYNC_CONTEXT,
				      "Async GMainContext",
				      "GMainContext to dispatch this connection's async I/O in",
				      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
	g_object_class_install_property (
		object_class, PROP_TIMEOUT,
		g_param_spec_uint (SOUP_CONNECTION_TIMEOUT,
				   "Timeout value",
				   "Value in seconds to timeout a blocking I/O",
				   0, G_MAXUINT, 0,
				   G_PARAM_READWRITE));
	g_object_class_install_property (
		object_class, PROP_IDLE_TIMEOUT,
		g_param_spec_uint (SOUP_CONNECTION_IDLE_TIMEOUT,
				   "Idle Timeout",
				   "Connection lifetime when idle",
				   0, G_MAXUINT, 0,
				   G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
}


/**
 * soup_connection_new:
 * @propname1: name of first property to set
 * @...: value of @propname1, followed by additional property/value pairs
 *
 * Creates an HTTP connection. There are three possibilities:
 *
 * If you set %SOUP_CONNECTION_SERVER_ADDRESS and not
 * %SOUP_CONNECTION_PROXY_ADDRESS, this will be a direct connection to
 * the indicated origin server.
 *
 * If you set %SOUP_CONNECTION_PROXY_ADDRESS and not
 * %SOUP_CONNECTION_SSL_CREDENTIALS, this will be a standard proxy
 * connection, which can be used for requests to multiple origin
 * servers.
 *
 * If you set %SOUP_CONNECTION_SERVER_ADDRESS,
 * %SOUP_CONNECTION_PROXY_ADDRESS, and
 * %SOUP_CONNECTION_SSL_CREDENTIALS, this will be a tunnel through the
 * proxy to the origin server.
 *
 * You must call soup_connection_connect_async() or
 * soup_connection_connect_sync() to connect it after creating it.
 *
 * Return value: the new connection (not yet ready for use).
 **/
SoupConnection *
soup_connection_new (const char *propname1, ...)
{
	SoupConnection *conn;
	va_list ap;

	va_start (ap, propname1);
	conn = (SoupConnection *)g_object_new_valist (SOUP_TYPE_CONNECTION,
						      propname1, ap);
	va_end (ap);

	return conn;
}

static void
set_property (GObject *object, guint prop_id,
	      const GValue *value, GParamSpec *pspec)
{
	SoupConnectionPrivate *priv = SOUP_CONNECTION_GET_PRIVATE (object);

	switch (prop_id) {
	case PROP_SERVER_ADDRESS:
		priv->server_addr = g_value_dup_object (value);
		goto changed_connection;

	case PROP_PROXY_ADDRESS:
		priv->proxy_addr = g_value_dup_object (value);
		goto changed_connection;

	case PROP_SSL_CREDS:
		priv->ssl_creds = g_value_get_pointer (value);
		goto changed_connection;

	changed_connection:
		if (priv->proxy_addr) {
			priv->conn_addr = priv->proxy_addr;
			if (priv->server_addr && priv->ssl_creds)
				priv->mode = SOUP_CONNECTION_MODE_TUNNEL;
			else
				priv->mode = SOUP_CONNECTION_MODE_PROXY;
		} else {
			priv->conn_addr = priv->server_addr;
			priv->mode = SOUP_CONNECTION_MODE_DIRECT;
		}
		break;

	case PROP_ASYNC_CONTEXT:
		priv->async_context = g_value_get_pointer (value);
		if (priv->async_context)
			g_main_context_ref (priv->async_context);
		break;
	case PROP_TIMEOUT:
		priv->io_timeout = g_value_get_uint (value);
		break;
	case PROP_IDLE_TIMEOUT:
		priv->idle_timeout = g_value_get_uint (value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
get_property (GObject *object, guint prop_id,
	      GValue *value, GParamSpec *pspec)
{
	SoupConnectionPrivate *priv = SOUP_CONNECTION_GET_PRIVATE (object);

	switch (prop_id) {
	case PROP_SERVER_ADDRESS:
		g_value_set_object (value, priv->server_addr);
		break;
	case PROP_PROXY_ADDRESS:
		g_value_set_object (value, priv->proxy_addr);
		break;
	case PROP_SSL_CREDS:
		g_value_set_pointer (value, priv->ssl_creds);
		break;
	case PROP_ASYNC_CONTEXT:
		g_value_set_pointer (value, priv->async_context ? g_main_context_ref (priv->async_context) : NULL);
		break;
	case PROP_TIMEOUT:
		g_value_set_uint (value, priv->io_timeout);
		break;
	case PROP_IDLE_TIMEOUT:
		g_value_set_uint (value, priv->idle_timeout);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static gboolean
idle_timeout (gpointer conn)
{
	soup_connection_disconnect (conn);
	return FALSE;
}

static void
start_idle_timer (SoupConnection *conn)
{
	SoupConnectionPrivate *priv = SOUP_CONNECTION_GET_PRIVATE (conn);

	if (priv->idle_timeout > 0 && !priv->idle_timeout_src) {
		priv->idle_timeout_src =
			soup_add_timeout (priv->async_context,
					  priv->idle_timeout * 1000,
					  idle_timeout, conn);
	}
}

static void
stop_idle_timer (SoupConnectionPrivate *priv)
{
	if (priv->idle_timeout_src) {
		g_source_destroy (priv->idle_timeout_src);
		priv->idle_timeout_src = NULL;
	}
}

static void
set_current_request (SoupConnectionPrivate *priv, SoupMessage *req)
{
	g_return_if_fail (priv->cur_req == NULL);

	stop_idle_timer (priv);

	soup_message_set_io_status (req, SOUP_MESSAGE_IO_STATUS_RUNNING);
	priv->cur_req = req;
	priv->in_use = TRUE;
	g_object_add_weak_pointer (G_OBJECT (req), (gpointer)&priv->cur_req);
}

static void
clear_current_request (SoupConnection *conn)
{
	SoupConnectionPrivate *priv = SOUP_CONNECTION_GET_PRIVATE (conn);

	priv->in_use = FALSE;
	start_idle_timer (conn);
	if (priv->cur_req) {
		SoupMessage *cur_req = priv->cur_req;

		g_object_remove_weak_pointer (G_OBJECT (priv->cur_req),
					      (gpointer)&priv->cur_req);
		priv->cur_req = NULL;

		if (!soup_message_is_keepalive (cur_req))
			soup_connection_disconnect (conn);
		else {
			priv->last_used = time (NULL);
			soup_message_io_stop (cur_req);
		}
	}
}

static void
socket_disconnected (SoupSocket *sock, gpointer conn)
{
	soup_connection_disconnect (conn);
}

static SoupMessage *
connect_message (SoupConnectionPrivate *priv)
{
	SoupURI *uri;
	SoupMessage *msg;

	uri = soup_uri_new (NULL);
	soup_uri_set_scheme (uri, SOUP_URI_SCHEME_HTTPS);
	soup_uri_set_host (uri, soup_address_get_name (priv->server_addr));
	soup_uri_set_port (uri, soup_address_get_port (priv->server_addr));
	soup_uri_set_path (uri, "");
	msg = soup_message_new_from_uri (SOUP_METHOD_CONNECT, uri);
	soup_uri_free (uri);

	return msg;
}

static void
tunnel_connect_finished (SoupMessage *msg, gpointer user_data)
{
	SoupConnection *conn = user_data;
	SoupConnectionPrivate *priv = SOUP_CONNECTION_GET_PRIVATE (conn);
	guint status = msg->status_code;

	clear_current_request (conn);

	if (SOUP_STATUS_IS_SUCCESSFUL (status) && priv->ssl_creds) {
		const char *server_name =
			soup_address_get_name (priv->server_addr);
		if (soup_socket_start_proxy_ssl (priv->socket, server_name,
						 NULL))
			priv->connected = TRUE;
		else
			status = SOUP_STATUS_SSL_FAILED;
	} else if (SOUP_STATUS_IS_REDIRECTION (status)) {
		/* Oops, the proxy thinks we're a web browser. */
		status = SOUP_STATUS_PROXY_AUTHENTICATION_REQUIRED;
	}

	if (priv->proxy_addr)
		status = soup_status_proxify (status);
	g_signal_emit (conn, signals[CONNECT_RESULT], 0, status);
	g_object_unref (msg);
}

static void
tunnel_connect_restarted (SoupMessage *msg, gpointer user_data)
{
	SoupConnection *conn = user_data;
	guint status = msg->status_code;

	/* We only allow one restart: if another one happens, treat
	 * it as "finished".
	 */
	g_signal_handlers_disconnect_by_func (msg, tunnel_connect_restarted, conn);
	g_signal_connect (msg, "restarted",
			  G_CALLBACK (tunnel_connect_finished), conn);

	if (status == SOUP_STATUS_PROXY_AUTHENTICATION_REQUIRED) {
		/* Our parent session has handled the authentication
		 * and attempted to restart the message.
		 */
		if (soup_message_is_keepalive (msg)) {
			/* Connection is still open, so just send the
			 * message again.
			 */
			soup_connection_send_request (conn, msg);
		} else {
			/* Tell the session to try again. */
			soup_message_set_status (msg, SOUP_STATUS_TRY_AGAIN);
			soup_message_finished (msg);
		}
	} else
		soup_message_finished (msg);
}

static void
socket_connect_result (SoupSocket *sock, guint status, gpointer user_data)
{
	SoupConnection *conn = user_data;
	SoupConnectionPrivate *priv = SOUP_CONNECTION_GET_PRIVATE (conn);

	if (!SOUP_STATUS_IS_SUCCESSFUL (status))
		goto done;

	if (priv->mode == SOUP_CONNECTION_MODE_DIRECT && priv->ssl_creds) {
		if (!soup_socket_start_ssl (sock, NULL)) {
			status = SOUP_STATUS_SSL_FAILED;
			goto done;
		}
	}

	if (priv->mode == SOUP_CONNECTION_MODE_TUNNEL) {
		SoupMessage *connect_msg = connect_message (priv);

		g_signal_connect (connect_msg, "restarted",
				  G_CALLBACK (tunnel_connect_restarted), conn);
		g_signal_connect (connect_msg, "finished",
				  G_CALLBACK (tunnel_connect_finished), conn);

		soup_connection_send_request (conn, connect_msg);
		return;
	}

	priv->connected = TRUE;
	start_idle_timer (conn);

 done:
	if (priv->proxy_addr)
		status = soup_status_proxify (status);
	g_signal_emit (conn, signals[CONNECT_RESULT], 0, status);
}

/* from soup-misc.c... will eventually go away */
guint soup_signal_connect_once  (gpointer instance, const char *detailed_signal,
				 GCallback c_handler, gpointer data);

static void
address_resolved (SoupAddress *addr, guint status, gpointer data)
{
	SoupConnection *conn = data;
	SoupConnectionPrivate *priv = SOUP_CONNECTION_GET_PRIVATE (conn);

	if (status != SOUP_STATUS_OK) {
		socket_connect_result (NULL, status, conn);
		return;
	}

	priv->socket =
		soup_socket_new (SOUP_SOCKET_REMOTE_ADDRESS, addr,
				 SOUP_SOCKET_SSL_CREDENTIALS, priv->ssl_creds,
				 SOUP_SOCKET_ASYNC_CONTEXT, priv->async_context,
				 NULL);
	soup_socket_connect_async (priv->socket, NULL,
				   socket_connect_result, conn);
	g_signal_connect (priv->socket, "disconnected",
			  G_CALLBACK (socket_disconnected), conn);
}

/**
 * soup_connection_connect_async:
 * @conn: the connection
 * @callback: callback to call when the connection succeeds or fails
 * @user_data: data for @callback
 *
 * Asynchronously connects @conn.
 **/
void
soup_connection_connect_async (SoupConnection *conn,
			       SoupConnectionCallback callback,
			       gpointer user_data)
{
	SoupConnectionPrivate *priv;

	g_return_if_fail (SOUP_IS_CONNECTION (conn));
	priv = SOUP_CONNECTION_GET_PRIVATE (conn);
	g_return_if_fail (priv->socket == NULL);

	if (callback) {
		soup_signal_connect_once (conn, "connect_result",
					  G_CALLBACK (callback), user_data);
	}

	soup_address_resolve_async (priv->conn_addr, priv->async_context, NULL,
				    address_resolved, conn);
}

/**
 * soup_connection_connect_sync:
 * @conn: the connection
 *
 * Synchronously connects @conn.
 *
 * Return value: the soup status
 **/
guint
soup_connection_connect_sync (SoupConnection *conn)
{
	SoupConnectionPrivate *priv;
	guint status;

	g_return_val_if_fail (SOUP_IS_CONNECTION (conn), SOUP_STATUS_MALFORMED);
	priv = SOUP_CONNECTION_GET_PRIVATE (conn);
	g_return_val_if_fail (priv->socket == NULL, SOUP_STATUS_MALFORMED);

	status = soup_address_resolve_sync (priv->conn_addr, NULL);
	if (!SOUP_STATUS_IS_SUCCESSFUL (status))
		goto fail;

	priv->socket =
		soup_socket_new (SOUP_SOCKET_REMOTE_ADDRESS, priv->conn_addr,
				 SOUP_SOCKET_SSL_CREDENTIALS, priv->ssl_creds,
				 SOUP_SOCKET_FLAG_NONBLOCKING, FALSE,
				 SOUP_SOCKET_TIMEOUT, priv->io_timeout,
				 NULL);

	status = soup_socket_connect_sync (priv->socket, NULL);

	if (!SOUP_STATUS_IS_SUCCESSFUL (status))
		goto fail;
		
	g_signal_connect (priv->socket, "disconnected",
			  G_CALLBACK (socket_disconnected), conn);

	if (priv->mode == SOUP_CONNECTION_MODE_DIRECT && priv->ssl_creds) {
		if (!soup_socket_start_ssl (priv->socket, NULL)) {
			status = SOUP_STATUS_SSL_FAILED;
			goto fail;
		}
	}

	if (priv->mode == SOUP_CONNECTION_MODE_TUNNEL) {
		SoupMessage *connect_msg = connect_message (priv);

		soup_connection_send_request (conn, connect_msg);
		status = connect_msg->status_code;

		if (status == SOUP_STATUS_PROXY_AUTHENTICATION_REQUIRED &&
		    SOUP_MESSAGE_IS_STARTING (connect_msg)) {
			if (soup_message_is_keepalive (connect_msg)) {
				/* Try once more */
				soup_connection_send_request (conn, connect_msg);
				status = connect_msg->status_code;
			} else
				status = SOUP_STATUS_TRY_AGAIN;
		}

		g_object_unref (connect_msg);

		if (SOUP_STATUS_IS_SUCCESSFUL (status)) {
			const char *server_name =
				soup_address_get_name (priv->server_addr);
			if (!soup_socket_start_proxy_ssl (priv->socket,
							  server_name,
							  NULL))
				status = SOUP_STATUS_SSL_FAILED;
		}
	}

	if (SOUP_STATUS_IS_SUCCESSFUL (status)) {
		priv->connected = TRUE;
		start_idle_timer (conn);
	} else {
	fail:
		if (priv->socket) {
			g_object_unref (priv->socket);
			priv->socket = NULL;
		}
	}

	if (priv->proxy_addr)
		status = soup_status_proxify (status);
	g_signal_emit (conn, signals[CONNECT_RESULT], 0, status);
	return status;
}


/**
 * soup_connection_disconnect:
 * @conn: a connection
 *
 * Disconnects @conn's socket and emits a %disconnected signal.
 * After calling this, @conn will be essentially useless.
 **/
void
soup_connection_disconnect (SoupConnection *conn)
{
	SoupConnectionPrivate *priv;

	g_return_if_fail (SOUP_IS_CONNECTION (conn));
	priv = SOUP_CONNECTION_GET_PRIVATE (conn);

	if (!priv->socket)
		return;

	g_signal_handlers_disconnect_by_func (priv->socket,
					      socket_disconnected, conn);
	soup_socket_disconnect (priv->socket);
	g_object_unref (priv->socket);
	priv->socket = NULL;

	/* Don't emit "disconnected" if we aren't yet connected */
	if (!priv->connected)
		return;

	priv->connected = FALSE;

	if (priv->cur_req &&
	    priv->cur_req->status_code == SOUP_STATUS_IO_ERROR &&
	    priv->last_used != 0) {
		/* There was a message queued on this connection, but
		 * the socket was closed while it was being sent.
		 * Since last_used is not 0, then that means at least
		 * one message was successfully sent on this
		 * connection before, and so the most likely cause of
		 * the IO_ERROR is that the connection was idle for
		 * too long and the server timed out and closed it
		 * (and we didn't notice until after we started
		 * sending the message). So we want the message to get
		 * tried again on a new connection. The only code path
		 * that could have gotten us to this point is through
		 * the call to io_cleanup() in
		 * soup_message_io_finished(), and so all we need to
		 * do to get the message requeued in this case is to
		 * change its status.
		 */
		soup_message_cleanup_response (priv->cur_req);
		soup_message_set_io_status (priv->cur_req,
					    SOUP_MESSAGE_IO_STATUS_QUEUED);
	}

	/* If cur_req is non-NULL but priv->last_used is 0, then that
	 * means this was the first message to be sent on this
	 * connection, and it failed, so the error probably means that
	 * there's some network or server problem, so we let the
	 * IO_ERROR be returned to the caller.
	 *
	 * (Of course, it's also possible that the error in the
	 * last_used != 0 case was because of a network/server problem
	 * too. It's even possible that the message crashed the
	 * server. In this case, requeuing it was the wrong thing to
	 * do, but presumably, the next attempt will also get an
	 * error, and eventually the message will be requeued onto a
	 * fresh connection and get an error, at which point the error
	 * will finally be returned to the caller.)
	 */

	/* NB: this might cause conn to be destroyed. */
	g_signal_emit (conn, signals[DISCONNECTED], 0);
}

SoupSocket *
soup_connection_get_socket (SoupConnection *conn)
{
	g_return_val_if_fail (SOUP_IS_CONNECTION (conn), NULL);

	return SOUP_CONNECTION_GET_PRIVATE (conn)->socket;
}

/**
 * soup_connection_is_in_use:
 * @conn: a connection
 *
 * Tests whether or not @conn is in use.
 *
 * Return value: %TRUE if there is currently a request being processed
 * on @conn.
 **/
gboolean
soup_connection_is_in_use (SoupConnection *conn)
{
	g_return_val_if_fail (SOUP_IS_CONNECTION (conn), FALSE);

	return SOUP_CONNECTION_GET_PRIVATE (conn)->in_use;
}

/**
 * soup_connection_last_used:
 * @conn: a #SoupConnection.
 *
 * Returns the last time a response was received on @conn.
 *
 * Return value: the last time a response was received on @conn, or 0
 * if @conn has not been used yet.
 */
time_t
soup_connection_last_used (SoupConnection *conn)
{
	g_return_val_if_fail (SOUP_IS_CONNECTION (conn), FALSE);

	return SOUP_CONNECTION_GET_PRIVATE (conn)->last_used;
}

static void
send_request (SoupConnection *conn, SoupMessage *req)
{
	SoupConnectionPrivate *priv = SOUP_CONNECTION_GET_PRIVATE (conn);

	if (req != priv->cur_req) {
		set_current_request (priv, req);
		g_signal_emit (conn, signals[REQUEST_STARTED], 0, req);
	}

	soup_message_send_request (req, priv->socket, conn,
				   priv->mode == SOUP_CONNECTION_MODE_PROXY);
}

/**
 * soup_connection_send_request:
 * @conn: a #SoupConnection
 * @req: a #SoupMessage
 *
 * Sends @req on @conn. This is a low-level function, intended for use
 * by #SoupSession.
 **/
void
soup_connection_send_request (SoupConnection *conn, SoupMessage *req)
{
	g_return_if_fail (SOUP_IS_CONNECTION (conn));
	g_return_if_fail (SOUP_IS_MESSAGE (req));
	g_return_if_fail (SOUP_CONNECTION_GET_PRIVATE (conn)->socket != NULL);

	SOUP_CONNECTION_GET_CLASS (conn)->send_request (conn, req);
}

/**
 * soup_connection_reserve:
 * @conn: a #SoupConnection
 *
 * Marks @conn as "in use" despite not actually having a message on
 * it. This is used by #SoupSession to keep it from accidentally
 * trying to queue two messages on the same connection from different
 * threads at the same time.
 **/
void
soup_connection_reserve (SoupConnection *conn)
{
	g_return_if_fail (SOUP_IS_CONNECTION (conn));

	SOUP_CONNECTION_GET_PRIVATE (conn)->in_use = TRUE;
}

/**
 * soup_connection_release:
 * @conn: a #SoupConnection
 *
 * Marks @conn as not "in use". This can be used to cancel the effect
 * of a soup_connection_reserve(). It is not necessary to call this
 * after soup_connection_send_request().
 **/
void
soup_connection_release (SoupConnection *conn)
{
	g_return_if_fail (SOUP_IS_CONNECTION (conn));

	clear_current_request (conn);
}
