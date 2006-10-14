/*
 * File: table.c
 * Author: Justin Zaun
 * Project: GGZ Core Client Lib
 * Date: 6/5/00
 * $Id$
 *
 * This fils contains functions for handling tables
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
#  include <config.h>		/* Site-specific config */
#endif

#include <stdlib.h>
#include <string.h>

#include <ggz.h>
#include <ggz_common.h>

#include "game.h"
#include "ggzcore.h"
#include "net.h"
#include "room.h"
#include "table.h"

/* 
 * The Table structure is meant to be a node in a linked list of
 * the tables in the current room .
 */


/* Table Information */
struct _GGZTable {
 
	/* Pointer to room this table resides in */
	GGZRoom *room;

        /* Server ID of table */
        int id;
 
        /* Game Type */
	const GGZGameType *gametype;

	/* Table description */
	const char * desc;        

        /* Table state */
        GGZTableState state;

        /* Total seats */
        unsigned int num_seats;

	/* Seats */
	GGZTableSeat *seats;

	/* Total spectator seats */
	unsigned int num_spectator_seats;

	/* Spectator seats - "type" is unused; player name is
	   NULL for empty seat. */
	GGZTableSeat *spectator_seats;
};


/*
 * Private functions - either static or declared in table.h
 */

static const GGZRoom *_ggzcore_table_get_room(const GGZTable *table)
{
	return table->room;
}


static int _ggzcore_table_get_id(const GGZTable *table)
{
	return table->id;
}


static const GGZGameType* _ggzcore_table_get_type(const GGZTable *table)
{
	return table->gametype;
}


static const char * _ggzcore_table_get_desc(const GGZTable *table)
{
	return table->desc;
}


static GGZTableState _ggzcore_table_get_state(const GGZTable *table)
{
	return table->state;
}


static int _ggzcore_table_get_num_seats(const GGZTable *table)
{
	return table->num_seats;
}


static int _ggzcore_table_get_seat_count(const GGZTable *table,
					 GGZSeatType type)
{
	int i, count = 0;

	for (i = 0; i < table->num_seats; i++)
		if (table->seats[i].type == type)
			count++;
	
	return count;
}


GGZTableSeat _ggzcore_table_get_nth_seat(const GGZTable *table,
					  unsigned int num)
{
	return table->seats[num];
}


static const char *_ggzcore_table_get_nth_player_name(const GGZTable *table,
						      unsigned int num)
{
	return table->seats[num].name;
}


static GGZSeatType _ggzcore_table_get_nth_player_type(const GGZTable *table,
						      unsigned int num)
{
	return table->seats[num].type;
}


static int _ggzcore_table_get_num_spectator_seats(const GGZTable *table)
{
	return table->num_spectator_seats;
}


GGZTableSeat _ggzcore_table_get_nth_spectator_seat(const GGZTable *table,
						    unsigned int num)
{
	return table->spectator_seats[num];
}


static const char *_ggzcore_table_get_nth_spectator_name(const GGZTable *table,
							 unsigned int num)
{
	return table->spectator_seats[num].name;
}


/*
 * Publicly exported functions
 */

GGZTable* ggzcore_table_new(void)
{
	return _ggzcore_table_new();
}


int ggzcore_table_init(GGZTable *table,
		       const GGZGameType *gametype,
		       const char *desc,
		       const unsigned int num_seats)
{
	/* A NULL desc is allowed. */
	if (table && gametype) {
		_ggzcore_table_init(table, gametype, desc,
				    num_seats,
				    GGZ_TABLE_CREATED, -1);
		return 0;
	} else
		return -1;
}


void ggzcore_table_free(GGZTable *table)
{
	if (table)
		_ggzcore_table_free(table);
}


int ggzcore_table_set_seat(GGZTable *table,
			   const unsigned int index,
			   GGZSeatType type,
			   const char *name)
{
	GGZServer *server;
	GGZNet *net;
	GGZTableSeat seat = {index, type, name};

	ggz_debug(GGZCORE_DBG_TABLE, "User changing seats... on %p", table);

	/* Check table and seat number. */
	if (!table || index >= table->num_seats)
		return -1;
		
	/* GGZ clients should only ever set seats to OPEN, BOT, or RESERVED. */
	if (type != GGZ_SEAT_OPEN
	    && type != GGZ_SEAT_BOT
	    && type != GGZ_SEAT_RESERVED)
		return -1;
		
	/* If we set a seat to RESERVED, we need a reservation name. */
	if (type == GGZ_SEAT_RESERVED && !name)
		return -1;

	/* If the table is newly created and not involved in a game
           yet, users may change seats all they want */
	if (table->state == GGZ_TABLE_CREATED)
		_ggzcore_table_set_seat(table, &seat);
	else {
		/* Otherwise we have to do it through the server */
		if (!table->room)
			return -1;

		if (!(server = ggzcore_room_get_server(table->room)))
			return -1;

		if (!(net = _ggzcore_server_get_net(server)))
			return -1;
		
		return _ggzcore_net_send_table_seat_update(net, table, &seat);
	}


	return 0;
}


int ggzcore_table_set_desc(GGZTable *table, const char *desc)
{
	GGZServer *server;
	GGZNet *net;

	ggz_debug(GGZCORE_DBG_TABLE, "User changing desc... on %p", table);

	/* Check table and seat number. */
	if (!table)
		return -1;
		
	/* If the table is newly created and not involved in a game
           yet, users may change desc all they want */
	if (table->state == GGZ_TABLE_CREATED)
		_ggzcore_table_set_desc(table, desc);
	else {
		/* Otherwise we have to do it through the server */
		if (!table->room)
			return -1;

		if (!(server = ggzcore_room_get_server(table->room)))
			return -1;

		if (!(net = _ggzcore_server_get_net(server)))
			return -1;
		
		return _ggzcore_net_send_table_desc_update(net, table, desc);
	}


	return 0;
}


int ggzcore_table_remove_player(GGZTable *table, const char *name)
{
	int i, status = -1;

	if (table && name) {
		for (i = 0; i < table->num_seats; i++)
			if (table->seats[i].name != NULL 
			    && strcmp(table->seats[i].name, name) == 0) {
				GGZTableSeat seat = {i, GGZ_SEAT_OPEN, NULL};

				_ggzcore_table_set_seat(table, &seat);
				status = 0;
			}
	}
	
	return status;
}


const GGZGameType* ggzcore_table_get_type(const GGZTable *table)
{
	if (table)
		return _ggzcore_table_get_type(table);
	else
		return NULL;
}


int ggzcore_table_get_id(const GGZTable *table)
{
	if (table)
		return _ggzcore_table_get_id(table);
	else
		return -1;
}


const GGZRoom *ggzcore_table_get_room(const GGZTable *table)
{
	if (table)
		return _ggzcore_table_get_room(table);
	else
		return NULL;
}

     
const char * ggzcore_table_get_desc(const GGZTable *table)
{
	if (table)
		return _ggzcore_table_get_desc(table);
	else
		return NULL;
}


GGZTableState ggzcore_table_get_state(const GGZTable *table)
{
	if (table)
		return _ggzcore_table_get_state(table);
	else
		return -1;
}


int ggzcore_table_get_num_seats(const GGZTable *table)
{
	if (table)
		return _ggzcore_table_get_num_seats(table);
	else
		return -1;
}


int ggzcore_table_get_seat_count(const GGZTable *table, GGZSeatType type)
{
	if (table)
		return _ggzcore_table_get_seat_count(table, type);
	else 
		return -1;
}


const char * ggzcore_table_get_nth_player_name(const GGZTable *table,
					       const unsigned int num)
{
	if (table && num < table->num_seats)
		return _ggzcore_table_get_nth_player_name(table, num);
	else
		return NULL;
}


GGZSeatType ggzcore_table_get_nth_player_type(const GGZTable *table,
					      const unsigned int num)
{
	if (table && num < table->num_seats)
		return _ggzcore_table_get_nth_player_type(table, num);
	else
		return 0;
}


int ggzcore_table_get_num_spectator_seats(const GGZTable *table)
{
	if (!table)
		return -1;

	return _ggzcore_table_get_num_spectator_seats(table);
}


const char *ggzcore_table_get_nth_spectator_name(const GGZTable *table,
						 const unsigned int num)
{
	if (!table || num >= table->num_spectator_seats)
		return NULL;

	return _ggzcore_table_get_nth_spectator_name(table, num);
}
					           

/* 
 * Internal library functions (prototypes in table.h) 
 * NOTE:All of these functions assume valid inputs!
 */

GGZTable* _ggzcore_table_new(void)
{
	GGZTable *table;

	table = ggz_malloc(sizeof(*table));

	/* FIXME: anything we should mark invalid? */
	return table;
}


void _ggzcore_table_init(GGZTable *table, 
			 const GGZGameType *gametype,
			 const char *desc,
			 const unsigned int num_seats,
			 const GGZTableState state,
			 const int id)
{
	int i;
	
	table->room = NULL;
	table->gametype = gametype;
	table->id = id;
	table->state = state;
	table->desc = ggz_strdup(desc);

	table->num_seats = num_seats;
	ggz_debug(GGZCORE_DBG_TABLE, "Allocating %d seats", num_seats);
	if (num_seats)
		table->seats = ggz_malloc(num_seats * sizeof(GGZTableSeat));
	for (i = 0; i < num_seats; i++) {
		/* FIXME: We should probably initialize seats to
		 * GGZ_SEAT_NONE.  Some code in netxml has to reset it
		 * manually. */
		table->seats[i].index = i;
		table->seats[i].type = GGZ_SEAT_OPEN;
		table->seats[i].name = NULL;
	}

	/* Allocated on demand later */
	table->num_spectator_seats = 0;
	table->spectator_seats = NULL;
}


void _ggzcore_table_free(GGZTable *table)
{
	int i;

	if (table->desc)
		ggz_free(table->desc);

	if (table->seats) {
		for (i = 0; i < table->num_seats; i++)
			if (table->seats[i].name)
				ggz_free(table->seats[i].name);
		ggz_free(table->seats);
	}

	if (table->spectator_seats) {
		for (i = 0; i < table->num_spectator_seats; i++)
			if (table->spectator_seats[i].name)
				ggz_free(table->spectator_seats[i].name);
		ggz_free(table->spectator_seats);
	}
	
	ggz_free(table);

}


void _ggzcore_table_set_room(GGZTable *table, GGZRoom *room)
{
	table->room = room;
}


void _ggzcore_table_set_id(GGZTable *table, const int id)
{
	table->id = id;
}


void _ggzcore_table_set_state(GGZTable *table, const GGZTableState state)
{
	ggz_debug(GGZCORE_DBG_TABLE, "Table %d new state %d",
		  table->id, state);
	table->state = state;
	
	/* If we're in a room, notify it of a table event */
	if (table->room)
		_ggzcore_room_table_event(table->room, GGZ_TABLE_UPDATE, NULL);
}


void _ggzcore_table_set_desc(GGZTable *table, const char *desc)
{
	ggz_debug(GGZCORE_DBG_TABLE, "Table %d new desc %s", table->id, desc);

	/* Free previous description */
	if (table->desc)
		ggz_free(table->desc);
	table->desc = ggz_strdup(desc);

	/* If we're in a room, notify it of a table event */
	if (table->room) 
		_ggzcore_room_table_event(table->room, GGZ_TABLE_UPDATE, NULL);
}


void _ggzcore_table_set_seat(GGZTable *table, GGZTableSeat *seat)
{
	/* Set up the new seat. */
	GGZTableSeat oldseat;
	GGZServer *server;
	GGZGame *game;

	/* Sanity check */
	if (seat->index >= table->num_seats) {
		ggz_debug(GGZCORE_DBG_TABLE,
			  "Attempt to set seat %d on table with only %d seats",
			  seat->index, table->num_seats);
	}

	oldseat = table->seats[seat->index];
	table->seats[seat->index].index = seat->index;
	table->seats[seat->index].type = seat->type;
	table->seats[seat->index].name = ggz_strdup(seat->name);
	
	/* Check for specific seat changes */
	if (seat->type == GGZ_SEAT_PLAYER) {
		ggz_debug(GGZCORE_DBG_TABLE, "%s joining seat %d at table %d",
			      seat->name, seat->index, table->id);
		if (table->room)
			_ggzcore_room_player_set_table(table->room,
						       seat->name, table->id);
	} else if (oldseat.type == GGZ_SEAT_PLAYER) {
		ggz_debug(GGZCORE_DBG_TABLE, "%s leaving seat %d at table %d",
			  oldseat.name, oldseat.index, table->id);
		if (table->room)
			_ggzcore_room_player_set_table(table->room,
						       oldseat.name, -1);
	} else {
		if (table->room) 
			_ggzcore_room_table_event(table->room,
						  GGZ_TABLE_UPDATE, NULL);
	}
	
	/* Get rid of the old seat. */
	if (oldseat.name)
		ggz_free(oldseat.name);

	/* If this is our table, alert the game module. */
	if (table->room
	    && (server = ggzcore_room_get_server(table->room))
	    && (game = ggzcore_server_get_cur_game(server))
	    && _ggzcore_room_get_id(table->room)
		== _ggzcore_game_get_room_id(game)) {
		const char *me = _ggzcore_server_get_handle(server);
		int game_table = _ggzcore_game_get_table_id(game);

		if (table->id == game_table)
			_ggzcore_game_set_seat(game, seat);
		if (seat->type == GGZ_SEAT_PLAYER
		    && !ggz_strcmp(seat->name, me)) {
			_ggzcore_game_set_player(game, 0, seat->index);
			if (game_table < 0) {
				_ggzcore_game_set_table(game,
					_ggzcore_game_get_room_id(game),
					table->id);
			}
		}
	}
}


void _ggzcore_table_set_spectator_seat(GGZTable *table,
				       GGZTableSeat *seat)
{
	GGZTableSeat oldseat;
	GGZServer *server;
	GGZGame *game;

	if (seat->index >= table->num_spectator_seats) {
		int new = table->num_spectator_seats, i;

		/* Grow the array geometrically to keep a constant ammortized
		   overhead. */
		while (seat->index >= new)
			new = new ? new * 2 : 1;

		ggz_debug(GGZCORE_DBG_TABLE,
			  "Increasing number of spectator seats to %d.", new);

		table->spectator_seats =
			ggz_realloc(table->spectator_seats,
				    new * sizeof(*table->spectator_seats));
		for (i = table->num_spectator_seats + 1; i < new; i++) {
			table->spectator_seats[i].index = i;
			table->spectator_seats[i].name = NULL;
		}
		table->num_spectator_seats = new;
	}

	oldseat = table->spectator_seats[seat->index];
	table->spectator_seats[seat->index].index = seat->index;
	table->spectator_seats[seat->index].name = ggz_strdup(seat->name);

	/* Check for specific seat changes */
	if (seat->name) {
		ggz_debug(GGZCORE_DBG_TABLE,
			  "%s spectating seat %d at table %d",
			  seat->name, seat->index, table->id);
		if (table->room)
			_ggzcore_room_player_set_table(table->room, seat->name,
						      table->id);
	}

	if (oldseat.name) {
		ggz_debug(GGZCORE_DBG_TABLE,
			  "%s stopped spectating seat %d at table %d",
			  oldseat.name, oldseat.index, table->id);
		if (table->room)
			_ggzcore_room_player_set_table(table->room,
						       oldseat.name, -1);
	}

	/* Get rid of the old seat. */
	if (oldseat.name)
		ggz_free(oldseat.name);

	/* If this is our table, alert the game module. */
	if (table->room
	    && (server = ggzcore_room_get_server(table->room))
	    && (game = _ggzcore_server_get_cur_game(server))
	    && (_ggzcore_room_get_id(table->room)
		== _ggzcore_game_get_room_id(game))) {
		const char *me = _ggzcore_server_get_handle(server);
		int game_table = _ggzcore_game_get_table_id(game);

		if (table->id == game_table)
			_ggzcore_game_set_spectator_seat(game, seat);
		if (!ggz_strcmp(seat->name, me)) {
			_ggzcore_game_set_player(game, 1, seat->index);
			if (game_table < 0) {
				_ggzcore_game_set_table(game,
					_ggzcore_game_get_room_id(game),
					table->id);
			}
		}
	}
}


int _ggzcore_table_compare(const void* p, const void* q)
{
	const GGZTable *table1 = p, *table2 = q;

	return table1->id - table2->id;
}


void* _ggzcore_table_create(void* p)
{
	GGZTable *new, *src = p;

	new = _ggzcore_table_new();
	_ggzcore_table_init(new, src->gametype, src->desc,
			    src->num_seats,
			    src->state, src->id);

	/* FIXME: copy players as well */
	
	return new;
}


void  _ggzcore_table_destroy(void* p)
{
	_ggzcore_table_free(p);
}
