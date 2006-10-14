/*
 * File: state.c
 * Author: Brent Hendricks
 * Project: GGZ Core Client Lib
 * Date: 9/22/00
 * $Id$
 *
 * Code for handling state manipulations
 *
 * Copyright (C) 2000 Brent Hendricks.
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

#include <ggz.h>

#include "hook.h"
#include "state.h"

/* Structure to represent state transition pairs */
struct _GGZTransition {
	
	/* Transition ID */
	GGZTransID id;

	/* Next state */
	GGZStateID next;
};


/* Structure for a particular client state */
struct _GGZState {

	/* Unique id number */
	GGZStateID id;
	
	/* Descriptive string (mainly for debugging purposes) */
	const char *name;

	/* Array of valid state transitions */
	struct _GGZTransition *transitions;
};


/* Giant list of transitions for each state */
static struct _GGZTransition _offline_transitions[] = {
	{GGZ_TRANS_CONN_TRY,     GGZ_STATE_CONNECTING},
	{-1, -1}
};

static struct _GGZTransition _connecting_transitions[] = {
	{GGZ_TRANS_CONN_OK,      GGZ_STATE_ONLINE},
	{GGZ_TRANS_CONN_FAIL,    GGZ_STATE_OFFLINE},
	{-1, -1}
};

static struct _GGZTransition _reconnecting_transitions[] = {
	{GGZ_TRANS_CONN_OK,      GGZ_STATE_ONLINE},
	{GGZ_TRANS_CONN_FAIL,    GGZ_STATE_RECONNECTING},
	{-1, -1}
};

static struct _GGZTransition _online_transitions[] = {
	{GGZ_TRANS_LOGIN_TRY,    GGZ_STATE_LOGGING_IN},
	{GGZ_TRANS_LOGOUT_TRY,   GGZ_STATE_LOGGING_OUT},
	{GGZ_TRANS_NET_ERROR,    GGZ_STATE_OFFLINE},
	{GGZ_TRANS_PROTO_ERROR,  GGZ_STATE_OFFLINE},
	{-1, -1}
};

static struct _GGZTransition _logging_in_transitions[] = {
	{GGZ_TRANS_LOGIN_OK,     GGZ_STATE_LOGGED_IN},
	{GGZ_TRANS_LOGIN_FAIL,   GGZ_STATE_ONLINE},
	{GGZ_TRANS_LOGOUT_TRY,   GGZ_STATE_LOGGING_OUT},
	{GGZ_TRANS_NET_ERROR,    GGZ_STATE_OFFLINE},
	{GGZ_TRANS_PROTO_ERROR,  GGZ_STATE_OFFLINE},
	{-1, -1}
};

static struct _GGZTransition _logged_in_transitions[] = {
	{GGZ_TRANS_ENTER_TRY,    GGZ_STATE_ENTERING_ROOM},
	{GGZ_TRANS_LOGOUT_TRY,   GGZ_STATE_LOGGING_OUT},
	{GGZ_TRANS_NET_ERROR,    GGZ_STATE_OFFLINE},
	{GGZ_TRANS_PROTO_ERROR,  GGZ_STATE_OFFLINE},
	{-1, -1}
};

static struct _GGZTransition _entering_room_transitions[] = {
	{GGZ_TRANS_ENTER_OK,     GGZ_STATE_IN_ROOM},
	{GGZ_TRANS_ENTER_FAIL,   GGZ_STATE_LOGGED_IN},
	{GGZ_TRANS_LOGOUT_TRY,   GGZ_STATE_LOGGING_OUT},
	{GGZ_TRANS_NET_ERROR,    GGZ_STATE_OFFLINE},
	{GGZ_TRANS_PROTO_ERROR,  GGZ_STATE_OFFLINE},
	{-1, -1}
};

static struct _GGZTransition _in_room_transitions[] = {
	{GGZ_TRANS_ENTER_TRY,    GGZ_STATE_BETWEEN_ROOMS},
	{GGZ_TRANS_LAUNCH_TRY,   GGZ_STATE_LAUNCHING_TABLE},
	{GGZ_TRANS_JOIN_TRY,     GGZ_STATE_JOINING_TABLE},
	{GGZ_TRANS_LOGOUT_TRY,   GGZ_STATE_LOGGING_OUT},
	{GGZ_TRANS_NET_ERROR,    GGZ_STATE_OFFLINE},
	{GGZ_TRANS_PROTO_ERROR,  GGZ_STATE_OFFLINE},
	{-1, -1}
};

static struct _GGZTransition _between_rooms_transitions[] = {
	{GGZ_TRANS_ENTER_OK,     GGZ_STATE_IN_ROOM},
	{GGZ_TRANS_ENTER_FAIL,   GGZ_STATE_IN_ROOM},
	{GGZ_TRANS_LOGOUT_TRY,   GGZ_STATE_LOGGING_OUT},
	{GGZ_TRANS_NET_ERROR,    GGZ_STATE_OFFLINE},
	{GGZ_TRANS_PROTO_ERROR,  GGZ_STATE_OFFLINE},
	{-1, -1}
};

static struct _GGZTransition _launching_table_transitions[] = {
	/* For now, server automatically tries to join us */
	{GGZ_TRANS_LAUNCH_OK,    GGZ_STATE_JOINING_TABLE},
	{GGZ_TRANS_LAUNCH_FAIL,  GGZ_STATE_IN_ROOM},
	{GGZ_TRANS_NET_ERROR,    GGZ_STATE_OFFLINE},
	{GGZ_TRANS_PROTO_ERROR,  GGZ_STATE_OFFLINE},
	{-1, -1}
};

static struct _GGZTransition _joining_table_transitions[] = {
	{GGZ_TRANS_JOIN_OK,      GGZ_STATE_AT_TABLE},
	{GGZ_TRANS_JOIN_FAIL,    GGZ_STATE_IN_ROOM},
	{GGZ_TRANS_NET_ERROR,    GGZ_STATE_OFFLINE},
	{GGZ_TRANS_PROTO_ERROR,  GGZ_STATE_OFFLINE},
	{-1, -1}
};

static struct _GGZTransition _at_table_transitions[] = {
	{GGZ_TRANS_LEAVE_TRY,    GGZ_STATE_LEAVING_TABLE},
	{GGZ_TRANS_LEAVE_OK,     GGZ_STATE_IN_ROOM},
	{GGZ_TRANS_NET_ERROR,    GGZ_STATE_OFFLINE},
	{GGZ_TRANS_PROTO_ERROR,  GGZ_STATE_OFFLINE},
	{-1, -1}
};

static struct _GGZTransition _leaving_table_transitions[] = {
	{GGZ_TRANS_LEAVE_OK,     GGZ_STATE_IN_ROOM},
	{GGZ_TRANS_LEAVE_FAIL,   GGZ_STATE_AT_TABLE},
	{GGZ_TRANS_NET_ERROR,    GGZ_STATE_OFFLINE},
	{GGZ_TRANS_PROTO_ERROR,  GGZ_STATE_OFFLINE},
	{-1, -1}
};

static struct _GGZTransition _logging_out_transitions[] = {
	{GGZ_TRANS_NET_ERROR,    GGZ_STATE_OFFLINE},
	{GGZ_TRANS_PROTO_ERROR,  GGZ_STATE_OFFLINE},
	{GGZ_TRANS_LOGOUT_OK,    GGZ_STATE_OFFLINE},
	{-1, -1}
};


/* Array of all GGZ states */
static struct _GGZState _ggz_states[] = {
	{GGZ_STATE_OFFLINE,         "offline",         _offline_transitions},
	{GGZ_STATE_CONNECTING,      "connecting",      _connecting_transitions},
	{GGZ_STATE_RECONNECTING,    "reconnecting",    _reconnecting_transitions},
	{GGZ_STATE_ONLINE,          "online",          _online_transitions}, 
	{GGZ_STATE_LOGGING_IN,      "logging_in",      _logging_in_transitions},
	{GGZ_STATE_LOGGED_IN,       "logged_in",       _logged_in_transitions},
	{GGZ_STATE_ENTERING_ROOM,   "entering_room",   _entering_room_transitions},
	{GGZ_STATE_IN_ROOM,         "in_room",         _in_room_transitions},
	{GGZ_STATE_BETWEEN_ROOMS,   "between_rooms",   _between_rooms_transitions},
	{GGZ_STATE_LAUNCHING_TABLE, "launching_table", _launching_table_transitions},
	{GGZ_STATE_JOINING_TABLE,   "joining_table",   _joining_table_transitions},
	{GGZ_STATE_AT_TABLE,        "at_table",        _at_table_transitions},
	{GGZ_STATE_LEAVING_TABLE,   "leaving_table",   _leaving_table_transitions},
	{GGZ_STATE_LOGGING_OUT,     "logging_out",     _logging_out_transitions},
};


/* Publicly Exported functions (prototypes in ggzcore.h) */

/* Internal library functions (prototypes in state.h) */

void _ggzcore_state_transition(GGZTransID trans, GGZStateID *cur)
{
	int i = 0;
	struct _GGZTransition *transitions;
	GGZStateID next = -1;

	transitions = _ggz_states[*cur].transitions;
	
	/* Look through valid transitions to see if this one is OK */
	while (transitions[i].id != -1) {
		if (transitions[i].id == trans) {
			next = transitions[i].next;
			break;
		}
		++i;
	}

	if (next != *cur && next != -1) {
		ggz_debug(GGZCORE_DBG_STATE, "State transition %s -> %s", 
			  _ggz_states[*cur].name, 
			  _ggz_states[next].name);
		*cur = next;
	} else if (next == -1) {
		ggz_error_msg("No state transition for %d from %s!", 
			      trans, _ggz_states[*cur].name);
	}
}


/* Static functions internal to this file */

