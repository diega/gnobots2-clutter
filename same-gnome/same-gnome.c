/* -*- mode: C; indent-tabs-mode: t; tab-width: 2; c-basic-offset: 2; -*- */

/* same-gnome.c : Same Game
 *
 * Copyright (c) 2004 by Callum McKenzie
 *
 */

#include "config.h"

#include <gnome.h>

#include <games-gconf.h>
#include <games-gridframe.h>

#include "globals.h"
#include "same-gnome.h"
#include "drawing.h"
#include "game.h"
#include "input.h"

#define MINIMUM_CANVAS_WIDTH 120
#define MINIMUM_CANVAS_HEIGHT 80

#define DEFAULT_WINDOW_WIDTH 450
#define DEFAULT_WINDOW_HEIGHT 350

#define DEFAULT_CUSTOM_WIDTH 15
#define DEFAULT_CUSTOM_HEIGHT 10
#define MINIMUM_CUSTOM_WIDTH 4
#define MINIMUM_CUSTOM_HEIGHT 4
#define MAXIMUM_CUSTOM_HEIGHT 100
#define MAXIMUM_CUSTOM_WIDTH 100

/* FIXME: The UI stuff should be split out into its own file. */
GtkWidget *application;
GtkWidget *scorewidget;
GtkWidget *gridframe = NULL;

gchar *theme;
gint   game_size;

/* We start at 1 so we can distinguish the gconf "unset" from a valid
 * value. */
enum {
  CUSTOM = 1, /* FIXME: Are we going to use this. */
  SMALL,
  MEDIUM,
  LARGE,
  MAX_SIZE,
};

#define DEFAULT_GAME_SIZE SMALL

#define GCONF_THEME_KEY "/apps/same-gnome/tileset"
#define GCONF_SIZE_KEY  "/apps/same-gnome/size"
#define GCONF_CUSTOM_WIDTH_KEY "/apps/same-gnome/custom_width"
#define GCONF_CUSTOM_HEIGHT_KEY "/apps/same-gnome/custom_height"
#define GCONF_WINDOW_WIDTH_KEY "/apps/same-gnome/window_width"
#define GCONF_WINDOW_HEIGHT_KEY "/apps/same-gnome/window_height"

GConfClient *gcclient;

/* Keep this in sync with the enum above. */
gint board_sizes[MAX_SIZE][3] = {{-1, -1, -1}, /* This is a dummy entry. */
																 {-1, -1, -1}, /* Space for the custom size. */
																 {15, 10, 3},
																 {30, 20, 4},
																 {45, 30, 4}};

gint board_width;
gint board_height;
gint ncolours;

gint window_width;
gint window_height;

static void set_sizes (gint size)
{
	board_width = board_sizes[size][0];
	board_height = board_sizes[size][1];
	ncolours = board_sizes[size][2];
	
	gconf_client_set_int (gcclient, GCONF_SIZE_KEY, size, NULL);

	/* FIXME: The flow should avoid the need for this test. */
	if (gridframe)
		games_grid_frame_set (GAMES_GRID_FRAME (gridframe), board_width, 
													board_height);
}

static void initialise_options (gint requested_size, gchar *requested_theme)
{
  gint intvalue;

  game_size = SMALL;
  theme = DEFAULT_THEME;

  if (requested_size != -1)
    game_size = requested_size;
  if (requested_theme != NULL)
    theme = requested_theme;

  gcclient = gconf_client_get_default ();
  games_gconf_sanity_check_string (gcclient, GCONF_THEME_KEY);

  intvalue = gconf_client_get_int (gcclient, GCONF_CUSTOM_WIDTH_KEY, NULL);
  if (intvalue == 0)
    intvalue = DEFAULT_CUSTOM_WIDTH;
  board_sizes[CUSTOM][0] = CLAMP (intvalue, MINIMUM_CUSTOM_WIDTH, 
				  MAXIMUM_CUSTOM_WIDTH);
  intvalue = gconf_client_get_int (gcclient, GCONF_CUSTOM_HEIGHT_KEY, NULL);
  if (intvalue == 0)
    intvalue = DEFAULT_CUSTOM_HEIGHT;
  board_sizes[CUSTOM][1] = CLAMP (intvalue, MINIMUM_CUSTOM_HEIGHT, 
				  MAXIMUM_CUSTOM_HEIGHT);

  if (requested_size != -1) 
    game_size = requested_size;
  else {
    game_size = gconf_client_get_int (gcclient, GCONF_SIZE_KEY, NULL);
    if (game_size == 0)
      game_size = DEFAULT_GAME_SIZE;
  }

	/* FIXME: This doesn't work for a custom size. */
  game_size = CLAMP (game_size, SMALL, MAX_SIZE - 1);
	set_sizes (game_size);

  intvalue = gconf_client_get_int (gcclient, GCONF_WINDOW_WIDTH_KEY, NULL);
  if (intvalue == 0)
    intvalue = DEFAULT_WINDOW_WIDTH;
  window_width = intvalue;

  intvalue = gconf_client_get_int (gcclient, GCONF_WINDOW_HEIGHT_KEY, NULL);
  if (intvalue == 0)
    intvalue = DEFAULT_WINDOW_HEIGHT;
  window_height = intvalue;

  if (requested_theme != NULL)
    theme = requested_theme;
  else
    theme = games_gconf_get_string (gcclient, GCONF_THEME_KEY, DEFAULT_THEME);

  /* An invalid theme will be picked up at load time. Although we can
   * guarantee that theme != NULL. */
}

void show_score (gint score)
{
	gchar *label;

	label = g_strdup_printf (_("Score: %d"), score);
	gtk_label_set_text (GTK_LABEL (scorewidget), label);
	g_free (label);
}

static void new_game_cb (void)
{
	new_game ();
}

static void theme_cb (void)
{

}

static void fullscreen_cb (GtkWidget *widget)
{
  GtkWidget * window;

	/* FIMXE: Why doesn't this work. */
  window = gtk_widget_get_toplevel (widget);
  if (gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (widget))) {
    gtk_window_fullscreen (GTK_WINDOW (window));
  } else {
    gtk_window_unfullscreen (GTK_WINDOW (window));
  }
}

static void scores_cb (void)
{

}

static void undo_cb (void)
{

}

static void redo_cb (void)
{

}

static void quit_cb (void)
{
	gtk_main_quit ();
}

static void size_change_cb (GtkRadioAction *action, gpointer data)
{
	set_sizes (gtk_radio_action_get_current_value (action));
	resize_graphics ();
	new_game ();
}

static void help_cb (void)
{
	gnome_help_display ("same-gnome.xml", NULL, NULL);
}

static void about_cb (GtkWidget *widget)
{
	const gchar *authors[] = { "Callum McKenzie", NULL };

	gtk_show_about_dialog (GTK_WINDOW (application),
												 "authors", authors,
												 "comments", _("I want to play that game! You know, they all go whirly-round and you click on them and they vanish!"),
												 "copyright", "Copyright \xc2\xa9 2004 Callum McKenzie",
												 "name", _(APPNAME_LONG),
												 "translator_credits", _("translator-credits"),
												 "version", VERSION,
												 NULL);
}

/* FIXME: Will we ever want this ? */
#if 0
static void custom_size_cb (void)
{

}
#endif

static gint window_resize_cb (GtkWidget *window, GdkEventConfigure *event)
{
	gconf_client_set_int (gcclient, GCONF_WINDOW_WIDTH_KEY, event->width, NULL);
	gconf_client_set_int (gcclient, GCONF_WINDOW_HEIGHT_KEY, event->height, 
												NULL);

	return FALSE;
}

void game_over_dialog (void)
{
	GtkWidget *dialog;

	dialog = gtk_message_dialog_new (GTK_WINDOW (application),
																	 GTK_DIALOG_DESTROY_WITH_PARENT,
																	 GTK_MESSAGE_INFO,
																	 GTK_BUTTONS_OK,
																	 _("Game over"));

	/* FIXME: Should we do the undo/quit thing like aisleriot ? */
	/* FIXME: We should provide an indication of how good the score was. */
	/* FIXME: Formatting. */
	/* FIXME: Give feedback about the bonus. */

	gtk_dialog_run (GTK_DIALOG (dialog));

	gtk_widget_destroy (dialog);

	new_game ();
}

const GtkActionEntry actions[] = {
  { "GameMenu", NULL, N_("_Game") },
  { "ViewMenu", NULL, N_("_View") },
  { "SizeMenu", NULL, N_("_Size") },
  { "HelpMenu", NULL, N_("_Help") },

  { "NewGame", GTK_STOCK_NEW, N_("_New Game"), "<control>N", NULL, G_CALLBACK (new_game_cb) },
  { "Scores", NULL, N_("_Scores..."), NULL, NULL, G_CALLBACK (scores_cb) },
  { "UndoMove", GTK_STOCK_UNDO, N_("_Undo Move"), "<control>Z", NULL, G_CALLBACK (undo_cb) },
  { "RedoMove", GTK_STOCK_REDO, N_("_Redo Move"), "<shift><control>Z", NULL, G_CALLBACK (redo_cb) },
  { "Quit", GTK_STOCK_QUIT, NULL, NULL, NULL, G_CALLBACK (quit_cb) },

  { "Fullscreen", NULL, N_("_Fullscreen"), NULL, NULL, G_CALLBACK (fullscreen_cb) },
  { "Theme", NULL, N_("_Theme..."), NULL, NULL, G_CALLBACK (theme_cb) },

  { "Contents", GTK_STOCK_HELP, N_("_Contents"), "F1", NULL, G_CALLBACK (help_cb) },
  {"About", GTK_STOCK_ABOUT, NULL, NULL, NULL, G_CALLBACK (about_cb)
}

};

const GtkRadioActionEntry radio_actions[] = {
  { "SizeSmall", NULL, N_("_Small"), NULL, NULL, SMALL },
  { "SizeMedium", NULL, N_("_Medium"), NULL, NULL, MEDIUM },
  { "SizeLarge", NULL, N_("_Large"), NULL, NULL, LARGE }
};

const char *ui_description =
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu action='GameMenu'>"
"      <menuitem action='NewGame'/>"
"      <menuitem action='Scores'/>"
"      <separator/>"
"      <menuitem action='UndoMove'/>"
"      <menuitem action='RedoMove'/>"
"      <separator/>"
"      <menuitem action='Quit'/>"
"    </menu>"
"    <menu action='ViewMenu'>"
"      <menuitem action='Fullscreen'/>"
"      <menuitem action='Theme'/>"
"    </menu>"
"    <menu action='SizeMenu'>"
"      <menuitem action='SizeSmall'/>"
"      <menuitem action='SizeMedium'/>"
"      <menuitem action='SizeLarge'/>"
"    </menu>"
"    <menu action='HelpMenu'>"
"      <menuitem action='Contents'/>"
"      <menuitem action='About'/>"
"    </menu>"
"  </menubar>"
"</ui>";

static void build_gui (void)
{
  GtkWidget *appbar;
  GtkWidget *canvas;
	GtkWidget *vbox;
	GtkUIManager *ui_manager;
	GtkActionGroup *action_group;

	/* FIXME: Will need to initialise the pixmap array to zero. */
	init_pixmaps ();

	/* FIXME: Set the icon. */

  application = gnome_app_new (APPNAME, _(APPNAME_LONG));
  gtk_window_set_default_size (GTK_WINDOW (application), window_width,
			       window_height);
  g_signal_connect (G_OBJECT (application), "delete_event",
		    G_CALLBACK (quit_cb), NULL);
  g_signal_connect (G_OBJECT (application), "configure_event",
		    G_CALLBACK (window_resize_cb), NULL);

	vbox = gtk_vbox_new (FALSE, 0);
  gnome_app_set_contents (GNOME_APP (application), vbox);

	action_group = gtk_action_group_new ("MenuActions");
	gtk_action_group_set_translation_domain (action_group, GETTEXT_PACKAGE);
	gtk_action_group_add_actions (action_group, actions, G_N_ELEMENTS (actions),
																NULL);
	gtk_action_group_add_radio_actions (action_group, radio_actions, 
																			G_N_ELEMENTS (radio_actions), 
																			game_size - SMALL, 
																			G_CALLBACK (size_change_cb), 
																			NULL);
	
	ui_manager = gtk_ui_manager_new ();
	gtk_ui_manager_insert_action_group (ui_manager, action_group, 1);
	gtk_ui_manager_add_ui_from_string (ui_manager, ui_description, -1, NULL);
  
	gtk_window_add_accel_group (GTK_WINDOW (application),
															gtk_ui_manager_get_accel_group (ui_manager));

	gtk_box_pack_start (GTK_BOX (vbox), 
											gtk_ui_manager_get_widget (ui_manager, "/MainMenu"),
											FALSE, FALSE, 0);

	appbar = gtk_statusbar_new ();
	gtk_box_pack_end (GTK_BOX (vbox), appbar, FALSE, FALSE, 0);
	scorewidget = gtk_label_new ("");
	gtk_box_pack_end (GTK_BOX (appbar), scorewidget, TRUE, FALSE, 0);
	

  gridframe = games_grid_frame_new (board_width, board_height);
	games_grid_frame_set_padding (GAMES_GRID_FRAME (gridframe), 1, 1);
	gtk_box_pack_start (GTK_BOX (vbox), gridframe, TRUE, TRUE, 0);

  canvas = gtk_drawing_area_new ();
	g_object_set (G_OBJECT (canvas), "can-focus", TRUE, NULL);
	gtk_widget_add_events (canvas, GDK_KEY_PRESS_MASK | 
												 GDK_POINTER_MOTION_MASK |
												 GDK_BUTTON_PRESS_MASK | 
												 GDK_LEAVE_NOTIFY_MASK);
  g_signal_connect (G_OBJECT (canvas), "configure_event",
		    G_CALLBACK (configure_cb), NULL);
  g_signal_connect (G_OBJECT (canvas), "expose_event",
		    G_CALLBACK (expose_cb), NULL);
  g_signal_connect (G_OBJECT (canvas), "key_press_event",
		    G_CALLBACK (keyboard_cb), NULL);
  g_signal_connect (G_OBJECT (canvas), "motion_notify_event",
		    G_CALLBACK (mouse_movement_cb), NULL);
  g_signal_connect (G_OBJECT (canvas), "leave_notify_event",
		    G_CALLBACK (mouse_leave_cb), NULL);
	g_signal_connect (G_OBJECT (canvas), "button_press_event",
		    G_CALLBACK (mouse_click_cb), NULL);
  gtk_widget_set_size_request (canvas, MINIMUM_CANVAS_WIDTH, 
			       MINIMUM_CANVAS_HEIGHT);
  gtk_container_add (GTK_CONTAINER (gridframe), canvas);

  gtk_widget_show_all (application);
  
}

int main (int argc, char *argv[])
{
  static gchar *requested_theme = NULL;
  static gint   requested_size  = -1;

  static const struct poptOption options[] = {
    { "theme", 't', POPT_ARG_STRING, &requested_theme, 0, N_("Set the theme"),
      N_("NAME") },
    { "scenario", 's', POPT_ARG_STRING, &requested_theme, 0, 
      N_("For backwards compatibility"), N_("NAME") },
    { "size", 'z', POPT_ARG_INT, &requested_size, 0, 
      N_("Game size (1=small, 3=large)"), N_("NUMBER") },
    { NULL, '\0', 0, NULL, 0, NULL, NULL }};

  /* Initialise i18n. */
  bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
  textdomain (GETTEXT_PACKAGE);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");

  /* Initialise GNOME. */
  gnome_program_init (APPNAME, VERSION, LIBGNOMEUI_MODULE, argc, argv,
											GNOME_PARAM_APP_DATADIR, DATADIR,
											GNOME_PARAM_POPT_TABLE, options,  
											NULL);

  initialise_options (requested_size, requested_theme);

  build_gui ();

	/* FIXME: This should be one alternative of an if statement, the other
	 * alternative should be to load an old game. */
	new_game ();

  gtk_main ();

  return 0;
}
