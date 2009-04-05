/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2008 Red Hat, Inc.
 */

#ifndef SOUP_COOKIE_JAR_H
#define SOUP_COOKIE_JAR_H 1

#include <libsoup/soup-types.h>

G_BEGIN_DECLS

#define SOUP_TYPE_COOKIE_JAR            (soup_cookie_jar_get_type ())
#define SOUP_COOKIE_JAR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SOUP_TYPE_COOKIE_JAR, SoupCookieJar))
#define SOUP_COOKIE_JAR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SOUP_TYPE_COOKIE_JAR, SoupCookieJarClass))
#define SOUP_IS_COOKIE_JAR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SOUP_TYPE_COOKIE_JAR))
#define SOUP_IS_COOKIE_JAR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((obj), SOUP_TYPE_COOKIE_JAR))
#define SOUP_COOKIE_JAR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), SOUP_TYPE_COOKIE_JAR, SoupCookieJarClass))

typedef struct {
	GObject parent;

} SoupCookieJar;

typedef struct {
	GObjectClass parent_class;

	void (*save)    (SoupCookieJar *jar);

	/* signals */
	void (*changed) (SoupCookieJar *jar,
			 SoupCookie    *old_cookie,
			 SoupCookie    *new_cookie);

	/* Padding for future expansion */
	void (*_libsoup_reserved1) (void);
	void (*_libsoup_reserved2) (void);
	void (*_libsoup_reserved3) (void);
} SoupCookieJarClass;

#define SOUP_COOKIE_JAR_READ_ONLY "read-only"

GType          soup_cookie_jar_get_type      (void);

SoupCookieJar *soup_cookie_jar_new           (void);

#ifndef LIBSOUP_DISABLE_DEPRECATED
void           soup_cookie_jar_save          (SoupCookieJar *jar);
#endif

char          *soup_cookie_jar_get_cookies   (SoupCookieJar *jar,
					      SoupURI       *uri,
					      gboolean       for_http);
void           soup_cookie_jar_set_cookie    (SoupCookieJar *jar,
					      SoupURI       *uri,
					      const char    *cookie);

void           soup_cookie_jar_add_cookie    (SoupCookieJar *jar,
					      SoupCookie    *cookie);
void           soup_cookie_jar_delete_cookie (SoupCookieJar *jar,
					      SoupCookie    *cookie);

GSList        *soup_cookie_jar_all_cookies   (SoupCookieJar *jar);


G_END_DECLS

#endif /* SOUP_COOKIE_JAR_H */
