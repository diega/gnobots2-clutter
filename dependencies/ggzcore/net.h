/*
 * File: net.h
 * Author: Brent Hendricks
 * Project: GGZ Core Client Lib
 * Date: 9/22/00
 * $Id$
 *
 * Code for performing network I/O
 *
 * Copyright (C) 2000 Brent Hendricks.
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


#ifndef __NET_H__
#define __NET_H__

#include "ggzcore.h"
#include "protocol.h"
#include "table.h"

typedef struct _GGZNet GGZNet;

GGZNet *_ggzcore_net_new(void);
void _ggzcore_net_init(GGZNet * net,
		       GGZServer * server,
		       const char *host,
		       unsigned int port, unsigned int use_tls);

int _ggzcore_net_set_dump_file(GGZNet * net, const char *filename);
void _ggzcore_net_set_fd(GGZNet * net, int fd);

void _ggzcore_net_free(GGZNet * net);

const char *_ggzcore_net_get_host(GGZNet * net);
unsigned int _ggzcore_net_get_port(GGZNet * net);
int _ggzcore_net_get_fd(GGZNet * net);
int _ggzcore_net_get_tls(GGZNet * net);

int _ggzcore_net_connect(GGZNet * net);

void _ggzcore_net_disconnect(GGZNet * net);

/* Functions for sending data/requests to server */
int _ggzcore_net_send_login(GGZNet * net, GGZLoginType login_type,
			    const char *handle, const char *password, const char *email,
			    const char *language);
int _ggzcore_net_send_channel(GGZNet * net, const char *id);
int _ggzcore_net_send_motd(GGZNet * net);
int _ggzcore_net_send_list_types(GGZNet * net, const char verbose);
int _ggzcore_net_send_list_rooms(GGZNet * net,
				 const int type, const char verbose);
int _ggzcore_net_send_join_room(GGZNet * net, const unsigned int room_num);

int _ggzcore_net_send_list_players(GGZNet * net);
int _ggzcore_net_send_list_tables(GGZNet * net,
				  const int type, const char global);

int _ggzcore_net_send_chat(GGZNet * net,
			   const GGZChatType op,
			   const char *player, const char *msg);
int _ggzcore_net_send_admin(GGZNet * net,
			   const GGZAdminType type,
			   const char *player, const char *reason);

int _ggzcore_net_send_player_info(GGZNet * net, int seat_num);

int _ggzcore_net_send_table_launch(GGZNet * net, struct _GGZTable *table);
int _ggzcore_net_send_table_join(GGZNet * net, const unsigned int num,
				 int spectator);
int _ggzcore_net_send_table_leave(GGZNet * net, int force, int spectator);
int _ggzcore_net_send_table_reseat(GGZNet * net,
				   GGZReseatType opcode, int seat_num);
int _ggzcore_net_send_table_seat_update(GGZNet * net, GGZTable * table,
					GGZTableSeat * seat);
int _ggzcore_net_send_table_desc_update(GGZNet * net, GGZTable * table,
					const char *desc);
int _ggzcore_net_send_table_boot_update(GGZNet * net, GGZTable * table,
					GGZTableSeat * seat);

int _ggzcore_net_send_game_data(GGZNet * net, int size, char *buffer);

int _ggzcore_net_send_logout(GGZNet * net);


/* Functions for reading data from server */
int _ggzcore_net_data_is_pending(GGZNet * net);
int _ggzcore_net_read_data(GGZNet * net);

#endif /* __NET_H__ */
