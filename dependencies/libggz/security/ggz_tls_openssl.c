/*************************************************************************
GenSecure - Transport Layer Security functionality for the GGZ Gaming Zone
Copyright (C) 2001 - 2006 Josef Spillner, josef@ggzgamingzone.org
Published under GNU LGPL conditions - see COPYING for details
**************************************************************************/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#ifdef GGZ_TLS_OPENSSL

/* Include files */
#include <stdio.h>
#include <string.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <unistd.h>
#ifdef HAVE_WINSOCK2_H
#  include <winsock2.h>
#endif

#include "ggz.h"

/* The control structure for TLS */
static SSL_CTX *_tlsctx = NULL;
/* Flag to indicate a correct state flow */
int _state = 0;
/* The registered callback to decrypt the private key (may be NULL) */
static pem_password_cb *_callback = NULL;
/* Location of the certificate */
static char *_cert = NULL;
/* Location of the private key */
static char *_key = NULL;
/* Password */
static char *_password = NULL;

/* List entries */
struct list_entry
{
	SSL *tls;
	int fd;
	int active;
};
static GGZList *openssllist;

/* Not everything in OpenSSL is well documented... */
#define OPENSSLCHECK(x)

/* Output error information */
#define TLSERROR(x) tls_error(x, __FILE__, __LINE__)

/* Critical error: resets state */
static void tls_error(const char *error, const char *file, int line)
{
	printf("*** ERROR! ***\n");
	printf("(TLS: %s)\n", error);
	printf("(in %s, line %i)\n", file, line);
	printf("**************\n");
	_state = 0;
}

/* Display extended array information */
static char *tls_exterror(SSL *_tls, int ret)
{
	switch(SSL_get_error(_tls, ret))
	{
		case SSL_ERROR_NONE:
			return "SSL_ERROR_NONE";
			break;
		case SSL_ERROR_ZERO_RETURN:
			return "SSL_ERROR_ZERO_RETURN";
			break;
		case SSL_ERROR_WANT_READ:
			return "SSL_ERROR_WANT_READ";
			break;
		case SSL_ERROR_WANT_WRITE:
			return "SSL_ERROR_WANT_WRITE";
			break;
		case SSL_ERROR_WANT_X509_LOOKUP:
			return "SSL_ERROR_WANT_X509_LOOKUP";
			break;
		case SSL_ERROR_SYSCALL:
			return "SSL_ERROR_SYSCALL";
			break;
		case SSL_ERROR_SSL:
			return "SSL_ERROR_SSL";
			break;
	}
	return NULL;
}

/* Setup certificate arguments */
static void tls_prepare(const char *cert, const char *key, pem_password_cb *callback)
{
	_callback = callback;
	_cert = (char*)cert;
	_key = (char*)key;
}

/* Verification callback */
static int tls_verify(int preverify_ok, X509_STORE_CTX *ctx)
{
	printf("### VERIFY CALLBACK: %i ###\n", preverify_ok);
	preverify_ok = 1;
	return preverify_ok;
}

/* Simple password callback */
static int passwordcallback(char *buf, int size, int rwflag, void *userdata)
{
	if(!_password) return 0;
	strncpy(buf, _password, size);
	return strlen(_password);
}

/* Initialize the library */
static void tls_init(int verify)
{
	if(_tlsctx)
	{
		TLSERROR("Library is already initialized!");
		return;
	}

	SSL_load_error_strings();
	SSL_library_init();

	_tlsctx = SSL_CTX_new(TLSv1_method());
	if(!_tlsctx) TLSERROR("Couldn't create TLS object.\n");
	else
	{
		OPENSSLCHECK(SSL_CTX_set_quiet_shutdown(_tlsctx, 1));
		OPENSSLCHECK(SSL_CTX_set_info_callback(_tlsctx, NULL));
		OPENSSLCHECK(SSL_CTX_load_verify_locations(ctx, CAfile, CApath));
		OPENSSLCHECK(SSL_CTX_set_default_verify_paths());
		if(verify == GGZ_TLS_VERIFY_PEER) SSL_CTX_set_verify(_tlsctx, SSL_VERIFY_PEER, tls_verify);
		else SSL_CTX_set_verify(_tlsctx, SSL_VERIFY_NONE, NULL);
	}

	openssllist = ggz_list_create(NULL, NULL, NULL, 0);
}

/* Load certificate and private key */
static void tls_certkey(SSL *_tls)
{
	int ret, ret2;

	if(!_tls)
	{
		TLSERROR("Certificate cannot be loaded.");
		return;
	}

	if((!_key) || (!_cert) || (!_callback))
	{
		printf("WARNING: certificates are disabled!\n");
		return;
	}

	SSL_CTX_set_default_passwd_cb(_tlsctx, _callback);

	/*ret = SSL_CTX_load_verify_locations(_tlsctx, "/usr/lib/ssl/cacert.pem", NULL);
	if(ret != 1)
	{
		TLSERROR("Couldn't load root CA file!");
		ret2 = ERR_get_error();
		printf("EXT: %s\n%s\n%s\n%s\n%s\n", tls_exterror(_tls, ret), ERR_error_string(ret2, NULL),
		ERR_lib_error_string(ret2), ERR_func_error_string(ret2), ERR_reason_error_string(ret2));
	}*/

	ret = SSL_use_RSAPrivateKey_file(_tls, _key, SSL_FILETYPE_PEM);
	if(ret != 1)
	{
		TLSERROR("Error loading TLS PEM private key.");
		ret2 = ERR_get_error();
		printf("EXT: %s\n%s\n%s\n%s\n%s\n", tls_exterror(_tls, ret), ERR_error_string(ret2, NULL),
		ERR_lib_error_string(ret2), ERR_func_error_string(ret2), ERR_reason_error_string(ret2));
	}
	ret = SSL_use_certificate_file(_tls, _cert, SSL_FILETYPE_PEM);
	if(ret != 1) TLSERROR("Error loading TLS PEM certificate.");
	ret = SSL_check_private_key(_tls);
	if(!ret) TLSERROR("Private key doesn't match certificate public key.");
	printf("*** certificate loaded ***\n");
}

/* Switch on a filedescriptor */
int ggz_tls_enable_fd(int fd, GGZTLSType mode, GGZTLSVerificationType verify)
{
	int ret, ret2;
	STACK_OF(SSL_CIPHER) *stack;
	SSL_CIPHER *cipher;
	int bits;
	char *cipherlist;
	SSL *_tls;
	int _tls_active;
	struct list_entry *entry;

	_state = 1;
	_tls_active = 0;
	if((mode != GGZ_TLS_CLIENT) && (mode != GGZ_TLS_SERVER))
	{
		TLSERROR("Wrong mode.");
		return 0;
	}

	if(!_tlsctx) tls_init(verify);
		
	_tls = SSL_new(_tlsctx);
	if(_tls)
	{
		cipherlist = NULL;
		stack = SSL_get_ciphers(_tls);
		while((cipher = (SSL_CIPHER*)sk_pop(stack)) != NULL)
		{
			printf("* Cipher: %s\n", SSL_CIPHER_get_name(cipher));
			printf("  Bits: %i\n", SSL_CIPHER_get_bits(cipher, &bits));
			printf("  Used bits: %i\n", bits);
			printf("  Version: %s\n", SSL_CIPHER_get_version(cipher));
			printf("  Description: %s\n", SSL_CIPHER_description(cipher, NULL, 0));
			if(cipherlist)
			{
				cipherlist = (char*)realloc(cipherlist, (strlen(cipherlist) + 1) + strlen(SSL_CIPHER_get_name(cipher)) + 1);
				strcat(cipherlist, ":");
				strcat(cipherlist, SSL_CIPHER_get_name(cipher));
			}
			else
			{
				cipherlist = (char*)malloc(strlen(SSL_CIPHER_get_name(cipher)) + 1);
				strcpy(cipherlist, SSL_CIPHER_get_name(cipher));
			}
		}
		printf("Available ciphers: %s\n", cipherlist);
		ret = SSL_set_cipher_list(_tls, cipherlist);
		if(!ret) TLSERROR("Cipher selection failed.");
		ret = SSL_set_fd(_tls, fd);
		if(!ret) TLSERROR("Assignment to connection failed.");
		else
		{
			SSL_set_read_ahead(_tls, 1);
			if(mode == GGZ_TLS_SERVER)
			{
				tls_certkey(_tls);
				if(_state)
				{
					SSL_set_accept_state(_tls);
					ret = SSL_accept(_tls);
				}
			}
			else
			{
				SSL_set_connect_state(_tls);
				ret = SSL_connect(_tls);
			}
			if((ret != 1) || (!_state))
			{
				printf("Ret: %i, State: %i\n", ret, _state);
				TLSERROR("Handshake failed.");
				ret2 = ERR_get_error();
				printf("EXT: %s\n%s\n%s\n%s\n%s\n", tls_exterror(_tls, ret), ERR_error_string(ret2, NULL),
					ERR_lib_error_string(ret2), ERR_func_error_string(ret2), ERR_reason_error_string(ret2));
			}
			else
			{
				printf(">>>>> Handshake successful.\n");
				if((mode == GGZ_TLS_SERVER) || (verify == GGZ_TLS_VERIFY_NONE)) _tls_active = 1;
				else
				{
					printf(">>>>> Client side, thus checking Certificate.\n");
					printf("Negotiated cipher: %s\n", SSL_get_cipher(_tls));
					printf("Shared ciphers: %s\n", SSL_get_shared_ciphers(_tls, NULL, 0));
					if(SSL_get_peer_certificate(_tls))
					{
						if(SSL_get_verify_result(_tls) == X509_V_OK)
						{
							_tls_active = 1;
						}
						else
						{
							printf("Error code: %li\n", SSL_get_verify_result(_tls));
							TLSERROR("Invalid certificate, or certificate is not self-signed.");
						}
					}
					else TLSERROR("Couldn't get certificate.");
				}
			}
			entry = (struct list_entry*)ggz_malloc(sizeof(struct list_entry));
			entry->tls = _tls;
			entry->fd = fd;
			entry->active = _tls_active;
			ggz_list_insert(openssllist, &entry);
			return 1;
		}
	}
	return 0;
}

/* Compare function */
static int list_entry_compare(void *a, void *b)
{
	struct list_entry x, y;
	x = *(struct list_entry*)a;
	y = *(struct list_entry*)b;
	return (x.fd == y.fd ? 0 : 1);
}

/* Read from a file descriptor */
size_t ggz_tls_read(int fd, void *buffer, size_t size)
{
	SSL *handler;
	struct list_entry *entry;
	struct list_entry entry2;
	int ret;

	entry2.fd = fd;
	entry = (struct list_entry*)ggz_list_search_alt(openssllist, &entry2, list_entry_compare);

	if(!entry)
	{
		/*TLSERROR("Given fd is not secure.");*/
#ifdef HAVE_WINSOCK2_H
		return recv(fd, buffer, size, 0);
#else
		return read(fd, buffer, size);
#endif
	}
	handler = entry->tls;
	ret = SSL_read(handler, buffer, size);
	if(ret <= 0)
	{
		switch(SSL_get_error(handler, ret))
		{
			case SSL_ERROR_WANT_READ:
			case SSL_ERROR_WANT_WRITE:
				break;
			case SSL_ERROR_SYSCALL:
				ret = ERR_get_error();
				if(!ret)
				{
					printf("Protocol violation: EOF\n");
				}
				else
				{
					printf("Unix IO error: %i\n", errno);
				}
				break;
			default:
				/*printf("SSL read error (%i) on fd %i!\n", ret, fd);
				printf("SSL: %s\n", tls_exterror(handler, ret));*/
				break;
		}
	}
	return ret;
}

/* Write to a file descriptor */
size_t ggz_tls_write(int fd, void *s, size_t size)
{
	struct list_entry *entry;
	struct list_entry entry2;
	SSL *handler;
	int ret;

	entry2.fd = fd;
	entry = (struct list_entry*)ggz_list_search_alt(openssllist, &entry2, list_entry_compare);

	if(!entry)
	{
		/*TLSERROR("Given fd is not secure.");*/
#ifdef HAVE_WINSOCK2_H
		return send(fd, s, size, 0);
#else
		return write(fd, s, size);
#endif
	}
	handler = entry->tls;
	ret = SSL_write(handler, s, size);
	if(ret <= 0)
	{
		switch(SSL_get_error(handler, ret))
		{
			case SSL_ERROR_WANT_READ:
			case SSL_ERROR_WANT_WRITE:
				break;
			case SSL_ERROR_SYSCALL:
				ret = ERR_get_error();
				if(!ret)
				{
					printf("Protocol violation: EOF\n");
				}
				else
				{
					printf("Unix IO error: %i\n", errno);
				}
				break;
			default:
				/*printf("SSL write error (%i) on fd %i!\n", ret, fd);
				printf("SSL: %s\n", tls_exterror(handler, ret));*/
				break;
		}
	}
	return ret;
}

/* If an fd is active, deactivate it */
int ggz_tls_disable_fd(int fd)
{
	struct list_entry *entry;
	struct list_entry entry2;
	SSL *handler;

	entry2.fd = fd;
	entry = (struct list_entry*)ggz_list_search_alt(openssllist, &entry2, list_entry_compare);

	if(entry)
	{
		handler = entry->tls;
		SSL_shutdown(handler);
		SSL_free(handler);
		ggz_list_delete_entry(openssllist, (GGZListEntry*)entry);
		return 1;
	}
	return 0;
}

/* Returns a value which indicates that this implementation supports TLS */
int ggz_tls_support_query()
{
	return 1;
}

const char *ggz_tls_support_name(void)
{
	return "openssl";
}

/* Initializes certification values */
void ggz_tls_init(const char *certfile, const char *keyfile, const char *password)
{
	tls_prepare(certfile, keyfile, passwordcallback);
	_password = ggz_strdup(password);
}

#endif

