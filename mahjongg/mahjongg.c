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

/* If defined this does a very bad job of rendering
 * sequence numbers on top of the tiles as they are drawn.
 */
/* #define SEQUENCE_DEBUG */

#define GAME_EVENTS (GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK)

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
tile tiles[MAX_TILES];

GtkWidget *window, *pref_dialog, *hint_dialog;
GtkWidget *mbox;
GtkWidget *draw_area;
GtkWidget *tiles_label;
GdkPixmap *tiles_pix, *mask;
GdkGC *my_gc;
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
  int set ;
} backgnd = {0,0,0} ;

struct _maps
{
  char *name ;
  tilepos *map ;
  int make_it_default ;
} maps[] = { { "easy",      easy_map },
	     { "difficult", hard_map } } ;

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
void redraw_area (int x1, int y1, int x2, int y2, int mlayer);
void redraw_tile (int i);
void about_callback (GtkWidget *widget, gpointer data);
void show_tb_callback (GtkWidget *widget, gpointer data);
void sound_on_callback (GtkWidget *widget, gpointer data);


GnomeUIInfo filemenu [] = {
         {GNOME_APP_UI_ITEM, N_("New"), NULL, new_game_callback, NULL, NULL,
         GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_NEW, 0, 0, NULL},

         {GNOME_APP_UI_ITEM, N_("New Seed..."), NULL, select_game_callback, NULL, NULL,
         GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_NEW, 0, 0, NULL},

         {GNOME_APP_UI_ITEM, N_("Restart"), NULL, restart_game_callback, NULL, NULL,
         GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_REFRESH, 0, 0, NULL},

         {GNOME_APP_UI_SEPARATOR},

         {GNOME_APP_UI_ITEM, N_("Hint"), NULL, hint_callback, NULL, NULL,
         GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},

         {GNOME_APP_UI_ITEM, N_("Undo"), NULL, undo_tile_callback, NULL, NULL,
         GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_UNDO, 0, 0, NULL},

         {GNOME_APP_UI_SEPARATOR},
	
         {GNOME_APP_UI_ITEM, N_("Exit"), NULL, quit_game_callback, NULL, NULL,
         GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_EXIT, 0, 0, NULL},

         {GNOME_APP_UI_ENDOFINFO}
};

GnomeUIInfo optionsmenu [] = {
	{GNOME_APP_UI_TOGGLEITEM, N_("Show Tool Bar"), NULL, show_tb_callback, NULL, NULL,
	 GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},

        {GNOME_APP_UI_SEPARATOR},

        {GNOME_APP_UI_TOGGLEITEM, N_("Sound"), NULL, NULL, NULL, NULL,
        GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},

        {GNOME_APP_UI_ITEM, N_("Properties..."), NULL, properties_callback, NULL, NULL,
        GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_PROP, 0, 0, NULL},

        
        {GNOME_APP_UI_ENDOFINFO}
};

GnomeUIInfo helpmenu[] = {
	{GNOME_APP_UI_HELP, NULL, NULL, NULL, NULL, NULL,
        GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
	
	{GNOME_APP_UI_ITEM, N_("About"), NULL, about_callback, NULL, NULL,
	 GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_ABOUT, 0, 0, NULL},
	
	{GNOME_APP_UI_ENDOFINFO}
};

GnomeUIInfo mainmenu [] = {
	{GNOME_APP_UI_SUBTREE, N_("Game"), NULL, filemenu, NULL, NULL,
	 GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},

         {GNOME_APP_UI_SUBTREE, N_("Options"), NULL, optionsmenu, NULL, NULL,
	 GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
	
	{GNOME_APP_UI_SUBTREE, N_("Help"), NULL, helpmenu, NULL, NULL,
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
	redraw_tile(lp) ;
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


void clear_window ()
{
  int x, y, w, h, d ;
  gdk_window_get_geometry (draw_area->window, &x, &y, &w, &h, &d) ;
  gdk_window_clear_area(draw_area->window, 0, 0, 1000, 1000) ;
}

void set_backgnd_colour (char *str)
{
  /* backgnd.colour.red = 0;
  backgnd.colour.green = 0x3040;
  backgnd.colour.blue = 0x1030;
  backgnd.colour.pixel = 0; */

  GdkColormap *colourmap ;

  if (str != backgnd.name)
    {
      if (backgnd.name)
	free (backgnd.name) ;
      backgnd.name = strdup (str) ;
    }
  colourmap = gtk_widget_get_colormap(draw_area) ;
  gdk_color_parse (backgnd.name, &backgnd.colour) ;

  gdk_color_alloc(colourmap, &backgnd.colour) ;

  gdk_window_set_background (draw_area->window, &backgnd.colour);
  clear_window() ;
  gtk_widget_draw (draw_area, NULL);
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
    gtk_widget_draw (draw_area, NULL);
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

void draw_selected_tile (int i)
{
	redraw_area (tiles[i].x, tiles[i].y,
		     tiles[i].x + TILE_WIDTH - 1, tiles[i].y + TILE_HEIGHT - 1,
		     -1);
}

void no_match (void)
{
	GtkWidget *mb;

	mb = gnome_message_box_new (_("Tiles don't match!"),
				   GNOME_MESSAGE_BOX_INFO,
				   _("Ok"), NULL);
	GTK_WINDOW(mb)->position = GTK_WIN_POS_MOUSE;
	gnome_dialog_set_modal (GNOME_DIALOG (mb));
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
 		gnome_dialog_set_modal (GNOME_DIALOG (mb)); 
 		gtk_widget_show (mb); 
 	} 
}

void redraw_tile (int i)
{
#ifndef PLACE_DEBUG
	gdk_window_clear_area (draw_area->window, tiles[i].x, tiles[i].y,
			       TILE_WIDTH, TILE_HEIGHT);
#endif

	redraw_area (tiles[i].x, tiles[i].y, 
		     tiles[i].x + TILE_WIDTH-1,
		     tiles[i].y + TILE_HEIGHT-1,
		     0);
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

void colour_changed_cb( GnomeColorSelector *widget, gchar **color )
{
  static char tmp[24] = "" ;
  int r,g,b;

  gnome_color_selector_get_color_int(widget, &r, &g, &b, 255 );
  
  sprintf( tmp, "#%02x%02x%02x", r, g, b ) ;

  *color = tmp ;
  backgnd.set = 1 ;
}          

void properties_callback (GtkWidget *widget, gpointer data)
{
	GtkDialog *d;
	GtkWidget *button;
	GtkWidget *tmenu, *mmenu, *otmenu, *ommenu, *l, *hb, *cb, *f, *fv;
	GnomeColorSelector *bcolour_gcs ;

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
	  bcolour_gcs  = gnome_color_selector_new( (SetColorFunc)colour_changed_cb,
						   &backgnd.name) ;
	  sscanf( backgnd.name, "#%02x%02x%02x", &ur,&ug,&ub );
	  gnome_color_selector_set_color_int( bcolour_gcs, ur, ug, ub, 255) ;
	}

	l = gtk_label_new (_("Background colour")) ;
	gtk_box_pack_start_defaults (GTK_BOX(f), l) ;
	gtk_box_pack_start_defaults (GTK_BOX(f),
				     gnome_color_selector_get_button(bcolour_gcs)) ;
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

void hint_callback (GtkWidget *widget, gpointer data)
{
  GtkDialog *d;
  GtkWidget *button, *f, *t;
  int i, j, free=0, type ;

  if (hint_dialog)
    return;

  /* Snarfed from check free
   * Tile Free is now _so_ much quicker, it is more elegant to do a
   * British Library search, and safer. */
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
		redraw_tile(i) ;
		redraw_tile(j) ;
	      }
	  }
      }
  /* This is a good way to test check_free
    for (i=0;i<MAX_TILES;i++)
    if (tiles[i].selected == 17)
    tiles[i].visible = 0 ;*/
  
  hint_dialog = gtk_dialog_new ();
  d = GTK_DIALOG(hint_dialog);
  gtk_signal_connect (GTK_OBJECT(hint_dialog), "delete_event",
		      (GtkSignalFunc)hint_cancel, NULL); 

  f = gtk_vbox_new (0, 5) ;
  gtk_container_border_width (GTK_CONTAINER(f), 5) ;

    t = gtk_label_new (_("Click to continue")) ;
    gtk_box_pack_start_defaults (GTK_BOX(f), t) ;
    gtk_widget_show (t) ;

  gtk_box_pack_start_defaults (GTK_BOX(d->vbox), f) ;
  gtk_widget_show (f) ;

  /* Misc bottom buttons */
  button = gnome_stock_button(GNOME_STOCK_BUTTON_OK);
  gtk_signal_connect(GTK_OBJECT(button), "clicked", 
		     GTK_SIGNAL_FUNC(hint_cancel), NULL); 
  gtk_box_pack_start(GTK_BOX(d->action_area), button, TRUE, TRUE, 5);
  gtk_widget_show(button);
  
  gtk_widget_show (hint_dialog);
	
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
    }
    selected_tile=MAX_TILES+1;
    sequence_number = 1 ;
    gtk_widget_draw (draw_area, NULL);
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
	redraw_tile (selected_tile) ;
	selected_tile = MAX_TILES + 1; 
      }
    change = 0 ;
    for (i=0; i<MAX_TILES; i++)
      if (tiles[i].sequence == sequence_number)
	{
	  tiles[i].selected = 0 ;
	  tiles[i].visible = 0 ;
	  visible_tiles-- ;
	  redraw_tile (i);
	  change = 1 ;
	}
    if (change)
      if (sequence_number<MAX_TILES)
	sequence_number++ ;

    sprintf(tmpchar,"%3d",visible_tiles) ;
    gtk_label_set(GTK_LABEL(tiles_label), tmpchar);
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
	redraw_tile (selected_tile) ;
	selected_tile = MAX_TILES + 1; 
      }

    for (i=0; i<MAX_TILES; i++)
      if (tiles[i].sequence == sequence_number)
	{
	  tiles[i].selected = 0 ;
	  tiles[i].visible = 1 ;
	  visible_tiles++ ;
	  redraw_tile(i) ;
	}

    sprintf(tmpchar,"%d",visible_tiles) ;
    gtk_label_set(GTK_LABEL(tiles_label), tmpchar);
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

void redraw_tile_in_area (int x1, int y1, int x2, int y2, int tile)
{
	GdkRectangle trect, arect, dest;
	int orig_x, orig_y;

#ifdef SEQUENCE_DEBUG
	int n ;
	GdkFont *font ;
	char fontname[1024] ;
	char tmpstr[4] ;
	GdkColor color;
	GdkGC *gc ;
	GdkColorContext *cc;
	sprintf(fontname, "-bitstream-courier-bold-r-*-*-%d-*-*-*-*-*-*-*", 16);	
	font = gdk_font_load(fontname);
	gc = gdk_gc_new(draw_area->window);
	color.red   = 0 ;
	color.green = 0 ;
	color.blue  = 0 ;
	color.pixel = 0; /* required! */
	cc = gdk_color_context_new (gtk_widget_get_visual (draw_area),
				    gtk_widget_get_colormap (draw_area));

	n = 0;
	gdk_color_context_get_pixels (cc, &color.red, &color.green, &color.blue, 1, &color.pixel, &n) ;
	gdk_gc_set_foreground(gc, &color);
#endif

	if (tile < MAX_TILES) {
		trect.x = tiles[tile].x;
		trect.y = tiles[tile].y;
		trect.width = TILE_WIDTH;
		trect.height = TILE_HEIGHT;

		arect.x = x1;
		arect.y = y1;
		arect.width = x2 - x1 + 1;
		arect.height = y2 - y1 + 1;

		if (gdk_rectangle_intersect (&trect, &arect, &dest)) {
			/* 21 is the number of tiles in a row of the source image */

			orig_x = (tiles[tile].image % 21) * TILE_WIDTH + dest.x - trect.x;
			orig_y = (tiles[tile].image / 21) * TILE_HEIGHT + dest.y - trect.y;

			if (tiles[tile].selected)
				orig_y += 2 * TILE_HEIGHT;

			gdk_gc_set_clip_origin (my_gc, trect.x, trect.y);

			gdk_draw_pixmap (draw_area->window, 
					 my_gc,
					 tiles_pix,
					 orig_x, orig_y,
					 dest.x, dest.y,
					 dest.width, dest.height);

#ifdef SEQUENCE_DEBUG
			sprintf (tmpstr, "%d", tiles[tile].sequence) ;
			gdk_draw_string(draw_area->window,
					font, gc,
					dest.x+4, dest.y+16,
					tmpstr) ;
			sprintf (tmpstr, "%d", tile) ;
			gdk_draw_string(draw_area->window,
					font, gc,
					dest.x+4, dest.y+32+16,
					tmpstr) ;
#endif
		}
	}
}

void redraw_area (int x1, int y1, int x2, int y2, int mlayer)
{
	int i;
	GdkRectangle area, trect, dest;

	area.x = x1;
	area.y = y1;
	area.width = x2 - x1 + 1;
        area.height = y2 - y1 + 1;

        for(i = MAX_TILES; i > -1; i --) {
        	if ((tiles[i].visible) && (tiles[i].layer >= mlayer)) {
			trect.x = tiles[i].x;
			trect.y = tiles[i].y;
			trect.width = TILE_WIDTH;
			trect.height = TILE_HEIGHT;

			if (gdk_rectangle_intersect (&area, &trect, &dest))
				redraw_tile_in_area (x1, y1, x2, y2, i);
		}
	}
}

void refresh (GdkRectangle *area)
{
  redraw_area (area->x, area->y, area->x + area->width - 1,
	       area->y + area->height - 1, 0);
}

void tile_gone (int i, int x, int y)
{
	gdk_window_clear_area (draw_area->window, tiles[i].x, tiles[i].y,
			       TILE_WIDTH, TILE_HEIGHT);
	
	redraw_area (tiles[i].x, tiles[i].y, 
		     tiles[i].x + TILE_WIDTH - 1,
		     tiles[i].y + TILE_HEIGHT - 1,
		     0);
}

/* You loose your re-do queue when you make a move */
void clear_undo_queue ()
{
  int lp ;
  for (lp=0;lp<MAX_TILES;lp++)
    if (tiles[lp].sequence>=sequence_number)
      tiles[lp].sequence = 0 ;
}

void button_pressed (int x, int y)
{
	int i;
        gchar tmpchar[4];
        
	i = find_tile (x, y);
	if (i < MAX_TILES) {
	    if (tile_free (i)) {
/*		printf ("{ %d %d %d },\n",
			(tiles[i].x - 5*tiles[i].layer - 30)/HALF_WIDTH,
			(tiles[i].y + 4*tiles[i].layer - 25)/HALF_HEIGHT, tiles[i].layer) */
		if (selected_tile < MAX_TILES) {
			    if ((tiles[selected_tile].type == tiles[i].type) &&
				    (i != selected_tile) ) {
					tiles[i].visible = 0;
					tiles[selected_tile].visible = 0;
					tile_gone (i, x, y);
					tile_gone (selected_tile,
						   tiles[selected_tile].x + 1,
						   tiles[selected_tile].y + 1);
					clear_undo_queue() ;
					tiles[i].sequence = sequence_number ;
					tiles[selected_tile].sequence = sequence_number ;
					sequence_number++ ;
					selected_tile = MAX_TILES + 1;
                                        visible_tiles -= 2;
                                        sprintf(tmpchar,"%d",visible_tiles);
                                        gtk_label_set(GTK_LABEL(tiles_label), tmpchar);
					check_free();
					if (visible_tiles <= 0)
						you_won ();
				}
				else if (i == selected_tile) {
					tiles[i].selected = 0;
					selected_tile = MAX_TILES + 1;
				}
				else {
					no_match();
					i = selected_tile;
				}
			}
			else if (tiles[i].selected == 0) {
				tiles[i].selected = 1;
				selected_tile = i;
			}
			else {
				tiles[i].selected = 0;
				selected_tile = MAX_TILES + 1;
			}
			draw_selected_tile (i);
		}
	}
}


gint area_event (GtkWidget *widget, GdkEvent *event, void *d)
{
	switch (event->type) {
	case GDK_EXPOSE : {
		GdkEventExpose *e = (GdkEventExpose *) event;
		refresh (&e->area);
		return TRUE;
	}

	case GDK_BUTTON_PRESS: {
		int x, y;

		gtk_widget_get_pointer (widget, &x, &y);
		button_pressed (x, y);
		return TRUE;
	}

	default:
		return FALSE;
	}
}

void set_map (char *name)
{
  int lp ;

  for (lp=0;lp<sizeof(maps)/sizeof(struct _maps);lp++)
    if (strcasecmp (maps[lp].name, name) == 0)
      {
	pos = maps[lp].map ;
	generate_dependancies() ;
	new_game () ;

	if (mapset)
	  g_free (mapset);
	mapset = g_strdup(name);
      }
}

void load_tiles (char *fname)
{
	char *tmp, *fn;

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
	mask = gdk_imlib_move_mask (tiles_image);

	g_free (fn);
}

void create_mahjongg_board (void)
{
 	GtkStyle *style;
	gchar *buf;
	
	gtk_widget_push_visual (gdk_imlib_get_visual ());
	gtk_widget_push_colormap (gdk_imlib_get_colormap ());

	draw_area = gtk_drawing_area_new ();

	gtk_widget_pop_colormap ();
	gtk_widget_pop_visual ();
	
	gtk_widget_set_events (draw_area, gtk_widget_get_events (draw_area) | GAME_EVENTS);

	style = gtk_widget_get_style (draw_area);

	gtk_box_pack_start_defaults (GTK_BOX (mbox), draw_area);
	gtk_widget_realize (draw_area);

	gtk_widget_show (draw_area);

	gtk_drawing_area_size (GTK_DRAWING_AREA (draw_area),
			       600,
			       480);

	buf = gnome_config_get_string_with_default ("/gmahjongg/Preferences/tileset=default.xpm", NULL);
	load_tiles (buf);

	buf = gnome_config_get_string_with_default ("/gmahjongg/Preferences/mapset=easy", NULL);
	set_map (buf);

	buf = gnome_config_get_string_with_default ("/gmahjongg/Preferences/bcolour=#003010", NULL) ;
	set_backgnd_colour (buf) ;

	my_gc = gdk_gc_new (draw_area->window);
	gdk_gc_set_clip_mask (my_gc, mask);

	gtk_signal_connect (GTK_OBJECT (draw_area), "event", (GtkSignalFunc) area_event, 0);
	
	gtk_widget_draw (draw_area, NULL);
	g_free (buf);
	new_game ();
}

#define ELEMENTS(x) (sizeof (x) / sizeof (x [0]))

void new_game ()
{
  sequence_number = 1 ;
  visible_tiles = MAX_TILES;
  selected_tile = MAX_TILES + 1;

  clear_window() ;
  generate_game() ;
  gtk_widget_draw (draw_area, NULL) ;
}

int main (int argc, char *argv [])
{
	argp_program_version = MAH_VERSION;

	bindtextdomain (PACKAGE, GNOMELOCALEDIR);
	textdomain (PACKAGE);

	gnome_init ("mahjongg", NULL, argc, argv, 0, NULL);

	srand (time (NULL));
	
	generate_dependancies () ;

	window = gnome_app_new ("gmahjongg", _("Gnome Mahjongg"));
	gtk_widget_realize (window);
	gtk_window_set_policy (GTK_WINDOW (window), FALSE, FALSE, TRUE);

	gnome_app_create_menus (GNOME_APP (window), mainmenu);
	gtk_menu_item_right_justify (GTK_MENU_ITEM(mainmenu[2].widget));

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

