/*
 * File: player.h
 * Author: Brent Hendricks
 * Project: GGZ Core Client Lib
 * Date: 6/5/00
 * $Id$
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


#ifndef __PLAYER_H_
#define __PLAYER_H_

#include "room.h"



GGZPlayer *_ggzcore_player_new(void);

void _ggzcore_player_free(GGZPlayer * player);

void _ggzcore_player_init(GGZPlayer * player,
			  const char *name,
			  GGZRoom * room,
			  const int table,
			  const GGZPlayerType type, const int lag);
void _ggzcore_player_init_stats(GGZPlayer * player,
				int wins, int losses, int ties,
				int forfeits, int rating, int ranking,
				int highscore);


void _ggzcore_player_set_table(GGZPlayer * player, const int table);
void _ggzcore_player_set_lag(GGZPlayer * player, const int lag);

char *_ggzcore_player_get_name(const GGZPlayer * player);
GGZPlayerType _ggzcore_player_get_type(const GGZPlayer * player);
struct _GGZTable *_ggzcore_player_get_table(const GGZPlayer * player);
int _ggzcore_player_get_lag(const GGZPlayer * player);
GGZRoom *_ggzcore_player_get_room(const GGZPlayer * player);

#define NO_RECORD -1
#define NO_RATING 0
#define NO_RANKING 0
#define NO_HIGHSCORE -1
int _ggzcore_player_get_record(const GGZPlayer * player,
			       int *wins, int *losses,
			       int *ties, int *forfeits);
int _ggzcore_player_get_rating(const GGZPlayer * player, int *rating);
int _ggzcore_player_get_ranking(const GGZPlayer * player, int *ranking);
int _ggzcore_player_get_highscore(const GGZPlayer * player,
				  int *highscore);

/* Utility functions used by _ggzcore_list */
int _ggzcore_player_compare(const void *p, const void *q);
void *_ggzcore_player_create(void *p);
void _ggzcore_player_destroy(void *p);

/* Room-player functions. */
void _ggzcore_room_set_player_lag(GGZRoom * room, const char *name,
				  int lag);
void _ggzcore_room_set_player_stats(GGZRoom * room, GGZPlayer * pdata);


#endif /* __PLAYER_H_ */
