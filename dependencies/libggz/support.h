/**
 * File   support.h
 * Author GGZ Development Team
 * Project: Libggz
 * Date: 02/03/03
 * $Id$
 * 
 * Replacements for non-supported functions.
 *
 * Copyright (C) 2003 GGZ Development Team.
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

/* HACK: on Cygwin, HAVE_SUN_LEN isn't getting defined but SUN_LEN
   is defined. */
#if !defined HAVE_SUN_LEN && !defined SUN_LEN
#define SUN_LEN(ptr) ((size_t)(((struct sockaddr_un *) 0)->sun_path) + strlen ((ptr)->sun_path))
#endif

#ifdef HAVE_MSGHDR_MSG_CONTROL
#ifndef HAVE_CMSG_ALIGN
#define CMSG_ALIGN(len) (((len) + sizeof (size_t) - 1) & ~(sizeof (size_t) - 1))
#endif
#ifndef HAVE_CMSG_SPACE
#define CMSG_SPACE(len) (CMSG_ALIGN (len) + CMSG_ALIGN (sizeof (struct cmsghdr)))
#endif
#ifndef HAVE_CMSG_LEN
#define CMSG_LEN(len)   (CMSG_ALIGN (sizeof (struct cmsghdr)) + (len))
#endif
#endif
