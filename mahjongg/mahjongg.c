/* -*- Mode: C; indent-tabs-mode: t; tab-width: 8; c-basic-offset: 8 -*- */

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
#include <glib/gi18n.h>
#include <gconf/gconf-client.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <games-clock.h>
#include <games-gconf.h>
#include <games-frame.h>
#include <games-stock.h>
#include <games-scores.h>
#include <games-scores-dialog.h>

#include "mahjongg.h"
#include "drawing.h"
#include "solubility.h"
#include "maps.h"

#define APPNAME "mahjongg"
#define APPNAME_LONG "Mahjongg"

/* #defines for the tile selection code. */
#define SELECTED_FLAG   1
#define HINT_FLAG       16

/* The number of half-cycles to blink during a hint (less 1) */
#define HINT_BLINK_NUM 5

tilepos *pos = 0;

tile tiles[MAX_TILES];

GtkWidget *window, *statusbar;
GtkWidget *tiles_label;
GtkWidget *toolbar;

GtkAction *pause_action;
GtkAction *hint_action;
GtkAction *redo_action;
GtkAction *undo_action;
GtkAction *restart_action;
GtkAction *scores_action;
GtkAction *show_toolbar_action;
GtkAction *fullscreen_action;
GtkAction *leavefullscreen_action;

gint selected_tile, visible_tiles;
gint sequence_number;

static gint windowwidth, windowheight;

GList * tileset_list = NULL;

gchar *tileset = NULL;
static gint mapset = -1;
static gchar *score_current_mapset = NULL;

static gchar *selected_tileset = NULL;

gboolean undo_state = FALSE;
gboolean redo_state = FALSE;

gint hint_tiles[2];
guint timer;
guint timeout_counter = HINT_BLINK_NUM + 1;

GtkWidget *moves_label;
gint moves_left=0;
GtkWidget *chrono;
gint paused=0;

/* for the preferences */
GConfClient *conf_client;
GtkWidget *warn_cb = NULL, *confirm_cb = NULL;
GtkWidget *colour_well = NULL;
GtkWidget *pref_dialog = NULL;

/* Has the map been changed ? */
gboolean new_map = TRUE;

GamesScores *highscores;

static const GamesScoresDescription scoredesc = {NULL,
						 "Easy",
						 "mahjongg",
						 GAMES_SCORES_STYLE_TIME_ASCENDING};

enum {
	GAME_RUNNING = 0,
	GAME_WAITING,
	GAME_WON,
	GAME_LOST,
	GAME_DEAD
} game_over = GAME_WAITING;

static void clear_undo_queue (void);
void you_won (void);
void check_free (void);
void undo_tile_callback (void);
void properties_callback (void);
void shuffle_tiles_callback (void);
void ensure_pause_off (void);
void pause_callback (void);
void new_game (void);
void restart_game (void);

void mahjongg_theme_warning (gchar *message)
{
	GtkWidget *dialog;
	GtkWidget *button;

	dialog = gtk_message_dialog_new (GTK_WINDOW (window),
					 GTK_DIALOG_DESTROY_WITH_PARENT,
					 GTK_MESSAGE_WARNING,
					 GTK_BUTTONS_CLOSE,
					 _("Could not load tile set"));
	gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), message);

	button = gtk_dialog_add_button (GTK_DIALOG (dialog),
					_("Preferences"), GTK_RESPONSE_ACCEPT
					);

	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT);

	g_signal_connect (button, "clicked", G_CALLBACK (properties_callback), NULL);

	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
}

static void
set_fullscreen_actions (gboolean is_fullscreen)
{
	gtk_action_set_sensitive (leavefullscreen_action, is_fullscreen);
	gtk_action_set_visible (leavefullscreen_action, is_fullscreen);

	gtk_action_set_sensitive (fullscreen_action, !is_fullscreen);
	gtk_action_set_visible (fullscreen_action, !is_fullscreen);
}

static void
fullscreen_callback (GtkAction *action)
{
	if (action == fullscreen_action)
		gtk_window_fullscreen (GTK_WINDOW (window));
	else
		gtk_window_unfullscreen (GTK_WINDOW (window));
}

static void
window_state_callback (GtkWidget *widget, GdkEventWindowState *event)
{
	if (!(event->changed_mask & GDK_WINDOW_STATE_FULLSCREEN))
		return;

	set_fullscreen_actions (event->new_window_state & 
				GDK_WINDOW_STATE_FULLSCREEN);
}

/* At the end of the game, hint, shuffle and pause all become unavailable. */
/* Undo and Redo are handled elsewhere. */
static void 
update_menu_sensitivities (void)
{
	gtk_action_set_sensitive (pause_action, 
				  game_over != GAME_WON &&
				  game_over != GAME_WAITING);
	gtk_action_set_sensitive (restart_action, undo_state);

	if (paused) {
		gtk_action_set_sensitive (hint_action, FALSE);
		gtk_action_set_sensitive (undo_action, FALSE);
		gtk_action_set_sensitive (redo_action, FALSE);
	} else {
		gtk_action_set_sensitive (hint_action, moves_left > 0);	
		gtk_action_set_sensitive (undo_action, undo_state);
		gtk_action_set_sensitive (redo_action, redo_state);
	}

}

static void clock_start (void)
{
	games_clock_start (GAMES_CLOCK (chrono));
	game_over = GAME_RUNNING;
	update_menu_sensitivities ();
}

/* Undo and redo sensitivity functionality. */
static void 
set_undoredo_state (gboolean undo, gboolean redo)
{
	undo_state = undo;
	redo_state = redo;

	update_menu_sensitivities ();
}

static void
tileset_callback (GtkWidget *widget, void *data)
{
	GList * entry;
	
	entry = g_list_nth (tileset_list,
			    gtk_combo_box_get_active (GTK_COMBO_BOX (widget)));	
	
	gconf_client_set_string (conf_client,
			"/apps/mahjongg/tileset",
			entry->data,
			NULL);
}

static void
tileset_changed_cb (GConfClient *client,
		    guint cnxn_id,
		    GConfEntry *entry,
		    gpointer user_data)
{
	char *tile_tmp = NULL;

	tile_tmp = gconf_client_get_string (conf_client,
			"/apps/mahjongg/tileset", NULL);
	if (tile_tmp) {
		if (strcmp (tile_tmp, selected_tileset) != 0) {
			g_free (selected_tileset);
			selected_tileset = tile_tmp;
			load_images (selected_tileset);
			draw_all_tiles ();
		} else {
			g_free (tile_tmp);
		}
	}
}

static void
show_toolbar_changed_cb (GConfClient *client,
	                 guint        cnxn_id,
			 GConfEntry  *entry,
			 gpointer     user_data)
{
	gboolean state;

	state = gconf_client_get_bool (conf_client,
	  		"/apps/mahjongg/show_toolbar", NULL);

	if (state)
		gtk_widget_show (toolbar);
	else
		gtk_widget_hide (toolbar);
}

static void
show_tb_callback (void)
{
	gboolean state;

	state = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (show_toolbar_action));

	gconf_client_set_bool (conf_client, "/apps/mahjongg/show_toolbar",
			       state, NULL);
}

static void
bg_colour_changed_cb (GConfClient *client,
		      guint cnxn_id,
		      GConfEntry *entry,
		      gpointer user_data)
{
	gchar *colour;
	
	colour = gconf_client_get_string (conf_client,
			"/apps/mahjongg/bgcolour", NULL);
	set_background (colour);
	if (colour_well != NULL)
	{
		gtk_color_button_set_color (GTK_COLOR_BUTTON(colour_well),
					    &bgcolour);
	}
	draw_all_tiles ();	
}

static void
bg_colour_callback (GtkWidget *widget, gpointer data)
{
	static char *tmp = "";
	GdkColor colour;

	gtk_color_button_get_color (GTK_COLOR_BUTTON (widget), &colour);

	tmp = g_strdup_printf ("#%04x%04x%04x", colour.red,
			       colour.green, colour.blue);

	gconf_client_set_string (conf_client,
			"/apps/mahjongg/bgcolour", tmp, NULL);
}

static gint get_mapset_index (void)
{
	gchar *mapset_name;
	gint newmapset = -1;
	gint i;

	mapset_name = gconf_client_get_string (conf_client,
					       "/apps/mahjongg/mapset",
					       NULL);
	for (i=0; i<nmaps; i++) 
		if (g_utf8_collate (mapset_name, maps[i].name) == 0) 
			newmapset = i;

	if (newmapset == -1) {  /* Oops: name not found. */
		if (mapset > 0) /* If we have a valid map, use it. */
			newmapset = mapset;
		else            /* Otherwise use the default. */
			newmapset = 0;
	} 

	g_free (mapset_name);

	return newmapset;
}

static void
mapset_changed_cb (GConfClient *client,
		   guint        cnxn_id,
		   GConfEntry  *entry,
		   gpointer     user_data)

{
	GtkWidget *dialog;
	gint response;


	mapset = get_mapset_index ();

	new_map = TRUE;

	/* Skip the dialog if a game isn't in play. */
	if (game_over || !games_clock_get_seconds(GAMES_CLOCK(chrono))) {
		new_game ();
		return;
	}

	dialog = gtk_message_dialog_new_with_markup (
		GTK_WINDOW (window),
		GTK_DIALOG_MODAL,
		GTK_MESSAGE_QUESTION,
		GTK_BUTTONS_NONE,
		"<b>%s</b>\n\n%s", 
		_("Do you want to finish the current game or start playing with the new map immediately?"),
		_("If you choose to finish with the old map then the next game will use the new map."));
	gtk_dialog_add_buttons (GTK_DIALOG (dialog),
				_("_Finish"), GTK_RESPONSE_REJECT,
				GTK_STOCK_NEW, GTK_RESPONSE_ACCEPT,
				NULL);
	gtk_dialog_set_default_response (GTK_DIALOG (dialog),
					 GTK_RESPONSE_ACCEPT);
	response = gtk_dialog_run (GTK_DIALOG (dialog));
	if (response == GTK_RESPONSE_ACCEPT)
		new_game ();
	gtk_widget_destroy (dialog);
}

static void
set_map_selection (GtkWidget *widget, void *data)
{
	gint target_mapset;

	target_mapset = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));
	
	gconf_client_set_string (conf_client,
				 "/apps/mahjongg/mapset",
				 maps[target_mapset].name, NULL);
}

static void
init_config (void)
{
	gconf_client_add_dir (conf_client,
			"/apps/mahjongg", GCONF_CLIENT_PRELOAD_ONELEVEL,
			NULL);
	gconf_client_notify_add (conf_client,
			"/apps/mahjongg/show_toolbar",
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
			"/apps/mahjongg/tileset",
			tileset_changed_cb,
			NULL, NULL, NULL);
}

static void
message (gchar *message)
{
	guint context_id;

	context_id = gtk_statusbar_get_context_id (GTK_STATUSBAR (statusbar), "message");
	gtk_statusbar_pop (GTK_STATUSBAR (statusbar), context_id);
	gtk_statusbar_push (GTK_STATUSBAR (statusbar), context_id, message);
}

static gboolean
message_flash_remove (guint flashid) {
	guint context_id;

	context_id = gtk_statusbar_get_context_id (GTK_STATUSBAR (statusbar), "flash");
        gtk_statusbar_remove (GTK_STATUSBAR (statusbar), context_id, flashid);
	return FALSE;
}

static void
message_flash (gchar *message)
{
	guint flashid;
	guint context_id;

	context_id = gtk_statusbar_get_context_id (GTK_STATUSBAR (statusbar), "flash");
	flashid = gtk_statusbar_push (GTK_STATUSBAR (statusbar), context_id, message);
	g_timeout_add (5000, (GSourceFunc)message_flash_remove, GUINT_TO_POINTER(flashid));
}

static void 
update_score_state ()
{
	GList *top;

	top = games_scores_get (highscores);
	gtk_action_set_sensitive (scores_action, top != NULL);
}

static gint
update_moves_left (void)
{
        char *tmpstr;

        check_free ();
	tmpstr = g_strdup_printf ("%2d", moves_left);
        gtk_label_set_text (GTK_LABEL (moves_label), tmpstr);
	g_free (tmpstr);

        return moves_left;
}

static void
select_tile (gint tileno)
{
        tiles[tileno].selected |= SELECTED_FLAG;
        draw_tile (tileno);
        selected_tile = tileno;
}

static void
unselect_tile (gint tileno)
{
        selected_tile = MAX_TILES + 1;
        tiles[tileno].selected &= ~SELECTED_FLAG;
        draw_tile (tileno);
}

static void
remove_pair (gint tile1, gint tile2)
{
	gchar * tmpstr;
	
	tiles[tile1].visible = tiles[tile2].visible = 0;
	tiles[tile1].selected &= ~SELECTED_FLAG;
	tiles[tile2].selected &= ~SELECTED_FLAG;
	draw_tile (tile1);
	draw_tile (tile2);
	clear_undo_queue ();
	tiles[tile1].sequence = tiles[tile2].sequence = sequence_number;
	sequence_number ++;
	selected_tile = MAX_TILES + 1;
	visible_tiles -= 2;
	tmpstr = g_strdup_printf("%3d", visible_tiles);
	gtk_label_set_text (GTK_LABEL(tiles_label), tmpstr);
	g_free (tmpstr);
	update_moves_left ();
	set_undoredo_state (TRUE, FALSE);
	
	if (visible_tiles <= 0) {
		games_clock_stop(GAMES_CLOCK(chrono));
		you_won ();
	}

}

void
tile_event (gint tileno, gint button)
{
	if (paused) {
		pause_callback ();
		return;
	}

	if (!tile_free (tileno))
		return;

	if (!games_clock_get_seconds (GAMES_CLOCK(chrono))) 
		clock_start ();
	
	switch (button) {
	case 1:
		if (tiles[tileno].selected & SELECTED_FLAG) {
			unselect_tile (tileno);
			return;
		}
		if (selected_tile >= MAX_TILES) {
			select_tile (tileno);
			return;
		}
		if ((tiles[selected_tile].type == tiles[tileno].type) ) {
			remove_pair (selected_tile, tileno);
			return;
		}
		/* Note the fallthrough, if the tiles don't match,
		 * we just select the new tile. */
	case 3:
		if (selected_tile < MAX_TILES) 
			unselect_tile (selected_tile);
		select_tile (tileno);
		
	default: 
		break;
	}
}

static void
fill_tile_menu (GtkWidget *menu, gchar *sdir)
{
	struct dirent *e;
	DIR *dir;
        gint itemno = 0;
	gchar *dname = NULL;

	if (tileset_list) {
		g_list_foreach (tileset_list,
				(GFunc) g_free,
				NULL);
		g_list_free (tileset_list);
	}

	tileset_list = NULL;
	
	dname = gnome_program_locate_file (NULL, GNOME_FILE_DOMAIN_APP_PIXMAP,
			(sdir), FALSE, NULL);
	dir = opendir (dname);

	if (!dir) {
		g_free (dname);
		return;
	}

	while ((e = readdir (dir)) != NULL){
		gchar *s = g_strdup (e->d_name);

		if (!(g_strrstr (s, ".xpm") ||
		      g_strrstr (s, ".svg") ||
		      g_strrstr (s, ".gif") ||
		      g_strrstr (s, ".png") ||
		      g_strrstr (s, ".jpg") ||
		      g_strrstr (s, ".xbm"))){
			g_free (s);
			continue;
		}

		gtk_combo_box_append_text (GTK_COMBO_BOX (menu), s);
		tileset_list = g_list_append (tileset_list, s);
		if (!strcmp(tileset, s)) {
			gtk_combo_box_set_active(GTK_COMBO_BOX (menu), itemno);
		}
		
	        itemno++;
	}

	closedir (dir);
	g_free (dname);
}

static void
fill_map_menu (GtkWidget *menu)
{
	gint lp;

	for (lp=0; lp<nmaps; lp++) {
		gtk_combo_box_append_text (GTK_COMBO_BOX (menu), 
					   Q_(maps[lp].name)) ;
	}
	gtk_combo_box_set_active (GTK_COMBO_BOX (menu), mapset);
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

		update_menu_sensitivities ();
                if (!game_over) {
			mb = gtk_message_dialog_new (GTK_WINDOW (window),
						     GTK_DIALOG_MODAL
						     | GTK_DIALOG_DESTROY_WITH_PARENT,
						     GTK_MESSAGE_INFO,
						     GTK_BUTTONS_NONE,
						     (_("There are no more moves.")));
			gtk_dialog_add_buttons (GTK_DIALOG (mb),
						GTK_STOCK_UNDO,
						GTK_RESPONSE_REJECT,
						_("Shuffle"),
						GTK_RESPONSE_ACCEPT,
						NULL);
			gtk_dialog_set_default_response (GTK_DIALOG (mb),
							 GTK_RESPONSE_ACCEPT);
			if (gtk_dialog_run (GTK_DIALOG (mb)) == GTK_RESPONSE_ACCEPT)
				shuffle_tiles_callback ();
			else
				undo_tile_callback ();

			gtk_widget_destroy (mb);
                }
 	} 
}

void
you_won (void)
{
        gint pos;
        time_t seconds;
	GamesScoreValue score;
        static GtkWidget *dialog = NULL;
	gchar *message;
	
        game_over = GAME_WON;

        seconds = games_clock_get_seconds (GAMES_CLOCK (chrono));

        score.time_double = (seconds / 60) * 1.0 + (seconds % 60) / 100.0;

	pos = games_scores_add_score (highscores, score);
	update_menu_sensitivities ();
        if (pos > 0) {
		if (dialog) {
			gtk_window_present (GTK_WINDOW (dialog));
		} else {
			dialog = games_scores_dialog_new (highscores, _("Mahjongg Scores"));
			games_scores_dialog_set_category_description (GAMES_SCORES_DIALOG (dialog),
							      _("Map:"));
			message = g_strdup_printf ("<b>%s</b>\n\n%s",
						   _("Congratulations!"),
						   _("Your score has made the top ten."));
			games_scores_dialog_set_message (GAMES_SCORES_DIALOG (dialog), message);
			g_free (message);
		}
		games_scores_dialog_set_hilight (GAMES_SCORES_DIALOG (dialog), pos);
		/* FIXME: Quit / New Game choice. */

		gtk_dialog_run  (GTK_DIALOG (dialog));
		gtk_widget_hide (dialog);
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

void
properties_callback (void)
{
	GtkWidget *omenu;
	GtkWidget *frame, *table, *widget, *label;
	GtkSizeGroup *group;
	GtkWidget *top_table;

	if (pref_dialog) {
		gtk_window_present (GTK_WINDOW (pref_dialog));
		return;
	}

	pref_dialog = gtk_dialog_new_with_buttons (_("Mahjongg Preferences"),
			GTK_WINDOW (window),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
			NULL);
	gtk_container_set_border_width (GTK_CONTAINER (pref_dialog), 5);
	gtk_box_set_spacing (GTK_BOX (GTK_DIALOG (pref_dialog)->vbox), 2);
	gtk_dialog_set_has_separator (GTK_DIALOG (pref_dialog), FALSE);
	gtk_window_set_resizable (GTK_WINDOW (pref_dialog), FALSE);
	gtk_dialog_set_default_response (GTK_DIALOG (pref_dialog),
					 GTK_RESPONSE_CLOSE);
	g_signal_connect (G_OBJECT (pref_dialog), "response",
			  G_CALLBACK (pref_dialog_response), NULL);

	group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

	top_table = gtk_table_new (4, 1, FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (top_table), 5);
	gtk_table_set_row_spacings (GTK_TABLE (top_table), 18);
	gtk_table_set_col_spacings (GTK_TABLE (top_table), 0);

	frame = games_frame_new (_("Tiles"));
	gtk_table_attach_defaults (GTK_TABLE (top_table), frame, 0, 1, 0, 1);

	table = gtk_table_new (2, 2, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (table), 6);
	gtk_table_set_col_spacings (GTK_TABLE (table), 12);
	
	label = gtk_label_new_with_mnemonic (_("_Tile set:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
	                  (GtkAttachOptions) GTK_FILL, 
			  (GtkAttachOptions) 0,
			  0, 0);

	omenu = gtk_combo_box_new_text ();
	fill_tile_menu (omenu, "mahjongg");
	g_signal_connect (G_OBJECT (omenu), "changed",
			  G_CALLBACK (tileset_callback), NULL); 
	gtk_table_attach_defaults (GTK_TABLE (table), omenu, 1, 2, 0, 1);
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), omenu);

	gtk_container_add(GTK_CONTAINER (frame), table);

	frame = games_frame_new (_("Maps"));
	gtk_table_attach_defaults (GTK_TABLE (top_table), frame, 0, 1, 1, 2);

	table = gtk_table_new (1, 2, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (table), 6);
	gtk_table_set_col_spacings (GTK_TABLE (table), 12);
	
	label = gtk_label_new_with_mnemonic (_("_Select map:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
	                  (GtkAttachOptions) GTK_FILL, 
			  (GtkAttachOptions) 0,
			  0, 0);

	omenu = gtk_combo_box_new_text ();
	fill_map_menu (omenu);
	g_signal_connect (G_OBJECT (omenu), "changed",
			  G_CALLBACK (set_map_selection), NULL); 
	gtk_table_attach_defaults (GTK_TABLE (table), omenu, 1, 2, 0, 1);
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), omenu);

	gtk_container_add(GTK_CONTAINER (frame), table);

	frame = games_frame_new (_("Colors"));
	gtk_table_attach_defaults (GTK_TABLE (top_table), frame, 0, 1, 2, 3);

	table = gtk_table_new (1, 2, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (table), 6);
	gtk_table_set_col_spacings (GTK_TABLE (table), 12);
	
	label = gtk_label_new_with_mnemonic (_("_Background color:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
	                  (GtkAttachOptions) GTK_FILL, 
			  (GtkAttachOptions) 0,
			  0, 0);

	widget  = gtk_color_button_new ();
	gtk_color_button_set_color (GTK_COLOR_BUTTON (widget), &bgcolour);
	g_signal_connect (G_OBJECT (widget), "color_set",
			  G_CALLBACK (bg_colour_callback), NULL);
	gtk_table_attach_defaults (GTK_TABLE (table), widget, 1, 2, 0, 1);
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), widget);

	gtk_container_add (GTK_CONTAINER (frame), table);

	gtk_box_pack_start_defaults (GTK_BOX (GTK_DIALOG (pref_dialog)->vbox), top_table);

	g_object_unref (group);
	gtk_widget_show_all (pref_dialog);
}

static gint
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
	draw_tile (hint_tiles[0]);
	draw_tile (hint_tiles[1]);
	
	return 1;
}

static void
stop_hints (void)
{
	if (timeout_counter > HINT_BLINK_NUM)
		return;

	timeout_counter = HINT_BLINK_NUM + 1;
	tiles[hint_tiles[0]].selected &= ~HINT_FLAG;
	tiles[hint_tiles[1]].selected &= ~HINT_FLAG;
	draw_tile (hint_tiles[0]);
	draw_tile (hint_tiles[1]);
	g_source_remove (timer);
}

static void
hint_callback (void)
{
        gint i, j, free=0, type ;

        if (paused || (game_over != GAME_RUNNING && game_over != GAME_WAITING))
                return;

	/* This prevents the flashing speeding up if the hint button is
	 * pressed multiple times. */
	if (timeout_counter <= HINT_BLINK_NUM)
		return;

	/* Snarfed from check free
	 * Tile Free is now _so_ much quicker, it is more elegant to do a
	 * British Library search, and safer. */
	/* Note: British Library should probably read British Museum.  */

	/* Clear any selection */
	if (selected_tile < MAX_TILES) {
		tiles[selected_tile].selected = 0;
		draw_tile (selected_tile);
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
					draw_tile (i);
					draw_tile (j);
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
	timer = g_timeout_add (250, (GSourceFunc) hint_timeout, NULL);
                
	/* 30s penalty */
	games_clock_add_seconds(GAMES_CLOCK(chrono), 30);
	clock_start ();
}

static void
about_callback (void)
{
	const gchar *authors [] = {
		_("Main game:"),
		"Francisco Bustamante",
		"Max Watson",
		"Heinz Hempe",
		"Michael Meeks",
                "Philippe Chavin",
		"Callum McKenzie",
		"",
		_("Maps:"),
		"Rexford Newbould",
		"Krzysztof Foltman",
		NULL
	};

	const gchar *artists [] = {
		_("Tiles:"),
		"Jonathan Buzzard",
		"Jim Evans",
		"Richard Hoelscher",
		"Gonzalo Odiard",
		"Max Watson",
		NULL
	};

	const gchar *documenters [] = {
		"Eric Baudais",
		NULL
	};

	gtk_show_about_dialog (GTK_WINDOW (window),
			       "name", _("Mahjongg"), 
			       "version", VERSION,
			       "comments", _("A matching game played with Mahjongg tiles."),
			       "copyright", "Copyright \xc2\xa9 1998-2005 Free Software Foundation, Inc.",
			       "license", "GPL 2+",
			       "authors", authors,
			       "artists", artists,
			       "documenters", documenters,
			       "translator_credits", _("translator-credits"),
			       "logo-icon-name", "gnome-mahjongg",
			       "website", "http://www.gnome.org/projects/gnome-games/",
			       "wrap-license", TRUE,
			       NULL);
}

void
pause_callback (void)
{
	static gboolean noloops = FALSE;

	/* The calls to set the menu bar toggle-button will
         * trigger another callback, which will trigger another
	 * callback ... this must be stopped. */
	if (noloops)
                return;

	noloops = TRUE;
        stop_hints ();
        paused = !paused;
	draw_all_tiles ();
        update_menu_sensitivities ();
	if (paused) {
		gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (pause_action), TRUE);
                games_clock_stop (GAMES_CLOCK (chrono));
                message(_("Game paused"));
        }
        else {
		gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (pause_action), FALSE);
                clock_start ();
                message ("");
        }
	noloops = FALSE;
}

void ensure_pause_off (void)
{
        if (paused)
		pause_callback ();
}

static void
scores_callback (GtkAction *action, gpointer data)
{
	static GtkWidget *dialog = NULL;

	if (dialog) {
		gtk_window_present (GTK_WINDOW (dialog));
	} else {
		dialog = games_scores_dialog_new (highscores, _("Mahjongg Scores"));
		games_scores_dialog_set_category_description (GAMES_SCORES_DIALOG (dialog),
							      _("Map:"));
	}

	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_hide (dialog);
}

static void
init_game (void)
{
	gchar *newtitle;

	newtitle = g_strdup_printf (_("%s - %s"), _("Mahjongg"), 
				    Q_(maps[mapset].name));
	gtk_window_set_title (GTK_WINDOW (window), newtitle);
	g_free (newtitle);

	score_current_mapset = maps[mapset].score_name;

        gtk_label_set_text (GTK_LABEL (tiles_label), MAX_TILES_STR);
        update_moves_left ();
        game_over = GAME_WAITING;
        sequence_number = 1 ;
        visible_tiles = MAX_TILES;
        selected_tile = MAX_TILES + 1;
	set_undoredo_state (FALSE, FALSE);

        games_clock_stop (GAMES_CLOCK (chrono));
        games_clock_set_seconds (GAMES_CLOCK (chrono), 0);
}

static void 
new_game_cb (GtkAction *action, gpointer data)
{
	stop_hints ();
	ensure_pause_off ();
	new_game ();
}

static void 
restart_game_cb (GtkAction *action, gpointer data)
{
	stop_hints ();
	restart_game ();
}

static void quit_cb (GObject *object, gpointer data)
{
	gtk_main_quit ();
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
    }
    draw_all_tiles ();
    init_game ();
}

static void
redo_tile_callback (GtkAction *action, gpointer data)
{
        gint i, change ;
        char *tmpstr;
	gboolean found;
        
        if (paused) 
                return; 
        if (sequence_number>(MAX_TILES/2))
                return ;
        
	stop_hints ();
        
        if (selected_tile<MAX_TILES) {
                tiles[selected_tile].selected = 0 ;
                draw_tile(selected_tile);
                selected_tile = MAX_TILES + 1; 
        }
        change = 0 ;
        for (i=0; i<MAX_TILES; i++)
                if (tiles[i].sequence == sequence_number) {
                        tiles[i].selected = 0 ;
                        tiles[i].visible = 0 ;
			draw_tile (i);
                        visible_tiles-- ;
                        change = 1 ;
                }
        if (change) {
                if (sequence_number < MAX_TILES)
                        sequence_number++ ;
        }

        tmpstr = g_strdup_printf ("%3d",visible_tiles);
        gtk_label_set_text(GTK_LABEL (tiles_label), tmpstr);

	found = FALSE;
    	for (i=0; i<MAX_TILES; i++) {
		if (tiles[i].sequence == sequence_number) {
			found = TRUE;
			break;
		}
	}
	set_undoredo_state (TRUE, found);
	
        update_moves_left ();
}

void
undo_tile_callback (void)
{
        gint i;
        char *tmpstr;
        
        if (paused || game_over == GAME_WON) 
                return;
        if (game_over == GAME_LOST)
                game_over = GAME_RUNNING;
        stop_hints ();

        if (sequence_number>1)
                sequence_number-- ;
        else
                return ;
        
        if (selected_tile<MAX_TILES) {
                tiles[selected_tile].selected = 0 ;
                draw_tile (selected_tile);
                selected_tile = MAX_TILES + 1; 
        }
        
        for (i=0; i<MAX_TILES; i++)
                if (tiles[i].sequence == sequence_number) {
                        tiles[i].selected = 0 ;
                        tiles[i].visible = 1 ;
                        visible_tiles++ ;
			draw_tile (i);
                }

        tmpstr = g_strdup_printf ("%3d", visible_tiles);
        gtk_label_set_text (GTK_LABEL(tiles_label), tmpstr);
	g_free (tmpstr);

	set_undoredo_state (sequence_number>1, TRUE);
        update_moves_left ();
}

/* You loose your re-do queue when you make a move */
static void
clear_undo_queue (void)
{
	gint lp ;

	for (lp=0;lp<MAX_TILES;lp++)
		if (tiles[lp].sequence>=sequence_number)
			tiles[lp].sequence = 0 ;
}

static void
load_map (void)
{
	new_map = FALSE;		
	pos = maps[mapset].map ;
	generate_dependencies ();
	calculate_view_geometry ();
	configure_pixmaps (); 
}

static void
do_game (void)
{
	games_scores_set_category (highscores, maps[mapset].score_name);
	if (new_map)
		load_map ();
	generate_game (g_random_int ()); /* puts in the positions of the tiles */
}

/* Record any changes to our window size. */
static gboolean window_configure_cb (GtkWidget *w, GdkEventConfigure *e,
				 gpointer data)
{
	gconf_client_set_int (conf_client, "/apps/mahjongg/width",
			      e->width, NULL);
	gconf_client_set_int (conf_client, "/apps/mahjongg/height",
			      e->height, NULL);

	return FALSE;
}

static void
load_preferences (void)
{
	gchar *buf;
	
	mapset = get_mapset_index ();

	buf = gconf_client_get_string (conf_client,
			"/apps/mahjongg/bgcolour", NULL) ;
	set_background (buf) ;
	g_free (buf);

	buf = gconf_client_get_string (conf_client, "/apps/mahjongg/tileset", NULL);
	if (buf == NULL) {
		buf = g_strdup("default.png");
	}
	selected_tileset = g_strdup (buf);
	g_free (buf);	
	
	windowwidth = gconf_client_get_int (conf_client,
					    "/apps/mahjongg/width", NULL);
	if (windowwidth <= 0)
		windowwidth = 530;
	
	windowheight = gconf_client_get_int (conf_client,
					     "/apps/mahjongg/height", NULL);
	if (windowheight <= 0)
		windowheight = 440;

	
	load_images (selected_tileset);
}

void
new_game (void)
{
	do_game ();

	draw_all_tiles ();

	init_game ();

	update_score_state ();
}

void
shuffle_tiles_callback (void)
{
	gboolean ok;

        if (paused || game_over == GAME_DEAD || game_over == GAME_WON) return;

        stop_hints ();

	/* Make sure no tiles are selected. */
	if (selected_tile < MAX_TILES) {
		unselect_tile(selected_tile);
	}

	ok = shuffle ();
   
	if (!ok) {
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

                draw_all_tiles ();
                
                game_over = GAME_RUNNING;

                /* 60s penalty */
                games_clock_add_seconds(GAMES_CLOCK(chrono), 60);

		update_moves_left ();
		/* Disable undo/redo after a shuffle. */
		sequence_number = 1;
		clear_undo_queue ();
		set_undoredo_state (FALSE, FALSE);
        }

	update_menu_sensitivities ();
}

static void
help_cb (GtkAction *action, gpointer data)
{
        gnome_help_display ("mahjongg.xml", NULL, NULL);
}

static const GtkActionEntry actions[] = {
        { "GameMenu", NULL, N_("_Game") },
        { "SettingsMenu", NULL, N_("_Settings") },
        { "HelpMenu", NULL, N_("_Help") },
        { "NewGame", GAMES_STOCK_NEW_GAME, NULL, NULL, N_("Start a new game"), G_CALLBACK (new_game_cb) },
        { "RestartGame", GAMES_STOCK_RESTART_GAME, NULL, NULL, N_("Restart the current game"), G_CALLBACK (restart_game_cb) },
        { "UndoMove", GAMES_STOCK_UNDO_MOVE, NULL, NULL, N_("Undo the last move"), G_CALLBACK (undo_tile_callback) },
        { "RedoMove", GAMES_STOCK_REDO_MOVE, NULL, NULL, N_("Redo the last move"), G_CALLBACK (redo_tile_callback) },
        { "Hint", GAMES_STOCK_HINT, NULL, NULL, N_("Show a hint"), G_CALLBACK (hint_callback) },
        { "Scores", GAMES_STOCK_SCORES, NULL, NULL, NULL, G_CALLBACK (scores_callback) },
        { "Quit", GTK_STOCK_QUIT, NULL, NULL, NULL, G_CALLBACK (quit_cb) },
	{ "Fullscreen", GAMES_STOCK_FULLSCREEN, NULL, NULL, NULL, G_CALLBACK (fullscreen_callback) },
	{ "LeaveFullscreen", GAMES_STOCK_LEAVE_FULLSCREEN, NULL, NULL, NULL, G_CALLBACK (fullscreen_callback) },
        { "Preferences", GTK_STOCK_PREFERENCES, NULL, NULL, NULL, G_CALLBACK (properties_callback) },
        { "Contents", GAMES_STOCK_CONTENTS, NULL, NULL, NULL, G_CALLBACK (help_cb) },
        { "About", GTK_STOCK_ABOUT, NULL, NULL, NULL, G_CALLBACK (about_callback) }
};

static const GtkToggleActionEntry toggle_actions[] = {
        { "ShowToolbar", NULL, N_("_Toolbar"), NULL, N_("Show or hide the toolbar"), G_CALLBACK (show_tb_callback) },
	{ "PauseGame", GAMES_STOCK_PAUSE_GAME, NULL, NULL, N_("Pause or continue the game"), G_CALLBACK (pause_callback) }
};

static const char ui_description[] =
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu action='GameMenu'>"
"      <menuitem action='NewGame'/>"
"      <menuitem action='RestartGame'/>"
"      <menuitem action='PauseGame'/>"
"      <separator/>"
"      <menuitem action='UndoMove'/>"
"      <menuitem action='RedoMove'/>"
"      <menuitem action='Hint'/>"
"      <separator/>"
"      <menuitem action='Scores'/>"
"      <separator/>"
"      <menuitem action='Quit'/>"
"    </menu>"
"    <menu action='SettingsMenu'>"
"      <menuitem action='Fullscreen'/>"
"      <menuitem action='LeaveFullscreen'/>"
"      <menuitem action='ShowToolbar'/>"
"      <separator/>"
"      <menuitem action='Preferences'/>"
"    </menu>"
"    <menu action='HelpMenu'>"
"      <menuitem action='Contents'/>"
"      <menuitem action='About'/>"
"    </menu>"
"  </menubar>"
"  <toolbar name='Toolbar'>"
"    <toolitem action='NewGame'/>"
"    <toolitem action='RestartGame'/>"
"    <toolitem action='PauseGame'/>"
"    <separator/>"
"    <toolitem action='UndoMove'/>"
"    <toolitem action='RedoMove'/>"
"    <toolitem action='Hint'/>"
"    <toolitem action='LeaveFullscreen'/>"
"  </toolbar>"
"</ui>";


static void
create_menus (GtkUIManager *ui_manager)
{
        GtkActionGroup *action_group;

        action_group = gtk_action_group_new ("group");

        gtk_action_group_set_translation_domain(action_group, GETTEXT_PACKAGE);
        gtk_action_group_add_actions (action_group, actions, G_N_ELEMENTS (actions), window);
        gtk_action_group_add_toggle_actions (action_group, toggle_actions, G_N_ELEMENTS (toggle_actions), window);

        gtk_ui_manager_insert_action_group (ui_manager, action_group, 0);
        gtk_ui_manager_add_ui_from_string (ui_manager, ui_description, -1, NULL);
	restart_action = gtk_action_group_get_action (action_group, "RestartGame");
	pause_action = gtk_action_group_get_action (action_group, "PauseGame");
	hint_action = gtk_action_group_get_action (action_group, "Hint");
	undo_action = gtk_action_group_get_action (action_group, "UndoMove");
	redo_action = gtk_action_group_get_action (action_group, "RedoMove");
	scores_action = gtk_action_group_get_action (action_group, "Scores");
	show_toolbar_action = gtk_action_group_get_action (action_group, "ShowToolbar");

	fullscreen_action = gtk_action_group_get_action (action_group, "Fullscreen");
	leavefullscreen_action = gtk_action_group_get_action (action_group, "LeaveFullscreen");
	set_fullscreen_actions (FALSE);

        gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (show_toolbar_action),
        			      gconf_client_get_bool (conf_client, "/apps/mahjongg/show_toolbar", NULL));
}

static void init_scores (void)
{
	int i;

	highscores = games_scores_new (&scoredesc);

	for (i=0; i<nmaps; i++) {
		games_scores_add_category (highscores, maps[i].score_name, 
					   maps[i].name);
	}
}

int
main (int argc, char *argv [])
{
	GtkWidget *vbox;
	GtkWidget *box;
	GtkWidget *board;
	GtkWidget *chrono_label;
	GtkWidget *status_box;
	GtkWidget *group_box;
	GtkWidget *spacer;

	GtkUIManager *ui_manager;
	GtkAccelGroup *accel_group;

	GnomeProgram *program;

	setgid_io_init ();

	bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

	program = gnome_program_init (APPNAME, VERSION,
				      LIBGNOMEUI_MODULE,
				      argc, argv,
				      GNOME_PARAM_APP_DATADIR, DATADIR,
				      NULL);

	games_stock_init ();

	gtk_window_set_default_icon_name ("gnome-mahjongg");

	load_maps ();
	init_scores ();

	conf_client = gconf_client_get_default ();
	if (!games_gconf_sanity_check_string (conf_client, "/apps/mahjongg/tileset")) {
		return 1;
	}
	
	window = gnome_app_new (APPNAME, _(APPNAME_LONG));

        load_preferences ();
        
	gtk_window_set_default_size (GTK_WINDOW (window), windowwidth,
				     windowheight);
	g_signal_connect (G_OBJECT (window), "configure_event",
			  G_CALLBACK (window_configure_cb), NULL);
	g_signal_connect (G_OBJECT (window), "window_state_event",
			  G_CALLBACK (window_state_callback), NULL);
	
	/* Statusbar for a chrono, Tiles left and Moves left */
	status_box = gtk_hbox_new (FALSE, 10);

	group_box = gtk_hbox_new (FALSE, 0);
	tiles_label = gtk_label_new (_("Tiles Left:"));
	gtk_box_pack_start (GTK_BOX (group_box), tiles_label, FALSE, FALSE, 0);
	spacer = gtk_label_new (" ");
	gtk_box_pack_start (GTK_BOX (group_box), spacer, FALSE, FALSE, 0);
	tiles_label = gtk_label_new (MAX_TILES_STR);
	gtk_box_pack_start (GTK_BOX (group_box), tiles_label, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (status_box), group_box, FALSE, FALSE, 0);

	group_box = gtk_hbox_new (FALSE, 0);
	moves_label = gtk_label_new(_("Moves Left:"));
	gtk_box_pack_start (GTK_BOX (group_box), moves_label, FALSE, FALSE, 0);
	spacer = gtk_label_new (" ");
	gtk_box_pack_start (GTK_BOX (group_box), spacer, FALSE, FALSE, 0);
	moves_label = gtk_label_new (MAX_TILES_STR);
	gtk_box_pack_start (GTK_BOX (group_box), moves_label, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (status_box), group_box, FALSE, FALSE, 0);

	group_box = gtk_hbox_new (FALSE, 0);
	chrono_label = gtk_label_new (_("Time:"));
	gtk_box_pack_start (GTK_BOX (group_box), chrono_label, FALSE, FALSE, 0);
	spacer = gtk_label_new (" ");
	gtk_box_pack_start (GTK_BOX (group_box), spacer, FALSE, FALSE, 0);
	chrono = games_clock_new ();
	gtk_box_pack_start (GTK_BOX (group_box), chrono, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (status_box), group_box, FALSE, FALSE, 0);

	/* show the status bar items */
	statusbar = gtk_statusbar_new();
	ui_manager = gtk_ui_manager_new ();

	games_stock_prepare_for_statusbar_tooltips (ui_manager, statusbar);
	gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR (statusbar), FALSE);

	create_menus (ui_manager);
	accel_group = gtk_ui_manager_get_accel_group (ui_manager);
	gtk_window_add_accel_group (GTK_WINDOW (window), accel_group);
	box = gtk_ui_manager_get_widget (ui_manager, "/MainMenu");

	g_signal_connect (G_OBJECT (window), "delete_event",
			  G_CALLBACK (quit_cb), NULL);

	board = create_mahjongg_board ();

	toolbar = gtk_ui_manager_get_widget (ui_manager, "/Toolbar");

	vbox = gtk_vbox_new (FALSE, 0);

	gtk_box_pack_start (GTK_BOX (vbox), box, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), toolbar, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), board, TRUE, TRUE, 0);
	
	gtk_box_pack_end(GTK_BOX(statusbar), status_box, FALSE, FALSE, 0);
	gtk_box_pack_end(GTK_BOX(vbox), statusbar, FALSE, FALSE, 0);
        
	gnome_app_set_contents (GNOME_APP (window), vbox);

	/* FIXME: get these in the best place (as per the comment below. */
	init_config ();

	do_game ();
	init_game ();
	update_score_state ();
	
	/* Note: we have to have a layout loaded before here so that the
	 * window knows how big to make the tiles. */

	gtk_widget_show_all (window);

	if (!gconf_client_get_bool (conf_client, "/apps/mahjongg/show_toolbar", NULL))
		gtk_widget_hide (toolbar);

  	message_flash (_("Remove matching pairs of tiles.")); 

	gtk_main ();

	gnome_accelerators_sync();

	g_object_unref (program);
	
	return 0;
}
