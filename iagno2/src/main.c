#include <gnome.h>
#include <gmodule.h>

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

extern gchar *board;
extern gchar *board_pixmaps;

extern Iagno2Plugin *players[2];

extern gchar whose_turn;

extern gint computer_timeout_id;
extern gint game_over_flip_id;

static GnomeUIInfo
game_menu[] = {
  GNOMEUIINFO_MENU_NEW_GAME_ITEM (new_game_cb, NULL),
  GNOMEUIINFO_SEPARATOR,
  GNOMEUIINFO_MENU_UNDO_MOVE_ITEM (NULL, NULL),
  GNOMEUIINFO_SEPARATOR,
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
    "Ian Peters",
    NULL
  };

  if (about != NULL) {
    gdk_window_raise (about->window);
    gdk_window_show (about->window);
    return;
  }

  about = gnome_about_new (_("Iagno II"),
                           "0.1.0",
                           _("Copyright 2000 Ian Peters"),
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
  
  if (board) {
    reversi_destroy_board (&board);
  }

  if (board_pixmaps) {
    reversi_destroy_board (&board_pixmaps);
  }

  if (computer_timeout_id) {
    gtk_timeout_remove (computer_timeout_id);
    computer_timeout_id = 0;
  }

  if (game_over_flip_id) {
    gtk_timeout_remove (game_over_flip_id);
    game_over_flip_id = 0;
  }

  reversi_init_board (&board);
  reversi_init_board (&board_pixmaps);
  
  board_pixmaps[INDEX (3, 3)] = (board[INDEX (3, 3)] = WHITE_TILE) - 1;
  board_pixmaps[INDEX (4, 4)] = (board[INDEX (4, 4)] = WHITE_TILE) - 1;
  board_pixmaps[INDEX (3, 4)] = (board[INDEX (3, 4)] = BLACK_TILE) + 1;
  board_pixmaps[INDEX (4, 3)] = (board[INDEX (4, 3)] = BLACK_TILE) + 1;

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

  reversi_init_board (&board);
  reversi_init_board (&board_pixmaps);

  iagno2_tileset_load ();

  iagno2_initialize_players (0);

  iagno2_app_init ();
  iagno2_appbar_init ();
  iagno2_drawing_area_init ();
  
  gtk_widget_show_all (app);

  gtk_main ();

  iagno2_properties_destroy (properties);

  reversi_destroy_board (&board);
  reversi_destroy_board (&board_pixmaps);
}
