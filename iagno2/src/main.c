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

#include <gnome.h>
#include <gmodule.h>

#include <signal.h>

#include "defines.h"
#include "iagno2.h"
#include "main.h"
#include "properties.h"
#include "preferences.h"
#include "../reversi-iagno2/reversi.h"
#include "plugin.h"

GtkWidget *app;
GtkWidget *new_game_dialog = NULL;
Iagno2Properties *properties;

Iagno2Plugin *plugin;

gboolean game_in_progress = FALSE;

ReversiBoard *board;
extern gchar *board_pixmaps;

extern Iagno2Plugin *players[2];

extern gchar whose_turn;

extern gint computer_timeout_id;
extern gint game_over_flip_id;
extern gint computer_thread;

extern GdkColor colors[2];

extern GdkPixmap *buffer_pixmap;

extern GnomeAppBar *appbar;

static GnomeUIInfo
game_menu[] = {
  GNOMEUIINFO_MENU_NEW_GAME_ITEM (new_game_cb, NULL),
  GNOMEUIINFO_SEPARATOR,
  GNOMEUIINFO_MENU_UNDO_MOVE_ITEM (NULL, NULL),
  GNOMEUIINFO_SEPARATOR,
  GNOMEUIINFO_MENU_END_GAME_ITEM (end_game_cb, NULL),
  GNOMEUIINFO_MENU_EXIT_ITEM (delete_event_cb, NULL),
  GNOMEUIINFO_END
};

static GnomeUIInfo
settings_menu[] = {
  GNOMEUIINFO_MENU_PREFERENCES_ITEM (iagno2_preferences_cb, NULL),
  GNOMEUIINFO_END
};

static GnomeUIInfo
help_menu[] = {
  GNOMEUIINFO_HELP ("iagno2"),
  GNOMEUIINFO_MENU_ABOUT_ITEM (about_cb, NULL),
  GNOMEUIINFO_END
};

GnomeUIInfo iagno2_menu[] = {
  GNOMEUIINFO_MENU_GAME_TREE (game_menu),
  GNOMEUIINFO_MENU_SETTINGS_TREE (settings_menu),
  GNOMEUIINFO_MENU_HELP_TREE (help_menu),
  GNOMEUIINFO_END
};

gint
about_cb ()
{
  static GtkWidget *about;

  const gchar *authors[] = {
    "Ian Peters (itp@gnu.org)",
    NULL
  };

  if (about != NULL) {
    gdk_window_raise (about->window);
    gdk_window_show (about->window);
    return;
  }

  about = gnome_about_new (_("Iagno II"),
                           "0.1.0",
                           _("(C) 1999-2000 Ian Peters"),
                           (const char **) authors,
                           _("A reversi game for GNOME."),
                           NULL);

  gtk_signal_connect (GTK_OBJECT (about), "destroy",
                      GTK_SIGNAL_FUNC (gtk_widget_destroyed),
                      &about);
  gnome_dialog_set_parent (GNOME_DIALOG (about), GTK_WINDOW (app));

  gtk_widget_show (about);
}

gint
delete_event_cb (GtkWidget *window, GdkEventAny *event, gpointer data)
{
  gtk_main_quit ();
  return FALSE;
}

static void
clean_up (gboolean full)
{  
  if (computer_timeout_id) {
    gtk_timeout_remove (computer_timeout_id);
    computer_timeout_id = 0;
  }

  if (computer_thread) {
    kill (computer_thread, SIGUSR1);
    waitpid (computer_thread, NULL, 0);
    computer_thread = 0;
  }

  if (board) {
    reversi_destroy_board (board);
    board = NULL;
  }

  if (board_pixmaps) {
    g_free (board_pixmaps);
    board_pixmaps = NULL;
  }

  if (game_over_flip_id) {
    gtk_timeout_remove (game_over_flip_id);
    game_over_flip_id = 0;
  }

  if (full) {
    if (properties) {
      iagno2_properties_destroy (properties);
      properties = NULL;
    }

    if (buffer_pixmap) {
      gdk_pixmap_unref (buffer_pixmap);
      buffer_pixmap = NULL;
    }

    gdk_colormap_free_colors (gdk_colormap_get_system (), colors, 2);
  }
}

static gint
end_game_cb (GtkWidget *widget, gpointer data)
{
  int i;

  if (!game_in_progress) {
    return;
  }

  game_in_progress = 0;

  clean_up (FALSE);

  for (i = 0; i < BOARDSIZE * BOARDSIZE; i++) {
    iagno2_render_tile_to_buffer (0, i);
  }

  gnome_appbar_set_status (GNOME_APPBAR (appbar), "");

  iagno2_render_buffer_to_screen (0, 0, BOARDWIDTH, BOARDHEIGHT);
}

gint
new_game_cb (GtkWidget *widget, gpointer data)
{
  if (new_game_dialog) {
    return;
  }

  if (!game_in_progress) {
    new_game_real_cb (0, NULL);
    return;
  }

  new_game_dialog = gnome_question_dialog_parented (_("Game already in progress!\nStart a new game?"),
                                                    new_game_real_cb,
                                                    NULL, GTK_WINDOW (app));
}

void
new_game_real_cb (gint reply, gpointer data)
{
  game_in_progress = TRUE;
  new_game_dialog = NULL;

  if (reply) {
    return;
  }

  clean_up (FALSE);

  board = reversi_init_board ();
  board_pixmaps = g_new0 (gchar, BOARDSIZE * BOARDSIZE);
  /*
  reversi_init_board (board_pixmaps);
  */
  
  board_pixmaps[INDEX (3, 3)] = (board->board[INDEX (3, 3)] = WHITE_TILE) - 1;
  board_pixmaps[INDEX (4, 4)] = (board->board[INDEX (4, 4)] = WHITE_TILE) - 1;
  board_pixmaps[INDEX (3, 4)] = (board->board[INDEX (3, 4)] = BLACK_TILE) + 1;
  board_pixmaps[INDEX (4, 3)] = (board->board[INDEX (4, 3)] = BLACK_TILE) + 1;

  whose_turn = BLACK_TILE;

  /*
  iagno2_board_changed ();
  */

  iagno2_force_board_redraw ();

  iagno2_setup_current_player (FALSE);
}

int
main (int argc, char **argv)
{
  gnome_init ("iagno2", "0.1", argc, argv);

  properties = iagno2_properties_new ();

  board = reversi_init_board ();
  /*
  reversi_init_board (&board_pixmaps);
  */
  board_pixmaps = g_new0 (gchar, 64);

  iagno2_tileset_load ();

  iagno2_initialize_players (0);

  iagno2_app_init ();
  iagno2_appbar_init ();
  iagno2_drawing_area_init ();

  gtk_widget_show_all (app);

  iagno2_set_bg_color ();

  gtk_main ();

  clean_up (TRUE);
}
