/* (C) 2003/2004 Sjoerd Langkemper
 * menus.c - drop down menus
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

#include "gataxx.h"
#include <glib.h>
#include <gnome.h>

static GnomeUIInfo game_menu[] = {
  GNOMEUIINFO_MENU_NEW_GAME_ITEM(new_game_cb, NULL),
  GNOMEUIINFO_SEPARATOR,
  GNOMEUIINFO_MENU_UNDO_MOVE_ITEM(undo_move_cb, NULL),
  GNOMEUIINFO_SEPARATOR,
  GNOMEUIINFO_MENU_QUIT_ITEM(quit_game_cb, NULL),
  GNOMEUIINFO_END
};

static GnomeUIInfo settings_menu[] = {
  GNOMEUIINFO_MENU_PREFERENCES_ITEM (properties_cb, NULL),
  GNOMEUIINFO_END
};

static GnomeUIInfo help_menu[] = {
  GNOMEUIINFO_HELP("gataxx"),
  GNOMEUIINFO_MENU_ABOUT_ITEM(about_cb, NULL),
  GNOMEUIINFO_END
};

GnomeUIInfo mainmenu[] = {
  GNOMEUIINFO_MENU_GAME_TREE(game_menu),
  GNOMEUIINFO_MENU_SETTINGS_TREE(settings_menu),
  GNOMEUIINFO_MENU_HELP_TREE(help_menu),
  GNOMEUIINFO_END
};

void menu_undo_set_sensitive(gboolean sens) {
	GnomeUIInfo * gamemenu=mainmenu[0].moreinfo;
	gtk_widget_set_sensitive(gamemenu[2].widget, sens);
}
	
