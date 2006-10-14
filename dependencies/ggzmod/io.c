/* 
 * File: io.c
 * Author: GGZ Dev Team
 * Project: ggzmod
 * Date: 10/14/01
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

#ifdef HAVE_CONFIG_H
#  include <config.h>			/* Site-specific config */
#endif

#include <assert.h>
#include <stdlib.h>

#include <ggz.h>

#include "ggzmod.h"
#include "mod.h"
#include "io.h"
#include "protocol.h"

/* Private IO reading functions (game side) */

static int _io_read_msg_launch(GGZMod *ggzmod);
static int _io_read_msg_server(GGZMod *ggzmod);
static int _io_read_msg_server_fd(GGZMod *ggzmod);
static int _io_read_msg_player(GGZMod *ggzmod);
static int _io_read_msg_seat(GGZMod *ggzmod);
static int _io_read_msg_spectator_seat(GGZMod *ggzmod);
static int _io_read_msg_chat(GGZMod *ggzmod);
static int _io_read_stats(GGZMod *ggzmod);
static int _io_read_info(GGZMod *ggzmod);

/* Functions for sending IO messages */
int _io_send_state(int fd, GGZModState state)
{
	if (ggz_write_int(fd, MSG_GAME_STATE) < 0
	    || ggz_write_char(fd, state) < 0)
		return -1;
	else
		return 0;
}


int _io_send_req_stand(int fd)
{
	if (ggz_write_int(fd, REQ_STAND) < 0)
		return -1;
	return 0;
}

int _io_send_req_sit(int fd, int seat_num)
{
	if (ggz_write_int(fd, REQ_SIT) < 0
	    || ggz_write_int(fd, seat_num) < 0)
		return -1;
	return 0;
}

int _io_send_req_boot(int fd, const char *name)
{
	if (ggz_write_int(fd, REQ_BOOT) < 0
	    || ggz_write_string(fd, name) < 0)
		return -1;
	return 0;
}

int _io_send_request_bot(int fd, int seat_num)
{
	if (ggz_write_int(fd, REQ_BOT) < 0
	    || ggz_write_int(fd, seat_num) < 0)
		return -1;
	return 0;
}

int _io_send_request_open(int fd, int seat_num)
{
	if (ggz_write_int(fd, REQ_OPEN) < 0
	    || ggz_write_int(fd, seat_num) < 0)
		return -1;
	return 0;
}

int _io_send_request_chat(int fd, const char *chat_msg)
{
	if (ggz_write_int(fd, REQ_CHAT) < 0
	    || ggz_write_string(fd, chat_msg) < 0)
		return -1;
	return 0;
}

int _io_send_req_info(int fd, int seat_num)
{
	if (ggz_write_int(fd, REQ_INFO) < 0
	    || ggz_write_int(fd, seat_num) < 0)
		return -1;
	return 0;
}


/* Functions for reading messages */
int _io_read_data(GGZMod *ggzmod)
{
	int op;

	if (ggz_read_int(ggzmod->fd, &op) < 0)
		return -1;

	if (ggzmod->type == GGZMOD_GAME) {
		switch ((ControlToTable)op) {
		case MSG_GAME_LAUNCH:
			return _io_read_msg_launch(ggzmod);
		case MSG_GAME_SERVER:
			return _io_read_msg_server(ggzmod);
		case MSG_GAME_SERVER_FD:
			return _io_read_msg_server_fd(ggzmod);
		case MSG_GAME_PLAYER:
			return _io_read_msg_player(ggzmod);
		case MSG_GAME_SEAT:
			return _io_read_msg_seat(ggzmod);
		case MSG_GAME_SPECTATOR_SEAT:
			return _io_read_msg_spectator_seat(ggzmod);
		case MSG_GAME_CHAT:
			return _io_read_msg_chat(ggzmod);
		case MSG_GAME_STATS:
			return _io_read_stats(ggzmod);
		case MSG_GAME_INFO:
			return _io_read_info(ggzmod);
		}
	}

	return -2;
}


static int _io_read_msg_launch(GGZMod *ggzmod)
{
	_ggzmod_handle_launch(ggzmod);

	return 0;
}


static int _io_read_msg_server(GGZMod *ggzmod)
{
	char *host = NULL, *handle = NULL;
	int port;

	if (ggz_read_string_alloc(ggzmod->fd, &host) < 0
	    || ggz_read_int(ggzmod->fd, &port) < 0
	    || ggz_read_string_alloc(ggzmod->fd, &handle) < 0) {
		if (host) ggz_free(host);
		if (handle) ggz_free(handle);
		return -1;
	}

	_ggzmod_handle_server(ggzmod, host, port, handle);
	ggz_free(host);
	ggz_free(handle);
	return 0;
}

static int _io_read_msg_server_fd(GGZMod *ggzmod)
{
	int fd;

	if (ggz_read_fd(ggzmod->fd, &fd) < 0)
		return -1;
	else
		_ggzmod_handle_server_fd(ggzmod, fd);

	return 0;
}

static int _io_read_msg_player(GGZMod *ggzmod)
{
	int is_spectator, seat_num;
	char *name;

	if (ggz_read_string_alloc(ggzmod->fd, &name) < 0
	    || ggz_read_int(ggzmod->fd, &is_spectator) < 0
	    || ggz_read_int(ggzmod->fd, &seat_num) < 0)
		return -1;

	_ggzmod_handle_player(ggzmod,
			      name[0] == '\0' ? NULL : name,
			      is_spectator, seat_num);

	ggz_free(name);
	return 0;
}

static int _io_read_msg_seat(GGZMod *ggzmod)
{
	GGZSeat seat;
	char *name;
	int type;

	if (ggz_read_int(ggzmod->fd, (int*)&seat.num) < 0
	    || ggz_read_int(ggzmod->fd, &type) < 0
	    || ggz_read_string_alloc(ggzmod->fd, &name) < 0)
		  return -1;

	/* Set seat values */
	seat.type = type;
	if (name[0] == '\0') {
		ggz_free(name);
		seat.name = NULL;
	} else
		seat.name = name;

	_ggzmod_handle_seat(ggzmod, &seat);

	if (seat.name)
		ggz_free(seat.name);

	return 0;
}

static int _io_read_msg_spectator_seat(GGZMod *ggzmod)
{
	GGZSpectatorSeat seat;
	char *name;

	if (ggz_read_int(ggzmod->fd, (int*)&seat.num) < 0
	    || ggz_read_string_alloc(ggzmod->fd, &name) < 0)
		return -1;

	/* Detect empty seat case */
	if (name[0] == '\0') {
		ggz_free(name);
		seat.name = NULL;
	} else
		seat.name = name;

	_ggzmod_handle_spectator_seat(ggzmod, &seat);

	if (seat.name)
		ggz_free(seat.name);


	return 0;
}

static int _io_read_msg_chat(GGZMod *ggzmod)
{
	char *player, *chat;

	if (ggz_read_string_alloc(ggzmod->fd, &player) < 0
	    || ggz_read_string_alloc(ggzmod->fd, &chat) < 0)
		return -1;

	_ggzmod_handle_chat(ggzmod, player, chat);

	ggz_free(player);
	ggz_free(chat);
	return 0;
}

static int _io_read_stats(GGZMod *ggzmod)
{
	int players = ggzmod->num_seats;
	int spectators = ggzmod->num_spectator_seats;
	GGZStat player_stats[players], *stat, spectator_stats[spectators];
	int i;

	for (i = 0; i < players + spectators; i++) {
		if (i >= players) {
			stat = &spectator_stats[i - players];
		} else {
			stat = &player_stats[i];
		}

		if (ggz_read_int(ggzmod->fd, &stat->have_record) < 0
		    || ggz_read_int(ggzmod->fd, &stat->have_rating) < 0
		    || ggz_read_int(ggzmod->fd, &stat->have_ranking) < 0
		    || ggz_read_int(ggzmod->fd, &stat->have_highscore) < 0
		    || ggz_read_int(ggzmod->fd, &stat->wins) < 0
		    || ggz_read_int(ggzmod->fd, &stat->losses) < 0
		    || ggz_read_int(ggzmod->fd, &stat->ties) < 0
		    || ggz_read_int(ggzmod->fd, &stat->forfeits) < 0
		    || ggz_read_int(ggzmod->fd, &stat->rating) < 0
		    || ggz_read_int(ggzmod->fd, &stat->ranking) < 0
		    || ggz_read_int(ggzmod->fd, &stat->highscore) < 0) {
			return -1;
		}
	}

	  _ggzmod_handle_stats(ggzmod, player_stats, spectator_stats);

	return 0;
}

static int _io_read_info(GGZMod *ggzmod)
{
	int i, num;
	int seat_num;
	char *realname, *photo, *host;

	if (ggz_read_int(ggzmod->fd, &num) < 0)
		return -1;

	for (i = 0; i < num; i++) {
		if (ggz_read_int(ggzmod->fd, &seat_num) < 0
		    || ggz_read_string_alloc(ggzmod->fd, &realname) < 0
		    || ggz_read_string_alloc(ggzmod->fd, &photo) < 0
		    || ggz_read_string_alloc(ggzmod->fd, &host) < 0) {
			return -1;
		}

		/* if we receive empty values, they should be NULL'd (discarded) */
		_ggzmod_handle_info(ggzmod, seat_num,
			(realname[0] == '\0' ? NULL : realname),
			(photo[0] == '\0' ? NULL : photo),
			(host[0] == '\0' ? NULL : host),
			(num == 1 ? 1 : 0));

		ggz_free(realname);
		ggz_free(photo);
		ggz_free(host);
	}

	/* only call finished event once instead of for every seat */
	if(num != 1)
	{
		_ggzmod_handle_info(ggzmod, -1, NULL, NULL, NULL, 1);
	}

	return 0;
}
