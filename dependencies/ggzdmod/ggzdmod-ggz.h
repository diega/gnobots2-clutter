/* 
 * File: ggzdmod-ggz.h
 * Author: GGZ Dev Team
 * Project: ggzdmod
 * Date: 10/27/02
 * Desc: GGZ game module functions, GGZ-side only
 * $Id$
 *
 * This file contains the GGZ-only interface for the ggzdmod library.  This
 * library facilitates the communication between the GGZ server (ggzd)
 * and game servers.  See ggzdmod.h for the rest of the library.
 *
 * Copyright (C) 2001-2002 GGZ Development Team.
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

#ifndef __GGZDMOD_GGZ_H__
#define __GGZDMOD_GGZ_H__

/** @brief Set the number of seats for the table.
 *  @param ggzdmod The GGZdMod object.
 *  @param num_seats The number of seats to set.
 *  @return 0 on success, -1 on failure.
 *  @note This will only work for ggzd.
 *  @todo Allow the table to change the number of seats. */
int ggzdmod_set_num_seats(GGZdMod * ggzdmod, int num_seats);

/** @brief Set the module executable, pwd, and arguments
 *
 *  GGZdmod must execute and launch the game to start a table; this
 *  function allows ggzd to specify how this should be done.
 *  @note This should not be called by the table, only ggzd.
 *  @param ggzdmod The GGZdmod object.
 *  @param game Name of the game as per its .dsc file
 *  @param pwd The working directory for the game, or NULL.
 *  @param args The arguments for the program, as needed by exec.
 *  @note The pwd directory must already exist.
 *  @note The executable must be an absolute path (or relative to pwd).
 */
void ggzdmod_set_module(GGZdMod * ggzdmod,
                        const char *game, const char *pwd, char **args);

/** @brief Set seat data.
 *
 *  A game server or the ggz server can use this function to set
 *  data about a seat.  The game server may only change the following
 *  things about a seat:
 *    - The name (only if the seat is a bot).
 *    - The socket FD (only if the FD is -1).
 *  @param seat The new seat structure (which includes seat number).
 *  @return 0 on success, negative on failure.
 *  @todo The game should be able to toggle between BOT and OPEN seats.
 *  @todo The game should be able to kick a player out of the table.
 */
int ggzdmod_set_seat(GGZdMod * ggzdmod, GGZSeat * seat);

/** @brief Set spectator data.
 *
 *  A game server or the ggz server can use this function to set
 *  data about a spectator.  The game server may only change the following
 *  things about a spectator:
 *    - The socket FD (only if the FD is -1).
 *  @param spectator The new spectator structure (which includes spectator number).
 *  @return 0 on success, negative on failure.
 */
int ggzdmod_set_spectator(GGZdMod * ggzdmod, GGZSpectator * spectator);


/** @brief Change a player's seat.
 *
 *  Move a player from a seat to another seat, from a spectator seat to
 *  a seat, or from a seat to a spectator seat.
 *  @param old_seat The (spectator) seat number of the player.
 *  @param was_spectator Whether the seat is a normal or spectator seat.
 *  @param new_seat The (spectator) seat the player moves to.
 *  @param is_spectator Whether the new seat is a normal/spectator seat.
 *  @note This should only be called by GGZ.
 *  @return 0 on success, negative on failure.
 */
int ggzdmod_reseat(GGZdMod * ggzdmod,
		   int old_seat, int was_spectator,
		   int new_seat, int is_spectator);

/** @brief Statistics structure as reported by a game.
 *
 *  Games which call ggzdmod_report_game() will submit scores for each
 *  player. This structure will be used to transfer it to ggzd.
 */
typedef struct {
	int num_players;
	char **names;
	GGZSeatType *types;
	int *teams;
	GGZGameResult *results;
	int *scores;
} GGZdModGameReportData;

/** @brief Callback events (GGZ-side additions).
 *
 *  These events are triggered by the game server and delivered
 *  to the GGZ server. The other events, those sent by ggzd to
 *  the game server and those sent by the two to each other,
 *  are held in GGZdModEvent in ggzdmod.h.
 */
typedef enum {
	/** @brief For GGZ only.  Reports the results of a game. */
	GGZDMOD_EVENT_GAMEREPORT = GGZDMOD_EVENT_ERROR + 1,

	/** @brief For GGZ only.  Reports a savegame. */
	GGZDMOD_EVENT_SAVEGAMEREPORT,

	/** @brief Module log request
	 *  This event occurs when a log request happens.  This will
	 *  only be used by the GGZ server; the game server should
	 *  use ggzdmod_log to generate the log. */
	GGZDMOD_EVENT_LOG,

	/* @brief GGZ-side only.  Requests a change in the number of seats. */
	GGZDMOD_EVENT_REQ_NUM_SEATS,

	/* @brief GGZ-side only.  Requests to boot a player. */
	GGZDMOD_EVENT_REQ_BOOT,

	/* @brief GGZ-side only.  Requests to fill in an AI player. */
	GGZDMOD_EVENT_REQ_BOT,

	/* @brief GGZ-side only.  Requests to open up a seat. */
	GGZDMOD_EVENT_REQ_OPEN,

	GGZDMOD_EVENT_ERROR_INTERNAL
	/* GGZDMOD_EVENT_ERROR_INTERNAL must be the last one! */
} GGZdModEventInternal;

#endif /* __GGZDMOD_GGZ_H__ */
