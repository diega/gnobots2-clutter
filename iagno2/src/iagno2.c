#include <gnome.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "defines.h"
#include "iagno2.h"
#include "main.h"
#include "properties.h"
#include "../reversi-iagno2/reversi.h"
#include "plugin.h"

extern GtkWidget *app;
extern GnomeUIInfo iagno2_menu[];
extern Iagno2Properties *properties;

extern gchar *board;

extern Iagno2Plugin *plugin;

/*
GtkWidget *canvas;
GdkImlibImage *tileset_images[32];
GnomeCanvasItem *background;
GnomeCanvasGroup *grid_lines;
GnomeCanvasItem *board_items[BOARDSIZE * BOARDSIZE];
GdkColor bg_color;
GdkColor grid_color;
*/

GtkWidget *drawing_area;
/*
GdkPixbuf *tileset_pixbufs[32];
*/
GdkPixbuf *tileset_pixbuf;
GdkColor bg_color;

GdkPixmap *buffer_pixmap = NULL;

gchar *board_pixmaps = NULL;

gchar whose_turn = BLACK_TILE;

gboolean iagno2_flipping_pixmaps;

Iagno2Plugin *players[2];

gboolean interactive = 0;

gint computer_timeout_id = 0;
gint game_over_flip_id = 0;

void
iagno2_tileset_load ()
{
  /*
  GdkImlibImage *tileset_image;
  */
  /*
  GdkPixbuf *tileset_pixbuf;
  */
  gchar *tmp_path;
  gchar *filename;
  /*
  gint i, j;
  */
  gint i;

  tmp_path = g_strconcat ("iagno2/", properties->tileset, NULL);
  filename = gnome_unconditional_pixmap_file (tmp_path);
  g_free (tmp_path);

  if (!g_file_exists (filename)) {
    g_print (_("Could not find \'%s\' tileset for Iagno2\n"), filename);
    exit (1);
  }

  tileset_pixbuf = gdk_pixbuf_new_from_file (filename);

  /*
  tileset_image = gdk_imlib_load_image (filename);
  */

  /*
  for (i = 0; i < 8; i++) {
    for (j = 0; j < 4; j++) {
      tileset_images[j * 8 + i] =
        gdk_imlib_crop_and_clone_image (tileset_image,
            i * TILEWIDTH,
            j * TILEHEIGHT,
            TILEWIDTH,
            TILEHEIGHT);
    }
  }
  */

  /*
  for (i = 0; i < 32; i++) {
    if (tileset_pixbufs[i]) {
      gdk_pixbuf_unref (tileset_pixbufs[i]);
      tileset_pixbufs[i] = NULL;
    }

    tileset_pixbufs[i] = gdk_pixbuf_new (ART_PIX_RGB, FALSE, 8,
                                         TILEWIDTH, TILEHEIGHT);
    
    gdk_pixbuf_copy_area (tileset_pixbuf,
                          i * TILEWIDTH, 0,
                          TILEWIDTH, TILEHEIGHT,
                          tileset_pixbufs[i],
                          0, 0);
  }
  */

  /*
  gdk_imlib_destroy_image (tileset_image);
  */

  /*
  gdk_pixbuf_unref (tileset_pixbuf);
  */

  g_free (filename);
}

void
iagno2_render_tile (int tile, int index)
{
  GdkGC *gc = drawing_area->style->fg_gc[GTK_WIDGET_STATE (drawing_area)];
  int x = ROW (index);
  int y = COL (index);

  /*
  gdk_pixbuf_render_to_drawable (tileset_pixbufs[tile],
  */
  gdk_pixbuf_render_to_drawable (tileset_pixbuf,
                                 drawing_area->window, gc,
                                 tile * TILEWIDTH, 0,
                                 /*
                                 0, 0,
                                 */
                                 x * TILEWIDTH, y * TILEHEIGHT,
                                 TILEWIDTH, TILEHEIGHT,
                                 GDK_RGB_DITHER_NORMAL,
                                 0, 0);
  
  /*
  gdk_pixbuf_render_to_drawable (tileset_pixbufs[tile],
  */
  gdk_pixbuf_render_to_drawable (tileset_pixbuf,
                                 buffer_pixmap, gc,
                                 tile * TILEWIDTH, 0,
                                 /*
                                 0, 0,
                                 */
                                 x * TILEWIDTH, y * TILEHEIGHT,
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

  /*
  gdk_pixbuf_render_to_drawable (tileset_pixbufs[tile],
  */
  gdk_pixbuf_render_to_drawable (tileset_pixbuf,
                                 buffer_pixmap, gc,
                                 tile * TILEWIDTH, 0,
                                 /*
                                 0, 0,
                                 */
                                 x * TILEWIDTH, y * TILEHEIGHT,
                                 TILEWIDTH, TILEHEIGHT,
                                 GDK_RGB_DITHER_NORMAL,
                                 0, 0);
}

static GdkColor
iagno2_get_pixbuf_color (GdkPixbuf *pixbuf)
{
  /*
  GdkVisual *visual;
  GdkColor color;
  GdkImage *image;
  GdkPixmap *pixmap;
  
  visual = gdk_imlib_get_visual ();
  if (visual->type != GDK_VISUAL_TRUE_COLOR) {
    gdk_imlib_set_render_type (RT_PLAIN_PALETTE);
  }
  gdk_imlib_render (imlib_image,
      imlib_image->rgb_width,
      imlib_image->rgb_height);
  pixmap = gdk_imlib_move_image (imlib_image);
  image = gdk_image_get (pixmap, 0, 0, 1, 1);
  
  color.pixel = gdk_image_get_pixel (image, 0, 0);

  gdk_image_destroy (image);
  gdk_pixmap_unref (pixmap);
  */

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
}

void
iagno2_set_bg_color ()
{
  /*
  bg_color = iagno2_get_pixbuf_color (tileset_pixbufs[0]);
  */
  bg_color = iagno2_get_pixbuf_color (tileset_pixbuf);

  /*
  grid_color.pixel = 0xFFFFFF - bg_color.pixel;
  */

  /*
  gnome_canvas_item_set (background,
      "fill_color_gdk", &bg_color,
      NULL);
      */

  gdk_window_set_background (drawing_area->window, &bg_color);
}

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

        /*
        grid_row = y / (TILEWIDTH + GRIDWIDTH);
        grid_col = x / (TILEHEIGHT + GRIDWIDTH);
        */

        grid_row = x / TILEWIDTH;
        grid_col = y / TILEHEIGHT;

        index = INDEX (grid_row, grid_col);

        if (board[index]) {
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

/*
void
iagno2_canvas_init ()
{
  GnomeCanvasGroup *group;
  GnomeCanvasItem *item;
  gint i;
  GnomeCanvasPoints *points;
  gchar r, c;
  double x, y;
  
  gtk_widget_push_visual (gdk_imlib_get_visual ());
  gtk_widget_push_colormap (gdk_imlib_get_colormap ());
  canvas = gnome_canvas_new ();
  gtk_widget_pop_colormap ();
  gtk_widget_pop_visual ();

  gtk_widget_set_usize (GTK_WIDGET (canvas),
      BOARDWIDTH, BOARDHEIGHT);

  gnome_canvas_set_scroll_region (GNOME_CANVAS (canvas),
      0, 0,
      BOARDWIDTH, BOARDHEIGHT);

  group = gnome_canvas_root (GNOME_CANVAS (canvas));

  background = gnome_canvas_item_new (group,
      gnome_canvas_rect_get_type (),
      "x1", 0.0,
      "y1", 0.0,
      "x2", (double) BOARDWIDTH,
      "y2", (double) BOARDHEIGHT,
      NULL);

  grid_lines = GNOME_CANVAS_GROUP (gnome_canvas_item_new (group,
        gnome_canvas_group_get_type (),
        "x", 0.0,
        "y", 0.0,
        NULL));

  points = gnome_canvas_points_new (2);

  for (i = 1; i < BOARDSIZE; i++) {
    points->coords[0] = 0.0;
    points->coords[1] = (double) (i * (TILEHEIGHT + GRIDWIDTH) - 1);
    points->coords[2] = BOARDWIDTH;
    points->coords[3] = points->coords[1];
    item = gnome_canvas_item_new (grid_lines,
        gnome_canvas_line_get_type (),
        "points", points,
        "width_units", 1.0,
        NULL);
    points->coords[0] = (double) (i * (TILEWIDTH + GRIDWIDTH) - 1);
    points->coords[1] = 0.0;
    points->coords[2] = points->coords[0];
    points->coords[3] = BOARDHEIGHT;
    item = gnome_canvas_item_new (grid_lines,
        gnome_canvas_line_get_type (),
        "points", points,
        "width_units", 1.0,
        NULL);
  }

  gnome_canvas_points_unref (points);

  iagno2_set_bg_color ();
  iagno2_show_grid_lines ();

  for (i = 0; i < BOARDSIZE * BOARDSIZE; i++) {
    r = ROW (i);
    c = COL (i);
    board_items[i] = gnome_canvas_item_new (group,
        gnome_canvas_image_get_type (),
        "image", tileset_images[0],
        "x", (double) (c * (TILEWIDTH + GRIDWIDTH)),
        "y", (double) (r * (TILEHEIGHT + GRIDWIDTH)),
        "anchor", GTK_ANCHOR_NORTH_WEST,
        "width", (double) TILEWIDTH,
        "height", (double) TILEHEIGHT,
        NULL);
    gtk_signal_connect (GTK_OBJECT (board_items[i]),
        "event",
        GTK_SIGNAL_FUNC (board_item_cb),
        NULL);
  }

  gnome_app_set_contents (GNOME_APP (app), canvas);
}
*/

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
  drawing_area_configure_event_cb (drawing_area, NULL);
  */
}

void
iagno2_force_board_redraw ()
{
  gint i;

  for (i = 0; i < BOARDSIZE * BOARDSIZE; i++) {
    /*
    gnome_canvas_item_set (GNOME_CANVAS_ITEM (board_items[i]),
        "image", tileset_images[board[i]],
        NULL);
        */
    iagno2_render_tile_to_buffer (board[i], i);
  }

  iagno2_render_buffer_to_screen (0, 0, BOARDWIDTH, BOARDHEIGHT);
}

void
iagno2_show_grid_lines ()
{
  /*
  GdkColor eff_grid_color;
  GList *lines;

  if (properties->draw_grid) {
    eff_grid_color = grid_color;
  } else {
    eff_grid_color = bg_color;
  }

  lines = grid_lines->item_list;

  while (lines != NULL) {
    gnome_canvas_item_set (GNOME_CANVAS_ITEM (lines->data),
        "fill_color_gdk", &eff_grid_color,
        NULL);
    lines = lines->next;
  }
  */
}

/*
static void
iagno2_draw_tile (gint item, gchar tile)
{
  gnome_canvas_item_set (GNOME_CANVAS_ITEM (board_items[item]),
      "image", tileset_images[tile],
      NULL);
}
*/

static gint
iagno2_flip_tiles ()
{
  gint i;
  gboolean more_to_flip = 0;
  gint delta;

  for (i = 0; i < BOARDSIZE * BOARDSIZE; i++) {
    if (board[i] == board_pixmaps[i]) {
      continue;
    }
    if (!board_pixmaps[i]) {
      board_pixmaps[i] = board[i];
      iagno2_render_tile (board_pixmaps[i], i);
      continue;
    }
    if (board_pixmaps[i] < board[i]) {
      delta = 1;
    } else {
      delta = -1;
    }
    board_pixmaps[i] += delta;
    iagno2_render_tile (board_pixmaps[i], i);
    if (board_pixmaps[i] != board[i]) {
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

void
iagno2_setup_current_player ()
{
  gchar player;

  computer_timeout_id = 0;
  
  if (whose_turn - 1) {
    player = 1;
  } else {
    player = 0;
  }

  if (players[player] == NULL) {
    interactive = 1;
  } else {
    interactive = 0;
    computer_timeout_id =
      gtk_timeout_add (1000,
          players[player]->plugin_move,
          board);
  }
}

static void
iagno2_post_move_check ()
{
  iagno2_board_changed ();

  whose_turn = other_player (whose_turn);

  if (!are_valid_moves (board, whose_turn)) {
    if (!are_valid_moves (board, other_player (whose_turn))) {
      printf ("The game is over!\n");
      game_over_flip_id = gtk_timeout_add (3000,
          iagno2_game_over, NULL);
      return;
    } else {
      printf ("A player had to pass!\n");
      whose_turn = other_player (whose_turn);
    }
  }

  iagno2_setup_current_player ();
}

void
iagno2_move (gchar index)
{
  move (board, index, whose_turn);

  iagno2_post_move_check ();
}

void
iagno2_initialize_players ()
{
  gint i;
  gchar *tmp_path;
  gchar *filename;

  if (players[0] != NULL) {
    iagno2_plugin_close (players[0]);
    players[0] = NULL;
  }
  if (!strcmp (properties->player1, "Human")) {
    players[0] = NULL;
    printf ("Player 1 is \"Human\"\n");
  } else {
    tmp_path = g_strconcat ("iagno2/",
        properties->player1, NULL);
    filename = gnome_unconditional_libdir_file (tmp_path);
    g_free (tmp_path);
    players[0] = iagno2_plugin_open (filename);
    g_free (filename);
    printf ("Player 1 is \"%s\"\n", players[0]->plugin_name ());
    players[0]->plugin_init ();
  }

  if (players[1] != NULL) {
    iagno2_plugin_close (players[1]);
    players[1] = NULL;
  }
  if (!strcmp (properties->player2, "Human")) {
    players[1] = NULL;
    printf ("Player 2 is \"Human\"\n");
  } else {
    tmp_path = g_strconcat ("iagno2/",
        properties->player2, NULL);
    filename = gnome_unconditional_libdir_file (tmp_path);
    g_free (tmp_path);
    players[1] = iagno2_plugin_open (filename);
    g_free (filename);
    printf ("Player 2 is \"%s\"\n", players[1]->plugin_name ());
    players[1]->plugin_init ();
  }
}

gint
iagno2_game_over ()
{
  gchar white_count = 0;
  gchar black_count = 0;
  gchar i;

  for (i = 0; i < BOARDSIZE * BOARDSIZE; i++) {
    if (board[i] == WHITE_TILE) {
      white_count++;
    } else {
      black_count++;
    }
  }

  for (i = 0; i < black_count; i++) {
    board[i] = BLACK_TILE;
  }

  for (i = black_count; i < BOARDSIZE * BOARDSIZE; i++) {
    board[i] = WHITE_TILE;
  }

  iagno2_board_changed ();

  game_over_flip_id = 0;

  return FALSE;
}
