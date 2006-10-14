/* 
 * File: ggzmod.h
 * Author: GGZ Dev Team
 * Project: ggzmod
 * Date: 10/14/01
 * Desc: GGZ game module functions
 * $Id$
 *
 * This file contains the main interface for the ggzmod library.  This
 * library facilitates the communication between the GGZ core client (ggz)
 * and game clients.  This file provides a unified interface that can be
 * used at both ends.
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

#include <ggz.h> /* libggz */

/** @file ggzmod.h
 *  @brief Common functions for interfacing a game client and GGZ.
 *
 * This file contains all libggzmod functions used by game clients to
 * interface with GGZ.  Just include ggzmod.h and make sure
 * your program is linked with libggzmod.  Then use the functions below as
 * appropriate.
 *
 * GGZmod currently provides an event-driven interface.  Data from
 * communication sockets is read in by the library, and a handler function
 * (registered as a callback) is invoked to handle any events.  The calling
 * program should not read/write data from/to the GGZ socket unless it really
 * knows what it is doing.
 *
 * This does not apply to the game server sockets: ggzmod provides
 * one file desriptor for communicating (TCP) to the game server.  If data
 * is ready to be read this file descriptor, ggzmod may invoke the appropriate
 * handler (see below), but will never actually read any data.
 *
 * For more information, see the documentation at http://www.ggzgamingzone.org/.
 */


#ifndef __GGZMOD_H__
#define __GGZMOD_H__

#define GGZMOD_VERSION_MAJOR 0
#define GGZMOD_VERSION_MINOR 0
#define GGZMOD_VERSION_MICRO 14
#define GGZMOD_VERSION_IFACE "4:0:0"

#include <ggz_common.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Table states
 *
 *  Each table has a current "state" that is tracked by ggzmod.  First
 *  the table is executed and begins running.  Then it receives a launch
 *  event from GGZ and begins waiting for players.  At some point a game
 *  will be started and played at the table, after which it may return
 *  to waiting.  Eventually the table will probably halt and then the
 *  program will exit.
 *
 *  More specifically, the game is in the CREATED state when it is first
 *  executed.  It moves to the CONNECTED state after GGZ first communicates
 *  with it, and to WAITING after the connection is established with the
 *  game server.  After this, the game server may use ggzmod_set_state to
 *  change between WAITING, PLAYING, and DONE states.  A WAITING game is
 *  considered waiting for players (or whatever), while a PLAYING game is
 *  actively being played (this information may be, but currently is not,
 *  propogated back to GGZ for display purposes).  Once the state is changed
 *  to DONE, the table is considered dead and will exit shortly
 *  thereafter.
 *
 *  Each time the game state changes, a GGZMOD_EVENT_STATE event will be
 *  propogated to the game server.
 */
typedef enum {
	/** @brief Initial state.
	 *  The game starts out in this state.  Once the
	 *  state is changed it should never be changed back. */
	GGZMOD_STATE_CREATED,

	/** @brief Connected state.
	 *  After the GGZ client and game client get connected, the game
	 *  changes into this state automatically.  Once this happens
	 *  messages may be sent between these two.  Once the game leaves
	 *  this state it should never be changed back. */
	GGZMOD_STATE_CONNECTED,

	/** @brief Waiting state.
	 *  After the game client and game server are connected, the client
	 *  enters the waiting state.  The game client may now call
	 *  ggzmod_set_state to change between WAITING, PLAYING, and DONE
	 *  states. */
	GGZMOD_STATE_WAITING,

	/** @brief Playing state.
	 *  This state is only entered after the game client changes state
	 *  to it via ggzmod_set_state.  State may be changed back and forth
	 *  between WAITING and PLAYING as many times as are wanted. */
	GGZMOD_STATE_PLAYING,

	/** @brief Done state.
	 *  Once the game client is done running, ggzmod_set_state should be
	 *  called to set the state to done.  At this point nothing "new" can
	 *  happen.  The state cannot be changed again after this.  However
	 *  the game client will not be terminated by the GGZ client; GGZ
	 *  just waits for it to exit of its own volition. */
	GGZMOD_STATE_DONE
} GGZModState;

/** @brief Callback events.
 *
 *  Each of these is a possible GGZmod event.  For each event, the
 *  table may register a handler with GGZmod to handle that
 *  event.
 *  @see GGZModHandler
 *  @see ggzmod_set_handler
 */
typedef enum {
	/** @brief Module status changed
	 *  This event occurs when the game's status changes.  The old
	 *  state (a GGZModState*) is passed as the event's data.
	 *  @see GGZModState */
	GGZMOD_EVENT_STATE,

	/** @brief A new server connection has been made 
	 * This event occurs when a new connection to the game server
	 * has been made, either by the core client or by the game client
	 * itself.  The fd is passed as the event's data.
	 * @see ggzmod_connect */
	GGZMOD_EVENT_SERVER,

	/** @brief The player's seat status has changed.
	 *
	 *  This event occurs when the player's seat status changes;
	 *  i.e. he changes seats or starts/stops spectating.  The
	 *  event data is a int[2] pair consisting of the old
	 *  {is_spectator, seat_num}. */
	GGZMOD_EVENT_PLAYER,

	/** @brief A seat change.
	 *
	 *  This event occurs when a seat change occurs.  The old seat
	 *  (a GGZSeat*) is passed as the event's data.  The seat
	 *  information will be updated before the event is invoked. */
	GGZMOD_EVENT_SEAT,

	/** @brief A spectator seat change.
	 *
	 *  This event occurs when a spectator seat change occurs.  The
	 *  old spectator (a GGZSpectator*) is passed as the event's data.
	 *  The spectator information will be updated before the event is
	 *  invoked. */
	GGZMOD_EVENT_SPECTATOR_SEAT,

	/** @brief A chat message event.
	 *
	 *  This event occurs when we receive a chat.  The chat may have
	 *  originated in another game client or from the GGZ client; in
	 *  either case it will be routed to us.  The chat information
	 *  (a GGZChat*) is passed as the event's data.  Note that the
	 *  chat may originate with a player or a spectator, and they may
	 *  have changed seats or left the table by the time it gets to
	 *  us. */
	GGZMOD_EVENT_CHAT,

	/** A players' stats have been updated.
	 *  @see ggzmod_player_get_record
	 *  @see ggzmod_player_get_rating
	 *  @see ggzmod_player_get_ranking
	 *  @see ggzmod_player_get_highscore */
	GGZMOD_EVENT_STATS,

	/** @brief Player information has arrived.
	 *
	 *  Information has been requested about one or more players and
	 *  it has now arrived. The event data is a GGZPlayerInfo*
	 *  structure or NULL if info about all players was requested. */
	GGZMOD_EVENT_INFO,
	
	/** @brief An error has occurred
	 *  This event occurs when a GGZMod error has occurred.  An
	 *  error message (a char*) will be passed as the event's data.
	 *  GGZMod may attempt to recover from the error, but it is
	 *  not guaranteed that the GGZ connection will continue to
	 *  work after an error has happened. */
	GGZMOD_EVENT_ERROR
} GGZModEvent;

/** @brief The "type" of ggzmod.
 *
 * The "flavor" of GGZmod object this is.  Affects what operations are
 * allowed.
 */
typedef enum {
	GGZMOD_GGZ,	/**< Used by the ggz client ("ggz"). */
	GGZMOD_GAME	/**< Used by the game client ("table"). */
} GGZModType;

/** @brief A seat at a GGZ game table.
 *
 *  Each seat at the table is tracked like this.
 */
typedef struct {
	unsigned int num;	/**< Seat index; 0..(num_seats-1). */
	GGZSeatType type;	/**< Type of seat. */
	const char * name;	/**< Name of player occupying seat. */
} GGZSeat;

/** @brief A game spectator entry.
 *
 * Spectators are kept in their own table.  A spectator seat is occupied
 * if it has a name, empty if the name is NULL.
 */
typedef struct {
	unsigned int num;	/**< Spectator seat index */
	const char * name;	/**< The spectator's name (NULL => empty) */
} GGZSpectatorSeat;

typedef struct {
	const char *player;
	const char *message;    /**< Chat message (in UTF-8). */
} GGZChat;

typedef struct {
	unsigned int num;	/**< Player's seat index */
	const char *realname;	/**< Player's real name (NULL => not given). */
	const char *photo;	/**< Photo URL (NULL => no photo). */
	const char *host;	/**< Hostname or IP address (NULL for C/S games). */
} GGZPlayerInfo;

/** @brief A GGZmod object, used for tracking a ggz<->table connection.
 *
 * A game client should track a pointer to a GGZMod object; it
 * contains all the state information for communicating with GGZ.  The
 * GGZ client will track one such object for every game table that is
 * running.  */
typedef struct GGZMod GGZMod;

/** @brief Event handler prototype
 *
 *  A function of this type will be called to handle a ggzmod event.
 *  @param mod The ggzmod state object.
 *  @param e The event that has occured.
 *  @param data Pointer to additional data for the event.
 *  The additional data will be of the following form:
 *    - GGZMOD_EVENT_STATE: The old state (GGZModState*)
 *    - GGZMOD_EVENT_SERVER: The fd of the server connection (int*)
 *    - GGZMOD_EVENT_PLAYER: The old player data (int[2])
 *    - GGZMOD_EVENT_SEAT: The old seat (GGZSeat*)
 *    - GGZMOD_EVENT_SPECTATOR_SEAT: The old seat (GGZSpectatorSeat*)
 *    - GGZMOD_EVENT_ERROR: An error string (char*)
 */
typedef void (*GGZModHandler) (GGZMod * mod, GGZModEvent e, const void *data);

/** @brief Is the program running in GGZ mode?
 *
 *  Call this function to see if the program was actually launched by GGZ. 
 *  This can be used to give an error message if the executable is run
 *  outside of the GGZ environment, or for games that will run both inside
 *  and outside of GGZ.
 *  @return A boolean value indicating whether the program is running in GGZ.
 *  @note Should only be called by game clients, not by GGZ itself.
 */
int ggzmod_is_ggz_mode(void);

/* 
 * Creation functions
 */

/** @brief Create a new ggzmod object.
 *
 *  Before connecting through ggzmod, a new ggzmod object is needed.
 *  @param type The type of ggzmod.  Should be GGZMOD_GAME for game servers.
 *  @see GGZModType
 */
GGZMod *ggzmod_new(GGZModType type);

/** @brief Destroy a finished ggzmod object.
 *
 *  After the connection is through, the object may be freed.
 *  @param ggzmod The GGZMod object.
 */
void ggzmod_free(GGZMod * ggzmod);

/* 
 * Accessor functions
 */

/** @brief Get the file descriptor for the GGZMod socket.
 *
 *  @param ggzmod The GGZMod object.
 *  @return GGZMod's main ggz <-> table socket FD.
 */
int ggzmod_get_fd(GGZMod * ggzmod);

/** @brief Get the type of the ggzmod object.
 *  @param ggzmod The GGZMod object.
 *  @return The type of the GGZMod object (GGZ or GAME).
 */
GGZModType ggzmod_get_type(GGZMod * ggzmod);

/** @brief Get the current state of the table.
 *  @param ggzmod The GGZMod object.
 *  @return The state of the table.
 */
GGZModState ggzmod_get_state(GGZMod * ggzmod);

/** @brief Get the fd of the game server connection
 *  @param ggzmod The GGZMod object.
 *  @return The server connection fd
 */
int ggzmod_get_server_fd(GGZMod * ggzmod);

/** @brief Get the total number of seats at the table.
 *  @return The number of seats, or -1 on error.
 *  @note If no connection is present, -1 will be returned.
 *  @note While in GGZMOD_STATE_CREATED, we don't know the number of seats.
 */
int ggzmod_get_num_seats(GGZMod *ggzmod);

/** @brief Get all data for the specified seat.
 *  @param ggzmod The GGZMod object.
 *  @param seat The seat number (0..(number of seats - 1)).
 *  @return A valid GGZSeat structure, if seat is a valid seat.
 */
GGZSeat ggzmod_get_seat(GGZMod *ggzmod, int seat);

/** @brief Get the maximum number of spectators.
 *  This function returns the maximum number of spectator seats available.  A
 *  game can use this to iterate over the spectator seats to look for
 *  spectators occupying them.  Since spectators may come and go at any point
 *  and there is no limit on the number of spectators, you should consider
 *  this value to be dynamic and call this function again each time you're
 *  looking for spectators.
 *  @return The number of available spectator seats, or -1 on error.
 */
int ggzmod_get_num_spectator_seats(GGZMod *ggzmod);

/** @brief Get a spectator's data.
 *  @param ggzmod The GGZMod object.
 *  @param seat The number, between 0 and (number of spectators - 1).
 *  @return A valid GGZSpectator structure, if given a valid seat.
 */
GGZSpectatorSeat ggzmod_get_spectator_seat(GGZMod * ggzmod, int seat);

/** @brief Get data about this player.
 *
 *  Call this function to find out where at the table this player is
 *  sitting.
 *  @param ggzmod The GGZMod object.
 *  @param is_spectator Will be set to TRUE iff player is spectating.
 *  @param seat_num Will be set to the number of our (spectator) seat.
 *  @return The name of the player (or NULL on error).
 */
const char * ggzmod_get_player(GGZMod *ggzmod,
			       int *is_spectator, int *seat_num);

/** @brief Return gamedata pointer
 *
 *  Each GGZMod object can be given a "gamedata" pointer that is returned
 *  by this function.  This is useful for when a single process serves
 *  multiple GGZmod's.
 *  @param ggzmod The GGZMod object.
 *  @return A pointer to the gamedata block (or NULL if none).
 *  @see ggzmod_set_gamedata */
void * ggzmod_get_gamedata(GGZMod * ggzmod);

/** @brief Set gamedata pointer
 *  @param ggzmod The GGZMod object.
 *  @param data The gamedata block (or NULL for none).
 *  @see ggzmod_get_gamedata */
void ggzmod_set_gamedata(GGZMod * ggzmod, void * data);

/** @brief Set a handler for the given event.
 *
 *  As described above, GGZmod uses an event-driven structure.  Each
 *  time an event is called, the event handler (there can be only one)
 *  for that event will be called.  This function registers such an
 *  event handler.
 *  @param ggzmod The GGZmod object.
 *  @param e The GGZmod event.
 *  @param func The handler function being registered.
 *  @see ggzmod_get_gamedata
 */
void ggzmod_set_handler(GGZMod * ggzmod, GGZModEvent e, GGZModHandler func);


/* 
 * Event/Data handling
 */

/** @brief Check for and handle input.
 *
 *  This function handles input from the communications sockets:
 *    - It will check for input, but will not block.
 *    - It will monitor input from the GGZmod socket.
 *    - It will monitor input from player sockets only if a handler is
 *      registered for the PLAYER_DATA event.
 *    - It will call an event handler as necessary.
 *  @param ggzmod The ggzmod object.
 *  @return -1 on error, the number of events handled (0-1) on success.
 */
int ggzmod_dispatch(GGZMod * ggzmod);

/* 
 * Control functions
 */

/** @brief Change the table's state.
 *
 *  This function should be called to change the state of a table.
 *  A game can use this function to change state between WAITING
 *  and PLAYING, or to set it to DONE.
 *  @param ggzmod The ggzmod object.
 *  @param state The new state.
 *  @return 0 on success, -1 on failure/error.
 */
int ggzmod_set_state(GGZMod * ggzmod, GGZModState state);

/** @brief Connect to ggz.
 *
 *  Call this function to make an initial GGZ core client <-> game
 *  client connection. Afterwards
 *  @param ggzmod The ggzmod object.
 *  @return 0 on success, -1 on failure.
 */
int ggzmod_connect(GGZMod * ggzmod);

/** @brief Disconnect from ggz.
 *
 *  This terminates the link between the game client and the
 *  GGZ core client.
 *  @param ggzmod The ggzmod object.
 *  @return 0 on success, -1 on failure.
 */
int ggzmod_disconnect(GGZMod * ggzmod);


/** @brief Stand up (move from your seat into a spectator seat).
 *  @param ggzmod The ggzmod object. */
void ggzmod_request_stand(GGZMod * ggzmod);

/** @brief Sit down (move from a spectator seat into a player seat).
 *  @param ggzmod The ggzmod object.
 *  @param seat_num The seat to sit in. */
void ggzmod_request_sit(GGZMod * ggzmod, int seat_num);

/** @brief Boot a player.  Only the game host may do this.
 *  @param ggzmod The ggzmod object.
 *  @param name The name of the player to boot. */
void ggzmod_request_boot(GGZMod * ggzmod, const char *name);

/** @brief Change the requested seat from an open seat to a bot.
 *  @param ggzmod The ggzmod object.
 *  @param seat_num The number of the seat to toggle. */
void ggzmod_request_bot(GGZMod * ggzmod, int seat_num);

/** @brief Change the requested seat from a bot to an open seat.
 *  @param ggzmod The ggzmod object.
 *  @param seat_num The number of the seat to toggle. */
void ggzmod_request_open(GGZMod * ggzmod, int seat_num);

/** @brief Chat!  This initiates a table chat.
 *  @param ggzmod The ggzmod object.
 *  @param chat_msg The chat message.
 *  @note The chat message should be in UTF-8. */
void ggzmod_request_chat(GGZMod *ggzmod, const char *chat_msg);

/** @brief Get the player's win-loss record.
 *  @return TRUE if there is a record; FALSE if not or on error.
 */
int ggzmod_player_get_record(GGZMod *ggzmod, GGZSeat *seat,
			     int *wins, int *losses,
			     int *ties, int *forfeits);

/** @brief Get the player's rating.
 *  @return TRUE if there is a rating; FALSE if not or on error.
 */
int ggzmod_player_get_rating(GGZMod *ggzmod, GGZSeat *seat, int *rating);

/** @brief Get the player's ranking.
 *  @return TRUE if there is a ranking; FALSE if not or on error.
 */
int ggzmod_player_get_ranking(GGZMod *ggzmod, GGZSeat *seat, int *ranking);

/** @brief Get the player's highscore.
 *  @return TRUE if there is a highscore; FALSE if not or on error.
 */
int ggzmod_player_get_highscore(GGZMod *ggzmod, GGZSeat *seat, int *highscore);

/** @brief Get the spectator's win-loss record.
 *  @return TRUE if there is a record; FALSE if not or on error.
 */
int ggzmod_spectator_get_record(GGZMod *ggzmod, GGZSpectatorSeat *seat,
				int *wins, int *losses,
				int *ties, int *forfeits);

/** @brief Get the spectator's rating.
 *  @return TRUE if there is a rating; FALSE if not or on error.
 */
int ggzmod_spectator_get_rating(GGZMod *ggzmod, GGZSpectatorSeat *seat, int *rating);

/** @brief Get the spectator's ranking.
 *  @return TRUE if there is a ranking; FALSE if not or on error.
 */
int ggzmod_spectator_get_ranking(GGZMod *ggzmod, GGZSpectatorSeat *seat, int *ranking);

/** @brief Get the spectator's highscore.
 *  @return TRUE if there is a highscore; FALSE if not or on error.
 */
int ggzmod_spectator_get_highscore(GGZMod *ggzmod, GGZSpectatorSeat *seat, int *highscore);


/** @brief Request extended player information for one or more players
 *
 *  Depending on the seat parameter (-1 or valid number), this function
 *  asynchronously requests information about player(s), which will arrive
 *  with a GGZMOD_EVENT_INFO event.
 *  @param ggzmod The ggzmod object.
 *  @param seat_num The seat number to request info for, or -1 to select all.
 *  @return TRUE if seat is -1 or valid number, FALSE for non-player seats.
 */
int ggzmod_player_request_info(GGZMod *ggzmod, int seat_num);

/** @brief Get the extended information for the specified seat.
 *  @param ggzmod The GGZMod object.
 *  @param seat The seat number (0..(number of seats - 1)).
 *  @return A valid GGZPlayerInfo structure, if seat is valid and has info.
 */
GGZPlayerInfo* ggzmod_player_get_info(GGZMod *ggzmod, int seat);

#ifdef __cplusplus
}
#endif

#endif /* __GGZMOD_H__ */
