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
Iagno2Properties *properties;

Iagno2Plugin *plugin;

extern gchar *board;
extern gchar *board_pixmaps;

extern Iagno2Plugin *players[2];

extern gchar whose_turn;

extern gint computer_timeout_id;
extern gint game_over_flip_id;

int number = 3;

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

GnomeUIInfo iagno2_menu[] = {
  GNOMEUIINFO_MENU_GAME_TREE (game_menu),
  GNOMEUIINFO_MENU_SETTINGS_TREE (settings_menu),
  GNOMEUIINFO_END
};

gint
delete_event_cb (GtkWidget *window, GdkEventAny *event, gpointer data)
{
  gtk_main_quit ();
  return FALSE;
}

gint
new_game_cb (GtkWidget *widget, gpointer data)
{
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

  iagno2_setup_current_player ();
}

int
main (int argc, char **argv)
{
  gnome_init ("iagno2", "0.1", argc, argv);

  properties = iagno2_properties_new ();

  reversi_init_board (&board);
  reversi_init_board (&board_pixmaps);

  iagno2_tileset_load ();

  iagno2_initialize_players ();

  iagno2_app_init ();
  iagno2_drawing_area_init ();
  
  gtk_widget_show_all (app);

  gtk_main ();

  iagno2_properties_destroy (properties);

  reversi_destroy_board (&board);
  reversi_destroy_board (&board_pixmaps);
}
