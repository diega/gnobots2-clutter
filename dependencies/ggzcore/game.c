/*
 * File: game.c
 * Author: Brent Hendricks
 * Project: GGZ Core Client Lib
 * Date: 2/28/2001
 * $Id$
 *
 * This fils contains functions for handling games being played
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

#ifdef HAVE_CONFIG_H
#  include <config.h>	/* Site-specific config */
#endif

#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#ifdef HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif
#include <sys/stat.h>
#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
#ifdef HAVE_WINSOCK2_H
# include <winsock2.h>
#endif
#include <unistd.h>

#include <ggz.h>

#include "ggzmod-ggz.h"

#include "game.h"
#include "ggzcore.h"
#include "hook.h"
#include "module.h"
#include "net.h"
#include "room.h"
#include "server.h"
#include "table.h"

/* Local functions */
static int _ggzcore_game_event_is_valid(GGZGameEvent event);
static GGZHookReturn _ggzcore_game_event(struct _GGZGame *game,
					 GGZGameEvent id, void *data);
#if 0	/* currently unused */
static char *_ggzcore_game_get_path(char **argv);
#endif


/* Array of GGZGame messages */
static char *_ggzcore_game_events[] = {
	"GGZ_GAME_LAUNCHED",
	"GGZ_GAME_LAUNCH_FAIL",
	"GGZ_GAME_NEGOTIATED",
	"GGZ_GAME_NEGOTIATE_FAIL",
	"GGZ_GAME_PLAYING",
	"GGZ_GAME_OVER",
	"GGZ_GAME_IO_ERROR",
	"GGZ_GAME_PROTO_ERROR"
};

/* Total number of server events messages */
static unsigned int _ggzcore_num_events =
    sizeof(_ggzcore_game_events) / sizeof(_ggzcore_game_events[0]);


/*
 * The GGZGame object manages information about a particular running game
 */
struct _GGZGame {

	/* Pointer to module this game is playing */
	GGZModule *module;

	/* Room events */
	GGZHookList *event_hooks[sizeof(_ggzcore_game_events) /
				 sizeof(_ggzcore_game_events[0])];

	/* GGZ Game module connection */
	GGZMod *client;

	/* The server data for this game. */
	GGZServer *server;

	/* Are we playing or watching at our table? */
	int spectating;

	/* What's our seat number? */
	int seat_num;

	/* Which room this game is in. */
	int room_id;

	/* The table ID for this game. */
	int table_id;
};


static void _ggzcore_game_handle_state(GGZMod * mod, GGZModEvent event,
				       const void *data);
static void _ggzcore_game_handle_sit(GGZMod * mod, GGZModTransaction t,
				     const void *data);
static void _ggzcore_game_handle_stand(GGZMod * mod, GGZModTransaction t,
				       const void *data);
static void _ggzcore_game_handle_boot(GGZMod * mod, GGZModTransaction t,
				      const void *data);
static void _ggzcore_game_handle_seatchange(GGZMod * mod,
					    GGZModTransaction t,
					    const void *data);
static void _ggzcore_game_handle_chat(GGZMod * mod, GGZModTransaction t,
				      const void *data);
static void _ggzcore_game_handle_info(GGZMod * mod, GGZModTransaction t,
				      const void *data);


/* Publicly exported functions */

GGZGame *ggzcore_game_new(void)
{
	return _ggzcore_game_new();
}


int ggzcore_game_init(GGZGame * game, GGZServer * server,
		      GGZModule * module)
{
	if (!game || !server || !_ggzcore_server_get_cur_room(server)
	    || _ggzcore_server_get_cur_game(server))
		return -1;

	if (!module && !_ggzcore_module_is_embedded())
		return -1;

	_ggzcore_game_init(game, server, module);

	return 0;
}


void ggzcore_game_free(GGZGame * game)
{
	if (game)
		_ggzcore_game_free(game);
}


/* Functions for attaching hooks to GGZGame events */
int ggzcore_game_add_event_hook(GGZGame * game,
				const GGZGameEvent event,
				const GGZHookFunc func)
{
	if (game && _ggzcore_game_event_is_valid(event)
	    && game->event_hooks[event])
		return _ggzcore_game_add_event_hook_full(game, event, func,
							 NULL);
	else
		return -1;
}


int ggzcore_game_add_event_hook_full(GGZGame * game,
				     const GGZGameEvent event,
				     const GGZHookFunc func,
				     const void *data)
{
	if (game && _ggzcore_game_event_is_valid(event)
	    && game->event_hooks[event])
		return _ggzcore_game_add_event_hook_full(game, event, func,
							 data);
	else
		return -1;
}


/* Functions for removing hooks from GGZGame events */
int ggzcore_game_remove_event_hook(GGZGame * game,
				   const GGZGameEvent event,
				   const GGZHookFunc func)
{
	if (game && _ggzcore_game_event_is_valid(event)
	    && game->event_hooks[event])
		return _ggzcore_game_remove_event_hook(game, event, func);
	else
		return -1;
}


int ggzcore_game_remove_event_hook_id(GGZGame * game,
				      const GGZGameEvent event,
				      const unsigned int hook_id)
{
	if (game && _ggzcore_game_event_is_valid(event)
	    && game->event_hooks[event])
		return _ggzcore_game_remove_event_hook_id(game, event,
							  hook_id);
	else
		return -1;
}


int ggzcore_game_get_control_fd(GGZGame * game)
{
	if (game)
		return _ggzcore_game_get_control_fd(game);
	else
		return -1;
}


int ggzcore_game_read_data(GGZGame * game)
{
	if (game)
		return _ggzcore_game_read_data(game);
	else
		return -1;
}


void ggzcore_game_set_server_fd(GGZGame *game, unsigned int fd)
{
	if (game)
		return _ggzcore_game_set_server_fd(game, fd);
}


GGZModule *ggzcore_game_get_module(GGZGame * game)
{
	if (game)
		return _ggzcore_game_get_module(game);
	else
		return NULL;
}


int ggzcore_game_launch(GGZGame * game)
{
	if (game && (game->module || _ggzcore_module_is_embedded()))
		return _ggzcore_game_launch(game);
	else
		return -1;
}


/* 
 * Internal library functions (prototypes in room.h) 
 * NOTE:All of these functions assume valid inputs!
 */


struct _GGZGame *_ggzcore_game_new(void)
{
	struct _GGZGame *game;

	game = ggz_malloc(sizeof(struct _GGZGame));

	return game;
}


void _ggzcore_game_init(struct _GGZGame *game,
			GGZServer * server, GGZModule * module)
{
	int i;
	GGZRoom *room = _ggzcore_server_get_cur_room(server);

	game->server = server;
	game->room_id = _ggzcore_room_get_id(room);
	game->table_id = -1;

	_ggzcore_server_set_cur_game(server, game);

	game->module = module;

	ggz_debug(GGZCORE_DBG_GAME, "Initializing new game");

	/* Setup event hook list */
	for (i = 0; i < _ggzcore_num_events; i++)
		game->event_hooks[i] = _ggzcore_hook_list_init(i);

	/* Setup client module connection */
	game->client = ggzmod_ggz_new(GGZMOD_GGZ);
	ggzmod_ggz_set_gamedata(game->client, game);
#ifdef HAVE_WINSOCK2_H
	ggzmod_ggz_set_server_host(game->client,
			       ggzcore_server_get_host(server),
			       ggzcore_server_get_port(server),
			       ggzcore_server_get_handle(server));
#endif
	ggzmod_ggz_set_handler(game->client, GGZMOD_EVENT_STATE,
			   &_ggzcore_game_handle_state);
	ggzmod_ggz_set_transaction_handler(game->client,
				       GGZMOD_TRANSACTION_SIT,
				       _ggzcore_game_handle_sit);
	ggzmod_ggz_set_transaction_handler(game->client,
				       GGZMOD_TRANSACTION_STAND,
				       _ggzcore_game_handle_stand);
	ggzmod_ggz_set_transaction_handler(game->client,
				       GGZMOD_TRANSACTION_BOOT,
				       _ggzcore_game_handle_boot);
	ggzmod_ggz_set_transaction_handler(game->client,
				       GGZMOD_TRANSACTION_OPEN,
				       _ggzcore_game_handle_seatchange);
	ggzmod_ggz_set_transaction_handler(game->client,
				       GGZMOD_TRANSACTION_BOT,
				       _ggzcore_game_handle_seatchange);
	ggzmod_ggz_set_transaction_handler(game->client,
				       GGZMOD_TRANSACTION_CHAT,
				       _ggzcore_game_handle_chat);
	ggzmod_ggz_set_transaction_handler(game->client,
				       GGZMOD_TRANSACTION_INFO,
				       _ggzcore_game_handle_info);
	ggzmod_ggz_set_player(game->client,
			  _ggzcore_server_get_handle(server), 0, -1);

	if (!_ggzcore_module_is_embedded())
		ggzmod_ggz_set_module(game->client, NULL,
				  _ggzcore_module_get_argv(game->module));
}

static void _ggzcore_game_send_player_stats(GGZGame *game)
{
	GGZRoom *room = _ggzcore_server_get_nth_room(game->server,
						     game->room_id);
	GGZTable *table = ggzcore_room_get_table_by_id(room, game->table_id);
	const int num_players = ggzcore_table_get_num_seats(table);
	const int num_spectators
	  = ggzcore_table_get_num_spectator_seats(table);
	int i;
	GGZStat stats[num_players < 0 ? 0 : num_players];
	GGZStat sstats[num_spectators < 0 ? 0 : num_spectators];

	memset(stats, 0, sizeof(stats));
	memset(sstats, 0, sizeof(sstats));

	/* FIXME: this is inefficient in that it has to resend all stats
	 * every time.  As long as tables aren't too big this is
	 * fine.  But it won't scale. */
	for (i = 0; i < num_players; i++) {
		GGZTableSeat seat = _ggzcore_table_get_nth_seat(table, i);
		GGZPlayer *player;

		/* TODO: What about RESERVED/ABANDONED? */
		if (seat.type == GGZ_SEAT_PLAYER
		    && (player
			= _ggzcore_room_get_player_by_name(room, seat.name))) {
			if (_ggzcore_player_get_record(player, &stats[i].wins,
						       &stats[i].losses,
						       &stats[i].ties,
						       &stats[i].forfeits)) {
				stats[i].have_record = 1;
			}
			if (_ggzcore_player_get_rating(player,
						       &stats[i].rating)) {
				stats[i].have_rating = 1;
			}
			if (_ggzcore_player_get_ranking(player,
							&stats[i].ranking)) {
				stats[i].have_ranking = 1;
			}
			if (_ggzcore_player_get_highscore(player,
						&stats[i].highscore)) {
				stats[i].have_highscore = 1;
			}
		}
	}

	for (i = 0; i < num_spectators; i++) {
		const char *name
		  = ggzcore_table_get_nth_spectator_name(table, i);
		GGZPlayer *player
		  = _ggzcore_room_get_player_by_name(room, name);

		/* TODO: What about RESERVED/ABANDONED? */
		if (player) {
			if (_ggzcore_player_get_record(player, &sstats[i].wins,
						       &sstats[i].losses,
						       &sstats[i].ties,
						       &sstats[i].forfeits)) {
				sstats[i].have_record = 1;
			}
			if (_ggzcore_player_get_rating(player,
						       &sstats[i].rating)) {
				sstats[i].have_rating = 1;
			}
			if (_ggzcore_player_get_ranking(player,
							&sstats[i].ranking)) {
				sstats[i].have_ranking = 1;
			}
			if (_ggzcore_player_get_highscore(player,
							  &sstats[i].highscore)) {
				sstats[i].have_highscore = 1;
			}
		}
	}

	ggzmod_ggz_set_stats(game->client, stats, sstats);
}


/* This function is called by ggzmod when the game state is changed.
 *
 * Game state changes are all initiated by the game client through ggzmod.
 * So if we get here we just have to update ggzcore and the ggz client based
 * on what changes have already happened. */
static void _ggzcore_game_handle_state(GGZMod * mod, GGZModEvent event,
				       const void *data)
{
	GGZGame *game = ggzmod_ggz_get_gamedata(mod);
	const GGZModState new = ggzmod_ggz_get_state(mod);
	const GGZModState *const prev = data;

	ggz_debug(GGZCORE_DBG_GAME, "Game changing from state %d to %d",
		  *prev, new);

	switch (*prev) {
	case GGZMOD_STATE_CREATED:
		ggz_debug(GGZCORE_DBG_GAME, "game negotiated");
		_ggzcore_game_send_player_stats(game);
		_ggzcore_game_event(game, GGZ_GAME_NEGOTIATED, NULL);
		if (new != GGZMOD_STATE_CONNECTED) {
			ggz_error_msg("Game changed state from created "
				      "to %d.", new);
		}
		break;
	case GGZMOD_STATE_CONNECTED:
		ggz_debug(GGZCORE_DBG_GAME, "game playing");
		_ggzcore_game_event(game, GGZ_GAME_PLAYING, NULL);
		if (new != GGZMOD_STATE_WAITING
		    && new != GGZMOD_STATE_PLAYING) {
			ggz_error_msg("Game changed state from connected "
				      "to %d.", new);
		}
		break;
	case GGZMOD_STATE_WAITING:
	case GGZMOD_STATE_PLAYING:
	case GGZMOD_STATE_DONE:
		break;
	}

	switch (new) {
	case GGZMOD_STATE_CONNECTED:
	case GGZMOD_STATE_WAITING:
	case GGZMOD_STATE_PLAYING:
	case GGZMOD_STATE_DONE:
		break;

	case GGZMOD_STATE_CREATED:
		/* Leave the game running. This should never happen since
		 * this is the initial state and we never return to it after
		 * leaving it. */
		ggz_error_msg("Game state changed to created!");
		break;

	}
}


static void _ggzcore_game_handle_sit(GGZMod * mod, GGZModTransaction t,
				     const void *data)
{
	GGZGame *game = ggzmod_ggz_get_gamedata(mod);
	GGZNet *net = _ggzcore_server_get_net(game->server);
	const int *seat_num = data;
	GGZReseatType op;

	if (game->spectating)
		op = GGZ_RESEAT_SIT;
	else
		op = GGZ_RESEAT_MOVE;

	_ggzcore_net_send_table_reseat(net, op, *seat_num);
}


static void _ggzcore_game_handle_stand(GGZMod * mod, GGZModTransaction t,
				       const void *data)
{
	GGZGame *game = ggzmod_ggz_get_gamedata(mod);
	GGZNet *net = _ggzcore_server_get_net(game->server);

	_ggzcore_net_send_table_reseat(net, GGZ_RESEAT_STAND, -1);
}

static void _ggzcore_game_handle_boot(GGZMod * mod, GGZModTransaction t,
				      const void *data)
{
	GGZGame *game = ggzmod_ggz_get_gamedata(mod);
	GGZNet *net = _ggzcore_server_get_net(game->server);
	GGZRoom *room = _ggzcore_server_get_nth_room(game->server,
						     game->room_id);
	GGZTable *table =
	    ggzcore_room_get_table_by_id(room, game->table_id);
	const char *name = data;
	int i;

	for (i = 0; i < ggzcore_table_get_num_seats(table); i++) {
		GGZTableSeat seat = _ggzcore_table_get_nth_seat(table, i);

		if (seat.type != GGZ_SEAT_PLAYER
		    || ggz_strcmp(seat.name, name))
			continue;
		_ggzcore_net_send_table_boot_update(net, table, &seat);
		return;
	}

	for (i = 0; i < ggzcore_table_get_num_spectator_seats(table); i++) {
		GGZTableSeat spectator
		    = _ggzcore_table_get_nth_spectator_seat(table, i);

		if (ggz_strcmp(spectator.name, name))
			continue;
		_ggzcore_net_send_table_boot_update(net, table,
						    &spectator);
		return;
	}

	/* If the player has already left, we won't find them and we'll
	   end up down here. */
}


static void _ggzcore_game_handle_seatchange(GGZMod * mod,
					    GGZModTransaction t,
					    const void *data)
{
	GGZGame *game = ggzmod_ggz_get_gamedata(mod);
	GGZNet *net = _ggzcore_server_get_net(game->server);
	const int *seat_num = data;
	GGZTableSeat seat = { .index = *seat_num, .name = NULL };
	GGZRoom *room = _ggzcore_server_get_nth_room(game->server,
						     game->room_id);
	GGZTable *table =
	    ggzcore_room_get_table_by_id(room, game->table_id);

	if (t == GGZMOD_TRANSACTION_OPEN)
		seat.type = GGZ_SEAT_OPEN;
	else
		seat.type = GGZ_SEAT_BOT;

	_ggzcore_net_send_table_seat_update(net, table, &seat);
}

static void _ggzcore_game_handle_chat(GGZMod * mod, GGZModTransaction t,
				      const void *data)
{
	GGZGame *game = ggzmod_ggz_get_gamedata(mod);
	const char *chat = data;
	GGZRoom *room = _ggzcore_server_get_cur_room(game->server);

	_ggzcore_room_chat(room, GGZ_CHAT_TABLE, NULL, chat);
}

static void _ggzcore_game_handle_info(GGZMod * mod, GGZModTransaction t,
				      const void *data)
{
	GGZGame *game = ggzmod_ggz_get_gamedata(mod);
	GGZNet *net = _ggzcore_server_get_net(game->server);
	const int *seat_num = data;

	_ggzcore_net_send_player_info(net, *seat_num);
}

void _ggzcore_game_free(struct _GGZGame *game)
{
	int i;

	ggz_debug(GGZCORE_DBG_GAME, "Destroying game object");

	ggzmod_ggz_disconnect(game->client);
	ggzmod_ggz_free(game->client);

	for (i = 0; i < _ggzcore_num_events; i++)
		_ggzcore_hook_list_destroy(game->event_hooks[i]);

	_ggzcore_server_set_cur_game(game->server, NULL);

	ggz_free(game);
}

void _ggzcore_game_set_table(GGZGame * game, int room_id, int table_id)
{
	GGZRoom *room;
	GGZTable *table;
	int num_seats, i;

	/* FIXME */
	assert(game->room_id == room_id);
	assert(game->table_id < 0 || game->table_id == table_id);

	room = ggzcore_server_get_cur_room(game->server);
	assert(_ggzcore_room_get_id(room) == room_id);

	game->table_id = table_id;
	table = ggzcore_room_get_table_by_id(room, table_id);
	assert(table && ggzcore_table_get_id(table) == table_id);

	num_seats = ggzcore_table_get_num_seats(table);
	for (i = 0; i < num_seats; i++) {
		GGZTableSeat seat = _ggzcore_table_get_nth_seat(table, i);

		_ggzcore_game_set_seat(game, &seat);
	}

	num_seats = ggzcore_table_get_num_spectator_seats(table);
	for (i = 0; i < num_seats; i++) {
		GGZTableSeat seat =
		    _ggzcore_table_get_nth_spectator_seat(table, i);

		_ggzcore_game_set_spectator_seat(game, &seat);
	}
}


void _ggzcore_game_set_seat(GGZGame * game, GGZTableSeat * seat)
{
	GGZSeat mseat = {
		.num = seat->index,
		.type = seat->type,
		.name = seat->name
	};

	ggzmod_ggz_set_seat(game->client, &mseat);
	_ggzcore_game_send_player_stats(game);
}

void _ggzcore_game_set_spectator_seat(GGZGame * game, GGZTableSeat * seat)
{
	GGZSpectatorSeat mseat = {
		.num = seat->index,
		.name = seat->name
	};
	ggzmod_ggz_set_spectator_seat(game->client, &mseat);
	_ggzcore_game_send_player_stats(game);
}

void _ggzcore_game_set_player(GGZGame * game, int is_spectator,
			      int seat_num)
{
	if (game->spectating == is_spectator && game->seat_num == seat_num)
		return;

	game->spectating = is_spectator;
	game->seat_num = seat_num;

	if (ggzmod_ggz_set_player(game->client,
			      _ggzcore_server_get_handle(game->server),
			      is_spectator, seat_num) < 0)
		assert(0);
}

void _ggzcore_game_set_info(GGZGame * game, int num, GGZList *infos)
{
	ggzmod_ggz_set_info(game->client, num, infos);
}

void _ggzcore_game_inform_chat(GGZGame * game,
			       const char *player, const char *msg)
{
	if (ggzmod_ggz_inform_chat(game->client, player, msg) < 0) {
		/* Nothing. */
	}
}

int _ggzcore_game_is_spectator(GGZGame * game)
{
	return game->spectating;
}


int _ggzcore_game_get_seat_num(GGZGame * game)
{
	return game->seat_num;
}


int _ggzcore_game_get_room_id(GGZGame * game)
{
	return game->room_id;
}


int _ggzcore_game_get_table_id(GGZGame * game)
{
	return game->table_id;
}


int _ggzcore_game_add_event_hook_full(struct _GGZGame *game,
				      const GGZGameEvent event,
				      const GGZHookFunc func,
				      const void *data)
{
	return _ggzcore_hook_add_full(game->event_hooks[event], func,
				      data);
}


/* Functions for removing hooks from struct _GGZGame events */
int _ggzcore_game_remove_event_hook(struct _GGZGame *game,
				    const GGZGameEvent event,
				    const GGZHookFunc func)
{
	return _ggzcore_hook_remove(game->event_hooks[event], func);
}


int _ggzcore_game_remove_event_hook_id(struct _GGZGame *game,
				       const GGZGameEvent event,
				       const unsigned int hook_id)
{
	return _ggzcore_hook_remove_id(game->event_hooks[event], hook_id);
}


int _ggzcore_game_get_control_fd(struct _GGZGame *game)
{
	return ggzmod_ggz_get_fd(game->client);
}

/*
 * Called when the game client fails.  Most likely the user has just
 * exited.  Make sure ggzd knows we've left the table.
 */
static void abort_game(struct _GGZGame *game)
{
      GGZTableLeaveEventData event_data = { reason:GGZ_LEAVE_NORMAL,
	      player:NULL
	};
	GGZServer *server = game->server;
	GGZRoom *room = _ggzcore_server_get_cur_room(server);

	/* This would be called automatically later (several times in fact),
	   but doing it now is safe enough and starts the ball rolling. */
	ggzmod_ggz_disconnect(game->client);

	if (room) {
		_ggzcore_room_table_event(room, GGZ_TABLE_LEFT,
					  &event_data);
	}

	if (room && ggzcore_server_get_state(server) == GGZ_STATE_AT_TABLE) {
		(void)ggzcore_room_leave_table(room, 1);
	}

	/* Make sure current game is free.  This way even if the leave
	 * failed above, we'll know to leave later. */
	game = _ggzcore_server_get_cur_game(server);
	if (game) {
		ggzcore_game_free(game);
	}
}

int _ggzcore_game_read_data(struct _GGZGame *game)
{
	int status;

	status = ggzmod_ggz_dispatch(game->client);
	ggz_debug(GGZCORE_DBG_GAME, "Result of reading from game: %d",
		  status);

	if (status < 0) {
		/* This doesn't have to be a launch error, but may be one */
		_ggzcore_game_event(game, GGZ_GAME_LAUNCH_FAIL, NULL);
		abort_game(game);
	}

	return status;
}


void _ggzcore_game_set_server_fd(struct _GGZGame *game, int fd)
{
	ggzmod_ggz_set_server_fd(game->client, fd);
}


GGZModule *_ggzcore_game_get_module(struct _GGZGame * game)
{
	return game->module;
}


int _ggzcore_game_launch(struct _GGZGame *game)
{
	int status;

	if (_ggzcore_module_is_embedded())
		ggz_debug(GGZCORE_DBG_GAME, "Launching embedded game");
	else
		ggz_debug(GGZCORE_DBG_GAME, "Launching game of %s",
			  _ggzcore_module_get_name(game->module));

	if ((status = ggzmod_ggz_connect(game->client)) == 0) {
		ggz_debug(GGZCORE_DBG_GAME, "Launched game module");
		_ggzcore_game_event(game, GGZ_GAME_LAUNCHED, NULL);
	} else {
		ggz_debug(GGZCORE_DBG_GAME,
			  "Failed to connect to game module");
		_ggzcore_game_event(game, GGZ_GAME_LAUNCH_FAIL, NULL);
	}

	return status;
}


/* Static functions internal to this file */

static int _ggzcore_game_event_is_valid(GGZGameEvent event)
{
	return (event >= 0 && event < _ggzcore_num_events);
}


static GGZHookReturn _ggzcore_game_event(struct _GGZGame *game,
					 GGZGameEvent id, void *data)
{
	return _ggzcore_hook_list_invoke(game->event_hooks[id], data);
}


#if 0	/* currently unused */
static char *_ggzcore_game_get_path(char **argv)
{
	char *mod_path;
	char *path;
	int len;

	mod_path = argv[0];

	if (mod_path[0] != '/') {
		ggz_debug(GGZCORE_DBG_GAME,
			  "Module has relative path, prepending gamedir");
		/* Calcualate string length, leaving room for a slash 
		   and the trailing null */
		len = strlen(GAMEDIR) + strlen(mod_path) + 2;
		path = ggz_malloc(len);
		strcpy(path, GAMEDIR);
		strcat(path, "/");
		strcat(path, mod_path);
	} else
		path = ggz_strdup(mod_path);

	return path;
}
#endif
