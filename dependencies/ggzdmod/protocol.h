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


#ifndef __GGZ_SERVER_PROTOCOL_H
#define __GGZ_SERVER_PROTOCOL_H

/*
 * Protocols for GGZDMOD game server <-> ggzd communication.
 */

/** Messages sent from the game server to the ggz server. */
typedef enum {
	/** @brief Signals the start of a MSG_LOG packet.
	 *
	 *  The packet is composed of:
	 *    - An integer containing MSG_LOG.
	 *    - An easysock-formatted string.
	 *  The packet may be sent at any time by the game server.  It
	 *  tells ggzd to log the message string.
	 */
	MSG_LOG,

	/** @brief Signals the start of a REQ_GAME_STATE packet.
	 *
	 *  The packet is composed of:
	 *    - An integer containing REQ_GAME_STATE.
	 *    - A single byte (char) containing the new game state.
	 *      The state is a GGZdModState enumerated value.
	 *  This packet tells the server of a change in game state.
	 *  The server will send a RSP_GAME_STATE packet in response.
	 */
	REQ_GAME_STATE,

	REQ_NUM_SEATS,
	REQ_BOOT,
	REQ_BOT,
	REQ_OPEN,

	MSG_GAME_REPORT,
	MSG_SAVEGAME_REPORT
} TableToControl;

/** Messages sent from the ggz server to the game server. */
typedef enum {
	/** @brief Signals the start of a MSG_GAME_LAUNCH packet.
	 *
	 *  The packet is composed of:
	 *    - An integer containing MSG_GAME_LAUNCH.
	 *    - A string containing the game's name
	 *    - An integer containing the number of seats at the table.
	 *    - An integer containing the number of spectators
	 *    - Seat data for every seat at the table:
	 *      - An integer containing the seat type (GGZSeatType).
	 *      - If the seat type is GGZ_SEAT_RESERVED, the name of the
	 *        player at the seat (as an easysock-formatted string).
	 *  This packet is sent to the table shortly after the table
	 *  executable has been launched.  GGZdMod-game, on receiving
	 *  this packet, updates its internal data and changes the
	 *  state from CREATED to WAITING.  It then notifies the table
	 *  and GGZ of these changes.
	 */
	MSG_GAME_LAUNCH,

	/** @brief Signals the start of a REQ_GAME_SEAT packet.
	 *
	 *  The packet is composed of:
	 *    - An integer containing MSG_GAME_SEAT.
	 *    - An integer containing the seat number of the changed seat.
	 *    - An integer containing the GGZ_SEAT_TYPE.
	 *    - An easysock-formatted string contining the player's name,
	 *      or "" if the player has no name.
	 *    - An easysock-formatted FD containing the FD for the
	 *      player's client socket, IF the seat is a player
	 *  This packet is sent any time a seat is modified.  It gives
	 *  the table all of the information it needs about the new
	 *  seat.
	 */
	MSG_GAME_SEAT,

	/** @brief Signals the start of a MSG_GAME_SPECTATOR_SEAT packet.
	 *
	 *  The packet is composed of:
	 *    - An integer containing MSG_GAME_SPECTATOR_SEAT.
	 *    - An integer containing the spectator seat number of the
	 *      changed spectator.
	 *    - An easysock-formatted string containing the spectator's name,
	 *      or "" for an empty seat.
	 *    - An easysock-formatted FD containing thee FD for the
	 *      spectator's client socket, IF the spectator seat is occupied.
	 */
	MSG_GAME_SPECTATOR_SEAT,

	/** @brief Signals a player changing seats.
	 *
	 *  The packet is composed of:
	 *    - An integer containing MSG_GAME_RESEAT.
	 *    - An integer, boolean, whether the player *was* a spectator.
	 *    - An integer, the (spectator) seat the player *was* at.
	 *    - An integer, boolean, whether the player *will be* a spectator.
	 *    - An integer, the (speectator) seat the player *will be* at.
	 */
	MSG_GAME_RESEAT,

	/** @brief Signals the start of a RSP_GAME_STATE packet.
	 *
	 *  The packet is composed of:
	 *    - An integer containing RSP_GAME_STATE.
	 *  This packet is sent in response to a REQ_GAME_STATE.  It's
	 *  only effect is to let the table know the information has
	 *  been received.
	 */
	RSP_GAME_STATE,
} ControlToTable;

#endif /* __GGZ_SERVER_PROTOCOL_H */
