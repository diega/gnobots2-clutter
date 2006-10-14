/*
 * File: xmlelement.c
 * Author: Brent Hendricks
 * Project: GGZ Core Client Lib
 * Date: 8/31/01
 *
 * This is the code for handling the an XML element
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

#include "memory.h"
#include "support.h"

static void ggz_xmlelement_do_free(GGZXMLElement *element);


GGZXMLElement *ggz_xmlelement_new(const char *tag, const char * const *attrs,
	void (*process)(void*, GGZXMLElement*), void (*free)(GGZXMLElement*))
{
	GGZXMLElement *element;

	element = ggz_malloc(sizeof(GGZXMLElement));
	
	ggz_xmlelement_init(element, tag, attrs, process, free);

	return element;
}


void ggz_xmlelement_init(GGZXMLElement *element, const char *tag,
	const char * const *attrs,
	void (*process)(void*, GGZXMLElement*), void (*free)(GGZXMLElement*))
{
	int i;

	if (element) {
		element->tag = ggz_strdup(tag);
		element->attributes = ggz_list_create(NULL, 
						      ggz_list_create_str,
						      ggz_list_destroy_str,
						      GGZ_LIST_ALLOW_DUPS);
		element->text = NULL;
		element->process = process;
		
		for (i = 0; attrs[i]; i++)
			ggz_list_insert(element->attributes, (void *)attrs[i]);

		element->free = (free ? free : ggz_xmlelement_do_free);
	}
}


void ggz_xmlelement_set_data(GGZXMLElement *element, void *data)
{
	if (element)
		element->data = data;
}


const char* ggz_xmlelement_get_tag(GGZXMLElement *element)
{
	return (element ? element->tag : NULL);
}


const char* ggz_xmlelement_get_attr(GGZXMLElement *element, const char *attr)
{
	GGZListEntry *item;
	char *data;
	char *value = NULL;

	item = ggz_list_head(element->attributes);
	while (item) {
		/* FIXME: we should use the list searching features */
		data = ggz_list_get_data(item);
		if (ggz_strcasecmp(data, attr) == 0) {
			value = ggz_list_get_data(ggz_list_next(item));
			break;
		}
			
		/* Every other item is a value */
		item = ggz_list_next(ggz_list_next(item));
	}

	return value;
}


void* ggz_xmlelement_get_data(GGZXMLElement *element)
{
	return (element ? element->data : NULL);
}


char* ggz_xmlelement_get_text(GGZXMLElement *element)
{
	return (element ? element->text : NULL);
}


void ggz_xmlelement_add_text(GGZXMLElement *element, const char *text, int len)
{
	int old_len = 0;
	int new_len = 0;

	if (element) {

		/* Allocate space for text if we haven't already */
		if (!element->text) {
			new_len = len + 1;
			element->text = ggz_malloc(new_len * sizeof(char));
			element->text[0] = '\0';
		}
		else {
			old_len = strlen(element->text);
			new_len = old_len + len + 1;
			element->text = ggz_realloc(element->text, new_len);
		}
		
		strncat(element->text, text, len);
		element->text[new_len - 1] = '\0';
	}
}


void ggz_xmlelement_free(GGZXMLElement *element)
{
	if (element) {
		if (element->tag)
			ggz_free(element->tag);
		if (element->text)
			ggz_free(element->text);
		if (element->attributes)
			ggz_list_free(element->attributes);
		if (element->free)
			element->free(element);
	}
}


static void ggz_xmlelement_do_free(GGZXMLElement *element)
{
	if (element) 
		ggz_free(element);
}
