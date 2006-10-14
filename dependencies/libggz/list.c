/*
 * File: lists.c
 * Author: Rich Gade
 * Project: GGZ Core Client Lib
 * Date: 9/19/00
 *
 * This is the code for handling list functions for ggzcore
 *
 * Copyright (C) 2000,2001 Brent Hendricks.
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

#include <stdlib.h>
#include <string.h>

#include "ggz.h"

#include "support.h"

/* ggz_list_create()
 *	Creates a new list structure, and associates a comparison function
 *	(for sort/searches), create function (for user data structures),
 *	and destroy function with it.  Options can be found in lists.h
 *
 *	Returns a pointer to the list data structure
 */
GGZList* ggz_list_create(ggzEntryCompare compare_func,
			 ggzEntryCreate create_func,
			 ggzEntryDestroy destroy_func,
			 int options)
{
	GGZList *new;

	if(compare_func == NULL && !(options & GGZ_LIST_ALLOW_DUPS)) {
		/* debug warn - "Cannot replace dups w/o a compare func" */
	}

	new = ggz_malloc(sizeof(GGZList));
	new->head = NULL;
	new->tail = NULL;
	new->compare_func = compare_func;
	new->create_func = create_func;
	new->destroy_func = destroy_func;
	new->options = options;
	new->entries = 0;

	return new;
}


/* ggz_list_compare_str()
 *	A sample comparison function, this compares simple strings
 *
 *	Returns <0 if a<b,
 *		=0 if a=b,
 *		>0 if a>b
 */
int ggz_list_compare_str(void *a, void *b)
{
	return ggz_strcmp(a, b);
}


/* ggz_list_create_str()
 *	A sample data create function, this makes a copy of a string
 *
 *	Returns a pointer to the allocated data entry
 */
void * ggz_list_create_str(void *data)
{
	return ggz_strdup(data);
}


/* ggz_list_destroy_str()
 *	A sample data destroy function, this destroys strings created by
 *	ggz_list_create_str()
 */
void ggz_list_destroy_str(void *data)
{
	if(!data) {
		/* debug FATAL - "NULL passed to ggz_list_destroy_str" */
		return;
	}

	ggz_free(data);
}


/* ggz_list_insert()
 *	Inserts an entry into a list, copying the data entry if an entry
 *	creation function is known and placing the data in-order if a
 *	comparison function is registered.
 *
 *	Returns 0 normally
 *		or 1 if an entry was replaced
 *		or -1 on error
 */
int ggz_list_insert(GGZList *list, void *data)
{
	GGZListEntry *new, *p, *q;
	int compare=-1;
	int rc=0;

	if(!list) {
		/* debug warn - "NULL list passed to ggz_list_insert" */
		return -1;
	}
	if(!data) {
		/* debug warn - "NULL data passed to ggz_list_insert" */
		return -1;
	}

	/* Setup the new entry */
	new = ggz_malloc(sizeof(GGZListEntry));
	if(!new) {
		/* debug FATAL - "malloc() failure in ggz_list_insert" */
		return -1;
	}
	new->prev = NULL;
	new->next = NULL;
	if(list->create_func)
		new->data = (*list->create_func) (data);
	else
		new->data = data;

	/* Find where to stuff it */
	if(list->compare_func)
	{
		/* Search for the first slot that has higher-order data */
		p = list->head;
		q = NULL;
		while(p) {
			compare = (*list->compare_func) (p->data, data);
			if(compare >= 0)
				break;
			q = p;
			p = p->next;
		}
		/* p should point AFTER the insertion point, q before */
		if(compare == 0
		   && !(list->options & GGZ_LIST_ALLOW_DUPS)) {
			/* The matching case, replace existing entry in place */
			if(q)
				q->next = new;
			else
				list->head = new;
			new->next = p->next;
			new->prev = q;
			if(p->next)
				p->next->prev = new;
			else
				list->tail = new;
			if(list->destroy_func)
				(*list->destroy_func) (p->data);
			ggz_free(p);
			rc = 1;
		} else {
			/* Slightly easier, just insert a new value */
			if(q)
				q->next = new;
			else
				list->head = new;
			new->next = p;
			new->prev = q;
			if(p)
				p->prev = new;
			else
				list->tail = new;
		}
	} else {
		/* Pretty easy if not doing in-order list */
		p = list->tail;
		if(p)
			p->next = new;
		new->next = NULL;
		new->prev = p;
		list->tail = new;
		if(!list->head)
			list->head = new;
	}

	list->entries++;

	return rc;
}


/* ggz_list_head/tail/next/prev()
 * ggz_list_get_data()
 * ggz_list_count()
 *	These functions simply return a pointer from a list or entry
 *	They aren't strictly necessary, but should make the user code
 *	a bit easier to read and maintain.
 */
GGZListEntry *ggz_list_head(GGZList *list)
{
	if(!list) {
		/* debug warn - "NULL list passed to ggz_list_head" */
		return NULL;
	}
	return list->head;
}

GGZListEntry *ggz_list_tail(GGZList *list)
{
	if(!list) {
		/* debug warn - "NULL list passed to ggz_list_tail" */
		return NULL;
	}
	return list->tail;
}

GGZListEntry *ggz_list_next(GGZListEntry *entry)
{
	if(!entry) {
		/* debug warn - "NULL entry passed to ggz_list_next" */
		return NULL;
	}
	return entry->next;
}

GGZListEntry *ggz_list_prev(GGZListEntry *entry)
{
	if(!entry) {
		/* debug warn - "NULL entry passed to ggz_list_prev" */
		return NULL;
	}
	return entry->prev;
}

void * ggz_list_get_data(GGZListEntry *entry)
{
	if(!entry) {
		/* debug warn - "NULL entry passed to ggz_list_get_data" */
		return NULL;
	}
	return entry->data;
}


int ggz_list_count(GGZList *list)
{
	return list->entries;
}


/* ggz_list_search()
 *	This function searches the list for a value using the
 *	user-supplied comparison function.  It will terminate early
 *	if it passes where the value should be in the list.
 *
 *	Returns pointer to list entry which contains value
 *		or NULL on failure
 */
GGZListEntry * ggz_list_search(GGZList *list, void *data)
{
	GGZListEntry *p;
	int result;

	if(!list) {
		/* debug warn - "NULL list passed to ggz_list_search" */
		return NULL;
	}
	if(!data) {
		/* debug warn - "NULL data passed to ggz_list_search" */
		return NULL;
	}

	if(list->compare_func == NULL) {
		/* debug FATAL - "Cannot search list w/o a compare func" */
		return NULL;
	}

	p = list->head;
	while(p) {
		result = (*list->compare_func) (p->data, data);
		if(result == 0)
			return p;
		if(result > 0)
			return NULL;
		p = p->next;
	}

	return NULL;
}


/* ggz_list_search_alt()
 *	This function searches the list for a value using the search
 *	function specified by compare_func.  Note that the search cannot
 *	terminate early unless it finds the search target as we cannot
 *	assume that the list is in order for this comparison function.
 *
 *	Returns pointer to list entry which contains value
 *		or NULL on failure
 */
GGZListEntry * ggz_list_search_alt(GGZList *list, void *data,
					      ggzEntryCompare compare_func)
{
	GGZListEntry *p;

	if(!list) {
		/* debug warn - "NULL list passed to ggz_list_search_alt"*/
		return NULL;
	}
	if(!data) {
		/* debug warn - "NULL data passed to ggz_list_search_alt"*/
		return NULL;
	}

	if(compare_func == NULL) {
		/* debug FATAL - "Cannot search list w/o a compare func" */
		return NULL;
	}

	p = list->head;
	while(p) {
		if((compare_func) (p->data, data) == 0)
			return p;
		p = p->next;
	}

	return NULL;
}



/* ggz_list_delete_entry()
 *	Removes an entry from a list, calling a destructor if registered
 */
void ggz_list_delete_entry(GGZList *list, GGZListEntry *entry)
{
	if(!list) {
		/* debug warn - "NULL list passed to ggz_list_delete_entry" */
		return;
	}
	if(!entry) {
		/* debug warn - "NULL entry passed to ggz_list_delete_entry" */
		return;
	}

	if(entry->prev) {
		entry->prev->next = entry->next;
		if(entry->next)
			entry->next->prev = entry->prev;
		else
			list->tail = entry->prev;
	} else {
		list->head = entry->next;
		if(entry->next)
			entry->next->prev = NULL;
		else
			list->tail = NULL;
	}

	if(list->destroy_func)
		(*list->destroy_func) (entry->data);

	ggz_free(entry);

	list->entries--;
}


/* ggz_list_free()
 *	Eliminates all list entries and deallocates the list structure
 */
void ggz_list_free(GGZList *list)
{
	GGZListEntry *p, *q;

	if(!list) {
		/* debug warn - "NULL list passed to ggz_list_destroy" */
		return;
	}

	p = list->head;
	while(p) {
		q = p->next;
		if(list->destroy_func)
			(*list->destroy_func) (p->data);
		ggz_free(p);
		p = q;
	}

	ggz_free(list);
}
