/*
 * Same-Gnome: the game.
 * (C) 1997 the Free Software Foundation
 *
 * Author: Miguel de Icaza.
 *         Federico Mena.
 *         Horacio Peña.
 *
 * The idea is originally from KDE's same game program.
 *
 */

#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <dirent.h>

#include <config.h>
#include <gnome.h>
#include <libgnomeui/gnome-window-icon.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gconf/gconf-client.h>
#include <games-gconf.h>


#define STONE_SIZE 40
#define STONE_COLS  15
#define STONE_LINES 10
#define GAME_EVENTS (GDK_EXPOSURE_MASK              |\
		     GDK_BUTTON_PRESS_MASK          |\
		     GDK_ENTER_NOTIFY_MASK          |\
		     GDK_LEAVE_NOTIFY_MASK          |\
		     GDK_POINTER_MOTION_MASK)

static GtkWidget *pref_dialog, *scorew;
static GtkWidget *app, *draw_area, *vb, *appbar;
static GdkPixbuf *image;
static GdkPixmap *stones, *mask;
static int tagged_count = 0;
static int ball_timeout_id = -1;
static int old_x = -1, old_y = -1;
static int score;
static gchar *scenario;
static gint restarted;
static gboolean game_over = FALSE;

void update_score_state ();

/* Prefs */
GConfClient *conf_client = NULL;

static struct ball {
	int color;
	int tag;
	int frame;
} field [STONE_COLS][STONE_LINES];

static int nstones;
static int ncolors;
static int sync_stones = 0;

#define mapx(x) (x)
#define mapy(y) (STONE_LINES-1-(y))

static void
draw_ball (int x, int y)
{
	int bx, by;

	if (field [x][y].color){
		by = STONE_SIZE * (field [x][y].color - 1);
		bx = STONE_SIZE * (field [x][y].frame);

		gdk_draw_drawable (draw_area->window,
				 draw_area->style->black_gc, stones,
				 bx, by, x * STONE_SIZE, y * STONE_SIZE,
				 STONE_SIZE, STONE_SIZE);
	} else {
		gdk_window_clear_area (draw_area->window, x * STONE_SIZE, y * STONE_SIZE,
				       STONE_SIZE, STONE_SIZE);
	}
}

static void
paint (GdkRectangle *area)
{
	int x1, y1, x2, y2, x, y;
	
	x1 = area->x / STONE_SIZE;
	y1 = area->y / STONE_SIZE;
	x2 = (area->x + area->width) / STONE_SIZE;
	y2 = (area->y + area->height) / STONE_SIZE;

	for (x = x1; x <= x2; x++){
		for (y = y1; y <= y2; y++){
			draw_ball (x, y);
		}
	}
}

static void
untag_all ()
{
	int x, y;

	for (x = 0; x < STONE_COLS; x++)
		for (y = 0; y < STONE_LINES; y++){
			field [x][y].tag   = 0;
			if (sync_stones){
				field [x][y].frame = 0;
				draw_ball (x, y);
			}
		}
}

static int
flood_fill (int x, int y, int color)
{
	int c = 0;
	
	if (field [x][y].color != color)
		return c;
	
	if (field [x][y].tag)
		return c;

	c = 1;
	field [x][y].tag = 1;
	
	if (x+1 < STONE_COLS)
		c += flood_fill (x+1, y, color);
	if (x)
		c += flood_fill (x-1, y, color);
	if (y+1 < STONE_LINES)
		c += flood_fill (x, y+1, color);
	if (y)
		c += flood_fill (x, y-1, color);
	return c;
}

static int
move_tagged_balls (void *data)
{
	int x, y;
	
	for (x = 0; x < STONE_COLS; x++)
		for (y = 0; y < STONE_LINES; y++){
			if (!field [x][y].tag)
				continue;
			field [x][y].frame = (field [x][y].frame + 1) % nstones;
			draw_ball (x, y);
		}
	return 1;
}

static void
disable_timeout ()
{
	if (ball_timeout_id != -1){
		g_source_remove (ball_timeout_id);
		ball_timeout_id = -1;
	}
}

static void
mark_balls (int x, int y)
{
	if (x == old_x && y == old_y)
		return;
	old_x = x;
	old_y = y;

	untag_all ();
	disable_timeout ();
	if (!field [x][y].color)
		return;
	
	tagged_count = flood_fill (x, y, field [x][y].color);
	
	if (tagged_count > 1) {
		char *b;
		ball_timeout_id = g_timeout_add (100, move_tagged_balls, 0);
		b = g_strdup_printf ("%d %s", tagged_count, _("stones selected"));
                gnome_appbar_set_status (GNOME_APPBAR(appbar), b);
		g_free (b);
	} else
                gnome_appbar_set_status (GNOME_APPBAR(appbar), _("No stones selected"));
}

static void
compress_column (int x)
{
	int y, ym;
	
	for (y = STONE_LINES - 1; y >= 0; y--){
		if (!field [mapx(x)][mapy(y)].tag)
			continue;
		for (ym = y; ym < STONE_LINES - 1; ym++)
			field [mapx(x)][mapy(ym)] = field [mapx(x)][mapy(ym+1)];
		field [mapx(x)][mapy(ym)].color = 0;
		field [mapx(x)][mapy(ym)].tag   = 0;
	}
}

static void
compress_y ()
{
	int x;

	for (x = 0; x < STONE_COLS; x++)
		compress_column (x);
}

static void
copy_col (int dest, int src)
{
	int y;
	
	for (y = 0; y < STONE_LINES; y++)
		field [mapx(dest)][mapy(y)] = field [mapx(src)][mapy(y)];
}

static void
clean_last_col ()
{
	int y;

	for (y = 0; y < STONE_LINES; y++){
		field [mapx(STONE_COLS-1)][mapy(y)].color = 0;
		field [mapx(STONE_COLS-1)][mapy(y)].tag   = 0;
	}
}

static void
compress_x ()
{
	int x, xm, l;

	for (x = 0; x < STONE_COLS; x++){
		for (l = STONE_COLS; field [mapx(x)][mapy(0)].color == 0 && l; l--){
			for (xm = x; xm < STONE_COLS-1; xm++)
				copy_col (xm, xm+1);
			clean_last_col ();
		} 
	}
}

static void
set_score (int new_score)
{
	char *b = NULL;
	
	score = new_score;
	b = g_strdup_printf ("%.5d", score);
	gtk_label_set_text (GTK_LABEL(scorew), b);
	g_free (b);
}

static void
show_scores (guint pos)
{
	GtkWidget *dialog;

	dialog = gnome_scores_display (_("The Same GNOME"), "same-gnome", NULL, pos);
	if (dialog != NULL) {
		gtk_window_set_transient_for (GTK_WINDOW(dialog), GTK_WINDOW(app));
		gtk_window_set_modal (GTK_WINDOW(dialog), TRUE);
	}
}

static void
game_top_ten_callback(GtkWidget *widget, gpointer data)
{
	show_scores(0);
}

static void
check_game_over (void)
{
	int cleared=1;
	int x,y;
	int pos;
	
	for(x = 0; x < STONE_COLS; x++)
		for(y = 0 ; y < STONE_LINES; y++) {
			if (!field [x][y].color)
				continue;
			cleared = 0;
			if(x+1 < STONE_COLS) 
				if(field[x][y].color == field[x+1][y].color)
					return;
			if(y+1 < STONE_LINES) 
				if(field[x][y].color == field[x][y+1].color)
					return;
		}

	if (cleared)
		set_score (score+1000);

	pos = gnome_score_log(score, NULL, TRUE);
	update_score_state ();
	show_scores(pos);
	game_over = TRUE;
}

static void
kill_balls (int x, int y)
{
	if (!field [x][y].color)
		return;
	
	if (tagged_count < 2)
		return;

	set_score (score + (tagged_count - 2) * (tagged_count - 2));
	compress_y ();
	compress_x ();
	gtk_widget_draw (draw_area, NULL);
	check_game_over ();
}

static gint
area_event (GtkWidget *widget, GdkEvent *event, void *d)
{
	switch (event->type){
	case GDK_EXPOSE: {
		GdkEventExpose *e = (GdkEventExpose *) event;
		paint (&e->area);
		return TRUE;
	}
	
	case GDK_BUTTON_PRESS: {
		int x, y;
		gtk_widget_get_pointer (widget, &x, &y);
		kill_balls (x / STONE_SIZE, y / STONE_SIZE);
		old_x = -1;
		old_y = -1;
	}

	case GDK_ENTER_NOTIFY:
	case GDK_MOTION_NOTIFY: {
		int x, y;
		
		gtk_widget_get_pointer (widget, &x, &y);
		mark_balls (x / STONE_SIZE, y / STONE_SIZE);
		return TRUE;
	}
	
	case GDK_LEAVE_NOTIFY:
		old_x = -1;
		old_y = -1;
		disable_timeout ();
		untag_all ();
                gnome_appbar_set_status (GNOME_APPBAR(appbar), "");
		return TRUE;

	default:
		return FALSE;
	}
}

static void
fill_board (void)
{
	int x, y;

	for (x = 0; x < STONE_COLS; x++)
		for (y = 0; y < STONE_LINES; y++){
			field [x][y].color = 1 + (rand () % ncolors);
			field [x][y].tag   = 0;
			field [x][y].frame = sync_stones ? 0 : (rand () % nstones);
		}
}

static void
new_game (void)
{
	game_over = FALSE;
	fill_board ();
	set_score (0);
	gtk_widget_draw (draw_area, NULL);
}

static void
configure_sync (char *fname)
{
	if (strstr (fname, "-sync.png"))
		sync_stones = 1;
	else
		sync_stones = 0;
}

static void
load_scenario (char *fname)
{
	char *tmp, *fn;
        GdkColor bgcolor;
        GdkImage *tmpimage;
	int i, j;
    
	g_return_if_fail (fname != NULL);

	fn = g_strconcat ( PIXMAPDIR, "/", fname, NULL);

	if (!g_file_test (fn, G_FILE_TEST_EXISTS)) {
		GtkWidget *box = gtk_message_dialog_new (GTK_WINDOW (app),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_ERROR,
			GTK_BUTTONS_OK,
			_("Could not find the theme:\n%s\n\n"
			"Please check your Same GNOME installation."), fn);

		gtk_dialog_set_default_response (GTK_DIALOG (box), GTK_RESPONSE_OK);
		gtk_dialog_run (GTK_DIALOG (box));
		gtk_widget_destroy (box);
                /* We may not have called gtk_main yet, but if we do we want
                 * to exit nicely. */
                if (gtk_main_level() > 0)
                  gtk_main_quit ();
                else
                  exit (1);
	}

	g_free (scenario);
	scenario = g_strdup(fname);

	configure_sync (fname);

	if (image)
		gdk_pixbuf_unref (image);

	image = gdk_pixbuf_new_from_file (fn, NULL);

	if (image == NULL) {
		GtkWidget *box = gtk_message_dialog_new (GTK_WINDOW (app),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_ERROR,
			GTK_BUTTONS_OK,
			_("Same GNOME can't load the image file:\n%s\n\n"
			 "Please check your Same GNOME installation"), fn);

		gtk_dialog_set_default_response (GTK_DIALOG (box), GTK_RESPONSE_OK);
		gtk_dialog_run (GTK_DIALOG (box));
		gtk_widget_destroy (box);
                /* We may not have called gtk_main yet, but if we do we want
                 * to exit nicely. */
                if (gtk_main_level() > 0)
                  gtk_main_quit ();
                else
                  exit(1);
	}

	gdk_pixbuf_render_pixmap_and_mask (image, &stones, &mask, 127);

        tmpimage = gdk_drawable_get_image (stones, 0, 0, 1, 1);
        bgcolor.pixel = gdk_image_get_pixel(tmpimage, 0, 0);
        gdk_window_set_background (draw_area->window, &bgcolor);
	gdk_image_unref (tmpimage);
  
	g_free( fn );

	nstones = gdk_pixbuf_get_width (image) / STONE_SIZE;
	for (i = 0; i < STONE_COLS; i++)
	{
		for (j = 0; j < STONE_LINES; j++)
		{
			if (sync_stones) {
				field[i][j].frame = 0;
			} else {
				field[i][j].frame %= nstones;
			}
		}
	}
/*	ncolors = image->rgb_height / STONE_SIZE; */
	ncolors = 3;


	gtk_widget_draw (draw_area, NULL);
}

static void
create_same_board (char *fname)
{
	draw_area = gtk_drawing_area_new ();

	gtk_widget_set_events (draw_area, gtk_widget_get_events (draw_area) | GAME_EVENTS);

	gtk_box_pack_start_defaults (GTK_BOX(vb), draw_area);
	gtk_widget_realize (draw_area);
  
	gtk_widget_show (draw_area);

	load_scenario (fname);
	gtk_drawing_area_size (GTK_DRAWING_AREA (draw_area),
			       STONE_COLS  * STONE_SIZE,
			       STONE_LINES * STONE_SIZE);
	g_signal_connect (G_OBJECT (draw_area), "event",
			  G_CALLBACK(area_event), 0);
}

static void
game_new_callback (GtkWidget *widget, void *data)
{
	new_game ();
}

static void
set_selection (GtkWidget *widget, gpointer data)
{
	load_scenario (data);
	gconf_client_set_string (conf_client, "/apps/same-gnome/tileset", data, NULL);
}

static void
fill_menu (GtkWidget *menu)
{
	struct dirent *e;
	DIR *dir;
        int itemno = 0;
	
	dir = opendir (PIXMAPDIR);

	if (!dir)
		return;
	
	while ((e = readdir (dir)) != NULL){
		GtkWidget *item;
		char *name, *label, *p;
		name = strdup (e->d_name);

		if (!(p = strstr (name, ".png"))) {
			free (name);
			continue;
		}
		
		item = gtk_menu_item_new_with_label (g_strndup (name, p - name ));
		gtk_widget_show (item);
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
		g_signal_connect (G_OBJECT (item), "activate",
				  G_CALLBACK (set_selection), name);
	  
	        if (!strcmp(scenario, name))
			gtk_menu_set_active(GTK_MENU(menu), itemno);
	  
	        itemno++;
	}
	closedir (dir);
}

static void
pref_dialog_response (GtkDialog *dialog, gint response, gpointer data)
{
	gtk_widget_destroy (pref_dialog);
	pref_dialog = NULL;
}

static void
game_preferences_callback (GtkWidget *widget, void *data)
{
	GtkWidget *menu, *omenu, *l, *hb;
	GtkWidget *button;

	if (pref_dialog) {
		gtk_window_present (GTK_WINDOW (pref_dialog));
		return;
	}
	
	pref_dialog = gtk_dialog_new_with_buttons (_("Preferences"),
			GTK_WINDOW (app),
			GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_NO_SEPARATOR,
			GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
			NULL);

	gtk_dialog_set_default_response (GTK_DIALOG (pref_dialog),
					 GTK_RESPONSE_OK);
	g_signal_connect (G_OBJECT (pref_dialog), "response",
			  G_CALLBACK(pref_dialog_response), NULL);

	omenu = gtk_option_menu_new ();
	menu = gtk_menu_new ();
	fill_menu (menu);
	gtk_widget_show (omenu);
	gtk_option_menu_set_menu (GTK_OPTION_MENU(omenu), menu);

	hb = gtk_hbox_new (FALSE, FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (hb), 4);
	gtk_widget_show (hb);
	
	l = gtk_label_new (_("Select scenario:"));
	gtk_widget_show (l);
	    
	gtk_box_pack_start (GTK_BOX(hb), l, TRUE, FALSE, 4);
	gtk_box_pack_start_defaults (GTK_BOX(hb), omenu);

	gtk_box_pack_start_defaults (GTK_BOX(GTK_DIALOG(pref_dialog)->vbox), hb);
	
        gtk_widget_show (pref_dialog);
}

static int
game_about_callback (GtkWidget *widget, void *data)
{
	GdkPixbuf *pixbuf = NULL;
	static GtkWidget *about = NULL;
	const gchar *authors[] = {
		"Miguel de Icaza.",
		"Federico Mena.",
		"Horacio J. PeÃ±a.",
		NULL
	};
	gchar *documenters[] = {
                NULL
        };
        /* Translator credits */
        gchar *translator_credits = _("translator_credits");

	if (about) {
		gtk_window_present (GTK_WINDOW (about));
		return;
	}

	{
		char *filename = NULL;

		filename = gnome_program_locate_file (NULL,
				GNOME_FILE_DOMAIN_APP_PIXMAP,  ("gnome-gsame.png"),
				TRUE, NULL);
		if (filename != NULL)
		{
			pixbuf = gdk_pixbuf_new_from_file(filename, NULL);
			g_free (filename);
		}
	}
	
	about = gnome_about_new (_("The Same GNOME"), VERSION,
				 "(C) 1997-1998 the Free Software Foundation",
				 _("Original idea from KDE's same game program."),
				 (const char **)authors,
				 (const char **)documenters,
				 strcmp (translator_credits, "translator_credits") != 0 ? translator_credits : NULL,
                                 pixbuf);
	if (pixbuf != NULL)
		gdk_pixbuf_unref (pixbuf);
	
	gtk_window_set_transient_for (GTK_WINDOW (about), GTK_WINDOW (app));
	gtk_widget_show (about);

	g_signal_connect (G_OBJECT (about), "destroy",
			  G_CALLBACK (gtk_widget_destroyed), &about);

	return TRUE;
}

static int
game_quit_callback (GtkWidget *widget, void *data)
{
	gtk_main_quit ();
	return FALSE;
}

GnomeUIInfo gamemenu[] = {

        GNOMEUIINFO_MENU_NEW_GAME_ITEM(game_new_callback, NULL),

	GNOMEUIINFO_SEPARATOR,

	GNOMEUIINFO_MENU_SCORES_ITEM(game_top_ten_callback, NULL),

	GNOMEUIINFO_SEPARATOR,

        GNOMEUIINFO_MENU_QUIT_ITEM(game_quit_callback, NULL),

	GNOMEUIINFO_END
};

GnomeUIInfo settingsmenu[] = {
        GNOMEUIINFO_MENU_PREFERENCES_ITEM(game_preferences_callback, NULL),

	GNOMEUIINFO_END
};

GnomeUIInfo helpmenu[] = {
        GNOMEUIINFO_HELP("same-gnome"),

	GNOMEUIINFO_MENU_ABOUT_ITEM(game_about_callback, NULL),

	GNOMEUIINFO_END
};

GnomeUIInfo mainmenu[] = {
	GNOMEUIINFO_MENU_GAME_TREE(gamemenu),
	GNOMEUIINFO_MENU_SETTINGS_TREE(settingsmenu),
	GNOMEUIINFO_MENU_HELP_TREE(helpmenu),
	GNOMEUIINFO_END
};

void update_score_state ()
{
        gchar **names = NULL;
        gfloat *scores = NULL;
        time_t *scoretimes = NULL;
	gint top;

	top = gnome_score_get_notable("same-gnome", NULL, &names, &scores, &scoretimes);
	if (top > 0) {
		gtk_widget_set_sensitive (gamemenu[2].widget, TRUE);
		g_strfreev(names);
		g_free(scores);
		g_free(scoretimes);
	} else {
		gtk_widget_set_sensitive (gamemenu[2].widget, FALSE);
	}
}

static int
save_state (GnomeClient *client,
	    gint phase,
	    GnomeSaveStyle save_style,
	    gint shutdown, 
	    GnomeInteractStyle interact_style,
	    gint fast,
	    gpointer client_data)
{
	gchar *argv []= { "rm", "-r", NULL };
	gchar *buf;
	struct ball *f = (struct ball*) field;
	int i;  
	
	gconf_client_set_int (conf_client, "/apps/same-gnome/score", score, NULL);
	gconf_client_set_int (conf_client, "/apps/same-gnome/nstones", sync_stones ? 1 : nstones, NULL);
	
	buf= g_malloc (STONE_COLS*STONE_LINES+1);

	for (i = 0 ; i < (STONE_COLS*STONE_LINES); i++){
		buf [i]= f [i].color + 'a';
	}
	buf [STONE_COLS*STONE_LINES]= '\0';
	gconf_client_set_string (conf_client, "/apps/same-gnome/field", buf, NULL);
	g_free(buf);

	/* FIXME if anybody knows what this does...
	argv[2]= gnome_config_get_real_path (prefix);
	gnome_client_set_discard_command (client, 3, argv);
	gnome_client_set_discard_command (client, 2, argv);
	*/
	
	return TRUE;
}


static void
restart (void)
{
	gchar *buf;
	struct ball *f = (struct ball*) field;
	int i;
	
	score = gconf_client_get_int (conf_client, "/apps/same-gnome/score", NULL);
	nstones = gconf_client_get_int (conf_client, "/apps/same-gnome/nstones", NULL);
	
	buf = gconf_client_get_string (conf_client, "/apps/same-gnome/field", NULL);

	if (buf) {
		for (i= 0; i < (STONE_COLS*STONE_LINES); i++) 
		{
			f[i].color= buf[i] - 'a';
			f[i].tag  = 0;
			f[i].frame= nstones ? (rand () % nstones) : 0;
		}
		g_free (buf);
	}
}

static gint
client_die (GnomeClient *client, gpointer client_data)
{
        gtk_main_quit ();

	return FALSE;
}

#ifndef GNOME_CLIENT_RESTARTED
#define GNOME_CLIENT_RESTARTED(client) \
(GNOME_CLIENT_CONNECTED (client) && \
 (gnome_client_get_previous_id (client) != NULL) && \
 (strcmp (gnome_client_get_id (client), \
  gnome_client_get_previous_id (client)) == 0))
#endif /* GNOME_CLIENT_RESTARTED */

int
main (int argc, char *argv [])
{
	static char *fname;
	static const struct poptOption options[] = {
		{ "scenario", 's', POPT_ARG_STRING, &fname, 0, N_("Set game scenario"), N_("NAME") },
		{ NULL, '\0', 0, NULL, 0 }
	};
        GtkWidget * label;
	GnomeClient *client;

	gnome_score_init("same-gnome");

	bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

        gnome_program_init ("same-gnome", VERSION,
                             LIBGNOMEUI_MODULE,
                             argc, argv,
                             GNOME_PARAM_POPT_TABLE, options,
                             GNOME_PARAM_APP_DATADIR, DATADIR, NULL);

	gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gnome-gsame.png");
	client= gnome_master_client ();

	g_signal_connect (G_OBJECT (client), "save_yourself",
			  G_CALLBACK (save_state), argv[0]);
	g_signal_connect (G_OBJECT (client), "die",
			  G_CALLBACK (client_die), NULL);

	if (GNOME_CLIENT_RESTARTED (client)){
		restart ();
		restarted = 1;
	}

	/* Get the default GConfClient */
	conf_client = gconf_client_get_default ();
	if (!games_gconf_sanity_check_string (conf_client, "/apps/same-gnome/tileset")) {
		return 1;
	}

	srand (time (NULL));

	app = gnome_app_new("same-gnome", _("Same GNOME"));

        gtk_window_set_policy(GTK_WINDOW(app), FALSE, FALSE, TRUE);
	g_signal_connect (G_OBJECT (app), "delete_event",
			  G_CALLBACK(game_quit_callback), NULL);

	appbar = gnome_appbar_new(FALSE, TRUE, GNOME_PREFERENCES_USER);
	gnome_app_set_statusbar(GNOME_APP (app), GTK_WIDGET(appbar));

	gnome_appbar_set_status(GNOME_APPBAR (appbar),
				_("Welcome to Same GNOME!"));

	gnome_app_create_menus(GNOME_APP(app), mainmenu);

	gnome_app_install_menu_hints(GNOME_APP (app), mainmenu);
  
        vb = gtk_vbox_new (FALSE, 0);
	gnome_app_set_contents (GNOME_APP (app), vb);

	if (!fname) {
		fname = gconf_client_get_string
			(conf_client, "/apps/same-gnome/tileset", NULL);
	}

	create_same_board (fname);

	update_score_state ();
	
	label = gtk_label_new (_("Score: "));
	scorew = gtk_label_new ("");
	set_score (score);

	gtk_box_pack_start(GTK_BOX(appbar), label, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(appbar), scorew, FALSE, TRUE, 0);
	
	if (!restarted)
		new_game ();
	
	g_free (fname);

        gtk_widget_show_all (app);

	gtk_main ();
	return 0;
}
