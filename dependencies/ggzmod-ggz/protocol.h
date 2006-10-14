/*
 * File: protocols.h
 * Author: Brent Hendricks
 * Project: GGZ
 * Date: 10/18/99
 * Desc: Protocol enumerations, etc.
 * $Id$
 *
 * Copyright (C) 1999 Brent Hendricks.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */


#ifndef __GGZ_CLIENT_PROTOCOL_H
#define __GGZ_CLIENT_PROTOCOL_H

/*
 * Protocols for GGZMOD game client <-> core client communication.
 * Must be kept synchronized between ggzmod-ggz and ggzmod-game!
 */

/** Messages sent from the game client to the ggz core client. */
typedef enum {
	MSG_GAME_STATE,

	REQ_STAND,
	REQ_SIT,
	REQ_BOOT,
	REQ_BOT,
	REQ_OPEN,
	REQ_CHAT,

	REQ_INFO
} TableToControl;

/** Messages sent from the ggz core client to the game client. */
typedef enum {
	MSG_GAME_LAUNCH,
	MSG_GAME_SERVER,
	MSG_GAME_SERVER_FD,

	/* Info about this player (us). */
	MSG_GAME_PLAYER,

	/* Sent from GGZ to game to tell of a seat change.  No
	   response is necessary. */
	MSG_GAME_SEAT,
	MSG_GAME_SPECTATOR_SEAT,

	MSG_GAME_CHAT,

	MSG_GAME_STATS,

	MSG_GAME_INFO
} ControlToTable;

#endif /* __GGZ_CLIENT_PROTOCOL_H */
