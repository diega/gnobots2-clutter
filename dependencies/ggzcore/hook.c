/*
 * File: hook.c
 * Author: Brent Hendricks
 * Project: GGZ Core Client Lib
 * Date: 11/01/00
 * $Id$
 *
 * This is the code for handling hook functions
 *
 * Many of the ideas for this interface come from the hook mechanism in GLib
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
#  include <config.h>	/* Site-specific config */
#endif

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#include <ggz.h>

#include "ggzcore.h"
#include "hook.h"

typedef struct _GGZHook GGZHook;

/* _GGZHook : data type for representing single hook in a list */
struct _GGZHook {

	/* Callback ID (unique within this event) */
	unsigned int id;

	/* Actual callback function */
	GGZHookFunc func;

	/* Pointer to user data */
	const void *user_data;

	/* Pointer to next GGZHook */
	GGZHook *next;
};


/* GGZHookList : list of hook functions */
struct _GGZHookList {

	/* Object ID unique to this hooklist: gets passed to callbacks */
	unsigned int id;

	/* Sequence number for callbacks */
	int seq_id;

	/* Linked list of hooks */
	GGZHook *hooks;

};


/* Static functions */
static void _ggzcore_hook_remove_actual(GGZHookList * list,
					GGZHook * hook, GGZHook * prev);

/* No publicly Exported functions */


/* Internal library functions (prototypes in hook.h) */


/* _ggzcore_hook_list_init() - Allocate room for and initialize a hook
 *                             list structure 
 *
 * Receives:
 * unsigned int id       : ID code of event for this hook list
 *                       : to be passed to all hook functions
 * Returns:
 * GGZHookList* : pointer to new hooklist structure
 */
GGZHookList *_ggzcore_hook_list_init(const unsigned int id)
{
	GGZHookList *hooks;

	hooks = ggz_malloc(sizeof(*hooks));
	hooks->id = id;

	return hooks;
}


/* _ggzcore_hook_add() - Add hook function to list
 *
 * Receives:
 * GGZHookList* list : Hooklist to which we are adding function
 * GGZHookFunc func  : Callback function
 *
 * Returns:
 * int : id for this callback 
 */
int _ggzcore_hook_add(GGZHookList * list, const GGZHookFunc func)
{
	return _ggzcore_hook_add_full(list, func, NULL);
}


/* _ggzcore_hook_add_full() - Add hook function to list, specifying
 *                            all parameters 
 * 
 * Receives:
 * GGZHookList* list     : Hooklist to which we are adding function
 * GGZHookFunc func      : Callback function
 * void* user_data       : "User" data to pass to callback
 * GGZDestroyFunc destroy: function to call to free user data
 *
 * Returns:
 * int : id for this callback 
 */
int _ggzcore_hook_add_full(GGZHookList * list,
			   const GGZHookFunc func, const void *user_data)
{
	GGZHook *hook, *cur, *next;

	hook = ggz_malloc(sizeof(*hook));

	/* Assign unique ID */
	hook->id = list->seq_id++;
	hook->func = func;
	hook->user_data = user_data;

	/* Append onto list of callbacks */
	if ((next = list->hooks) == NULL)
		list->hooks = hook;
	else {
		while (next) {
			cur = next;
			next = cur->next;
		}
		cur->next = hook;
	}

	return hook->id;
}


/* _ggzcore_hook_remove_all() - Remove all hooks from a list
 *
 * Receives:
 * GGZHookList* list : Hooklist from which to remove hooks
 */
void _ggzcore_hook_remove_all(GGZHookList * list)
{
	GGZHook *cur, *next;

	next = list->hooks;
	while (next) {
		cur = next;
		next = cur->next;
		ggz_free(cur);
	}
	list->hooks = NULL;
}


/* _ggzcore_hook_remove_id() - Remove specific hook from a list
 *
 * Receives:
 * GGZHookList* list : Hooklist from which to remove hooks
 * unsigned int id   : ID of hook to remove
 *
 * Returns:
 * int : 0 if successful, -1 on error
 */
int _ggzcore_hook_remove_id(GGZHookList * list, const unsigned int id)
{
	int status = -1;
	GGZHook *cur, *prev = NULL;

	cur = list->hooks;
	while (cur && cur->id != id) {
		prev = cur;
		cur = cur->next;
	}

	if (cur) {
		_ggzcore_hook_remove_actual(list, cur, prev);
		status = 0;
	}

	return status;
}


/* _ggzcore_hook_remove() - Remove specific hook from a list
 *
 * Receives:
 * GGZHookList* list : Hooklist from which to remove hoook
 * GGZHookFunc func  : pointer to hook function 
 *
 * Returns:
 * int : 0 if successful, -1 on error
 */
int _ggzcore_hook_remove(GGZHookList * list, const GGZHookFunc func)
{
	int status = -1;
	GGZHook *cur, *prev = NULL;

	cur = list->hooks;
	while (cur && (cur->func != func)) {
		prev = cur;
		cur = cur->next;
	}

	if (cur) {
		_ggzcore_hook_remove_actual(list, cur, prev);
		status = 0;
	}

	return status;
}


/* _ggzcore_hook_list_invoke() - Invoke list of hook functions
 *
 * Receives:
 * GGZHookList *list : list of hooks to invoke
 * void *data        : commin data passed to all hook functions in list
 *
 * Returns:
 * GGZ_HOOK_OK     if all hooks execute properly (even if some are removed)
 * GGZ_HOOK_ERROR  if at least one hook returned an error message
 * GGZ_HOOK_CRISIS if a hook terminated the sequence with a crisis
 */
GGZHookReturn _ggzcore_hook_list_invoke(GGZHookList * list,
					const void *event_data)
{
	GGZHookReturn status, retval = GGZ_HOOK_OK;
	GGZHook *cur, *next, *prev = NULL;

	if (!list) {
		/* This should never happen ?! */
		return GGZ_HOOK_CRISIS;
	}

	cur = list->hooks;
	while (cur != NULL) {
		next = cur->next;
		status =
		    (cur->func) (list->id, event_data, cur->user_data);

		if (status == GGZ_HOOK_ERROR)
			retval = GGZ_HOOK_ERROR;
		else if (status == GGZ_HOOK_REMOVE) {
			/* Remove this hook, then back up one so when we
			   continue we go straight to the next hook.  This
			   isn't an error so the return value isn't
			   changed. */
			_ggzcore_hook_remove_actual(list, cur, prev);
			cur = prev;
		} else if (status == GGZ_HOOK_CRISIS) {
			retval = GGZ_HOOK_CRISIS;
			break;
		}

		prev = cur;
		cur = next;
	}

	return retval;
}


/* _ggzcore_hook_list_destroy() - Deallocate and destroy hook list
 *
 * Receives:
 * GGZHookList *list : hooklist to destroy
 */
void _ggzcore_hook_list_destroy(GGZHookList * list)
{
	_ggzcore_hook_remove_all(list);
	ggz_free(list);
}


/* _ggzcore_hook_list_dump() - Dump list of hooks to screen for debugging
 *
 * Receives:
 * GGZHookList *list : hooklist to destroy
 */
void _ggzcore_hook_list_dump(GGZHookList * list)
{
	GGZHook *cur;

	for (cur = list->hooks; cur != NULL; cur = cur->next)
		ggz_debug(GGZCORE_DBG_HOOK, "  Hook id %d", cur->id);
}


/* Static functions internal to this file */

/* Remove a particular hook node (prev should be NULL for first node) */
static void _ggzcore_hook_remove_actual(GGZHookList * list,
					GGZHook * hook, GGZHook * prev)
{
	/* Special case if it was first in the list */
	if (!prev)
		list->hooks = hook->next;
	else
		prev->next = hook->next;

	ggz_free(hook);
}
