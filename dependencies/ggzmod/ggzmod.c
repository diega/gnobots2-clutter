/* 
 * File: ggzmod.c
 * Author: GGZ Dev Team
 * Project: ggzmod
 * Date: 10/14/01
 * Desc: GGZ game module functions
 * $Id$
 *
 * This file contains the backend for the ggzmod library.  This
 * library facilitates the communication between the GGZ core client (ggz)
 * and game clients.  This file provides backend code that can be
 * used at both ends.
 *
 * Copyright (C) 2001-2002 GGZ Development Team.
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
#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_SYS_SELECT_H
# include <sys/select.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
#ifdef HAVE_WINSOCK2_H
# include <winsock2.h>
#endif
#include <sys/time.h>
#include <sys/types.h>
#ifdef HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif
#include <unistd.h>

#include <ggz.h>

#include "ggzcore-ggz.h"

#include "ggzmod.h"
/*#include "ggzmod-ggz.h"*/
#include "mod.h"
#include "io.h"
#include "protocol.h"


/* 
 * internal function prototypes
 */

static void call_handler(GGZMod *ggzmod, GGZModEvent event, void *data);
static int _ggzmod_handle_event(GGZMod * ggzmod, fd_set read_fds);
static void _ggzmod_set_state(GGZMod * ggzmod, GGZModState state);

/* Functions for manipulating seats */
static GGZSeat* seat_copy(GGZSeat *orig);
static int seat_compare(GGZSeat *a, GGZSeat *b);
static void seat_free(GGZSeat *seat);
static GGZSpectatorSeat* spectator_seat_copy(GGZSpectatorSeat *orig);
static int spectator_seat_compare(GGZSpectatorSeat *a, GGZSpectatorSeat *b);
static void spectator_seat_free(GGZSpectatorSeat *seat);

/* 
 * Creating/destroying a ggzmod object
 */

static int stats_compare(const void *p, const void *q)
{
	const GGZStat *s_p = p, *s_q = q;

	return s_p->number - s_q->number;
}

static void *stats_copy(void *porig)
{
	GGZStat *stat, *orig = porig;

	stat = ggz_malloc(sizeof(*stat));
	*stat = *orig;

	return stat;
}

static void stats_free(void *pstat)
{
	GGZStat *stat = pstat;

	ggz_free(stat);
}

static int infos_compare(const void *p, const void *q)
{
	const GGZPlayerInfo *s_p = p, *s_q = q;

	return s_p->num - s_q->num;
}

static void *infos_copy(void *porig)
{
	GGZPlayerInfo *info, *orig = porig;

	info = ggz_malloc(sizeof(*info));
	*info = *orig;
	info->realname = ggz_strdup(info->realname);
	info->photo = ggz_strdup(info->photo);
	info->host = ggz_strdup(info->host);

	return info;
}

static void infos_free(void *pstat)
{
	GGZPlayerInfo *info = pstat;

	ggz_free(info->realname);
	ggz_free(info->photo);
	ggz_free(info->host);
	ggz_free(info);
}


int ggzmod_is_ggz_mode(void)
{
	char *ggzmode;

#ifdef HAVE_GETENV
	ggzmode = getenv("GGZMODE");
#else
	ggzmode = GetEnvironmentVariable("GGZMODE");
#endif
	return (ggzmode && strcmp(ggzmode, "true") == 0);
}


/*
 * How a game is launched (incomplete):
 *
 * 1.  ggz client invokes ggzcore to execute the game.  The game executable is
 *     started.  If socketpair() is available then it is used to create a
 *     ggz<->game socket connection.
 *
 * 2.  A game launch packet is sent from the GGZ client to the game client.
 *
 * 3.  The game client sets its state to CONNECTED (from created)..
 *
 * 4.  GGZ-client is informed of the game state change.
 *
 * 5a. ggzmod_set_server informs the game client about where to connect to.
 *     A "server" packet is sent GGZ -> game.
 *
 * 6ab.The GGZ core client (or the game client) creates a socket and connects
 *     to GGZ...
 *
 * 7b. ...GGZ_GAME_NEGOTIATED ggzcore event is triggered...
 *
 * 8b. ggzcore_set_server_fd is called by the GGZ core client...
 *     ...it calls ggzmod_set_server_fd
 *
 * 9.  the game client sets its state to WAITING
 *
 * 10.  GGZ-client is informed of the game state change
 *
 * 11.  GGZ_GAME_PLAYING ggzcore event is triggered
 *
 * 12.  table-join or table-launch packet is sent from ggz CLIENT->SERVER
 *
 * 13.  ggz SERVER handles join/launch packets, sends response
 *
 * 14.  ... and then what? ...
 */

/* Creates a new ggzmod object. */
GGZMod *ggzmod_new(GGZModType type)
{
	int i;
	GGZMod *ggzmod;

	/* verify parameter */
	if (type != GGZMOD_GGZ && type != GGZMOD_GAME)
		return NULL;

	/* allocate */
	ggzmod = ggz_malloc(sizeof(*ggzmod));

	/* initialize */
	ggzmod->type = type;
	ggzmod->state = GGZMOD_STATE_CREATED;
	ggzmod->fd = -1;
	ggzmod->server_fd = -1;
	ggzmod->server_host = NULL;
	ggzmod->server_port = 0;
	ggzmod->server_handle = NULL;
	for (i = 0; i < GGZMOD_NUM_HANDLERS; i++)
		ggzmod->handlers[i] = NULL;
	ggzmod->gamedata = NULL;
	ggzmod->my_seat_num = -1;

	ggzmod->seats = ggz_list_create((ggzEntryCompare)seat_compare,
					(ggzEntryCreate)seat_copy,
					(ggzEntryDestroy)seat_free,
					GGZ_LIST_REPLACE_DUPS);
	ggzmod->spectator_seats =
		ggz_list_create((ggzEntryCompare)spectator_seat_compare,
				(ggzEntryCreate)spectator_seat_copy,
				(ggzEntryDestroy)spectator_seat_free,
				GGZ_LIST_REPLACE_DUPS);
	ggzmod->num_seats = ggzmod->num_spectator_seats = 0;

	ggzmod->stats = ggz_list_create(stats_compare, stats_copy, stats_free,
					GGZ_LIST_REPLACE_DUPS);
	ggzmod->spectator_stats = ggz_list_create(stats_compare,
						  stats_copy,
						  stats_free,
						  GGZ_LIST_REPLACE_DUPS);

	ggzmod->infos = ggz_list_create(infos_compare, infos_copy, infos_free,
					GGZ_LIST_REPLACE_DUPS);

	/* GGZ-side only initialization code was here*/

	return ggzmod;
}


/* Frees (deletes) a ggzmod object */
void ggzmod_free(GGZMod * ggzmod)
{
	if (!ggzmod) {
		return;
	}
	
	if (ggzmod->fd != -1)
		(void)ggzmod_disconnect(ggzmod);
	
	if (ggzmod->server_host) ggz_free(ggzmod->server_host);
	if (ggzmod->server_handle) ggz_free(ggzmod->server_handle);

	ggzmod->type = -1;

	if (ggzmod->my_name)
		ggz_free(ggzmod->my_name);

	/* GGZ-side only deinitialization code was here*/

	/* Free the object */
	ggz_free(ggzmod);
}


/* 
 * Accesor functions for GGZMod
 */

/* The ggzmod FD is the main ggz<->game server communications socket. */
int ggzmod_get_fd(GGZMod * ggzmod)
{
	if (!ggzmod) {
		return -1;
	}
	return ggzmod->fd;
}


GGZModType ggzmod_get_type(GGZMod * ggzmod)
{
	if (!ggzmod) {
		return -1;	/* not very useful */
	}
	return ggzmod->type;
}


GGZModState ggzmod_get_state(GGZMod * ggzmod)
{
	if (!ggzmod) {
		return -1;	/* not very useful */
	}
	return ggzmod->state;
}


int ggzmod_get_server_fd(GGZMod *ggzmod)
{
	if (!ggzmod) {
		return -1;
	}
	return ggzmod->server_fd;
}


void* ggzmod_get_gamedata(GGZMod * ggzmod)
{
	if (!ggzmod) {
		return NULL;
	}
	return ggzmod->gamedata;
}


void ggzmod_set_gamedata(GGZMod * ggzmod, void * data)
{
	if (ggzmod)
		ggzmod->gamedata = data;
}


void ggzmod_set_handler(GGZMod * ggzmod, GGZModEvent e,
			 GGZModHandler func)
{
	if (!ggzmod || e < 0 || e >= GGZMOD_NUM_HANDLERS) {
		ggz_error_msg("ggzmod_set_handler: "
			      "invalid params");
		return;		/* not very useful */
	}

	ggzmod->handlers[e] = func;
}


int ggzmod_set_state(GGZMod * ggzmod, GGZModState state)
{
	if (!ggzmod)
		return -1;

	if (ggzmod->type == GGZMOD_GAME) {
		/* The game may only change the state from one of
		   these two states. */
		if (ggzmod->state != GGZMOD_STATE_WAITING &&
		    ggzmod->state != GGZMOD_STATE_PLAYING)
			return -1;

		/* The game may only change the state to one of
		   these three states. */
		if (state == GGZMOD_STATE_PLAYING ||
		    state == GGZMOD_STATE_WAITING ||
		    state == GGZMOD_STATE_DONE)
			_ggzmod_set_state(ggzmod, state);
		else
			return -1;
	} else {
		/* TODO: an extension to the communications protocol will be
		   needed for this to work ggz-side.  Let's get the rest
		   of it working first... */
		return -1;
	}
	return 0;
}

static void _ggzmod_set_player(GGZMod *ggzmod,
			       const char *name,
			       int is_spectator, int seat_num)
{
	if (ggzmod->my_name)
		ggz_free(ggzmod->my_name);
	ggzmod->my_name = ggz_strdup(name);

	ggzmod->i_am_spectator = is_spectator;
	ggzmod->my_seat_num = seat_num;
}

void _ggzmod_handle_player(GGZMod *ggzmod,
			   const char *name,
			   int is_spectator, int seat_num)
{
	/* FIXME: better event data */
	int old[2] = {ggzmod->i_am_spectator, ggzmod->my_seat_num};

	assert(ggzmod->type == GGZMOD_GAME);
	_ggzmod_set_player(ggzmod, name, is_spectator, seat_num);

	if (ggzmod->state != GGZMOD_STATE_CREATED)
		call_handler(ggzmod, GGZMOD_EVENT_PLAYER, old);
}

const char *ggzmod_get_player(GGZMod *ggzmod,
			      int *is_spectator, int *seat_num)
{
	if (ggzmod->state == GGZMOD_STATE_CREATED) {
		ggz_error_msg("ggzmod_get_my_seat:"
			      " can't call when state is CREATED.");
		return NULL;
	}

	if (is_spectator) {
		*is_spectator = ggzmod->i_am_spectator;
	}
	if (seat_num) {
		*seat_num = ggzmod->my_seat_num;
	}

	return ggzmod->my_name;
}


/*
 * Seats and spectator seats.
 */

static GGZSeat *seat_copy(GGZSeat *orig)
{
	GGZSeat *seat;

	seat = ggz_malloc(sizeof(*seat));

	seat->num = orig->num;
	seat->type = orig->type;
	seat->name = ggz_strdup(orig->name);

	return seat;
}

static int seat_compare(GGZSeat *a, GGZSeat *b)
{
	return a->num - b->num;
}

static void seat_free(GGZSeat *seat)
{
	if (seat->name)
		ggz_free(seat->name);
	ggz_free(seat);
}

static GGZSpectatorSeat* spectator_seat_copy(GGZSpectatorSeat *orig)
{
	GGZSpectatorSeat *seat;

	seat = ggz_malloc(sizeof(*seat));

	seat->num = orig->num;
	seat->name = ggz_strdup(orig->name);

	return seat;
}

static int spectator_seat_compare(GGZSpectatorSeat *a, GGZSpectatorSeat *b)
{
	return a->num - b->num;
}

static void spectator_seat_free(GGZSpectatorSeat *seat)
{
	if (seat->name)
		ggz_free(seat->name);

	ggz_free(seat);
}

int ggzmod_get_num_seats(GGZMod * ggzmod)
{
	/* Note: num_seats is initialized to 0 and isn't
	   changed until we hear differently from GGZ. */
	return ggzmod->num_seats;
}

int ggzmod_get_num_spectator_seats(GGZMod * ggzmod)
{
	/* Note: num_spectator_seats is initialized to 0 and isn't
	   changed until we hear differently from GGZ. */
	return ggzmod->num_spectator_seats;
}

GGZSeat ggzmod_get_seat(GGZMod *ggzmod, int num)
{
	GGZSeat seat;
	seat.num = num;
	seat.type = GGZ_SEAT_NONE;
	seat.name = NULL;

	if (num >= 0 && num < ggzmod->num_seats) {
		GGZListEntry *entry;
		entry = ggz_list_search(ggzmod->seats, &seat);
		if (entry)
			seat = *(GGZSeat*)ggz_list_get_data(entry);
	}

	return seat;
}

GGZSpectatorSeat ggzmod_get_spectator_seat(GGZMod * ggzmod, int num)
{
	GGZSpectatorSeat seat;
	seat.num = num;
	seat.name = NULL;

	if (num >= 0 && num < ggzmod->num_spectator_seats) {
		GGZListEntry *entry;
		entry = ggz_list_search(ggzmod->spectator_seats, &seat);
		if (entry)
		  seat = *(GGZSpectatorSeat*)ggz_list_get_data(entry);
	}

	return seat;
}

static void _ggzmod_set_seat(GGZMod *ggzmod, GGZSeat *seat)
{
	if (seat->num >= ggzmod->num_seats)
		ggzmod->num_seats = seat->num + 1;
	ggz_list_insert(ggzmod->seats, seat);
}

void _ggzmod_handle_seat(GGZMod * ggzmod, GGZSeat *seat)
{
	GGZSeat *old_seat;
	GGZListEntry *entry;

	/* Copy current seat to old_seat */
	entry = ggz_list_search(ggzmod->seats, &seat);
	if (!entry) {
		GGZSeat myseat;
		myseat.num = seat->num;
		myseat.type = GGZ_SEAT_NONE;
		myseat.name = NULL;
		old_seat = seat_copy(&myseat);
	} else {
		old_seat = ggz_list_get_data(entry);
		old_seat = seat_copy(old_seat);
	}

	/* Place the new seat into the list */
	_ggzmod_set_seat(ggzmod, seat);

	/* Invoke the handler */
	if (ggzmod->state != GGZMOD_STATE_CREATED)
		call_handler(ggzmod, GGZMOD_EVENT_SEAT, old_seat);

	/* Free old_seat */
	seat_free(old_seat);
}

static void _ggzmod_set_spectator_seat(GGZMod * ggzmod, GGZSpectatorSeat *seat)
{
	if (seat->name) {
		if (seat->num >= ggzmod->num_spectator_seats)
			ggzmod->num_spectator_seats = seat->num + 1;
		ggz_list_insert(ggzmod->spectator_seats, seat);
	} else {
		/* Non-occupied seats are just empty entries in the list. */
		GGZListEntry *entry = ggz_list_search(ggzmod->spectator_seats,
						      seat);
		ggz_list_delete_entry(ggzmod->spectator_seats, entry);

		/* FIXME: reduce num_spectator_seats */
	}
}

void _ggzmod_handle_spectator_seat(GGZMod * ggzmod, GGZSpectatorSeat * seat)
{
	GGZSpectatorSeat *old_seat;
	GGZListEntry *entry;

	/* Copy current seat to old_seat */
	entry = ggz_list_search(ggzmod->spectator_seats, seat);
	if (!entry) {
		GGZSpectatorSeat myseat;
		myseat.num = seat->num;
		myseat.name = NULL;

		old_seat = spectator_seat_copy(&myseat);
	} else {
		old_seat = ggz_list_get_data(entry);
		old_seat = spectator_seat_copy(old_seat);
	}

	/* Place the new seat into the list */
	_ggzmod_set_spectator_seat(ggzmod, seat);

	/* Invoke the handler */
	if (ggzmod->state != GGZMOD_STATE_CREATED)
		call_handler(ggzmod, GGZMOD_EVENT_SPECTATOR_SEAT, old_seat);

	/* Free old_seat */
	spectator_seat_free(old_seat);
}

void _ggzmod_handle_chat(GGZMod *ggzmod, char *player, char *chat_msg)
{
	GGZChat chat;

	chat.player = player;
	chat.message = chat_msg;

	call_handler(ggzmod, GGZMOD_EVENT_CHAT, &chat);
}

void _ggzmod_handle_stats(GGZMod *ggzmod, GGZStat *player_stats,
			  GGZStat *spectator_stats)
{
	GGZListEntry *entry;
	GGZSeat *seat;
	GGZSpectatorSeat *spectator;
	GGZStat stat;
	int i;

	i = 0;
	for(entry = ggz_list_head(ggzmod->seats); entry; entry = ggz_list_next(entry)) {
		seat = ggz_list_get_data(entry);
		stat = player_stats[i];
		stat.number = seat->num;
		ggz_list_insert(ggzmod->stats, &stat);
		i++;
	}

	i = 0;
	for(entry = ggz_list_head(ggzmod->spectator_seats); entry; entry = ggz_list_next(entry)) {
		spectator = ggz_list_get_data(entry);
		stat = spectator_stats[i];
		stat.number = spectator->num;
		ggz_list_insert(ggzmod->spectator_stats, &stat);
		i++;
	}

	call_handler(ggzmod, GGZMOD_EVENT_STATS, NULL);
}

void _ggzmod_handle_info(GGZMod * ggzmod, int seat_num, const char *realname,
			 const char *photo, const char *host, int finish)
{
	GGZPlayerInfo info = {
		.num = seat_num,
		.realname = realname,
		.photo = photo,
		.host = host
	};

	if(seat_num != -1) {
		ggz_list_insert(ggzmod->infos, &info);
	}

	if(finish) {
		if(seat_num == -1) {
			call_handler(ggzmod, GGZMOD_EVENT_INFO, NULL);
		} else {
			call_handler(ggzmod, GGZMOD_EVENT_INFO, &info);
		}
	}
}


/* 
 * GGZmod actions
 */

int ggzmod_connect(GGZMod * ggzmod)
{
	char *ggzsocketstr;
	int ggzsocket;
	int items;

	if (!ggzmod)
		return -1;

	if (ggzmod->type == GGZMOD_GAME) {
		ggzsocket = 0;
#ifdef HAVE_GETENV
		ggzsocketstr = getenv("GGZSOCKET");
#else
		GetEnvironmentVariable("GGZSOCKET", buf, sizeof(buf));
#endif
		if(ggzsocketstr) {
			items = sscanf(ggzsocketstr, "%d", &ggzsocket);
			if (items == 0) {
				ggzsocket = 103;
			}
		} else {
			ggzsocket = 103;
		}

#ifdef HAVE_SOCKETPAIR
		ggzmod->fd = ggzsocket;
#else /* Winsock implementation: see game_fork(). */
		int sock;

		if (ggzsocket == 0) {
			ggz_error_msg_exit("Could not determine port.");
		}

		sock = ggz_make_socket(GGZ_SOCK_CLIENT, ggzsocket, "localhost");
		if (sock < 0) {
			ggz_error_msg_exit("Could not connect to port.");
		}
		ggzmod->fd = sock;
#endif
	}
	
	return 0;
}


int ggzmod_dispatch(GGZMod * ggzmod)
{
	struct timeval timeout;
	fd_set read_fd_set;
	int status;

	if (!ggzmod)
		return -1;

	if (ggzmod->fd < 0)
		return -1;

	FD_ZERO(&read_fd_set);
	FD_SET(ggzmod->fd, &read_fd_set);

	timeout.tv_sec = timeout.tv_usec = 0;	/* is this really portable? */

	status = select(ggzmod->fd + 1, &read_fd_set, NULL, NULL, &timeout);

	if (status == 0) {
		/* Nothing to read. */
		return 0;
	} else if (status < 0) {
		if (errno == EINTR)
			return 0;
		return -1;
	}

	return _ggzmod_handle_event(ggzmod, read_fd_set);
}

int ggzmod_disconnect(GGZMod * ggzmod)
{
	if (!ggzmod) {
		return -1;
	}
	if (ggzmod->fd < 0) {
		/* This isn't an error; it usually means
		   we already disconnected.  The invariant is that the
		   process (ggzmod->pid) exists iff the socket (ggzmod->fd)
		   exists. */
		return 0;
	}

	if (ggzmod->type == GGZMOD_GAME) {
		/* For client the game side we send a game over message */

		/* First warn the server of halt (if we haven't already) */
		_ggzmod_set_state(ggzmod, GGZMOD_STATE_DONE);
		ggz_debug("GGZMOD", "Disconnected from GGZ server.");
	}

	/* We no longer free the seat data here.  It will stick around until
	   ggzmod_free is called or it is used again. */

	/* Clean up the ggzmod object.  In theory it could now reconnect for
	   a new game. */
#ifdef HAVE_WINSOCK2_H
	closesocket(ggzmod->fd);
#else
	close(ggzmod->fd);
#endif
	ggzmod->fd = -1;

	return 0;
}



/* Returns -1 on error, the number of events handled on success. */
static int _ggzmod_handle_event(GGZMod * ggzmod, fd_set read_fds)
{
	int status = 0;
	
	if (FD_ISSET(ggzmod->fd, &read_fds)) {
		status = _io_read_data(ggzmod);
		if (status < 0) {
			_ggzmod_error(ggzmod, "Error reading data");
			/* FIXME: should be disconnect? */
			_ggzmod_set_state(ggzmod, GGZMOD_STATE_DONE);
		}
	}

	return status;
}


static void _ggzmod_set_state(GGZMod * ggzmod, GGZModState state)
{
	GGZModState old_state = ggzmod->state;
	if (state == ggzmod->state)
		return;		/* Is this an error? */

	/* The callback function retrieves the state from ggzmod_get_state.
	   It could instead be passed as an argument. */
	ggzmod->state = state;
	call_handler(ggzmod, GGZMOD_EVENT_STATE, &old_state);

	/* If we are the game module, send the new state to GGZ */
	if (ggzmod->type == GGZMOD_GAME) {
		ggz_debug("GGZMOD", "Game setting state to %d", 
			    state);
		if (_io_send_state(ggzmod->fd, state) < 0)
			/* FIXME: do some sort of error handling? */
			return;
	}
}


/**** Transaction requests  ****/

void ggzmod_request_stand(GGZMod * ggzmod)
{
	_io_send_req_stand(ggzmod->fd);
}

void ggzmod_request_sit(GGZMod * ggzmod, int seat_num)
{
	_io_send_req_sit(ggzmod->fd, seat_num);
}

void ggzmod_request_boot(GGZMod * ggzmod, const char *name)
{
	_io_send_req_boot(ggzmod->fd, name);
}

void ggzmod_request_bot(GGZMod * ggzmod, int seat_num)
{
	_io_send_request_bot(ggzmod->fd, seat_num);
}

void ggzmod_request_open(GGZMod * ggzmod, int seat_num)
{
	_io_send_request_open(ggzmod->fd, seat_num);
}

void ggzmod_request_chat(GGZMod *ggzmod, const char *chat_msg)
{
	_io_send_request_chat(ggzmod->fd, chat_msg);
}

/**** Internal library functions ****/

/* Invokes handlers for the specified event */
static void call_handler(GGZMod *ggzmod, GGZModEvent event, void *data)
{
	if (ggzmod->handlers[event])
		(*ggzmod->handlers[event]) (ggzmod, event, data);
}


void _ggzmod_error(GGZMod *ggzmod, char* error)
{
	call_handler(ggzmod, GGZMOD_EVENT_ERROR, error);
}


void _ggzmod_handle_state(GGZMod * ggzmod, GGZModState state)
{
	/* There's only certain ones the game is allowed to set it to,
	   and they can only change it if the state is currently
	   WAITING or PLAYING. */
	switch (state) {
	case GGZMOD_STATE_CREATED:
	case GGZMOD_STATE_CONNECTED:
	case GGZMOD_STATE_WAITING:
	case GGZMOD_STATE_PLAYING:
	case GGZMOD_STATE_DONE:
		/* In contradiction to what I say above, the game
		   actually _is_ allowed to change its state from
		   CREATED to CONNECTED and subsequently to WAITING.
		   or PLAYING.  When ggzmod-ggz sends a
		   launch packet to ggzmod-game, ggzmod-game
		   automatically changes the state from CREATED
		   to CONNECTED.  When this happens, it tells
		   ggzmod-ggz of this change and we end up back
		   here.  So, although it's a bit unsafe, we have
		   to allow this for now.  The alternative would
		   be to have ggzmod-ggz and ggzmod-game both
		   separately change states when the launch packet
		   is sent. */
		_ggzmod_set_state(ggzmod, state);
		return;
	}
	_ggzmod_error(ggzmod,
		      "Game requested incorrect state value");

	/* Is this right? has the gameover happened yet? */
}


void _ggzmod_handle_launch(GGZMod * ggzmod)
{
	/* Normally we let the game control its own state, but
	   we control the transition from CREATED to CONNECTED. */
	_ggzmod_set_state(ggzmod, GGZMOD_STATE_CONNECTED);
}


void _ggzmod_handle_server(GGZMod * ggzmod, const char *host,
			   unsigned int port, const char *handle)
{
	ggzmod->server_host = ggz_strdup(host);
	ggzmod->server_port = port;
	ggzmod->server_handle = ggz_strdup(handle);

	/* In the case of game clients connecting themselves, do it here. */
	ggzmod->server_fd = ggzcore_channel_connect(host, port, handle);
	if (ggzmod->server_fd < 0) {
		_ggzmod_error(ggzmod, "Could not create channel.");
		return;
	}
	_ggzmod_set_state(ggzmod, GGZMOD_STATE_WAITING);
	call_handler(ggzmod, GGZMOD_EVENT_SERVER, &ggzmod->server_fd);
}


void _ggzmod_handle_server_fd(GGZMod * ggzmod, int fd)
{
	ggzmod->server_fd = fd;

	/* Everything's done now, so we move into the waiting state. */
	_ggzmod_set_state(ggzmod, GGZMOD_STATE_WAITING);
	call_handler(ggzmod, GGZMOD_EVENT_SERVER, &ggzmod->server_fd);
}


int ggzmod_player_get_record(GGZMod *ggzmod, GGZSeat *seat,
			     int *wins, int *losses,
			     int *ties, int *forfeits)
{
	GGZStat search_stat = {.number = seat->num};
	GGZListEntry *entry = ggz_list_search(ggzmod->stats, &search_stat);
	GGZStat *stat = ggz_list_get_data(entry);

	if (!stat || !stat->have_record) return 0;

	*wins = stat->wins;
	*losses = stat->losses;
	*ties = stat->ties;
	*forfeits = stat->forfeits;
	return 1;
}

int ggzmod_player_get_rating(GGZMod *ggzmod, GGZSeat *seat, int *rating)
{
	GGZStat search_stat = {.number = seat->num};
	GGZListEntry *entry = ggz_list_search(ggzmod->stats, &search_stat);
	GGZStat *stat = ggz_list_get_data(entry);

	if (!stat || !stat->have_rating) return 0;

	*rating = stat->rating;
	return 1;
}

int ggzmod_player_get_ranking(GGZMod *ggzmod, GGZSeat *seat, int *ranking)
{
	GGZStat search_stat = {.number = seat->num};
	GGZListEntry *entry = ggz_list_search(ggzmod->stats, &search_stat);
	GGZStat *stat = ggz_list_get_data(entry);

	if (!stat || !stat->have_ranking) return 0;

	*ranking = stat->ranking;
	return 1;
}

int ggzmod_player_get_highscore(GGZMod *ggzmod, GGZSeat *seat, int *highscore)
{
	GGZStat search_stat = {.number = seat->num};
	GGZListEntry *entry = ggz_list_search(ggzmod->stats, &search_stat);
	GGZStat *stat = ggz_list_get_data(entry);

	if (!stat || !stat->have_highscore) return 0;

	*highscore = stat->highscore;
	return 1;
}

int ggzmod_spectator_get_record(GGZMod *ggzmod, GGZSpectatorSeat *seat,
				int *wins, int *losses,
				int *ties, int *forfeits)
{
	GGZStat search_stat = {.number = seat->num};
	GGZListEntry *entry = ggz_list_search(ggzmod->spectator_stats,
					      &search_stat);
	GGZStat *stat = ggz_list_get_data(entry);

	if(!stat) return 0;
	*wins = stat->wins;
	*losses = stat->losses;
	*ties = stat->ties;
	*forfeits = stat->forfeits;
	return 1;
}

int ggzmod_spectator_get_rating(GGZMod *ggzmod, GGZSpectatorSeat *seat, int *rating)
{
	GGZStat search_stat = {.number = seat->num};
	GGZListEntry *entry = ggz_list_search(ggzmod->spectator_stats,
					      &search_stat);
	GGZStat *stat = ggz_list_get_data(entry);

	if(!stat) return 0;
	*rating = stat->rating;
	return 1;
}

int ggzmod_spectator_get_ranking(GGZMod *ggzmod, GGZSpectatorSeat *seat, int *ranking)
{
	GGZStat search_stat = {.number = seat->num};
	GGZListEntry *entry = ggz_list_search(ggzmod->spectator_stats,
					      &search_stat);
	GGZStat *stat = ggz_list_get_data(entry);

	if(!stat) return 0;
	*ranking = stat->ranking;
	return 1;
}

int ggzmod_spectator_get_highscore(GGZMod *ggzmod, GGZSpectatorSeat *seat, int *highscore)
{
	GGZStat search_stat = {.number = seat->num};
	GGZListEntry *entry = ggz_list_search(ggzmod->spectator_stats,
					      &search_stat);
	GGZStat *stat = ggz_list_get_data(entry);

	if(!stat) return 0;
	*highscore = stat->highscore;
	return 1;
}

int ggzmod_player_request_info(GGZMod *ggzmod, int seat_num)
{
	GGZSeat seat;
	if(seat_num != -1) {
		if(seat_num < -1) return 0;
		if(seat_num >= ggzmod_get_num_seats(ggzmod)) return 0;
		seat = ggzmod_get_seat(ggzmod, seat_num);
		if(seat.type != GGZ_SEAT_PLAYER) return 0;
	}
	_io_send_req_info(ggzmod->fd, seat_num);
	return 1;
}

GGZPlayerInfo* ggzmod_player_get_info(GGZMod *ggzmod, int seat)
{
	GGZPlayerInfo search_info = {.num = seat};
	GGZListEntry *entry = ggz_list_search(ggzmod->infos, &search_info);

	return entry ?  ggz_list_get_data(entry) : NULL;
}
