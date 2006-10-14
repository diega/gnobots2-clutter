/*
 * File: room.h
 * Author: Brent Hendricks
 * Project: GGZ Core Client Lib
 * Date: 6/5/00
 * $Id$
 *
 * This fils contains functions for handling rooms
 *
 * Copyright (C) 1998 Brent Hendricks.
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


#ifndef __ROOM_H__
#define __ROOM_H__

#include <ggz.h>

#include "gametype.h"
#include "ggzcore.h"
#include "hook.h"
#include "player.h"
#include "server.h"
#include "table.h"

/* Allocate space for a new room object */
GGZRoom *_ggzcore_room_new(void);

/* Initialize room object */
void _ggzcore_room_init(GGZRoom * room,
			GGZServer * server,
			const unsigned int id,
			const char *name,
			const unsigned int game,
			const char *desc, int player_count);

/* De-allocate room object and its children */
void _ggzcore_room_free(GGZRoom * room);


/* Functions for manipulating hooks to GGZRoom events */
int _ggzcore_room_add_event_hook_full(GGZRoom * room,
				      const GGZRoomEvent event,
				      const GGZHookFunc func,
				      const void *data);

int _ggzcore_room_remove_event_hook(GGZRoom * room,
				    const GGZRoomEvent event,
				    const GGZHookFunc func);

int _ggzcore_room_remove_event_hook_id(GGZRoom * room,
				       const GGZRoomEvent event,
				       const unsigned int hook_id);

/* Functions for changing GGZRoom data */
void _ggzcore_room_set_monitor(GGZRoom * room, char monitor);

void _ggzcore_room_set_player_list(GGZRoom * room,
				   unsigned int count, GGZList * list);
void _ggzcore_room_set_players(GGZRoom * room, int players);
void _ggzcore_room_add_player(GGZRoom * room, GGZPlayer * pdata,
			      int room_known, GGZRoom *from_room);
void _ggzcore_room_remove_player(GGZRoom * room, const char *name,
				 int room_known, GGZRoom *to_room);

void _ggzcore_room_set_table_list(GGZRoom * room,
				  unsigned int count, GGZList * list);
void _ggzcore_room_add_table(GGZRoom * room, struct _GGZTable *table);
void _ggzcore_room_remove_table(GGZRoom * room, const unsigned int id);


void _ggzcore_room_player_set_table(GGZRoom * room,
				    const char *name, int table);

void _ggzcore_room_table_event(GGZRoom *, GGZRoomEvent, void *data);



void _ggzcore_room_add_chat(GGZRoom * room,
			    GGZChatType type,
			    const char *name, const char *msg);

/* Functions for notifying GGZRoom */
void _ggzcore_room_set_table_launch_status(GGZRoom * room, int status);
void _ggzcore_room_set_table_join(GGZRoom * room, int table_index);
void _ggzcore_room_set_table_join_status(GGZRoom * room, int status);
void _ggzcore_room_set_table_leave(GGZRoom * room,
				   GGZLeaveType reason,
				   const char *player);
void _ggzcore_room_set_table_leave_status(GGZRoom * room, int status);


/* Functions for invoking GGZRoom "actions" */

int _ggzcore_room_load_playerlist(GGZRoom * room);
int _ggzcore_room_load_tablelist(GGZRoom * room,
				 const int type, const char global);

int _ggzcore_room_chat(GGZRoom * room,
		       const GGZChatType type,
		       const char *player, const char *msg);

int _ggzcore_room_admin(GGZRoom *room,
	               GGZAdminType type,
	               const char *player,
	               const char *reason);

int _ggzcore_room_launch_table(GGZRoom * room, struct _GGZTable *table);
int _ggzcore_room_join_table(GGZRoom * room, const unsigned int num,
			     int spectator);
int _ggzcore_room_leave_table(GGZRoom * room, int force);
int _ggzcore_room_leave_table_spectator(GGZRoom * room);

int _ggzcore_room_send_game_data(GGZRoom * room, char *buffer);
void _ggzcore_room_recv_game_data(GGZRoom * room, char *buffer);

unsigned int _ggzcore_room_get_id(const GGZRoom * room);

int _ggzcore_room_get_num(const GGZRoom *room);
void _ggzcore_room_set_num(GGZRoom *room, int num);

GGZPlayer *_ggzcore_room_get_player_by_name(const GGZRoom * room,
					    const char *name);
GGZHookReturn _ggzcore_room_event(GGZRoom * room, GGZRoomEvent id,
				  const void *data);

/* Utility functions for room lists */
int _ggzcore_room_compare(void *p, void *q);
void *_ggzcore_room_copy(void *p);
void _ggzcore_room_destroy(void *p);

/* Dynamic room updates */
void _ggzcore_room_close(GGZRoom *room);

#endif /* __ROOM_H_ */
