/*
 * File: protocol.h
 * Author: Brent Hendricks
 * Project: GGZ
 * Date: 10/18/99
 * Desc: Protocol enumerations, etc.
 * $Id$
 *
 * Copyright (C) 1999 Brent Hendricks.
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


#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

typedef enum {
	GGZ_RESEAT_SIT,
	GGZ_RESEAT_STAND,
	GGZ_RESEAT_MOVE
} GGZReseatType;

/* I thought of putting this into ggz_common (in libggz) along with the
   protocol opcodes, but it really needs to stay tied to the network code
   itself (in ggzd and ggzcore). */
#define GGZ_CS_PROTO_VERSION 10

#endif /*__PROTOCOL_H__*/
