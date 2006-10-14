/*
 * File: gametype.c
 * Author: GGZ Development Team
 * Project: GGZ Core Client Lib
 * Date: 6/5/00
 * $Id$
 *
 * This file contains functions for hadiling game types.
 *
 * Copyright (C) 1998-2005 GGZ Development Team.
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

#include "gametype.h"
#include "ggzcore.h"

/* Structure describing specific game type */
struct _GGZGameType {
	
	/* Name of the game */
	char *name;

	/* Version */
	char *version;

	/* Protocol engine */
	char *prot_engine;

	/* Protocol version */
	char *prot_version;

	/* Game description */
	char *desc;

	/* Author */
	char *author;

	/* Homepage for game */
	char *url;

	/* Bitmask of alowable numbers of players */
	GGZNumberList player_allow_list;

	/* Bitmask of alowable numbers of bots */
	GGZNumberList bot_allow_list;

	/* Whether spectators are allowed or not */
	unsigned int spectators_allowed;
	
	/* ID of this game on the server */
	unsigned int id;

	/* Array of named bots, in name-class pairs */
	char ***named_bots;

	/* Whether peer hostname disclosure is allowed or not */
	unsigned int peers_allowed;
};


/*
 * Internal library functions
 * (either static or with prototypes in gametype.h)
 */

GGZGameType* _ggzcore_gametype_new(void)
{
	GGZGameType *gametype;

	gametype = ggz_malloc(sizeof(*gametype));
	
	/* FIXME: any fields we should fill in defaults? */
	return gametype;
}


void _ggzcore_gametype_init(GGZGameType *gametype,
			    const unsigned int id,
			    const char* name, 
			    const char* version,
			    const char* prot_engine,
			    const char* prot_version,
			    const GGZNumberList player_allow_list,
			    const GGZNumberList bot_allow_list,
			    const unsigned int spectators_allowed,
			    const unsigned int peers_allowed,
			    const char* desc,
			    const char* author,
			    const char *url)
{
	gametype->id = id;
	gametype->player_allow_list = player_allow_list;
	gametype->bot_allow_list = bot_allow_list;
	gametype->spectators_allowed = spectators_allowed;
	gametype->peers_allowed = peers_allowed;
	
	gametype->name = ggz_strdup(name);
	gametype->version = ggz_strdup(version);
	gametype->prot_engine = ggz_strdup(prot_engine);
	gametype->prot_version = ggz_strdup(prot_version);
	gametype->desc = ggz_strdup(desc);
	gametype->author = ggz_strdup(author);
	gametype->url = ggz_strdup(url);

	gametype->named_bots = NULL;
}


void _ggzcore_gametype_add_namedbot(GGZGameType *gametype,
			    const char *botname,
			    const char *botclass)
{
	int size = ggzcore_gametype_get_num_namedbots(gametype);
	gametype->named_bots = (char***)ggz_realloc(gametype->named_bots,
		(size + 2) * sizeof(char**));
	gametype->named_bots[size] = (char**)ggz_malloc(2 * sizeof(char*));
	gametype->named_bots[size][0] = ggz_strdup(botname);
	gametype->named_bots[size][1] = ggz_strdup(botclass);
	gametype->named_bots[size + 1] = NULL;
}


void _ggzcore_gametype_free(GGZGameType *type)
{
	int size, i;

	if (type->name)
		ggz_free(type->name);
	if (type->prot_engine)
		ggz_free(type->prot_engine);
	if (type->prot_version)
		ggz_free(type->prot_version);
	if (type->version)
		ggz_free(type->version);
	if (type->desc)
		ggz_free(type->desc);
	if (type->author)
		ggz_free(type->author);
	if (type->url)
		ggz_free(type->url);

	if (type->named_bots) {
		size = ggzcore_gametype_get_num_namedbots(type);
		for (i = 0; i < size; i++)
		{
			ggz_free(type->named_bots[i][0]);
			ggz_free(type->named_bots[i][1]);
			ggz_free(type->named_bots[i]);
		}
		ggz_free(type->named_bots);
	}

	ggz_free(type);
}


static unsigned int _ggzcore_gametype_get_id(const GGZGameType *type)
{
	return type->id;
}


static const char *_ggzcore_gametype_get_name(const GGZGameType *type)
{
	return type->name;
}


static const char * _ggzcore_gametype_get_prot_engine(const GGZGameType *type)
{
	return type->prot_engine;
}


static const char * _ggzcore_gametype_get_prot_version(const GGZGameType *type)
{
	return type->prot_version;
}


static const char * _ggzcore_gametype_get_version(const GGZGameType *type)
{
	return type->version;
}


static const char * _ggzcore_gametype_get_author(const GGZGameType *type)
{
	return type->author;
}


static const char * _ggzcore_gametype_get_url(const GGZGameType *type)
{
	return type->url;
}


static const char * _ggzcore_gametype_get_desc(const GGZGameType *type)
{
	return type->desc;
}


/* Return the maximum number of allowed players/bots */
static unsigned int _ggzcore_gametype_get_max_players(const GGZGameType *type)
{
	return ggz_numberlist_get_max(&type->player_allow_list);
}


static unsigned int _ggzcore_gametype_get_max_bots(const GGZGameType *type)
{
	return ggz_numberlist_get_max(&type->bot_allow_list);
}


static unsigned int _ggzcore_gametype_get_spectators_allowed(const GGZGameType
							     *type)
{
	return type->spectators_allowed;
}


static unsigned int _ggzcore_gametype_get_peers_allowed(const GGZGameType *type)
{
	return type->peers_allowed;
}

/* Verify that a paticular number of players/bots is valid */
static int _ggzcore_gametype_num_players_is_valid(const GGZGameType *type,
						  unsigned int num)
{
	return ggz_numberlist_isset(&type->player_allow_list, num);
}


static int _ggzcore_gametype_num_bots_is_valid(const GGZGameType *type,
					       unsigned int num)
{
	return num == 0
	  || ggz_numberlist_isset(&type->bot_allow_list, num);
}


/* Return 0 if equal, -1 greaterthan, 1 lessthan */
int _ggzcore_gametype_compare(void* p, void* q)
{
	const GGZGameType *t1 = p, *t2 = q;

	return t1->id - t2->id;
}


void* _ggzcore_gametype_create(void* p)
{
	GGZGameType *new, *src = p;

	new = _ggzcore_gametype_new();

	_ggzcore_gametype_init(new, src->id, src->name, src->version,
			       src->prot_engine, src->prot_version,
			       src->player_allow_list, src->bot_allow_list,
			       src->spectators_allowed, src->peers_allowed,
			       src->desc, src->author, src->url);
	
	return new;
}


void  _ggzcore_gametype_destroy(void* p)
{
	_ggzcore_gametype_free(p);
}


static int _ggzcore_gametype_get_num_namedbots(const GGZGameType *type)
{
	int i = 0;
	if (!type->named_bots) return 0;
	while (type->named_bots[i]) i++;
	return i;
}

static const char* _ggzcore_gametype_get_namedbot_name(const GGZGameType *type, unsigned int num)
{
	return type->named_bots[num][0];
}

static const char* _ggzcore_gametype_get_namedbot_class(const GGZGameType *type, unsigned int num)
{
	return type->named_bots[num][1];
}



/*
 * Publicly exported functions
 */

unsigned int ggzcore_gametype_get_id(const GGZGameType *type)
{
	if (!type)
		return (unsigned int) -1;

	return _ggzcore_gametype_get_id(type);
}

const char * ggzcore_gametype_get_name(const GGZGameType *type)
{
	if (!type)
		return NULL;

	return _ggzcore_gametype_get_name(type);
}


const char * ggzcore_gametype_get_author(const GGZGameType *type)
{
	if (!type)
		return NULL;

	return _ggzcore_gametype_get_author(type);
}

const char * ggzcore_gametype_get_prot_engine(const GGZGameType *type)
{
	if (!type)
		return NULL;

	return _ggzcore_gametype_get_prot_engine(type);
}


const char * ggzcore_gametype_get_prot_version(const GGZGameType *type)
{
	if (!type)
		return NULL;

	return _ggzcore_gametype_get_prot_version(type);
}


const char * ggzcore_gametype_get_version(const GGZGameType *type)
{
	if (!type)
		return NULL;

	return _ggzcore_gametype_get_version(type);
}


const char * ggzcore_gametype_get_url(const GGZGameType *type)
{
	if (!type)
		return NULL;

	return _ggzcore_gametype_get_url(type);
}


const char * ggzcore_gametype_get_desc(const GGZGameType *type)
{
	if (!type)
		return NULL;

	return _ggzcore_gametype_get_desc(type);
}

/* Return the maximum number of allowed players/bots */
int ggzcore_gametype_get_max_players(const GGZGameType *type)
{
	if (!type)
		return -1;

	return _ggzcore_gametype_get_max_players(type);
}


int ggzcore_gametype_get_max_bots(const GGZGameType *type)
{
	if (!type)
		return -1;

	return _ggzcore_gametype_get_max_bots(type);
}


int ggzcore_gametype_get_spectators_allowed(const GGZGameType *type)
{
	if (!type)
		return 0;

	return _ggzcore_gametype_get_spectators_allowed(type);
}


int ggzcore_gametype_get_peers_allowed(const GGZGameType *type)
{
	if (!type)
		return 0;

	return _ggzcore_gametype_get_peers_allowed(type);
}


/* Verify that a paticular number of players/bots is valid */
int ggzcore_gametype_num_players_is_valid(const GGZGameType *type,
					  unsigned int num)
{
	if (!type)
		return 0;

	return _ggzcore_gametype_num_players_is_valid(type, num);
}


int ggzcore_gametype_num_bots_is_valid(const GGZGameType *type,
				       unsigned int num)
{
	if (!type)
		return 0;

	return _ggzcore_gametype_num_bots_is_valid(type, num);
}

int ggzcore_gametype_get_num_namedbots(const GGZGameType *type)
{
	if (!type)
		return 0;

	return _ggzcore_gametype_get_num_namedbots(type);
}

const char* ggzcore_gametype_get_namedbot_name(const GGZGameType *type, unsigned int num)
{
	if (!type)
		return NULL;

	return _ggzcore_gametype_get_namedbot_name(type, num);
}

const char* ggzcore_gametype_get_namedbot_class(const GGZGameType *type, unsigned int num)
{
	if (!type)
		return NULL;

	return _ggzcore_gametype_get_namedbot_class(type, num);
}

