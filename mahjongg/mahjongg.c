/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * GNOME-Mahjongg
 * (C) 1998-1999 the Free Software Foundation
 *
 *
 * Author: Francisco Bustamante
 *
 *
 * http://www.nuclecu.unam.mx/~pancho/
 * pancho@nuclecu.unam.mx
 */

#include <sys/types.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <dirent.h>
#include <config.h>

#include <gtk/gtk.h>
#include <gnome.h>
#include <gconf/gconf-client.h>
#include <libgnomeui/gnome-window-icon.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <games-clock.h>
#include <games-gconf.h>

#include "mahjongg.h"
#include "solubility.h"
#include "gnome-canvas-pimage.h"

#define APPNAME "mahjongg"
#define APPNAME_LONG "Mahjongg"

#define AREA_WIDTH 600
#define AREA_HEIGHT 470
#define TILE_WIDTH 40
#define TILE_HEIGHT 56
#define HALF_WIDTH 18
#define HALF_HEIGHT 26
#define THICKNESS 5

/* #defines for the tile selection code. */
#define SELECTED_FLAG   1
#define HINT_FLAG       16

/* The number of half-cycles to blink during a hint (less 1) */
#define HINT_BLINK_NUM 5

/* Sorted such that the bottom leftest are first, and layers decrease
 * Bottom left = high y, low x ! */

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

tilepos *pos = 0;
gint xpos_offset;
gint ypos_offset;

tile tiles[MAX_TILES];

GtkWidget *window, *appbar;
GtkWidget *canvas;
GtkWidget *tiles_label;
GtkWidget *seed_label;
gint selected_tile, visible_tiles;
gint sequence_number;

guint current_seed, next_seed;

static GdkPixbuf *tiles_image, *bg_image;

static gchar *tileset = NULL;
static gchar *bg_tileset = NULL;
static gchar *mapset = NULL;
static gchar *score_current_mapset = NULL;

static gchar *selected_tileset = NULL;
static gchar *selected_bg = NULL;

static struct {
  GdkColor colour ;
  gchar *name ;
  gint set;
} backgnd = {
	{0, 0, 0, 0}, NULL, 0
};

struct _maps
{
  gchar *name ;
  tilepos *map ;
} maps[] = {
	{ "easy",      easy_map },
	{ "difficult", hard_map }
};


gint hint_tiles[2];
guint timer;
guint timeout_counter = HINT_BLINK_NUM + 1;

GtkWidget *moves_label;
gint moves_left=0;
GtkWidget *chrono;
gint paused=0;

/* for the preferences */
GConfClient *conf_client;
gboolean popup_warn = FALSE, popup_confirm = FALSE;
GtkWidget *warn_cb = NULL, *confirm_cb = NULL;
GtkWidget *colour_well = NULL;
GtkWidget *pref_dialog = NULL;

typedef enum {
	NEW_GAME,
	NEW_GAME_WITH_SEED,
	RESTART_GAME,
	QUIT_GAME
} game_state;

enum {
	GAME_RUNNING,
	GAME_WON,
	GAME_LOST,
	GAME_DEAD
} game_over;

static void change_tiles (void);
static void change_tile_image (tile *tile_inf);
void clear_undo_queue ();
void you_won (void);
void no_match (void);
void check_free (void);
void load_tiles (gchar *fname, gchar *bg_fname);
void undo_tile_callback    (GtkWidget *widget, gpointer data);
void redo_tile_callback    (GtkWidget *widget, gpointer data);
void hint_callback         (GtkWidget *widget, gpointer data);
void properties_callback   (GtkWidget *widget, gpointer data);
void about_callback        (GtkWidget *widget, gpointer data);
void show_tb_callback      (GtkWidget *widget, gpointer data);
void sound_on_callback     (GtkWidget *widget, gpointer data);
void scores_callback       (GtkWidget *widget, gpointer data);
gboolean delete_event_callback (GtkWidget *widget, GdkEventAny *any, gpointer data);
void confirm_action       (GtkWidget *widget, gpointer data);
void shuffle_tiles_callback   (GtkWidget *widget, gpointer data);
void ensure_pause_off (void);
void pause_callback (void);
void new_game (gboolean with_seed);
void restart_game (void);
void select_game (GtkWidget *widget, gpointer data);
void set_backgnd_colour (gchar *str);

GnomeUIInfo gamemenu [] = {
         GNOMEUIINFO_MENU_NEW_GAME_ITEM(confirm_action, NEW_GAME),

         {GNOME_APP_UI_ITEM, N_("New game with _seed..."),
		 N_("Start a new game giving a seed number..."),
		 select_game, NULL, NULL, GNOME_APP_PIXMAP_STOCK,
		 GTK_STOCK_NEW, 0, 0, NULL},

	 GNOMEUIINFO_MENU_RESTART_GAME_ITEM(confirm_action, RESTART_GAME),

	 GNOMEUIINFO_SEPARATOR,

	 GNOMEUIINFO_MENU_UNDO_MOVE_ITEM(undo_tile_callback, NULL),
	 GNOMEUIINFO_MENU_REDO_MOVE_ITEM(redo_tile_callback, NULL),

	 GNOMEUIINFO_MENU_HINT_ITEM(hint_callback, NULL),

         {GNOME_APP_UI_ITEM, N_("Shu_ffle tiles"), N_("Shuffle tiles"),
		 shuffle_tiles_callback, NULL, NULL, GNOME_APP_PIXMAP_STOCK,
		 NULL, 0, 0, NULL},

	 GNOMEUIINFO_SEPARATOR,

         GNOMEUIINFO_MENU_SCORES_ITEM(scores_callback, NULL),

	 GNOMEUIINFO_SEPARATOR,
         GNOMEUIINFO_MENU_QUIT_ITEM(confirm_action, QUIT_GAME),

	 GNOMEUIINFO_END
};

GnomeUIInfo settingsmenu [] = {
        GNOMEUIINFO_TOGGLEITEM(N_("Show _Tool Bar"),
			       N_("Toggle display of the toolbar"),
			       show_tb_callback, NULL),

	GNOMEUIINFO_SEPARATOR,

#ifdef SOUND_SUPPORT_FINISHED
        {GNOME_APP_UI_TOGGLEITEM, N_("_Sound"), NULL, NULL, NULL, NULL,
        GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
#endif

	GNOMEUIINFO_MENU_PREFERENCES_ITEM(properties_callback, NULL),
	GNOMEUIINFO_END
};

GnomeUIInfo helpmenu[] = {
        GNOMEUIINFO_HELP("mahjongg"),
	GNOMEUIINFO_MENU_ABOUT_ITEM(about_callback, NULL),
	GNOMEUIINFO_END

};

GnomeUIInfo mainmenu [] = {
	GNOMEUIINFO_MENU_GAME_TREE(gamemenu),
	GNOMEUIINFO_MENU_SETTINGS_TREE(settingsmenu),
	GNOMEUIINFO_MENU_HELP_TREE(helpmenu),
	GNOMEUIINFO_END
};

GnomeUIInfo toolbar_uiinfo [] = {
	{GNOME_APP_UI_ITEM, N_("New"), N_("New game"), confirm_action,
		(gpointer)NEW_GAME, NULL,
		GNOME_APP_PIXMAP_STOCK, GTK_STOCK_NEW, 0, 0, NULL},

        {GNOME_APP_UI_ITEM, N_("Restart"), N_("Restart game"), confirm_action,
		(gpointer)RESTART_GAME, NULL,
		GNOME_APP_PIXMAP_STOCK, GTK_STOCK_REFRESH, 0, 0, NULL},

        {GNOME_APP_UI_ITEM, N_("Hint"), N_("Get a hint"), hint_callback,
		NULL, NULL, GNOME_APP_PIXMAP_STOCK, GTK_STOCK_HELP, GDK_H,
		GDK_CONTROL_MASK, NULL},

        {GNOME_APP_UI_ITEM, N_("Undo"), N_("Undo previous move"),
		undo_tile_callback, NULL, NULL,
		GNOME_APP_PIXMAP_STOCK, GTK_STOCK_UNDO, 0, 0, NULL},

        {GNOME_APP_UI_ITEM, N_("Redo"), N_("Redo"), redo_tile_callback,
		NULL, NULL, GNOME_APP_PIXMAP_STOCK, GTK_STOCK_REDO, 0, 0, NULL},

	{GNOME_APP_UI_ITEM, N_("Shuffle"), N_("Shuffle tiles"),
		shuffle_tiles_callback, NULL, NULL,
		GNOME_APP_PIXMAP_STOCK, GTK_STOCK_EXECUTE, 0, 0, NULL},

        /* If you change the place for this button, change the index in
           the definition of PAUSE_BUTTON below */
        {GNOME_APP_UI_TOGGLEITEM, N_("Pause"), N_("Pause game"),
		pause_callback, NULL, NULL,
		GNOME_APP_PIXMAP_STOCK, GTK_STOCK_STOP, 0, 0, NULL},

#ifdef SOUND_SUPPORT_FINISHED
        {GNOME_APP_UI_TOGGLEITEM, N_("Sound"), N_("Toggle sound"),
		sound_on_callback, NULL, NULL,
		GNOME_APP_PIXMAP_DATA, mini_sound_xpm, 0, 0, NULL},
#endif

	{GNOME_APP_UI_ENDOFINFO}
};

#define PAUSE_BUTTON GTK_TOGGLE_BUTTON(toolbar_uiinfo[6].widget)
#define HIGHSCORE_WIDGET gamemenu[9].widget

static void
tileset_callback (GtkWidget *widget, void *data)
{
	selected_tileset = data;

	gconf_client_set_string (conf_client,
			"/apps/mahjongg/tileset",
			selected_tileset,
			NULL);
}

void
tileset_changed_cb (GConfClient *client,
		guint cnxn_id,
		GConfEntry *entry,
		gpointer user_data)
{
	char *tile_tmp = NULL, *bg_tmp = NULL;

	tile_tmp = gconf_client_get_string (conf_client,
			"/apps/mahjongg/tileset", NULL);
	bg_tmp = gconf_client_get_string (conf_client,
			"/apps/mahjongg/background", NULL);
	if (strcmp (tile_tmp, selected_tileset) != 0)
	{
		selected_tileset = tile_tmp;
	} else {
		g_free (tile_tmp);
	}

	load_tiles (selected_tileset, bg_tmp);
	change_tiles();
	g_free (bg_tmp);
	gnome_canvas_update_now(GNOME_CANVAS(canvas));
	//FIXME apply in the GUI
}

static void
bg_callback (GtkWidget *widget, void *data)
{
	selected_bg = data;

	gconf_client_set_string (conf_client,
			"/apps/mahjongg/background",
			selected_bg,
			NULL);
}

void
bg_changed_cb (GConfClient *client,
		guint cnxn_id,
		GConfEntry *entry,
		gpointer user_data)
{
	char *tile_tmp = NULL, *bg_tmp = NULL;

	tile_tmp = gconf_client_get_string (conf_client,
			"/apps/mahjongg/tileset", NULL);
	bg_tmp = gconf_client_get_string (conf_client,
			"/apps/mahjongg/background", NULL);
	if (strcmp (bg_tmp, selected_bg) != 0)
	{
		selected_bg = bg_tmp;
	} else {
		g_free (bg_tmp);
	}

	load_tiles (tile_tmp, selected_bg);
	change_tiles();
	g_free (tile_tmp);
	gnome_canvas_update_now(GNOME_CANVAS(canvas));
	//FIXME apply in the GUI
}

void
popup_warn_callback (GtkWidget *widget, gpointer data)
{
	popup_warn = gtk_toggle_button_get_active
		(GTK_TOGGLE_BUTTON (widget));

	gconf_client_set_bool (conf_client,
			"/apps/mahjongg/warn", popup_warn, NULL);
}

void
popup_warn_changed_cb (GConfClient *client,
		guint cnxn_id,
		GConfEntry *entry,
		gpointer user_data)
{
	gboolean popup_warn_tmp;

	popup_warn_tmp = gconf_client_get_bool (conf_client,
			"/apps/mahjongg/warn", NULL);
	if (popup_warn_tmp != popup_warn)
	{
		popup_warn = popup_warn_tmp;
		if (warn_cb != NULL)
			gtk_toggle_button_set_active
				(GTK_TOGGLE_BUTTON (warn_cb),
				popup_warn);
	}
}

void
popup_confirm_callback (GtkWidget *widget, gpointer data)
{
	popup_confirm = gtk_toggle_button_get_active
		(GTK_TOGGLE_BUTTON (widget));

	gconf_client_set_bool (conf_client,
			"/apps/mahjongg/confirm", popup_confirm, NULL);
}

void
popup_confirm_changed_cb (GConfClient *client,
		guint cnxn_id,
		GConfEntry *entry,
		gpointer user_data)
{
	gboolean popup_confirm_tmp;

	popup_confirm_tmp = gconf_client_get_bool (conf_client,
			"/apps/mahjongg/confirm", NULL);
	if (popup_confirm_tmp != popup_confirm)
	{
		popup_confirm = popup_confirm_tmp;
		if (confirm_cb != NULL)
			gtk_toggle_button_set_active
				(GTK_TOGGLE_BUTTON (confirm_cb),
				popup_confirm);
	}
}

void
show_toolbar_changed_cb (GConfClient *client,
		guint cnxn_id,
		GConfEntry *entry,
		gpointer user_data)
{
	BonoboDockItem *gdi;
	gboolean shown;

	shown = gconf_client_get_bool (conf_client,
			"/apps/mahjongg/show-toolbar", NULL);

	gdi = gnome_app_get_dock_item_by_name (GNOME_APP (window),
			GNOME_APP_TOOLBAR_NAME);

	if (shown == TRUE)
	{
		gtk_check_menu_item_set_active
			(GTK_CHECK_MENU_ITEM(settingsmenu[0].widget), TRUE);
		gtk_widget_show(GTK_WIDGET(gdi));
	} else {
		gtk_check_menu_item_set_active
			(GTK_CHECK_MENU_ITEM(settingsmenu[0].widget), FALSE);
		gtk_widget_hide(GTK_WIDGET(gdi));
		gtk_widget_queue_resize (window);
	}
}

void
show_tb_callback (GtkWidget *widget, gpointer data)
{
	if ((GTK_CHECK_MENU_ITEM (widget))->active) {
		gconf_client_set_bool(conf_client,
				"/apps/mahjongg/show-toolbar", TRUE, NULL);
	} else {
		gconf_client_set_bool(conf_client,
				"/apps/mahjongg/show-toolbar", FALSE, NULL);
	}
}

void
bg_colour_changed_cb (GConfClient *client,
		      guint cnxn_id,
		      GConfEntry *entry,
		      gpointer user_data)
{
	gchar *colour;

	colour = gconf_client_get_string (conf_client,
			"/apps/mahjongg/bgcolour", NULL);
	set_backgnd_colour (colour);
	if (colour_well != NULL)
	{
		gint ur,ug,ub ;

		sscanf (backgnd.name, "#%02x%02x%02x", &ur,&ug,&ub);
		gnome_color_picker_set_i8 (GNOME_COLOR_PICKER(colour_well),
				ur, ug, ub, 0);
	}
}

void
bg_colour_callback (GtkWidget *widget, gpointer data)
{
	static char *tmp = "";
	guint8 r, g, b, a;

	gnome_color_picker_get_i8(GNOME_COLOR_PICKER(widget), &r, &g, &b, &a);

	tmp = g_strdup_printf ("#%02x%02x%02x", r, g, b);

	gconf_client_set_string (conf_client,
			"/apps/mahjongg/bgcolour", tmp, NULL);
}

void
mapset_changed_cb (GConfClient *client,
		   guint        cnxn_id,
		   GConfEntry  *entry,
		   gpointer     user_data)
{
	GtkWidget *dialog;
	char *mapset_tmp;

	mapset_tmp = gconf_client_get_string (conf_client,
			"/apps/mahjongg/mapset",
			NULL);
	if ((mapset != NULL) && (strcmp (mapset, mapset_tmp) != 0)) {
		g_free (mapset);
		mapset = mapset_tmp;
	} else
		g_free (mapset_tmp);
	
	dialog = gtk_message_dialog_new (
		GTK_WINDOW (window),
		GTK_DIALOG_MODAL,
		GTK_MESSAGE_INFO,
		GTK_BUTTONS_OK,
		_("This new mapset will take effect when you start "
		  "a new game, or when Mahjongg is restarted."));

	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
}

void
set_map_selection (GtkWidget *widget, void *data)
{
	struct _maps *map = (struct _maps*) data;

	g_free (mapset);
	mapset = g_strdup (map->name);

	gconf_client_set_string (conf_client,
				 "/apps/mahjongg/mapset",
				 mapset, NULL);
}

void
init_config (void)
{
	gconf_client_add_dir (conf_client,
			"/apps/mahjongg", GCONF_CLIENT_PRELOAD_ONELEVEL,
			NULL);
	gconf_client_notify_add (conf_client,
			"/apps/mahjongg/show-toolbar",
			show_toolbar_changed_cb,
			NULL, NULL, NULL);
	gconf_client_notify_add (conf_client,
			"/apps/mahjongg/bgcolour",
			bg_colour_changed_cb,
			NULL, NULL, NULL);
	gconf_client_notify_add (conf_client,
			"/apps/mahjongg/mapset",
			mapset_changed_cb,
			NULL, NULL, NULL);
	gconf_client_notify_add (conf_client,
			"/apps/mahjongg/confirm",
			popup_confirm_changed_cb,
			NULL, NULL, NULL);
	gconf_client_notify_add (conf_client,
			"/apps/mahjongg/warn",
			popup_warn_changed_cb,
			NULL, NULL, NULL);
	gconf_client_notify_add (conf_client,
			"/apps/mahjongg/tileset",
			tileset_changed_cb,
			NULL, NULL, NULL);
	gconf_client_notify_add (conf_client,
			"/apps/mahjongg/background",
			bg_changed_cb,
			NULL, NULL, NULL);
}

static void
free_str (GtkWidget *widget, void *data)
{
	g_free (data);
}

void
message (gchar *message)
{
	gnome_appbar_pop (GNOME_APPBAR (appbar));
	gnome_appbar_push (GNOME_APPBAR (appbar), message);
}

void update_score_state ()
{
        gchar **names = NULL;
        gfloat *scores = NULL;
        time_t *scoretimes = NULL;
	gint top;

	top = gnome_score_get_notable(APPNAME, score_current_mapset, &names, &scores, &scoretimes);
	if (top > 0) {
		gtk_widget_set_sensitive (HIGHSCORE_WIDGET, TRUE);
		g_strfreev(names);
		g_free(scores);
		g_free(scoretimes);
	} else {
		gtk_widget_set_sensitive (HIGHSCORE_WIDGET, FALSE);
	}
}


void
chrono_start (void)
{
	games_clock_stop (GAMES_CLOCK (chrono));
	games_clock_set_seconds (GAMES_CLOCK (chrono), 0);
	games_clock_start (GAMES_CLOCK (chrono));
}

gint
update_moves_left (void)
{
        char *tmpstr;

        check_free ();
	tmpstr = g_strdup_printf ("%2d", moves_left);
        gtk_label_set_text (GTK_LABEL (moves_label), tmpstr);

        return moves_left;
}

void
set_backgnd_colour (gchar *str)
{
	GdkColormap *colourmap ;
	GtkStyle *widget_style, *temp_style;

	g_return_if_fail (str != NULL);

	if (str != backgnd.name) {
		g_free (backgnd.name) ;
		backgnd.name = g_strdup (str) ;
	}
	colourmap = gtk_widget_get_colormap (canvas);
	gdk_color_parse (backgnd.name, &backgnd.colour);

	gdk_color_alloc (colourmap, &backgnd.colour);

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
change_tiles (void)
{
        gint i;
        
        for (i = 0; i < MAX_TILES; i++)
                change_tile_image (&tiles[i]);
}

static void
change_tile_image (tile *tile_inf)
{
	gint orig_x, orig_y;

	orig_x = (tile_inf->image % 21) * TILE_WIDTH;
	orig_y = (tile_inf->image / 21) * TILE_HEIGHT;

	if (tile_inf->selected) {
		orig_y += 2 * TILE_HEIGHT;
	}

       /*gdk_pixbuf_finalize (tile_inf->current_image);
          gdk_pixbuf_finalize (tile_inf->current_bg);*/
	
	gdk_pixbuf_copy_area (tiles_image, orig_x, orig_y,
			      TILE_WIDTH, TILE_HEIGHT,
			      tile_inf->current_image, 0, 0);
	gdk_pixbuf_copy_area (bg_image,
			pos[tile_inf->number].layer * TILE_WIDTH,
			(tile_inf->selected != 0 ? 1 : 0) * TILE_HEIGHT,
			TILE_WIDTH, TILE_HEIGHT,
			tile_inf->current_bg, 0, 0);
        
	gnome_canvas_item_set (tile_inf->bg_item, "image",
			tile_inf->current_bg, NULL);
	gnome_canvas_item_set (tile_inf->image_item, "image",
			tile_inf->current_image, NULL);
}

void
select_tile (tile *tile_inf)
{
        tile_inf->selected |= SELECTED_FLAG;
        change_tile_image(tile_inf);
        selected_tile = tile_inf->number;
}

void
unselect_tile (tile *tile_inf)
{
        selected_tile = MAX_TILES + 1;
        tile_inf->selected &= ~SELECTED_FLAG;
        change_tile_image (tile_inf);
}

static void
tile_event (GnomeCanvasItem *item, GdkEvent *event, tile *tile_inf)
{
	char *tmpstr;

	if (paused)
		return;

	switch (event->type) {
	case GDK_BUTTON_PRESS :
		if (tile_free (tile_inf->number)) {
			switch (event->button.button) {
			case 1:
				if (tile_inf->selected & SELECTED_FLAG)
					unselect_tile (tile_inf);
				else {
					if (selected_tile < MAX_TILES) {
						if ((tiles[selected_tile].type == tile_inf->type) ) {
							tiles[selected_tile].visible = 0;
							tile_inf->visible = 0;
							tiles[selected_tile].selected &= ~SELECTED_FLAG;
							change_tile_image (&tiles[selected_tile]);
							gnome_canvas_item_hide (tiles[selected_tile].canvas_item);
							gnome_canvas_item_hide (tile_inf->canvas_item);
							clear_undo_queue ();
							tiles[selected_tile].sequence = tile_inf->sequence = sequence_number;
							sequence_number ++;
							selected_tile = MAX_TILES + 1;
							visible_tiles -= 2;
							tmpstr = g_strdup_printf("%3d", visible_tiles);
							gtk_label_set_text (GTK_LABEL(tiles_label), tmpstr);
							update_moves_left ();

							if (visible_tiles <= 0) {
								games_clock_stop(GAMES_CLOCK(chrono));
								you_won ();
							}
						}
						else
							no_match ();
					}
					else 
						select_tile (tile_inf);
				}
				break;
                          
			case 3:
				if (selected_tile < MAX_TILES) 
					unselect_tile (&tiles[selected_tile]);
				select_tile (tile_inf);
                          
			default: 
				break;
			}
			break;

		default :
			break;
		}
	}
}

static void
fill_tile_menu (GtkWidget *menu, gchar *sdir, gint is_tile)
{
	struct dirent *e;
	DIR *dir;
        gint itemno = 0;
	gchar *dname = NULL;

	dname = gnome_program_locate_file (NULL, GNOME_FILE_DOMAIN_APP_PIXMAP,
			(sdir), FALSE, NULL);
	dir = opendir (dname);

	if (!dir) {
		g_free (dname);
		return;
	}

	while ((e = readdir (dir)) != NULL){
		GtkWidget *item;
		gchar *s = g_strdup (e->d_name);

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
		gtk_menu_shell_append (GTK_MENU_SHELL(menu), item);
                if (is_tile) {
                        g_signal_connect (G_OBJECT (item), "activate",
                                          G_CALLBACK (tileset_callback), s); 
                        g_signal_connect (G_OBJECT (item), "destroy",
                                          G_CALLBACK (free_str), s);
                } else {
                        g_signal_connect (G_OBJECT (item), "activate",
                                          G_CALLBACK (bg_callback), s); 
                        g_signal_connect (G_OBJECT (item), "destroy",
                                          G_CALLBACK (free_str), s);
                }

		if (is_tile) {
			if (!strcmp(tileset, s)) {
				gtk_menu_set_active(GTK_MENU(menu), itemno);
			}
		} else {
			if (!strcmp(bg_tileset, s)) {
				gtk_menu_set_active(GTK_MENU(menu), itemno);
			}
		}

	        itemno++;
	}

	closedir (dir);
	g_free (dname);
}

static void
fill_map_menu (GtkWidget *menu)
{
	gint lp, itemno=0 ;
	GtkWidget *item;

	for (lp=0;lp<G_N_ELEMENTS(maps);lp++) {
		gchar *str = g_strdup (_(maps[lp].name)) ;

		item = gtk_menu_item_new_with_label (str) ;
		gtk_widget_show (item);
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
		g_signal_connect (G_OBJECT(item), "activate",
				  G_CALLBACK (set_map_selection), &maps[lp]); 
		g_signal_connect (G_OBJECT(item), "destroy",
				  G_CALLBACK (free_str), str); 
		if (!g_ascii_strcasecmp (mapset, maps[lp].name))
			gtk_menu_set_active (GTK_MENU (menu), itemno); 
		itemno++ ;
	}
}

void
no_match (void)
{
	if (popup_warn == TRUE) {
		GtkWidget *mb;

		mb = gtk_message_dialog_new (GTK_WINDOW (window),
				GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_INFO,
				GTK_BUTTONS_OK,
				_("These tiles don't match."));
		gtk_dialog_run (GTK_DIALOG (mb));
		gtk_widget_destroy (mb);
        } else
		gnome_app_flash (GNOME_APP (window),
				_("These tiles don't match."));
}

void
check_free (void)
{
	gint i;
        gint tile_count[MAX_TILES];

        moves_left = 0;

        for (i=0; i<MAX_TILES; i++)
                tile_count[i] = 0;

	for (i=0;i<MAX_TILES;i++) {
                if (tile_free(i))
                        tile_count[tiles[i].type]++;
        }

        for (i=0; i<MAX_TILES; i++)
                moves_left += tile_count[i]>>1;

 	if ((moves_left == 0) && (visible_tiles>0)) { 
                GtkWidget *mb;

                if (!game_over) {
			mb = gtk_message_dialog_new (GTK_WINDOW (window),
						     GTK_DIALOG_MODAL
						     | GTK_DIALOG_DESTROY_WITH_PARENT,
						     GTK_MESSAGE_INFO,
						     GTK_BUTTONS_OK,
						     (_("There are no more moves.")));
			gtk_dialog_run (GTK_DIALOG (mb));
			gtk_widget_destroy (mb);
                }
                game_over = GAME_LOST;
 	} 
}

void
you_won (void)
{
        gint pos;
        time_t seconds;
        gfloat score;
        GtkWidget *dialog;
	
        game_over = GAME_WON;

        seconds = GAMES_CLOCK (chrono)->stopped;

        score = (seconds / 60) * 1.0 + (seconds % 60) / 100.0;
        if (pos = gnome_score_log (score, score_current_mapset, FALSE)) {
                dialog = gnome_scores_display (_(APPNAME_LONG), APPNAME, score_current_mapset, pos);
		if (dialog != NULL) {
			gtk_window_set_transient_for (GTK_WINDOW(dialog),
						      GTK_WINDOW(window));
			gtk_window_set_modal (GTK_WINDOW(dialog), TRUE);
		}
        } 
}

static void
pref_dialog_response (GtkDialog *dialog, gint response, gpointer data)
{
	gtk_widget_destroy (pref_dialog);
	pref_dialog = NULL;
	warn_cb = NULL;
	confirm_cb = NULL;
	colour_well = NULL;
}

GtkWidget *
bold_frame (gchar * title)
{
	gchar *markup;
	GtkWidget * frame;
	
	markup = g_strdup_printf ("<b>%s</b>",title);
	frame = gtk_frame_new(markup);
	g_free(markup);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);
	gtk_label_set_use_markup (GTK_LABEL (gtk_frame_get_label_widget(GTK_FRAME(frame))), TRUE);
	
	return frame;
}

void
properties_callback (GtkWidget *widget, gpointer data)
{
	GtkWidget *menu, *omenu;
	GtkWidget *vbox, *frame, *table, *w, *label;
	GtkSizeGroup *group;

	if (pref_dialog) {
		gtk_window_present (GTK_WINDOW (pref_dialog));
		return;
	}

	pref_dialog = gtk_dialog_new_with_buttons (_("Preferences"),
			GTK_WINDOW (window),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
			NULL);

	gtk_window_set_resizable (GTK_WINDOW (pref_dialog), FALSE);
	gtk_dialog_set_default_response (GTK_DIALOG (pref_dialog),
					 GTK_RESPONSE_CLOSE);
	g_signal_connect (G_OBJECT (pref_dialog), "response",
			  G_CALLBACK(pref_dialog_response), NULL);

	group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 8);

	frame = bold_frame (_("Tiles"));

	table = gtk_table_new (2, 2, FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (table), 8);
	gtk_table_set_row_spacings (GTK_TABLE (table), 4);
	gtk_table_set_col_spacings (GTK_TABLE (table), 8);
	
	label = gtk_label_new (_("Tile Set"));
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_size_group_add_widget (group, label);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 0, 1);

	omenu = gtk_option_menu_new ();
	menu = gtk_menu_new ();
	fill_tile_menu (menu, "mahjongg", 1);
	gtk_option_menu_set_menu (GTK_OPTION_MENU(omenu), menu);
	gtk_table_attach_defaults (GTK_TABLE (table), omenu, 1, 2, 0, 1);

	label = gtk_label_new (_("Tile Background"));
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_size_group_add_widget (group, label);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 1, 2);

	omenu = gtk_option_menu_new ();
	menu = gtk_menu_new ();
	fill_tile_menu (menu, "mahjongg/bg", 0);
	gtk_option_menu_set_menu (GTK_OPTION_MENU(omenu), menu);
	gtk_table_attach_defaults (GTK_TABLE (table), omenu, 1, 2, 1, 2);

	gtk_container_add(GTK_CONTAINER (frame), table);
	gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);


	frame = bold_frame (_("Maps"));

	table = gtk_table_new (1, 2, FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (table), 8);
	gtk_table_set_row_spacings (GTK_TABLE (table), 4);
	gtk_table_set_col_spacings (GTK_TABLE (table), 8);
	
	label = gtk_label_new (_("Select Map"));
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_size_group_add_widget (group, label);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 0, 1);

	omenu = gtk_option_menu_new ();
	menu = gtk_menu_new ();
	fill_map_menu (menu);
	gtk_option_menu_set_menu (GTK_OPTION_MENU(omenu), menu);
	gtk_table_attach_defaults (GTK_TABLE (table), omenu, 1, 2, 0, 1);

	gtk_container_add(GTK_CONTAINER (frame), table);
	gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);


	frame = bold_frame (_("Colours"));

	table = gtk_table_new (1, 2, FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (table), 8);
	gtk_table_set_row_spacings (GTK_TABLE (table), 4);
	gtk_table_set_col_spacings (GTK_TABLE (table), 8);
	
	label = gtk_label_new (_("Background Color"));
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_size_group_add_widget (group, label);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 0, 1);

	{
	  int ur,ug,ub ;

	  w  = gnome_color_picker_new();
	  sscanf (backgnd.name, "#%02x%02x%02x", &ur,&ug,&ub);
	  gnome_color_picker_set_i8 (GNOME_COLOR_PICKER(w), ur, ug, ub, 0);
	  g_signal_connect (G_OBJECT(w), "color_set",
			    G_CALLBACK (bg_colour_callback), &backgnd.name);
	}
	gtk_table_attach_defaults (GTK_TABLE (table), w, 1, 2, 0, 1);

	gtk_container_add(GTK_CONTAINER (frame), table);
	gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);


	frame = bold_frame (_("Warnings"));

	table = gtk_vbox_new (FALSE, FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (table), 8);

	w = gtk_check_button_new_with_label (_("Warn when tiles don't match"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (w), popup_warn);
	g_signal_connect (G_OBJECT(w), "clicked", G_CALLBACK (popup_warn_callback), NULL);
	gtk_box_pack_start_defaults (GTK_BOX (table), w);

	w = gtk_check_button_new_with_label (_("Show confirmation dialogs"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(w), popup_confirm);
	g_signal_connect (G_OBJECT(w), "clicked", G_CALLBACK (popup_confirm_callback), NULL);
	gtk_box_pack_start_defaults (GTK_BOX(table), w);
	
	gtk_container_add(GTK_CONTAINER (frame), table);
	gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);

	gtk_box_pack_start_defaults (GTK_BOX(GTK_DIALOG(pref_dialog)->vbox), vbox);

	g_object_unref (group);
	gtk_widget_show_all (pref_dialog);
}

gint
hint_timeout (gpointer data)
{
	timeout_counter ++;

	if (timeout_counter > HINT_BLINK_NUM) {
		if (selected_tile < MAX_TILES)
			tiles[selected_tile].selected = 1;
		return 0;
	}

	tiles[hint_tiles[0]].selected ^= HINT_FLAG;
	tiles[hint_tiles[1]].selected ^= HINT_FLAG;
	change_tile_image(&tiles[hint_tiles[0]]);
	change_tile_image(&tiles[hint_tiles[1]]);

	return 1;
}

void
hint_callback (GtkWidget *widget, gpointer data)
{
        gint i, j, free=0, type ;
        time_t seconds;

        if (paused || game_over)
                return;

	/* This prevents the flashing speeding up if the hint button is
	 * pressed multiple times. */
	if (timeout_counter <= HINT_BLINK_NUM)
		return;

	/* Snarfed from check free
	 * Tile Free is now _so_ much quicker, it is more elegant to do a
	 * British Library search, and safer. */

	/* Clear any selection */
	if (selected_tile < MAX_TILES) {
		tiles[selected_tile].selected = 0;
		change_tile_image (&tiles[selected_tile]);
		selected_tile = MAX_TILES + 1;
	}
                
	for (i=0;i<MAX_TILES && !free;i++)
		if (tile_free(i)) {
			type = tiles[i].type ;
			for (j=0;j<MAX_TILES && !free;j++) {
				free = (tiles[j].type == type && i != j && tile_free(j)) ;
				if (free) {
					tiles[i].selected ^= HINT_FLAG;
					tiles[j].selected ^= HINT_FLAG;
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
                
	/* 30s penalty */
	games_clock_stop (GAMES_CLOCK(chrono));
	seconds = GAMES_CLOCK(chrono)->stopped;
	games_clock_set_seconds(GAMES_CLOCK(chrono), (gint) (seconds+30));
	games_clock_start (GAMES_CLOCK(chrono));
}

void
about_callback (GtkWidget *widget, gpointer data)
{
	GtkWidget *about;
	GdkPixbuf *pixbuf = NULL;
	const gchar *authors [] = {
		"Code: Francisco Bustamante",
		"      Max Watson",
		"      Heinz Hempe",
		"      Michael Meeks",
                "      Philippe Chavin",
		"Tiles: Jonathan Buzzard",
		"       Max Watson",
		NULL
	};
	gchar *documenters[] = {
                NULL
        };
        /* Translator credits */
        gchar *translator_credits = _("translator_credits");

	{
		char *filename = NULL;

		filename = gnome_program_locate_file (NULL,
				GNOME_FILE_DOMAIN_APP_PIXMAP, 
				"gnome-mahjongg.png",
				TRUE, NULL);
		if (filename != NULL) {
			pixbuf = gdk_pixbuf_new_from_file(filename, NULL);
			g_free (filename);
		}
	}


	about = gnome_about_new (_("GNOME Mahjongg"), VERSION,
				 "(C) 1998 The Free Software Foundation",
				  _("Send comments and bug reports to:\n"
				   "        pancho@nuclecu.unam.mx or\n"
				   "        mmeeks@gnu.org\n\n"
				   "Tiles under the General Public License."),
				 (const gchar **)authors,
				 (const gchar **)documenters,
				 strcmp (translator_credits, "translator_credits") != 0 ? translator_credits : NULL,
				pixbuf);
	
	if (pixbuf != NULL)
		gdk_pixbuf_unref (pixbuf);
	
	gtk_window_set_transient_for (GTK_WINDOW (about), GTK_WINDOW (window));
	gtk_widget_show (about);
}

void
pause_callback (void)
{
        gint i;
        if (game_over) {
                gtk_toggle_button_set_active (PAUSE_BUTTON, FALSE);
                return;
        }
        paused = !paused;
        if (paused) {
                games_clock_stop (GAMES_CLOCK (chrono));
                for (i = 0; i < MAX_TILES; i++)
                        if (tiles[i].visible)
                                gnome_canvas_item_hide (tiles[i].image_item);
                message(_("... Game paused ..."));
        }
        else {
                for (i = 0; i < MAX_TILES; i++)
                        if (tiles[i].visible)
                                gnome_canvas_item_show (tiles[i].image_item);
                message ("");
                games_clock_start (GAMES_CLOCK(chrono));
        }
}

void ensure_pause_off (void)
{
        gint i;

        if (paused) {
                gtk_toggle_button_set_active (PAUSE_BUTTON, FALSE);
                for (i = 0; i < MAX_TILES; i++)
                        if (tiles[i].visible)
                                gnome_canvas_item_show (tiles[i].image_item);
                message("");
        }
        paused = FALSE;
}

void
scores_callback (GtkWidget *widget, gpointer data)
{
	GtkWidget *dialog;

	dialog = gnome_scores_display (_(APPNAME_LONG), APPNAME, score_current_mapset, 0);
	if (dialog != NULL) {
		gtk_window_set_transient_for (GTK_WINDOW(dialog),
					      GTK_WINDOW(window));
		gtk_window_set_modal (GTK_WINDOW(dialog), TRUE);
	}
}

void
init_game (void)
{
        gtk_label_set_text (GTK_LABEL (tiles_label), MAX_TILES_STR);
        update_moves_left ();
        game_over = GAME_RUNNING;
        sequence_number = 1 ;
        visible_tiles = MAX_TILES;
        selected_tile = MAX_TILES + 1;
        gnome_canvas_update_now(GNOME_CANVAS(canvas));

        chrono_start();
}

void
confirm_action (GtkWidget *widget, gpointer data)
{
	gboolean doit = TRUE;

	if (popup_confirm == TRUE) {
		gchar *confirm_text;
		GtkWidget *dialog;
		gint response;
		
		switch ((game_state)data) {
		case RESTART_GAME :
			confirm_text = _("Are you sure you want to restart this game?");
			break;
		case QUIT_GAME :
			/* GNOME IS AN ACRONYM, DAMNIT! */
			confirm_text = _("Are you sure you want to quit GNOME Mahjongg?");
			break;
		case NEW_GAME:
		case NEW_GAME_WITH_SEED:
			confirm_text = _("Are you sure you want to start a new game?");
			break;
		default:
			confirm_text = _("Serious internal error");
			break;
		}

		/* Special case the quit because the buttons are different */

		if ((game_state)data == QUIT_GAME)
		{
			dialog = gtk_message_dialog_new (GTK_WINDOW (window),
						 GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
						 GTK_MESSAGE_QUESTION,
						 GTK_BUTTONS_NONE,
						 confirm_text);
			gtk_dialog_add_buttons (GTK_DIALOG (dialog),
						GTK_STOCK_CANCEL,
						GTK_RESPONSE_NO,
						GTK_STOCK_QUIT,
						GTK_RESPONSE_YES,
						NULL);
		}
		else {
                        dialog = gtk_message_dialog_new (GTK_WINDOW (window),
                                                 GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                                 GTK_MESSAGE_QUESTION,
                                                 GTK_BUTTONS_YES_NO,
                                                 confirm_text);
		}

		gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_YES);

		response = gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		
		doit = (response == GTK_RESPONSE_YES);
	}
	
	if (doit) {
		switch ((gint)data) {
		case NEW_GAME:
		case NEW_GAME_WITH_SEED:
			ensure_pause_off ();
			new_game ((gint)data == NEW_GAME);
			break;
		case RESTART_GAME:
			restart_game ();
			break;
		case QUIT_GAME:
			gtk_main_quit ();
			break;
		default:
			break;
		}
	}
}

gboolean
delete_event_callback (GtkWidget *widget, GdkEventAny *any, gpointer data)
{
        confirm_action (widget, data);
        return TRUE;
}

void
restart_game (void)
{
    gint i;

    ensure_pause_off ();
    for (i = 0; i < MAX_TILES; i++) {
        tiles[i].visible = 1;
        tiles[i].selected = 0;
	tiles[i].sequence = 0;
	if (i == selected_tile)
	  change_tile_image (&tiles[selected_tile]);
	  
	gnome_canvas_item_show (tiles[i].canvas_item);
    }
    init_game ();
}

void
redo_tile_callback (GtkWidget *widget, gpointer data)
{
        gint i, change ;
        char *tmpstr;
        
        if (paused) 
                return; 
        if (sequence_number>(MAX_TILES/2))
                return ;
        
        if (selected_tile<MAX_TILES) {
                tiles[selected_tile].selected = 0 ;
                change_tile_image (&tiles[selected_tile]);
                selected_tile = MAX_TILES + 1; 
        }
        change = 0 ;
        for (i=0; i<MAX_TILES; i++)
                if (tiles[i].sequence == sequence_number) {
                        tiles[i].selected = 0 ;
                        tiles[i].visible = 0 ;
                        gnome_canvas_item_hide (tiles[i].canvas_item);
                        visible_tiles-- ;
                        change = 1 ;
                }
        if (change) {
                if (sequence_number < MAX_TILES)
                        sequence_number++ ;
        }
        else
                  	gnome_app_flash (GNOME_APP (window), "No more redo!");
        tmpstr = g_strdup_printf ("%3d",visible_tiles);
        gtk_label_set_text(GTK_LABEL (tiles_label), tmpstr);
        
        update_moves_left ();
        gnome_canvas_update_now (GNOME_CANVAS (canvas));
}

void
undo_tile_callback (GtkWidget *widget, gpointer data)
{
        gint i;
        char *tmpstr;
        
        if (paused || game_over == GAME_WON) 
                return;
        if (game_over == GAME_LOST)
                game_over = GAME_RUNNING;
        if (sequence_number>1)
                sequence_number-- ;
        else
                return ;
        
        if (selected_tile<MAX_TILES) {
                tiles[selected_tile].selected = 0 ;
                change_tile_image (&tiles[selected_tile]);
                selected_tile = MAX_TILES + 1; 
        }
        
        for (i=0; i<MAX_TILES; i++)
                if (tiles[i].sequence == sequence_number) {
                        tiles[i].selected = 0 ;
                        tiles[i].visible = 1 ;
                        visible_tiles++ ;
                        gnome_canvas_item_show (tiles[i].canvas_item);
                }

        tmpstr = g_strdup_printf ("%3d", visible_tiles);
        gtk_label_set_text (GTK_LABEL(tiles_label), tmpstr);
        gnome_canvas_update_now (GNOME_CANVAS (canvas));

        update_moves_left ();
}

void
select_game (GtkWidget *widget, gpointer data)
{
	GtkWidget *dialog, *entry, *label;
	GtkWidget *box;
	gint response;

	dialog = gtk_dialog_new_with_buttons (_("Select Game"),
						 GTK_WINDOW (window),
						 GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
						 GTK_STOCK_CANCEL,
						 GTK_RESPONSE_CANCEL,
						 GTK_STOCK_OK,
						 GTK_RESPONSE_OK,
						 NULL);
	
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
	gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
	box = GTK_DIALOG (dialog)->vbox;

	gtk_box_set_spacing (GTK_BOX (box), 8);
	gtk_container_set_border_width (GTK_CONTAINER (dialog), 8);

	label = gtk_label_new (_("Game Number:"));
	gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_box_pack_start_defaults (GTK_BOX(box), label);
	
	entry = gtk_entry_new ();
	gtk_box_pack_start_defaults (GTK_BOX(box), entry);
	
	gtk_entry_set_activates_default (GTK_ENTRY (entry), TRUE);
	gtk_window_set_focus (GTK_WINDOW (dialog), entry);

	gtk_widget_show_all (dialog);
	
	response = gtk_dialog_run (GTK_DIALOG (dialog));
	
	if (response == GTK_RESPONSE_OK) {
		next_seed = atoi (gtk_entry_get_text (GTK_ENTRY (entry)));
		gtk_widget_destroy (dialog);
		confirm_action (widget, (gpointer) NEW_GAME_WITH_SEED);
	} else {
              	gtk_widget_destroy (dialog);
	}
}

void
sound_on_callback (GtkWidget *widget, gpointer data)
{
	printf ("mer\n");
}

/* You loose your re-do queue when you make a move */
void
clear_undo_queue (void)
{
	gint lp ;

	for (lp=0;lp<MAX_TILES;lp++)
		if (tiles[lp].sequence>=sequence_number)
			tiles[lp].sequence = 0 ;
}

void
load_map (void)
{
	gchar* name = mapset;
	gint lp ;
	gint xmax = 0, ymax = 0;
	tilepos *t;

	for (lp=0;lp<G_N_ELEMENTS(maps);lp++)
		if (g_ascii_strcasecmp (maps[lp].name, name) == 0) {
			pos = maps[lp].map ;

			for (t = pos ; t < pos + MAX_TILES ; t++) {
				if ( (*t).x  > xmax )
					xmax = (*t).x;
				if ( (*t).y  > ymax )
					ymax = (*t).y;
			}
			xpos_offset = ( AREA_WIDTH - (HALF_WIDTH * (xmax+1)) ) / 2;
			ypos_offset = ( AREA_HEIGHT - (HALF_HEIGHT * (ymax+1) ) ) / 2;

			generate_dependancies() ;
		}
}

gint
canvas_x (gint i)
{
	return pos[i].x * (HALF_WIDTH-0) + xpos_offset + (THICKNESS * pos[i].layer);
}

gint
canvas_y (gint i)
{
	return pos[i].y * (HALF_HEIGHT-0) + ypos_offset - (THICKNESS * pos[i].layer);
}

void
load_images (void)
{
	gint i;
  
	for (i = MAX_TILES - 1; i >= 0; i --) {
		gnome_canvas_item_set (tiles[i].image_item,
				       "x", (double)canvas_x(i),
				       "y", (double)canvas_y(i),
				       NULL);
		gnome_canvas_item_set (tiles[i].bg_item,
				       "x", (double)canvas_x(i),
				       "y", (double)canvas_y(i),
				       NULL);
	}
}

void
create_canvas_items (void)
{
	gint orig_x, orig_y, i;
  
	/* It's essential that the tiles are already sorted ginto layer order (lowest first) */
	for (i = MAX_TILES - 1; i >= 0; i --) {
		tiles[i].canvas_item = gnome_canvas_item_new (gnome_canvas_root(GNOME_CANVAS(canvas)),
							      gnome_canvas_group_get_type (),
							      NULL);
		orig_x = (tiles[i].image % 21) * TILE_WIDTH;
		orig_y = (tiles[i].image / 21) * TILE_HEIGHT;
	
		tiles[i].number = i;

		tiles[i].current_image = gdk_pixbuf_new (gdk_pixbuf_get_colorspace(tiles_image),
							 TRUE, gdk_pixbuf_get_bits_per_sample(tiles_image),
							 TILE_WIDTH, TILE_HEIGHT);
		tiles[i].current_bg = gdk_pixbuf_new (gdk_pixbuf_get_colorspace(tiles_image),
						      TRUE, gdk_pixbuf_get_bits_per_sample(tiles_image),
						      TILE_WIDTH, TILE_HEIGHT);
	
		gdk_pixbuf_copy_area (tiles_image, orig_x, orig_y,
				      TILE_WIDTH, TILE_HEIGHT,
				      tiles[i].current_image, 0, 0);
		gdk_pixbuf_copy_area (bg_image, pos[i].layer * TILE_WIDTH, 0,
				      TILE_WIDTH, TILE_HEIGHT,
				      tiles[i].current_bg, 0, 0);


		tiles[i].bg_item = gnome_canvas_item_new (GNOME_CANVAS_GROUP (tiles[i].canvas_item),
							  gnome_canvas_pimage_get_type(),
							  "image", tiles[i].current_bg,
							  "x", (double)canvas_x(i),
							  "y", (double)canvas_y(i),
							  "width", (double)TILE_WIDTH,
							  "height", (double)TILE_HEIGHT,
							  NULL);

		tiles[i].image_item = gnome_canvas_item_new (GNOME_CANVAS_GROUP (tiles[i].canvas_item),
							     gnome_canvas_pimage_get_type(),
							     "image", tiles[i].current_image,
							     "x", (double)canvas_x(i),
							     "y", (double)canvas_y(i),
							     "width", (double)TILE_WIDTH,
							     "height", (double)TILE_HEIGHT,
							     NULL);
	
		g_signal_connect (G_OBJECT (tiles[i].canvas_item), "event",
				  G_CALLBACK (tile_event), &tiles[i]);
	}
}

void
load_tiles (gchar *fname, gchar *bg_fname)
{
	gchar *tmp, *fn, *bg_fn;

	tmp = g_strconcat ("mahjongg/", fname, NULL);
	
	fn = gnome_program_locate_file (NULL, GNOME_FILE_DOMAIN_APP_PIXMAP, (tmp), FALSE, NULL);
	g_free (tmp);

	tmp = g_strconcat ("mahjongg/bg/", bg_fname, NULL);

	bg_fn = gnome_program_locate_file (NULL, GNOME_FILE_DOMAIN_APP_PIXMAP, (tmp), FALSE, NULL);
	g_free (tmp);

	if (!g_file_test ((fn), G_FILE_TEST_EXISTS)) {
		gchar *s = g_strdup_printf (_("Could not find file %s"), fn);
		GtkWidget *box;

		box = gtk_message_dialog_new (GTK_WINDOW (window),
					      GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
					      GTK_MESSAGE_ERROR,
					      GTK_BUTTONS_OK,
					      s);
	
		gtk_dialog_run (GTK_DIALOG (box));
		
		exit (1);
	}

	if (!g_file_test ((bg_fn), G_FILE_TEST_EXISTS)) {
		gchar *s = g_strdup_printf (_("Could not find file %s"), bg_fn);
		GtkWidget *box;
	
		box = gtk_message_dialog_new (GTK_WINDOW (window),	
					      GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
					      GTK_MESSAGE_ERROR,
					      GTK_BUTTONS_OK,
					      s);
		gtk_dialog_run (GTK_DIALOG (box));

		exit (1);
	}

	g_free (tileset);
	tileset = g_strdup(fname);
	
	g_free (bg_tileset);
	bg_tileset = g_strdup(bg_fname);
	
	if (tiles_image)
		gdk_pixbuf_unref (tiles_image);

	if (bg_image)
		gdk_pixbuf_unref (bg_image);

	tiles_image = gdk_pixbuf_new_from_file (fn, NULL);

        bg_image = gdk_pixbuf_new_from_file (bg_fn, NULL);

	g_free (bg_fn);
	g_free (fn);
}

static void
do_game (void)
{
	char *str;

	current_seed = next_seed;
	str = g_strdup_printf ("%d", current_seed);
	gtk_label_set_text (GTK_LABEL (seed_label), str);
	g_free (str);
	
	load_map (); /* assigns pos, and calculates dependencies */
	generate_game (current_seed); /* puts in the positions of the tiles */
}

static void
create_mahjongg_board (GtkWidget *mbox)
{
	gchar *buf, *buf2;
	
	canvas = gnome_canvas_new();

	gtk_box_pack_start_defaults (GTK_BOX (mbox), canvas);

	gtk_widget_set_usize (canvas, AREA_WIDTH, AREA_HEIGHT);
	gnome_canvas_set_pixels_per_unit (GNOME_CANVAS (canvas), 1);
	gnome_canvas_set_scroll_region (GNOME_CANVAS (canvas),
					0, 0, AREA_WIDTH, AREA_HEIGHT);
	gtk_widget_show (canvas);

	mapset = gconf_client_get_string (conf_client,
			"/apps/mahjongg/mapset", NULL);

	buf = gconf_client_get_string (conf_client,
			"/apps/mahjongg/bgcolour", NULL) ;
	set_backgnd_colour (buf) ;
	g_free (buf);

	buf = gconf_client_get_string (conf_client, "/apps/mahjongg/tileset", NULL);
	if (buf == NULL) {
		buf = g_strdup("default.png");
	}
	buf2 = gconf_client_get_string (conf_client, "/apps/mahjongg/background", NULL);
	if (buf2 == NULL) {
		buf2 = g_strdup("bg1.png");
	}
        
	popup_warn = gconf_client_get_bool (conf_client,
					    "/apps/mahjongg/warn", NULL);
	
	popup_confirm = gconf_client_get_bool (conf_client,
					       "/apps/mahjongg/confirm", NULL);

	do_game ();
	load_tiles (buf, buf2);

	create_canvas_items ();

	g_free (buf2);
	g_free (buf);
	init_game ();
	update_score_state ();
}

void
new_seed ()
{
	struct timeval t;
	gettimeofday (&t, NULL);

	next_seed = (guint) (t.tv_sec ^ t.tv_usec);
}

void
new_game (gboolean re_seed)
{
	gint i;

	if (re_seed)
		new_seed ();
	do_game ();
	load_images ();

	for (i = 0; i < MAX_TILES; i++) {
		change_tile_image (&tiles[i]);
		gnome_canvas_item_show (tiles[i].canvas_item);
	}

	init_game ();

	if (score_current_mapset != NULL)
		g_free (score_current_mapset);

	score_current_mapset = strdup (mapset);
	update_score_state ();
}

void
shuffle_tiles_callback (GtkWidget *widget, gpointer data)
{
        gint i, previous = 0, first=1, num_shuffle=0;
        tile temp;
        time_t seconds;

        if (paused || game_over == GAME_DEAD || game_over == GAME_WON) return;

        do {
                num_shuffle++;
                /* We do a circular permutation */
                for (i=0; i<MAX_TILES; i++) {
                        if (tiles[i].visible) {
                                if (first) {
                                        temp = tiles[i];
                                        first--; }
                                else {
                                        tiles[previous].type = tiles[i].type;
                                        tiles[previous].image = tiles[i].image;
                                }
                                previous = i; 
                        }
                }
                tiles[previous].type = temp.type;
                tiles[previous].image = temp.image;
        }
        while (!(update_moves_left ()) && num_shuffle < visible_tiles);
        
        if (num_shuffle >= visible_tiles) {
                GtkWidget *mb;
                game_over = GAME_DEAD;
                games_clock_stop (GAMES_CLOCK (chrono));
		mb = gtk_message_dialog_new (GTK_WINDOW (window),
					     GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
					     GTK_MESSAGE_ERROR,
					     GTK_BUTTONS_OK,
					     (_("Sorry, I was unable to find a playable configuration.")));
		gtk_dialog_run (GTK_DIALOG (mb));
		gtk_widget_destroy (mb);

        } else {
                
                for (i=0; i<MAX_TILES; i++) {
                        tiles[i].sequence = 0;
                        if (tiles[i].visible) {
                                change_tile_image (&tiles[i]);
                                gnome_canvas_item_show (tiles[i].canvas_item);
                        }
                }
                
                game_over = GAME_RUNNING;

                /* 60s penalty */
                games_clock_stop (GAMES_CLOCK(chrono));
                seconds = GAMES_CLOCK(chrono)->stopped;
                games_clock_set_seconds(GAMES_CLOCK(chrono), (gint) (seconds+60));
                games_clock_start (GAMES_CLOCK(chrono));
        }
}

int
main (int argc, char *argv [])
{
	GtkWidget *mbox;
	GtkWidget *chrono_label;
	GtkWidget *status_box;
	GtkWidget *group_box;
	gboolean show=TRUE;

	gnome_score_init (APPNAME);

	bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

	gnome_program_init (APPNAME, VERSION,
			    LIBGNOMEUI_MODULE,
			    argc, argv,
			    GNOME_PARAM_APP_DATADIR, DATADIR,
			    NULL);

	gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gnome-mahjongg.png");

	conf_client = gconf_client_get_default ();
	if (!games_gconf_sanity_check_string (conf_client, "/apps/mahjongg/tileset")) {
		return 1;
	}

	new_seed ();

	window = gnome_app_new (APPNAME, _(APPNAME_LONG));
	gtk_window_set_resizable (GTK_WINDOW (window), FALSE);

	/* Statusbar for a chrono, Tiles left and Moves left */
	status_box = gtk_hbox_new (FALSE, 10);

	group_box = gtk_hbox_new (FALSE, 0);
	tiles_label = gtk_label_new (_("Tiles Left: "));
	gtk_box_pack_start (GTK_BOX (group_box), tiles_label, FALSE, FALSE, 0);
	tiles_label = gtk_label_new (MAX_TILES_STR);
	gtk_box_pack_start (GTK_BOX (group_box), tiles_label, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (status_box), group_box, FALSE, FALSE, 0);

	group_box = gtk_hbox_new (FALSE, 0);
	moves_label = gtk_label_new(_("Moves Left: "));
	gtk_box_pack_start (GTK_BOX (group_box), moves_label, FALSE, FALSE, 0);
	moves_label = gtk_label_new (MAX_TILES_STR);
	gtk_box_pack_start (GTK_BOX (group_box), moves_label, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (status_box), group_box, FALSE, FALSE, 0);

	group_box = gtk_hbox_new (FALSE, 0);
	chrono_label = gtk_label_new (_("Time: "));
	gtk_box_pack_start (GTK_BOX (group_box), chrono_label, FALSE, FALSE, 0);
	chrono = games_clock_new ();
	gtk_box_pack_start (GTK_BOX (group_box), chrono, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (status_box), group_box, FALSE, FALSE, 0);

	group_box = gtk_hbox_new (FALSE, 0);
	seed_label = gtk_label_new (_("Seed: "));
	gtk_box_pack_start (GTK_BOX (group_box), seed_label, FALSE, FALSE, 0);
	seed_label = gtk_label_new ("");
	gtk_box_pack_start (GTK_BOX (group_box), seed_label, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (status_box), group_box, FALSE, FALSE, 0);

	/* show the status bar items */
	gtk_widget_show_all (status_box);

	appbar = gnome_appbar_new (FALSE, TRUE, GNOME_PREFERENCES_USER);
	gtk_box_pack_end(GTK_BOX(appbar), status_box, FALSE, FALSE, 0);
	gnome_app_set_statusbar (GNOME_APP (window), appbar);

	gnome_app_create_menus (GNOME_APP (window), mainmenu);
	gnome_app_install_menu_hints(GNOME_APP (window), mainmenu);

	gnome_app_create_toolbar (GNOME_APP (window), toolbar_uiinfo);

	g_signal_connect (G_OBJECT (window), "delete_event",
			  G_CALLBACK (delete_event_callback), (gpointer)QUIT_GAME);

	mbox = gtk_vbox_new (FALSE, 0);
	gnome_app_set_contents (GNOME_APP (window), mbox);
	create_mahjongg_board (mbox);

	gtk_widget_show (window);

	show = gconf_client_get_bool (conf_client,
			"/apps/mahjongg/show-toolbar", NULL);

	if (show) {
		gtk_check_menu_item_set_active
			(GTK_CHECK_MENU_ITEM(settingsmenu[0].widget), TRUE);
	} else {
		BonoboDockItem *gdi;
		
		gtk_check_menu_item_set_active
			(GTK_CHECK_MENU_ITEM(settingsmenu[0].widget), FALSE);
		gdi = gnome_app_get_dock_item_by_name
			(GNOME_APP (window), GNOME_APP_TOOLBAR_NAME);
		gtk_widget_hide(GTK_WIDGET(gdi)) ;
		gtk_widget_queue_resize (window);
	}

	init_config();

	score_current_mapset = strdup (mapset);
	update_score_state ();

  	gnome_app_flash (GNOME_APP (window), 
  				_("Welcome to GNOME Mahjongg!")); 

	gtk_main ();
	
	return 0;
}
