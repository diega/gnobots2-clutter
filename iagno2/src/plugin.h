/*
 * Iagno II: An extensible Reversi game for GNOME
 * Copyright (C) 1999-2000 Ian Peters <itp@gnu.org>
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
 */

#ifndef _IAGNO2_PLUGIN_H_
#define _IAGNO2_PLUGIN_H_

#include <gnome.h>
#include <gmodule.h>

#include "../reversi-iagno2/reversi.h"

#define PLUGIN_VERSION "2"

typedef struct {
	GModule *module;

  /* Prepare to play for a side */
	void (*plugin_init_player)(gchar player);

  /* Done playing for a side */
  void (*plugin_deinit_player)(gchar player);

  /* Perform whatever initialization your plugin needs, ie init a random number
   * generator, set up a network connection... */
  void (*plugin_setup)(gchar player);
  
  /* Find a move on the given ReversiBoard for the given player.  Iagno2 has
   * already checked that you have a move, so you /must return a move (index
   * 0-63). */
	gint (*plugin_move)(ReversiBoard *board, gchar player);

  /* Return the string that is the name of your plugin.  This will show up in
   * the option menu in the preferences dialog. */
	const gchar *(*plugin_name)();

  /* The message you want your plugin to display in the status window while
   * deciding on a move to make. */
  const gchar *(*plugin_busy_message)(gchar player);

  /* Pops up a configuration dialog (this should be modal) to configure your
   * plugin.  The plugin /must/ save the configuration options entered to disk
   * (using gnome_config, for example), as it is not guaranteed that this is the
   * same copy of the plugin that will be loaded in a given game.  This does not
   * have to be defined. */
	void (*plugin_preferences)(GtkWidget *parent, gchar player);

  /* An about window (see the random plugin for an example), does not have to be
   * defined. */
  void (*plugin_about_window)(GtkWidget *parent);

} Iagno2Plugin;

Iagno2Plugin *iagno2_plugin_open (const gchar *plugin_file);

void iagno2_plugin_close (Iagno2Plugin *plugin);

#endif
