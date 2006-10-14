/* 
 * File: io.c
 * Author: GGZ Dev Team
 * Project: ggzdmod
 * Date: 10/14/01
 * Desc: Functions for reading/writing messages from/to game modules
 * $Id$
 *
 * This file contains the backend for the ggzdmod library.  This
 * library facilitates the communication between the GGZ server (ggzd)
 * and game servers.  This file provides backend code that can be
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

#include <stdlib.h>

#include <ggz.h>

#include "ggzdmod.h"
#include "mod.h"
#include "io.h"
#include "protocol.h"


/* Private IO reading functions */
static int _io_read_req_state(GGZdMod *ggzdmod);
static int _io_read_msg_log(GGZdMod *ggzdmod);
static int _io_read_msg_report(GGZdMod *ggzdmod);
static int _io_read_msg_savegame(GGZdMod *ggzdmod);
static int _io_read_req_num_seats(GGZdMod * ggzdmod);
static int _io_read_req_boot(GGZdMod *ggzdmod);
static int _io_read_req_bot(GGZdMod *ggzdmod);
static int _io_read_req_open(GGZdMod *ggzdmod);

static int _io_read_req_launch(GGZdMod *ggzdmod);
static int _io_read_msg_seat_change(GGZdMod * ggzdmod);
static int _io_read_msg_reseat(GGZdMod * ggzdmod);
static int _io_read_msg_spectator_seat_change(GGZdMod *ggzdmod);


/* Functions for sending IO messages */
int _io_send_launch(int fd, const char *name, int seats, int spectators)
{
	if (ggz_write_int(fd, MSG_GAME_LAUNCH) < 0 
	    || ggz_write_string(fd, name) < 0
	    || ggz_write_int(fd, seats) < 0
		|| ggz_write_int(fd, spectators) < 0)
		return -1;
	else
		return 0;
}


int _io_send_seat_change(int fd, const GGZSeat *seat)
{
	ggz_debug("GGZDMOD", "Sending seat change");
	if (ggz_write_int(fd, MSG_GAME_SEAT) < 0
	    || ggz_write_int(fd, seat->num) < 0
	    || ggz_write_int(fd, seat->type) < 0
	    || ggz_write_string(fd, seat->name ? seat->name : "") < 0)
		return -1;

	if (seat->type == GGZ_SEAT_PLAYER) {
		ggz_debug("GGZDMOD", "Sending seat change fd");
		if (ggz_write_fd(fd, seat->fd) < 0)
			return -1;
	}
	return 0;
}


int _io_send_reseat(int fd,
		    int old_seat, int was_spectator,
		    int new_seat, int is_spectator)
{
	ggz_debug("GGZDMOD", "Sending reseat");
	if (ggz_write_int(fd, MSG_GAME_RESEAT) < 0
	    || ggz_write_int(fd, old_seat) < 0
	    || ggz_write_int(fd, was_spectator) < 0
	    || ggz_write_int(fd, new_seat) < 0
	    || ggz_write_int(fd, is_spectator) < 0)
		return -1;
	return 0;
}


int _io_send_spectator_change(int fd, const GGZSpectator *spectator)
{
	const char *name = spectator->name ? spectator->name : "";

	ggz_debug("GGZDMOD", "Sending spectator change");
	if (ggz_write_int(fd, MSG_GAME_SPECTATOR_SEAT) < 0
	    || ggz_write_int(fd, spectator->num) < 0
	    || ggz_write_string(fd, name) < 0)
		return -1;

	if (spectator->name
	    && ggz_write_fd(fd, spectator->fd) < 0)
		return -1;

	return 0;
}


int _io_send_state(int fd, GGZdModState state)
{
	if (ggz_write_int(fd, REQ_GAME_STATE) < 0
	    || ggz_write_char(fd, state) < 0)
		return -1;
	else
		return 0;
}


int _io_send_seat(int fd, const GGZSeat *seat)
{
	if (ggz_write_int(fd, seat->type) < 0)
		return -1;
	
	if (seat->type == GGZ_SEAT_RESERVED || seat->type == GGZ_SEAT_BOT) {
		if (ggz_write_string(fd, (seat->name ? seat->name : "")) < 0)
			return -1;
	}

	return 0;
}


int _io_send_log(int fd, const char *msg)
{
	if (ggz_write_int(fd, MSG_LOG) < 0 
	    || ggz_write_string(fd, msg) < 0)
		return -1;
	else
		return 0;
}


int _io_send_game_report(int fd, int num_players,
			 const char * const *names, const GGZSeatType *types,
			 const int *teams, const GGZGameResult *results,
			 const int *scores)
{
	int p;

	if (ggz_write_int(fd, MSG_GAME_REPORT) < 0
	    || ggz_write_int(fd, num_players) < 0)
		return -1;

	/* Note - we need to send the number of players, as well as the
	   list of player names, because some players could be in the
	   process of joining/leaving and GGZd may have a different updated
	   "master" list of who's at the table. */

	for (p = 0; p < num_players; p++) {
		int team = teams ? teams[p] : p;
		int result = results[p];
		int score = scores ? scores[p] : 0;
		const char *name = names[p] ? names[p] : "";
		if (ggz_write_string(fd, name) < 0
		    || ggz_write_int(fd, types[p]) < 0
		    || ggz_write_int(fd, team) < 0
		    || ggz_write_int(fd, result) < 0
		    || ggz_write_int(fd, score) < 0)
			return -1;
	}

	return 0;
}

int _io_send_savegame_report(int fd, const char *savegame)
{
	if (ggz_write_int(fd, MSG_SAVEGAME_REPORT) < 0
	    || ggz_write_string(fd, savegame) < 0)
		return -1;

	return 0;
}

int _io_send_req_num_seats(int fd, int num_seats)
{
	if (ggz_write_int(fd, REQ_NUM_SEATS) < 0
	    || ggz_write_int(fd, num_seats) < 0)
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

int _io_send_req_bot(int fd, int seat_num)
{
	if (ggz_write_int(fd, REQ_BOT) < 0
	    || ggz_write_int(fd, seat_num) < 0)
		return -1;
	return 0;
}

int _io_send_req_open(int fd, int seat_num)
{
	if (ggz_write_int(fd, REQ_OPEN) < 0
	    || ggz_write_int(fd, seat_num) < 0)
		return -1;
	return 0;
}


int _io_respond_state(int fd)
{
	return ggz_write_int(fd, RSP_GAME_STATE);
}


/* Functions for reading messages */
int _io_read_data(GGZdMod * ggzdmod)
{
	int op;

	if (ggz_read_int(ggzdmod->fd, &op) < 0)
		return -1;

	if (ggzdmod->type == GGZDMOD_GAME) {
		switch ((ControlToTable)op) {
		case MSG_GAME_LAUNCH:
			return _io_read_req_launch(ggzdmod);
		case MSG_GAME_SEAT:
			return _io_read_msg_seat_change(ggzdmod);
		case MSG_GAME_RESEAT:
			return _io_read_msg_reseat(ggzdmod);
		case MSG_GAME_SPECTATOR_SEAT:
			return _io_read_msg_spectator_seat_change(ggzdmod);
		case RSP_GAME_STATE:
			_ggzdmod_handle_state_response(ggzdmod);
			return 0;
		}
	} else {
		switch ((TableToControl)op) {
		case REQ_GAME_STATE:
			return _io_read_req_state(ggzdmod);
		case MSG_LOG:
			return _io_read_msg_log(ggzdmod);
		case MSG_GAME_REPORT:
			return _io_read_msg_report(ggzdmod);
		case MSG_SAVEGAME_REPORT:
			return _io_read_msg_savegame(ggzdmod);
		case REQ_NUM_SEATS:
			return _io_read_req_num_seats(ggzdmod);
		case REQ_BOOT:
			return _io_read_req_boot(ggzdmod);
		case REQ_BOT:
			return _io_read_req_bot(ggzdmod);
		case REQ_OPEN:
			return _io_read_req_open(ggzdmod);
		}
	}

	return -1;
}


static int _io_read_req_state(GGZdMod * ggzdmod)
{
	char state;

	if (ggz_read_char(ggzdmod->fd, &state) < 0)
		return -1;
	else
		_ggzdmod_handle_state(ggzdmod, state);
	
	return 0;
}


static int _io_read_msg_log(GGZdMod * ggzdmod)
{
	char *msg;

	if (ggz_read_string_alloc(ggzdmod->fd, &msg) < 0)
		return -1;
	else {
		_ggzdmod_handle_log(ggzdmod, msg);
		ggz_free(msg);
	}
	
	return 0;
}


static int _io_read_msg_report(GGZdMod *ggzdmod)
{
	int num_players;

	if (ggz_read_int(ggzdmod->fd, &num_players) < 0)
		return -1;
	else {
		char *names[num_players];
		int teams[num_players];
		int scores[num_players];
		GGZGameResult results[num_players];
		GGZSeatType types[num_players];
		int p;

		for (p = 0; p < num_players; p++) {
			int result;
			char *name;
			int type;

			if (ggz_read_string_alloc(ggzdmod->fd, &name) < 0
			    || ggz_read_int(ggzdmod->fd, &type) < 0
			    || ggz_read_int(ggzdmod->fd, &teams[p]) < 0
			    || ggz_read_int(ggzdmod->fd, &result) < 0
			    || ggz_read_int(ggzdmod->fd, &scores[p]) < 0)
				return -1; /* FIXME - mem leak */

			if (name)
				names[p] = name;
			else {
				ggz_free(name);
				names[p] = NULL; /* Bot */
			}
			results[p] = result;
			types[p] = type;
		}

		_ggzdmod_handle_report(ggzdmod, num_players,
				       names, types, teams, results, scores);

		for (p = 0; p < num_players; p++)
			if (names[p])
				ggz_free(names[p]);
	}

	return 0;
}


static int _io_read_msg_savegame(GGZdMod *ggzdmod)
{
	char *savegame;

	if (ggz_read_string_alloc(ggzdmod->fd, &savegame) < 0)
		return -1;

	_ggzdmod_handle_savegame(ggzdmod, savegame);

	ggz_free(savegame);

	return 0;
}


static int _io_read_req_num_seats(GGZdMod * ggzdmod)
{
	int num_seats;
	if (ggz_read_int(ggzdmod->fd, &num_seats) < 0)
		return -1;
	_ggzdmod_handle_num_seats_request(ggzdmod, num_seats);
	return 0;
}


static int _io_read_req_boot(GGZdMod *ggzdmod)
{
	char *name;

	if (ggz_read_string_alloc(ggzdmod->fd, &name) < 0)
		return -1;
	_ggzdmod_handle_boot_request(ggzdmod, name);
	ggz_free(name);
	return 0;
}


static int _io_read_req_bot(GGZdMod *ggzdmod)
{
	int seat_num;
	if (ggz_read_int(ggzdmod->fd, &seat_num) < 0)
		return -1;
	_ggzdmod_handle_bot_request(ggzdmod, seat_num);
	return 0;
}


static int _io_read_req_open(GGZdMod *ggzdmod)
{
	int seat_num;
	if (ggz_read_int(ggzdmod->fd, &seat_num) < 0)
		return -1;
	_ggzdmod_handle_open_request(ggzdmod, seat_num);
	return 0;
}


static int _io_read_req_launch(GGZdMod * ggzdmod)
{
	int seats, spectators, i;
	GGZSeat seat;
	char *game;
	
	if (ggz_read_string_alloc(ggzdmod->fd, &game) < 0)
		return -1;
	if (ggz_read_int(ggzdmod->fd, &seats) < 0)
		return -1;
	if (ggz_read_int(ggzdmod->fd, &spectators) < 0)
		return -1;

	_ggzdmod_handle_launch_begin(ggzdmod, game, seats, spectators);

	for (i = 0; i < seats; i++) {
		char *name = NULL;

		/* Reset seat */
		seat.num = i;
		seat.fd = -1;
		
		if (ggz_read_int(ggzdmod->fd, (int*)&seat.type) < 0)
			return -1;

		if (seat.type == GGZ_SEAT_RESERVED || seat.type == GGZ_SEAT_BOT) {
			if (ggz_read_string_alloc(ggzdmod->fd, &name) < 0)
				return -1;
			if (name[0] == '\0')
				name = NULL;
		}
		seat.name = name;
		
		_ggzdmod_handle_launch_seat(ggzdmod, seat);

		/* Free up name (if it was allocated) */
		if (name)
			ggz_free(name);
	}

	ggz_free(game);

	_ggzdmod_handle_launch_end(ggzdmod);
	
	return 0;
}


static int _io_read_msg_seat_change(GGZdMod * ggzdmod)
{
	GGZSeat seat;
	int type;
	char *name;
	
	if (ggz_read_int(ggzdmod->fd, (int*)&seat.num) < 0
	    || ggz_read_int(ggzdmod->fd, &type) < 0
	    || ggz_read_string_alloc(ggzdmod->fd, &name) < 0)
		return -1;
	seat.name = name;
	seat.type = type;

	if (seat.name[0] == '\0') {
		ggz_free(seat.name);
		seat.name = NULL;
	}

	if (seat.type == GGZ_SEAT_PLAYER) {
		if (ggz_read_fd(ggzdmod->fd, &seat.fd) < 0)
			return -1;
	} else
		seat.fd = -1;
	
	_ggzdmod_handle_seat(ggzdmod, &seat);

	if (seat.name)
		ggz_free(seat.name);
	
	return 0;
}


static int _io_read_msg_reseat(GGZdMod * ggzdmod)
{
	int old_seat, was_spectator;
	int new_seat, is_spectator;

	if (ggz_read_int(ggzdmod->fd, &old_seat) < 0
	    || ggz_read_int(ggzdmod->fd, &was_spectator) < 0
	    || ggz_read_int(ggzdmod->fd, &new_seat) < 0
	    || ggz_read_int(ggzdmod->fd, &is_spectator) < 0)
		return -1;

	_ggzdmod_handle_reseat(ggzdmod,
			       old_seat, was_spectator,
			       new_seat, is_spectator);
	return 0;
}

static int _io_read_msg_spectator_seat_change(GGZdMod * ggzdmod)
{
	GGZSpectator seat;
	char *name;
	
	if (ggz_read_int(ggzdmod->fd, (int*)&seat.num) < 0
	    || ggz_read_string_alloc(ggzdmod->fd, &name) < 0)
		return -1;
	seat.name = name;

	if (seat.name[0] == '\0') {
		ggz_free(seat.name);
		seat.name = NULL;
		seat.fd = -1;
	} else {
		if (ggz_read_fd(ggzdmod->fd, &seat.fd) < 0)
			return -1;
	}

	_ggzdmod_handle_spectator_seat(ggzdmod, &seat);

	if (seat.name)
		ggz_free(seat.name);

	return 0;
}
