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
#include <glib/gi18n.h>
#include <string.h>
#include <dirent.h>
#include <games-clock.h>
#include <games-frame.h>
#include <gconf/gconf-client.h>
#include "gtkgridboard.h"

#include "games-files.h"

#include "properties.h"
#include "gataxx.h"

#define GCONF_PREFIX "/apps/gataxx/"

#define GCONF_BLACKLEVEL GCONF_PREFIX "blacklevel"
#define GCONF_WHITELEVEL GCONF_PREFIX "whitelevel"
#define GCONF_QUICKMOVES GCONF_PREFIX "quickmoves"
#define GCONF_THEME GCONF_PREFIX "theme"
#define GCONF_ANIMATE GCONF_PREFIX "animate"

typedef struct {
	gint black_level;
	gint white_level;
	gboolean animate;
	gboolean quick_moves;
        gchar * theme;
} PropertiesData;

PropertiesData _props;
PropertiesData * props=&_props;

static GtkWidget *propbox = NULL;
static GtkWindow * mainwindow = NULL;
static GtkTooltips *tooltips = NULL;

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

gboolean props_get_animate() {
	return props->animate;
}

gboolean props_get_quick_moves() {
	return props->quick_moves;
}

gchar * props_get_theme() {
	return props->theme;
}

/* functions to get things from gconf */
static int props_get_int(const gchar * key, int def, int low, int high) {
	int result;
	GError * error=NULL;

	result=gconf_client_get_int(get_gconf_client(), key, &error);
	if (error!=NULL) {
		g_error_free(error);
		return def;
	}
	
	return CLAMP (result, low, high);
}

static gboolean props_get_bool(const gchar * key, gboolean def) {
	GError * error=NULL;
	gboolean result;

	result=gconf_client_get_bool(get_gconf_client(), key, &error);
	if (error!=NULL) {
		g_error_free(error);
		return def;
	}

	return result;
}

/* FIXME: watch this: */
/* Returns gchar* you should free with g_free(). Returns NULL on not-found
 * or other error. */
static gchar * props_get_string(const gchar *key)
{
	GError *error = NULL;
	gchar *retval;

	retval = gconf_client_get_string (get_gconf_client(), key, &error);
	if (error != NULL) {
		g_error_free (error);
		return NULL;
	}

	return retval;
}

void load_properties (void)
{
	props->black_level=props_get_int(GCONF_BLACKLEVEL, 0, 0, 3);
	props->white_level=props_get_int(GCONF_WHITELEVEL, 0, 0, 3);

	props->theme=props_get_string(GCONF_THEME);
	if (props->theme == NULL) props->theme = g_strdup("Plain");

	props->animate=props_get_bool(GCONF_ANIMATE, TRUE);
	props->quick_moves=props_get_bool(GCONF_QUICKMOVES, FALSE);
}

static void
save_properties (void)
{
	gconf_client_set_int (get_gconf_client(), GCONF_BLACKLEVEL,
	                      props->black_level, NULL);
	gconf_client_set_int (get_gconf_client(), GCONF_WHITELEVEL,
	                      props->white_level, NULL);
	gconf_client_set_bool (get_gconf_client(), GCONF_QUICKMOVES,
	                       props->quick_moves, NULL);
	gconf_client_set_string (get_gconf_client(), GCONF_THEME,
	                         props->theme, NULL);
	gconf_client_set_bool (get_gconf_client(), GCONF_ANIMATE,
	                      props->animate, NULL);
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

static void
set_selection (GtkWidget *widget, gpointer data)
{
	int n;

	if (props->theme)
		g_free (props->theme);

	/* We can't use _get_active_text since the text may be
	 * translated. */
	n = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));
	props->theme = g_strdup (gtk_gridboard_themes[n].name);

	save_properties ();
	apply_changes();
}

/* fills the pixmap options menu */
static GtkWidget *
fill_theme_menu (void)
{
        GtkWidget *combo_box;
	GtkGridBoardTheme *t;
	int n;

	combo_box = gtk_combo_box_new_text ();

	t = gtk_gridboard_themes;
	n = 0;
	while (t->name != NULL) {
	  gtk_combo_box_append_text (GTK_COMBO_BOX (combo_box), _(t->name));
	  if (g_utf8_collate (t->name, props->theme) == 0)
	    gtk_combo_box_set_active (GTK_COMBO_BOX (combo_box), n);
	  n++;
	  t++;
	}

	return combo_box;
}

/* makes a perty radio button for a particular computer level */
static GtkWidget * add_level(GtkWidget * container, 
		GtkWidget * prev,
		const gchar * caption,
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
	
	gtk_window_set_resizable (GTK_WINDOW (propbox), FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (propbox), 5);
	gtk_box_set_spacing (GTK_BOX (GTK_DIALOG (propbox)->vbox), 2);
	
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
	gtk_container_set_border_width (GTK_CONTAINER (notebook), 5);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (propbox)->vbox), notebook, TRUE, TRUE, 0);
	
	/* Players tab */
	
	label = gtk_label_new (_("Players"));

	vbox = gtk_vbox_new (FALSE, 18);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 12);
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), vbox, label);

	table = gtk_table_new (1, 2, FALSE);
	gtk_table_set_col_spacings (GTK_TABLE (table), 18);
        gtk_box_pack_start (GTK_BOX (vbox), table, FALSE, FALSE, 0);

	vbox2 = gtk_vbox_new (FALSE, 0);
        gtk_box_pack_start (GTK_BOX (vbox), vbox2, FALSE, FALSE, 0);
	button = gtk_check_button_new_with_mnemonic (_("_Quick moves"));
	gtk_tooltips_set_tip(tooltips, button, _("Shortens the time a computer waits before doing a move"), NULL);	
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button),
				     props->quick_moves);
	g_signal_connect (G_OBJECT (button), "toggled",
			  G_CALLBACK (set_variable_cb), &(props->quick_moves));
	gtk_box_pack_start (GTK_BOX (vbox2), button, FALSE, FALSE, 0);
	
	
	/* Dark level select */
	frame = games_frame_new (_("Dark"));
	
	vbox2 = gtk_vbox_new (FALSE, 6);
	
	button=add_level(vbox2, NULL,    _("Human"), 	        0, BLACK);
	button=add_level(vbox2, button,  _("Very easy"),	1, BLACK);
	button=add_level(vbox2, button, Q_("gataxx|Easy"), 	2, BLACK);
	button=add_level(vbox2, button, Q_("gataxx|Medium"), 	3, BLACK);
	button=add_level(vbox2, button,  _("Hard"), 	        4, BLACK);
	button=add_level(vbox2, button,  _("Very hard"),        5, BLACK);

	gtk_container_add (GTK_CONTAINER (frame), vbox2);
	
	gtk_table_attach (GTK_TABLE (table), frame, 1, 2, 0, 1,
			  GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL,
			  0, 0);
	
	/* Light level select */
	frame = games_frame_new (_("Light"));

	vbox2 = gtk_vbox_new (FALSE, 6);
	
	button=add_level(vbox2, NULL, _("Human"), 0, WHITE);
	button=add_level(vbox2, button, _("Very easy"), 1, WHITE);
	button=add_level(vbox2, button, _("Easy"), 	2, WHITE);
	button=add_level(vbox2, button, _("Medium"), 	3, WHITE);
	button=add_level(vbox2, button, _("Hard"), 	4, WHITE);
	button=add_level(vbox2, button, _("Very hard"), 5, WHITE);
	
	gtk_container_add (GTK_CONTAINER (frame), (vbox2));

	gtk_table_attach (GTK_TABLE (table), frame, 0, 1, 0, 1,
			  GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL,
			  0, 0);

	/* Appearance tab */
	label = gtk_label_new (_("Appearance"));
	vbox = gtk_vbox_new (FALSE, 6);

        frame = games_frame_new (_("Appearance"));
	gtk_container_add (GTK_CONTAINER (frame), vbox);
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), frame, label);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 12);

	/* animation button */
	button=gtk_check_button_new_with_mnemonic(_("_Animation"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), props->animate);
	g_signal_connect(G_OBJECT(button), "toggled",
			G_CALLBACK(set_variable_cb), &(props->animate));
	gtk_tooltips_set_tip(tooltips, button, _("Flip the pieces with some visual effects"), NULL);	
	gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);

	/*  theme select */
	hbox=gtk_hbox_new(FALSE, 12);
	label=gtk_label_new_with_mnemonic(_("_Tile set:"));
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	menu = fill_theme_menu ();
	g_signal_connect (G_OBJECT (menu), "changed",
			  G_CALLBACK (set_selection), NULL);
	gtk_tooltips_set_tip(tooltips, menu, _("The appearance of the pieces"), NULL);	
	gtk_box_pack_start(GTK_BOX(hbox), menu, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	gtk_label_set_mnemonic_widget(GTK_LABEL (label), menu);

	/* end of appearance tab */
	gtk_widget_show_all (propbox);

}	

/* FIXME rewrite this to clear the whole struct */
void
reload_properties (void)
{
	g_free (props->theme);
	props->theme = NULL;
	load_properties ();
	apply_changes ();
}

