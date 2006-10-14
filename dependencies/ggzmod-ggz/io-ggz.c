/* 
 * File: io-ggz.c
 * Author: GGZ Dev Team
 * Project: ggzmod
 * Date: 10/14/01
 * Desc: Functions for reading/writing messages from/to game modules, GGZ side
 * $Id$
 *
 * This file contains the backend for the ggzmod library.  This
 * library facilitates the communication between the GGZ core client (ggz)
 * and game clients.  This file provides backend code that can be
 * used at both ends.
 *
 * Copyright (C) 2001 GGZ Dev Team.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>			/* Site-specific config */
#endif

#include <assert.h>
#include <stdlib.h>

#include <ggz.h>

#include "ggzmod-ggz.h"
#include "mod-ggz.h"
#include "io-ggz.h"
#include "protocol.h"

/* Private IO reading functions (ggz side) */

static int _io_ggz_read_msg_state(GGZMod *ggzmod);
static int _io_ggz_read_req_stand(GGZMod *ggzmod);
static int _io_ggz_read_req_sit(GGZMod *ggzmod);
static int _io_ggz_read_req_boot(GGZMod *ggzmod);
static int _io_ggz_read_req_bot(GGZMod *ggzmod);
static int _io_ggz_read_req_open(GGZMod *ggzmod);
static int _io_ggz_read_req_chat(GGZMod *ggzmod);
static int _io_ggz_read_req_info(GGZMod *ggzmod);

/* Functions for sending IO messages */
int _io_ggz_send_launch(int fd)
{
	if (ggz_write_int(fd, MSG_GAME_LAUNCH) < 0)
		return -1;

	return 0;
}

int _io_ggz_send_server(int fd, const char *host, unsigned int port,
		    const char *handle)
{
	if (ggz_write_int(fd, MSG_GAME_SERVER) < 0
	    || ggz_write_string(fd, host) < 0
	    || ggz_write_int(fd, port) < 0
	    || ggz_write_string(fd, handle) < 0)
		return -1;
	else
		return 0;
}


int _io_ggz_send_server_fd(int fd, int server_fd)
{
	if (ggz_write_int(fd, MSG_GAME_SERVER_FD) < 0
	    || ggz_write_fd(fd, server_fd) < 0)
		return -1;
	else
		return 0;
}


int _io_ggz_send_state(int fd, GGZModState state)
{
	if (ggz_write_int(fd, MSG_GAME_STATE) < 0
	    || ggz_write_char(fd, state) < 0)
		return -1;
	else
		return 0;
}

int _io_ggz_send_player(int fd, const char *name, int is_spectator, int seat_num)
{
	if (ggz_write_int(fd, MSG_GAME_PLAYER) < 0
	    || ggz_write_string(fd, name ? name : "") < 0
	    || ggz_write_int(fd, is_spectator) < 0
	    || ggz_write_int(fd, seat_num) < 0)
		return -1;

	return 0;
}

int _io_ggz_send_seat(int fd, GGZSeat *seat)
{
	if (ggz_write_int(fd, MSG_GAME_SEAT) < 0
	    || ggz_write_int(fd, seat->num) < 0
	    || ggz_write_int(fd, seat->type) < 0
	    || ggz_write_string(fd, seat->name ? seat->name : "") < 0)
		return -1;

	return 0;
}

int _io_ggz_send_spectator_seat(int fd, GGZSpectatorSeat *seat)
{
	const char * name = seat->name ? seat->name : "";

	if (ggz_write_int(fd, MSG_GAME_SPECTATOR_SEAT) < 0
	    || ggz_write_int(fd, seat->num) < 0
	    || ggz_write_string(fd, name) < 0)
		return -1;

	return 0;
}

int _io_ggz_send_msg_chat(int fd, const char *player, const char *chat_msg)
{
	if (ggz_write_int(fd, MSG_GAME_CHAT) < 0
	    || ggz_write_string(fd, player) < 0
	    || ggz_write_string(fd, chat_msg) < 0)
		return -1;
	return 0;
}

int _io_ggz_send_stats(int fd, int num_players, GGZStat *player_stats,
		   int num_spectators, GGZStat *spectator_stats)
{
	int i;
	GGZStat *stat;

	if (ggz_write_int(fd, MSG_GAME_STATS) < 0)
		return -1;

	for (i = 0; i < num_players + num_spectators; i++) {
		if (i >= num_players) {
			stat = &spectator_stats[i - num_players];
		} else {
			stat = &player_stats[i];
		}

		if (ggz_write_int(fd, stat->have_record) < 0
		    || ggz_write_int(fd, stat->have_rating) < 0
		    || ggz_write_int(fd, stat->have_ranking) < 0
		    || ggz_write_int(fd, stat->have_highscore) < 0
		    || ggz_write_int(fd, stat->wins) < 0
		    || ggz_write_int(fd, stat->losses) < 0
		    || ggz_write_int(fd, stat->ties) < 0
		    || ggz_write_int(fd, stat->forfeits) < 0
		    || ggz_write_int(fd, stat->rating) < 0
		    || ggz_write_int(fd, stat->ranking) < 0
		    || ggz_write_int(fd, stat->highscore) < 0) {
			return -1;
		}
	}
	return 0;
}

int _io_ggz_send_msg_info(int fd, int num, GGZList *infos)
{
	GGZListEntry *entry;

	if (ggz_write_int(fd, MSG_GAME_INFO) < 0
	    || ggz_write_int(fd, num) < 0)
		return -1;

	for (entry = ggz_list_head(infos); entry; entry = ggz_list_next(entry)) {
		GGZPlayerInfo *info = ggz_list_get_data(entry);
		if (ggz_write_int(fd, info->num) < 0
		    || ggz_write_string(fd, info->realname) < 0
		    || ggz_write_string(fd, info->photo) < 0
		    || ggz_write_string(fd, info->host) < 0)
			return -1;
	}

	return 0;
}


/* Functions for reading messages */
int _io_ggz_read_data(GGZMod *ggzmod)
{
	int op;

	if (ggz_read_int(ggzmod->fd, &op) < 0)
		return -1;

	if (ggzmod->type == GGZMOD_GGZ) {
		switch ((TableToControl)op) {
		case MSG_GAME_STATE:
			return _io_ggz_read_msg_state(ggzmod);
		case REQ_STAND:
			return _io_ggz_read_req_stand(ggzmod);
		case REQ_SIT:
			return _io_ggz_read_req_sit(ggzmod);
		case REQ_BOOT:
			return _io_ggz_read_req_boot(ggzmod);
		case REQ_BOT:
			return _io_ggz_read_req_bot(ggzmod);
		case REQ_OPEN:
			return _io_ggz_read_req_open(ggzmod);
		case REQ_CHAT:
			return _io_ggz_read_req_chat(ggzmod);
		case REQ_INFO:
			return _io_ggz_read_req_info(ggzmod);
		}
	}

	return -2;
}


static int _io_ggz_read_msg_state(GGZMod *ggzmod)
{
	char state;

	if (ggz_read_char(ggzmod->fd, &state) < 0)
		return -1;
	else
		_ggzmod_ggz_handle_state(ggzmod, state);
	
	return 0;
}

static int _io_ggz_read_req_stand(GGZMod *ggzmod)
{
	_ggzmod_ggz_handle_stand_request(ggzmod);
	return 0;
}

static int _io_ggz_read_req_sit(GGZMod *ggzmod)
{
	int seat_num;

	if (ggz_read_int(ggzmod->fd, &seat_num) < 0)
		return -1;

	_ggzmod_ggz_handle_sit_request(ggzmod, seat_num);
	return 0;
}

static int _io_ggz_read_req_boot(GGZMod *ggzmod)
{
	char *name;

	if (ggz_read_string_alloc(ggzmod->fd, &name) < 0)
		return -1;
	_ggzmod_ggz_handle_boot_request(ggzmod, name);
	ggz_free(name);
	return 0;
}

static int _io_ggz_read_req_bot(GGZMod *ggzmod)
{
	int seat_num;
	if (ggz_read_int(ggzmod->fd, &seat_num) < 0)
		return -1;
	_ggzmod_ggz_handle_bot_request(ggzmod, seat_num);
	return 0;
}

static int _io_ggz_read_req_open(GGZMod *ggzmod)
{
	int seat_num;
	if (ggz_read_int(ggzmod->fd, &seat_num) < 0)
		return -1;
	_ggzmod_ggz_handle_open_request(ggzmod, seat_num);
	return 0;
}


static int _io_ggz_read_req_chat(GGZMod *ggzmod)
{
	char *chat_msg;

	if (ggz_read_string_alloc(ggzmod->fd, &chat_msg) < 0)
		return -1;
	_ggzmod_ggz_handle_chat_request(ggzmod, chat_msg);
	ggz_free(chat_msg);
	return 0;
}

static int _io_ggz_read_req_info(GGZMod *ggzmod)
{
	int seat_num;

	if (ggz_read_int(ggzmod->fd, &seat_num) < 0)
		return -1;
	_ggzmod_ggz_handle_info_request(ggzmod, seat_num);
	return 0;
}

