/*
 * Gnome-Mahjongg
 * (C) 1998 the Free Software Foundation
 *
 *
 * Author: Francisco Bustamante
 *
 *
 * http://www.nuclecu.unam.mx/~pancho/
 * pancho@nuclecu.unam.mx
 *
 */

#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <dirent.h>
#include <config.h>

#include <gtk/gtk.h>
#include <gnome.h>
#include <gdk_imlib.h>

#include "mahjongg.h"
#include "solubility.h"

#include "button-images.h"

typeinfo type_info [MAX_TILES+70] = {
  	{ 0, 0, {0, 0} },
	{ 0, 0, {0, 0} },
	{ 1, 0, {1, 1} },
	{ 1, 0, {1, 1} },
	{ 2, 0, {2, 2} },
	{ 2, 0, {2, 2} },
	{ 3, 0, {3, 3} },
	{ 3, 0, {3, 3} },
	{ 4, 0, {4, 4} },
	{ 4, 0, {4, 4} },
	{ 5, 0, {5, 5} },
	{ 5, 0, {5, 5} },
	{ 6, 0, {6, 6} },
	{ 6, 0, {6, 6} },
	{ 7, 0, {7, 7} },
	{ 7, 0, {7, 7} },
	{ 8, 0, {8, 8} },
	{ 8, 0, {8, 8} },
	{ 9, 0, {9, 9} },
	{ 9, 0, {9, 9} },
	{ 10, 0, {10, 10} },
	{ 10, 0, {10, 10} },
	{ 11, 0, {11, 11} },
	{ 11, 0, {11, 11} },
	{ 12, 0, {12, 12} },
	{ 12, 0, {12, 12} },
	{ 13, 0, {13, 13} },
	{ 13, 0, {13, 13} },
	{ 14, 0, {14, 14} },
	{ 14, 0, {14, 14} },
	{ 15, 0, {15, 15} },
	{ 15, 0, {15, 15} },
	{ 16, 0, {16, 16} },
	{ 16, 0, {16, 16} },
	{ 17, 0, {17, 17} },
	{ 17, 0, {17, 17} },
	{ 18, 0, {18, 18} },
	{ 18, 0, {18, 18} },
	{ 19, 0, {19, 19} },
	{ 19, 0, {19, 19} },
	{ 20, 0, {20, 20} },
	{ 20, 0, {20, 20} },
	{ 21, 0, {21, 21} },
	{ 21, 0, {21, 21} },
	{ 22, 0, {22, 22} },
	{ 22, 0, {22, 22} },
	{ 23, 0, {23, 23} },
	{ 23, 0, {23, 23} },
	{ 24, 0, {24, 24} },
	{ 24, 0, {24, 24} },
	{ 25, 0, {25, 25} },
	{ 25, 0, {25, 25} },
	{ 26, 0, {26, 26} },
	{ 26, 0, {26, 26} },
	{ 27, 0, {27, 27} },
	{ 27, 0, {27, 27} },
	{ 28, 0, {28, 28} },
	{ 28, 0, {28, 28} },
	{ 29, 0, {29, 29} },
	{ 29, 0, {29, 29} },
	{ 30, 0, {30, 30} },
	{ 30, 0, {30, 30} },
	{ 31, 0, {31, 31} },
	{ 31, 0, {31, 31} },
	{ 32, 0, {32, 32} },
	{ 32, 0, {32, 32} },
	{ 33, 0, {33, 34} },
	{ 33, 0, {35, 36} },
	{ 34, 0, {37, 37} },
	{ 34, 0, {37, 37} },
	{ 35, 0, {38, 39} },
	{ 35, 0, {40, 41} }
};

tilepos easy_map [MAX_TILES] = {
 {13, 7,  4}, {12, 8,  3}, {14, 8,  3}, {12, 6,  3},
 {14, 6,  3}, {10, 10,  2}, {12, 10,  2}, {14, 10,  2},
 {16, 10,  2}, {10, 8,  2}, {12, 8,  2}, {14, 8,  2},
 {16, 8,  2}, {10, 6,  2}, {12, 6,  2}, {14, 6,  2},
 {16, 6,  2}, {10, 4,  2}, {12, 4,  2}, {14, 4,  2},
 {16, 4,  2}, {8, 12,  1}, {10, 12,  1}, {12, 12,  1},
 {14, 12,  1}, {16, 12,  1}, {18, 12,  1}, {8, 10,  1},
 {10, 10,  1}, {12, 10,  1}, {14, 10,  1}, {16, 10,  1}, 
 {18, 10,  1}, {8, 8,  1}, {10, 8,  1}, {12, 8,  1},
 {14, 8,  1}, {16, 8,  1}, {18, 8,  1}, {8, 6,  1},
 {10, 6,  1}, {12, 6,  1}, {14, 6,  1}, {16, 6,  1},
 {18, 6,  1}, {8, 4,  1}, {10, 4,  1}, {12, 4,  1},
 {14, 4,  1}, {16, 4,  1}, {18, 4,  1}, {8, 2,  1},
 {10, 2,  1}, {12, 2,  1}, {14, 2,  1}, {16, 2,  1},
 {18, 2,  1}, {2, 14,  0}, {4, 14,  0}, {6, 14,  0},
 {8, 14,  0}, {10, 14,  0}, {12, 14,  0}, {14, 14,  0}, 
 {16, 14,  0}, {18, 14,  0}, {20, 14,  0}, {22, 14,  0},
 {24, 14,  0}, {6, 12,  0}, {8, 12,  0}, {10, 12,  0},
 {12, 12,  0}, {14, 12,  0}, {16, 12,  0}, {18, 12,  0},
 {20, 12,  0}, {4, 10,  0}, {6, 10,  0}, {8, 10,  0},
 {10, 10,  0}, {12, 10,  0}, {14, 10,  0}, {16, 10,  0},
 {18, 10,  0}, {20, 10,  0}, {22, 10,  0}, {0, 7,  0},
 {2, 8,  0}, {4, 8,  0}, {6, 8,  0}, {8, 8,  0},
 {10, 8,  0}, {12, 8,  0}, {14, 8,  0}, {16, 8,  0},
 {18, 8,  0}, {20, 8,  0}, {22, 8,  0}, {24, 8,  0},
 {2, 6,  0}, {4, 6,  0}, {6, 6,  0}, {8, 6,  0},
 {10, 6,  0}, {12, 6,  0}, {14, 6,  0}, {16, 6,  0},
 {18, 6,  0}, {20, 6,  0}, {22, 6,  0}, {24, 6,  0},
 {4, 4,  0}, {6, 4,  0}, {8, 4,  0}, {10, 4,  0},
 {12, 4,  0}, {14, 4,  0}, {16, 4,  0}, {18, 4,  0},
 {20, 4,  0}, {22, 4,  0}, {6, 2,  0}, {8, 2,  0},
 {10, 2,  0}, {12, 2,  0}, {14, 2,  0}, {16, 2,  0},
 {18, 2,  0}, {20, 2,  0}, {2, 0,  0}, {4, 0,  0},
 {6, 0,  0}, {8, 0,  0}, {10, 0,  0}, {12, 0,  0},
 {14, 0,  0}, {16, 0,  0}, {18, 0,  0}, {20, 0,  0},
 {22, 0,  0}, {24, 0,  0}, {26, 7, 0}, {28, 7, 0} 
};

/* Sorted such that the bottom leftest are first
 * Bottom left = high y, low x ! */
tilepos hard_map [MAX_TILES] = {
	{ 10, 6,  6},
	{ 9, 6,  5},
	{ 11, 6,  5},
	{ 8, 6,  4},
	{ 10, 6,  4},
	{ 12, 6,  4},
	{ 5, 6,  3},
	{ 7, 7,  3},
	{ 7, 5,  3},
	{ 9, 7,  3},
	{ 9, 5,  3},
	{ 11, 7,  3},
	{ 11, 5,  3},
	{ 13, 7,  3},
	{ 13, 5,  3},
	{ 15, 6,  3},
	{ 5, 8,  2},
	{ 7, 8,  2},
	{ 9, 8,  2},
	{ 11, 8,  2},
	{ 13, 8,  2},
	{ 15, 8,  2},
	{ 4, 6,  2},
	{ 6, 6,  2},
	{ 8, 6,  2},
	{ 10, 6,  2},
	{ 12, 6,  2},
	{ 14, 6,  2},
	{ 16, 6,  2},
	{ 5, 4,  2},
	{ 7, 4,  2},
	{ 9, 4,  2},
	{ 11, 4,  2},
	{ 13, 4,  2},
	{ 15, 4,  2},
	{ 7, 12,  1},
	{ 9, 11,  1},
	{ 11, 11,  1},
	{ 13, 12,  1},
	{ 2, 10,  1},
	{ 4, 10,  1},
	{ 6, 10,  1},
	{ 8, 9,  1},
	{ 10, 9,  1},
	{ 12, 9,  1},
	{ 14, 10,  1},
	{ 16, 10,  1},
	{ 18, 10,  1},
	{ 3, 8,  1},
	{ 3, 6,  1},
	{ 5, 8,  1},
	{ 5, 6,  1},
	{ 7, 7,  1},
	{ 9, 7,  1},
	{ 11, 7,  1},
	{ 13, 7,  1},
	{ 15, 8,  1},
	{ 17, 8,  1},
	{ 3, 4,  1},
	{ 5, 4,  1},
	{ 7, 5,  1},
	{ 9, 5,  1},
	{ 11, 5,  1},
	{ 13, 5,  1},
	{ 15, 6,  1},
	{ 17, 6,  1},
	{ 2, 2,  1},
	{ 4, 2,  1},
	{ 6, 2,  1},
	{ 8, 3,  1},
	{ 10, 3,  1},
	{ 12, 3,  1},
	{ 15, 4,  1},
	{ 17, 4,  1},
	{ 7, 0,  1},
	{ 9, 1,  1},
	{ 11, 1,  1},
	{ 14, 2,  1},
	{ 16, 2,  1},
	{ 18, 2,  1},
	{ 13, 0,  1},
	{ 6, 12,  0},
	{ 8, 12,  0},
	{ 10, 12,  0},
	{ 12, 12,  0},
	{ 14, 12,  0},
	{ 1, 11,  0},
	{ 3, 11,  0},
	{ 1, 9,  0},
	{ 0, 6,  0},
	{ 3, 9,  0},
	{ 5, 10,  0},
	{ 7, 10,  0},
	{ 9, 10,  0},
	{ 11, 10,  0},
	{ 13, 10,  0},
	{ 15, 10,  0},
	{ 17, 11,  0},
	{ 19, 11,  0},
	{ 2, 7,  0},
	{ 4, 7,  0},
	{ 6, 8,  0},
	{ 8, 8,  0},
	{ 2, 5,  0},
	{ 4, 5,  0},
	{ 6, 6,  0},
	{ 8, 6,  0},
	{ 10, 8,  0},
	{ 10, 6,  0},
	{ 12, 8,  0},
	{ 12, 6,  0},
	{ 14, 8,  0},
	{ 14, 6,  0},
	{ 17, 9,  0},
	{ 16, 7,  0},
	{ 19, 9,  0},
	{ 18, 7,  0},
	{ 1, 3,  0},
	{ 3, 3,  0},
	{ 6, 4,  0},
	{ 8, 4,  0},
	{ 10, 4,  0},
	{ 12, 4,  0},
	{ 14, 4,  0},
	{ 16, 5,  0},
	{ 18, 5,  0},
	{ 20, 6,  0},
	{ 1, 1,  0},
	{ 3, 1,  0},
	{ 5, 2,  0},
	{ 7, 2,  0},
	{ 9, 2,  0},
	{ 11, 2,  0},
	{ 13, 2,  0},
	{ 15, 2,  0},
	{ 17, 3,  0},
	{ 19, 3,  0},
	{ 17, 1,  0},
	{ 19, 1,  0},
	{ 6, 0,  0},
	{ 8, 0,  0},
	{ 10, 0,  0},
	{ 12, 0,  0},
	{ 14, 0,  0}
};

tilepos *pos = hard_map ;
int items_created = 0;
int xpos_offset;
int ypos_offset;

tile tiles[MAX_TILES];

GtkWidget *window, *pref_dialog, *hint_dialog;
GtkWidget *mbox;
GtkWidget *canvas;
GtkWidget *tiles_label;
GdkPixmap *tiles_pix, *mask;
int selected_tile, visible_tiles;
int sequence_number;

static GdkImlibImage *tiles_image;
static gchar *tileset = 0 ;

static struct {
  char *tileset;
  int make_it_default;
} selected_tileset = {0,0};

static gchar *mapset = 0 ;

static struct {
  char *mapset ;
  int make_it_default;
} selected_mapset = {0,0} ;

static struct {
  GdkColor colour ;
  char *name ;
  int set;
} backgnd = {{0,0,0,0},NULL,0} ;

struct _maps
{
  char *name ;
  tilepos *map ;
  int make_it_default ;
} maps[] = { { "easy",      easy_map },
	     { "difficult", hard_map } } ;

gint hint_tiles[2];
guint timer;
guint timeout_counter = 0;

void clear_undo_queue ();
void you_won (void);
void no_match (void);
void check_free (void);
void set_map (char *name) ;
void load_tiles (char *fname);
void quit_game_callback (GtkWidget *widget, gpointer data);
void new_game_callback (GtkWidget *widget, gpointer data);
void restart_game_callback (GtkWidget *widget, gpointer data);
void undo_tile_callback (GtkWidget *widget, gpointer data);
void redo_tile_callback (GtkWidget *widget, gpointer data);
void select_game_callback (GtkWidget *widget, gpointer date);
void new_game (void);
void hint_callback (GtkWidget *widget, gpointer data);
void properties_callback (GtkWidget *widget, gpointer data);
void about_callback (GtkWidget *widget, gpointer data);
void show_tb_callback (GtkWidget *widget, gpointer data);
void sound_on_callback (GtkWidget *widget, gpointer data);


GnomeUIInfo filemenu [] = {
         {GNOME_APP_UI_ITEM, N_("_New"), NULL, new_game_callback, NULL, NULL,
         GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_NEW, 'n', GDK_CONTROL_MASK, NULL},

         {GNOME_APP_UI_ITEM, N_("New _Seed..."), NULL, select_game_callback, NULL, NULL,
         GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_NEW, 0, 0, NULL},

         {GNOME_APP_UI_ITEM, N_("_Restart"), NULL, restart_game_callback, NULL, NULL,
         GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_REFRESH, 0, 0, NULL},

         {GNOME_APP_UI_SEPARATOR},

         {GNOME_APP_UI_ITEM, N_("_Hint"), NULL, hint_callback, NULL, NULL,
         GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},

         {GNOME_APP_UI_ITEM, N_("_Undo"), NULL, undo_tile_callback, NULL, NULL,
         GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_UNDO, 'z', GDK_CONTROL_MASK, NULL},

         {GNOME_APP_UI_SEPARATOR},
	
         {GNOME_APP_UI_ITEM, N_("E_xit"), NULL, quit_game_callback, NULL, NULL,
         GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_EXIT, 'q', GDK_CONTROL_MASK, NULL},

         {GNOME_APP_UI_ENDOFINFO}
};

GnomeUIInfo optionsmenu [] = {
	{GNOME_APP_UI_TOGGLEITEM, N_("Show _Tool Bar"), NULL, show_tb_callback, NULL, NULL,
	 GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},

        {GNOME_APP_UI_SEPARATOR},

        {GNOME_APP_UI_TOGGLEITEM, N_("_Sound"), NULL, NULL, NULL, NULL,
        GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},

        {GNOME_APP_UI_ITEM, N_("_Properties..."), NULL, properties_callback, NULL, NULL,
        GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_PROP, 0, 0, NULL},

        
        {GNOME_APP_UI_ENDOFINFO}
};

GnomeUIInfo helpmenu[] = {
/* 	{GNOME_APP_UI_HELP, NULL, NULL, NULL, NULL, NULL, */
/*         GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL}, */
	
	{GNOME_APP_UI_ITEM, N_("_About Mahjongg"), NULL, about_callback, NULL, NULL,
	 GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_ABOUT, 0, 0, NULL},
	
	{GNOME_APP_UI_ENDOFINFO}
};

GnomeUIInfo mainmenu [] = {
	{GNOME_APP_UI_SUBTREE, N_("_Game"), NULL, filemenu, NULL, NULL,
	 GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},

         {GNOME_APP_UI_SUBTREE, N_("_Options"), NULL, optionsmenu, NULL, NULL,
	 GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
	
	{GNOME_APP_UI_SUBTREE, N_("_Help"), NULL, helpmenu, NULL, NULL,
	 GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
	
	{GNOME_APP_UI_ENDOFINFO}
};

GnomeUIInfo toolbar [] = {
	{GNOME_APP_UI_ITEM, N_("New"), NULL, new_game_callback, NULL, NULL,
         GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_PIXMAP_NEW, 0, 0, NULL},

	{GNOME_APP_UI_ITEM, N_("New Seed"), NULL, select_game_callback, NULL, NULL,
         GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_PIXMAP_NEW, 0, 0, NULL},

         {GNOME_APP_UI_ITEM, N_("Restart"), NULL, restart_game_callback, NULL, NULL,
         GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_PIXMAP_REFRESH, 0, 0, NULL},

         {GNOME_APP_UI_ITEM, N_("Hint"), NULL, hint_callback, NULL, NULL,
         GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_PIXMAP_HELP, 0, 0, NULL},

         {GNOME_APP_UI_ITEM, N_("Undo"), NULL, undo_tile_callback, NULL, NULL,
         GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_PIXMAP_UNDO, 0, 0, NULL},

         {GNOME_APP_UI_ITEM, N_("Re-do"), NULL, redo_tile_callback, NULL, NULL,
         GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_PIXMAP_REDO, 0, 0, NULL},

         {GNOME_APP_UI_TOGGLEITEM, N_("Sound"), NULL, sound_on_callback, NULL, NULL,
         GNOME_APP_PIXMAP_DATA, mini_sound_xpm, 0, 0, NULL},

         {GNOME_APP_UI_ITEM, N_("Properties"), NULL, properties_callback, NULL, NULL,
         GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_PIXMAP_PROPERTIES, 0, 0, NULL},

         {GNOME_APP_UI_SEPARATOR},

	{GNOME_APP_UI_ENDOFINFO}
};

static void
pref_cancel (GtkWidget *widget, void *data)
{
	gtk_widget_destroy (pref_dialog);
	pref_dialog = 0;
}

static void
hint_cancel (GtkWidget *widget, void *data)
{
  int lp ;
  for (lp=0;lp<MAX_TILES;lp++)
    if (tiles[lp].selected == 17)
      {
	tiles[lp].selected = 0 ;
/* 	redraw_tile(lp) ; */
      }

  gtk_widget_destroy (hint_dialog);
  hint_dialog = 0;
}

static void
set_tile_selection (GtkWidget *widget, void *data)
{
	selected_tileset.tileset = data;
}

static void
set_tile_selection_def (GtkWidget *widget, gpointer *data)
{
	selected_tileset.make_it_default = GTK_TOGGLE_BUTTON (widget)->active;
}

static void
set_map_selection (GtkWidget *widget, void *data)
{
	selected_mapset.mapset = data;
}

static void
set_map_selection_def (GtkWidget *widget, gpointer *data)
{
	selected_mapset.make_it_default = GTK_TOGGLE_BUTTON (widget)->active;
}

static void
free_str (GtkWidget *widget, void *data)
{
	free (data);
}



void set_backgnd_colour (char *str)
{
  GdkColormap *colourmap ;
  GtkStyle *widget_style, *temp_style;

  if (str != backgnd.name)
    {
      if (backgnd.name)
	free (backgnd.name) ;
      backgnd.name = strdup (str) ;
    }
  colourmap = gtk_widget_get_colormap(canvas);
  gdk_color_parse (backgnd.name, &backgnd.colour);

  gdk_color_alloc(colourmap, &backgnd.colour);

  widget_style = gtk_widget_get_style (canvas);
  temp_style = gtk_style_copy (widget_style);
  temp_style->bg[0] = backgnd.colour;
  temp_style->bg[1] = backgnd.colour;
  temp_style->bg[2] = backgnd.colour;
  temp_style->bg[3] = backgnd.colour;
  temp_style->bg[4] = backgnd.colour;
  gtk_widget_set_style (canvas, temp_style);
  gnome_canvas_update_now (GNOME_CANVAS(canvas));
}

static void
change_tile_image (tile *tile_inf) {
	GdkImlibImage *new_image;
	gint orig_x, orig_y;

	orig_x = (tile_inf->image % 21) * TILE_WIDTH;
	orig_y = (tile_inf->image / 21) * TILE_HEIGHT;

	if ((tile_inf->selected == 1) || (tile_inf->selected == 17)) {
		orig_y += 2 * TILE_HEIGHT;
	}

	gdk_imlib_destroy_image (tile_inf->current_image);
	
	tile_inf->current_image = new_image = gdk_imlib_crop_and_clone_image (tiles_image, orig_x, orig_y, TILE_WIDTH, TILE_HEIGHT);

	gnome_canvas_item_set (tile_inf->image_item, "image", new_image, NULL);
}

static void
tile_event (GnomeCanvasItem *item, GdkEvent *event, tile *tile_inf)
{
  gchar tmpchar[4];
  
  switch(event->type) {
	  case GDK_BUTTON_PRESS :
	    if((event->button.button == 1) && tile_free(tile_inf->number)) {
	      if (tile_inf->selected == 1) {
		selected_tile = MAX_TILES + 1;
		tile_inf->selected = 0;
		change_tile_image (tile_inf);
	      }
	      else {
		if (selected_tile < MAX_TILES) {
		  if ((tiles[selected_tile].type == tile_inf->type) && tile_free (tile_inf->number)) {
		    tiles[selected_tile].visible = 0;
		    tile_inf->visible = 0;
		    tiles[selected_tile].selected = 0;
		    change_tile_image (&tiles[selected_tile]);
		    gnome_canvas_item_hide (tiles[selected_tile].image_item);
		    gnome_canvas_item_hide (tile_inf->image_item);
		    clear_undo_queue ();
		    tiles[selected_tile].sequence = tile_inf->sequence = sequence_number;
		    sequence_number ++;
		    selected_tile = MAX_TILES + 1;
		    visible_tiles -= 2;
		    sprintf(tmpchar, "%d", visible_tiles);
		    gtk_label_set (GTK_LABEL(tiles_label), tmpchar);
		    check_free();
		    if (visible_tiles <= 0)
		      you_won ();
		  }
		  else
		    no_match ();
		}
		else if (tile_free(tile_inf->number)) {
		  tile_inf->selected = 1;
		  change_tile_image(tile_inf);
		  selected_tile = tile_inf->number;
		}
	      }
	    }
	    break;
	  default :
	    break;
  }
}

static void
load_callback (GtkWidget *widget, void *data)
{
  int redraw = 0, sync = 0 ;
  if (selected_tileset.tileset)
    {
      load_tiles (selected_tileset.tileset);
      redraw = 1 ;
      if (selected_tileset.make_it_default)
	{
	  gnome_config_set_string ("/gmahjongg/Preferences/tileset", 
				   selected_tileset.tileset);
	  sync = 1 ;
	}
      selected_tileset.tileset = 0 ;
    }
  if (selected_mapset.mapset)
    {
      set_map (selected_mapset.mapset) ;
      redraw = 1 ;
      if (selected_mapset.make_it_default)
	{
	  gnome_config_set_string ("/gmahjongg/Preferences/mapset", 
				   selected_mapset.mapset);
	  sync = 1 ;
	}
      selected_mapset.mapset = 0 ;
    }
  if (backgnd.set)
    {
      set_backgnd_colour (backgnd.name) ;
      gnome_config_set_string ("/gmahjongg/Preferences/bcolour",
			       backgnd.name) ;
      sync = 1 ;
      backgnd.set = 0 ;
    }
  if (sync)
    {
    gnome_config_sync();
    printf ("Synced\n") ;
    }
  if (redraw)
    gnome_canvas_update_now(GNOME_CANVAS(canvas));
  pref_cancel (0,0);
}

static void
fill_tile_menu (GtkWidget *menu)
{
	struct dirent *e;
	char *dname = gnome_unconditional_pixmap_file ("mahjongg");
	DIR *dir;
        int itemno = 0;
	
	dir = opendir (dname);

	if (!dir)
		return;
	
	while ((e = readdir (dir)) != NULL){
		GtkWidget *item;
		char *s = strdup (e->d_name);

		if (!(strstr (e->d_name, ".xpm") ||
		      strstr (e->d_name, ".gif") ||
		      strstr (e->d_name, ".png") ||
		      strstr (e->d_name, ".jpg") ||
		      strstr (e->d_name, ".xbm"))){
			free (s);
			continue;
		}
			
		item = gtk_menu_item_new_with_label (s);
		gtk_widget_show (item);
		gtk_menu_append (GTK_MENU(menu), item);
 		gtk_signal_connect (GTK_OBJECT(item), "activate",
				    (GtkSignalFunc)set_tile_selection, s); 
 		gtk_signal_connect (GTK_OBJECT(item), "destroy",
				    (GtkSignalFunc) free_str, s); 
	  
 	        if (!strcmp(tileset, s)) 
 	        { 
 		  gtk_menu_set_active(GTK_MENU(menu), itemno); 
 		} 
	  
	        itemno++;
	}
	closedir (dir);
}

static void
fill_map_menu (GtkWidget *menu)
{
  int lp, itemno=0 ;
  GtkWidget *item;

  for (lp=0;lp<sizeof(maps)/sizeof(struct _maps);lp++)
    {
      char *str = strdup (maps[lp].name) ;

      item = gtk_menu_item_new_with_label (str) ;
      gtk_widget_show (item);
      gtk_menu_append (GTK_MENU(menu), item);
      gtk_signal_connect (GTK_OBJECT(item), "activate",
			  (GtkSignalFunc)set_map_selection, str); 
      gtk_signal_connect (GTK_OBJECT(item), "destroy",
			   (GtkSignalFunc) free_str, str); 
      if (!strcasecmp(mapset, str))
	  gtk_menu_set_active(GTK_MENU(menu), itemno); 
      itemno++ ;
    }
}

int find_tile_in_layer (int x, int y, int layer)
{
	int i, tile_num = MAX_TILES + 1;

	for (i = 0; i < MAX_TILES; i ++) {
		if ((tiles[i].x < x) && ((tiles[i].x + TILE_WIDTH - 1) > x)) {
			if ((tiles[i].y < y) && ((tiles[i].y + TILE_HEIGHT - 1) > y)) {
				if ((tiles[i].layer == layer) && (tiles[i].visible == 1))
					tile_num = i;
			}
		}
	}
	return tile_num;
}
	
int find_tile (int x, int y)
{
	int i, tile_num = MAX_TILES + 1, layer = 0;

	for (i = 0; i < MAX_TILES; i++) {
		if ((tiles[i].x < x) && ((tiles[i].x + TILE_WIDTH - 1) > x) && (tiles[i].visible)) {
			if ((tiles[i].y < y) && ((tiles[i].y + TILE_HEIGHT - 1) > y)) {
				if ((tiles[i].layer >= layer) && (tiles[i].visible == 1)) {
					tile_num = i;
					layer = tiles[i].layer;
				}
			}
		}
	}
	return tile_num;
}

void no_match (void)
{
	GtkWidget *mb;

	mb = gnome_message_box_new (_("Tiles don't match!"),
				   GNOME_MESSAGE_BOX_INFO,
				   _("Ok"), NULL);
	GTK_WINDOW(mb)->position = GTK_WIN_POS_MOUSE;
	gtk_window_set_modal (&GNOME_MESSAGE_BOX(mb)->dialog.window, TRUE);
	gtk_widget_show (mb);
}

void check_free (void)
{
	int i, j, type, free = 0;
	GtkWidget *mb;
	
	/* Tile Free is now _so_ much quicker, it is more elegant to do a
	 * British Library search, and safer. */
	for (i=0;i<MAX_TILES && !free;i++)
	    if (tile_free(i))
	    {
		type = tiles[i].type ;
		for (j=0;j<MAX_TILES && !free;j++)
		  free = (i != j && tiles[j].type == type && tile_free(j)) ;
	    }
 	if (!free && visible_tiles>0) { 
 		mb = gnome_message_box_new (_("No more movements"), 
 					    GNOME_MESSAGE_BOX_INFO, 
 					    _("Ok"), NULL); 
 		GTK_WINDOW(mb)->position = GTK_WIN_POS_MOUSE; 
		gtk_window_set_modal (&GNOME_MESSAGE_BOX(mb)->dialog.window, TRUE);
 		gtk_widget_show (mb); 
 	} 
}

void you_won (void)
{
	GtkWidget *mb;

	mb = gnome_message_box_new (_("You won!"),
				   GNOME_MESSAGE_BOX_INFO,
				   _("Ok"), NULL);
	GTK_WINDOW(mb)->position = GTK_WIN_POS_MOUSE;
	gnome_dialog_set_modal (GNOME_DIALOG (mb));
	gtk_signal_connect_object (GTK_OBJECT(mb),
				   "clicked",
				   GTK_SIGNAL_FUNC (new_game_callback),
				   NULL);
	gtk_widget_show (mb);
}

void colour_changed_cb (GtkWidget *w, int r1, int g1, int b1, int a1, 
			gchar ** color)
{
  static char tmp[24] = "" ;
  guint8 r,g,b,a;

  gnome_color_picker_get_i8(GNOME_COLOR_PICKER(w), &r, &g, &b, &a);
  
  sprintf( tmp, "#%02x%02x%02x", r, g, b ) ;

  *color = tmp ;
  backgnd.set = 1 ;
}          

void properties_callback (GtkWidget *widget, gpointer data)
{
	GtkDialog *d;
	GtkWidget *button;
	GtkWidget *tmenu, *mmenu, *otmenu, *ommenu, *l, *hb, *cb, *f, *fv;
	GtkWidget *bcolour_gcs ;

	if (pref_dialog)
		return;
	
	pref_dialog = gtk_dialog_new ();
	d = GTK_DIALOG(pref_dialog);
 	gtk_signal_connect (GTK_OBJECT(pref_dialog), "delete_event",
			    (GtkSignalFunc)pref_cancel, NULL); 

	/* The Tile sub-menu */
	otmenu = gtk_option_menu_new ();
	tmenu = gtk_menu_new ();
	fill_tile_menu (tmenu);
	gtk_widget_show (otmenu);
	gtk_option_menu_set_menu (GTK_OPTION_MENU(otmenu), tmenu);

	f = gtk_frame_new (_ ("Tiles"));
	gtk_container_border_width (GTK_CONTAINER (f), 5);

	hb = gtk_hbox_new (FALSE, FALSE);
	gtk_widget_show (hb);
	
	l = gtk_label_new (_("Select Tiles:"));
	gtk_widget_show (l);
	    
	gtk_box_pack_start_defaults (GTK_BOX(hb), l);
	gtk_box_pack_start_defaults (GTK_BOX(hb), otmenu);

	cb = gtk_check_button_new_with_label ( _("Make it the default tile set") );
 	gtk_signal_connect (GTK_OBJECT(cb), "clicked",
			    (GtkSignalFunc)set_tile_selection_def, NULL);
	gtk_widget_show (cb);

	fv = gtk_vbox_new (0, 5);
	gtk_container_border_width (GTK_CONTAINER (fv), 5);
	gtk_widget_show (fv);
	
	gtk_box_pack_start_defaults (GTK_BOX(fv), hb);
	gtk_box_pack_start_defaults (GTK_BOX(fv), cb);
	gtk_box_pack_start_defaults (GTK_BOX(d->vbox), f);
	gtk_container_add (GTK_CONTAINER (f), fv);
	gtk_widget_show (f);

	/* The Map sub-menu */
	ommenu = gtk_option_menu_new ();
	mmenu = gtk_menu_new ();
	fill_map_menu (mmenu);
	gtk_widget_show (ommenu);
	gtk_option_menu_set_menu (GTK_OPTION_MENU(ommenu), mmenu);

	f = gtk_frame_new (_ ("Maps"));
	gtk_container_border_width (GTK_CONTAINER (f), 5);

	hb = gtk_hbox_new (FALSE, FALSE);
	gtk_widget_show (hb);
	
	l = gtk_label_new (_("Select Map:"));
	gtk_widget_show (l);
	    
	gtk_box_pack_start_defaults (GTK_BOX(hb), l);
	gtk_box_pack_start_defaults (GTK_BOX(hb), ommenu);

	cb = gtk_check_button_new_with_label ( _("Make it the default map") );
 	gtk_signal_connect (GTK_OBJECT(cb), "clicked",
			    (GtkSignalFunc)set_map_selection_def, NULL);
	gtk_widget_show (cb) ;

	fv = gtk_vbox_new (0, 5);
	gtk_container_border_width (GTK_CONTAINER (fv), 5);
	gtk_widget_show (fv);
	
	gtk_box_pack_start_defaults (GTK_BOX(fv), hb);
	gtk_box_pack_start_defaults (GTK_BOX(fv), cb);
	gtk_box_pack_start_defaults (GTK_BOX(d->vbox), f);
	gtk_container_add (GTK_CONTAINER (f), fv);
	gtk_widget_show (f);

	f = gtk_vbox_new (0, 5) ;
	gtk_container_border_width (GTK_CONTAINER (f), 5);

	{
	  int ur,ug,ub ;
	  bcolour_gcs  = gnome_color_picker_new();
	  sscanf( backgnd.name, "#%02x%02x%02x", &ur,&ug,&ub );
	  gnome_color_picker_set_i8( GNOME_COLOR_PICKER(bcolour_gcs), ur, 
				     ug, ub, 0);
	  gtk_signal_connect(GTK_OBJECT(bcolour_gcs), "color_set", 
			GTK_SIGNAL_FUNC(colour_changed_cb), &backgnd.name);
	}

	l = gtk_label_new (_("Background colour")) ;
	gtk_box_pack_start_defaults (GTK_BOX(f), l) ;
	gtk_box_pack_start_defaults (GTK_BOX(f), bcolour_gcs);
	gtk_widget_show(l) ;
	gtk_box_pack_start_defaults (GTK_BOX(d->vbox), f) ;
	gtk_widget_show(f) ;
	
	/* Misc bottom buttons */
        button = gnome_stock_button(GNOME_STOCK_BUTTON_OK);
	 	gtk_signal_connect(GTK_OBJECT(button), "clicked", 
 			   GTK_SIGNAL_FUNC(load_callback), NULL); 
	gtk_box_pack_start(GTK_BOX(d->action_area), button, TRUE, TRUE, 5);
        gtk_widget_show(button);
        button = gnome_stock_button(GNOME_STOCK_BUTTON_CANCEL);
 	gtk_signal_connect(GTK_OBJECT(button), "clicked", 
			   (GtkSignalFunc)pref_cancel,
 			   (gpointer)1);
	gtk_box_pack_start(GTK_BOX(d->action_area), button, TRUE, TRUE, 5);
        gtk_widget_show(button);

        gtk_widget_show_all (pref_dialog);
	
}

gint hint_timeout (gpointer data)
{
  if (timeout_counter <= 4) {
    if (tiles[hint_tiles[0]].selected == 17) {
      tiles[hint_tiles[0]].selected = 0;
      tiles[hint_tiles[1]].selected = 0;
    }
    else {
      tiles[hint_tiles[0]].selected = 17;
      tiles[hint_tiles[1]].selected = 17;
    }
    change_tile_image(&tiles[hint_tiles[0]]);
    change_tile_image(&tiles[hint_tiles[1]]);
  }
  else {
    gtk_timeout_remove (timer);
  }
  timeout_counter ++;
  return 1;
}

void hint_callback (GtkWidget *widget, gpointer data)
{
  int i, j, free=0, type ;

  if (hint_dialog)
    return;

  /* Snarfed from check free
   * Tile Free is now _so_ much quicker, it is more elegant to do a
   * British Library search, and safer. */

  if (selected_tile < MAX_TILES) {
	  tiles[selected_tile].selected = 0;
  }
  selected_tile = MAX_TILES + 1;
  
  for (i=0;i<MAX_TILES && !free;i++)
    if (tile_free(i))
      {
	type = tiles[i].type ;
	for (j=0;j<MAX_TILES && !free;j++)
	  {
	    free = (tiles[j].type == type && i != j && tile_free(j)) ;
	    if (free)
	      {
		if (!tiles[i].selected)
		  tiles[i].selected = 17 ;
		if (!tiles[j].selected)
		  tiles[j].selected = 17 ;
		change_tile_image (&tiles[i]);
		change_tile_image (&tiles[j]);
		hint_tiles[0] = i;
		hint_tiles[1] = j;
	      }
	  }
      }
  /* This is a good way to test check_free
    for (i=0;i<MAX_TILES;i++)
    if (tiles[i].selected == 17)
    tiles[i].visible = 0 ;*/

  timeout_counter = 0;
  timer = gtk_timeout_add (250, (GtkFunction) hint_timeout, NULL);
}

void about_callback (GtkWidget *widget, gpointer data)
{
	GtkWidget *about;
	const gchar *authors [] = {
		"Code: Francisco Bustamante",
		"      Max Watson",
		"      Heinz Hempe",
		"      Michael Meeks",
		"Tiles: Jonathan Buzzard",
		"       Max Watson",
		NULL
	};

	about = gnome_about_new (_("Gnome Mahjongg"), MAH_VERSION,
				 "(C) 1998 The Free Software Foundation",
				 (const char **)authors,
				 _("Send comments and bug reports to:\n"
				   "        pancho@nuclecu.unam.mx or\n"
				   "        michael@imaginator.com\n\n"
				   "Tiles under the General Public License."),
				 NULL);
	gtk_widget_show (about);
}

void quit_game_callback (GtkWidget *widget, gpointer data)
{
    gnome_config_sync();
    gtk_main_quit ();
}

void new_game_callback (GtkWidget *widget, gpointer data)
{
    gtk_label_set(GTK_LABEL(tiles_label), MAX_TILES_STR);
    new_game ();
}

void restart_game_callback (GtkWidget *widget, gpointer data)
{
    int i;
    
    visible_tiles = MAX_TILES;
    gtk_label_set(GTK_LABEL(tiles_label), MAX_TILES_STR);
    for (i = 0; i < MAX_TILES; i++) {
        tiles[i].visible = 1;
        tiles[i].selected = 0;
	tiles[i].sequence = 0;
	gnome_canvas_item_show (tiles[i].image_item);
    }
    selected_tile=MAX_TILES+1;
    sequence_number = 1 ;
    gnome_canvas_update_now(GNOME_CANVAS(canvas));
}

void redo_tile_callback (GtkWidget *widget, gpointer data)
{
    int i, change ;
    gchar tmpchar[4] ;
    
    if (sequence_number>(MAX_TILES/2))
      return ;

    if (selected_tile<MAX_TILES) 
      {
	tiles[selected_tile].selected = 0 ;
	change_tile_image (&tiles[i]);
	selected_tile = MAX_TILES + 1; 
      }
    change = 0 ;
    for (i=0; i<MAX_TILES; i++)
      if (tiles[i].sequence == sequence_number)
	{
	  tiles[i].selected = 0 ;
	  tiles[i].visible = 0 ;
	  gnome_canvas_item_hide (tiles[i].image_item);
	  visible_tiles-- ;
	  change = 1 ;
	}
    if (change)
      if (sequence_number<MAX_TILES)
	sequence_number++ ;

    sprintf(tmpchar,"%3d",visible_tiles) ;
    gtk_label_set(GTK_LABEL(tiles_label), tmpchar);
    gnome_canvas_update_now (GNOME_CANVAS(canvas));
}

void undo_tile_callback (GtkWidget *widget, gpointer data)
{
    int i;
    gchar tmpchar[4] ;

    if (sequence_number>1)
      sequence_number-- ;
    else
      return ;

    if (selected_tile<MAX_TILES) 
      {
	tiles[selected_tile].selected = 0 ;
	change_tile_image (&tiles[i]);
	selected_tile = MAX_TILES + 1; 
      }

    for (i=0; i<MAX_TILES; i++)
      if (tiles[i].sequence == sequence_number)
	{
	  tiles[i].selected = 0 ;
	  tiles[i].visible = 1 ;
	  visible_tiles++ ;
	  gnome_canvas_item_show (tiles[i].image_item);
	}

    sprintf(tmpchar,"%d",visible_tiles) ;
    gtk_label_set(GTK_LABEL(tiles_label), tmpchar);
    gnome_canvas_update_now (GNOME_CANVAS(canvas));
}

static void
input_callback (GtkWidget *widget, gpointer data)
{
	srand (atoi (GTK_ENTRY (data)->text));
	new_game ();
}

static void
cancel_callback (GtkWidget *widget, gpointer data)
{
	gtk_widget_destroy (GTK_WIDGET (data));
}

void select_game_callback (GtkWidget *widget, gpointer data)
{
	GtkWidget *dialog, *entry, *label, *button, *vbox;

	dialog = gtk_dialog_new ();
	GTK_WINDOW (dialog)->position = GTK_WIN_POS_MOUSE;
	gtk_window_set_title (GTK_WINDOW (dialog), _("Select Game"));
	gtk_signal_connect (GTK_OBJECT (dialog), "delete_event",
			    GTK_SIGNAL_FUNC (cancel_callback),
			    (gpointer)dialog);
	label = gtk_label_new (_("Game Number:"));
	gtk_box_pack_start_defaults (GTK_BOX (GTK_DIALOG (dialog)->vbox), label);
	gtk_widget_show (label);
	
	entry = gtk_entry_new ();
	gtk_box_pack_start_defaults (GTK_BOX(GTK_DIALOG(dialog)->vbox), entry);
	gtk_widget_show (entry);

	vbox = gtk_hbox_new (TRUE, 4);
	gtk_container_add (GTK_CONTAINER(GTK_DIALOG(dialog)->action_area), vbox);
  
	button = gnome_stock_button (GNOME_STOCK_BUTTON_OK);
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
			    GTK_SIGNAL_FUNC (input_callback),
			    (gpointer)entry);
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
			    GTK_SIGNAL_FUNC (cancel_callback),
			    (gpointer)dialog);
	gtk_box_pack_start_defaults (GTK_BOX(vbox), button);
	gtk_widget_show(button);
  
	button = gnome_stock_button (GNOME_STOCK_BUTTON_CANCEL);
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
			    GTK_SIGNAL_FUNC (cancel_callback),
			    (gpointer)dialog);
	gtk_box_pack_start_defaults (GTK_BOX(vbox), button);
	gtk_widget_show(button);
  
	gtk_widget_show(vbox);
  
	gtk_grab_add (dialog);
	gtk_widget_show (dialog);
	
}

void show_tb_callback (GtkWidget *widget, gpointer data)
{
    if((GTK_CHECK_MENU_ITEM(optionsmenu[0].widget))->active)
    {
        gnome_config_set_bool("gmahjongg/toolbar/show", TRUE);
        gtk_widget_show(GTK_WIDGET(GNOME_APP(window)->toolbar->parent));
    }
    else
    {
        gnome_config_set_bool("gmahjongg/toolbar/show", FALSE);
        gtk_widget_hide(GTK_WIDGET(GNOME_APP(window)->toolbar->parent));
    }
}

void sound_on_callback (GtkWidget *widget, gpointer data)
{
    printf("mer\n");
}

/* You loose your re-do queue when you make a move */
void clear_undo_queue ()
{
  int lp ;
  for (lp=0;lp<MAX_TILES;lp++)
    if (tiles[lp].sequence>=sequence_number)
      tiles[lp].sequence = 0 ;
}

void set_map (char *name)
{
  int lp ;
  int xmax = 0, ymax = 0;
  tilepos *t;
  int i;

  for (lp=0;lp<sizeof(maps)/sizeof(struct _maps);lp++)
    if (strcasecmp (maps[lp].name, name) == 0)
      {
	pos = maps[lp].map ;

	for ( t = pos ; t < pos + MAX_TILES ; t++ ) {
	    if ( (*t).x  > xmax ) xmax = (*t).x;
	    if ( (*t).y  > ymax ) ymax = (*t).y;
	}
	xpos_offset = ( AREA_WIDTH - (HALF_WIDTH * (xmax+1)) ) / 2;
	ypos_offset = ( AREA_HEIGHT - (HALF_HEIGHT * (ymax+1) ) ) / 2;

	generate_dependancies() ;
	if (mapset)
	  g_free (mapset);
	mapset = g_strdup(name);
      }
  new_game ();
  if (items_created) {
	  for (i = MAX_TILES - 1; i >= 0; i --) {
		  gnome_canvas_item_set (tiles[i].image_item,
					 "x", (double)tiles[i].x,
					 "y", (double)tiles[i].y,
					 NULL);
	  }
  }
}

void create_canvas_items (void)
{
  GdkImlibImage *image;
  gint orig_x, orig_y, i, layer, max_x, tile_was_set = 0;
  
  for (layer = 0; layer < 7; layer ++) {
    for (i = MAX_TILES - 1; i >= 0; i --) {
      if (tiles[i].layer == layer) {
	tiles[i].canvas_item = gnome_canvas_item_new (gnome_canvas_root(GNOME_CANVAS(canvas)),
						      gnome_canvas_group_get_type (),
						      NULL);
	
	orig_x = (tiles[i].image % 21) * TILE_WIDTH;
	orig_y = (tiles[i].image / 21) * TILE_HEIGHT;
	
	tiles[i].number = i;
	
	tiles[i].current_image = image = gdk_imlib_crop_and_clone_image (tiles_image, orig_x, orig_y, TILE_WIDTH, TILE_HEIGHT);
	
	tiles[i].image_item = gnome_canvas_item_new (GNOME_CANVAS_GROUP (tiles[i].canvas_item),
						      gnome_canvas_image_get_type(),
						      "image", image,
						      "x", (double)tiles[i].x,
						      "y", (double)tiles[i].y,
						      "width", (double)TILE_WIDTH,
						      "height", (double)TILE_HEIGHT,
						      "anchor", GTK_ANCHOR_NW,
						      NULL);
	
	gtk_signal_connect (GTK_OBJECT (tiles[i].canvas_item), "event",
			    (GtkSignalFunc) tile_event,
			    &tiles[i]);
      }
    }
  }
  items_created = 1;
}

void load_tiles (char *fname)
{
	char *tmp, *fn;
	int i;

	tmp = g_copy_strings ("mahjongg/", fname, NULL);

	fn = gnome_unconditional_pixmap_file (tmp);
	g_free (tmp);

	if (!g_file_exists (fn)) {
		printf ("Could not find file \'%s\'\n", fn);
		exit (1);
	}

	if (tileset)
		g_free (tileset);

	tileset = g_strdup(fname);
	
	if (tiles_image)
		gdk_imlib_destroy_image (tiles_image);

	tiles_image = gdk_imlib_load_image (fn);
	gdk_imlib_render (tiles_image, tiles_image->rgb_width, tiles_image->rgb_height);
	
	tiles_pix = gdk_imlib_move_image (tiles_image);

	if (items_created) {
		for (i = 0; i < MAX_TILES; i++) {
			change_tile_image (&tiles[i]);
		}
	}
		

	g_free (fn);
}

void create_mahjongg_board (void)
{
 	GtkStyle *style;
	gchar *buf;
	
	gtk_widget_push_visual (gdk_imlib_get_visual ());
	gtk_widget_push_colormap (gdk_imlib_get_colormap ());

	canvas = gnome_canvas_new();

	gtk_widget_pop_colormap ();
	gtk_widget_pop_visual ();
	
	gtk_box_pack_start_defaults (GTK_BOX (mbox), canvas);

	gnome_canvas_set_size(GNOME_CANVAS(canvas), AREA_WIDTH, AREA_HEIGHT);
	gnome_canvas_set_pixels_per_unit (GNOME_CANVAS(canvas), 1);
	gnome_canvas_set_scroll_region(GNOME_CANVAS(canvas), 0, 0, AREA_WIDTH, AREA_HEIGHT);
	
	gtk_widget_show (canvas);

	buf = gnome_config_get_string_with_default ("/gmahjongg/Preferences/mapset=easy", NULL);
	set_map (buf);

	buf = gnome_config_get_string_with_default ("/gmahjongg/Preferences/tileset=default.png", NULL);
	load_tiles (buf);
	new_game ();
	create_canvas_items ();

	buf = gnome_config_get_string_with_default ("/gmahjongg/Preferences/bcolour=#003010", NULL) ;
	set_backgnd_colour (buf) ;

	gnome_canvas_update_now(GNOME_CANVAS(canvas));
	g_free (buf);
	new_game ();
}

#define ELEMENTS(x) (sizeof (x) / sizeof (x [0]))

void new_game ()
{
  int i;
  
  sequence_number = 1 ;
  visible_tiles = MAX_TILES;
  selected_tile = MAX_TILES + 1;

  generate_game() ;

  if (items_created) {
    for (i = 0; i < MAX_TILES; i++) {
      change_tile_image (&tiles[i]);
      gnome_canvas_item_show (tiles[i].image_item);
    }
  }
  
  gnome_canvas_update_now (GNOME_CANVAS(canvas));
}

int main (int argc, char *argv [])
{
	bindtextdomain (PACKAGE, GNOMELOCALEDIR);
	textdomain (PACKAGE);

	gnome_init ("mahjongg", MAH_VERSION, argc, argv);

	srand (time (NULL));
	
	generate_dependancies () ;

	window = gnome_app_new ("gmahjongg", _("Gnome Mahjongg"));
	gtk_widget_realize (window);
	gtk_window_set_policy (GTK_WINDOW (window), FALSE, FALSE, TRUE);

	gnome_app_create_menus (GNOME_APP (window), mainmenu);

        gnome_app_create_toolbar (GNOME_APP (window), toolbar);
        gtk_toolbar_set_space_size(GTK_TOOLBAR(GNOME_APP(window)->toolbar), 25);
        
        if(gnome_config_get_bool("/gmahjongg/toolbar/show=TRUE"))
            gtk_check_menu_item_set_state(GTK_CHECK_MENU_ITEM(optionsmenu[0].widget), TRUE);
        else
            gtk_widget_hide(GNOME_APP(window)->toolbar->parent);

        tiles_label = gtk_label_new(_(" Tiles "));
        gtk_widget_show(tiles_label);
        gtk_toolbar_append_widget(GTK_TOOLBAR(GNOME_APP(window)->toolbar), tiles_label,
                                  NULL, NULL);
        tiles_label = gtk_label_new(_(" Remaining: "));
        gtk_widget_show(tiles_label);
        gtk_toolbar_append_widget(GTK_TOOLBAR(GNOME_APP(window)->toolbar), tiles_label,
                                  NULL, NULL);
        tiles_label = gtk_label_new(MAX_TILES_STR);
        gtk_widget_show(tiles_label);
        gtk_toolbar_append_widget(GTK_TOOLBAR(GNOME_APP(window)->toolbar), tiles_label,
                                  NULL, NULL);
        gtk_toolbar_append_space(GTK_TOOLBAR(GNOME_APP(window)->toolbar));

	gtk_signal_connect (GTK_OBJECT (window), "delete_event",
			    GTK_SIGNAL_FUNC (quit_game_callback), NULL);

	mbox = gtk_vbox_new (FALSE, 0);

	gnome_app_set_contents (GNOME_APP (window), mbox);
	create_mahjongg_board ();

	gtk_widget_show (window);
	gtk_main ();
	
	return 0;
}

