/* (C) 2003/2004 Sjoerd Langkemper
 * gataxx.h -
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * For more details see the file COPYING.
 */

#ifndef GATAXX_H
#define GATAXX_H

#include <glib.h>
#include <libgnomeui/libgnomeui.h>
#include <gconf/gconf-client.h>
#include <gtkgridboard.h>

#define BHEIGHT 7		/* board height */
#define BWIDTH 7		/* board width  */
#define DEF_TIMEOUT 1000 	/* computer timeout default */

typedef struct {
	int x;
	int y;
	int valid;
} position;

typedef struct {
	position from;
	position to;
} move;

void new_game_cb(GtkWidget * widget, gpointer data);
void undo_move_cb(GtkWidget * widget, gpointer data);
void quit_game_cb(GtkWidget * widget, gpointer data);
void properties_cb(GtkWidget * widget, gpointer data);
void about_cb(GtkWidget * widget, gpointer data);
void boxclicked_cb(GtkWidget * widget, int x, int y);
GnomeUIInfo * get_mainmenu(void);
char * get_gconf_uri(const char * item);
GConfClient * get_gconf_client(void);
char * get_tileset_path(char * tileset);
gboolean computer_move_cb(gpointer turn);
void do_move(move m);
void do_select(int x, int y);
void gridboard_move(GtkWidget * gridboard, move m);
void turn_pieces(GtkWidget * gridboard, int x, int y);
void apply_changes(void);
GConfClient * get_gconf_client(void);
void menu_undo_set_sensitive(gboolean sens);
gboolean move_possible(GtkWidget * gridboard, int turn);
gboolean move_possible_to(GtkWidget * gridboard, int x, int y, int turn);
gboolean end_game_cb(gpointer data);
void flip_final(GtkWidget * gridboard, int wc, int bc);

#endif /* GATAXX_H */
