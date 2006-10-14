/* 
 * File: io.h
 * Author: GGZ Dev Team
 * Project: ggzmod
 * Date: 11/18/01
 * Desc: Functions for reading/writing messages from/to game modules
 * $Id$
 *
 * This file contains the backend for the ggzmod library.  This
 * library facilitates the communication between the GGZ core client (ggz)
 * and game clients.  This file provides backend code that can be
 * used at both ends.
 *
 * Copyright (C) 2001 GGZ Dev Team.
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

#ifndef __GGZ_IO_H__
#define __GGZ_IO_H__

#include "ggzmod.h"
#include "mod.h"

/* Functions for sending IO messages (ggz+game) */
int _io_send_state(int fd, GGZModState state);

/* Functions for sending IO messages (game only) */
int _io_send_req_stand(int fd);
int _io_send_req_sit(int fd, int seat_num);
int _io_send_req_boot(int fd, const char *name);
int _io_send_request_bot(int fd, int seat_num);
int _io_send_request_open(int fd, int seat_num);
int _io_send_request_chat(int fd, const char *chat_msg);

int _io_send_req_info(int fd, int seat_num);

/* Read and dispatch message */
int _io_read_data(GGZMod * ggzmod);

#endif /* __GGZ_IO_H__ */
