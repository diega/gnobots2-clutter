/* -*- mode:C++; tab-width:8; c-basic-offset:8; indent-tabs-mode:true -*- */

/*
 * written by J. Marcin Gorycki <marcin.gorycki@intel.com>
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

#include "tetris.h"
#include "field.h"
#include "blockops.h"
#include "blocks.h"
#include "preview.h"
#include "scoreframe.h"
#include "sound.h"

#include <games-gconf.h>
#include <games-frame.h>
#include <games-controls.h>

#include <libgnomevfs/gnome-vfs.h>

#include <gdk/gdkkeysyms.h>
#include <config.h>
#include <dirent.h>
#include <string.h>

GdkPixbuf **pic;

int LINES = 20;
int COLUMNS = 11;

int BLOCK_SIZE = 40;

int posx = COLUMNS / 2;
int posy = 0;

int blocknr = 0;
int rot = 0;
int color = 0;

int blocknr_next = -1;
int rot_next = -1;
int color_next = -1;

bool random_block_colors = false;
bool do_preview = true;
bool rotateCounterClockWise = true;

#define TETRIS_OBJECT "gnometris-tetris-object"

#define KEY_OPTIONS_DIR "/apps/gnometris/options"
#define KEY_SOUND "/apps/gnometris/options/sound"
#define KEY_BLOCK_PIXMAP "/apps/gnometris/options/block_pixmap"
#define KEY_STARTING_LEVEL "/apps/gnometris/options/starting_level"
#define KEY_DO_PREVIEW "/apps/gnometris/options/do_preview"
#define KEY_RANDOM_BLOCK_COLORS "/apps/gnometris/options/random_block_colors"
#define KEY_ROTATE_COUNTER_CLOCKWISE "/apps/gnometris/options/rotate_counter_clock_wise"
#define KEY_LINE_FILL_HEIGHT "/apps/gnometris/options/line_fill_height"
#define KEY_LINE_FILL_PROBABILITY "/apps/gnometris/options/line_fill_probability"

#define KEY_CONTROLS_DIR "/apps/gnometris/controls"
#define KEY_MOVE_LEFT "/apps/gnometris/controls/key_left"
#define KEY_MOVE_RIGHT "/apps/gnometris/controls/key_right"
#define KEY_MOVE_DOWN "/apps/gnometris/controls/key_down"
#define KEY_MOVE_DROP "/apps/gnometris/controls/key_drop"
#define KEY_MOVE_ROTATE "/apps/gnometris/controls/key_rotate"
#define KEY_MOVE_PAUSE "/apps/gnometris/controls/key_pause"

#define KEY_BG_COLOUR "/apps/gnometris/options/bgcolor"
#define KEY_USE_BG_IMAGE "/apps/gnometris/options/usebgimage"

#define TILE_THRESHOLD 65

enum {
	URI_LIST,
	TEXT_PLAIN,
	COLOUR,
	RESET
};


Tetris::Tetris(int cmdlLevel): 
	blockPixmap(0),
	field(0),
	preview(0),
	paused(false), 
	timeoutId(0), 
	onePause(false), 
	inPlay(false),
	image(0),
	bgimage(0),
	setupdialog(0), 
	cmdlineLevel(cmdlLevel), 
	fastFall(false),
        dropBlock(false)
{
	double x1, y1, x2, y2;
	double width;
	double pts;
	gchar * outdir;
	GtkTargetEntry targets[] = {{"text/uri-list", 0, URI_LIST}, 
				    {"property/bgimage", 0, URI_LIST},
				    {"text/plain", 0, TEXT_PLAIN},
				    {"STRING", 0, TEXT_PLAIN},
				    {"application/x-color", 0, COLOUR},
				    {"x-special/gnome-reset-background", 0, RESET}};

	if (!sound)
		sound = new Sound();

	pic = new GdkPixbuf*[tableSize];
	for (int i = 0; i < tableSize; ++i)
		pic[i] = 0;

	/* Locate our background image. */
	outdir = g_build_filename (gnome_user_dir_get (), "gnometris.d", 
				   NULL);
	if (!g_file_test (outdir, G_FILE_TEST_EXISTS))
	    mkdir (outdir, 0700);
	bgPixmap = g_build_filename (outdir, "background.bin", NULL);
	g_free (outdir);
	
	w = gnome_app_new("gnometris", _("Gnometris"));
	g_signal_connect (w, "delete_event", G_CALLBACK (gameQuit), this);
	gtk_window_set_resizable (GTK_WINDOW (w), FALSE);
	gtk_drag_dest_set (w, GTK_DEST_DEFAULT_ALL, targets, 
			   G_N_ELEMENTS(targets), GDK_ACTION_COPY);
	g_signal_connect (G_OBJECT (w), "drag_data_received", 
			  G_CALLBACK (dragDrop), this);

	static GnomeUIInfo game_menu[] = 
	{
		GNOMEUIINFO_MENU_NEW_GAME_ITEM(gameNew, this),
		GNOMEUIINFO_MENU_PAUSE_GAME_ITEM(gamePause, this),
		GNOMEUIINFO_SEPARATOR,
		GNOMEUIINFO_MENU_SCORES_ITEM(gameTopTen, this),
		GNOMEUIINFO_MENU_END_GAME_ITEM(gameEnd, this),
		GNOMEUIINFO_SEPARATOR,
		GNOMEUIINFO_MENU_QUIT_ITEM(gameQuit, this),
		GNOMEUIINFO_END
	};

	gameMenuPtr = game_menu;
	
	static GnomeUIInfo settings_menu[] = 
	{
		GNOMEUIINFO_MENU_PREFERENCES_ITEM(gameProperties, this),
		GNOMEUIINFO_END
	};

	gameSettingsPtr = settings_menu;
	
	GnomeUIInfo help_menu[] = 
	{
		GNOMEUIINFO_HELP((gpointer)"gnometris"),
		GNOMEUIINFO_MENU_ABOUT_ITEM(gameAbout, this),
		GNOMEUIINFO_END
	};

	GnomeUIInfo mainmenu[] = 
	{
		GNOMEUIINFO_MENU_GAME_TREE(game_menu),
		GNOMEUIINFO_MENU_SETTINGS_TREE(settings_menu),
		GNOMEUIINFO_MENU_HELP_TREE(help_menu),
		GNOMEUIINFO_END
	};

	line_fill_height = 0;
	line_fill_prob = 5;

	/* init gconf */
	gconf_client = gconf_client_get_default ();
	games_gconf_sanity_check_string (gconf_client, KEY_BLOCK_PIXMAP);

	gconf_client_add_dir (gconf_client, KEY_OPTIONS_DIR, GCONF_CLIENT_PRELOAD_NONE, NULL);
	gconf_client_notify_add (gconf_client, KEY_OPTIONS_DIR, gconfNotify, this, NULL, NULL);

	gconf_client_add_dir (gconf_client, KEY_CONTROLS_DIR, GCONF_CLIENT_PRELOAD_NONE, NULL);
	gconf_client_notify_add (gconf_client, KEY_CONTROLS_DIR, gconfNotify, this, NULL, NULL);

	initOptions ();

	gnome_app_create_menus(GNOME_APP(w), mainmenu);

	GtkWidget * hb = gtk_hbox_new(FALSE, 0);
	gnome_app_set_contents(GNOME_APP(w), hb);

	field = new Field(/*ops*/);
	ops = new BlockOps(field);

	gtk_widget_set_events(w, gtk_widget_get_events(w) | 
						  GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK);
	
	GtkWidget *vb1 = gtk_vbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vb1), 10);
	gtk_box_pack_start_defaults(GTK_BOX(vb1), field->getWidget());
	gtk_box_pack_start(GTK_BOX(hb), vb1, 0, 0, 0);
	field->show();
	setupPixmap();

	g_signal_connect (w, "key_press_event", G_CALLBACK (keyPressHandler), this);
	g_signal_connect (w, "key_release_event", G_CALLBACK (keyReleaseHandler), this);
  
	GtkWidget *vb2 = gtk_vbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(vb2), 10);
	gtk_box_pack_end(GTK_BOX(hb), vb2, 0, 0, 0);
	
	preview = new Preview();
	
	gtk_box_pack_start(GTK_BOX(vb2), preview->getWidget(), FALSE, FALSE, 0);
	
	scoreFrame = new ScoreFrame(cmdlineLevel);
	
	gtk_box_pack_end(GTK_BOX(vb2), scoreFrame->getWidget(), TRUE, FALSE, 0);

	setOptions ();
        setupScoreState ();

	themeList = NULL;
	
	gtk_widget_show(hb);
	gtk_widget_show(vb1);
	gtk_widget_show(vb2);
	scoreFrame->show();
	gtk_widget_show(w);

	gtk_widget_set_sensitive(gameMenuPtr[1].widget, FALSE);
	gtk_widget_set_sensitive(gameMenuPtr[4].widget, FALSE);
	gtk_widget_set_sensitive(gameSettingsPtr[0].widget, TRUE);

        pauseMessage = gnome_canvas_item_new (gnome_canvas_root(GNOME_CANVAS(field->getWidget())),
                                              gnome_canvas_text_get_type(),
                                              "fill_color",
                                              "white",
                                              "x", COLUMNS*BLOCK_SIZE/2.0,
                                              "y", LINES*BLOCK_SIZE/2.0,
                                              "text", _("Paused"),
                                              "size_points", 36.0,
                                              0
                                              );

	/* Since gnome_canvas doesn't support setting the size of text in
	 * pixels (read the source where the "size" parameter gets set)
	 * and pango isn't forthcoming about how it scales things (see
	 * http://mail.gnome.org/archives/gtk-i18n-list/2003-August/msg00001.html
	 * and bug #119081). We guess at the size, see what size it is rendered
	 * to and then adjust the point size to fit. 36.0 points is pretty
	 * close for 96 dpi . */

	gnome_canvas_item_get_bounds (pauseMessage, &x1, &y1, &x2, &y2);
	width = x2 - x1;
	/* 0.8 is the fraction of the screen we want to use and 36.0 is
	 * the guess we use previously for the point size. */
	pts = 0.8*36.0*COLUMNS*BLOCK_SIZE/width;
	gnome_canvas_item_set (pauseMessage, "size_points", pts, 0);

        gnome_canvas_item_hide (pauseMessage);

        gameoverMessage = gnome_canvas_item_new (gnome_canvas_root(GNOME_CANVAS(field->getWidget())),
                                                 gnome_canvas_text_get_type(),
                                                 "fill_color",
                                                 "white",
                                                 "x", COLUMNS*BLOCK_SIZE/2.0,
                                                 "y", LINES*BLOCK_SIZE/2.0,
                                                 "text", _("Game Over"),
                                                 "size_points", 36.0,
                                                 0
                                                 );

	gnome_canvas_item_get_bounds (gameoverMessage, &x1, &y1, &x2, &y2);
	width = x2 - x1;
	/* 0.9 is the fraction of the screen we want to use and 36.0 is
	 * the guess we use previously for the point size. */
	pts = 0.9*36.0*COLUMNS*BLOCK_SIZE/width;
	gnome_canvas_item_set (gameoverMessage, "size_points", pts, 0);

        gnome_canvas_item_hide (gameoverMessage);
}

Tetris::~Tetris()
{
	delete ops;
	delete field;
	delete preview;
	delete scoreFrame;

	if (image)
		g_object_unref (G_OBJECT (image));
	if (bgimage)
		g_object_unref (G_OBJECT (bgimage));		

	if (blockPixmap)
		g_free(blockPixmap);
	if (bgPixmap)
		g_free(bgPixmap);

	delete[] pic;
}

void
Tetris::setupScoreState ()
{
        gchar **names = NULL;
        gfloat *scores = NULL;
        time_t *scoretimes = NULL;
	gint top = 0;

	top = gnome_score_get_notable ("gnometris", NULL, &names, &scores, &scoretimes);
	if (top > 0) {
		gtk_widget_set_sensitive (gameMenuPtr[3].widget, TRUE);
		g_strfreev (names);
		g_free (scores);
		g_free (scoretimes);
	} else {
		gtk_widget_set_sensitive (gameMenuPtr[3].widget, FALSE);
	}
}

void 
Tetris::setupdialogDestroy(GtkWidget *widget, void *d)
{
	Tetris *t = (Tetris*) d;
	if (t->setupdialog)
		gtk_widget_destroy(t->setupdialog);
	t->setupdialog = 0;
	gtk_widget_set_sensitive(t->gameMenuPtr[0].widget, TRUE);
}

void
Tetris::setupdialogResponse (GtkWidget *dialog, gint response_id, void *d)
{
	setupdialogDestroy (NULL, d);
}

void
Tetris::setupPixmap()
{
	gchar *pixname, *fullpixname;
	
	pixname = g_build_filename ("gnometris", blockPixmap, NULL);
	fullpixname = gnome_program_locate_file (NULL, GNOME_FILE_DOMAIN_APP_PIXMAP, pixname, FALSE, NULL);
	g_free (pixname);

	if (!g_file_test (fullpixname, G_FILE_TEST_EXISTS))
	{
		GtkWidget *widget = gtk_message_dialog_new (NULL,
						       GTK_DIALOG_DESTROY_WITH_PARENT,
						       GTK_MESSAGE_ERROR,
						       GTK_BUTTONS_OK,
						       _("Could not find the theme: \n%s\n\nPlease check your gnome-games installation"), fullpixname);
		gtk_dialog_run (GTK_DIALOG (widget));
		exit(1);
	}

	if(image)
		g_object_unref (G_OBJECT (image));

	image = gdk_pixbuf_new_from_file(fullpixname, NULL);

	if (image == NULL) {
		GtkWidget *widget= gtk_message_dialog_new (NULL,
						       GTK_DIALOG_DESTROY_WITH_PARENT,
						       GTK_MESSAGE_ERROR,
						       GTK_BUTTONS_OK,
						       _("Can't load the image: \n%s\n\nPlease check your gnome-games installation"), fullpixname);
		gtk_dialog_run (GTK_DIALOG (widget));
		exit (1);
	}
	g_free (fullpixname);

	BLOCK_SIZE = gdk_pixbuf_get_height(image);
	nr_of_colors = gdk_pixbuf_get_width(image) / BLOCK_SIZE;

	for (int i = 0; i < tableSize; ++i)
	{
		if (pic[i])
			g_object_unref (G_OBJECT (pic[i]));

		pic[i] = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE, 8, BLOCK_SIZE, BLOCK_SIZE);

		gdk_pixbuf_copy_area (image, (i % nr_of_colors) * BLOCK_SIZE, 0,
					BLOCK_SIZE, BLOCK_SIZE, pic[i], 0, 0);

	}

	if (bgimage)
		g_object_unref (G_OBJECT (bgimage));

	if (!usebg)
		bgimage = NULL;
	else {
		if (g_file_test (bgPixmap, G_FILE_TEST_EXISTS)) 
			bgimage = gdk_pixbuf_new_from_file (bgPixmap, NULL);
		else 
			bgimage = NULL;
	}

	/* A nasty hack to tile the image if it looks tileable (i.e. it
	 * is small enough. */
	if (bgimage) {
		int width, height;
		int bgwidth, bgheight;
			
		bgwidth = COLUMNS*BLOCK_SIZE;
		bgheight = LINES*BLOCK_SIZE;

		width = gdk_pixbuf_get_width (bgimage);
		height = gdk_pixbuf_get_height (bgimage);

		/* The heuristic is, anything less than 65 pixels on a side,
		 * or is square and smaller than the playing field is tiled. */
		/* Note that this heuristic fails for the standard nautilus
		 * background burlap.jpg because it is 97x91 */
		if ((width < TILE_THRESHOLD) || (height < TILE_THRESHOLD) ||
		    ((width == height) && (width < bgwidth))) {
			GdkPixbuf * temp;
			int i, j;
			
			temp = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE, 8, 
					       bgwidth, bgheight); 
		
			for (i=0; i<=bgwidth/width; i++) {
				for (j=0; j<=bgheight/height; j++) {
					int x, y, ww, hh;

					x = i*width;
					y = j*height;
					ww = MIN (width, bgwidth - x);
					hh = MIN (height, bgheight - y);

					gdk_pixbuf_copy_area (bgimage, 0, 0,
							      ww, hh, temp,
							      x, y);
				}
			}
			g_object_unref (bgimage);
			bgimage = temp;
		}
	}

	if (field)
	{
		field->updateSize (bgimage, &bgcolour);
		gtk_widget_queue_resize (field->getWidget());
	}
	
	if (preview)
	{
		preview->updateSize ();
		gtk_widget_queue_resize (field->getWidget());
	}
}

void
Tetris::gconfNotify (GConfClient *tmp_client, guint cnx_id, GConfEntry *tmp_entry, gpointer tmp_data)
{
	Tetris *t = (Tetris *)tmp_data;

	t->initOptions ();
	t->setOptions ();
}

char *
Tetris::gconfGetString (GConfClient *client, const char *key, const char *default_val)
{
	char *val;
	GError *error = NULL;

	val = gconf_client_get_string (client, key, &error);
	if (error) {
		g_warning ("gconf error: %s\n", error->message);
		g_error_free (error);
		error = NULL;
		val = g_strdup (default_val);
	}

	return val;
}

int
Tetris::gconfGetInt (GConfClient *client, const char *key, int default_val)
{
	int val;
	GError *error = NULL;

	val = gconf_client_get_int (client, key, &error);
	if (error) {
		g_warning ("gconf error: %s\n", error->message);
		g_error_free (error);
		error = NULL;
		val = default_val;
	}

	return val;
}

gboolean
Tetris::gconfGetBoolean (GConfClient *client, const char *key, gboolean default_val)
{
	gboolean val;
	GError *error = NULL;

	val = gconf_client_get_bool (client, key, &error);
	if (error) {
		g_warning ("gconf error: %s\n", error->message);
		g_error_free (error);
		error = NULL;
		val = default_val;
	}

	return val;
}

void
Tetris::initOptions ()
{
	gchar *bgcolourstr;

	if (blockPixmap)
		g_free (blockPixmap);

	blockPixmap = gconfGetString (gconf_client, KEY_BLOCK_PIXMAP, "7blocks-tig.png");

	startingLevel = gconfGetInt (gconf_client, KEY_STARTING_LEVEL, 1);
	if (startingLevel < 1) 
		startingLevel = 1;
	if (startingLevel > 10) 
		startingLevel = 10;

	if (gconfGetBoolean (gconf_client, KEY_SOUND, TRUE)) 
		sound->turnOn ();
	else
		sound->turnOff ();

	do_preview = gconfGetBoolean (gconf_client, KEY_DO_PREVIEW, TRUE);

	random_block_colors = gconfGetBoolean (gconf_client, KEY_RANDOM_BLOCK_COLORS, TRUE);

	rotateCounterClockWise = gconfGetBoolean (gconf_client, KEY_ROTATE_COUNTER_CLOCKWISE, TRUE);

	line_fill_height = gconfGetInt (gconf_client, KEY_LINE_FILL_HEIGHT, 0);
	if (line_fill_height < 0)
		line_fill_height = 0;
	if (line_fill_height > 19)
		line_fill_height = 19;

	line_fill_prob = gconfGetInt (gconf_client, KEY_LINE_FILL_PROBABILITY, 0);
	if (line_fill_prob < 0)
		line_fill_prob = 0;
	if (line_fill_prob > 10)
		line_fill_prob = 10;

	moveLeft = gconfGetInt (gconf_client, KEY_MOVE_LEFT, GDK_Left);
	moveRight = gconfGetInt (gconf_client, KEY_MOVE_RIGHT, GDK_Right);
	moveDown = gconfGetInt (gconf_client, KEY_MOVE_DOWN, GDK_Down);
	moveDrop = gconfGetInt (gconf_client, KEY_MOVE_DROP, GDK_Pause);
	moveRotate = gconfGetInt (gconf_client, KEY_MOVE_ROTATE, GDK_Up);
	movePause = gconfGetInt (gconf_client, KEY_MOVE_PAUSE, GDK_space);

	bgcolourstr = gconfGetString (gconf_client, KEY_BG_COLOUR, "Black");
	gdk_color_parse (bgcolourstr, &bgcolour);
	g_free (bgcolourstr);

	usebg = gconfGetBoolean (gconf_client, KEY_USE_BG_IMAGE, FALSE);
}

void
Tetris::setOptions ()
{
	if (setupdialog) {
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (sentry), startingLevel);
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (fill_prob_spinner), line_fill_prob);
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (fill_height_spinner), line_fill_height);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (sound_toggle), sound->isOn ());
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (do_preview_toggle), do_preview);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (random_block_colors_toggle), random_block_colors);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (rotate_counter_clock_wise_toggle), rotateCounterClockWise);
	}

	scoreFrame->setLevel (startingLevel);
	scoreFrame->setStartingLevel (startingLevel);
	setupPixmap ();
}

void
Tetris::setSound (GtkWidget *widget, gpointer data)
{
	Tetris *t = (Tetris*)data;
	gconf_client_set_bool (t->gconf_client, KEY_SOUND,
			       GTK_TOGGLE_BUTTON (widget)->active, NULL);
}

void 
Tetris::setSelectionPreview(GtkWidget *widget, void *d)
{
	Tetris *t = (Tetris*) d;
	gconf_client_set_bool (t->gconf_client, KEY_DO_PREVIEW,
			       GTK_TOGGLE_BUTTON (widget)->active, NULL);
}

void 
Tetris::setSelectionBlocks(GtkWidget *widget, void *d)
{
	Tetris *t = (Tetris*) d;
	gconf_client_set_bool (t->gconf_client, KEY_RANDOM_BLOCK_COLORS,
			       GTK_TOGGLE_BUTTON (widget)->active, NULL);
}

void 
Tetris::setRotateCounterClockWise(GtkWidget *widget, void *d)
{
	Tetris *t = (Tetris*) d;
	gconf_client_set_bool (t->gconf_client, KEY_ROTATE_COUNTER_CLOCKWISE,
			       GTK_TOGGLE_BUTTON (widget)->active, NULL);
}

void
Tetris::setSelection(GtkWidget *widget, void *data)
{
	Tetris *t;
	GList * item;

      	t = (Tetris *)g_object_get_data (G_OBJECT (widget), TETRIS_OBJECT);

	item = g_list_nth (t->themeList,
			   gtk_combo_box_get_active (GTK_COMBO_BOX (widget)));

	gconf_client_set_string (t->gconf_client, KEY_BLOCK_PIXMAP,
				 (char *)item->data, NULL);
}

void
Tetris::lineFillHeightChanged (GtkWidget *spin, gpointer data)
{
	Tetris *t = (Tetris *)data;
	gint value = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (spin));
	gconf_client_set_int (t->gconf_client, KEY_LINE_FILL_HEIGHT,
			      value, NULL);
}

void
Tetris::lineFillProbChanged (GtkWidget *spin, gpointer data)
{
	Tetris *t = (Tetris *)data;
	gint value = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (spin));
	gconf_client_set_int (t->gconf_client, KEY_LINE_FILL_PROBABILITY,
			      value, NULL);
}

void
Tetris::startingLevelChanged (GtkWidget *spin, gpointer data)
{
	Tetris *t = (Tetris *)data;
	gint value = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (spin));
	gconf_client_set_int (t->gconf_client, KEY_STARTING_LEVEL,
			      value, NULL);
}

void
Tetris::fillMenu(GtkWidget *menu, char *pixname, char *dirname, 
		 GList ** listp, bool addnone /*= false*/)
{
	struct dirent *e;
	char *dname = gnome_program_locate_file (NULL, GNOME_FILE_DOMAIN_APP_PIXMAP, dirname, FALSE, NULL);
	DIR *dir;
	int itemno = 0;
	GList * list;

	list = *listp;

	if (list) {
		g_list_foreach (list, (GFunc) g_free, NULL);
		g_list_free (list);
		*listp = NULL;
	}
	
	dir = opendir (dname);

	if (!dir)
		return;
	
	char *s;
	
	while ((e = readdir (dir)) != 0)
	{
		s = g_strdup(e->d_name);

		if (!(strstr (e->d_name, ".png") || strstr (e->d_name, ".jpg"))) 
		{
			free(s);
			continue;
		}

		gtk_combo_box_append_text (GTK_COMBO_BOX (menu), s);
		*listp = g_list_append (*listp, s);
			  
		if (!strcmp(pixname, s))
		{
		  gtk_combo_box_set_active(GTK_COMBO_BOX (menu), itemno);
		}
		itemno++;
	}
	
	if (addnone)
	{
		s = g_strdup(_("<none>"));
		gtk_combo_box_append_text (GTK_COMBO_BOX (menu), _("<none>"));
		*listp = g_list_append (*listp, s);
	}
	
	closedir(dir);
}

int 
Tetris::gameProperties(GtkWidget *widget, void *d)
{
	GtkWidget *notebook;
	GtkWidget *vbox;
	GtkWidget *label;
	GtkWidget *frame;
	GtkWidget *table;
	GtkWidget *fvbox;
	GtkObject *adj;
	GtkWidget *controls_list;
        
	Tetris *t = (Tetris*) d;
	
	if (t->setupdialog) {
		gtk_window_present (GTK_WINDOW(t->setupdialog));
		return FALSE;
	}

	/* create the dialog */
	t->setupdialog =
		gtk_dialog_new_with_buttons(_("Gnometris preferences"), 
					    GTK_WINDOW (t->w),
					    (GtkDialogFlags)0,
					    GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
					    NULL);
	gtk_dialog_set_has_separator (GTK_DIALOG (t->setupdialog), FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (t->setupdialog), 12);
	g_signal_connect (t->setupdialog, "close",
			  G_CALLBACK (setupdialogDestroy), d);
	g_signal_connect (t->setupdialog, "response",
			  G_CALLBACK (setupdialogResponse), d);

	notebook = gtk_notebook_new ();
	gtk_box_pack_start (GTK_BOX(GTK_DIALOG(t->setupdialog)->vbox), notebook,
			    TRUE, TRUE, 0);

	/* game page */
	vbox = gtk_vbox_new (FALSE, 0);
	label = gtk_label_new_with_mnemonic (_("_Game"));
	gtk_notebook_append_page (GTK_NOTEBOOK(notebook), vbox, label);

	frame = games_frame_new (_("Setup"));
	table = gtk_table_new (3, 2, FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (table), 0);
	gtk_table_set_row_spacings (GTK_TABLE (table), 6);
	gtk_table_set_col_spacings (GTK_TABLE (table), 6);

	/* pre-filled rows */
	label = gtk_label_new(_("Number of pre-filled rows:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 0, 1);

	adj = gtk_adjustment_new (t->line_fill_height, 0, LINES-1, 1, 5, 0);
	t->fill_height_spinner = gtk_spin_button_new (GTK_ADJUSTMENT (adj), 10, 0);
	gtk_spin_button_set_update_policy
		(GTK_SPIN_BUTTON (t->fill_height_spinner), GTK_UPDATE_ALWAYS);
	gtk_spin_button_set_snap_to_ticks 
		(GTK_SPIN_BUTTON (t->fill_height_spinner), TRUE);
	g_signal_connect (t->fill_height_spinner, "value_changed",
			  G_CALLBACK (lineFillHeightChanged), t);
	gtk_table_attach_defaults (GTK_TABLE (table), t->fill_height_spinner, 1, 2, 0, 1);

	/* pre-filled rows density */
	label = gtk_label_new (_("Density of blocks in a pre-filled row:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 1, 2);

	adj = gtk_adjustment_new (t->line_fill_prob, 0, 10, 1, 5, 0);
	t->fill_prob_spinner = gtk_spin_button_new (GTK_ADJUSTMENT (adj), 10, 0);
	gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (t->fill_prob_spinner),
					  GTK_UPDATE_ALWAYS);
	gtk_spin_button_set_snap_to_ticks
		(GTK_SPIN_BUTTON (t->fill_prob_spinner), TRUE);
	g_signal_connect (t->fill_prob_spinner, "value_changed",
		          G_CALLBACK (lineFillProbChanged), t);
	gtk_table_attach_defaults (GTK_TABLE (table), t->fill_prob_spinner, 1, 2, 1, 2);

	/* starting level */
	label = gtk_label_new (_("Starting Level:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 2, 3);

	adj = gtk_adjustment_new (t->startingLevel, 1, 10, 1, 5, 10);
	t->sentry = gtk_spin_button_new (GTK_ADJUSTMENT (adj), 10, 0);
	gtk_spin_button_set_update_policy (GTK_SPIN_BUTTON (t->sentry),
					   GTK_UPDATE_ALWAYS);
	gtk_spin_button_set_snap_to_ticks (GTK_SPIN_BUTTON (t->sentry), TRUE);
	g_signal_connect (t->sentry, "value_changed",
			  G_CALLBACK (startingLevelChanged), t);
	gtk_table_attach_defaults (GTK_TABLE (table), t->sentry, 1, 2, 2, 3);

	gtk_container_add (GTK_CONTAINER (frame), table);
	gtk_box_pack_start (GTK_BOX (vbox), frame, 
			    FALSE, FALSE, 0);

	frame = games_frame_new (_("Operation"));
	fvbox = gtk_vbox_new (FALSE, 6);

	/* sound */
	t->sound_toggle =
		gtk_check_button_new_with_label (_("Enable sounds"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (t->sound_toggle),
				     sound->isOn ());
	g_signal_connect (t->sound_toggle, "clicked",
			  G_CALLBACK (setSound), d);
	gtk_box_pack_start (GTK_BOX (fvbox), t->sound_toggle, 0, 0, 0);
	

	/* preview next block */
	t->do_preview_toggle =
		gtk_check_button_new_with_label (_("Preview next block"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (t->do_preview_toggle),
				     do_preview);
	g_signal_connect (t->do_preview_toggle, "clicked",
			  G_CALLBACK (setSelectionPreview), d);
	gtk_box_pack_start (GTK_BOX (fvbox), t->do_preview_toggle, 0, 0, 0);

	/* random blocks */
	t->random_block_colors_toggle =
		gtk_check_button_new_with_label(_("Use random block colors"));
	gtk_toggle_button_set_active
		(GTK_TOGGLE_BUTTON (t->random_block_colors_toggle),
		 random_block_colors);
	g_signal_connect (t->random_block_colors_toggle, "clicked",
			  G_CALLBACK (setSelectionBlocks), d);
	gtk_box_pack_start (GTK_BOX (fvbox), t->random_block_colors_toggle,
			    0, 0, 0);

	/* rotate counter clock wise */
 	t->rotate_counter_clock_wise_toggle =
		gtk_check_button_new_with_label (_("Rotate blocks counterclockwise"));
 	gtk_toggle_button_set_active
		(GTK_TOGGLE_BUTTON (t->rotate_counter_clock_wise_toggle),
		 rotateCounterClockWise);
	g_signal_connect (t->rotate_counter_clock_wise_toggle, "clicked",
			  G_CALLBACK (setRotateCounterClockWise), d);
 	gtk_box_pack_start (GTK_BOX (fvbox), t->rotate_counter_clock_wise_toggle,
			    0, 0, 0);

	gtk_container_add (GTK_CONTAINER (frame), fvbox);
	gtk_box_pack_start (GTK_BOX (vbox), frame, 
			    FALSE, FALSE, 0);

	frame = games_frame_new (_("Theme"));
	table = gtk_table_new (2, 2, FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (table), 0);
	gtk_table_set_row_spacings (GTK_TABLE (table), 6);
	gtk_table_set_col_spacings (GTK_TABLE (table), 6);

	/* Block pixmap */
	label = gtk_label_new(_("Block image:"));
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_table_attach_defaults (GTK_TABLE (table), label, 0, 1, 0, 1);

	GtkWidget *omenu = gtk_combo_box_new_text ();
	g_object_set_data (G_OBJECT (omenu), TETRIS_OBJECT, t);	
	t->fillMenu (omenu, t->blockPixmap, "gnometris", &(t->themeList));
	g_signal_connect (omenu, "changed", G_CALLBACK (setSelection), NULL);
	gtk_table_attach_defaults (GTK_TABLE (table), omenu, 1, 2, 0, 1);

	gtk_container_add (GTK_CONTAINER (frame), table);
	gtk_box_pack_start (GTK_BOX (vbox), frame, 
			    FALSE, FALSE, 0);

	/* controls page */
	vbox = gtk_vbox_new (FALSE, 0);
	label = gtk_label_new_with_mnemonic (_("_Controls"));
	gtk_notebook_append_page (GTK_NOTEBOOK(notebook), vbox, label);

	frame = games_frame_new (_("Keyboard controls"));
	fvbox = gtk_vbox_new (FALSE, 6);
	gtk_container_add (GTK_CONTAINER (frame), fvbox);

	controls_list = games_controls_list_new ();
	games_controls_list_add_controls (GAMES_CONTROLS_LIST (controls_list),
					  KEY_MOVE_LEFT,
					  KEY_MOVE_RIGHT,
					  KEY_MOVE_DOWN,
					  KEY_MOVE_DROP,
					  KEY_MOVE_ROTATE,
					  KEY_MOVE_PAUSE,
					  NULL);

	gtk_box_pack_start (GTK_BOX (fvbox), controls_list, TRUE, TRUE, 0);

	gtk_container_add (GTK_CONTAINER (vbox), frame);
	
	gtk_widget_show_all (t->setupdialog);
	gtk_widget_set_sensitive(t->gameMenuPtr[0].widget, FALSE);
	return TRUE;
}

int
Tetris::gamePause(GtkWidget *widget, void *d)
{
	Tetris *t = (Tetris*) d;
	t->togglePause();
	return TRUE;
}

int
Tetris::gameEnd(GtkWidget *widget, void *d)
{
	Tetris *t = (Tetris*) d;

	g_source_remove(t->timeoutId);
	t->timeoutId = 0;
	blocknr_next = -1;
	t->endOfGame();
	return TRUE;
}

int
Tetris::gameQuit(GtkWidget *widget, void *d)
{
	Tetris *t = (Tetris*) d;

	/* Record the score if the game isn't over. */
	if (t->inPlay && (t->scoreFrame->getScore() > 0))
		gnome_score_log(t->scoreFrame->getScore(), 0, TRUE);

	if (t->w)
		gtk_widget_destroy(t->w);
	gtk_main_quit();

	return TRUE;
}

void
Tetris::generateTimer(int level)
{
	g_source_remove(timeoutId);

	int intv = 1000 - 100 * (level - 1);
	if (intv <= 0)
		intv = 100;
		
	timeoutId = g_timeout_add_full(0, intv, timeoutHandler, this, NULL);
}

void
Tetris::manageFallen()
{
	ops->fallingToLaying();
	sound->playSound (SOUND_LAND);

	int levelBefore = scoreFrame->getLevel();
	ops->checkFullLines(scoreFrame);
	int levelAfter = scoreFrame->getLevel();
	if (levelAfter != levelBefore) 
		sound->playSound (SOUND_GNOMETRIS);
	if ((levelBefore != levelAfter) || fastFall)
		generateTimer(levelAfter);
	
	generate();
}

int
Tetris::timeoutHandler(void *d)
{
	Tetris *t = (Tetris*) d;
	
	if (t->paused)
		return TRUE;

 	if (t->onePause)
 	{
		t->onePause = false;
		gtk_widget_queue_draw(t->field->getWidget());
	}
 	else
	{
		bool res = t->ops->moveBlockDown();
		gtk_widget_queue_draw(t->field->getWidget());

		if (res)
		{
			t->manageFallen();
			if(t->fastFall) {
				t->scoreFrame->incScore(t->fastFallPoints);
				t->fastFall = false;
			}
		}
		else if(t->fastFall)
			++t->fastFallPoints;
	}

	return TRUE;	
}

gboolean
Tetris::keyPressHandler(GtkWidget *widget, GdkEvent *event, Tetris *t)
{
	int keyval;
	bool res = false;
	int bonus;

	if (t->timeoutId == 0)
		return FALSE;

	keyval = ((GdkEventKey*)event)->keyval;

	if (keyval == t->movePause)
	{
		t->togglePause();
		return TRUE;
	}

	if (t->paused)
		return FALSE;

	if (keyval == t->moveLeft) {
		sound->playSound (SOUND_SLIDE);
		res = t->ops->moveBlockLeft();
		t->onePause = false;
	} else if (keyval == t->moveRight) {
		sound->playSound (SOUND_SLIDE);
		res = t->ops->moveBlockRight();
		t->onePause = false;
	} else if (keyval == t->moveRotate) {
		sound->playSound (SOUND_TURN);
		res = t->ops->rotateBlock(rotateCounterClockWise);
		t->onePause = false;
	} else if (keyval == t->moveDown) {
		if (!t->fastFall && !t->onePause) {
			t->fastFall = true;
			t->fastFallPoints = 0;
			g_source_remove (t->timeoutId);
			t->timeoutId = g_timeout_add_full(0, 10, timeoutHandler, t, NULL);
			res = true;
		}
	} else if (keyval == t->moveDrop) {
		if (!t->dropBlock) {
			t->dropBlock = true;
			bonus = t->ops->dropBlock();
			t->scoreFrame->incScore(bonus);
			t->manageFallen();
			res = TRUE;
		}        
	}

	return res;
}

gint
Tetris::keyReleaseHandler(GtkWidget *widget, GdkEvent *event, Tetris *t)
{
	bool res = false;

	if (t->timeoutId == 0)
		return FALSE;
	
	if (t->paused)
		return FALSE;

	int keyval = ((GdkEventKey*)event)->keyval;

	if (keyval == t->moveDown) {
		if (t->fastFall) {
			t->fastFall = false;
 			t->generateTimer(t->scoreFrame->getLevel());
		}
		res = TRUE;
	} else if (keyval == t->moveDrop) {
		t->dropBlock = false;
		res = TRUE;
	}

	return res;
}

void Tetris::saveBgOptions ()
{
	gchar * cbuffer;

	gconf_client_set_bool (gconf_client, KEY_USE_BG_IMAGE, 
				  usebg, NULL);

	cbuffer = g_strdup_printf ("#%04x%04x%04x", bgcolour.red,
				   bgcolour.green, bgcolour.blue);
	gconf_client_set_string (gconf_client, KEY_BG_COLOUR, cbuffer,
				 NULL);
	g_free (cbuffer);
}

void
Tetris::decodeColour (guint16 *data, Tetris *t)
{
	t->bgcolour.red = data[0];
	t->bgcolour.green = data[1];
	t->bgcolour.blue = data[2];
	/* Ignore the alpha channel. */

	t->usebg = FALSE;
	t->saveBgOptions ();
}

void
Tetris::resetColour (Tetris *t)
{
	t->bgcolour.red = 0;
	t->bgcolour.green = 0;
	t->bgcolour.blue = 0;
	/* Ignore the alpha channel. */

	t->usebg = FALSE;
	t->saveBgOptions ();
}

gchar * 
Tetris::decodeDropData(gchar * data, gint type)
{
	gchar *start, *end;

	if (data == NULL)
		return NULL;

	if (type == TEXT_PLAIN)
		return g_strdup (data);

	if (type == URI_LIST) {
		start = data;
		/* Skip any comments. */
		if (*start == '#') {
			while (*start != '\n') {
				start++;
				if (*start == '\0')
					return NULL;
			}
			start++;
			if (*start == '\0')
				return NULL;
		}

		/* Now extract the first URI. */
		end = start;
		while ((*end != '\0') && (*end != '\r') && (*end != '\n'))
			end++;
		*end = '\0';

		return g_strdup (start);
	}

	return NULL;
}

void
Tetris::dragDrop(GtkWidget *widget, GdkDragContext *context,
		 gint x, gint y, GtkSelectionData *data, guint info, 
		 guint time, Tetris * t)
{
	gchar * fileuri;
	GnomeVFSHandle *inhandle;
	GnomeVFSHandle *outhandle;
	GnomeVFSResult result;
	GnomeVFSFileInfo fileinfo;
	GnomeVFSFileSize bytesread;
	GnomeVFSFileSize filesize;
	GdkPixbufLoader * loader;
	GdkPixbuf * pixbuf;
	guchar * buffer;


	/* Accept a dropped filename and try and load it as the
	   background image. In the event of any kind of failure we
	   silently ignore it. */
	
	/* FIXME: We don't handle colour gradients (e.g. from the gimp) */

	/* FIXME: Drag and drop from konqueror doesn't work, we
	 * aren't even registering that it is providing test/uri-list
	 * content. */

	/* FIXME: Dropped URLs from mozilla don't work (see below). */

	if (data->length < 0) {
		gtk_drag_finish (context, FALSE, FALSE, time);
		return;
	}

	gtk_drag_finish (context, TRUE, FALSE, time);

	if (info == COLOUR) {
		if (data->length == 8) 
			decodeColour ((guint16 *)data->data, t);
		return;
	}

	if (info == RESET) {
		resetColour (t);
		return;
	}

	fileuri = decodeDropData ((gchar *)data->data, info);
	/* Silently ignore bad data. */
	if (fileuri == NULL)
		goto error_exit;

	/* Now that we have a URI we load it and test it to see if it is 
	 * an image file. */

	/* FIXME: All this is slow and synchronous. It also
	 * doesn't give any feedback about errors. */
	
	result = gnome_vfs_open (&inhandle, fileuri, GNOME_VFS_OPEN_READ);
	if (result != GNOME_VFS_OK)
		goto error_exit;

	result = gnome_vfs_get_file_info_from_handle (inhandle, &fileinfo,
						      GNOME_VFS_FILE_INFO_FOLLOW_LINKS);
	if (result != GNOME_VFS_OK)
		goto error_exit_handle;

	/* This is where Drag and Drop of URLs from mozilla fails. */

	if (!(fileinfo.valid_fields & GNOME_VFS_FILE_INFO_FIELDS_SIZE))
		goto error_exit_handle;

	filesize = fileinfo.size;

	buffer = (guchar *)g_malloc (filesize);
	if (buffer == NULL)
		goto error_exit_handle;
	
	result = gnome_vfs_read (inhandle, buffer, filesize, &bytesread);
	/* FIXME: We should reread if not enough was read. */
	if ((result != GNOME_VFS_OK) || (bytesread != filesize))
		goto error_exit_buffer;

	loader = gdk_pixbuf_loader_new ();

	if (!gdk_pixbuf_loader_write (loader, buffer, filesize, NULL))
		goto error_exit_loader;

	gdk_pixbuf_loader_close (loader, NULL);

	pixbuf = gdk_pixbuf_loader_get_pixbuf (loader);
	if (pixbuf == NULL)
		goto error_exit_loader;

	g_object_ref (pixbuf);

	/* We now have an image file, in memory, that we know gdk-pixbuf
	 * can handle. Now we save it to disk. This is necessary so that
	 * "slow" URIs (e.g. http) behave well in the long run. */

	result = gnome_vfs_create (&outhandle, t->bgPixmap, GNOME_VFS_OPEN_WRITE,
				   FALSE, 0600);
	if (result != GNOME_VFS_OK)
		goto error_exit_loader;

	result = gnome_vfs_write (outhandle, buffer, filesize, &bytesread);
	if ((result != GNOME_VFS_OK) || (bytesread != filesize))
	    goto error_exit_saver;

	t->usebg = TRUE;
	t->saveBgOptions ();

 error_exit_saver:
	gnome_vfs_close (outhandle);
 error_exit_loader:
	g_object_unref (loader);
 error_exit_buffer:
	g_free (buffer);
 error_exit_handle:
	gnome_vfs_close (inhandle);
 error_exit:
	return;

}

void
Tetris::togglePause()
{
	paused = !paused;
        if (paused) {
                gnome_canvas_item_show (pauseMessage);
                gnome_canvas_item_raise_to_top (pauseMessage);
        } else
                gnome_canvas_item_hide (pauseMessage);
}

void
Tetris::generate()
{
	if (ops->generateFallingBlock())
	{
		ops->putBlockInField(false);
		gtk_widget_queue_draw(preview->getWidget());
		onePause = true;
	}
	else
	{
		g_source_remove(timeoutId);
		timeoutId = 0;
		blocknr_next = -1;
		
		endOfGame();
	}
}

void
Tetris::endOfGame()
{
	gtk_widget_set_sensitive(gameMenuPtr[1].widget, FALSE);
	gtk_widget_set_sensitive(gameMenuPtr[4].widget, FALSE);
	gtk_widget_set_sensitive(gameSettingsPtr[0].widget, TRUE);
	color_next = -1;
	blocknr_next = -1;
	rot_next = -1;
        gnome_canvas_item_hide (pauseMessage);
        gnome_canvas_item_show (gameoverMessage);
        gnome_canvas_item_raise_to_top (gameoverMessage);
	sound->playSound (SOUND_GAMEOVER);
	inPlay = false;

	if (scoreFrame->getScore() > 0) 
	{
		int pos = gnome_score_log(scoreFrame->getScore(), 0, TRUE);
		showScores("Gnometris", pos);
	}
}

void
Tetris::showScores(gchar *title, guint pos)
{
	GtkWidget *dialog;

	dialog = gnome_scores_display(title, "gnometris", 0, pos);
	if (dialog != NULL) {
		gtk_window_set_transient_for (GTK_WINDOW(dialog), GTK_WINDOW(this->getWidget()));
		gtk_window_set_modal (GTK_WINDOW(dialog), TRUE);
	}
}

int
Tetris::gameNew(GtkWidget *widget, void *d)
{
	Tetris *t = (Tetris*) d;

	if (t->timeoutId) 
	{
		g_source_remove(t->timeoutId);
		t->timeoutId = 0;

		/* Catch the case where we started a new game without
		 * finishing the old one. */
		if ((t->scoreFrame->getScore() > 0) && t->inPlay)
			gnome_score_log(t->scoreFrame->getScore(), 0, TRUE);
	}

	t->inPlay = true;

	int level = t->cmdlineLevel ? t->cmdlineLevel : t->startingLevel;

	t->fastFall = false;
	
	t->scoreFrame->setLevel(level);
	t->scoreFrame->setStartingLevel(level);

	t->timeoutId = g_timeout_add_full(0, 1000 - 100 * (level - 1), timeoutHandler, t, NULL);
	t->ops->emptyField(t->line_fill_height,t->line_fill_prob);

	t->scoreFrame->resetLines();
	t->paused = false;
	
	t->ops->generateFallingBlock();
	gtk_widget_queue_draw(t->field->getWidget());
	gtk_widget_queue_draw(t->preview->getWidget());

	gtk_widget_set_sensitive(t->gameMenuPtr[1].widget, TRUE);
	gtk_widget_set_sensitive(t->gameMenuPtr[4].widget, TRUE);
	gtk_widget_set_sensitive(t->gameSettingsPtr[0].widget, FALSE);

        gnome_canvas_item_hide (t->pauseMessage);
        gnome_canvas_item_hide (t->gameoverMessage);
        
	sound->playSound (SOUND_GNOMETRIS);

	return TRUE;
}

int
Tetris::gameAbout(GtkWidget *widget, void *d)
{
	Tetris *t = (Tetris*) d;
	GdkPixbuf *pixbuf = NULL;
	static GtkWidget *about = NULL;

	const gchar *authors[] = {"J. Marcin Gorycki", 0};

	const gchar *documenters[] = {
		NULL
	};

	const gchar *translator_credits = _("translator-credits");

	if (about != NULL) {
		gtk_window_present (GTK_WINDOW(about));
		return TRUE;
	}
        {
		char *filename = NULL;

		filename = gnome_program_locate_file (NULL,
				GNOME_FILE_DOMAIN_APP_PIXMAP, 
				"gnome-gtetris.png",
				TRUE, NULL);
		if (filename != NULL)
		{
			pixbuf = gdk_pixbuf_new_from_file(filename, NULL);
			g_free (filename);
		}
	}

	about = gnome_about_new("Gnometris", 
				VERSION, 
				"(C) 2000 J. Marcin Gorycki",
				_("Written for my wife, Matylda\n"
				  "Send comments and bug reports to: "
				  "janusz.gorycki@intel.com"),
				(const char **)authors,
				(const char **)documenters,
				strcmp (translator_credits, "translator-credits") != 0 ? translator_credits : NULL,
				pixbuf);
	
	if (pixbuf != NULL)
		gdk_pixbuf_unref (pixbuf);
	
	gtk_window_set_transient_for (GTK_WINDOW (about), GTK_WINDOW (t->getWidget()));
	g_signal_connect (G_OBJECT (about), "destroy", G_CALLBACK (gtk_widget_destroyed), &about);
	gtk_widget_show(about);

	return TRUE;
}

int
Tetris::gameTopTen(GtkWidget *widget, void *d)
{
	Tetris *t = (Tetris*) d;
	t->showScores("Gnometris", 0);

	return TRUE;
}
