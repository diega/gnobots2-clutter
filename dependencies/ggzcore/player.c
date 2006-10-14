/*
 * File: player.c
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

#ifdef HAVE_CONFIG_H
#  include <config.h>	/* Site-specific config */
#endif

#include <stdlib.h>
#include <string.h>

#include <ggz.h>

#include "player.h"
#include "room.h"

/* 
 * The Player structure is meant to be a node in a list of
 * the players in the current room .
 */
struct _GGZPlayer {

	/* Name of player */
	char *name;

	/* Type of player */
	GGZPlayerType type;

	/* Pointer to room player is in */
	GGZRoom *room;

	/* Server ID of table player is at */
	int table;

	/* Lag of the player */
	int lag;

	/* Record of the player (or NO_RECORD). */
	int wins, losses, ties, forfeits;

	/* Rating of the player (or NO_RATING) */
	int rating;

	/* Ranking of the player (or NO_RANKING) */
	int ranking;

	/* Player's highest score (or NO_HIGHSCORE) */
	int highscore;
};


/* Publicly exported functions */

char *ggzcore_player_get_name(const GGZPlayer * player)
{
	if (!player)
		return NULL;
	return _ggzcore_player_get_name(player);
}


GGZPlayerType ggzcore_player_get_type(const GGZPlayer * player)
{
	if (!player)
		return -1;
	return _ggzcore_player_get_type(player);
}


GGZRoom *ggzcore_player_get_room(const GGZPlayer * player)
{
	if (!player)
		return NULL;
	return _ggzcore_player_get_room(player);
}

GGZTable *ggzcore_player_get_table(const GGZPlayer * player)
{
	if (!player)
		return NULL;
	return _ggzcore_player_get_table(player);
}


int ggzcore_player_get_lag(const GGZPlayer * player)
{
	if (!player)
		return 0;
	return _ggzcore_player_get_lag(player);
}



int ggzcore_player_get_record(const GGZPlayer * player,
			      int *wins, int *losses,
			      int *ties, int *forfeits)
{
	if (!player || !wins || !losses || !ties || !forfeits)
		return 0;
	return _ggzcore_player_get_record(player, wins, losses,
					  ties, forfeits);
}

int ggzcore_player_get_rating(const GGZPlayer * player, int *rating)
{
	if (!player || !rating)
		return 0;
	return _ggzcore_player_get_rating(player, rating);
}

int ggzcore_player_get_ranking(const GGZPlayer * player, int *ranking)
{
	if (!player || !ranking)
		return 0;
	return _ggzcore_player_get_ranking(player, ranking);
}

int ggzcore_player_get_highscore(const GGZPlayer * player, int *highscore)
{
	if (!player || !highscore)
		return 0;
	return _ggzcore_player_get_highscore(player, highscore);
}


/* 
 * Internal library functions (prototypes in player.h) 
 * NOTE:All of these functions assume valid inputs!
 */

GGZPlayer *_ggzcore_player_new(void)
{
	GGZPlayer *player;

	player = ggz_malloc(sizeof(GGZPlayer));

	/* Set to invalid table */
	player->table = -1;

	/* Assume no lag */
	player->lag = -1;

	player->wins = NO_RECORD;
	player->losses = NO_RECORD;
	player->ties = NO_RECORD;
	player->forfeits = NO_RECORD;
	player->rating = NO_RATING;
	player->ranking = NO_RANKING;
	player->highscore = NO_HIGHSCORE;

	return player;
}


void _ggzcore_player_init(GGZPlayer * player,
			  const char *name,
			  GGZRoom * room,
			  const int table,
			  const GGZPlayerType type, const int lag)
{
	player->name = ggz_strdup(name);
	player->room = room;
	player->table = table;
	player->type = type;
	player->lag = lag;
}


void _ggzcore_player_init_stats(GGZPlayer * player,
				int wins, int losses, int ties,
				int forfeits, int rating, int ranking,
				int highscore)
{
	player->wins = wins;
	player->losses = losses;
	player->ties = ties;
	player->forfeits = forfeits;
	player->rating = rating;
	player->ranking = ranking;
	player->highscore = highscore;
}


void _ggzcore_player_free(GGZPlayer * player)
{
	if (player->name)
		ggz_free(player->name);

	ggz_free(player);
}


void _ggzcore_player_set_table(GGZPlayer * player, const int table)
{
	player->table = table;
}


void _ggzcore_player_set_lag(GGZPlayer * player, const int lag)
{
	player->lag = lag;
}


char *_ggzcore_player_get_name(const GGZPlayer * player)
{
	return player->name;
}


GGZPlayerType _ggzcore_player_get_type(const GGZPlayer * player)
{
	return player->type;
}


GGZRoom *_ggzcore_player_get_room(const GGZPlayer * player)
{
	return player->room;
}


GGZTable *_ggzcore_player_get_table(const GGZPlayer * player)
{
	if (player->table == -1)
		return NULL;

	return ggzcore_room_get_table_by_id(player->room, player->table);
}


int _ggzcore_player_get_lag(const GGZPlayer * player)
{
	return player->lag;
}


int _ggzcore_player_get_record(const GGZPlayer * player,
			       int *wins, int *losses,
			       int *ties, int *forfeits)
{
	if (player->wins == NO_RECORD
	    && player->losses == NO_RECORD
	    && player->ties == NO_RECORD
	    && player->forfeits == NO_RECORD) {
		*wins = NO_RECORD;
		*losses = NO_RECORD;
		*ties = NO_RECORD;
		*forfeits = NO_RECORD;
		return 0;
	}
#ifndef MAX
#  define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

	/* NO_RECORD is -1.  If we have a stat for anything, we should
	   return all stats and assume 0 for any we don't know. */

	*wins = MAX(player->wins, 0);
	*losses = MAX(player->losses, 0);
	*ties = MAX(player->ties, 0);
	*forfeits = MAX(player->forfeits, 0);

	return 1;
}


int _ggzcore_player_get_rating(const GGZPlayer * player, int *rating)
{
	if (player->rating == NO_RATING) {
		*rating = NO_RATING;
		return 0;
	}
	*rating = player->rating;
	return 1;
}


int _ggzcore_player_get_ranking(const GGZPlayer * player, int *ranking)
{
	if (player->ranking == NO_RANKING) {
		*ranking = NO_RANKING;
		return 0;
	}
	*ranking = player->ranking;
	return 1;
}


int _ggzcore_player_get_highscore(const GGZPlayer * player, int *highscore)
{
	if (player->highscore == NO_HIGHSCORE) {
		*highscore = NO_HIGHSCORE;
		return 0;
	}
	*highscore = player->highscore;
	return 1;
}


int _ggzcore_player_compare(const void *p, const void *q)
{
	const GGZPlayer *s_p = p, *s_q = q;

	if (s_p->name && s_q->name) {
		return strcmp(s_p->name, s_q->name);
	} else {
		/* This shouldn't happen, but it has been known to. */
		return (s_p->name ? 1 : 0) - (s_q->name ? 1 : 0);
	}
}


void *_ggzcore_player_create(void *p)
{
	GGZPlayer *new, *src = p;

	new = _ggzcore_player_new();
	_ggzcore_player_init(new, src->name, src->room, src->table,
			     src->type, src->lag);

	return (void *)new;
}


void _ggzcore_player_destroy(void *p)
{
	_ggzcore_player_free(p);
}


/* Room-player functions. */

void _ggzcore_room_set_player_lag(GGZRoom * room, const char *name,
				  int lag)
{
	/* FIXME: This should be sending a player "class-based" event */
	GGZPlayer *player;

	ggz_debug(GGZCORE_DBG_ROOM, "Setting lag to %d for %s", lag, name);

	player = _ggzcore_room_get_player_by_name(room, name);
	if (player) {	/* make sure they're still in room */
		_ggzcore_player_set_lag(player, lag);
		_ggzcore_room_event(room, GGZ_PLAYER_LAG, name);
	}
}


void _ggzcore_room_set_player_stats(GGZRoom * room, GGZPlayer * pdata)
{
	/* FIXME: This should be sending a player "class-based" event */
	GGZPlayer *player;

	ggz_debug(GGZCORE_DBG_ROOM, "Setting stats for %s: %d-%d-%d",
		  ggzcore_player_get_name(pdata), pdata->wins,
		  pdata->losses, pdata->ties);

	player = _ggzcore_room_get_player_by_name(room, pdata->name);

	/* make sure they're still in room */
	if (!player)
		return;

	_ggzcore_player_init_stats(player,
				   pdata->wins,
				   pdata->losses,
				   pdata->ties,
				   pdata->forfeits,
				   pdata->rating,
				   pdata->ranking, pdata->highscore);
	_ggzcore_room_event(room, GGZ_PLAYER_STATS, player->name);
}
