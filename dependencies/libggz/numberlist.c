/*
 * File: numberlist.c
 * Author: GGZ Dev Team
 * Project: GGZ Common Library
 * Date: 01/13/2002
 * $Id$
 *
 * This provides GGZ-specific functionality that is common to
 * some or all of the ggz-server, game-server, ggz-client, and
 * game-client.
 *
 * Copyright (C) 2002 GGZ Development Team.
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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ggz.h"
#include "ggz_common.h"

#include "support.h"

GGZNumberList ggz_numberlist_new(void)
{
	GGZNumberList list;
	list.values = 0;
	list.min = -1;
	list.max = -1;
	return list;
}

GGZNumberList ggz_numberlist_read(const char *text)
{
	GGZNumberList list = ggz_numberlist_new();
	char copy[strlen(text) + 1];
	char *next, *this = copy;
	int error = 0;

	if (!text) return list;

	strcpy(copy, text);

	do {
		char *str;

		while (isspace(*this)) this++;
		if (*this == '\0') break;

		next = this + 1;
		while (*next != '\0' && !isspace(*next)) next++;
		if (*next == '\0')
			next = NULL;
		else
			*next = '\0';

		str = strstr(this, "..");
		if (str) {
			int val1, val2;

			*str = '\0';
			str += 2;

			val1 = atoi(this);
			val2 = atoi(str);

			if (val1 < 0 || val2 < 0 || val1 >= val2)
				error = 1;
			else if (list.max >= 0)
				error = 1;
			else {
				list.min = val1;
				list.max = val2;
			}
		} else {
			int val = atoi(this);
		  
			if (val <= 0 || val > 32)
				error = 1;
			else if (list.values & (1 << (val - 1)))
				error = 1;
			else
				list.values |= (1 << (val - 1));
		}

		this = next + 1;
	} while (next);

	if (error)
		ggz_error_msg("Error reading number list \"%s\".", text);

	return list;
}

char *ggz_numberlist_write(GGZNumberList *list)
{
	/* The theoretical maximum string length is less than this. */ 
	char str[256] = "";
	int i;

	for (i = 1; i < 32; i++) {
		if (list->values & (1 << (i - 1))) {
			char this[10];
			snprintf(this, sizeof(this), "%d ", i);
			strcat(str, this);
		}
	}

	if (list->min > 0) {
		char this[32];
		if (list->max < 0) {
			ggz_error_msg("Invalid range %d/%d in number list.",
				      list->min, list->max);
			list->max = list->min;
		}

		snprintf(this, sizeof(this), "%d..%d", list->min, list->max);
		strcat(str, this);
	} else {
		/* Remove trailing space. */
		str[strlen(str) - 1] = '\0';
	}

	return ggz_strdup(str);
}

int ggz_numberlist_isset(const GGZNumberList *list, int value)
{
	if (value <= 0)
  		return 0;

	if (list->min > 0
	    && value >= list->min && value <= list->max)
		return 1;

	if (value > 32)
		return 0;

	return !!(list->values & (1 << (value - 1)));
}

int ggz_numberlist_get_max(const GGZNumberList *list)
{
	int min = list->max, i;

	if (min <= 0)
		min = 0;

	/* FIXME: come up with a cool bit maniuplation to do this */
	for (i = 32; i > min; i--)
		if (list->values & (1 << (i - 1)))
			return i;

	return min;
}
