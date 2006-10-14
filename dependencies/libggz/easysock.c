/*
 * File: easysock.c
 * Author: Brent Hendricks
 * Project: libeasysock
 * Date: 4/16/98
 * $Id$
 *
 * A library of useful routines to make life easier while using 
 * sockets
 *
 * Copyright (C) 1998-2001 Brent Hendricks.
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

/*
 * actually used definitions:
HAVE_WINSOCK2_H
GGZ_HAVE_PF_LOCAL
SUN_LEN (local substitute)
  -> also in support.h!, therefore deleted
GGZ_HAVE_SENDMSG
HAVE_MSGHDR_MSG_CONTROL
  HAVE_CMSG_* -> support.h
  CMSG_* -> support.h
DEBUG_MEM
*/

#include "config.h"

#ifdef GAI_A
#define _GNU_SOURCE
#endif

#ifdef DEBUG_MEM
#include <dmalloc.h>
#endif

#include <sys/types.h>
#include <sys/param.h>

#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>

#include "ggz.h"

#include "support.h"

#define SA struct sockaddr  
#define HOSTNAMELEN 1024

static ggzIOError _err_func = NULL;
static ggzIOExit _exit_func = exit;
static ggzNetworkNotify _notify_func = NULL;
static unsigned int ggz_alloc_limit = 32768;
static unsigned int ggz_socketcreation = 0;
static unsigned int ggz_socketport = 0;


int ggz_set_io_error_func(ggzIOError func)
{
	_err_func = func;
	return 0;
}


ggzIOError ggz_remove_io_error_func(void)
{
	ggzIOError old = _err_func;
	_err_func = NULL;
	return old;
}


int ggz_set_io_exit_func(ggzIOExit func)
{
	_exit_func = func;
	return 0;
}


ggzIOExit ggz_remove_io_exit_func(void)
{
	ggzIOExit old = _exit_func;
	_exit_func = exit;
	return old;
}


unsigned int ggz_get_io_alloc_limit(void)
{
	return ggz_alloc_limit;
}


unsigned int ggz_set_io_alloc_limit(const unsigned int limit)
{
	unsigned int old = ggz_alloc_limit;
	ggz_alloc_limit = limit;
	return old;
}


static void ggz_network_shutdown(void)
{
#ifdef HAVE_WINSOCK2_H
	WSACleanup();
#endif
}


/* Do network initialization. */
int ggz_init_network(void)
{
	static int initialized = 0;

	if (!initialized) {

#ifdef HAVE_WINSOCK2_H
		WSADATA wsa;
		if (WSAStartup(MAKEWORD(2, 2), &wsa) !=0 ){
			return -1;
		}
#endif

		initialized = 1;

		atexit(ggz_network_shutdown);
	}

	return 0;
}


static int es_bind(const char *host, int port)
{
	int sockfd;
	const int on = 1;
	/* FIXME: better test on getaddrinfo() directly */
	int n;
	struct addrinfo hints, *res, *ressave;
	char serv[30];
#ifdef HAVE_WINSOCK2_H
	struct sockaddr_in name;

	sockfd = socket(PF_INET, SOCK_STREAM, 0);

	if (sockfd < 0) {
		if (_err_func) {
			const char *msg = "could-not-create-socket-error";
			(*_err_func) (msg, GGZ_IO_CREATE, 0, GGZ_DATA_NONE);
		}
		return -1;
	}

	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (void*)&on, sizeof(on));

	memset(&name, 0, sizeof(name));
	name.sin_family = AF_INET;
	name.sin_port = htons(port);
	name.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(sockfd, &name, sizeof(name)) != 0) {
		if (_err_func) {
			const char *msg = strerror(errno);
			(*_err_func) (msg, GGZ_IO_CREATE, 0, GGZ_DATA_NONE);
		}
		close(sockfd);
		sockfd = -1;
	}
#else

	snprintf(serv, sizeof(serv), "%d", (unsigned int)port);

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((n = getaddrinfo(host, serv, &hints, &res)) != 0) {
		if (_err_func) {
			const char *msg = gai_strerror(n);
			(*_err_func) (msg, GGZ_IO_CREATE, 0, GGZ_DATA_NONE);
		}
		return -1;
	}

	ressave = res;

	do {
		sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (sockfd < 0)
			continue;

		setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (void*)&on, sizeof(on));

		if (bind(sockfd, res->ai_addr, res->ai_addrlen) == 0)
			break;

		close(sockfd);
		sockfd = -1;

	} while ( (res = res->ai_next) != NULL);

	freeaddrinfo(ressave);
#endif

	return(sockfd);
}


static int es_connect(const char *host, int port)
{
	int sockfd;
#ifdef HAVE_WINSOCK2_H
	struct hostent *h;
	struct sockaddr_in name;

	sockfd = socket(PF_INET, SOCK_STREAM, 0);

	if (sockfd < 0) {
		if (_err_func) {
			const char *msg = "could-not-create-socket-error";
			(*_err_func) (msg, GGZ_IO_CREATE, 0, GGZ_DATA_NONE);
		}
		return -1;
	}

	h = gethostbyname(host);
	if (!h) {
		if (_err_func) {
			const char *msg = "could-not-create-socket-error";
			(*_err_func) (msg, GGZ_IO_CREATE, 0, GGZ_DATA_NONE);
		}
		return -1;
	}

	memset(&name, 0, sizeof(name));
	name.sin_family = AF_INET;
	name.sin_port = htons(port);
	memcpy(&name.sin_addr, h->h_addr, h->h_length);

	if (connect(sockfd, &name, sizeof(name)) != 0) {
		if (_err_func) {
			const char *msg = strerror(errno);
			(*_err_func) (msg, GGZ_IO_CREATE, 0, GGZ_DATA_NONE);
		}
		close(sockfd);
		sockfd = -1;
	}
#else
	int n;
	struct addrinfo hints, *res, *ressave;
	char serv[30];

	snprintf(serv, sizeof(serv), "%d", (unsigned int)port);

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((n = getaddrinfo(host, serv, &hints, &res)) != 0) {
		if (_err_func) {
			const char *msg = gai_strerror(n);

			(*_err_func) (msg, GGZ_IO_CREATE, 0, GGZ_DATA_NONE);
		}
		return -1;
	}

	ressave = res;

	do {
		sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (sockfd < 0)
			continue;
		if (connect(sockfd, res->ai_addr, res->ai_addrlen) == 0)
			break;

		close(sockfd);
		sockfd = -1;

	} while ( (res = res->ai_next) != NULL);

	freeaddrinfo(ressave);
#endif

	return(sockfd);
}


int ggz_make_socket(const GGZSockType type, const unsigned short port, 
		   const char *server)
{
	int sock = -1;

	if (ggz_init_network() < 0) { /* Just in case. */
		return -1;
	}

	switch (type) {

	case GGZ_SOCK_SERVER:
		if ( (sock = es_bind(server, port)) < 0) {
			if (_err_func)
				(*_err_func) (strerror(errno), GGZ_IO_CREATE, 
					      sock, GGZ_DATA_NONE);
			return -1;
		}
		break;

	case GGZ_SOCK_CLIENT:
		if ((_notify_func) && (!ggz_socketcreation)) {
			ggz_socketcreation = 1;
			ggz_socketport = port;
			ggz_resolvename(server);
			return -3;
		}
		if ( (sock = es_connect(server, port)) < 0) {
			if (_err_func)
				(*_err_func) (strerror(errno), GGZ_IO_CREATE, 
					      sock, GGZ_DATA_NONE);
			return -1;
		}
		break;
	}

	return sock;
}


int ggz_make_socket_or_die(const GGZSockType type, const unsigned short port, 
			  const char *server)
{
	int sock;
	
	if ( (sock = ggz_make_socket(type, port, server)) < 0)
		(*_exit_func) (-1);
	
	return sock;
}

int ggz_make_unix_socket(const GGZSockType type, const char* name)
{
#if GGZ_HAVE_PF_LOCAL
	int sock;
	struct sockaddr_un addr;

	ggz_init_network(); /* Just in case. */
	
	if ( (sock = socket(PF_LOCAL, SOCK_STREAM, 0)) < 0) {
		if (_err_func)
			(*_err_func) (strerror(errno), GGZ_IO_CREATE, -1, GGZ_DATA_NONE);
		return -1;
	}

	
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_LOCAL;
	/* Copy in filename up to the limit, leaving room for \0 term. */
	strncpy(addr.sun_path, name, (sizeof(addr.sun_path) - 1));

	switch(type) {

	case GGZ_SOCK_SERVER:
		unlink(name);
		if (bind(sock, (SA *)&addr, SUN_LEN(&addr)) < 0 ) {
			if (_err_func)
				(*_err_func) (strerror(errno), GGZ_IO_CREATE, 
					      sock, GGZ_DATA_NONE);
			return -1;
		}
		break;
	case GGZ_SOCK_CLIENT:
		if (connect(sock, (SA *)&addr, sizeof(addr)) < 0) {
			if (_err_func)
				(*_err_func) (strerror(errno), GGZ_IO_CREATE, 
					      sock, GGZ_DATA_NONE);
			return -1;
		}
		break;
	}
	return sock;
#else
	return -1;
#endif
}


int ggz_make_unix_socket_or_die(const GGZSockType type, const char* name) 
{
#if GGZ_HAVE_PF_LOCAL
	int sock;
	
	if ( (sock = ggz_make_unix_socket(type, name)) < 0)
		(*_exit_func) (-1);
	
	return sock;
#else
	return -1;
#endif
}


int ggz_write_char(const int sock, const char message)
{
	if (ggz_writen(sock, &message, sizeof(char)) < 0) {
		ggz_debug(GGZ_SOCKET_DEBUG, "Error sending char.");
		if (_err_func)
			(*_err_func) (strerror(errno), GGZ_IO_WRITE, sock, GGZ_DATA_CHAR);
		return -1;
	}

	ggz_debug(GGZ_SOCKET_DEBUG, "Sent \"%d\" : char.", message);
	return 0;
}


void ggz_write_char_or_die(const int sock, const char data)
{
	if (ggz_write_char(sock, data) < 0)
		(*_exit_func) (-1);
}


int ggz_read_char(const int sock, char *message)
{
	int status;

	if ( (status = ggz_readn(sock, message, sizeof(char))) < 0) {
		ggz_debug(GGZ_SOCKET_DEBUG, "Error receiving char: %s",
			  strerror(errno));
		if (_err_func)
			(*_err_func) (strerror(errno), GGZ_IO_READ, sock, GGZ_DATA_CHAR);
		return -1;
	}

	if ((unsigned)status < sizeof(char)) {
		ggz_debug(GGZ_SOCKET_DEBUG, "Warning: fd is closed.");
		if (_err_func)
			(*_err_func) ("fd closed", GGZ_IO_READ, sock, GGZ_DATA_CHAR);
		return -1;
	}
	
	ggz_debug(GGZ_SOCKET_DEBUG, "Received \"%d\" : char.", *message);
	return 0;
}


void ggz_read_char_or_die(const int sock, char *data)
{
	if (ggz_read_char(sock, data) < 0)
		(*_exit_func) (-1);
}


/*
 * Take a host-byte order int and write on fd using
 * network-byte order.
 */
int ggz_write_int(const int sock, const int message)
{
	int data = htonl(message);

	if (ggz_writen(sock, &data, sizeof(int)) < 0) {
		ggz_debug(GGZ_SOCKET_DEBUG, "Error sending int.");
		if (_err_func)
			(*_err_func) (strerror(errno), GGZ_IO_WRITE, sock, GGZ_DATA_INT);
		return -1;
	}

	ggz_debug(GGZ_SOCKET_DEBUG, "Sent \"%d\" : int.", message);
	return 0;
}


void ggz_write_int_or_die(const int sock, const int data)
{
	if (ggz_write_int(sock, data) < 0)
		(*_exit_func) (-1);
}


/*
 * Read a network byte-order integer from the fd and
 * store it in host-byte order.
 */
int ggz_read_int(const int sock, int *message)
{
	int data, status;
	
	if ( (status = ggz_readn(sock, &data, sizeof(int))) < 0) {
		ggz_debug(GGZ_SOCKET_DEBUG, "Error receiving int: %s",
			  strerror(errno));
		if (_err_func)
			(*_err_func) (strerror(errno), GGZ_IO_READ, sock, GGZ_DATA_INT);
		return -1;
	}
	
	if ((unsigned)status < sizeof(int)) {
		ggz_debug(GGZ_SOCKET_DEBUG, "Warning: fd is closed.");
		if (_err_func)
			(*_err_func) ("fd closed", GGZ_IO_READ, sock, GGZ_DATA_INT);
		return -1;
	}
	
	*message = ntohl(data);
	ggz_debug(GGZ_SOCKET_DEBUG, "Received \"%d\" : int.", *message);
	return 0;
}


void ggz_read_int_or_die(const int sock, int *data)
{
	if (ggz_read_int(sock, data) < 0)
		(*_exit_func) (-1);
}


/*
 * Write a char string to the given fd preceeded by its size
 */
int ggz_write_string(const int sock, const char *message)
{
	unsigned int size = strlen(message) * sizeof(char) + 1;
	
	if (ggz_write_int(sock, size) < 0)
		return -1;
	
	if (ggz_writen(sock, message, size) < 0) {
		ggz_debug(GGZ_SOCKET_DEBUG, "Error sending string.");
		if (_err_func)
			(*_err_func) (strerror(errno), GGZ_IO_WRITE, sock, GGZ_DATA_STRING);
		return -1;
	}
	
	ggz_debug(GGZ_SOCKET_DEBUG, "Sent \"%s\" : string.", message);
	return 0;
}


int ggz_va_write_string(const int sock, const char *fmt, ...)
{
	char buf[4096];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	return ggz_write_string(sock, buf);
}


void ggz_write_string_or_die(const int sock, const char *data)
{
	if (ggz_write_string(sock, data) < 0)
		(*_exit_func) (-1);
}


void ggz_va_write_string_or_die(const int sock, const char *fmt, ...)
{
	char buf[4096];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	ggz_write_string_or_die(sock, buf);
}


/*
 * Read a char string from the given fd
 */
int ggz_read_string(const int sock, char *message, const unsigned int len)
{
	unsigned int size;
	int status;

	if (ggz_read_int(sock, (int*)&size) < 0)
		return -1;
	
	if (size > len) {
		ggz_debug(GGZ_SOCKET_DEBUG, "String too long for buffer.");
		if (_err_func)
			(*_err_func) ("String too long", GGZ_IO_READ, sock, GGZ_DATA_STRING);
		return -1;
	}
	
       	if ( (status = ggz_readn(sock, message, size)) < 0) {
		ggz_debug(GGZ_SOCKET_DEBUG, "Error receiving string: %s",
			  strerror(errno));
		if (_err_func)
			(*_err_func) (strerror(errno), GGZ_IO_READ, sock, GGZ_DATA_STRING);
		return -1;
	}

	if ((unsigned)status < size) {
		ggz_debug(GGZ_SOCKET_DEBUG, "Warning: fd is closed.");
		if (_err_func)
			(*_err_func) ("fd closed", GGZ_IO_READ, sock, GGZ_DATA_STRING);
		return -1;
	}
	
	/* Guarantee NULL-termination */
	message[len-1] = '\0';
	
	ggz_debug(GGZ_SOCKET_DEBUG, "Received \"%s\" : string.", message);
	return 0;
}


void ggz_read_string_or_die(const int sock, char *data, const unsigned int len)
{
	if (ggz_read_string(sock, data, len) < 0)
		(*_exit_func) (-1);
}


/*
 * Read a char string from the given fd and allocate space for it.
 */
int ggz_read_string_alloc(const int sock, char **message)
{
	unsigned int size;
	int status;

	if (ggz_read_int(sock, (int*)&size) < 0)
		return -1;

	if (size > ggz_alloc_limit) {
		ggz_debug(GGZ_SOCKET_DEBUG, "Error: Easysock allocation limit exceeded.");
		if (_err_func)
			(*_err_func) ("Allocation limit exceeded", GGZ_IO_ALLOCATE,
				      sock, GGZ_DATA_STRING);
		return -1;
	}
	
	/* FIXME: what happens if we don't have enough memory?  Exiting in
	   this case seems like a security risk. */
	*message = ggz_malloc((size+1) * sizeof(char));

	if ( (status = ggz_readn(sock, *message, size)) < 0) {
		ggz_debug(GGZ_SOCKET_DEBUG, "Error receiving string: %s",
			  strerror(errno));
		if (_err_func)
			(*_err_func) (strerror(errno), GGZ_IO_READ, sock, GGZ_DATA_STRING);
		return -1;
	}
	
	/* ggz_malloc zeroes memory, but we do it again anyway. */
	(*message)[size] = 0;

	if ((unsigned)status < size) {
		ggz_debug(GGZ_SOCKET_DEBUG, "Warning: fd is closed.");
		if (_err_func)
			(*_err_func) ("fd closed", GGZ_IO_READ, sock, GGZ_DATA_STRING);
		return -1;
	}

	ggz_debug(GGZ_SOCKET_DEBUG, "Received \"%s\" : string.", *message);
	return 0;
}


void ggz_read_string_alloc_or_die(const int sock, char **data)
{
	if (ggz_read_string_alloc(sock, data) < 0)
		(*_exit_func) (-1);
}


/* Write "n" bytes to a descriptor. */
int ggz_writen(const int sock, const void *vptr, size_t n)
{

	size_t nleft;
	ssize_t nwritten;
	const char *ptr;

	ptr = vptr;
	nleft = n;
	while (nleft > 0) {
#ifdef HAVE_WINSOCK2_H
		nwritten = send(sock, ptr, nleft, 0);
#else
		nwritten = write(sock, ptr, nleft);
#endif
		if (nwritten <= 0) {
			if (
#ifdef HAVE_WINSOCK2_H
			    /* FIXME: Somehow the socket is created
			       nonblocking under windows, and WSAWOULDBLOCK
			       is being returned.  The code as it is just
			       busywaits waiting for data, which is
			       extremely bad.  We need to make the socket
			       blocking. */
			    (WSAGetLastError() == WSAEINTR
			     || WSAGetLastError() == WSAEWOULDBLOCK)
#else
			    errno == EINTR
#endif
			    )
				nwritten = 0;	/* and call write() again */
			else
				return (-1);	/* error */
		}
		
		nleft -= nwritten;
		ptr += nwritten;
	}
	ggz_debug(GGZ_SOCKET_DEBUG, "Wrote %zi bytes.", n);
	return (n);
}


/* Read "n" bytes from a descriptor. */
int ggz_readn(const int sock, void *vptr, size_t n)
{

	size_t nleft;
	ssize_t nread;
	char *ptr;

	ptr = vptr;
	nleft = n;
	while (nleft > 0) {
#ifdef HAVE_WINSOCK2_H
		nread = recv(sock, ptr, nleft, 0);
#else
		nread = read(sock, ptr, nleft);
#endif
		if (nread < 0) {
			if (
#ifdef HAVE_WINSOCK2_H
			    /* FIXME: Somehow the socket is created
			       nonblocking under windows, and WSAWOULDBLOCK
			       is being returned.  The code as it is just
			       busywaits waiting for data, which is
			       extremely bad.  We need to make the socket
			       blocking. */
			    (WSAGetLastError() == WSAEINTR
			     || WSAGetLastError() == WSAEWOULDBLOCK)
#else
			    errno == EINTR
#endif
			    )
				nread = 0;	/* and call read() again */
			else
				return (-1);
		} else if (nread == 0)
			break;	/* EOF */

		nleft -= nread;
		ptr += nread;
	}
	ggz_debug(GGZ_SOCKET_DEBUG, "Read %zi bytes.", (n - nleft));
	return (n - nleft);	/* return >= 0 */
}


int ggz_write_fd(int sock, int sendfd)
{
#if GGZ_HAVE_SENDMSG
	struct msghdr msg;
	struct iovec iov[1];

#ifdef	HAVE_MSGHDR_MSG_CONTROL
	union {
		struct cmsghdr cm;
		char control[CMSG_SPACE(sizeof(int))];
	} control_un;
	struct cmsghdr *cmptr;

	/* Zero out the msg struct.  We should really initialize it properly.
	   There are some extra fields that don't get initialized below, but
	   I don't know if it's portable to add initializers for them.  Try
	   removing this line and running under valgrind to see the error.
	   See also the memset down below. */
	memset(&msg, 0, sizeof(msg));

	msg.msg_control = control_un.control;
	msg.msg_controllen = sizeof(control_un.control);

	cmptr = CMSG_FIRSTHDR(&msg);
	cmptr->cmsg_len = CMSG_LEN(sizeof(int));
	cmptr->cmsg_level = SOL_SOCKET;
	cmptr->cmsg_type = SCM_RIGHTS;
	*((int *) CMSG_DATA(cmptr)) = sendfd;
#else
	/* See the comment for memset() above. */
	memset(&msg, 0, sizeof(msg));

	msg.msg_accrights = (caddr_t) &sendfd;
	msg.msg_accrightslen = sizeof(int);
#endif

	msg.msg_name = NULL;
	msg.msg_namelen = 0;

        /* We're just sending a fd, so it's a dummy byte */
        iov[0].iov_base = "";
	iov[0].iov_len = 1;

	msg.msg_iov = iov;
	msg.msg_iovlen = 1;

	if (sendmsg(sock, &msg, 0) < 0) {
		ggz_debug(GGZ_SOCKET_DEBUG, "Error sending fd.");
		if (_err_func)
			(*_err_func) (strerror(errno), GGZ_IO_WRITE, sock, GGZ_DATA_FD);
		return -1;
	}

	ggz_debug(GGZ_SOCKET_DEBUG, "Sent \"%d\" : fd.", sendfd);
	return 0;
#else
	return -1;
#endif
}


int ggz_read_fd(int sock, int *recvfd)
{
#if GGZ_HAVE_SENDMSG
	struct msghdr msg;
	struct iovec iov[1];
	ssize_t	n;
        char dummy;
#ifndef HAVE_MSGHDR_MSG_CONTROL
	int newfd;
#endif


#ifdef	HAVE_MSGHDR_MSG_CONTROL
	union {
		struct cmsghdr cm;
		char control[CMSG_SPACE(sizeof(int))];
	} control_un;
	struct cmsghdr *cmptr;

	msg.msg_control = control_un.control;
	msg.msg_controllen = sizeof(control_un.control);
#else
	msg.msg_accrights = (caddr_t) &newfd;
	msg.msg_accrightslen = sizeof(int);
#endif

	msg.msg_name = NULL;
	msg.msg_namelen = 0;

        /* We're just sending a fd, so it's a dummy byte */
	iov[0].iov_base = &dummy;
	iov[0].iov_len = 1;

	msg.msg_iov = iov;
	msg.msg_iovlen = 1;

	if ( (n = recvmsg(sock, &msg, 0)) < 0) {
		ggz_debug(GGZ_SOCKET_DEBUG, "Error reading fd msg.");
		if (_err_func)
			(*_err_func) (strerror(errno), GGZ_IO_READ, sock, GGZ_DATA_FD);
		return -1;
	}

        if (n == 0) {
		ggz_debug(GGZ_SOCKET_DEBUG, "Warning: fd is closed.");
		if (_err_func)
			(*_err_func) ("fd closed", GGZ_IO_READ, sock, GGZ_DATA_FD);
	        return -1;
	}

#ifdef	HAVE_MSGHDR_MSG_CONTROL
        if ( (cmptr = CMSG_FIRSTHDR(&msg)) == NULL
	     || cmptr->cmsg_len != CMSG_LEN(sizeof(int))) {
		ggz_debug(GGZ_SOCKET_DEBUG, "Bad cmsg.");
		if (_err_func)
			(*_err_func) ("Bad cmsg", GGZ_IO_READ, sock, GGZ_DATA_FD);
		return -1;
	}

	if (cmptr->cmsg_level != SOL_SOCKET) {
		ggz_debug(GGZ_SOCKET_DEBUG, "Bad cmsg.");
		if (_err_func)
			(*_err_func) ("level != SOL_SOCKET", GGZ_IO_READ, sock, GGZ_DATA_FD);
		return -1;
	}

	if (cmptr->cmsg_type != SCM_RIGHTS) {
		ggz_debug(GGZ_SOCKET_DEBUG, "Bad cmsg.");
		if (_err_func)
			(*_err_func) ("type != SCM_RIGHTS", GGZ_IO_READ, sock, GGZ_DATA_FD);
		return -1;
	}

	/* Everything is good */
	*recvfd = *((int *) CMSG_DATA(cmptr));
#else
	if (msg.msg_accrightslen != sizeof(int)) {
		ggz_debug(GGZ_SOCKET_DEBUG, "Bad msg.");
		if (_err_func)
			(*_err_func) ("Bad msg", GGZ_IO_READ, sock, GGZ_DATA_FD);
		return -1;
	}		

	/* Everything is good */
	*recvfd = newfd;
#endif
	
	ggz_debug(GGZ_SOCKET_DEBUG, "Received \"%d\" : fd.", *recvfd);
        return 0;
#else
	return -1;
#endif
}

int ggz_set_network_notify_func(ggzNetworkNotify func)
{
	_notify_func = func;
	return 0;
}

#ifdef GAI_A
static struct gaicb *global_req = NULL;

static void ggz_resolved(sigval_t arg)
{
	char hostname[64];
	struct addrinfo *res;
	int ret;
	struct gaicb *req;

	req = global_req;
	if(gai_error(req)) {
		(*_notify_func)(req->ar_name, -2);
		return;
	}
	res = req->ar_result;
	ret = getnameinfo(res->ai_addr, res->ai_addrlen,
		hostname, sizeof(hostname), NULL, 0, NI_NUMERICHOST);
	if(ret) {
		(*_notify_func)(req->ar_name, -1);
		return;
	}
	if(!ggz_socketcreation) {
		(*_notify_func)(hostname, -1);
	} else {
		ret = ggz_make_socket(GGZ_SOCK_CLIENT, ggz_socketport, hostname);
		ggz_socketcreation = 0;
		(*_notify_func)(hostname, ret);
	}
}
#endif

const char *ggz_resolvename(const char *name)
{
	if (_notify_func) {
#ifdef GAI_A
	  	/* Start asynchronous lookup */
	  	struct sigevent sigev;
		struct gaicb *req;
		int ret;

		req = ggz_malloc(sizeof(*req));
		req->ar_name = name;
		req->ar_service = NULL;
		req->ar_request = NULL;
		sigev.sigev_notify = SIGEV_THREAD;
		sigev.sigev_value.sival_ptr = NULL;
		sigev.sigev_notify_function = ggz_resolved;
		sigev.sigev_notify_attributes = NULL;
		global_req = req;
		ret = getaddrinfo_a(GAI_NOWAIT, &req, 1, &sigev);
		if (ret) {
		  	(*_notify_func)(name, -2);
		}
#else
		/* Pass back unresolved name */
		(*_notify_func)(name, -2);
#endif
		return NULL;
	} else {
		return name;
	}
}

const char *ggz_getpeername(int fd, int resolve)
{
	char *ip;

#ifndef HAVE_WINSOCK2_H
	struct sockaddr_storage addr;
	socklen_t addrsize;
	int ret;

	addrsize = sizeof(addr);
	ret = getpeername(fd, (struct sockaddr*)&addr, &addrsize);
	if(ret != 0)
	{
		return NULL;
	}

	if(resolve)
	{
		ip = (char*)ggz_malloc(HOSTNAMELEN);
		ret = getnameinfo((struct sockaddr*)&addr, addrsize,
			ip, HOSTNAMELEN, NULL, 0, NI_NUMERICHOST);
		if(ret != 0)
		{
				ggz_free(ip);
				return NULL;
		}
		return ip;
	}

	/* FIXME: IPv4 compatibility? One could use getnameinfo() on addr... */
	if(addr.ss_family == AF_INET6)
	{
		ip = (char*)ggz_malloc(INET6_ADDRSTRLEN);
		inet_ntop(AF_INET6, (void*)&(((struct sockaddr_in6*)&addr)->sin6_addr),
			ip, INET6_ADDRSTRLEN);
	}
	else if(addr.ss_family == AF_INET)
	{
		ip = (char*)ggz_malloc(INET_ADDRSTRLEN);
		inet_ntop(AF_INET, (void*)&(((struct sockaddr_in*)&addr)->sin_addr),
			ip, INET_ADDRSTRLEN);
	}
	else ip = NULL;
#else
	ip = NULL;
#endif

	return ip;
}

