/* (C) 2003/2004 Sjoerd Langkemper
 * (C) 1999-2003 Chris Rogers
 * properties.c - properties dialog and info 
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * For more details see the file COPYING.
 */

#include "config.h"
#include <gnome.h>
#include <string.h>
#include <dirent.h>
#include <games-clock.h>
#include <games-frame.h>
#include <gconf/gconf-client.h>
#include "gtkgridboard.h"

#include "games-files.h"

#include "properties.h"
#include "gataxx.h"

PropertiesData _props;
PropertiesData * props=&_props;

static GtkWidget *propbox = NULL;
static GtkWindow * mainwindow = NULL;
static GtkTooltips *tooltips = NULL;
static GamesFileList * theme_file_list = NULL;

/* makes sure input lies between low and high */
static gint clamp_int (gint input, gint low, gint high)
{
	if (input < low)
		input = low;
	if (input > high)
		input = high;

	return input;
}

/* functions to get things from the PropertiesData struct */

int props_is_human(int piece) {
	return props_get_level(piece)==0;
}

int props_get_level(int piece) {
	if (piece==WHITE) return props_get_white_level();
		     else return props_get_black_level();
}

int props_get_white_level() {
	return props->white_level;
}

int props_get_black_level() {
	return props->black_level;
}

gboolean props_get_flip_final() {
	return props->flip_final;
}

gboolean props_get_show_grid() {
	return props->show_grid;
}

gboolean props_get_animate() {
	return props->animate;
}

gboolean props_get_quick_moves() {
	return props->quick_moves;
}

gchar * props_get_tile_set() {
	return props->tile_set;
}

/* functions to get things from gconf */
static int props_get_int(const char * setting, int def, int low, int high) {
	int result;
	GError * error=NULL;

	result=gconf_client_get_int(get_gconf_client(), get_gconf_uri(setting), &error);
	if (error!=NULL) {
		g_error_free(error);
		return def;
	}
	clamp_int(result, low, high);
	return result;
}

static gboolean props_get_bool(gchar * key, gboolean def) {
	GError * error=NULL;
	gboolean result;

	result=gconf_client_get_bool(get_gconf_client(), get_gconf_uri(key), &error);
	if (error!=NULL) {
		g_error_free(error);
		return def;
	}

	return result;
}

/* FIXME: watch this: */
/* Returns gchar* you should free with g_free(). Returns NULL on not-found
 * or other error. */
static gchar * props_get_string(gchar *key)
{
	GError *error = NULL;
	gchar *retval;

	retval = gconf_client_get_string (get_gconf_client(), get_gconf_uri(key), &error);
	if (error != NULL) {
		g_error_free (error);
		return NULL;
	}

	return retval;
}

void load_properties (void)
{
	props->black_level=props_get_int("blacklevel", 0, 0, 3);
	props->white_level=props_get_int("whitelevel", 0, 0, 3);

	props->tile_set=props_get_string("tileset");
	if (props->tile_set == NULL) props->tile_set = g_strdup("classic.png");

	props->animate=props_get_bool("animate", TRUE);
	props->show_grid=props_get_bool("showgrid", TRUE);
	props->quick_moves=props_get_bool("quickmoves", FALSE);
	props->flip_final=props_get_bool ("flipfinal", TRUE);

}

static void
save_properties (void)
{
	gconf_client_set_int (get_gconf_client(), get_gconf_uri("blacklevel"),
	                      props->black_level, NULL);
	gconf_client_set_int (get_gconf_client(), get_gconf_uri("whitelevel"),
	                      props->white_level, NULL);
	gconf_client_set_bool (get_gconf_client(), get_gconf_uri("quickmoves"),
	                       props->quick_moves, NULL);
	gconf_client_set_string (get_gconf_client(), get_gconf_uri("tileset"),
	                         props->tile_set, NULL);
	gconf_client_set_bool (get_gconf_client(), get_gconf_uri("animate"),
	                      props->animate, NULL);
	gconf_client_set_bool (get_gconf_client(), get_gconf_uri("showgrid"),
	                      props->show_grid, NULL);
	gconf_client_set_bool (get_gconf_client(), get_gconf_uri("flipfinal"),
	                       props->flip_final, NULL);
}

/* callbacks for the various check and radiobuttons */
static void
black_level_select (GtkWidget *widget, gpointer data) {
	if (GTK_TOGGLE_BUTTON (widget)->active) {
		props->black_level = GPOINTER_TO_INT(data);
	}
	save_properties ();
	apply_changes ();
}

static void
white_level_select (GtkWidget *widget, gpointer data) {
	if (GTK_TOGGLE_BUTTON (widget)->active) {
		props->white_level = GPOINTER_TO_INT(data);
	}
	save_properties ();
	apply_changes ();
}

static void
set_variable_cb(GtkWidget * widget, gpointer data) {
	*((gint *)data)=GTK_TOGGLE_BUTTON(widget)->active;
	apply_changes();
}

/* callback for the tileset menu */ 
static void
set_selection (GtkWidget *widget, gpointer data)
{
        gchar *filename;

	if (props->tile_set)
		g_free (props->tile_set);

	filename = games_file_list_get_nth (theme_file_list, 
					    gtk_combo_box_get_active (GTK_COMBO_BOX (widget)));
	props->tile_set = g_strdup (filename);

	save_properties ();
	apply_changes();
}

/* fills the pixmap options menu */
static GtkWidget *
fill_tileset_menu (void)
{
	/* Note that we are sharing pixmaps with iagno here. */
	gchar *dname = gnome_program_locate_file (NULL,
						  GNOME_FILE_DOMAIN_APP_PIXMAP,
						  ("iagno"), FALSE, NULL);


        if (theme_file_list)
                g_object_unref (theme_file_list);
 
        theme_file_list = games_file_list_new_images (dname, NULL);
        g_free (dname);
        games_file_list_transform_basename (theme_file_list);
 
        return games_file_list_create_widget (theme_file_list, props->tile_set,
                                              GAMES_FILE_LIST_REMOVE_EXTENSION
|
                                              GAMES_FILE_LIST_REPLACE_UNDERSCORES);
}

/* makes a perty radio button for a particular computer level */
static GtkWidget * add_level(GtkWidget * container, 
		GtkWidget * prev,
		gchar * caption,
		gint level,
		gint piece) {
	GtkWidget * button;
	int computer_level;
	GCallback callback;

	if (piece==BLACK) {
		computer_level=props->black_level;
		callback=G_CALLBACK(black_level_select);
	} else {
		computer_level=props->white_level;
		callback=G_CALLBACK(white_level_select);
	}
	
	button=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(prev), caption);
	if (computer_level==level) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
	}
	g_signal_connect(G_OBJECT(button), "toggled", callback,
                         GINT_TO_POINTER(level));
	gtk_box_pack_start(GTK_BOX(container), button, FALSE, FALSE, 0);
	return button;
}

static gboolean quit_properties_dialog(GtkWidget * widget, gpointer data) {
	save_properties();
	apply_changes();
	g_object_unref (tooltips);
	tooltips = NULL;
	g_object_unref (theme_file_list);
	theme_file_list = NULL;
	gtk_widget_destroy(propbox);
	propbox = NULL;
	return FALSE;
}

void props_init(GtkWindow * window, char * title) {
	mainwindow=window;
	load_properties();
}

void show_properties_dialog (void) {
	GtkWidget *notebook;
	GtkWidget *frame;
	GtkWidget *label;
	GtkWidget *table;
	GtkWidget *button;
	GtkWidget *vbox, *vbox2;
	GtkWidget *hbox;
	GtkWidget *menu;
	
	if (propbox != NULL) {
		gtk_window_present (GTK_WINDOW(propbox));
		return;
	}

	/* create window */
        propbox = gtk_dialog_new_with_buttons (_("Ataxx Preferences"),
					       GTK_WINDOW (mainwindow),
					       GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_NO_SEPARATOR,
					       GTK_STOCK_CLOSE,
					       GTK_RESPONSE_CLOSE,
					       NULL);
	
	g_signal_connect (G_OBJECT (propbox), "response",
			  G_CALLBACK (quit_properties_dialog), &propbox);

	g_signal_connect (G_OBJECT (propbox), "delete_event",
			  G_CALLBACK (quit_properties_dialog), &propbox);

	/* create tooltips */	
	tooltips = gtk_tooltips_new();
	g_object_ref (tooltips);
	gtk_object_sink (GTK_OBJECT (tooltips));

	/* create notebook (tabs) */
	notebook = gtk_notebook_new ();
	gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_TOP);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (propbox)->vbox), notebook, TRUE, TRUE, 0);
	
	/* Players tab */
	
	/* FIXME mnemonic doesn't seem to work */
	label = gtk_label_new_with_mnemonic (_("_Players"));

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), vbox, label);

	table = gtk_table_new (1, 2, FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (table), 6);
        gtk_box_pack_start (GTK_BOX (vbox), table, FALSE, FALSE, 0);

	vbox2 = gtk_vbox_new (FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (vbox2), 12);
        gtk_box_pack_start (GTK_BOX (vbox), vbox2, FALSE, FALSE, 0);
	button = gtk_check_button_new_with_label (_("Quick Moves"));
	gtk_tooltips_set_tip(tooltips, button, _("Shortens the time a computer waits before doing a move"), NULL);	
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button),
				     props->quick_moves);
	g_signal_connect (G_OBJECT (button), "toggled",
			  G_CALLBACK (set_variable_cb), &(props->quick_moves));
	gtk_box_pack_start (GTK_BOX (vbox2), button, FALSE, FALSE, 0);
	
	
	/* Dark level select */
	frame = games_frame_new (_("Dark"));
	
	vbox2 = gtk_vbox_new (FALSE, 6);
	gtk_container_set_border_width (GTK_CONTAINER (vbox2), 6);
	
	button=add_level(vbox2, NULL, _("Human"), 	0, BLACK);
	button=add_level(vbox2, button, _("Very Easy"),	1, BLACK);
	button=add_level(vbox2, button, _("Easy"), 	2, BLACK);
	button=add_level(vbox2, button, _("Medium"), 	3, BLACK);
	button=add_level(vbox2, button, _("Hard"), 	4, BLACK);
	button=add_level(vbox2, button, _("Very Hard"), 5, BLACK);

	gtk_container_add (GTK_CONTAINER (frame), vbox2);
	
	gtk_table_attach (GTK_TABLE (table), frame, 1, 2, 0, 1,
			  GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL,
			  6, 6);
	
	/* Light level select */
	frame = games_frame_new (_("Light"));

	vbox2 = gtk_vbox_new (FALSE, 6);
	gtk_container_set_border_width (GTK_CONTAINER (vbox2), 6);
	
	button=add_level(vbox2, NULL, _("Human"), 0, WHITE);
	button=add_level(vbox2, button, _("Very Easy"), 1, WHITE);
	button=add_level(vbox2, button, _("Easy"), 	2, WHITE);
	button=add_level(vbox2, button, _("Medium"), 	3, WHITE);
	button=add_level(vbox2, button, _("Hard"), 	4, WHITE);
	button=add_level(vbox2, button, _("Very Hard"), 5, WHITE);
	
	gtk_container_add (GTK_CONTAINER (frame), (vbox2));

	gtk_table_attach (GTK_TABLE (table), frame, 0, 1, 0, 1,
			  GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL,
			  6, 6);

	/* Appearance tab */
	label = gtk_label_new_with_mnemonic (_("_Appearance"));
	vbox = gtk_vbox_new (FALSE, 6);

        frame = games_frame_new (_("Appearance"));
	gtk_container_add (GTK_CONTAINER (frame), vbox);
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), frame, label);

	/* animation button */
	button=gtk_check_button_new_with_label(_("Animation"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), props->animate);
	g_signal_connect(G_OBJECT(button), "toggled",
			G_CALLBACK(set_variable_cb), &(props->animate));
	gtk_tooltips_set_tip(tooltips, button, _("Flip the pieces with some visual effects"), NULL);	
	gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);

	/* flip tiles button */
	button=gtk_check_button_new_with_label(_("Flip final results"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), props->flip_final);
	g_signal_connect(G_OBJECT(button), "toggled",
			G_CALLBACK(set_variable_cb), &(props->flip_final));
	gtk_tooltips_set_tip(tooltips, button, _("Put all the white pieces at the bottom and all the black pieces on the top of the board when a game is over"), NULL);	
	gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);

	/* show grid button */
	button=gtk_check_button_new_with_label(_("Show grid"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), props->show_grid);
	g_signal_connect(G_OBJECT(button), "toggled",
			G_CALLBACK(set_variable_cb), &(props->show_grid));
	gtk_tooltips_set_tip(tooltips, button, _("Whether the grid should be shown"), NULL);	
	gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);

	/* FIXME stagger flips button? */

	/*  tileset select */
	hbox=gtk_hbox_new(FALSE, GNOME_PAD);
	label=gtk_label_new(_("Tile set:"));
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	menu = fill_tileset_menu ();
	g_signal_connect (G_OBJECT (menu), "changed",
			  G_CALLBACK (set_selection), NULL);
	gtk_tooltips_set_tip(tooltips, menu, _("The appearance of the pieces"), NULL);	
	gtk_box_pack_start(GTK_BOX(hbox), menu, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

	/* end of appearance tab */
	gtk_widget_show_all (propbox);

}	

/* FIXME rewrite this to clear the whole struct */
void
reload_properties (void)
{
	g_free (props->tile_set);
	props->tile_set = NULL;
	load_properties ();
	apply_changes ();
}

