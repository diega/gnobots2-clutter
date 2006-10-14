/*
 * File: ggz_common.c
 * Author: GGZ Dev Team
 * Project: GGZ Common Library
 * Date: 01/13/2002
 * $Id$
 *
 * This provides GGZ-specific functionality that is common to
 * some or all of the ggz-server, game-server, ggz-client, and
 * game-client.
 *
 * Copyright (C) 2002 Brent Hendricks.
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

#include "config.h"

#include <string.h>
#include <strings.h> /* For strcasecmp */

#include "ggz.h"
#include "ggz_common.h"

#include "support.h"

#define OPEN_SEAT_NAME "open"
#define BOT_SEAT_NAME "bot"
#define PLAYER_SEAT_NAME "player"
#define RESERVED_SEAT_NAME "reserved"
#define ABANDONED_SEAT_NAME "abandoned"
#define NONE_SEAT_NAME "none"

const char *ggz_seattype_to_string(GGZSeatType type)
{
	switch (type) {
	case GGZ_SEAT_OPEN:
		return OPEN_SEAT_NAME;
	case GGZ_SEAT_BOT:
		return BOT_SEAT_NAME;
	case GGZ_SEAT_RESERVED:
		return RESERVED_SEAT_NAME;
	case GGZ_SEAT_PLAYER:
		return PLAYER_SEAT_NAME;
	case GGZ_SEAT_ABANDONED:
		return ABANDONED_SEAT_NAME;
	case GGZ_SEAT_NONE:
		return NONE_SEAT_NAME;
	}

	ggz_error_msg("ggz_seattype_to_string: "
		      "invalid seattype %d given.", type);
	return NONE_SEAT_NAME;
}

GGZSeatType ggz_string_to_seattype(const char *type_str)
{
	/* If it doesn't match _exactly_ we return GGZ_SEAT_NONE.  This
	   is bad for, say, user input, but perfectly acceptable as an
	   inverse to ggz_seattype_to_string(). */
	if (!type_str)
		return GGZ_SEAT_NONE;

	if (strcasecmp(type_str, OPEN_SEAT_NAME) == 0)
		return GGZ_SEAT_OPEN;
	else if (strcasecmp(type_str, BOT_SEAT_NAME) == 0)
		return GGZ_SEAT_BOT;
	else if (strcasecmp(type_str, RESERVED_SEAT_NAME) == 0)
		return GGZ_SEAT_RESERVED;
	else if (strcasecmp(type_str, PLAYER_SEAT_NAME) == 0)
		return GGZ_SEAT_PLAYER;
	else if (strcasecmp(type_str, ABANDONED_SEAT_NAME) == 0)
		return GGZ_SEAT_ABANDONED;

	return GGZ_SEAT_NONE;
}

#define NORMAL_CHAT_NAME "normal"
#define ANNOUNCE_CHAT_NAME "announce"
#define BEEP_CHAT_NAME "beep"
#define PRIVATE_CHAT_NAME "private"
#define TABLE_CHAT_NAME "table"

const char *ggz_chattype_to_string(GGZChatType type)
{
	switch (type) {
	case GGZ_CHAT_NORMAL:
		return NORMAL_CHAT_NAME;
	case GGZ_CHAT_ANNOUNCE:
		return ANNOUNCE_CHAT_NAME;
	case GGZ_CHAT_BEEP:
		return BEEP_CHAT_NAME;
	case GGZ_CHAT_PERSONAL:
		return PRIVATE_CHAT_NAME;
	case GGZ_CHAT_TABLE:
		return TABLE_CHAT_NAME;
	case GGZ_CHAT_UNKNOWN:
		break;
	}

	ggz_error_msg("ggz_chattype_to_string: "
		      "invalid chattype %d given.", type);
	return ""; /* ? */
}

GGZChatType ggz_string_to_chattype(const char *type_str)
{
	/* If it doesn't match _exactly_ we return GGZ_CHAT_NONE.  This
	   is bad for, say, user input, but perfectly acceptable as an
	   inverse to ggz_chattype_to_string(). */
	if (!type_str)
		return GGZ_CHAT_UNKNOWN;

	if (strcasecmp(type_str, NORMAL_CHAT_NAME) == 0)
		return GGZ_CHAT_NORMAL;
	else if (strcasecmp(type_str, ANNOUNCE_CHAT_NAME) == 0)
		return GGZ_CHAT_ANNOUNCE;
	else if (strcasecmp(type_str, BEEP_CHAT_NAME) == 0)
		return GGZ_CHAT_BEEP;
	else if (strcasecmp(type_str, PRIVATE_CHAT_NAME) == 0)
		return GGZ_CHAT_PERSONAL;
	else if (strcasecmp(type_str, TABLE_CHAT_NAME) == 0)
		return GGZ_CHAT_TABLE;

	return GGZ_CHAT_UNKNOWN;
}

#define GAG_ADMIN_NAME "gag"
#define UNGAG_ADMIN_NAME "ungag"
#define KICK_ADMIN_NAME "kick"
#define BAN_ADMIN_NAME "ban"

const char *ggz_admintype_to_string(GGZAdminType type)
{
	switch (type) {
	case GGZ_ADMIN_GAG:
		return GAG_ADMIN_NAME;
	case GGZ_ADMIN_UNGAG:
		return UNGAG_ADMIN_NAME;
	case GGZ_ADMIN_KICK:
		return KICK_ADMIN_NAME;
	case GGZ_ADMIN_BAN:
		return BAN_ADMIN_NAME;
	case GGZ_ADMIN_UNKNOWN:
		break;
	}

	ggz_error_msg("ggz_admintype_to_string: "
		      "invalid admintype %d given.", type);
	return ""; /* ? */
}

GGZAdminType ggz_string_to_admintype(const char *type_str)
{
	if (!type_str)
		return GGZ_ADMIN_UNKNOWN;

	if (strcasecmp(type_str, GAG_ADMIN_NAME) == 0)
		return GGZ_ADMIN_GAG;
	else if (strcasecmp(type_str, UNGAG_ADMIN_NAME) == 0)
		return GGZ_ADMIN_UNGAG;
	else if (strcasecmp(type_str, KICK_ADMIN_NAME) == 0)
		return GGZ_ADMIN_KICK;
	else if (strcasecmp(type_str, BAN_ADMIN_NAME) == 0)
		return GGZ_ADMIN_BAN;

	return GGZ_ADMIN_UNKNOWN;
}

#define NORMAL_LEAVETYPE_NAME "normal"
#define BOOT_LEAVETYPE_NAME "boot"
#define GAMEOVER_LEAVETYPE_NAME "gameover"
#define GAMEERROR_LEAVETYPE_NAME "gameerror"

const char *ggz_leavetype_to_string(GGZLeaveType type)
{
	switch (type) {
	case GGZ_LEAVE_NORMAL:
		return NORMAL_LEAVETYPE_NAME;
	case GGZ_LEAVE_BOOT:
		return BOOT_LEAVETYPE_NAME;
	case GGZ_LEAVE_GAMEOVER:
		return GAMEOVER_LEAVETYPE_NAME;
	case GGZ_LEAVE_GAMEERROR:
		return GAMEERROR_LEAVETYPE_NAME;
	}

	ggz_error_msg("ggz_leavetype_to_string: "
		      "invalid leavetype %d given.", type);
	return ""; /* ? */
}

GGZLeaveType ggz_string_to_leavetype(const char *type_str)
{
	if (!type_str)
		return GGZ_LEAVE_GAMEERROR;

	if (strcasecmp(type_str, NORMAL_LEAVETYPE_NAME) == 0)
		return GGZ_LEAVE_NORMAL;
	else if (strcasecmp(type_str, BOOT_LEAVETYPE_NAME) == 0)
		return GGZ_LEAVE_BOOT;
	else if (strcasecmp(type_str, GAMEOVER_LEAVETYPE_NAME) == 0)
		return GGZ_LEAVE_GAMEOVER;
	else if (strcasecmp(type_str, GAMEERROR_LEAVETYPE_NAME) == 0)
		return GGZ_LEAVE_GAMEERROR;

	return GGZ_LEAVE_GAMEERROR;
}

#define GUEST_PLAYER_NAME "guest"
#define NORMAL_PLAYER_NAME "normal"
#define ADMIN_PLAYER_NAME "admin"
#define HOST_PLAYER_NAME "host"
#define BOT_PLAYER_NAME "bot"
#define UNKNOWN_PLAYER_NAME "unknown"

const char *ggz_playertype_to_string(GGZPlayerType type)
{
	switch (type) {
	case GGZ_PLAYER_GUEST:
		return GUEST_PLAYER_NAME;
	case GGZ_PLAYER_NORMAL:
		return NORMAL_PLAYER_NAME;
	case GGZ_PLAYER_ADMIN:
		return ADMIN_PLAYER_NAME;
	case GGZ_PLAYER_HOST:
		return HOST_PLAYER_NAME;
	case GGZ_PLAYER_BOT:
		return BOT_PLAYER_NAME;
	case GGZ_PLAYER_UNKNOWN:
		return UNKNOWN_PLAYER_NAME;
	}

	ggz_error_msg("ggz_playertype_to_string: "
		      "invalid playertype %d given.", type);
	return UNKNOWN_PLAYER_NAME; /* ? */
}

GGZPlayerType ggz_string_to_playertype(const char *type_str)
{
	if (!type_str) {
		return GGZ_PLAYER_UNKNOWN;
	}

	if (strcasecmp(type_str, NORMAL_PLAYER_NAME) == 0) {
		return GGZ_PLAYER_NORMAL;
	} else if (strcasecmp(type_str, GUEST_PLAYER_NAME) == 0) {
		return GGZ_PLAYER_GUEST;
	} else if (strcasecmp(type_str, ADMIN_PLAYER_NAME) == 0) {
		return GGZ_PLAYER_ADMIN;
	} else if (strcasecmp(type_str, HOST_PLAYER_NAME) == 0) {
		return GGZ_PLAYER_HOST;
	} else if (strcasecmp(type_str, BOT_PLAYER_NAME) == 0) {
		return GGZ_PLAYER_BOT;
	}

	return GGZ_PLAYER_UNKNOWN; /* ? */
}

#define TRUE_NAME "true"
#define FALSE_NAME "false"

char *bool_to_str(int bool_val)
{
	if (bool_val)
		return TRUE_NAME;
	else
		return FALSE_NAME;
}

/* Convert a possibly-null string that should contain "true" or "false"
   to a boolean (int) value.  The default value is returned if an invalid
   or empty value is sent. */
int str_to_bool(const char *str, int dflt)
{
	if (!str)
		return dflt;
  
	if (strcasecmp(str, TRUE_NAME) == 0)
		return 1;

	if (strcasecmp(str, FALSE_NAME) == 0)
		return 0;

	return dflt;
}

const char *ggz_error_to_string(GGZClientReqError err)
{
	switch (err) {
	case E_OK:
		return "ok";
	case E_USR_LOOKUP:
		return "usr lookup";
	case E_BAD_OPTIONS:
		return "bad options";
	case E_ROOM_FULL:
		return "room full";
	case E_TABLE_FULL:
		return "table full";
	case E_TABLE_EMPTY:
		return "table empty";
	case E_LAUNCH_FAIL:
		return "launch fail";
	case E_JOIN_FAIL:
		return "join fail";
	case E_NO_TABLE:
		return "no table";
	case E_LEAVE_FAIL:
		return "leave fail";
	case E_LEAVE_FORBIDDEN:
		return "leave forbidden";
	case E_ALREADY_LOGGED_IN:
		return "already logged in";
	case E_NOT_LOGGED_IN
		: return "not logged in";
	case E_NOT_IN_ROOM:
		return "not in room";
	case E_AT_TABLE:
		return "at table";
	case E_IN_TRANSIT:
		return "in transit";
	case E_NO_PERMISSION:
		return "no permission";
	case E_BAD_XML:
		return "bad xml";
	case E_SEAT_ASSIGN_FAIL:
		return "seat assign fail";
	case E_NO_CHANNEL:
		return "no channel";
	case E_TOO_LONG:
		return "too long";
	case E_BAD_USERNAME:
		return "bad username";
	case E_USR_TYPE:
		return "wrong login type";
	case E_USR_FOUND:
		return "user not found";
	case E_USR_TAKEN:
		return "username already taken";
	case E_NO_STATUS:
	case E_UNKNOWN:
		break;
	}

	ggz_error_msg("ggz_error_to_string: invalid error %d given.", err);
	return "[unknown]";
}

GGZClientReqError ggz_string_to_error(const char *str)
{
	if (!str)
		return E_OK;

	if (!strcasecmp(str, "ok"))
		return E_OK;
	if (!strcasecmp(str, "0")) {
		/* This provides a tiny bit of backwards compatability.
		   It should go away eventually. */
		return E_OK;
	}
	if (!strcasecmp(str, "usr lookup"))
		return E_USR_LOOKUP;
	if (!strcasecmp(str, "bad options"))
		return E_BAD_OPTIONS;
	if (!strcasecmp(str, "room full"))
		return E_ROOM_FULL;
	if (!strcasecmp(str, "table full"))
		return E_TABLE_FULL;
	if (!strcasecmp(str, "table empty"))
		return E_TABLE_EMPTY;
	if (!strcasecmp(str, "launch fail"))
		return E_LAUNCH_FAIL;
	if (!strcasecmp(str, "join fail"))
		return E_JOIN_FAIL;
	if (!strcasecmp(str, "no table"))
		return E_NO_TABLE;
	if (!strcasecmp(str, "leave fail"))
		return E_LEAVE_FAIL;
	if (!strcasecmp(str, "leave forbidden"))
		return E_LEAVE_FORBIDDEN;
	if (!strcasecmp(str, "already logged in"))
		return E_ALREADY_LOGGED_IN;
	if (!strcasecmp(str, "not logged in"))
		return E_NOT_LOGGED_IN;
	if (!strcasecmp(str, "not in room"))
		return E_NOT_IN_ROOM;
	if (!strcasecmp(str, "at table"))
		return E_AT_TABLE;
	if (!strcasecmp(str, "in transit"))
		return E_IN_TRANSIT;
	if (!strcasecmp(str, "no permission"))
		return E_NO_PERMISSION;
	if (!strcasecmp(str, "bad xml"))
		return E_BAD_XML;
	if (!strcasecmp(str, "seat assign fail"))
		return E_SEAT_ASSIGN_FAIL;
	if (!strcasecmp(str, "no channel"))
		return E_NO_CHANNEL;
	if (!strcasecmp(str, "too long"))
		return E_TOO_LONG;
	if (!strcasecmp(str, "bad username"))
		return E_BAD_USERNAME;
	if (!strcasecmp(str, "wrong login type"))
		return E_USR_TYPE;
	if (!strcasecmp(str, "user not found"))
		return E_USR_FOUND;
	if (!strcasecmp(str, "username already taken"))
		return E_USR_TAKEN;

	return E_UNKNOWN;
}
