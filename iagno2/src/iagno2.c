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
#include <gdk-pixbuf/gdk-pixbuf.h>

#include <pthread.h>

#include "defines.h"
#include "iagno2.h"
#include "main.h"
#include "properties.h"
#include "../reversi-iagno2/reversi.h"
#include "plugin.h"

extern GtkWidget *app;
extern GtkWidget *new_game_dialog;
extern GnomeUIInfo iagno2_menu[];
extern Iagno2Properties *properties;

extern ReversiBoard *board;

extern Iagno2Plugin *plugin;

extern gboolean game_in_progress;

GtkWidget *drawing_area;
GdkPixbuf *tileset_pixbuf;

GdkPixmap *buffer_pixmap = NULL;

GdkColor colors[2];

gchar *board_pixmaps = NULL;

gchar whose_turn = BLACK_TILE;

gboolean iagno2_flipping_pixmaps;

Iagno2Plugin *players[2];

gboolean interactive = 0;

gint computer_timeout_id = 0;
gint game_over_flip_id = 0;

gint computer_return = 64;

GnomeAppBar *appbar;

gint move_count = 0;

void
iagno2_tileset_load ()
{
  gchar *tmp_path;
  gchar *filename;
  gint i;

  tmp_path = g_strconcat ("iagno2/", properties->tileset, NULL);
  filename = gnome_unconditional_pixmap_file (tmp_path);
  g_free (tmp_path);

  if (!g_file_exists (filename)) {
    g_print (_("Could not find \'%s\' tileset for Iagno II\n"), filename);
    exit (1);
  }

  tileset_pixbuf = gdk_pixbuf_new_from_file (filename);

  g_free (filename);
}

void
iagno2_render_tile (int tile, int index)
{
  GdkGC *gc = drawing_area->style->fg_gc[GTK_WIDGET_STATE (drawing_area)];
  int x = ROW (index);
  int y = COL (index);

  gdk_pixbuf_render_to_drawable (tileset_pixbuf,
                                 drawing_area->window, gc,
                                 tile * TILEWIDTH, 0,
                                 /*
                                 x * TILEWIDTH, y * TILEHEIGHT,
                                 */
                                 x * (TILEWIDTH + GRIDWIDTH),
                                 y * (TILEHEIGHT + GRIDWIDTH),
                                 TILEWIDTH, TILEHEIGHT,
                                 GDK_RGB_DITHER_NORMAL,
                                 0, 0);
  
  gdk_pixbuf_render_to_drawable (tileset_pixbuf,
                                 buffer_pixmap, gc,
                                 tile * TILEWIDTH, 0,
                                 /*
                                 x * TILEWIDTH, y * TILEHEIGHT,
                                 */
                                 x * (TILEWIDTH + GRIDWIDTH),
                                 y * (TILEHEIGHT + GRIDWIDTH),
                                 TILEWIDTH, TILEHEIGHT,
                                 GDK_RGB_DITHER_NORMAL,
                                 0, 0);
}

void
iagno2_render_tile_to_buffer (int tile, int index)
{
  GdkGC *gc = drawing_area->style->fg_gc[GTK_WIDGET_STATE (drawing_area)];
  int x = ROW (index);
  int y = COL (index);

  gdk_pixbuf_render_to_drawable (tileset_pixbuf,
                                 buffer_pixmap, gc,
                                 tile * TILEWIDTH, 0,
                                 /*
                                 x * TILEWIDTH, y * TILEHEIGHT,
                                 */
                                 x * (TILEWIDTH + GRIDWIDTH),
                                 y * (TILEHEIGHT + GRIDWIDTH),
                                 TILEWIDTH, TILEHEIGHT,
                                 GDK_RGB_DITHER_NORMAL,
                                 0, 0);
}

/*
static GdkColor
iagno2_get_pixbuf_color (GdkPixbuf *pixbuf)
{
  GdkColor color;
  guchar *pixels;
  gint bits;

  pixels = gdk_pixbuf_get_pixels (pixbuf);

  color.red = pixels[0];
  color.green = pixels[1];
  color.blue = pixels[2];
  bits = gdk_pixbuf_get_bits_per_sample (pixbuf);

  color.pixel = color.red << (bits * 2) + color.green << (bits) + color.blue;

  return (color);
}

static GdkColor
iagno2_get_grid_color (GdkColor bg_color)
{
  GdkColor color;

  color.pixel = 0xFFFFFF - bg_color.pixel;
}
*/

void
iagno2_set_bg_color ()
{
  /*
  GdkColor color;
  */
  guchar *pixels;
  GdkGC *grid_gc;

  pixels = gdk_pixbuf_get_pixels (tileset_pixbuf);

  colors[0].red = pixels[0] << 8;
  colors[0].green = pixels[1] << 8;
  colors[0].blue = pixels[2] << 8;

  gdk_colormap_alloc_color (gdk_colormap_get_system (), &colors[0],
                            FALSE, TRUE);

  gdk_window_set_background (drawing_area->window, &colors[0]);

  /*
  gdk_gc_set_background (gc, &color);
  gdk_gc_set_foreground (gc, &color);
  */

  colors[1].red = 0xFFFF - colors[0].red;
  colors[1].green = 0xFFFF - colors[0].green;
  colors[1].blue = 0xFFFF - colors[0].blue;

  gdk_colormap_alloc_color (gdk_colormap_get_system (), &colors[1],
                            FALSE, TRUE);

  iagno2_draw_grid ();
}

void
iagno2_app_init ()
{
  app = gnome_app_new ("iagno2", _("Iagno II"));

  gnome_app_create_menus (GNOME_APP (app), iagno2_menu);

  gtk_window_set_policy (GTK_WINDOW (app), FALSE, FALSE, TRUE);

  gtk_signal_connect (GTK_OBJECT (app),
      "delete_event",
      GTK_SIGNAL_FUNC (delete_event_cb),
      NULL);

  /*
  iagno2_set_bg_color ();
  */
}

void
iagno2_appbar_init ()
{
  appbar = GNOME_APPBAR (gnome_appbar_new (FALSE, TRUE, FALSE));

  gnome_app_set_statusbar (GNOME_APP (app), GTK_WIDGET (appbar));

  gnome_appbar_set_status (GNOME_APPBAR (appbar), _(" Welcome to Iagno II!"));
}

/*
void
iagno2_set_bg_color ()
{
  bg_color = iagno2_get_pixbuf_color (tileset_pixbuf);

  grid_color = iagno2_get_grid_color (bg_color);

  gdk_window_set_background (drawing_area->window, &bg_color);
}
*/

static void
iagno2_render_buffer_to_screen (int x, int y, int width, int height)
{
  GdkGC *gc = drawing_area->style->fg_gc[GTK_WIDGET_STATE (drawing_area)];

  gdk_draw_pixmap (drawing_area->window, gc,
                   buffer_pixmap,
                   x, y,
                   x, y,
                   width, height);
}

void
iagno2_draw_grid ()
{
  GdkGC *grid_gc;
  int i, j;

  grid_gc = gdk_gc_new (drawing_area->window);

  gdk_gc_copy (grid_gc, drawing_area->style->bg_gc[0]);

  if (properties->draw_grid) {
    i = 1;
  } else {
    i = 0;
  }

  gdk_gc_set_background (grid_gc, &colors[i]);
  gdk_gc_set_foreground (grid_gc, &colors[i]);

  for (i = 1; i < BOARDSIZE; i++) {
    gdk_draw_line (buffer_pixmap, grid_gc, i * (TILEWIDTH + GRIDWIDTH) - 1, 0,
                   i * (TILEWIDTH + GRIDWIDTH) - 1, BOARDHEIGHT);
    gdk_draw_line (buffer_pixmap, grid_gc, 0, i * (TILEHEIGHT + GRIDWIDTH) - 1,
                   BOARDWIDTH, i * (TILEHEIGHT + GRIDWIDTH) - 1);
  }

  gdk_gc_unref (grid_gc);

  iagno2_render_buffer_to_screen (0, 0, BOARDWIDTH, BOARDHEIGHT);
}

static gint
drawing_area_expose_event_cb (GtkWidget *widget, GdkEventExpose *event)
{
  iagno2_render_buffer_to_screen (event->area.x, event->area.y,
                                  event->area.width, event->area.height);
}

static gint
drawing_area_configure_event_cb (GtkWidget *widget, GdkEventConfigure *event)
{
  gint i;

  if (buffer_pixmap != NULL) {
    gdk_pixmap_unref (buffer_pixmap);
    buffer_pixmap = NULL;
  }

  buffer_pixmap = gdk_pixmap_new (drawing_area->window,
                                  BOARDWIDTH, BOARDHEIGHT,
                                  -1);

  for (i = 0; i < BOARDSIZE * BOARDSIZE; i++) {
    iagno2_render_tile_to_buffer (0, i);
  }
  
  return (TRUE);
}

static gint
drawing_area_button_press_event_cb (GtkWidget *widget, GdkEvent *event,
                                    gpointer data)
{
  double x, y;
  gint grid_row, grid_col, index;

  switch (event->type) {
    case GDK_BUTTON_PRESS:
      if (interactive) {
        x = event->button.x;
        y = event->button.y;

        grid_row = x / (TILEWIDTH + GRIDWIDTH);
        grid_col = y / (TILEHEIGHT + GRIDWIDTH);

        index = INDEX (grid_row, grid_col);

        if (board->board[index]) {
          return;
        }

        if (is_valid_move (board, index, whose_turn)) {
          iagno2_move (index);
  
          iagno2_board_changed ();
        }
      }

      break;
  }

  return FALSE;
}

void
iagno2_drawing_area_init ()
{
  gtk_widget_push_visual (gdk_rgb_get_visual ());
  gtk_widget_push_colormap (gdk_rgb_get_cmap ());
  drawing_area = gtk_drawing_area_new ();
  gtk_widget_pop_colormap ();
  gtk_widget_pop_visual ();

  gtk_drawing_area_size (GTK_DRAWING_AREA (drawing_area),
                         BOARDWIDTH, BOARDHEIGHT);

  gtk_signal_connect (GTK_OBJECT (drawing_area), "expose_event",
                      GTK_SIGNAL_FUNC (drawing_area_expose_event_cb), NULL);
  gtk_signal_connect (GTK_OBJECT (drawing_area), "configure_event",
                      GTK_SIGNAL_FUNC (drawing_area_configure_event_cb), NULL);
  gtk_signal_connect (GTK_OBJECT (drawing_area), "button_press_event",
                      GTK_SIGNAL_FUNC (drawing_area_button_press_event_cb),
                      NULL);
  gtk_widget_set_events (drawing_area,
                         GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK);

  gnome_app_set_contents (GNOME_APP (app), drawing_area);

  /*
  if (!gc) {
    gc = gdk_gc_new (drawing_area->window);
    gdk_gc_copy (gc, drawing_area->style->bg_gc[0]);
  }

  printf ("gc initialized\n");
  */
}

void
iagno2_force_board_redraw ()
{
  gint i;

  for (i = 0; i < BOARDSIZE * BOARDSIZE; i++) {
    iagno2_render_tile_to_buffer (board->board[i], i);
  }

  iagno2_render_buffer_to_screen (0, 0, BOARDWIDTH, BOARDHEIGHT);
}

static gint
iagno2_flip_tiles ()
{
  gint i;
  gboolean more_to_flip = 0;
  gint delta;

  for (i = 0; i < BOARDSIZE * BOARDSIZE; i++) {
    if (board->board[i] == board_pixmaps[i]) {
      continue;
    }
    if (!board->board[i]) {
      board_pixmaps[i] = board->board[i];
      iagno2_render_tile (board_pixmaps[i], i);
      continue;
    }
    if (!board_pixmaps[i]) {
      board_pixmaps[i] = board->board[i];
      iagno2_render_tile (board_pixmaps[i], i);
      continue;
    }
    if (board_pixmaps[i] < board->board[i]) {
      delta = 1;
    } else {
      delta = -1;
    }
    board_pixmaps[i] += delta;
    iagno2_render_tile (board_pixmaps[i], i);
    if (board_pixmaps[i] != board->board[i]) {
      more_to_flip = 1;
    }
  }

  if (!more_to_flip) {
    iagno2_flipping_pixmaps = 0;
  }

  return (more_to_flip);
}

void
iagno2_board_changed ()
{
  if (!iagno2_flipping_pixmaps) {
    gtk_timeout_add (20, iagno2_flip_tiles, NULL);
    iagno2_flipping_pixmaps = 1;
  }
}

static gint
iagno2_get_computer_move ()
{
  if (computer_return != 64) {
    iagno2_move (computer_return);
    return (FALSE);
  }

  return TRUE;
}

static void
iagno2_computer_thread ()
{
  int player = PLAYER (whose_turn);
  
  computer_return = players[player]->plugin_move (board, whose_turn);

  _exit (0);
}

static gint
iagno2_computer_player_wrapper ()
{
  pthread_t tid;

  computer_return = 64;

  pthread_create (&tid, NULL, (void *)iagno2_computer_thread, NULL);

  computer_timeout_id = gtk_timeout_add (500, iagno2_get_computer_move, NULL);

  return FALSE;
}

void
iagno2_setup_current_player (gboolean pass)
{
  gchar player = PLAYER (whose_turn);
  gchar *sides[2];
  gchar *interactive_message;
  gchar *message;
  gchar *pad;

  sides[0] = g_strdup (_("Dark"));
  sides[1] = g_strdup (_("Light"));

  if (pass) {
    pad = g_strconcat (" [", sides[(player)?0:1],
                       _(" was forced to pass] "), NULL);
  } else {
    pad = g_strdup (" ");
  }

  computer_timeout_id = 0;
  
  if (players[player] == NULL) {
    interactive = 1;
    message = g_strconcat (pad, _("Waiting for input from user... ["),
                           sides[player], "]", NULL);
  } else {
    interactive = 0;
    computer_timeout_id = gtk_timeout_add (1000,
                                           iagno2_computer_player_wrapper,
                                           NULL);
    message = g_strconcat (pad, players[player]->plugin_busy_message (),
                           " [", sides[player], "]", NULL);
  }

  gnome_appbar_set_status (GNOME_APPBAR (appbar), message);

  g_free (sides[0]);
  g_free (sides[1]);
  g_free (message);
  g_free (pad);
}

static void
iagno2_post_move_check ()
{
  gboolean pass = FALSE;
  
  iagno2_board_changed ();

  whose_turn = OTHER_TILE (whose_turn);

  if (!are_valid_moves (board, whose_turn)) {
    if (!are_valid_moves (board, OTHER_TILE (whose_turn))) {
      game_over_flip_id = gtk_timeout_add (3000,
          iagno2_game_over, NULL);
      gnome_appbar_set_status (GNOME_APPBAR (appbar), " Game over!");
      return;
    } else {
      whose_turn = OTHER_TILE (whose_turn);
      pass = TRUE;
    }
  }

  iagno2_setup_current_player (pass);
}

void
iagno2_move (gchar index)
{
  move (board, index, whose_turn);

  iagno2_post_move_check ();
}

void
iagno2_initialize_players (int which)
{
  gint i;
  gchar *tmp_path;
  gchar *filename;

  if ((which == 0) || (which == 1)) {
    if (players[0] != NULL) {
      iagno2_plugin_close (players[0]);
      players[0] = NULL;
    }
    if (!strcmp (properties->player1, "Human")) {
      players[0] = NULL;
    } else {
      tmp_path = g_strconcat ("iagno2/",
          properties->player1, NULL);
      filename = gnome_unconditional_libdir_file (tmp_path);
      g_free (tmp_path);
      players[0] = iagno2_plugin_open (filename);
      g_free (filename);
      if (players[0] != NULL) {
        players[0]->plugin_init ();
      } else {
        g_free (properties->player1);
        properties->player1 = g_strdup ("Human");
        iagno2_properties_sync (properties);
      }
    }
  }

  if ((which ==0) || (which == 2)) {
    if (players[1] != NULL) {
      iagno2_plugin_close (players[1]);
      players[1] = NULL;
    }
    if (!strcmp (properties->player2, "Human")) {
      players[1] = NULL;
    } else {
      tmp_path = g_strconcat ("iagno2/",
          properties->player2, NULL);
      filename = gnome_unconditional_libdir_file (tmp_path);
      g_free (tmp_path);
      players[1] = iagno2_plugin_open (filename);
      g_free (filename);
      if (players[1] != NULL) {
        players[1]->plugin_init ();
      } else {
        g_free (properties->player2);
        properties->player2 = g_strdup ("Human");
        iagno2_properties_sync (properties);
      }
    }
  }
}

gint
iagno2_game_over ()
{
  gchar white_count = 0;
  gchar black_count = 0;
  gchar i;
  gchar *message;

  game_in_progress = FALSE;

  for (i = 0; i < BOARDSIZE * BOARDSIZE; i++) {
    if (board->board[i] == WHITE_TILE) {
      white_count++;
    } else {
      black_count++;
    }
  }

  for (i = 0; i < black_count; i++) {
    board->board[i] = BLACK_TILE;
  }

  for (i = black_count; i < (64 - white_count); i++) {
    board->board[i] = 0;
  }

  for (i = (64 - white_count); i < BOARDSIZE * BOARDSIZE; i++) {
    board->board[i] = WHITE_TILE;
  }

  iagno2_board_changed ();

  game_over_flip_id = 0;

  if (black_count > white_count) {
    message = g_strdup_printf (_(" Dark player wins by a score of %d to %d"),
                               black_count, white_count);
  } else if (white_count > black_count) {
    message = g_strdup_printf (_(" Light player wins by a score of %d to %d"),
                               white_count, black_count);
  } else {
    message = g_strdup_printf (_(" The game was a tie at %d"), black_count);
  }

  gnome_appbar_set_status (GNOME_APPBAR (appbar), message);

  g_free (message);

  return FALSE;
}
