/* (C) 2003/2004 Sjoerd Langkemper
 * properties.h - 
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 * For more details see the file COPYING.
 */

#ifndef _PROPERTIES_H_
#define _PROPERTIES_H_

#include <glib.h>

void load_properties (void);
void reload_properties (void);
void show_properties_dialog (void);
int props_get_level(int piece);
int props_get_white_level(void);
int props_get_black_level(void);
void load_properties(void);
void show_properties_dialog(void);
void reload_properties(void);
gboolean props_get_flip_final(void);
gboolean props_get_animate(void);
gboolean props_get_quick_moves(void);
gchar * props_get_tile_set(void);
int props_is_human(int piece);
void props_init(GtkWindow * window, char * title);

typedef struct {
	gint black_level;
	gint white_level;
	gboolean flip_final;
	gboolean animate;
	gboolean quick_moves;
	gchar * tile_set;
} PropertiesData;

#endif
