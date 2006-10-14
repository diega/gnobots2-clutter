/* 
 * File: ggzdmod.c
 * Author: GGZ Dev Team
 * Project: ggzdmod
 * Date: 10/14/01
 * Desc: GGZ game module functions
 * $Id$
 *
 * This file contains the backend for the ggzdmod library.  This
 * library facilitates the communication between the GGZ server (ggzd)
 * and game servers.  This file provides backend code that can be
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
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <ggz.h>

#include "ggzdmod.h"
#include "ggzdmod-ggz.h"
#include "mod.h"
#include "io.h"


/* This checks the ggzdmod object for validity.  It could do more checking if
   desired. */
#define CHECK_GGZDMOD(ggzdmod) (ggzdmod)



/* 
 * internal function prototypes
 */

static void call_handler(GGZdMod *ggzdmod, GGZdModEvent event, void *data);
static int get_fd_max(GGZdMod * ggzdmod);
static fd_set get_active_fd_set(GGZdMod * ggzdmod);
static void _ggzdmod_set_game(GGZdMod * ggzdmod, const char *game);
static void _ggzdmod_set_num_seats(GGZdMod * ggzdmod, int num_seats);
static void _ggzdmod_set_max_num_spectators(GGZdMod * ggzdmod,
					    int num_spectators);
static void set_state(GGZdMod * ggzdmod, GGZdModState state);
static int handle_event(GGZdMod * ggzdmod, fd_set read_fds);
static int send_game_launch(GGZdMod * ggzdmod);
static void postfork_error(int fd, const char *fmt, ...)
	ggz__attribute((format(printf, 2, 3)))
	ggz__attribute((noreturn));
static int game_fork(GGZdMod * ggzdmod);

/* Functions for manipulating seats */
static GGZSeat* seat_copy(GGZSeat *orig);
static int seat_compare(GGZSeat *a, GGZSeat *b);
static void seat_free(GGZSeat *seat);

/* Functions for manipulating spectators */
static GGZSpectator* spectator_copy(GGZSpectator *orig);
static int spectator_compare(GGZSpectator *a, GGZSpectator *b);
static void spectator_free(GGZSpectator *spectator);

/* Debugging function (see also ggzdmod_check) */
static void seat_print(GGZdMod * ggzdmod, GGZSeat *seat);


int ggzdmod_is_ggz_mode(void)
{
	char *ggzmode;

	/* ggzdmod won't run in windows (windows doesn't have getenv()) */
	ggzmode = getenv("GGZMODE");
	return (ggzmode && strcmp(ggzmode, "true") == 0);
}

/* Invokes handlers for the specefied event */
static void call_handler(GGZdMod * ggzdmod, GGZdModEvent event, void *data)
{
	if (event < 0 || event >= GGZDMOD_NUM_EVENTS) {
		fprintf(stderr,
			"GGZDMOD: call_handler called for unknown event %d.\n"
			"This is a bug in libggzdmod.  Please e-mail the GGZ\n"
			"development team at ggz-dev@mail.ggzgamingzone.org\n"
			"to report it.\n", event);
		return;
	}
		  

	if (ggzdmod->handlers[event])
		(*ggzdmod->handlers[event]) (ggzdmod, event, data);
	else {
		/* We'd like to send a debugging message if there's
		   no handler registered for an event.  But in the
		   case of the ERROR event, this can cause problems
		   since the problem is usually a missing connection!
		   So, we use this hack to avoid a recursive loop. */
		char *which = ggzdmod->type == GGZDMOD_GAME ? "game" : "ggz";
		if (event != GGZDMOD_EVENT_ERROR)
			ggzdmod_log(ggzdmod,
				    "GGZDMOD: unhandled event %d by %s.",
				    event, which);
		else
			fprintf(stderr,
				"GGZDMOD: unhandled error event %d by %s: %s\n",
				event, which, (char*)data);
	}
}


/* Returns the highest-numbered FD used by ggzdmod. */
static int get_fd_max(GGZdMod * ggzdmod)
{
	int max = ggzdmod->fd;
	GGZListEntry *entry;
	GGZSeat *seat;
	GGZSpectator *spectator;

	/* If we don't have a player data handler set
	   up, we won't monitor the player data sockets. */
	if (ggzdmod->handlers[GGZDMOD_EVENT_PLAYER_DATA])
		for (entry = ggz_list_head(ggzdmod->seats);
		     entry != NULL;
		     entry = ggz_list_next(entry)) {

			seat = ggz_list_get_data(entry);
			if (seat->fd > max)
				max = seat->fd;
		}

	/* Same for spectators */
	if (ggzdmod->handlers[GGZDMOD_EVENT_SPECTATOR_DATA])
		for (entry = ggz_list_head(ggzdmod->spectators);
		     entry != NULL;
		     entry = ggz_list_next(entry)) {

			spectator = ggz_list_get_data(entry);
			if (spectator->fd > max)
				max = spectator->fd;
		}

	return max;
}


static fd_set get_active_fd_set(GGZdMod * ggzdmod)
{
	fd_set active_fd_set;
	GGZListEntry *entry;
	GGZSeat *seat;
	GGZSpectator *spectator;
	
	FD_ZERO(&active_fd_set);
	if (ggzdmod->fd != -1)
		FD_SET(ggzdmod->fd, &active_fd_set);

	/* Only monitory player fds if there is a PLAYER_DATA handler set... */
	if (ggzdmod->handlers[GGZDMOD_EVENT_PLAYER_DATA]) {
		for (entry = ggz_list_head(ggzdmod->seats);
		     entry != NULL;
		     entry = ggz_list_next(entry)) {
			
			seat = ggz_list_get_data(entry);
			if (seat->fd != -1)
				FD_SET(seat->fd, &active_fd_set);
		}
	}

	/* ... or a SPECTATOR_DATA handler */
	if (ggzdmod->handlers[GGZDMOD_EVENT_SPECTATOR_DATA]) {
		for (entry = ggz_list_head(ggzdmod->spectators);
		     entry != NULL;
		     entry = ggz_list_next(entry)) {
			
			spectator = ggz_list_get_data(entry);
			if (spectator->fd != -1)
				FD_SET(spectator->fd, &active_fd_set);
		}
	}

	/* FIXME: shouldn't we log an error if there are no available FD's? */
	
	return active_fd_set;
}


/* 
 * Creating/destroying a ggzdmod object
 */

/* Creates a new ggzdmod object. */
GGZdMod *ggzdmod_new(GGZdModType type)
{
	int i;
	GGZdMod *ggzdmod;

	/* verify parameter */
	if (type != GGZDMOD_GGZ && type != GGZDMOD_GAME)
		return NULL;

	/* allocate */
	ggzdmod = ggz_malloc(sizeof(*ggzdmod));

	/* initialize */
	ggzdmod->type = type;
	ggzdmod->state = GGZDMOD_STATE_CREATED;
	ggzdmod->fd = -1;
	ggzdmod->seats = ggz_list_create((ggzEntryCompare)seat_compare,
					 (ggzEntryCreate)seat_copy,
					 (ggzEntryDestroy)seat_free,
					 GGZ_LIST_REPLACE_DUPS);
	ggzdmod->spectators = ggz_list_create((ggzEntryCompare)spectator_compare,
					(ggzEntryCreate)spectator_copy,
					(ggzEntryDestroy)spectator_free,
					GGZ_LIST_REPLACE_DUPS);
	ggzdmod->num_seats = 0;
	ggzdmod->max_num_spectators = 0;

	for (i = 0; i < GGZDMOD_NUM_EVENTS; i++)
		ggzdmod->handlers[i] = NULL;
	ggzdmod->gamedata = NULL;

	ggzdmod->pid = -1;
	ggzdmod->argv = NULL;
	ggzdmod->game = NULL;
	/* Put any other necessary initialization here.  All fields
	   should be initialized.  Note NULL may not necessarily be 0
	   on all platforms. */

	return ggzdmod;
}


/* Frees (deletes) a ggzdmod object */
void ggzdmod_free(GGZdMod * ggzdmod)
{
	int i;

	if (!CHECK_GGZDMOD(ggzdmod)) {
		return;
	}
	
	if (ggzdmod->fd != -1)
		(void)ggzdmod_disconnect(ggzdmod);
	
	ggz_list_free(ggzdmod->seats);
	ggzdmod->num_seats = 0;
	ggz_list_free(ggzdmod->spectators);
	ggzdmod->max_num_spectators = 0;

	ggzdmod->type = -1;

	if (ggzdmod->pwd)
		ggz_free(ggzdmod->pwd);
	
	if (ggzdmod->argv) {
		for (i = 0; ggzdmod->argv[i]; i++)
			if (ggzdmod->argv[i])
				ggz_free(ggzdmod->argv[i]);
		ggz_free(ggzdmod->argv);
	}

	if(ggzdmod->game)
		ggz_free(ggzdmod->game);

	/* Free the object */
	ggz_free(ggzdmod);
}


/* 
 * Accessor functions for GGZdMod
 */

/* The ggzdmod FD is the main ggzd<->game server communications socket. */
int ggzdmod_get_fd(GGZdMod * ggzdmod)
{
	if (!CHECK_GGZDMOD(ggzdmod)) {
		return -1;
	}
	return ggzdmod->fd;
}


GGZdModType ggzdmod_get_type(GGZdMod * ggzdmod)
{
	if (!CHECK_GGZDMOD(ggzdmod)) {
		return -1;	/* not very useful */
	}
	return ggzdmod->type;
}


GGZdModState ggzdmod_get_state(GGZdMod * ggzdmod)
{
	if (!CHECK_GGZDMOD(ggzdmod)) {
		return -1;	/* not very useful */
	}
	return ggzdmod->state;
}


int ggzdmod_get_num_seats(GGZdMod * ggzdmod)
{
	if (!CHECK_GGZDMOD(ggzdmod)) {
		return -1;
	}
	/* Note: num_seats is initialized to 0 and isn't changed until the
	   launch event (GGZDMOD_EVENT_LAUNCH). */
	return ggzdmod->num_seats;
}


int ggzdmod_get_max_num_spectators(GGZdMod * ggzdmod)
{
	if (!CHECK_GGZDMOD(ggzdmod) || ggzdmod->fd < 0) {
		return -1;
	}

	return ggzdmod->max_num_spectators;
}


GGZSeat ggzdmod_get_seat(GGZdMod * ggzdmod, int num)
{
	GGZSeat seat = {.num = num,
			.type = GGZ_SEAT_NONE,
			.name = NULL,
			.fd = -1};
	GGZListEntry *entry;

	if (CHECK_GGZDMOD(ggzdmod) && num >= 0 && num < ggzdmod->num_seats) {
		if ( (entry = ggz_list_search(ggzdmod->seats, &seat)))
			seat = *(GGZSeat*)ggz_list_get_data(entry);
	}
	
	return seat;
}


GGZSpectator ggzdmod_get_spectator(GGZdMod * ggzdmod, int num)
{
	GGZSpectator spectator = {.num = num, .name = NULL, .fd = -1};
	GGZListEntry *entry;

	if (CHECK_GGZDMOD(ggzdmod)
	    && num >= 0
	    && num < ggzdmod->max_num_spectators) {
		if ((entry = ggz_list_search(ggzdmod->spectators, &spectator)))
			spectator = *(GGZSpectator*)ggz_list_get_data(entry);
	}

	return spectator;
}

void* ggzdmod_get_gamedata(GGZdMod * ggzdmod)
{
	if (!CHECK_GGZDMOD(ggzdmod)) {
		return NULL;
	}
	return ggzdmod->gamedata;
}

static void _ggzdmod_set_game(GGZdMod *ggzdmod, const char *game)
{
	ggzdmod->game = ggz_strdup(game);
}

static void _ggzdmod_set_num_seats(GGZdMod *ggzdmod, int num_seats)
{
	int i, old_num;

	old_num = ggzdmod->num_seats;
	
	/* If the table size is increasing, add more seats */
	if (num_seats > old_num) {
		/* Initialize new seats */
		for (i = old_num; i < num_seats; i++) {
			GGZSeat seat = {.num = i,
					.type = GGZ_SEAT_OPEN,
					.name = NULL,
					.fd = -1};
			ggz_list_insert(ggzdmod->seats, &seat);
		}
	} else if (num_seats < old_num) {
		GGZListEntry *entry;
		GGZSeat seat = {.num = num_seats};

		/* Delete extra seats (FIXME: UNTESTED) */
		entry = ggz_list_search(ggzdmod->seats, &seat);
		while (entry) {
			GGZListEntry *next = ggz_list_next(entry);
			ggz_list_delete_entry(ggzdmod->seats, entry);
			entry = next;
		}
	}
	
	ggzdmod->num_seats = num_seats;
}

char* ggzdmod_get_bot_class(GGZdMod *ggzdmod, const char *name)
{
#if 0 /* This doesn't belong in ggzdmod. */
	int ch;
	char *conffile;
	const char *game;
	char *botclass;
	int len;

	/* FIXME: find out which game we play here */
	/* FIXME: Named bots mapping should probably be preloaded before */
	/* FIXME: launch, e.g. in set_module() */
	game = ggzdmod->game;
	if(!game) return NULL;

	len = strlen(GGZDCONFDIR) + strlen("games") + strlen(game) + 7;
	conffile = ggz_malloc(len);
	snprintf(conffile, len, "%s/games/%s.dsc", GGZDCONFDIR, game);
	ch = ggz_conf_parse(conffile, GGZ_CONF_RDONLY);
	if(ch < 0) return NULL;
	ggz_free(conffile);

	botclass = ggz_conf_read_string(ch, "NamedBots", name, NULL);
	ggz_conf_close(ch);

	return botclass;
#endif
	return NULL;
}

static void _ggzdmod_set_max_num_spectators(GGZdMod *ggzdmod,
					    int num_spectators)
{
	GGZSpectator spectator;
	int i, old_num;

	old_num = ggzdmod->max_num_spectators;
	
	/* See the appropriate seat function */
	if (num_spectators > old_num) {
		for (i = old_num; i < num_spectators; i++) {
			spectator.num = i;
			spectator.name = NULL;
			spectator.fd = -1;
			ggz_list_insert(ggzdmod->spectators, &spectator);
		}
	}
	else if (num_spectators < old_num) {
		/* FIXME: delete extra spectators */
	}
	
	ggzdmod->max_num_spectators = num_spectators;
}


int ggzdmod_set_num_seats(GGZdMod * ggzdmod, int num_seats)
{
	/* Check parameters */
	if (!CHECK_GGZDMOD(ggzdmod) || num_seats < 0
	    || ggzdmod->type != GGZDMOD_GGZ) {
		return -1;
	}
	
	_ggzdmod_set_num_seats(ggzdmod, num_seats);

	return 0;
}

void ggzdmod_set_module(GGZdMod * ggzdmod,
                        const char *game, const char *pwd, char **argv)
{
	int i;

	ggz_debug("GGZDMOD", "Setting arguments");
	
	if (!CHECK_GGZDMOD(ggzdmod))
		return;

	if (ggzdmod->type != GGZDMOD_GGZ) {
		_ggzdmod_error(ggzdmod, "Cannot set module args from module");
		return;
	}
		
	/* Check parameters */
	if (!argv || !argv[0]) {
		_ggzdmod_error(ggzdmod, "Bad module arguments");
		return;
	}

	/* Count the number of args so we know how much to allocate */
	for (i = 0; argv[i]; i++) {}

	ggz_debug("GGZDMOD", "Set %d arguments", i);
	
	ggzdmod->argv = ggz_malloc(sizeof(*ggzdmod->argv) * (i + 1));
	ggzdmod->pwd = ggz_strdup(pwd);
	ggzdmod->game = ggz_strdup(game);
	
	for (i = 0; argv[i]; i++) 
		ggzdmod->argv[i] = ggz_strdup(argv[i]);
}


void ggzdmod_set_gamedata(GGZdMod * ggzdmod, void * data)
{
	/* Check parameters */
	if (!CHECK_GGZDMOD(ggzdmod)) {
		return;		/* not very useful */
	}

	ggzdmod->gamedata = data;
}


int ggzdmod_set_handler(GGZdMod * ggzdmod, GGZdModEvent e,
			GGZdModHandler func)
{
	if (!CHECK_GGZDMOD(ggzdmod)) return -1;
	if (e < 0) return -2;
	if (e >= GGZDMOD_NUM_EVENTS) return -3;

	/* Extra checks */
	if (ggzdmod->type == GGZDMOD_GAME) {
		if (e > GGZDMOD_EVENT_ERROR) return -4;
	}

	ggzdmod->handlers[e] = func;
	return 0;
}

static int _ggzdmod_set_seat(GGZdMod * ggzdmod, GGZSeat *seat)
{
	ggz_debug("GGZDMOD", "Seat %d set to type %d (%s)",
		  seat->num, seat->type, seat->name);

	ggz_list_insert(ggzdmod->seats, seat);
	
	return 0;
}


int ggzdmod_set_seat(GGZdMod * ggzdmod, GGZSeat *seat)
{
	GGZSeat oldseat;
	if (!CHECK_GGZDMOD(ggzdmod) || !seat || seat->num < 0
	    || seat->num >= ggzdmod->num_seats) {
		return -2;		
	}

	/* FIXME: check for correctness, (ie. name not NULL for reserved) */
	
	/* If there is no such seat, return error */
	oldseat = ggzdmod_get_seat(ggzdmod, seat->num);
	if (oldseat.type == GGZ_SEAT_NONE)
		return -1;
	
	if (ggzdmod->type == GGZDMOD_GAME) {
		/* The game is not currently allowed to change the type
		   of the seat, but is allowed to change the name and
		   FD for bot players only.  This information is not
		   transmitted back to ggzd. */
		if (seat->type != oldseat.type)
			return -1;
			
		/* we allow changing the fd for bot players.  Note
		   this will cause the FD to be monitored by ggzdmod-game,
		   and a GGZDMOD_EVENT_PLAYER_DATA may then be generated
		   for it. */
		if (oldseat.type != GGZ_SEAT_BOT
		    && seat->fd != oldseat.fd)
			return -1;

		/* we allow games to change the names of Bot players. */
		if (oldseat.type != GGZ_SEAT_BOT
		    && ggz_strcmp(seat->name, oldseat.name))
			return -1;
	}

	/* Note, some other parts of the code assume that if
	   ggzdmod-game changes seat information, the data is not sent back
	   to ggzdmod-ggz.  Specifically, the game server is allowed to change
	   the player FD (if it is -1) and also the name of bot players. */

	/* If we're connected to the game, send a message */
	if (ggzdmod->type == GGZDMOD_GGZ
	    && ggzdmod->state != GGZDMOD_STATE_CREATED) {
		if (_io_send_seat_change(ggzdmod->fd, seat) < 0) {
			/* ggzdmod_error will most likely call
			   ggzdmod_disconnect - but this doesn't free the
			   seat data.  Be careful with this! */
			_ggzdmod_error(ggzdmod,
				       "Error writing seat change to game");
		}

		/* We (GGZ) don't need the fd now (the exact same thing is
		   done for spectators too, see ggzdmod_set_spectator). */
		if (seat->type == GGZ_SEAT_PLAYER) {
			close(seat->fd);
			seat->fd = -1;
		}
	}

	return _ggzdmod_set_seat(ggzdmod, seat);
}

static int _ggzdmod_set_spectator(GGZdMod * ggzdmod, GGZSpectator *spectator)
{
	ggz_debug("GGZDMOD", "Spectator %d set to type 'spectator' (%s)",
		  spectator->num, spectator->name);

	if (spectator->name) {
		if (spectator->num >= ggzdmod->max_num_spectators)
			ggzdmod->max_num_spectators = spectator->num + 1;
		ggz_list_insert(ggzdmod->spectators, spectator);
	} else {
		/* Non-occupied seats are just empty entries in the list. */
		GGZListEntry *entry = ggz_list_search(ggzdmod->spectators,
						      spectator);
		ggz_list_delete_entry(ggzdmod->spectators, entry);
	}

	return 0;
}


int ggzdmod_set_spectator(GGZdMod * ggzdmod, GGZSpectator *spectator)
{
	GGZSpectator oldspectator;

	if (!CHECK_GGZDMOD(ggzdmod) || !spectator) {
		return -1;		
	}

	/* The game currently has no need to change the spectator data. */
	if (ggzdmod->type == GGZDMOD_GAME)
		return -2;

	/* The spectator number must be valid. */
	if (spectator->num < 0)
		return -3;

	/* The name must be null iff the FD is invalid. */
	if ( (spectator->name && spectator->fd < 0)
	     || (!spectator->name && spectator->fd >= 0))
		return -4;

	/* The new spectator must be valid iff the old one isn't. */
	oldspectator = ggzdmod_get_spectator(ggzdmod, spectator->num);
	if ( (oldspectator.name && spectator->name)
	     || (!oldspectator.name && !spectator->name))
		return -5;

	/* If we're connected to the game, send a message */
	if (ggzdmod->type == GGZDMOD_GGZ
	    && ggzdmod->state != GGZDMOD_STATE_CREATED) {
		if (_io_send_spectator_change(ggzdmod->fd, spectator) < 0) {
			_ggzdmod_error(ggzdmod,
				       "Error writing spectator "
				       "change to game");
			return -1;
		}

		/* We (GGZ) don't need the fd now (the exact same thing is
		   done for regular players too, see ggzdmod_set_seat). */
		if (spectator->fd >= 0) {
			close(spectator->fd);
			spectator->fd = -1;
		}
	}

	return _ggzdmod_set_spectator(ggzdmod, spectator);
}



int ggzdmod_reseat(GGZdMod * ggzdmod,
		   int old_seat, int was_spectator,
		   int new_seat, int is_spectator)
{
	const char *name;

	if (!CHECK_GGZDMOD(ggzdmod) || ggzdmod->type != GGZDMOD_GGZ)
		return -1;

	if (old_seat < 0 || new_seat < 0)
		return -2;

	if (was_spectator) {
		name = ggzdmod_get_spectator(ggzdmod, old_seat).name;
		if (old_seat >= ggzdmod->max_num_spectators || !name)
			return -3;
	} else {
		GGZSeatType old_type = ggzdmod_get_seat(ggzdmod,
							old_seat).type;
		name = ggzdmod_get_seat(ggzdmod, old_seat).name;
		if (old_seat >= ggzdmod->num_seats
		    || old_type != GGZ_SEAT_PLAYER
		    || !name)
			return -4;
	}

	if (is_spectator) {
		if (ggzdmod_get_spectator(ggzdmod, new_seat).name)
			return -5;
	} else {
		GGZSeatType new_type = ggzdmod_get_seat(ggzdmod,
							new_seat).type;

		/* I'm not sure if this is correct.  Does this allow seat
		 * reservations to be stolen?  That would be bad. */
		if (new_seat >= ggzdmod->num_seats
		    || !(new_type == GGZ_SEAT_OPEN
			 || new_type == GGZ_SEAT_RESERVED
			 || new_type == GGZ_SEAT_ABANDONED))
			return -6;
	}


	if (_io_send_reseat(ggzdmod->fd,
			    old_seat, was_spectator,
			    new_seat, is_spectator) < 0) {
		_ggzdmod_error(ggzdmod, "ggzdmod_reseat failed");
		return -7;
	}


	name = ggz_strdup(name);

	if (was_spectator) {
		GGZSpectator s = {.num = old_seat,
				  .name = NULL,
				  .fd = -1};
		if (_ggzdmod_set_spectator(ggzdmod, &s) < 0)
			_ggzdmod_error(ggzdmod, "ggzdmod_reseat failed");
	} else {
		GGZSeat s = {.num = old_seat,
			     .name = NULL,
			     .type = GGZ_SEAT_OPEN,
			     .fd = -1};
		const char *name;

		name = ggz_strdup(ggzdmod_get_seat(ggzdmod, old_seat).name);
		if (ggzdmod->state == GGZDMOD_STATE_PLAYING) {
			/* Mark the seat as abandoned rather than open.
			   FIXME - this should probably be done inside ggzd
			   rather than here, which would allow it to be
			   controlled by game options. However this will
			   take some interface changes.  See also the comment
			   in handle_reseat. */
			s.name = name;
			s.type = GGZ_SEAT_ABANDONED;
		}

		if (_ggzdmod_set_seat(ggzdmod, &s) < 0)
			_ggzdmod_error(ggzdmod, "ggzdmod_reseat failed");
		ggz_free(name); /* It's been copied in _ggzdmod_set_seat. */
	}

	if (is_spectator) {
		GGZSpectator s = {.num = new_seat,
				  .name = name,
				  .fd = -1};
		if (_ggzdmod_set_spectator(ggzdmod, &s) < 0)
			_ggzdmod_error(ggzdmod, "ggzdmod_reseat failed");
	} else {
		GGZSeat s = {.num = new_seat,
			     .name = name,
			     .type = GGZ_SEAT_PLAYER,
			     .fd = -1};
		if (_ggzdmod_set_seat(ggzdmod, &s) < 0)
			_ggzdmod_error(ggzdmod, "ggzdmod_reseat failed");
	}

	ggz_free(name);

	return 0;
}


/* This information could be tracked every time  a player
   leaves/joins, but that's not really necessary at this point. */
/* Note also that this function doesn't directly access any
   ggzdmod data at all - it could just as easily be written
   by the game, or included in a wrapper library.  But every
   game we have uses it, so it's here for now... */
int ggzdmod_count_seats(GGZdMod *ggzdmod, GGZSeatType seat_type)
{
	int count = 0;
#if 0 /* This form could be used as part of a wrapper library. */
	int i;
	for (i=0; i<ggzdmod_get_num_seats(mod); i++)
		if (ggzdmod_get_seat(mod, i).type == seat_type)
			count++;
	return count;
#else /* This form is better and must be ggzdmod-internal. */
	GGZListEntry *entry;
	GGZSeat *seat;
	
	if (!CHECK_GGZDMOD(ggzdmod))
		return -1;

	for (entry = ggz_list_head(ggzdmod->seats);
	     entry != NULL;
	     entry = ggz_list_next(entry)) {
		
		seat = ggz_list_get_data(entry);
		if (seat->type == seat_type)
			count++;
	}
	
	return count;
#endif
	/* Note we could do even better if we tracked these
	   values on player join/leave. */
}


/* FIXME: Since the spectator list is dynamic, we could return the size */
int ggzdmod_count_spectators(GGZdMod *ggzdmod)
{
	int i, ret = 0;
	GGZSpectator spectator;

	if (!ggzdmod) return ret;
	for (i = 0; i < ggzdmod->max_num_spectators; i++) {
		spectator = ggzdmod_get_spectator(ggzdmod, i);
		if(spectator.fd != -1) ret++;
	}

	return ret;
}


/* 
 * Event/Data handling
 */

#if 0
/* If anyone is left out or would prefer a different name, go right ahead and 
   change it.  No longer than 13 characters.  --JDS */
static char *bot_names[] = {
	"bcox", "Crouton", "Boffo", "Bugg", "DJH", "Dobey",
	"Dr. Maux", "jDorje", "Jzaun", "Oojah", "Perdig", "RGade",
	"riq", "rpd"
};

/* This function randomizes the order of the names assigned to bots. It is
   ENTIRELY UNNECESSARY but entertaining.  --JDS */
static void randomize_names(char **names, char **randnames, int num)
{
	char *rnames2[num];
	int i, choice;

	/* copy names array to rnames2 array */
	for (i = 0; i < num; i++)
		rnames2[i] = names[i];

	/* now pick names from rnames2 */
	for (i = 0; i < num; i++) {
		choice = random() % (num - i);
		randnames[i] = rnames2[choice];
		rnames2[choice] = rnames2[num - i - 1];
	}
}
#endif

/* FIXME: this assumes that ggzdmod-game is always deciding
   the state, and informing ggzdmod-ggz afterwards.  Once
   ggzd is able to change the state (within limited values),
   this function will no longer be viable and should be
   replaced by (I think) _ggzdmod_set_state, which can be
   called from the network/io code. */
static void set_state(GGZdMod * ggzdmod, GGZdModState state)
{
	GGZdModState old_state = ggzdmod->state;
	if (state == ggzdmod->state)
		return;		/* Is this an error? */

	/* The callback function retrieves the state from ggzdmod_get_state.
	   It could instead be passed as an argument. */
	ggzdmod->state = state;
	call_handler(ggzdmod, GGZDMOD_EVENT_STATE, &old_state);

	/* If we are the game module, send the new state to GGZ */
	if (ggzdmod->type == GGZDMOD_GAME) {
		ggzdmod_log(ggzdmod, "GGZDMOD: Game setting state to %d", 
			    state);
		if (_io_send_state(ggzdmod->fd, state) < 0)
			/* FIXME: do some sort of error handling? */
			return;
	}
}

/* Returns -1 on error, the number of events handled on success. */
static int handle_event(GGZdMod * ggzdmod, fd_set read_fds)
{
	int status, count = 0;
	GGZListEntry *entry;
	GGZSeat *seat;
	GGZSpectator *spectator;

	if (FD_ISSET(ggzdmod->fd, &read_fds)) {
		status = _io_read_data(ggzdmod);
		if (status < 0) {
			_ggzdmod_error(ggzdmod, "Error reading data");
			/* FIXME: should be disconnect? */
			ggzdmod_set_state(ggzdmod, GGZDMOD_STATE_DONE);
		}
		count++;
	}

	/* Only monitor player fds if there is a PLAYER_DATA handler set */
	if (ggzdmod->handlers[GGZDMOD_EVENT_PLAYER_DATA]) {
		for (entry = ggz_list_head(ggzdmod->seats);
		     entry != NULL;
		     entry = ggz_list_next(entry)) {
			
			seat = ggz_list_get_data(entry);
			if (seat->fd != -1 && FD_ISSET(seat->fd, &read_fds)) {
				call_handler(ggzdmod,
					     GGZDMOD_EVENT_PLAYER_DATA,
					     &seat->num);
				
				count++;
			}
		}
	}

	/* The same for spectators */
	if (ggzdmod->handlers[GGZDMOD_EVENT_SPECTATOR_DATA]) {
		for (entry = ggz_list_head(ggzdmod->spectators);
		     entry != NULL;
		     entry = ggz_list_next(entry)) {
			
			spectator = ggz_list_get_data(entry);
			if (spectator->fd != -1 && FD_ISSET(spectator->fd, &read_fds)) {
				call_handler(ggzdmod,
					     GGZDMOD_EVENT_SPECTATOR_DATA,
					     &spectator->num);
				
				count++;
			}
		}
	}

	return count;
}


/* Returns number of events that occured. */
int ggzdmod_dispatch(GGZdMod * ggzdmod)
{
	struct timeval timeout;
	fd_set read_fd_set;
	int status;
	int fd_max;

	if (!CHECK_GGZDMOD(ggzdmod)) {
		return -1;
	}

	/* get_fd_max returns -1 if there are no FD's to be read. */
	fd_max = get_fd_max(ggzdmod);
	if (fd_max < 0) {
		return -1;	
	}

	read_fd_set = get_active_fd_set(ggzdmod);
	timeout.tv_sec = timeout.tv_usec = 0;	/* is this really portable? */
	
	status = select(fd_max + 1, &read_fd_set, NULL, NULL, &timeout);

	/* FIXME: if you don't have a player event handler registered, this
	   function may errantly return 0 sometimes when it should have
	   handled data.  This can be fixed later. */

	if (status == 0) {
		/* Nothing to read. */
		return 0;
	}

	if (status < 0) {
		/* Select error... */
		if (errno == EINTR) {
			/* Select was interrupted (nothing to read).
			 * This may not be possible with a timeout of 0. */
			return 0;
		}
		return -1;
	}

	return handle_event(ggzdmod, read_fd_set);
}

/* unlike the old ggzd_main_loop() this one doesn't connect/disconnect. */
int ggzdmod_loop(GGZdMod * ggzdmod)
{
	if (!CHECK_GGZDMOD(ggzdmod)) {
		return -1;
	}
	while (ggzdmod->state != GGZDMOD_STATE_DONE) {
		fd_set read_fd_set;
		int status, fd_max;

		/* get_fd_max returns -1 if there are no FD's to be read. */
		fd_max = get_fd_max(ggzdmod);
		if (fd_max < 0)
			return -1;
			
		read_fd_set = get_active_fd_set(ggzdmod);

		/* we have to select so that we can determine what file
		   descriptors are waiting to be read. */
		status = select(fd_max + 1, &read_fd_set, NULL, NULL, NULL);
		if (status <= 0) {
			if (errno != EINTR) {
				/* FIXME: handle error */
			}
			continue;
		}

		(void) handle_event(ggzdmod, read_fd_set);
	}
	return 0;		/* should handle errors */
}

/* FIXME

For seats, any change must be done in ggzdmod-ggz.  This means a single point
 where a change can be made.  If the game server wants to change seats it
sends a request to ggzd which is then approved (or not) and the new seat 
data sent back.

For game state, however, it is the opposite.  The change must be done in 
ggzdmod-game.  There is currently no way for the ggzd to change the state 
(maybe there never will be).

However this conflicts.  Some seat changes should depend on the game 
state, and the game state as known by ggzdmod-ggz (which determines seat 
changes) may be out of date.  The solution is that the same mechanism 
should be used for state changes as is used for seat changes.  This may 
affect games however because it means the state change is asynchronous and 
not immediate (there is a state-change callback in ggzdmod, that currently 
gets invoked from inside ggzdmod_set_state; if this were changed then it 
wouldn't be invoked until the message was sent to ggzd and the response 
was received).
*/
int ggzdmod_set_state(GGZdMod * ggzdmod, GGZdModState state)
{
	if (!CHECK_GGZDMOD(ggzdmod))
		return -1;
	
	if (ggzdmod->type == GGZDMOD_GAME) {
		/* The game may only change the state from one of
		   these two states. */
		if (ggzdmod->state != GGZDMOD_STATE_WAITING &&
		    ggzdmod->state != GGZDMOD_STATE_PLAYING)
			return -1;

		/* The game may only change the state to one of
		   these three states. */
		if (state == GGZDMOD_STATE_PLAYING ||
		    state == GGZDMOD_STATE_WAITING ||
		    state == GGZDMOD_STATE_DONE)
			set_state(ggzdmod, state);
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

/* 
 * ggzd specific actions
 */

/* Sends a game launch packet to ggzdmod-game. A negative return value
   indicates a serious (fatal) error. */
/* No locking should be necessary within this function. */
static int send_game_launch(GGZdMod * ggzdmod)
{
	GGZListEntry *entry;
	GGZSeat *seat;
		

	if (_io_send_launch(ggzdmod->fd, ggzdmod->game, ggzdmod->num_seats,
			    ggzdmod->max_num_spectators) < 0) {
		_ggzdmod_error(ggzdmod, "Error writing launch to game");
		return -1;
	}

	for (entry = ggz_list_head(ggzdmod->seats);
	     entry != NULL;
	     entry = ggz_list_next(entry)) {
		
		seat = ggz_list_get_data(entry);
		if (_io_send_seat(ggzdmod->fd, seat) < 0) {
			_ggzdmod_error(ggzdmod,
				       "Error writing launch seats to game");
			return -1;
		}
	}
	
	return 0;
}


static void postfork_error(int fd, const char *fmt, ...)
{
	char buf[4096];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	/* To get things to close cleanly, we send the error message and
	   then set the state to DONE.  This would work better if there
	   was a GGZDMOD_STATE_ERROR instead.  Note that we still can't
	   use ggz_debug and friends. */
	(void) _io_send_log(fd, buf);
	(void) _io_send_state(fd, GGZDMOD_STATE_DONE);
	close(fd);

	exit(EXIT_FAILURE);
}

/* Forks the game.  A negative return value indicates a serious error. */
/* No locking should be necessary within this function. */
static int game_fork(GGZdMod * ggzdmod)
{
	int pid;
	int fd_pair[2];		/* socketpair */

	/* If there are no args, we don't know what to run! */
	if (ggzdmod->argv == NULL || ggzdmod->argv[0] == NULL) {
		_ggzdmod_error(ggzdmod, "No arguments");
		return -1;
	}

	if (socketpair(PF_LOCAL, SOCK_STREAM, 0, fd_pair) < 0)
		ggz_error_sys_exit("socketpair failed");

	if ( (pid = fork()) < 0)
		ggz_error_sys_exit("fork failed");
	else if (pid == 0) {
		/* child */
		close(fd_pair[0]);

		/* After the fork we shouldn't send any debugging messages
		   with ggz_debug or its friends.  We could use ggzdmod_log,
		   but the ggzdmod object isn't set up correctly for that.
		   Thus the function postfork_error() has been
		   written.  --JDS */

		/* Now we copy one end of the socketpair to fd 3 */
		if (fd_pair[1] != 3) {
			/* We'd like to send an error message if either of
			   these fail, but we can't.  --JDS */
			if (dup2(fd_pair[1], 3) != 3)
				postfork_error(fd_pair[1],
					       "ERROR: couldn't dup fd: %s",
					       strerror(errno));
			if (close(fd_pair[1]) < 0)
				postfork_error(3,
					       "ERROR: couldn't close fd: %s",
					       strerror(errno));
		}

		/* It is not necessary to close other fd's if we use
		   CLOSE_ON_EXEC (?).  --JDS */

		/* Set working directory */
		if (ggzdmod->pwd
		    && chdir(ggzdmod->pwd) < 0) {
			/* FIXME: what to do? */
		}

		/* Setup environment variables */
		setenv("GGZMODE", "true", 1);
		setenv("GGZSOCKET", "3", 1);

		/* FIXME: can we call ggzdmod_log() from here? */
		execv(ggzdmod->argv[0], ggzdmod->argv);	/* run game */

		/* We will only get here if the exec fails */
		postfork_error(3, "ERROR: exec of %s failed: %s",
			       ggzdmod->argv[0], strerror(errno));
	} else {
		/* parent */
		close(fd_pair[1]);

		ggzdmod->fd = fd_pair[0];
		ggzdmod->pid = pid;
		
		/* FIXME: should we delete the argv arguments? */
		
		/* That's all! */
	}
	return 0;
}


int ggzdmod_connect(GGZdMod * ggzdmod)
{
	char *ggzsocketstr;
	int ggzsocket;
	int items;

	if (!CHECK_GGZDMOD(ggzdmod)) {
		return -1;
	}

	if (ggzdmod->type == GGZDMOD_GGZ) {
		/* For the ggz side, we fork the game and then send the launch message */
		
		if (game_fork(ggzdmod) < 0) {
			_ggzdmod_error(ggzdmod, "Error: table fork failed");
			return -1;
		}
		
		if (send_game_launch(ggzdmod) < 0) {
			_ggzdmod_error(ggzdmod, "Error sending launch to game");
			/* FIXME: this might result in a forked but unused table. */
			/* FIXME: it might also result in a ggzdmod with
			   initialized but wrong data in it. */
			return -1;
		}
	} else {
		/* For the game side we setup the fd */
		ggzsocket = 3;
		ggzsocketstr = getenv("GGZSOCKET");
		if(ggzsocketstr) {
			items = sscanf(ggzsocketstr, "%d", &ggzsocket);
			if (items == 0) {
				ggzsocket = 3;
			}
		}
		ggzdmod->fd = ggzsocket;
		
		if (ggzdmod_log(ggzdmod, "GGZDMOD: Connecting to GGZ server.") < 0) {
			ggzdmod->fd = -1;
			/* I'm not entirely sure if we _should_ be printing
			   this stuff here, but it certainly is convenient
			   to not have to do it for every game. */
			fprintf(stderr, "\nCouldn't connect to GGZ!\n\n"
					"Most likely this occurred because you ran\n"
					"the server from the command line.  Instead\n"
					"you should connect to a GGZ server and\n"
					"launch a game through the client.\n\n");
			return -1;
		}
	}
	
	/* We used to return the GGZdmod file descriptor, we really
	   don't even want to tempt games to use this socket. */
	return 0; /* success */
}

int ggzdmod_disconnect(GGZdMod * ggzdmod)
{
	if (!CHECK_GGZDMOD(ggzdmod) || ggzdmod->fd < 0 ) {
		return -1;
	}

	if (ggzdmod->type == GGZDMOD_GGZ) {
		/* For the ggz side, we kill the game server and close the socket */
		
		/* Make sure game server is dead */
		if (ggzdmod->pid > 0) {
			kill(ggzdmod->pid, SIGINT);
			waitpid(ggzdmod->pid, NULL, 0);
		}
		ggzdmod->pid = -1;
		
		set_state(ggzdmod, GGZDMOD_STATE_DONE);
		/* FIXME: what other cleanups should we do? */
	} else {
		/* For client the game side we send a game over message */
		
		/* First warn the server of halt (if we haven't already) */
		ggzdmod_set_state(ggzdmod, GGZDMOD_STATE_DONE);
		ggzdmod_log(ggzdmod, "GGZDMOD: Disconnected from GGZ server.");
	}
	
	/* We no longer free the seat data here.  It will stick around until
	   ggzdmod_free is called or it is used again.  This is important
	   because we don't know who the callers are or if they expect to
	   have valid seat data to look at (see ggzdmod_set_seat). */

	/* Clean up the ggzdmod object.  In theory it could now reconnect for
	   a new game. */
	close(ggzdmod->fd);
	ggzdmod->fd = -1;

	return 0;
}


/* 
 * module specific actions
 */

int ggzdmod_log(GGZdMod * ggzdmod, const char *fmt, ...)
{
	char buf[4096];
	va_list ap;

	if (!CHECK_GGZDMOD(ggzdmod) || !fmt ||
	    (ggzdmod->type == GGZDMOD_GAME && ggzdmod->fd < 0)) {
		/* This will happen when ggzdmod_log is called before
		   connection.  We could store the buffer for later, but... */
		return -1;
	}

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	
	if (ggzdmod->type == GGZDMOD_GAME) {
		if (_io_send_log(ggzdmod->fd, buf) < 0) {
			_ggzdmod_error(ggzdmod, "Error writing to GGZ");
			return -1;
		}
	} else {
		/* This is here mainly so that ggzdmod can use
		   ggzdmod_log internally. */
		_ggzdmod_handle_log(ggzdmod, buf);
	}

	return 0;
}


/**** Internal library functions ****/

void _ggzdmod_error(GGZdMod *ggzdmod, char* error)
{
	call_handler(ggzdmod, GGZDMOD_EVENT_ERROR, error);
}


void _ggzdmod_handle_state(GGZdMod * ggzdmod, GGZdModState state)
{
	_io_respond_state(ggzdmod->fd);

	/* There's only certain ones the game is allowed to set it to,
	   and they can only change it if the state is currently
	   WAITING or PLAYING. */
	switch (state) {
	case GGZDMOD_STATE_WAITING:
	case GGZDMOD_STATE_PLAYING:
	case GGZDMOD_STATE_DONE:
		/* In contradiction to what I say above, the game
		   actually _is_ allowed to change its state from
		   CREATED to WAITING.  When ggzdmod-ggz sends a
		   launch packet to ggzdmod-game, ggzdmod-game
		   automatically changes the state from CREATED
		   to WAITING.  When this happens, it tells
		   ggzdmod-ggz of this change and we end up back
		   here.  So, although it's a bit unsafe, we have
		   to allow this for now.  The alternative would
		   be to have ggzdmod-ggz and ggzdmod-game both
		   separately change states when the launch packet
		   is sent. */
		set_state(ggzdmod, state);
		/* Is this right? has the gameover happened yet? */   
		return;
	case GGZDMOD_STATE_CREATED:
		break;
	}

	_ggzdmod_error(ggzdmod, "Game requested incorrect state value");
}


void _ggzdmod_handle_log(GGZdMod * ggzdmod, char *msg)
{
	call_handler(ggzdmod, GGZDMOD_EVENT_LOG, msg);
}


/* Game-side event: launch event received from ggzd */
void _ggzdmod_handle_launch_begin(GGZdMod * ggzdmod, const char *game, int num_seats, int num_spectators)
{
#if 0
	int bots = 0;
#define NUM_BOT_NAMES (sizeof(bot_names)/sizeof(bot_names[0]))
	char *rand_bot_names[NUM_BOT_NAMES];

	randomize_names(bot_names, rand_bot_names, NUM_BOT_NAMES);
#endif

	if ((num_seats < 0) || (num_spectators < 0) || (num_seats + num_spectators <= 0)) {
		_ggzdmod_error(ggzdmod, "Error: zero seats and spectators received from GGZ");
		ggzdmod_log(ggzdmod,
			    "GGZDMOD: ERROR: %d seats, %d spectators sent upon launch.",
			    num_seats, num_spectators);
		return;
	}

	_ggzdmod_set_game(ggzdmod, game);
	_ggzdmod_set_num_seats(ggzdmod, num_seats);
	_ggzdmod_set_max_num_spectators(ggzdmod, num_spectators);
}


void _ggzdmod_handle_launch_seat(GGZdMod * ggzdmod, GGZSeat seat)
{
	int status = -1;

	switch (seat.type) {
	case GGZ_SEAT_OPEN:
		ggzdmod_log(ggzdmod, "GGZDMOD: Seat %d is open", seat.num);
		status = 0;
		break;

	case GGZ_SEAT_BOT:
		/* Eventually ggzd may give us AI player names.  In the
		   meantime there's just a placeholder. */
		if(!seat.name)
			seat.name = "AI";
#if 0		
		len = strlen(rand_bot_names[bots]) + 4;
		seat.name = ggz_malloc(len);

		snprintf(seat.name, len, "%s-AI",
			 rand_bot_names[bots]);
		bots++;
#endif
		ggzdmod_log(ggzdmod, "GGZDMOD: Seat %d is a bot named %s",
			    seat.num, seat.name);
		status = 0;
		break;

	case GGZ_SEAT_RESERVED:
		ggzdmod_log(ggzdmod, "GGZDMOD: Seat %d reserved for %s",
			    seat.num, seat.name);
		status = 0;
		break;
	case GGZ_SEAT_NONE:
	case GGZ_SEAT_PLAYER:
	case GGZ_SEAT_ABANDONED:
		break;
	}


	if (status < 0) {
		_ggzdmod_error(ggzdmod,
			       "Error: received unknown seat from GGZ");
		ggzdmod_log(ggzdmod, "GGZDMOD: Unknown seat type %d",
			    seat.type);
		return;
	}

	if (_ggzdmod_set_seat(ggzdmod, &seat) < 0) {
		_ggzdmod_error(ggzdmod, "Error setting seat");
		ggzdmod_log(ggzdmod, "GGZDMOD: Error setting seat");
		seat_print(ggzdmod, &seat);
	}
}


void _ggzdmod_handle_launch_end(GGZdMod * ggzdmod)
{
	/* Normally we let the game control its own state, but
	   we control the transition from CREATED to WAITING. */
        set_state(ggzdmod, GGZDMOD_STATE_WAITING);
}


/* game-side event: seat change event received from ggzd.  This code is
   virtually identical to _ggzdmod_handle_join */
void _ggzdmod_handle_seat(GGZdMod * ggzdmod, GGZSeat *seat)
{
	GGZSeat old = ggzdmod_get_seat(ggzdmod, seat->num);
	GGZSeat *old_seat = seat_copy(&old);
	GGZdModEvent event;

	/* Tag on AI name.  See _ggzdmod_handle_launch_seat. */
	if (seat->type == GGZ_SEAT_BOT && !seat->name)
		seat->name = ggz_strdup("AI");

	/* Place the new seat into the list */
	_ggzdmod_set_seat(ggzdmod, seat);

	/* Figure out which event to use. */
	if (seat->type == GGZ_SEAT_PLAYER
	    && (old_seat->type == GGZ_SEAT_OPEN 
		|| old_seat->type == GGZ_SEAT_RESERVED
		|| old_seat->type == GGZ_SEAT_ABANDONED))
		event = GGZDMOD_EVENT_JOIN;
	else if (seat->type == GGZ_SEAT_OPEN
		 && old_seat->type == GGZ_SEAT_PLAYER)
		event = GGZDMOD_EVENT_LEAVE;
	else {
		event = GGZDMOD_EVENT_SEAT;
	}

	/* Invoke the handler. */
	call_handler(ggzdmod, event, old_seat);

	/* Free old_seat */
	seat_free(old_seat);
}


void _ggzdmod_handle_reseat(GGZdMod * ggzdmod,
			    int old_seat, int was_spectator,
			    int new_seat, int is_spectator)
{
	char *name;
	int fd;
	GGZListEntry *entry;
	void *old_old, *new_old;
	GGZdModEvent old_event, new_event;

	/* Change the old seat, and dup the previous value */
	if (was_spectator) {
		GGZSpectator s = {.num = old_seat,
				  .name = NULL,
				  .fd = -1};
		GGZSpectator *old;

		/* We have to manually pull off the FD to prevent it
		   from being closed */
		entry = ggz_list_search(ggzdmod->spectators, &s);
		old = ggz_list_get_data(entry);
		fd = old->fd;
		old->fd = -1;

		name = ggz_strdup(old->name);

		old_old = spectator_copy(old);
		_ggzdmod_set_spectator(ggzdmod, &s);
		old_event = GGZDMOD_EVENT_SPECTATOR_LEAVE;
	} else {
		GGZSeat s = {.num = old_seat,
			     .name = NULL,
			     .type = GGZ_SEAT_OPEN,
			     .fd = -1};
		GGZSeat *old;

		/* We have to manually pull of the FD to prevent it
		   from being closed */
		entry = ggz_list_search(ggzdmod->seats, &s);
		old = ggz_list_get_data(entry);
		fd = old->fd;
		old->fd = -1;

		name = ggz_strdup(old->name);

		if (ggzdmod->state == GGZDMOD_STATE_PLAYING) {
			/* Mark the seat as abandoned rather than open.
			   FIXME - rather than have ggzdmod-game deduce this
			   on its own, ggzdmod-ggz should instead send the
			   full value of the new seat.  See also the comment
			   in ggzdmod_reseat. */
			s.name = name;
			s.type = GGZ_SEAT_ABANDONED;
		}

		old_old = seat_copy(old);
		_ggzdmod_set_seat(ggzdmod, &s);
		old_event = GGZDMOD_EVENT_LEAVE;
	}

	/* Change the new seat, and dup the preevious value */
	if (is_spectator) {
		GGZSpectator s = {.num = new_seat,
				  .name = name,
				  .fd = fd};
		GGZSpectator old = ggzdmod_get_spectator(ggzdmod, new_seat);

		new_old = spectator_copy(&old);
		_ggzdmod_set_spectator(ggzdmod, &s);
		new_event = GGZDMOD_EVENT_SPECTATOR_JOIN;
	} else {
		GGZSeat s = {.num = new_seat,
			     .name = name,
			     .type = GGZ_SEAT_PLAYER,
			     .fd = fd};
		GGZSeat old = ggzdmod_get_seat(ggzdmod, new_seat);

		new_old = seat_copy(&old);
		_ggzdmod_set_seat(ggzdmod, &s);
		new_event = GGZDMOD_EVENT_JOIN;
	}

	/* Since the events are sent asynchronously, there could be
	   problems for games. */
	call_handler(ggzdmod, old_event, old_old);
	call_handler(ggzdmod, new_event, new_old);

	ggz_free(name);

	if (was_spectator) {
		spectator_free(old_old);
	} else {
		seat_free(old_old);
	}

	if (is_spectator) {
		spectator_free(new_old);
	} else {
		seat_free(new_old);
	}
}

/* game-side event: spectator change event received from ggzd.  */
void _ggzdmod_handle_spectator_seat(GGZdMod * ggzdmod, GGZSpectator *seat)
{
	GGZSpectator old = ggzdmod_get_spectator(ggzdmod, seat->num);
	GGZSpectator *old_seat = spectator_copy(&old);
	GGZdModEvent event;

	/* Increase max_num_spectators, if necessary. */
	_ggzdmod_set_spectator(ggzdmod, seat);

	/* Figure out which event to use. */
	if (seat->name && !old_seat->name)
		event = GGZDMOD_EVENT_SPECTATOR_JOIN;
	else if (!seat->name && old_seat->name)
		event = GGZDMOD_EVENT_SPECTATOR_LEAVE;
	else {
		event = GGZDMOD_EVENT_SPECTATOR_SEAT;
	}

	/* Invoke the handler. */
	call_handler(ggzdmod, event, old_seat);

	/* Free old_seat */
	spectator_free(old_seat);
}


/* game-side event: game state response received from ggzd */
void _ggzdmod_handle_state_response(GGZdMod * ggzdmod)
{
	/* FIXME: what do we do here? */
}


/* Create a new copy of a seat object */
static GGZSeat* seat_copy(GGZSeat *orig)
{
	GGZSeat *seat;

	seat = ggz_malloc(sizeof(GGZSeat));

	seat->num = orig->num;
	seat->type = orig->type;
	seat->fd = orig->fd;
	
	/* The name may be NULL if unknown. */
	seat->name = orig->name ? ggz_strdup(orig->name) : NULL;
	
	return seat;
}


static int seat_compare(GGZSeat *a, GGZSeat *b)
{
	return a->num - b->num;
}


static void seat_free(GGZSeat *seat)
{
	if (seat->fd != -1)
		close(seat->fd);
	if (seat->name)
		ggz_free(seat->name);

	ggz_free(seat);
}

static void seat_print(GGZdMod * ggzdmod, GGZSeat * seat)
{
	const char *type = ggz_seattype_to_string(seat->type);
	ggzdmod_log(ggzdmod, "GGZDMOD: Seat %d is %s (%s) on %d",
		    seat->num, type, seat->name, seat->fd);
}


/* Create a new copy of a spectator object */
static GGZSpectator* spectator_copy(GGZSpectator *orig)
{
	GGZSpectator *spectator;

	spectator = ggz_malloc(sizeof(GGZSpectator));

	spectator->type = GGZ_SEAT_NONE;
	spectator->num = orig->num;
	spectator->fd = orig->fd;
	
	/* The name may be NULL if unknown. */
	spectator->name = orig->name ? ggz_strdup(orig->name) : NULL;
	
	return spectator;
}


static int spectator_compare(GGZSpectator *a, GGZSpectator *b)
{
	return a->num - b->num;
}


static void spectator_free(GGZSpectator *spectator)
{
	if (spectator->fd != -1)
		close(spectator->fd);
	if (spectator->name)
		ggz_free(spectator->name);

	ggz_free(spectator);
}


void ggzdmod_check(GGZdMod *ggzdmod)
{
	GGZListEntry *entry;
	if (!CHECK_GGZDMOD(ggzdmod))
		return;
		
	ggzdmod_log(ggzdmod, "--- GGZDMOD CHECK ---");
	ggzdmod_log(ggzdmod, "    TYPE: %s",
		    ggzdmod->type == GGZDMOD_GAME ? "GGZDMOD_GAME" :
		    ggzdmod->type == GGZDMOD_GGZ ?  "GGZDMOD_GGZ"  :
		    "UNKNOWN");
	ggzdmod_log(ggzdmod, "    FD: %d.", ggzdmod->fd);

	for (entry = ggz_list_head(ggzdmod->seats);
	     entry != NULL;
	     entry = ggz_list_next(entry)) {
		
		GGZSeat *seat = ggz_list_get_data(entry);
	
		ggzdmod_log(ggzdmod, "    Seat %d | type=%s | name=%s | fd=%d",
			    seat->num,
			    ggz_seattype_to_string(seat->type),
			    seat->name ? seat->name : "<none>",
			    seat->fd);
		
	}
	
	ggzdmod_log(ggzdmod, "--- GGZDMOD CHECK COMPLETE ---");
}


void ggzdmod_report_game(GGZdMod *ggzdmod,
			 int *the_teams,
			 GGZGameResult *the_results, int *the_scores)
{
	if (ggzdmod && ggzdmod->type == GGZDMOD_GAME) {
		GGZSeatType types[ggzdmod->num_seats];
		const char *names[ggzdmod->num_seats];
		int teams[ggzdmod->num_seats];
		GGZGameResult results[ggzdmod->num_seats];
		int scores[ggzdmod->num_seats];
		int p, r = 0;

		/* Currently the values reported correspond to the seats
		 * currently at the table.  However this could be changed
		 * to allow the game to give us this info. */
		for (p = 0; p < ggzdmod->num_seats; p++) {
			GGZSeat seat = ggzdmod_get_seat(ggzdmod, p);
			int usable = 0;

			switch (seat.type) {
			case GGZ_SEAT_PLAYER:
			case GGZ_SEAT_BOT:
			case GGZ_SEAT_ABANDONED:
				usable = 1;
				break;
			case GGZ_SEAT_RESERVED:
			case GGZ_SEAT_OPEN:
			case GGZ_SEAT_NONE:
				break;
			}
			/* An ugly switch is used so if a new type is added
			 * we'll get a warning. */
			if (!usable) {
				continue;
			}

			types[r] = seat.type;
			names[r] = seat.name;
			if (seat.type == GGZ_SEAT_BOT
			    && (!names[r] || names[r][0] == '\0')) {
				/* Game may have already set the name; if not
				   we do it here. */
				names[r] = "AI";
			}
			teams[r] = the_teams ? the_teams[p] : r;
			results[r] = the_results[p];
			scores[r] = the_scores ? the_scores[p] : 0;
			r++;
		}

		_io_send_game_report(ggzdmod->fd, r,
				     names, types, teams, results, scores);
	}
}


void ggzdmod_report_savegame(GGZdMod *ggzdmod, const char *savegame)
{
	if (ggzdmod && ggzdmod->type == GGZDMOD_GAME) {
		if (!savegame) return;

		_io_send_savegame_report(ggzdmod->fd, savegame);
	}
}


void ggzdmod_request_num_seats(GGZdMod * ggzdmod, int num_seats)
{
	_io_send_req_num_seats(ggzdmod->fd, num_seats);
}


void ggzdmod_request_boot(GGZdMod * ggzdmod, const char *name)
{
	_io_send_req_boot(ggzdmod->fd, name);
}


void ggzdmod_request_bot(GGZdMod * ggzdmod, int seat_num)
{
	_io_send_req_bot(ggzdmod->fd, seat_num);
}


void ggzdmod_request_open(GGZdMod * ggzdmod, int seat_num)
{
	_io_send_req_open(ggzdmod->fd, seat_num);
}


void _ggzdmod_handle_report(GGZdMod * ggzdmod,
			    int num_players, char **names, GGZSeatType *types,
			    int *teams, GGZGameResult *results, int *scores)
{
	GGZdModGameReportData data = {.num_players = num_players,
				      .names = names,
				      .types = types,
				      .teams = teams,
				      .results = results,
				      .scores = scores};
	call_handler(ggzdmod, GGZDMOD_EVENT_GAMEREPORT, &data);
}


void _ggzdmod_handle_savegame(GGZdMod * ggzdmod, char *savegame)
{
	call_handler(ggzdmod, GGZDMOD_EVENT_SAVEGAMEREPORT, savegame);
}


void _ggzdmod_handle_num_seats_request(GGZdMod * ggzdmod, int num_seats)
{
	call_handler(ggzdmod, GGZDMOD_EVENT_REQ_NUM_SEATS, &num_seats);
}


void _ggzdmod_handle_boot_request(GGZdMod *ggzdmod, char *name)
{
	call_handler(ggzdmod, GGZDMOD_EVENT_REQ_BOOT, name);
}


void _ggzdmod_handle_bot_request(GGZdMod *ggzdmod, int seat_num)
{
	call_handler(ggzdmod, GGZDMOD_EVENT_REQ_BOT, &seat_num);
}


void _ggzdmod_handle_open_request(GGZdMod *ggzdmod, int seat_num)
{
	call_handler(ggzdmod, GGZDMOD_EVENT_REQ_OPEN, &seat_num);
}
