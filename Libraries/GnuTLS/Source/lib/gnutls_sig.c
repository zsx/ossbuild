/*
 * Copyright (C) 2001, 2004, 2005, 2006, 2007 Free Software Foundation
 *
 * Author: Nikos Mavrogiannopoulos
 *
 * This file is part of GNUTLS.
 *
 * The GNUTLS library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
 * USA
 *
 */

#include <gnutls_int.h>
#include <gnutls_errors.h>
#include <x509_b64.h>
#include <auth_cert.h>
#include <gnutls_cert.h>
#include <libtasn1.h>
#include <gnutls_datum.h>
#include <gnutls_mpi.h>
#include <gnutls_global.h>
#include <gnutls_pk.h>
#include <debug.h>
#include <gnutls_buffers.h>
#include <gnutls_sig.h>
#include <gnutls_kx.h>

static int
_gnutls_tls_sign (gnutls_session_t session,
		  gnutls_cert * cert, gnutls_privkey * pkey,
		  const gnutls_datum_t * hash_concat,
		  gnutls_datum_t * signature);


/* Generates a signature of all the previous sent packets in the 
 * handshake procedure. (20040227: now it works for SSL 3.0 as well)
 */
int
_gnutls_tls_sign_hdata (gnutls_session_t session,
			gnutls_cert * cert, gnutls_privkey * pkey,
			gnutls_datum_t * signature)
{
  gnutls_datum_t dconcat;
  int ret;
  opaque concat[36];
  digest_hd_st td_md5;
  digest_hd_st td_sha;
  gnutls_protocol_t ver = gnutls_protocol_get_version (session);

  ret =
    _gnutls_hash_copy (&td_sha, &session->internals.handshake_mac_handle_sha);
  if (ret < 0)
    {
      gnutls_assert ();
      return ret;
    }

  if (ver == GNUTLS_SSL3)
    {
      ret = _gnutls_generate_master (session, 1);
      if (ret < 0)
	{
	  gnutls_assert ();
	  return ret;
	}

      _gnutls_mac_deinit_ssl3_handshake (&td_sha, &concat[16],
					 session->security_parameters.
					 master_secret, GNUTLS_MASTER_SIZE);
    }
  else
    _gnutls_hash_deinit (&td_sha, &concat[16]);

  switch (cert->subject_pk_algorithm)
    {
    case GNUTLS_PK_RSA:
      ret =
	_gnutls_hash_copy (&td_md5,
			   &session->internals.handshake_mac_handle_md5);
      if (ret < 0)
	{
	  gnutls_assert ();
	  return ret;
	}

      if (ver == GNUTLS_SSL3)
	_gnutls_mac_deinit_ssl3_handshake (&td_md5, concat,
					   session->security_parameters.
					   master_secret, GNUTLS_MASTER_SIZE);
      else
	_gnutls_hash_deinit (&td_md5, concat);

      dconcat.data = concat;
      dconcat.size = 36;
      break;
    case GNUTLS_PK_DSA:
      dconcat.data = &concat[16];
      dconcat.size = 20;
      break;

    default:
      gnutls_assert ();
      return GNUTLS_E_INTERNAL_ERROR;
    }
  ret = _gnutls_tls_sign (session, cert, pkey, &dconcat, signature);
  if (ret < 0)
    {
      gnutls_assert ();
    }

  return ret;
}


/* Generates a signature of all the random data and the parameters.
 * Used in DHE_* ciphersuites.
 */
int
_gnutls_tls_sign_params (gnutls_session_t session, gnutls_cert * cert,
			 gnutls_privkey * pkey, gnutls_datum_t * params,
			 gnutls_datum_t * signature)
{
  gnutls_datum_t dconcat;
  int ret;
  digest_hd_st td_sha;
  opaque concat[36];
  gnutls_protocol_t ver = gnutls_protocol_get_version (session);

  ret = _gnutls_hash_init (&td_sha, GNUTLS_MAC_SHA1);
  if (ret < 0)
    {
      gnutls_assert ();
      return ret;
    }

  _gnutls_hash (&td_sha, session->security_parameters.client_random,
		GNUTLS_RANDOM_SIZE);
  _gnutls_hash (&td_sha, session->security_parameters.server_random,
		GNUTLS_RANDOM_SIZE);
  _gnutls_hash (&td_sha, params->data, params->size);

  switch (cert->subject_pk_algorithm)
    {
    case GNUTLS_PK_RSA:
      if (ver < GNUTLS_TLS1_2)
	{
	  digest_hd_st td_md5;

	  ret = _gnutls_hash_init (&td_md5, GNUTLS_MAC_MD5);
	  if (ret < 0)
	    {
	      gnutls_assert ();
	      return ret;
	    }

	  _gnutls_hash (&td_md5, session->security_parameters.client_random,
			GNUTLS_RANDOM_SIZE);
	  _gnutls_hash (&td_md5, session->security_parameters.server_random,
			GNUTLS_RANDOM_SIZE);
	  _gnutls_hash (&td_md5, params->data, params->size);

	  _gnutls_hash_deinit (&td_md5, concat);
	  _gnutls_hash_deinit (&td_sha, &concat[16]);

	  dconcat.size = 36;
	}
      else
	{
#if 1
	  /* Use NULL parameters. */
	  memcpy (concat,
		  "\x30\x21\x30\x09\x06\x05\x2b\x0e\x03\x02\x1a\x05\x00\x04\x14",
		  15);
	  _gnutls_hash_deinit (&td_sha, &concat[15]);
	  dconcat.size = 35;
#else
	  /* No parameters field. */
	  memcpy (concat,
		  "\x30\x1f\x30\x07\x06\x05\x2b\x0e\x03\x02\x1a\x04\x14", 13);
	  _gnutls_hash_deinit (&td_sha, &concat[13]);
	  dconcat.size = 33;
#endif
	}
      dconcat.data = concat;
      break;
    case GNUTLS_PK_DSA:
      _gnutls_hash_deinit (&td_sha, concat);
      dconcat.data = concat;
      dconcat.size = 20;
      break;

    default:
      gnutls_assert ();
      _gnutls_hash_deinit (&td_sha, NULL);
      return GNUTLS_E_INTERNAL_ERROR;
    }
  ret = _gnutls_tls_sign (session, cert, pkey, &dconcat, signature);
  if (ret < 0)
    {
      gnutls_assert ();
    }

  return ret;

}


/* This will create a PKCS1 or DSA signature, using the given parameters, and the
 * given data. The output will be allocated and be put in signature.
 */
int
_gnutls_sign (gnutls_pk_algorithm_t algo, bigint_t * params,
	      int params_size, const gnutls_datum_t * data,
	      gnutls_datum_t * signature)
{
  int ret;

  switch (algo)
    {
    case GNUTLS_PK_RSA:
      /* encrypt */
      if ((ret = _gnutls_pkcs1_rsa_encrypt (signature, data, params,
					    params_size, 1)) < 0)
	{
	  gnutls_assert ();
	  return ret;
	}

      break;
    case GNUTLS_PK_DSA:
      /* sign */
      if ((ret = _gnutls_dsa_sign (signature, data, params, params_size)) < 0)
	{
	  gnutls_assert ();
	  return ret;
	}
      break;
    default:
      gnutls_assert ();
      return GNUTLS_E_INTERNAL_ERROR;
      break;
    }

  return 0;
}

/* This will create a PKCS1 or DSA signature, as defined in the TLS protocol.
 * Cert is the certificate of the corresponding private key. It is only checked if
 * it supports signing.
 */
static int
_gnutls_tls_sign (gnutls_session_t session,
		  gnutls_cert * cert, gnutls_privkey * pkey,
		  const gnutls_datum_t * hash_concat,
		  gnutls_datum_t * signature)
{

  /* If our certificate supports signing
   */

  if (cert != NULL)
    if (cert->key_usage != 0)
      if (!(cert->key_usage & KEY_DIGITAL_SIGNATURE))
	{
	  gnutls_assert ();
	  return GNUTLS_E_KEY_USAGE_VIOLATION;
	}

  /* External signing. */
  if (!pkey || pkey->params_size == 0)
    {
      if (!session->internals.sign_func)
	return GNUTLS_E_INSUFFICIENT_CREDENTIALS;

      return (*session->internals.sign_func)
	(session, session->internals.sign_func_userdata,
	 cert->cert_type, &cert->raw, hash_concat, signature);
    }

  return _gnutls_sign (pkey->pk_algorithm, pkey->params,
		       pkey->params_size, hash_concat, signature);
}

static int
_gnutls_verify_sig (gnutls_cert * cert,
		    const gnutls_datum_t * hash_concat,
		    gnutls_datum_t * signature, size_t sha1pos)
{
  int ret;
  gnutls_datum_t vdata;

  if (cert->version == 0 || cert == NULL)
    {				/* this is the only way to check
				 * if it is initialized
				 */
      gnutls_assert ();
      return GNUTLS_E_CERTIFICATE_ERROR;
    }

  /* If the certificate supports signing continue.
   */
  if (cert != NULL)
    if (cert->key_usage != 0)
      if (!(cert->key_usage & KEY_DIGITAL_SIGNATURE))
	{
	  gnutls_assert ();
	  return GNUTLS_E_KEY_USAGE_VIOLATION;
	}

  switch (cert->subject_pk_algorithm)
    {
    case GNUTLS_PK_RSA:

      vdata.data = hash_concat->data;
      vdata.size = hash_concat->size;

      /* verify signature */
      if ((ret = _gnutls_rsa_verify (&vdata, signature, cert->params,
				     cert->params_size, 1)) < 0)
	{
	  gnutls_assert ();
	  return ret;
	}

      break;
    case GNUTLS_PK_DSA:

      vdata.data = &hash_concat->data[sha1pos];
      vdata.size = 20;		/* sha1 */

      /* verify signature */
      if ((ret = _gnutls_dsa_verify (&vdata, signature, cert->params,
				     cert->params_size)) < 0)
	{
	  gnutls_assert ();
	  return ret;
	}

      break;

    default:
      gnutls_assert ();
      return GNUTLS_E_INTERNAL_ERROR;
    }



  return 0;
}


/* Verifies a TLS signature (like the one in the client certificate
 * verify message). 
 */
int
_gnutls_verify_sig_hdata (gnutls_session_t session, gnutls_cert * cert,
			  gnutls_datum_t * signature)
{
  int ret;
  opaque concat[36];
  digest_hd_st td_md5;
  digest_hd_st td_sha;
  gnutls_datum_t dconcat;
  gnutls_protocol_t ver = gnutls_protocol_get_version (session);

  ret =
    _gnutls_hash_copy (&td_md5, &session->internals.handshake_mac_handle_md5);
  if (ret < 0)
    {
      gnutls_assert ();
      return ret;
    }

  ret =
    _gnutls_hash_copy (&td_sha, &session->internals.handshake_mac_handle_sha);
  if (ret < 0)
    {
      gnutls_assert ();
      _gnutls_hash_deinit (&td_md5, NULL);
      return GNUTLS_E_HASH_FAILED;
    }

  if (ver == GNUTLS_SSL3)
    {
      ret = _gnutls_generate_master (session, 1);
      if (ret < 0)
	{
	  gnutls_assert ();
	  return ret;
	}

      _gnutls_mac_deinit_ssl3_handshake (&td_md5, concat,
					 session->security_parameters.
					 master_secret, GNUTLS_MASTER_SIZE);
      _gnutls_mac_deinit_ssl3_handshake (&td_sha, &concat[16],
					 session->security_parameters.
					 master_secret, GNUTLS_MASTER_SIZE);
    }
  else
    {
      _gnutls_hash_deinit (&td_md5, concat);
      _gnutls_hash_deinit (&td_sha, &concat[16]);
    }

  dconcat.data = concat;
  dconcat.size = 20 + 16;	/* md5+ sha */

  ret = _gnutls_verify_sig (cert, &dconcat, signature, 16);
  if (ret < 0)
    {
      gnutls_assert ();
      return ret;
    }

  return ret;

}

/* Generates a signature of all the random data and the parameters.
 * Used in DHE_* ciphersuites.
 */
int
_gnutls_verify_sig_params (gnutls_session_t session, gnutls_cert * cert,
			   const gnutls_datum_t * params,
			   gnutls_datum_t * signature)
{
  gnutls_datum_t dconcat;
  int ret;
  digest_hd_st td_md5;
  digest_hd_st td_sha;
  opaque concat[36];
  gnutls_protocol_t ver = gnutls_protocol_get_version (session);

  if (ver < GNUTLS_TLS1_2)
    {
      ret = _gnutls_hash_init (&td_md5, GNUTLS_MAC_MD5);
      if (ret < 0)
	{
	  gnutls_assert ();
	  return ret;
	}

      _gnutls_hash (&td_md5, session->security_parameters.client_random,
		    GNUTLS_RANDOM_SIZE);
      _gnutls_hash (&td_md5, session->security_parameters.server_random,
		    GNUTLS_RANDOM_SIZE);
      _gnutls_hash (&td_md5, params->data, params->size);
    }

  ret = _gnutls_hash_init (&td_sha, GNUTLS_MAC_SHA1);
  if (ret < 0)
    {
      gnutls_assert ();
      if (ver < GNUTLS_TLS1_2)
	_gnutls_hash_deinit (&td_md5, NULL);
      return ret;
    }

  _gnutls_hash (&td_sha, session->security_parameters.client_random,
		GNUTLS_RANDOM_SIZE);
  _gnutls_hash (&td_sha, session->security_parameters.server_random,
		GNUTLS_RANDOM_SIZE);
  _gnutls_hash (&td_sha, params->data, params->size);

  if (ver < GNUTLS_TLS1_2)
    {
      _gnutls_hash_deinit (&td_md5, concat);
      _gnutls_hash_deinit (&td_sha, &concat[16]);
      dconcat.size = 36;
    }
  else
    {
#if 1
      /* Use NULL parameters. */
      memcpy (concat,
	      "\x30\x21\x30\x09\x06\x05\x2b\x0e\x03\x02\x1a\x05\x00\x04\x14",
	      15);
      _gnutls_hash_deinit (&td_sha, &concat[15]);
      dconcat.size = 35;
#else
      /* No parameters field. */
      memcpy (concat,
	      "\x30\x1f\x30\x07\x06\x05\x2b\x0e\x03\x02\x1a\x04\x14", 13);
      _gnutls_hash_deinit (&td_sha, &concat[13]);
      dconcat.size = 33;
#endif
    }

  dconcat.data = concat;

  ret = _gnutls_verify_sig (cert, &dconcat, signature, dconcat.size - 20);
  if (ret < 0)
    {
      gnutls_assert ();
      return ret;
    }

  return ret;

}
