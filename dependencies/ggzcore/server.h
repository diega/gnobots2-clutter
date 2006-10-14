/*
 * File: server.h
 * Author: Brent Hendricks
 * Project: GGZ Core Client Lib
 * Date: 1/19/01
 * $Id$
 *
 * Code for handling server connection state and properties
 *
 * Copyright (C) 2001 Brent Hendricks.
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

#ifndef __SERVER_H__
#define __SERVER_H__

#include "gametype.h"
#include "ggzcore.h"
#include "room.h"
#include "state.h"
#include "net.h"

GGZServer *_ggzcore_server_new(void);

/* functions to get a GGZServer attribute */
GGZNet *_ggzcore_server_get_net(const GGZServer * server);
GGZLoginType _ggzcore_server_get_type(const GGZServer * server);
const char *_ggzcore_server_get_handle(const GGZServer * server);
const char *_ggzcore_server_get_password(const GGZServer * server);
GGZStateID _ggzcore_server_get_state(const GGZServer * server);
int _ggzcore_server_get_tls(const GGZServer * server);

int _ggzcore_server_get_num_players(const GGZServer * server);
GGZPlayer* _ggzcore_server_get_player(GGZServer *server, const char *name);

int _ggzcore_server_get_num_rooms(const GGZServer * server);
GGZRoom *_ggzcore_server_get_cur_room(const GGZServer * server);
GGZRoom *_ggzcore_server_get_nth_room(const GGZServer * server,
				      const unsigned int num);
int _ggzcore_server_get_room_num(const GGZServer *server,
				 const GGZRoom *room);
GGZRoom *_ggzcore_server_get_room_by_id(const GGZServer * server,
					const unsigned int id);

int _ggzcore_server_get_num_gametypes(const GGZServer * server);
GGZGameType *_ggzcore_server_get_nth_gametype(const GGZServer * server,
					      const unsigned int num);
GGZGameType *_ggzcore_server_get_type_by_id(const GGZServer * server,
					    const unsigned int id);
GGZGame *_ggzcore_server_get_cur_game(const GGZServer * server);

void _ggzcore_server_set_cur_game(GGZServer * server, GGZGame * game);

/* functions to set a GGZServer attribute */
void _ggzcore_server_set_logintype(GGZServer * server,
				   const GGZLoginType type);
void _ggzcore_server_set_handle(GGZServer * server, const char *handle);
void _ggzcore_server_set_password(GGZServer * server,
				  const char *password);
void _ggzcore_server_set_email(GGZServer * server,
				  const char *email);

void _ggzcore_server_set_cur_room(GGZServer * server, GGZRoom * room);

/* functions to pass status of other information to server object */
void _ggzcore_server_set_negotiate_status(GGZServer * server,
					  GGZNet * net,
					  GGZClientReqError status);
void _ggzcore_server_set_login_status(GGZServer * server,
				      GGZClientReqError status);
void _ggzcore_server_set_room_join_status(GGZServer * server,
					  GGZClientReqError status);
void _ggzcore_server_set_table_launching(GGZServer * server);
void _ggzcore_server_set_table_joining(GGZServer * server);
void _ggzcore_server_set_table_leaving(GGZServer * server);
void _ggzcore_server_set_table_launch_status(GGZServer * server,
					     GGZClientReqError status);
void _ggzcore_server_set_table_join_status(GGZServer * server,
					   GGZClientReqError status);
void _ggzcore_server_set_table_leave_status(GGZServer * server,
					    GGZClientReqError status);
void _ggzcore_server_session_over(GGZServer * server, GGZNet * net);

/* functions to perform an action */
int _ggzcore_server_log_session(GGZServer * server, const char *filename);
void _ggzcore_server_reset(GGZServer * server);
int _ggzcore_server_connect(GGZServer * server);
int _ggzcore_server_create_channel(GGZServer * server);
int _ggzcore_server_login(GGZServer * server);
int _ggzcore_server_load_motd(GGZServer * server);
int _ggzcore_server_load_typelist(GGZServer * server, const char verbose);
int _ggzcore_server_load_roomlist(GGZServer * server,
				  const int type, const char verbose);
int _ggzcore_server_join_room(GGZServer * server, GGZRoom *room);

int _ggzcore_server_logout(GGZServer * server);
int _ggzcore_server_disconnect(GGZServer * server);
void _ggzcore_server_net_error(GGZServer * server, char *message);
void _ggzcore_server_protocol_error(GGZServer * server, char *message);

void _ggzcore_server_clear(GGZServer * server);
void _ggzcore_server_clear_reconnect(GGZServer * server);

void _ggzcore_server_free(GGZServer * server);

/* Functions for manipulating list of rooms */
void _ggzcore_server_init_roomlist(GGZServer * server, const int num);
void _ggzcore_server_free_roomlist(GGZServer * server);
void _ggzcore_server_grow_roomlist(GGZServer * server);
void _ggzcore_server_add_room(GGZServer * server, GGZRoom * room);
void _ggzcore_server_delete_room(GGZServer * server, GGZRoom * room);

/* Functions for manipulating list of gametypes */
void _ggzcore_server_init_typelist(GGZServer * server, const int num);
void _ggzcore_server_free_typelist(GGZServer * server);
void _ggzcore_server_add_type(GGZServer * server, GGZGameType * type);

/* Various event functions */
int _ggzcore_server_event_is_valid(GGZServerEvent event);
void _ggzcore_server_change_state(GGZServer * server, GGZTransID trans);
GGZHookReturn _ggzcore_server_event(GGZServer *, GGZServerEvent, void *);
void _ggzcore_server_queue_players_changed(GGZServer * server);

/* Options enabled for ggzcore */
void _ggzcore_server_set_reconnect(void);
void _ggzcore_server_set_threaded_io(void);

#endif /* __SERVER_H__ */
