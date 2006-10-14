/*
 * File: ggz_common.h
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

#ifndef __GGZ_COMMON_H__
#define __GGZ_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @brief Seat status values.
 *
 * Each "seat" at a table of a GGZ game can have one of these values.
 * They are used by the GGZ client, GGZ server, and game servers; their
 * use in game clients is completely optional.
 */
typedef enum {
	GGZ_SEAT_NONE,		/**< This seat does not exist. */
	GGZ_SEAT_OPEN,		/**< The seat is open (unoccupied). */
	GGZ_SEAT_BOT,		/**< The seat has a bot (AI) in it. */
	GGZ_SEAT_PLAYER,	/**< The seat has a regular player in it. */
	GGZ_SEAT_RESERVED,	/**< The seat is reserved for a player. */
	GGZ_SEAT_ABANDONED	/**< The seat is abandoned by a player. */
} GGZSeatType;


/** @brief Table state values.
 *
 * GGZ tables take one of the following states. They are used by the
 * GGZ client and GGZ server.  This is *not* necessarily the same as
 * the game state kept by libggzdmod
 */
typedef enum {
	GGZ_TABLE_ERROR   = -1,  /**< There is some error with the table */
	GGZ_TABLE_CREATED = 0,   /**< Initial created state for the table. */
	GGZ_TABLE_WAITING = 1,   /**< Waiting for enough players to join before playing */
	GGZ_TABLE_PLAYING = 2,   /**< Playing a game */
	GGZ_TABLE_DONE    = 3    /**< The game session is finished and the table will soon exit */
} GGZTableState;


/** @brief Chat types.
 *
 *  Each time we send a chat to the server, it will have one of these
 *  types associated with it.
 *  @see ggzcore_room_chat
 */
typedef enum {
	/** An unknown/unrecognized chat type - likely either a future chat
	 *  op or a communications error.  It can either be ignored or
	 *  handled generically. */
	GGZ_CHAT_UNKNOWN,

	/** A normal chat, just a message sent to the current room. */
	GGZ_CHAT_NORMAL,
	
	/** An announcement, usually triggered with /wall.  Only
	 *  admins can do this, and it is announced in all rooms. */
	GGZ_CHAT_ANNOUNCE,
	
	/** A beep.  We beep a player, and their client will run
	 *  the beep. */
	GGZ_CHAT_BEEP,
	
	/** A personal message to another player.  It consists of both
	 *  a message and a target player. */
	GGZ_CHAT_PERSONAL,

	/** A table-wide chat.  It goes only to players at the current
	 *  table. */
	GGZ_CHAT_TABLE
} GGZChatType;

/** @brief A leave type.
 *
 *  When a player leaves a table, this is a possible reason for the leave.
 */
typedef enum {
	/** A normal leave - at user request. */
	GGZ_LEAVE_NORMAL,

	/** The player has been booted from the table. */
	GGZ_LEAVE_BOOT,

	/** The game is over; the server exited normally. */
	GGZ_LEAVE_GAMEOVER,

	/** There was an error in the game server and it was terminated. */
	GGZ_LEAVE_GAMEERROR
} GGZLeaveType;

/** @brief Administrative actions.
 *
 * Administrators or to some extent hosts are able to dissolve player
 * problems by several levels of punishment.
 */
typedef enum {
	GGZ_ADMIN_GAG   = 0,   /**< Player chat is ignored by all others */
	GGZ_ADMIN_UNGAG = 1,   /**< Reversion of temporary gagging */
	GGZ_ADMIN_KICK  = 2,   /**< Player is kicked from the server */
	GGZ_ADMIN_BAN   = 3,   /**< Player is banned permanently (NOT USED YET) */
	GGZ_ADMIN_UNKNOWN = 4  /**< Invalid admin type */
} GGZAdminType;

/** @brief A player type.
 *
 *  The server will tell the client the type of each player.
 */
typedef enum {
	/** A normal player is registered but has no special permission. */
	GGZ_PLAYER_NORMAL,

	/** A guest player is not registered. */
	GGZ_PLAYER_GUEST,

	/* An admin player is registered and has all special permissions. */
	GGZ_PLAYER_ADMIN,

	/* A host player is registered and has a few special permissions. */
	GGZ_PLAYER_HOST,

	/* A bot is a special type of player. */
	GGZ_PLAYER_BOT,

	/* This is an unknown type of player. */
	GGZ_PLAYER_UNKNOWN
} GGZPlayerType;

/** @brief Get a string identifier for the GGZSeatType.
 *
 *  This returns a pointer to a static string describing the given seat type.
 *  It is useful for text-based communications protocols and debugging
 *  output.
 *  @param type The GGZSeatType, which determines the string returned.
 *  @note This is the inverse of ggz_string_to_seattype.
 */
const char *ggz_seattype_to_string(GGZSeatType type);

/** @brief Get a GGZSeatType for the given string identifier.
 *
 *  This returns a GGZSeatType that is associated with the given string
 *  description.
 *  @param type_str A string describing a GGZSeatType.
 *  @note If the type_str cannot be parsed GGZ_SEAT_NONE may be returned.
 *  @note This is the inverse of ggz_seattype_to_string.
 */
GGZSeatType ggz_string_to_seattype(const char *type_str);


/** @brief Get a string identifier for the GGZChatType.
 *
 *  This returns a pointer to a static string describing the given chat
 *  opcode.  It is useful for text-based communications protocols and
 *  debugging output.
 *  @param op The GGZChatType, which determines the string returned.
 *  @note This is the inverse of ggz_string_to_chattype.
 */
const char *ggz_chattype_to_string(GGZChatType type);

/** @brief Get a GGZChatType for the given string identifier.
 *
 *  This returns a GGZChatType that is associated with the given string
 *  description.
 *  @param type_str A string describing a GGZChatType.
 *  @note If the op_str cannot be parsed GGZ_CHAT_NORMAL will be returned.
 *  @note This is the inverse of ggz_chattype_to_string.
 */
GGZChatType ggz_string_to_chattype(const char *type_str);

/** @brief Get a GGZLeaveType for the given string identifier.
 *
 *  This returns a pointer to a static string describing the given chat
 *  opcode.  It is useful for text-based communications protocols and
 *  debugging output.
 *  @param op The GGZLeaveType, which determines thee string returned.
 *  @note This is the inverse of ggz_string_to_leavetype.
 */
const char *ggz_leavetype_to_string(GGZLeaveType type);

/** @brief Get a GGZLeaveType for the given string identifier.
 *
 *  This returns a GGZLeaveType that is associated with the given string
 *  description.
 *  @param type_str A string describing a GGZLeaveType.
 *  @note If the op_str cannot be parsed GGZ_LEAVE_GAMEERROR will be returned.
 *  @note This is the inverse of ggz_leavetype_to_string.
 */
GGZLeaveType ggz_string_to_leavetype(const char *type_str);

/** @brief Get a GGZPlayerType for the given string identifier.
 *
 *  This returns a pointer to a static string describing the given player
 *  type.  It is useful for text-based communications protocols and
 *  debugging output.
 *  @param type the GGZPlayerType, which determines the string returned.
 *  @note This is the inverse of ggz_string_to_playertype. */
const char *ggz_playertype_to_string(GGZPlayerType type);

/** @brief Get a GGZPlayerType for the given string identifier.
 *
 *  This returns a GGZPlayerType that is associated with the given string
 *  description.
 *  @param type_str A string describing a GGZPlayerType.
 *  @note If the type_str cannot be parsed GGZ_PLAYER_GUEST will be returned.
 *  @note This is the inverse of ggz_playertype_to_string.
 */
GGZPlayerType ggz_string_to_playertype(const char *type_str);

/** @brief Get a string identifier for the GGZAdminType.
 *
 *  This returns a pointer to a static string describing the given admin
 *  action.  It is useful for text-based communications protocols and
 *  debugging output.
 *  @param op The GGZAdminType, which determines the string returned.
 *  @note This is the inverse of ggz_string_to_admintype.
 */
const char *ggz_admintype_to_string(GGZAdminType type);

/** @brief Get a GGZAdminType for the given string identifier.
 *
 *  This returns a GGZAdminType that is associated with the given string
 *  description.
 *  @param type_str A string describing a GGZAdminType.
 *  @note This is the inverse of ggz_admintype_to_string.
 */
GGZAdminType ggz_string_to_admintype(const char *type_str);


/** @brief Convert a string to a boolean.
 * 
 *  The string should contain "true" or "false".
 *  @param str The string in question.
 *  @param dflt The default, if the string is unreadible or NULL.
 *  @return The boolean value.
 */
int str_to_bool(const char *str, int dflt);

/** @brief Convert a boolean value to a string.
 *
 *  @param bool_value A boolean value.
 *  @return "true" or "false", as appropriate.
 */
char *bool_to_str(int bool_val);

/**
 * @defgroup numberlist Number lists
 *
 * These functions provide a method for storing and retrieving a simple list
 * of positive integers.  The list must follow a very restrictive form:
 * each value within [1..32] may be included explicitly in the list.  Higher
 * values may only be included as the part of a single given range [x..y].
 */

/** @brief The number list type. */
typedef struct {
	int values;
	int min, max;
} GGZNumberList;

/** @brief Return an empty number list. */
GGZNumberList ggz_numberlist_new(void);

/** @brief Read a number list from a text string.
 *
 *  The string is comprised of a list of values (in the range 1..32)
 *  separated by spaces, followed by an optional range (separated by "..").
 *  Examples: "2 3 4", "2..4", "1..1000", "2, 3, 10 15-50"
 */
GGZNumberList ggz_numberlist_read(const char* text);

/** @brief Write a number list to a ggz-malloc'd text string. */
char *ggz_numberlist_write(GGZNumberList *list);

/** @brief Check to see if the given value is set in the number list. */
int ggz_numberlist_isset(const GGZNumberList *list, int value);

/** @brief Return the largest value in the set. */
int ggz_numberlist_get_max(const GGZNumberList *list);

/** }@ */


/*
 * GGZ Protocols - used by the GGZ server and client.
 * These should perhaps go in ggz_protocols.h
 */

/* Changing anything below may require bumping up the protocol version.
 * Experiment at your own risk. */

/* Error opcodes. */
typedef enum {
	E_NO_STATUS	    = 1, /* internal placeholder; a statusless event */
	E_OK		    = 0, /* No error */
	E_USR_LOOKUP	    = -1,
	E_BAD_OPTIONS	    = -2,
	E_ROOM_FULL	    = -3,
	E_TABLE_FULL	    = -4,
	E_TABLE_EMPTY	    = -5,
	E_LAUNCH_FAIL	    = -6,
	E_JOIN_FAIL	    = -7,
	E_NO_TABLE	    = -8,
	E_LEAVE_FAIL	    = -9,
	E_LEAVE_FORBIDDEN   = -10,
	E_ALREADY_LOGGED_IN = -11,
	E_NOT_LOGGED_IN	    = -12,
	E_NOT_IN_ROOM	    = -13,
	E_AT_TABLE	    = -14,
	E_IN_TRANSIT	    = -15,
	E_NO_PERMISSION	    = -16,
	E_BAD_XML	    = -17,
	E_SEAT_ASSIGN_FAIL  = -18,
	E_NO_CHANNEL        = -19,
	E_TOO_LONG          = -20,
	E_UNKNOWN           = -21,
	E_BAD_USERNAME      = -22,
	E_USR_TYPE          = -23,
	E_USR_FOUND         = -24,
	E_USR_TAKEN         = -25
} GGZClientReqError;

/** @brief Get a string identifier for the GGZClientReqError.
 *
 *  This returns a pointer to a static string describing the given status.
 *  It is useful for text-based communications protocols and debugging
 *  output.
 *  @param err The GGZClientReqError, which determines the string returned.
 *  @note This is the inverse of ggz_string_to_error.
 */
const char *ggz_error_to_string(GGZClientReqError err);

/** @brief Get a GGZClientReqError for the given string identifier.
 *
 *  This returns a GGZClientLeaveType that is associated with the given string
 *  description.
 *  @param err_str A string describing a GGZClientReqEerror.
 *  @note If the err_str cannot be parsed E_UNKNOWN will be returned.
 *  @note This is the inverse of ggz_error_to_string.
 */
GGZClientReqError ggz_string_to_error(const char *err_str);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GGZ_COMMON_H__ */
