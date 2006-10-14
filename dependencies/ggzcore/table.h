/*
 * File: player.h
 * Author: Brent Hendricks
 * Project: GGZ Core Client Lib
 * Date: 6/5/00
 *
 * This fils contains functions for handling players
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


#ifndef __TABLE_H_
#define __TABLE_H_

#include <ggz_common.h>

#include "gametype.h"
#include "ggzcore.h"


typedef struct GGZTableSeat GGZTableSeat;

struct GGZTableSeat {
	/* Seat index */
	int index;

	/* Type of player in seat.  Used for regular seats only;
	   spectator seats just ignore it.*/
	GGZSeatType type;

	/* Player's name; or NULL if none.  An empty spectator seat
	   will have no name. */
	const char *name;
};


GGZTable* _ggzcore_table_new(void);

void _ggzcore_table_init(GGZTable *table, 
			 const GGZGameType *gametype,
			 const char *desc,
			 const unsigned int num_seats,
			 const GGZTableState state,
			 const int id);

void _ggzcore_table_free(GGZTable *table);

void _ggzcore_table_set_room(GGZTable *table, GGZRoom *room);
void _ggzcore_table_set_id(GGZTable *table, const int id);
void _ggzcore_table_set_state(GGZTable *table, const GGZTableState state);
void _ggzcore_table_set_desc(GGZTable *table, const char *desc);

/** @brief Change a seat value.
 *
 *  This changes the seat status for any seat at the table.  It is
 *  called by both front-end and back-end functions to do the
 *  actual work of changing the seat.
 */
void _ggzcore_table_set_seat(GGZTable *table, GGZTableSeat *seat);


/** @brief Change a spectator seat value.
 *
 *  This changes the seat status for any spectator seat at the table.
 */
void _ggzcore_table_set_spectator_seat(GGZTable *table,
				       GGZTableSeat *seat);

/* These functions return pointers to the seat data directly within the
 * table. */
GGZTableSeat _ggzcore_table_get_nth_seat(const GGZTable *table,
					 unsigned int num);
GGZTableSeat _ggzcore_table_get_nth_spectator_seat(const GGZTable *table,
						   unsigned int num);
/* Utility functions used by _ggzcore_list */
int _ggzcore_table_compare(const void* p, const void* q);
void* _ggzcore_table_create(void* p);
void  _ggzcore_table_destroy(void* p);

#endif /* __TABLE_H_ */

