/*
 * File: stack.c
 * Author: Brent Hendricks
 * Project: GGZ Core Client Lib
 * Date: 8/31/01
 *
 * This is the code for handling stacks
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

#include "ggz.h"

#include "support.h"

GGZStack* ggz_stack_new(void)
{
	return ggz_list_create(NULL, NULL, NULL, GGZ_LIST_ALLOW_DUPS);
}


void ggz_stack_push(GGZStack *stack, void *item)
{
	ggz_list_insert(stack, item);
}


void* ggz_stack_pop(GGZStack *stack)
{
	GGZListEntry *entry;
	void *head;
	
	/* The stacks 'head' is actually the lists tail */
	entry = ggz_list_tail(stack);

	if (entry) {
		head = ggz_list_get_data(entry);
		ggz_list_delete_entry(stack, entry);
	}
	else
		head = NULL;

	return head;
}


void* ggz_stack_top(GGZStack *stack)
{
	GGZListEntry *entry;

	/* The stacks 'head' is actually the lists tail */
	entry = ggz_list_tail(stack);

	if (entry)
		return ggz_list_get_data(entry);
	else
		return NULL;
}


void ggz_stack_free(GGZStack *stack)
{
	ggz_list_free(stack);
}

