/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-nossl.c
 *
 * Copyright (C) 2003, Ximian, Inc.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "soup-ssl.h"
#include "soup-misc.h"

#ifndef HAVE_SSL

const gboolean soup_ssl_supported = FALSE;

GIOChannel *
soup_ssl_wrap_iochannel (GIOChannel *sock, gboolean non_blocking,
			 SoupSSLType type, const char *hostname,
			 SoupSSLCredentials *creds)
{
	return NULL;
}

SoupSSLCredentials *
soup_ssl_get_client_credentials (const char *ca_file)
{
	return NULL;
}

void
soup_ssl_free_client_credentials (SoupSSLCredentials *client_creds)
{
	;
}

SoupSSLCredentials *
soup_ssl_get_server_credentials (const char *cert_file, const char *key_file)
{
	return NULL;
}

void
soup_ssl_free_server_credentials (SoupSSLCredentials *server_creds)
{
	;
}

#endif /* ! HAVE_SSL */

/**
 * SOUP_SSL_ERROR:
 *
 * A #GError domain representing an SSL error. Used with #SoupSSLError.
 **/
/**
 * soup_ssl_error_quark:
 *
 * The quark used as %SOUP_SSL_ERROR
 *
 * Return value: The quark used as %SOUP_SSL_ERROR
 **/
GQuark
soup_ssl_error_quark (void)
{
	static GQuark error;
	if (!error)
		error = g_quark_from_static_string ("soup_ssl_error_quark");
	return error;
}

/**
 * SoupSSLError:
 * @SOUP_SSL_ERROR_HANDSHAKE_NEEDS_READ: Internal error. Never exposed
 * outside of libsoup.
 * @SOUP_SSL_ERROR_HANDSHAKE_NEEDS_WRITE: Internal error. Never exposed
 * outside of libsoup.
 * @SOUP_SSL_ERROR_CERTIFICATE: Indicates an error validating an SSL
 * certificate
 *
 * SSL-related I/O errors.
 **/
