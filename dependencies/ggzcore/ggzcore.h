/*
 * File: ggzcore.h
 * Author: GGZ Development Team
 * Project: GGZ Core Client Lib
 * Date: 9/15/00
 * $Id$
 *
 * Interface file to be included by client frontends
 *
 * Copyright (C) 2000-2005 GGZ Development Team.
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


#ifndef __GGZCORE_H__
#define __GGZCORE_H__

#define GGZCORE_VERSION_MAJOR 0
#define GGZCORE_VERSION_MINOR 0
#define GGZCORE_VERSION_MICRO 14
#define GGZCORE_VERSION_IFACE "8:1:1"

#include <stdarg.h>
#include <sys/types.h>

#include <ggz_common.h>

/** @file ggzcore.h
 *  @brief The interface for the ggzcore library used by GGZ clients
 */

#ifdef __cplusplus
extern "C" {
#endif

/* Definitions and enumerations */
/* ---------------------------- */

/** @brief ggz_debug debugging type for configuration system. */
#define GGZCORE_DBG_CONF   "GGZCORE:CONF"

/** @brief ggz_debug debugging type for game communication. */
#define GGZCORE_DBG_GAME   "GGZCORE:GAME"

/** @brief ggz_debug debugging type for hook system. */
#define GGZCORE_DBG_HOOK   "GGZCORE:HOOK"

/** @brief ggz_debug debugging type for accessing modules. */
#define GGZCORE_DBG_MODULE "GGZCORE:MODULE"

/** @brief ggz_debug debugging type for network interaction. */
#define GGZCORE_DBG_NET    "GGZCORE:NET"

/** @brief ggz_debug debugging type for debugging while polling. */
#define GGZCORE_DBG_POLL   "GGZCORE:POLL"

/** @brief ggz_debug debugging type for room events and data. */
#define GGZCORE_DBG_ROOM   "GGZCORE:ROOM"

/** @brief ggz_debug debugging type for server events and data. */
#define GGZCORE_DBG_SERVER "GGZCORE:SERVER"

/** @brief ggz_debug debugging type for state changes. */
#define GGZCORE_DBG_STATE  "GGZCORE:STATE"

/** @brief ggz_debug debugging type for table data. */
#define GGZCORE_DBG_TABLE  "GGZCORE:TABLE"

/** @brief ggz_debug debugging type for XML processing. */
#define GGZCORE_DBG_XML    "GGZCORE:XML"

/* GGZCore library features */
typedef enum {
	GGZ_OPT_PARSER      = 0x0001, /* %0000 0000 */ /**< Load the default configuration file (unused). */
	GGZ_OPT_MODULES     = 0x0002, /* %0000 0010 */ /**< Load the game module list. */
	GGZ_OPT_THREADED_IO = 0x0004, /* %0000 0100 */ /**< Provide multi-threaded network IO (for asynchronous lookups). */
	GGZ_OPT_EMBEDDED    = 0x0008, /* %0000 1000 */ /**< Run game with integrated core client. */
	GGZ_OPT_RECONNECT   = 0x0010  /* %0001 0000 */ /**< Reconnect automatically after connection loss. */
} GGZOptionFlags;


/** Options structure for ggzcore library */
typedef struct _GGZOptions {
	
	/** Option flags */
	GGZOptionFlags flags;

} GGZOptions;


/** ggzcore_init() - Initializtion function for ggzcore lib.
 *
 * @param options options structure
 *
 * @return int : 0 if successful, -1 on failure
 */
int ggzcore_init(GGZOptions options);


/** ggzcore_reload() - Reload game module database
 */
void ggzcore_reload(void);

/** ggzcore_destroy() - Cleanup function for ggzcore lib.
 */
void ggzcore_destroy(void);


/* Definitions for all internal ggzcore structures. */

/** @brief A server object containing all information about a connection */
typedef struct _GGZServer   GGZServer;

/** @brief Contains information about a single room on a server. */
typedef struct _GGZRoom     GGZRoom;

/** @brief Contains information about a single player. */
typedef struct _GGZPlayer   GGZPlayer;

/** @brief Contains information about a single table. */
typedef struct _GGZTable    GGZTable;

/** @brief Contains information about a _game type_.
 *  @note Each room has one game type; a game may be used in multiple rooms.
 */
typedef struct _GGZGameType GGZGameType;

/** @brief Contains information about a single module.
 *  A game module, on the client, is an executable designed to play a game.
 *  Each game type may have many modules that play it.
 */
typedef struct _GGZModule   GGZModule;

/** @brief Contains information about a single game table.
 *  This contains information about a table we are present at or are about
 *  to launch.  It is thus associated with both a GGZTable and a GGZModule.
 */
typedef struct _GGZGame     GGZGame;

/** GGZ Hook function return types */
typedef enum {
	GGZ_HOOK_OK, /**< Success! */
	GGZ_HOOK_REMOVE, /**< Remove this hook immediately. */
	GGZ_HOOK_ERROR, /**< A localized error. */
	GGZ_HOOK_CRISIS /**< A major error; stop processing the event. */
} GGZHookReturn;

/** GGZ Event hook function type, used as a vallback for events */
typedef GGZHookReturn (*GGZHookFunc)(unsigned int id, 
				     const void* event_data, 
				     const void* user_data);

/** @brief GGZ object destroy function type
 *
 *  @todo This is not currently used.
 */
typedef void (*GGZDestroyFunc)(void* data);


/**
 * This controls the type of login a user chooses.  A different
 * value will require different information to be sent to
 * the server.
 */
typedef enum {
	/** Standard login; uname and correct passwd needed. */
	GGZ_LOGIN,
	
	/** Guest login; only a uname is required. */	
	GGZ_LOGIN_GUEST,
	
	/** New user login; only a uname is required.  Password will be
	 *  assigned by the server (but can be passed along). */
	GGZ_LOGIN_NEW
} GGZLoginType;


/** @brief The data describing an error.
 *
 *  When an error occurrs, a pointer to a struct of this type will be
 *  passed as the event data.
 */
typedef struct {
	/** @brief A default error message.
	 *  @note This also provides backward-compatability.
	 */
	char message[128];

	/** @brief The type of error that occurred.
	 *  @note Not all errors are possible with all events.
	 */
	GGZClientReqError status;
} GGZErrorEventData;

/** @brief The data associated with a GGZ_MOTD_LOADED server event. */
typedef struct {
	const char *motd; /**< MOTD text message */
	const char *url; /**< URL of a graphical MOTD webpage, or NULL */
} GGZMotdEventData;

/**
 * A GGZServerEvent is an event triggered by a communication from the
 * server.  Each time an event occurs, the associated event handler
 * will be called, and will be passed the event data (a void*).  Most events
 * are generated as a result of ggzcore_server_read_data.
 * @see ggzcore_server_add_event_hook
 */
typedef enum {
	/** We have just made a connection to the server.  After this point
	 *  the server's socket should be accessible and should be monitored
	 *  for data.  It happens in direct response to ggzcore_server_connect.
	 *  Note that most events after this will only happen by calling
	 *  ggzcore_server_read_data on the server's FD!
	 *  @param data NULL
	 *  @see ggzcore_server_connect */
	GGZ_CONNECTED,

	/** Error: we have failed to connect to the server.  This is
	 *  generated in place of GGZ_CONNECTED if the connection could
	 *  not be made.  The server object is otherwise unaffected.
	 *  @param data An error string (created by strerror)
	 *  @see ggzcore_server_connect */
	GGZ_CONNECT_FAIL,

	/** We have negotiated a connection to the server.  This will
	 *  happen automatically once a connection has been established,
	 *  if the server socket is monitored.
	 *  @note This just means we've determined ggzd is at the other end.
	 *  @param data NULL
	 *  @see ggzcore_server_read_data */
	GGZ_NEGOTIATED,

	/** Error: negotiation failure.  Could be the wrong version.  This
	 *  will happen in place of a GGZ_NEGOTIATED if the server could
	 *  not be negotiated with.
	 *  @param data A useless error string.
	 *  @see ggzcore_server_read_data */
	GGZ_NEGOTIATE_FAIL,

	/** We have successfully logged in.  We can now start doing stuff.
	 *  This will not happen until the client sends their login
	 *  information.
	 *  @see ggzcore_server_login
	 *  @param data NULL
	 *  @see ggzcore_server_read_data */
	GGZ_LOGGED_IN,

	/** Error: login failure.  This will happen in place of GGZ_LOGGED_IN
	 *  if the login failed.  The server object will be otherwise
	 *  unaffected.
	 *  @param data A pointer to a GGZErrorEventData.
	 *  @see GGZErrorEventData
	 *  @see ggzcore_server_read_data */
	GGZ_LOGIN_FAIL,

	/** The MOTD has been read from the server and can be displayed.
	 *  The server will send us the MOTD automatically after login; it
	 *  can also be requested by ggzcore_server_motd.  It is up to the
	 *  client whether or not to display it.  See the online
	 *  documentation (somewhere?) about the MOTD markup format.
	 *  @param data Pointer to a GGZMotdEventData including the full MOTD text.
	 *  @see ggzcore_server_motd
	 *  @todo The MOTD cannot be accessed outside of this event
	 *  @see ggzcore_server_read_data! */
	GGZ_MOTD_LOADED,

	/** The room list arrived.  This will only happen after the list is
	 *  requested by ggzcore_server_list_rooms().  The list may be
	 *  accessed through ggzcore_server_get_num_rooms() and
	 *  ggzcore_server_get_nth_room().  Until this event arrives these
	 *  functions will be useless!
	 *  @param data NULL 
	 *  @see ggzcore_server_read_data*/
	GGZ_ROOM_LIST,

	/** The list of game types is available.  This will only happen after
	 *  the list is requested by ggzcore_server_list_types().  The list
	 *  may be accessed through ggzcore_server_get_num_gametypes() and
	 *  ggzcore_server_get_nth_gametype().  Until this event arrives
	 *  these functions will be useless!
	 *  @param data NULL
	 *  @see ggzcore_server_read_data */
	GGZ_TYPE_LIST,

	/** The number of players on the server has changed.  This event is
	 *  issued rather frequently every time players enter or leave.
	 *  @param data NULL
	 *  @see ggzcore_server_get_num_players
	 *  @see ggzcore_server_read_data */
	GGZ_SERVER_PLAYERS_CHANGED,

	/** We have successfully entered a room.  This will be issued to
	 *  tell us a room join has succeeded, after it has been requested.
	 *  @param data NULL
	 *  @see ggzcore_server_join_room
	 *  @see ggzcore_server_read_data */
	GGZ_ENTERED,

	/** Error: we have tried to enter a room and failed.  This will be
	 *  issued to tell us a room join has failed.
	 *  @param data A pointer to a GGZErrorEventData.
	 *  @see GGZErrorEventData
	 *  @see ggzcore_server_join_room
	 *  @see ggzcore_server_read_data */
	GGZ_ENTER_FAIL,

	/** Logged out of the server.  This will happen when the server
	 *  completes the communication; usually after
	 *  ggzcore_net_send_logout is called.
	 *  @param data NULL
	 *  @see ggzcore_server_read_data */
	GGZ_LOGOUT,

	/** Error: a network (transmission) error occurred.  The server will
	 *  automatically disconnect.
	 *  @param data A generally unhelpful error string.
	 *  @see ggzcore_server_read_data */
	GGZ_NET_ERROR,

	/** Error: a communication protocol error occured.  This can happen
	 *  in a variety of situations when the server sends us something
	 *  we can't handle.  The server will be automatically disconnected.
	 *  @param data A technical error string.
	 *  @see ggzcore_server_read_data */
	GGZ_PROTOCOL_ERROR,

	/** Error: A chat message could not be sent.  This will happen when
	 *  we try to send a chat and the server rejects it.
	 *  @param data A pointer to a GGZErrorEventData.
	 *  @see GGZErrorEventData
	 *  @see ggzcore_server_read_data */
	GGZ_CHAT_FAIL,

	/** The internal state of ggzcore has changed.  This may happen at
	 *  any time.
	 *  @param data NULL
	 *  @see GGZStateID
	 *  @see ggzcore_server_get_state */
	GGZ_STATE_CHANGE,

	/** Status event: a requested direct game connection has been
	 *  established.  To start a game (table), a channel must be
	 *  created.  This event will alert that the channel has been
	 *  established.  The channel's FD should then be monitored for
	 *  input, which should then be passed back to the server object
	 *  for handling.
	 *  @param data NULL
	 *  @note This event is deprecated and should not be used.
	 *  @see ggzcore_server_get_channel
	 *  @see ggzcore_server_read_data */
	GGZ_CHANNEL_CONNECTED,

	/** Game channel is ready for read/write operations.  After the
	 *  channel has been connected, if we continue to monitor the socket
	 *  eventually it will be negotiated and ready to use.  At this point
	 *  it is ready for the game client to use.
	 *  @param data NULL
	 *  @note This event is deprecated and should not be used.
	 *  @see ggzcore_server_read_data */
	GGZ_CHANNEL_READY,

	/** Error: Failure during setup of direct connection to game server.
	 *  If the channel could not be prepared, this event will happen
	 *  instead of GGZ_CHANNEL_READY or GGZ_CHANNEL_CONNECTED event.  At
	 *  this point the channel is no longer useful (I think).
	 *  @param data An unhelpful error string
	 *  @note This event is deprecated and should not be used.
	 *  @see ggzcore_server_read_data */
	GGZ_CHANNEL_FAIL,

	/** The room configuration on the server changed. A room was either
	 *  added or removed, or scheduled for removing (closed).
	 *  @param data NULL
	 *  @see ggzcore_room_get_closed() */
	GGZ_SERVER_ROOMS_CHANGED,

	/** Terminator.  Do not use. */
	GGZ_NUM_SERVER_EVENTS
} GGZServerEvent;

/** @brief The data associated with a GGZ_CHAT_EVENT room event. */
typedef struct {
	GGZChatType type; /**< The type of chat. */
	const char *sender; /**< The person who sent the message, or NULL */
	const char *message; /**< The message itself (in UTF-8), or NULL */
} GGZChatEventData;

/** @brief The data associated with a GGZ_TABLE_LEFT room event. */
typedef struct {
	/** @brief This is the reason for why we left. */
	GGZLeaveType reason;

	/** If we were booted from the table (GGZ_LEAVE_BOOT), this is
	 *  the player who kicked us out. */
	const char *player;
} GGZTableLeaveEventData;

/** @brief The data associated with GGZ_ROOM_ENTER/GGZ_ROOM_LEAVE events. */
typedef struct {
	/** @brief The name of the player entering/leaving. */
	const char *player_name;

	/* If this value is false (0) then any NULL rooms (below) mean that
	   the room is actually not known.  This should only happen when
	   connecting to an older server. */
	int rooms_known;

	/** @brief The room we are entering.
	 *
	 *  This may be NULL if the player is leaving the server or if the
	 *  info is unknown. */
	GGZRoom *to_room;

	/** @brief The room the player is leaving.
	 *
	 *  This may be NULL if the player is just entering the server or
	 *  if the info is unknown. */
	GGZRoom *from_room;
} GGZRoomChangeEventData;

/** A GGZRoomEvent is an event associated with the room, that is triggered
 *  by a communication from the server.  When a room event occurs, the
 *  associated event handler will be called, and will be passed the event
 *  data (a void*), along with the (optional) user data.  All room events
 *  apply to the current room unless a room number is given.  Room events
 *  are almost always triggered by calling ggzcore_server_read_data.
 *  @see ggzcore_room_add_event_hook
 *  @see ggzcore_server_read_data
 */
typedef enum {
	/** The list of players in a room has arrived.
	 *  @param data The room (GGZRoom*)
	 *  @note This will only be issued for the current room.
	 *  @see ggzcore_room_list_players */
	GGZ_PLAYER_LIST,

	/** Received the list of active tables.
	 *  @param data NULL
	 *  @see ggzcore_room_list_tables */
	GGZ_TABLE_LIST,

	/** Received a chat message of any kind.  This can happen at any
	 *  time when you're in a room.
	 *  @param data The GGZChatEventData associated with the chat.
	 *  @see GGZChatEventData */
	GGZ_CHAT_EVENT,

	/** A player has entered the room with you.
	 *  @param data A GGZRoomChangeEventData structure. */
	GGZ_ROOM_ENTER,

	/** A player has left your room.
	 *  @param data A GGZRoomChangeEventData structure. */
	GGZ_ROOM_LEAVE,

	/** One of the tables in the current room has changed.
	 *  @todo How are you supposed to know which table has changed?
	 *  @param data NULL */
	GGZ_TABLE_UPDATE,

	/** The table you tried to launch has launched!
	 *  @see ggzcore_room_launch_table
	 *  @param data NULL */
	GGZ_TABLE_LAUNCHED,

	/** The table you tried to launch couldn't be launched
	 *  @see GGZ_TABLE_LAUNCHED
	 *  @param data A pointer to a GGZErrorEventData */
	GGZ_TABLE_LAUNCH_FAIL,

	/** Your table join attempt has succeeded.
	 *  @see ggzcore_room_join_table
	 *  @param data The table index (int*) of the table we joined. */
	GGZ_TABLE_JOINED,

	/** Joining a table did not succeed.
	 *  @see GGZ_TABLE_JOINED
	 *  @param data A helpful error string. */
	GGZ_TABLE_JOIN_FAIL,

	/** You have successfully left the table you were at.
	 *  @see ggzcore_room_leave_table
	 *  @param data The GGZTableLeaveEventData associated with the leave.
	 *  @see GGZTableLeaveEventData */
	GGZ_TABLE_LEFT,

	/** Your attempt to leave the table has failed.
	 *  @see GGZ_TABLE_LEFT
	 *  @param data A helpful error string. */
	GGZ_TABLE_LEAVE_FAIL,

	/** A player's lag (measure of connection speed) has been updated
	 *  @see ggzcore_player_get_lag
	 *  @param data The name of the player whose lag has changed. */
	GGZ_PLAYER_LAG,

	/** A player's stats have been updated.
	 *  @see GGZ_PLAYER_LIST
	 *  @see ggzcore_player_get_record
	 *  @see  ggzcore_player_get_rating
	 *  @see ggzcore_player_get_ranking
	 *  @see ggzcore_player_get_highscore
	 *  @param data The name of the player whose stats have changed. */
	GGZ_PLAYER_STATS,

	/** The number of players in a room has arrived.
	 *  @param data The room (GGZRoom*) */
	GGZ_PLAYER_COUNT
} GGZRoomEvent;


/** A GGZGameEvent is an event associated with the game, that is triggered
 *  by a communication from the server or from the game.  When a game
 *  event occurs, the associated event handle will be called, and will be
 *  passed the event data (a void*) along with the (optional) user data.
 *  All game events apply to the current game.  Game events are usually
 *  triggered by calling ggzcore_server_read_data or ggzcore_game_read_data.
 *  @see ggzcore_game_add_event_hook
 *  @see ggzcore_server_read_data
 */
typedef enum {
	/** A game was launched by the player (you).  After this the core
	 *  client should call ggzcore_game_get_control_fd, monitor the
	 *  socket that function returns, and call ggzcore_game_read_data
	 *  when there is data pending.  This event is triggered inside of
	 *  ggzcore_game_launch.
	 *  @param data NULL
	 *  @see ggzcore_game_launch */
	GGZ_GAME_LAUNCHED,

	/** Your game launch has failed.  Triggered instead of
	 *  GGZ_GAME_LAUNCHED when there's a failure somewhere.
	 *  @param data NULL
	 *  @see GGZ_GAME_LAUNCHED */
	GGZ_GAME_LAUNCH_FAIL,

	/** Negotiation with server was successful.  This should happen
	 *  some time after the launch succeeds.  The core client need do
	 *  nothing at this point.
	 *  @param data NULL */
	GGZ_GAME_NEGOTIATED,

	/** Negotiation was not successful, game launch failed.
	 *  @todo Currently this can't actually happen... */
	GGZ_GAME_NEGOTIATE_FAIL,

	/** Game reached the 'playing' state.  When this happens the core
	 *  client should call ggzcore_room_launch_table or
	 *  ggzcore_room_join_table to finalize the game join.
	 *  @param data NULL */
	GGZ_GAME_PLAYING
} GGZGameEvent;


/** @brief The states a server connection may be in.
 *
 *  On the client side, a simplistic state maching is used to tell what's
 *  going on.  A game client should usually consult the current state when
 *  determining what actions are possible.
 */
typedef enum {
	GGZ_STATE_OFFLINE, /**< Not connected (at all) */
	GGZ_STATE_CONNECTING, /**< In the process of connecting. */
	GGZ_STATE_RECONNECTING, /**< Continuous reconnection attempts. */
	GGZ_STATE_ONLINE, /**< Connected, but not doing anything. */
	GGZ_STATE_LOGGING_IN, /**< In the process of logging in. */
	GGZ_STATE_LOGGED_IN, /**< Online and logged in! */
	GGZ_STATE_ENTERING_ROOM, /**< Moving into a room. */ 
	GGZ_STATE_IN_ROOM, /**< Online, logged in, and in a room. */
	GGZ_STATE_BETWEEN_ROOMS, /**< Moving between rooms. */
	GGZ_STATE_LAUNCHING_TABLE, /**< Trying to launch a table. */
	GGZ_STATE_JOINING_TABLE, /**< Trying to join a table. */
	GGZ_STATE_AT_TABLE, /**< Online, loggied in, in a room, at a table. */
	GGZ_STATE_LEAVING_TABLE, /**< Waiting to leave a table. */
	GGZ_STATE_LOGGING_OUT /**< In the process of logging out. */
} GGZStateID;


/** @brief The environment a game frontend expects.
 *
 *  Core clients should offer those game modules which fit their
 *  own environment.
 */
typedef enum {
	GGZ_ENVIRONMENT_PASSIVE, /**< No GUI, no interaction with user */
	GGZ_ENVIRONMENT_CONSOLE, /**< Text console */
	GGZ_ENVIRONMENT_FRAMEBUFFER, /**< VESA or framebuffer */
	GGZ_ENVIRONMENT_XWINDOW, /**< X11 windowed mode (default) */
	GGZ_ENVIRONMENT_XFULLSCREEN /**< X11 fullscreen mode */
} GGZModuleEnvironment;

/* Server object related functions */
/* ------------------------------- */

/** @brief Create a new server object.
 *
 *  Call this function to create a server object.  This object holds
 *  all state data for communicating with a ggz server.  It is
 *  necessary for any kind of connection.
 */
GGZServer* ggzcore_server_new(void);

/** @brief Reset the server object.
 *
 *  After you've disconnected, call this function to discard all state
 *  data and reset the state of the server object.  You can then connect
 *  again.
 *  @note You should disconnect before resetting.
 */
int ggzcore_server_reset(GGZServer *server);

/*
 * Functions for attaching hooks to GGZServer events
 */

/** @brief Register a callback handler for a server event.
 *
 *  Call this function to register the given GGZHookFunc as a
 *  handler for the given event.  Then any time that event
 *  happens the handler function will be called.
 *  @param server The GGZ server object.
 *  @param event The server event to be handled.
 *  @param func The handler function to be called when the event occurs.
 *  @return A hook ID value to identify this handler.
 *  @note Equivalent to ggzcore_server_add_event_hook_full with data==NULL.
 *  @note More than one handler can be registered for each event.
 */
int ggzcore_server_add_event_hook(GGZServer *server,
				  const GGZServerEvent event, 
				  const GGZHookFunc func);

/** @brief Register a callback handler for a server event.
 *  @see ggzcore_server_add_event_hook
 *  @param data An arbitrary pointer that will be passed to the hook function.
 */					
int ggzcore_server_add_event_hook_full(GGZServer *server,
				       const GGZServerEvent event, 
				       const GGZHookFunc func,
				       const void *data);

/** @brief Remove a single hook function from an event's hook list.
 *
 *  @param server The GGZ server object.
 *  @param event The server event the hook is associated with.
 *  @param func The function to be removed from the hook list.
 *  @return 0 on success (hook removed); -1 on failure (no hook removed)
 *  @note At most one copy of the function will be removed.
 *  @see ggzcore_server_remove_event_hook_id
 */
int ggzcore_server_remove_event_hook(GGZServer *server,
				     const GGZServerEvent event, 
				     const GGZHookFunc func);

/** @brief Remove a hook function with given ID from the event's hook list.
 *
 *  @param server The GGZ server object.
 *  @param event The server event the hook is associated with.
 *  @param hook_id The ID of the hook event.
 *  @return 0 on success (hook removed); -1 on failure (no hook removed)
 *  @note The hook ID is given by ggzcore_server_add_event_hook
 */
int ggzcore_server_remove_event_hook_id(GGZServer *server,
					const GGZServerEvent event, 
					const unsigned int hook_id);

/*
 * Functions for setting GGZServer data
 */

/** @brief Set host info for connecting to the server.
 *
 *  Call this function to set host info for the GGZ server
 *  before trying to connect to it.
 *
 *  @param server The GGZ server object.
 *  @param host A string containing the hostname.
 *  @param port The port to connect to.
 *  @param use_tls If set, the connection will be encrypted.
 *  @return 0 on success, -1 on error.
 *  @note Should never fail when given valid input.
 *  @see ggzcore_server_connect
 */
int ggzcore_server_set_hostinfo(GGZServer *server,
				const char *host,
				const unsigned int port,
				const unsigned int use_tls);

/** @brief Set login info for logging in to the server.
 *
 *  Call this function to set login info for the GGZ server
 *  before trying to login.  This should only be called when the server is in
 *  certain states.
 *
 *  @param server The GGZ server object.
 *  @param type The type of login to attempt.
 *  @param handle The username to use with the server.
 *  @param password The password to use (may be NULL with some login types).
 *  @param email The email address to use (may be NULL with some login types).
 *  @return 0 on success, -1 on error.
 */
int ggzcore_server_set_logininfo(GGZServer *server,
				 const GGZLoginType type,
				 const char *handle,
				 const char *password,
				 const char *email);

/** @brief Initiate logging of ggzcore events
 *
 * Normally, ggzcore traffic is not logged anywhere. With this functions, such
 * output can be directed into a file. It contains all the network messages
 * received from the server.
 *
 * @param server The GGZ server object.
 * @param filename The file the messages are written to.
 * @return 0 on success, -1 on error.
 */
int ggzcore_server_log_session(GGZServer *server, const char *filename);


/* Functions for querying a GGZServer object for information */
/* --------------------------------------------------------- */

/** @brief Get the hostname of the server.
 *
 *  @param server The GGZ server object.
 *  @return A string containing the host name, or NULL on error.
 *  @see ggzcore_server_set_hostinfo
 */
const char* ggzcore_server_get_host(const GGZServer *server);

/** @brief Get the port of the server.
 *
 *  @param server The GGZ server object.
 *  @return The port number of the server, or -1 on error.
 *  @see ggzcore_server_set_hostinfo
 */
int ggzcore_server_get_port(const GGZServer *server);

/** @brief Get the login type being used for this server.
 *
 *  @param server The GGZ server object.
 *  @return The login type set for the server, or -1 on error.
 *  @see ggzcore_server_set_logininfo
 */
GGZLoginType ggzcore_server_get_type(const GGZServer *server);

/** @brief Get the handle being used for this server.
 *
 *  @param server The GGZ server object.
 *  @return A string containing the handle, or NULL on error.
 *  @see ggzcore_server_set_logininfo
 */
const char* ggzcore_server_get_handle(const GGZServer *server);

/** @brief Get the password being used for this server.
 *
 *  @param server The GGZ server object.
 *  @return A string containing the password, or NULL on error.
 *  @see ggzcore_server_set_logininfo
 */
const char* ggzcore_server_get_password(const GGZServer *server);

/** @brief Get the socket used for connection with the server.
 *
 *  This returns the file descriptor of the primary socket for
 *  the TCP connection to the server.  All GGZ data goes across
 *  this socket.
 *
 *  @param server The GGZ server object.
 *  @return The file descriptor of the connection socket.
 *  @see ggzcore_server_connect
 */
int ggzcore_server_get_fd(const GGZServer *server);

/** @brief Get the socket used for direct gane connections
 *
 *  This returns the file descriptor of the socket for
 *  the TCP game connection.  This will be handed off to a game module 
 *  when it is ready.
 *  Needed only for channels set up by ggzcore.
 *
 *  @param server The GGZ server object.
 *  @return The file descriptor of the connection socket.
 *  @see ggzcore_server_create_channel
 */
int ggzcore_server_get_channel(GGZServer *server);

/** @brief Get the state of the server connection.
 *
 *  @param server The GGZ server object.
 *  @return The state of the connection, or -1 on error.
 */
GGZStateID ggzcore_server_get_state(const GGZServer *server);

/** @brief Get the current TLS status of this server
 *
 *  @param server The GGZ server object
 *  @return Whether TLS is active or not
 */
int ggzcore_server_get_tls(const GGZServer *server);

/** @brief Get the total number of players on the server.
 *
 * @param server The GGZ server object
 * return An approximate number of players in all rooms of the server. */
int ggzcore_server_get_num_players(const GGZServer *server);

/** @brief Return the number of rooms on the server, or -1 on error.
 *  @note Until we retrieve the list of rooms, this will return 0.
 */
int ggzcore_server_get_num_rooms(const GGZServer *server);

/** @brief Return the nth room on the server, or NULL on error. */
GGZRoom* ggzcore_server_get_nth_room(const GGZServer *server, 
				     const unsigned int num);

/** @brief Return the number (position in the room list) of the room.
 *  @see ggzcore_server_get_nth_room */
int ggzcore_server_get_room_num(const GGZServer *server,
				const GGZRoom *room);

/** @brief Return the current room, or NULL if there is none. */
GGZRoom* ggzcore_server_get_cur_room(const GGZServer *server);

/** @brief Find the player, by name (or NULL).
 *  @note Only players in the current room can currently be found.
 *  @note This function is inefficient. */
GGZPlayer* ggzcore_server_get_player(GGZServer *server, const char *name);

/** @brief Return the overall number of game types on the server.
 *
 * @param server The GGZ server object.
 * @return The number of game types on this server, or -1 on error.
 * @note This number is 0 until @see GGZ_TYPE_LIST.
 */
int ggzcore_server_get_num_gametypes(const GGZServer *server);

/** @brief Get the nth gametype, or NULL on error. */
GGZGameType* ggzcore_server_get_nth_gametype(const GGZServer *server, 
					     const unsigned int num);

/** @brief Return the player's current game. */
GGZGame* ggzcore_server_get_cur_game(const GGZServer *server);

/** @brief Return TRUE iff the server is online (connected?) */
int ggzcore_server_is_online(const GGZServer *server);

/** @brief Return TRUE iff we are logged into the server. */
int ggzcore_server_is_logged_in(const GGZServer *server);

/** @brief Return TRUE iff we are in a room on the server. */
int ggzcore_server_is_in_room(const GGZServer *server);

/** @brief Return TRUE iff we are at a table on the server. */
int ggzcore_server_is_at_table(const GGZServer *server);

/* GGZ Server Actions */
/* ------------------ */

/** @brief Connect to the server.
 *
 *  Call this function to initially connect to a GGZ server.  Connection
 *  info is set using the ggzcore_server_set_hostinfo function.
 *
 *  The function is asynchronous and will return very quickly.  After
 *  the connection is (hopefully) established we will receive either
 *  a GGZ_CONNECTED or GGZ_CONNECT_FAIL server event.  If the
 *  connection succeeds, negotiations with the GGZ server will begin
 *  automatically.  Once this is complete, we will receive either a
 *  GGZ_NEGOTIATED or GGZ_NEGOTIATE_FAIL event.
 *
 *  @param server The GGZ server object.
 *  @return 0 on success, -1 on failure.
 *  @note On success a GGZ_CONNECTED event will be generated.
 *  @note On failure a GGZ_CONNECT_FAIL event may or may not be generated.
 */
int ggzcore_server_connect(GGZServer *server);

/** @brief Establish a direct connection.
 *
 *  Direct connections are requested for games. They are similar to
 *  connections, instead of that no login takes place, but a channel for
 *  arbitrary game data is created.
 *  Needed only for channels set up by ggzcore.
 *
 *  @param server The GGZ server object.
 *  @return 0 on success, -1 on failure.
 */
int ggzcore_server_create_channel(GGZServer *server);

/** @brief Log in to the server.
 *
 *  Call this function to log in to the server once a connection
 *  has been established.  Typically you must first connect to the
 *  server, then wait to receive the GGZ_CONNECTED and GGZ_NEGOTIATED
 *  events before attempting to log in.  Login info is set using the
 *  ggzcore_server_set_logininfo function.
 *
 *  The function is asynchronous and will return immediately.  After the
 *  login request is sent, we will wait to receive either a
 *  GGZ_LOGGED_IN or GGZ_LOGIN_FAIL server event.
 *
 *  @param server The GGZ server object.
 *  @return 0 on success, -1 on failure.
 *  @note On failure no events will be generated.
 */
int ggzcore_server_login(GGZServer *server);

/** @brief Request the MOTD from the server. */
int ggzcore_server_motd(GGZServer *server);

/** @brief Request room list.
 *
 * @param server The GGZ server object.
 * @param type Not used yet.
 * @param verbose Receive all information about a room or only the essentials.
 * @return 0 on success, -1 on failure.
 * @note A GGZ_ROOM_LIST might be generated thereafter.
 */
int ggzcore_server_list_rooms(GGZServer *server, const int type, const char verbose);

/** @brief Request game type list.
 *
 * @param server The GGZ server object.
 * @param verbose Receive detailed gametype information or not.
 * @return 0 on success, -1 on failure.
 * @note A GGZ_TYPE_LIST event will be the asynchronous response on success.
 */
int ggzcore_server_list_gametypes(GGZServer *server, const char verbose);

/** @brief Join a room on the server
 *
 * @param server The GGZ server object.
 * @param room The number of the room to join.
 * @return 0 on success, -1 on failure (e.g. non-existing room number).
 */
int ggzcore_server_join_room(GGZServer *server, GGZRoom *room);

/** @brief Log out of a server. */
int ggzcore_server_logout(GGZServer *server);

/** @brief Disconnect from a server after having logged out. */
int ggzcore_server_disconnect(GGZServer *server);


/* Functions for data processing */
/** @brief Check for data pending from the server socket.*/
int ggzcore_server_data_is_pending(GGZServer *server);

/** @brief Read data for the server on the specified FD.
 *  @return negative on error */
int ggzcore_server_read_data(GGZServer *server, int fd);

/** @brief Free GGZServer object and accompanying data */
void ggzcore_server_free(GGZServer *server);


/* Functions for manipulating GGZRoom objects */
/* ------------------------------------------ */

/** @brief Allocate space for a new room object */
GGZRoom* ggzcore_room_new(void);

/** @brief De-allocate room object and its children */
void ggzcore_room_free(GGZRoom *room);


/** @brief Return the server for this room (or NULL on error). */
GGZServer *ggzcore_room_get_server(const GGZRoom *room);

/** @brief Return the name of the room (or NULL on error). */
const char* ggzcore_room_get_name(const GGZRoom *room);

/** @brief Return the description of the room (or NULL on error). */
const char* ggzcore_room_get_desc(const GGZRoom *room);

/** @brief Return the type of game played in this room (or NULL on error). */
GGZGameType* ggzcore_room_get_gametype(const GGZRoom *room);

/** @brief Return the number of players in the room (or negative on error). */
int ggzcore_room_get_num_players(const GGZRoom *room);

/** @brief Return the nth player in the room (or NULL on error). */
GGZPlayer* ggzcore_room_get_nth_player(const GGZRoom *room,
				       const unsigned int num);

/** @brief Return the number of tables in the room (or negative on error). */
int ggzcore_room_get_num_tables(const GGZRoom *room);

/** @brief Return the nth table in the room (or NULL on error). */
GGZTable* ggzcore_room_get_nth_table(const GGZRoom *room,
				     const unsigned int num);

/** @brief Return the table in this room with matching ID (NULL on error). */
GGZTable* ggzcore_room_get_table_by_id(const GGZRoom *room,
				       const unsigned int id);

/** @brief Return whether this room is closed (1), or open as usual (0) */
int ggzcore_room_get_closed(const GGZRoom *room);


/** @brief Register a handler (hook) for the room event.
 *
 *  A room event will happen when data is received from the server.  To make
 *  updates to the frontend, the client will need to register a hook function
 *  to handle the event.  This hook function will be called each time the
 *  room event occurrs.  More than one hook function may be specified, in
 *  which case they will all be called (in FIFO order).
 *  @param room The room object to associate the hook with.
 *  @param event The event the handler is going to be "hooked" onto.
 *  @param func The event handler itself.  This is called during the event.
 *  @return The hook ID, or negative on error.
 *  @see ggzcore_room_add_event_hook_full
 *  @see ggzcore_room_remove_event_hook
 *  @see ggzcore_room_remove_event_hook_id
 */
int ggzcore_room_add_event_hook(GGZRoom *room,
				const GGZRoomEvent event, 
				const GGZHookFunc func);

/** @brief Register a handler (hook) for thee room event, with data.
 *
 *  This function is similar to ggzcore_room_add_event_hook, except that
 *  user data will be associated with the hook.  This data will be passed
 *  back to the function each time it is invoked on this event.
 *  @param room The room object to associate the hook with.
 *  @param event The event the handler is going to be "hooked" onto.
 *  @param func The event handler itself.  This is called during the event.
 *  @param data The user data associated with the hook.
 *  @return The hook ID, or negative on error.
 *  @see ggzcore_room_add_event_hook
 */
int ggzcore_room_add_event_hook_full(GGZRoom *room,
				     const GGZRoomEvent event, 
				     const GGZHookFunc func,
				     const void *data);

/** @brief Remove a hook from an event.
 *
 *  Removes a specific hook function from the hook list for the given room
 *  event.  If more than one copy of the function exists in the list, the
 *  oldest one will be removed.
 *  @param room The room object to associate the hook with.
 *  @param event The event the handler is to be unhooked from.
 *  @param func The event handler function to remove.
 *  @return 0 on success, negative on failure.
 *  @see ggzcore_room_add_event_hook
 */
int ggzcore_room_remove_event_hook(GGZRoom *room,
				   const GGZRoomEvent event, 
				   const GGZHookFunc func);

/** @brief Remove a hook from an event, by ID.
 *
 *  Removes a specific hook from the hook list for the given room.  The "ID"
 *  should be the same as that returned when the hook was added.
 *  @param room The room object to associate the hook with.
 *  @param event The event the handler is to be unhooked from.
 *  @param id The ID of the hook to remove, as returned by the add function
 *  return 0 on success, negative on failure
 *  @see ggzcore_room_add_event_hook
 */
int ggzcore_room_remove_event_hook_id(GGZRoom *room,
				      const GGZRoomEvent event, 
				      const unsigned int hook_id);


/** @brief Call to request a list of players in the room.
 *  @see GGZ_PLAYER_LIST */
int ggzcore_room_list_players(GGZRoom *room);

/** @brief Call to request a list of tables in the room.
 *  @param room Your current room
 *  @param type currently ignored (???)
 *  @param global currently ignored (???)
 *  @see GGZ_TABLE_LIST */
int ggzcore_room_list_tables(GGZRoom *room,
			     const int type,
			     const char global);

/** @brief Chat!
 *  @param room Your current room.
 *  @param opcode The chat type.
 *  @param player The name of the target player (only for certain chat types)
 *  @param msg The text of the chat message (some chat types don't need it)
 *  @return 0 on success, negative on (any) failure
 *  @note The chat message should be in UTF-8. */
int ggzcore_room_chat(GGZRoom *room,
		      const GGZChatType opcode,
		      const char *player,
		      const char *msg);

/** @brief Administrative actions.
 *  @param room Your current room.
 *  @param type Type of action (gag, ungag, kick, ...)
 *  @param player Name of the target player
 *  @param reason The reason for the action (only for kicking)
 *  @return 0 on success, negative on failure */
int ggzcore_room_admin(GGZRoom *room,
	               GGZAdminType type,
	               const char *player,
	               const char *reason);

/** @brief Launch a table in the room.
 *
 *  When a player wants to launch a new table, this is the function to do
 *  it.  You must first create the table and set up the number and type of
 *  seats.  Then call this function to initiate the launch.
 *  @param room Your current room.
 *  @param table The table to launch.
 *  @return 0 on success, negative on (any) failure */
int ggzcore_room_launch_table(GGZRoom *room, GGZTable *table);

/** @brief Join a table in the room, so that you can then play at it.
 *  @param room Your current room.
 *  @param table_id The table to join.
 *  @param spectator TRUE if you wish to spectate, FALSE if you want to play
 *  @return 0 on success, negative on (any) failure */
int ggzcore_room_join_table(GGZRoom *room, const unsigned int table_id, 
			    int spectator);

/** @brief Leave the table you are currently playing at.
 *
 *  This function tries to leave your current table.  You should "force" the
 *  leave only if the game client is inoperable, since for some games this
 *  will destroy the game server as well.
 *  @param room Your current room.
 *  @param force TRUE to force the leave, FALSE to leave it up to ggzd
 *  @return 0 on success, negative on (any) failure */
int ggzcore_room_leave_table(GGZRoom *room, int force);


/* Functions for manipulating GGZPlayer objects */
/* -------------------------------------------- */

/** @brief Return the name of the player. */
char *ggzcore_player_get_name(const GGZPlayer *player);

/** @brief Return the type of the player (admin/registered/guest) */
GGZPlayerType ggzcore_player_get_type(const GGZPlayer *player);

/** @brief Return the player's room, or NULL if none. */
GGZRoom *ggzcore_player_get_room(const GGZPlayer *player);

/** @brief Return the player's table, or NULL if none */
GGZTable* ggzcore_player_get_table(const GGZPlayer *player);

/** @brief Return the player's lag class (1..5) */
int ggzcore_player_get_lag(const GGZPlayer *player);

/** @brief Get the player's win-loss record.
 *  @return TRUE if there is a record; FALSE if not or on error.
 */
int ggzcore_player_get_record(const GGZPlayer *player,
			      int *wins, int *losses,
			      int *ties, int *forfeits);

/** @brief Get the player's rating.
 *  @return TRUE if there is a rating; FALSE if not or on error.
 */
int ggzcore_player_get_rating(const GGZPlayer *player, int *rating);

/** @brief Get the player's ranking.
 *  @return TRUE if there is a ranking; FALSE if not or on error.
 */
int ggzcore_player_get_ranking(const GGZPlayer *player, int *ranking);

/** @brief Get the player's highscore.
 *  @return TRUE if there is a highscore; FALSE if not or on error.
 */
int ggzcore_player_get_highscore(const GGZPlayer *player, int *highscore);


/** @brief Create a new table object.
 *  @note Useful when launching a game. */
GGZTable* ggzcore_table_new(void);

/** @brief Set data on a table object.
 *  @note Useful when launching a game. */
int ggzcore_table_init(GGZTable *table,
		       const GGZGameType *gametype,
		       const char *desc,
		       const unsigned int num_seats);

/** @brief Free the table object. */
void ggzcore_table_free(GGZTable *table);

/** @brief Set a seat type at a table, pre-launch.
 *
 *  When launching a table, call this function to set up a particular
 *  seat at the table.  It can also be used to fiddle with already
 *  existing tables, but that would be extremely unwise.
 *
 *  @param table The table object to change.
 *  @param seat The seat number at the table to change.
 *  @param type The type of seat to make it (open, reserved, or bot).
 *  @param name The name of the seat (must be valid for reserved seats).
 *  @return 0 on success, -1 on error.
 *  @todo How do we stop the GGZ client from fiddling with random tables?
 */
int ggzcore_table_set_seat(GGZTable *table,
			   const unsigned int seat,
			   GGZSeatType type,
			   const char *name);

/** @brief Find and remove the player from the table. */
int ggzcore_table_remove_player(GGZTable *table, const char *name);

/** @brief Return the ID of the table. */
int ggzcore_table_get_id(const GGZTable *table);

/** @brief Return the room this table is in. */
const GGZRoom *ggzcore_table_get_room(const GGZTable *table);

/** @brief Return the game type of the table. */
const GGZGameType *ggzcore_table_get_type(const GGZTable *table);

/** @brief Return the table's description (or NULL). */
const char *ggzcore_table_get_desc(const GGZTable *table);

/** @brief Return the state of the table. */
GGZTableState ggzcore_table_get_state(const GGZTable *table);

/** @brief Return the number of seats at the table. */
int ggzcore_table_get_num_seats(const GGZTable *table);

/** @brief Set the table description. */
int ggzcore_table_set_desc(GGZTable *table, const char *desc);

/** @brief Count the seats of the given type.
 *
 *  Given a table and a seat type, this function returns the number of
 *  seats at the table that match the type.
 *
 *  @param table A GGZ table.
 *  @param type A GGZSeatType.
 *  @return The number of seats matching the type, or -1 on error.
 */
int ggzcore_table_get_seat_count(const GGZTable *table, GGZSeatType type);

/** @brief Return the name of a player at the table, or NULL on error. */
const char *ggzcore_table_get_nth_player_name(const GGZTable *table,
					      const unsigned int num);

/** @brief Return the number of spectator seats at the table, or -1. */
int ggzcore_table_get_num_spectator_seats(const GGZTable *table);

/** @brief Return the name of the nth spectator, or NULL if seat is empty. */
const char *ggzcore_table_get_nth_spectator_name(const GGZTable *table,
						 const unsigned int num);

/** @brief Return the type of a player at the table, or GGZ_PLAYER_NONE on
 *  error. */
GGZSeatType ggzcore_table_get_nth_player_type(const GGZTable *table, 
					      const unsigned int num);


/** @brief Get the ID of this gametype.
 *  @note This is not normally useful for a GGZ client to know. */
unsigned int ggzcore_gametype_get_id(const GGZGameType *type);

/** @brief Get the name of the game type. */
const char * ggzcore_gametype_get_name(const GGZGameType *type);

/** @brief Get the protocol "engine" used by the game type. */
const char* ggzcore_gametype_get_prot_engine(const GGZGameType *type);

/** @brief Get the version of the protocol the game uses. */
const char* ggzcore_gametype_get_prot_version(const GGZGameType *type);

/** @brief Get the version of the game itself. */
const char* ggzcore_gametype_get_version(const GGZGameType *type);

/** @brief Get the author of the game. */
const char* ggzcore_gametype_get_author(const GGZGameType *type);

/** @brief Get a URL for more info about the game. */
const char* ggzcore_gametype_get_url(const GGZGameType *type);

/** @brief Get a description of the game. */
const char* ggzcore_gametype_get_desc(const GGZGameType *type);

/** @brief Get the maximum number of players the game can support.
 *  @see ggzcore_gametype_num_players_is_valid */
int ggzcore_gametype_get_max_players(const GGZGameType *type);

/** @brief Get the maximum number of bots the game can support.
 *  @see ggzcore_gametype_bots_is_valid */
int ggzcore_gametype_get_max_bots(const GGZGameType *type);

/** @brief Return TRUE iff spectators are allowed for this game type. */
int ggzcore_gametype_get_spectators_allowed(const GGZGameType *type);

/** @brief Return TRUE iff the given number of players is valid. */
int ggzcore_gametype_num_players_is_valid(const GGZGameType *type,
					  unsigned int num);

/** @brief Return TRUE iff the given number of bots is valid. */
int ggzcore_gametype_num_bots_is_valid(const GGZGameType *type,
				       unsigned int num);

/** @brief Return the number of named bots for this gametype. */
int ggzcore_gametype_get_num_namedbots(const GGZGameType *type);
const char* ggzcore_gametype_get_namedbot_name(const GGZGameType *type, unsigned int num);
const char* ggzcore_gametype_get_namedbot_class(const GGZGameType *type, unsigned int num);

/** @brief Return TRUE iff this game may disclose the player's hostname. */
int ggzcore_gametype_get_peers_allowed(const GGZGameType *type);


/* Group of configuration functions */
/* -------------------------------- */

/** ggzcore_conf_initialize()
 *	Opens the global and/or user configuration files for the frontend.
 *	Either g_path or u_path can be NULL if the file is not to be used.
 *	The user config file will be created if it does not exist.
 *
 *	@return: 0 on success, negative on failure
 */
int ggzcore_conf_initialize	(const char	*g_path,
				 const char	*u_path);

/** ggzcore_conf_write_string() - Write a string to the user config file
 *
 * @param section	section to store value in
 * @param key		key to store value under
 * @param value		value to store
 *
 * @return int : 0 if successful, -1 on error
 */
int ggzcore_conf_write_string(const char *section, 
			      const char *key, 
			      const char *value);

/** ggzcore_conf_write_int() - Write a integer to the user config file
 *
 * @param section	section to store value in
 * @param key		key to store value under
 * @param value		value to store
 *
 * @return int : 0 if successful, -1 on error
 */
int ggzcore_conf_write_int(const char *section, 
			   const char *key, 
			   int value);

/** ggzcore_conf_write_list() - Write a list to the user config file
 *
 * @param section	section to store value in
 * @param key		key to store value under
 * @param argc		count of string arguments in list
 * @param argv		array of NULL terminated strings
 *
 * @return int : 0 if successful, -1 on error
 */
int ggzcore_conf_write_list(const char *section, 
			    const char *key, 
			    int argc, 
			    char **argv);

/** ggzcore_conf_read_string() - Read a string from the configuration file(s)
 *
 * @param section	section to get value from
 * @param key		key value was stored under
 * @param def		default value to return if none is found
 *
 * @return
 *   a dynamically allocated string from the configuration file
 * OR
 *   a dynamically allocated copy of the default string
 *
 * @note The default may be set to NULL, in which case a NULL will be
 * returned if the value could not be found in either configuration file.
 */
char * ggzcore_conf_read_string(const char *section, 
				const char *key, 
				const char *def);

/** ggzcore_conf_read_int() - Read a integer from the configuration file(s)
 *
 * @param section	section to get value from
 * @param key		key value was stored under
 * @param def		default value to return if none is found
 *
 * @return
 *   an integer from the configuration file
 * OR
 *   the default value
 *
 * @note There is no guaranteed way to find if the call failed.  If you
 * must know, call ggzcore_conf_read_string with a NULL default value and
 * check for the NULL return.
 */
int ggzcore_conf_read_int(const char *section, const char *key, int def);

/** ggzcore_conf_read_list() - Read a list from the configuration file(s)
 *
 * @param section	section to get value from
 * @param key		key value was stored under
 * @param argcp		ptr to int which will receive the list entry count
 * @param argvp		a pointer to a dynamically allocated array
 * 			that ggzcore_conf_read_list() will build
 *
 * @return int : 0 if successful, -1 on error
 */
int ggzcore_conf_read_list(const char *section, 
			   const char *key, 
			   int *argcp, 
			   char ***argvp);

/** ggzcore_conf_remove_section() - Removes a section from the user config file
 *
 * @param section	section to remove
 *
 * @return
 * int : 0 if successful, -1 on error, 1 on soft error (section did not exist)
 */
int ggzcore_conf_remove_section(const char *section);

/** ggzcore_conf_remove_key() - Removes a key entry from the user config file
 *
 * @param section	section to remove
 * @param key		key entry to remove
 *
 * @return
 * int : 0 if success, -1 on error, 1 on soft error (section/key didn't exist)
 */
int ggzcore_conf_remove_key(const char *section, const char *key);

/** ggzcore_conf_commit() - Commits the core user config file to disk
 *
 * @return:
 * int : 0 if successful, -1 on error
 */
int ggzcore_conf_commit(void);


/* Game module related functions */
/* ----------------------------- */

/** @brief This returns the number of registered modules */
unsigned int ggzcore_module_get_num(void);


/** This adds a local module to the list.  It returns 0 if successful or
    -1 on failure. */
int ggzcore_module_add(const char *name,
	               const char *version,
	               const char *prot_engine,
	               const char *prot_version,
                       const char *author,
		       const char *frontend,
		       const char *url,
		       const char *exe_path,
		       const char *icon_path,
		       const char *help_path,
			   GGZModuleEnvironment environment);		       


/** @brief Returns how many modules support this game and protocol */
int ggzcore_module_get_num_by_type(const char *game, 
				   const char *engine,
				   const char *version);

/** @brief Returns n-th module that supports this game and protocol */
GGZModule* ggzcore_module_get_nth_by_type(const char *game, 
					  const char *engine,
					  const char *version,
					  const unsigned int num);

/** @brief Return the name of the module. */
const char * ggzcore_module_get_name(GGZModule *module);

/** @brief Return the (game?) version of the module. */
const char* ggzcore_module_get_version(GGZModule *module);

/** @brief Return the name of the module's protocol engine. */
const char* ggzcore_module_get_prot_engine(GGZModule *module);

/** @brief Return the version of the module's protocol engine. */
const char* ggzcore_module_get_prot_version(GGZModule *module);

/** @brief Return the author of the module. */
const char* ggzcore_module_get_author(GGZModule *module);

/** @brief Return the module's frontend type. */
const char* ggzcore_module_get_frontend(GGZModule *module);

/** @brief Return the URL associated with the module. */
const char* ggzcore_module_get_url(GGZModule *module);

/** This is (intended to be) an optional xpm file that the module can provide
 *  to use for representing the game graphically. */
const char* ggzcore_module_get_icon_path(GGZModule *module);

/** @brief Return the help path of the module (?). */
const char* ggzcore_module_get_help_path(GGZModule *module);

/** @brief Return the executable arguments for the module.  See exec(). */
char** ggzcore_module_get_argv(GGZModule *module);

/** @brief Return the preferred environment type. */
GGZModuleEnvironment ggzcore_module_get_environment(GGZModule *module);

/* Functions related to game clients */
/* --------------------------------- */

/** @brief Make a new game object */
GGZGame* ggzcore_game_new(void);

/** @brief Initialize the game object. */
int ggzcore_game_init(GGZGame *game, GGZServer *server, GGZModule *module);

/** @brief Free the game object. */
void ggzcore_game_free(GGZGame *game);

/** @brief Register a hook for a game event.
 *  @see ggzcore_server_add_event_hook
 *  @see ggzcore_room_add_event_hook
 */
int ggzcore_game_add_event_hook(GGZGame *game,
				const GGZGameEvent event, 
				const GGZHookFunc func);

/** @brief Register a hook for a game event.
 *  @see ggzcore_server_add_event_hook_full
 *  @see ggzcore_room_add_event_hook_full
 */
int ggzcore_game_add_event_hook_full(GGZGame *game,
				     const GGZGameEvent event, 
				     const GGZHookFunc func,
				     const void *data);

/** @brief Remove a hook from a game event.
 *  @see ggzcore_server_remove_event_hook
 *  @see ggzcore_room_remove_event_hook
 */
int ggzcore_game_remove_event_hook(GGZGame *game,
				   const GGZGameEvent event, 
				   const GGZHookFunc func);

/** @brief Remove a specified hook from a game event.
 *  @see ggzcore_server_remove_event_hook_id
 *  @see ggzcore_room_remove_event_hook_id
 */
int ggzcore_game_remove_event_hook_id(GGZGame *game,
				      const GGZGameEvent event, 
				      const unsigned int hook_id);

/** @brief Return the control (ggzmod) socket for the game. */
int  ggzcore_game_get_control_fd(GGZGame *game);

/** @brief Return the game's server socket. Needed only for channels set up by ggzcore. */
void ggzcore_game_set_server_fd(GGZGame *game, unsigned int fd);

/** @brief Return the module set for the game. */
GGZModule* ggzcore_game_get_module(GGZGame *game);

/** @brief Launch thee game! */
int ggzcore_game_launch(GGZGame *game);

/** @brief Read data from the game.
 *  When data is pending on the control socket, call this function.
 *  @return negative on error
 *  @see ggzcore_game_get_control_fd */
int ggzcore_game_read_data(GGZGame *game);

#ifdef __cplusplus
}
#endif 

#endif  /* __GGZCORE_H__ */
