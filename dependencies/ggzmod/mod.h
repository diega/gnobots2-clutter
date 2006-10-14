/* 
 * File: mod.h
 * Author: GGZ Dev Team
 * Project: ggzmod
 * Date: 11/18/01
 * Desc: Functions for reading/writing messages from/to game modules
 * $Id$
 *
 * This file contains the backend for the ggzmod library.  This
 * library facilitates the communication between the GGZ server (ggz)
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


#ifndef __GGZ_MOD_H__
#define __GGZ_MOD_H__

#include <ggz.h>

#include "ggzmod.h"

/* The number of event handlers there are. */
#define GGZMOD_NUM_HANDLERS (GGZMOD_EVENT_ERROR + 1)

/* This is the actual structure, but it's only visible internally. */
struct GGZMod {
	GGZModType type;	/* ggz-end or game-end */
	GGZModState state;	/* the state of the game */
	int fd;			/* file descriptor */
	GGZModHandler handlers[GGZMOD_NUM_HANDLERS];
	void *gamedata;         /* game-specific data */

	int server_fd;
	const char *server_host;
	unsigned int server_port;
	const char *server_handle;

	const char *my_name;
	int i_am_spectator;
	int my_seat_num;

	/* Seat and spectator seat data. */
	int num_seats;
	GGZList *seats;
	GGZList *stats;
	GGZList *infos;
	int num_spectator_seats;
	GGZList *spectator_seats;
	GGZList *spectator_stats;

	/* ggz-only data */
	/* see mod-ggz.h */

	/* etc. */
};

/* Structure for player statistics */
typedef struct GGZStat {
	int number;
	int have_record, have_rating, have_ranking, have_highscore;
	int wins, losses, ties, forfeits;
	int rating, ranking, highscore;
} GGZStat;

/* GGZ+game side error function */
void _ggzmod_error(GGZMod *ggzmod, char* error);

/* GGZ+game side functions for handling various messages */
void _ggzmod_handle_state(GGZMod * ggzmod, GGZModState state);

/* Game side functions for handling various messages */
void _ggzmod_handle_launch(GGZMod * ggzmod);
void _ggzmod_handle_server(GGZMod * ggzmod,
			   const char *host, unsigned int port,
			   const char *handle);
void _ggzmod_handle_server_fd(GGZMod * ggzmod, int fd);
void _ggzmod_handle_player(GGZMod *ggzmod,
			   const char *name, int is_spectator, int seat_num);
void _ggzmod_handle_seat(GGZMod *ggzmod, GGZSeat *seat);
void _ggzmod_handle_spectator_seat(GGZMod *ggzmod, GGZSpectatorSeat *seat);
void _ggzmod_handle_chat(GGZMod *ggzmod, char *player, char *chat_msg);
void _ggzmod_handle_stats(GGZMod *ggzmod, GGZStat *player_stats,
			  GGZStat *spectator_stats);
void _ggzmod_handle_info(GGZMod * ggzmod, int seat_num,
			 const char *realname, const char *photo,
			 const char *host, int finish);

#endif /* __GGZ_MOD_H__ */
