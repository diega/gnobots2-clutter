/*
 * File: tls.c
 * Author: Rich Gade
 * Project: GGZ Core Client Lib
 * Date: 10/21/02
 *
 * Routines to enable easysock to utilize TLS using gnutls
 *
 * Copyright (C) 2002 Brent Hendricks.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#ifdef GGZ_TLS_NONE

#include <unistd.h>
#ifdef HAVE_WINSOCK2_H
#  include <winsock2.h>
#endif

#include "ggz.h"

/*****************************
 * Empty versions of functions
 * to prevent linkage errors
 *****************************/

void ggz_tls_init(const char *certfile, const char *keyfile, const char *password)
{
}

int ggz_tls_support_query(void)
{
	return 0;
}

const char *ggz_tls_support_name(void)
{
	return NULL;
}

int ggz_tls_enable_fd(int fdes, GGZTLSType whoami, GGZTLSVerificationType verify)
{
	return 0;
}

int ggz_tls_disable_fd(int fdes)
{
	return 0;
}

size_t ggz_tls_write(int fd, void *ptr, size_t n)
{
#ifdef HAVE_WINSOCK2_H
	return send(fd, ptr, n, 0);
#else
	return write(fd, ptr, n);
#endif
}

size_t ggz_tls_read(int fd, void *ptr, size_t n)
{
#ifdef HAVE_WINSOCK2_H
	return recv(fd, ptr, n, 0);
#else
	return read(fd, ptr, n);
#endif
}

#endif

