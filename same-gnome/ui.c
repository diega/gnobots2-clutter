/* -*- mode: C; indent-tabs-mode: t; tab-width: 2; c-basic-offset: 2; -*- */

/* ui.c : User interface handling.
 *
 * Copyright (c) 2004 by Callum McKenzie
 *
 */

/* FIXME: The original didn't have to include this, what has chagned ? */
#include <libintl.h>
#include <gnome.h>

#include <games-gridframe.h>
#include <games-scores-dialog.h>

#include "config.h"

#include "same-gnome.h"

#include "drawing.h"
#include "game.h"
#include "input.h"
#include "ui.h"

/* Define an alternative to ngettext if we don't have it. Of course it isn't
 * a proper substitute for ngettext, but it is the best we can do. */
#ifndef HAVE_NGETTEXT
#define ngettext(one,lots,n) gettext(lots)
#endif

/* A collection of widgets we need to reference repeatedly. */
GtkWidget *application;
GtkWidget *messagewidget;
GtkWidget *scorewidget;
GtkWidget *gridframe = NULL;
GtkToggleAction *fullscreenaction;
GtkWidget *undo_widget;
GtkWidget *redo_widget;

/* All quit events must go through here to ensure consistant behaviour. */
static void quit_cb (void)
{
  gtk_main_quit ();
}

void set_message (gint count)
{
	gchar *message;
	gchar *part1;
	gchar *part2;
	gint s;

	if (count > 1) {
		s = calculate_score (count);
		part1 = g_strdup_printf (ngettext ("%d object selected", 
																			 "%d objects selected", count), count);
		part2 = g_strdup_printf (ngettext ("%d point", "%d points", s), s);
		/* Translators: the previous messages get merged into this 
		 * format string. */
		message = g_strdup_printf (_("%s (%s)"), part1, part2);
		gtk_label_set_text (GTK_LABEL (messagewidget), message);
		g_free (message);
	} else {
		gtk_label_set_text (GTK_LABEL (messagewidget), _("No objects selected"));
	}
}

void set_message_destroyed (gint count)
{
	gchar *message;
	gint s;

	if (count < 3) {
		gtk_label_set_text (GTK_LABEL (messagewidget), _("No points"));
		return;
	}

	s = calculate_score (count);
	message = g_strdup_printf (ngettext ("%d point !", "%d points !", s), s);
	gtk_label_set_text (GTK_LABEL (messagewidget), message);
	g_free (message);
} 

void set_message_general (gchar *message)
{
	gtk_label_set_text (GTK_LABEL (messagewidget), message);
}

void show_score (gint score)
{
  gchar *label;
  
  label = g_strdup_printf (_("Score: %d"), score);
  gtk_label_set_text (GTK_LABEL (scorewidget), label);
  g_free (label);
}

void  new_frame_ratio (gint board_width, gint board_height)
{
  /* FIXME: The flow should avoid the need for this test. */
  if (gridframe)
    games_grid_frame_set (GAMES_GRID_FRAME (gridframe), board_width, 
			  board_height);
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

void game_over_dialog (void)
{
  GtkWidget *dialog;
	gchar *message;
  
	message = g_strdup_printf ("<b>%s</b>", _("Game over!"));
  dialog = gtk_message_dialog_new_with_markup (GTK_WINDOW (application),
																							 GTK_DIALOG_DESTROY_WITH_PARENT,
																							 GTK_MESSAGE_INFO,
																							 GTK_BUTTONS_NONE,
																							 message);
	g_free (message);
  
	gtk_dialog_add_buttons (GTK_DIALOG (dialog), 
													GTK_STOCK_QUIT, GTK_RESPONSE_CLOSE,
													_("New Game"), GTK_RESPONSE_ACCEPT,
													NULL);
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT);

  /* FIXME: We should provide an indication of how good the score was. */
  /* FIXME: Better text and formatting. */
  
  switch (gtk_dialog_run (GTK_DIALOG (dialog))) {
	case GTK_RESPONSE_CLOSE:
		quit_cb ();
		break;
	case GTK_RESPONSE_ACCEPT:
	default:
		new_game ();
		break;
	}
  
  gtk_widget_destroy (dialog);
}

static void new_game_cb (void)
{
	new_game ();
}

static void theme_cb (void)
{

}

static void fullscreen_cb (GtkToggleAction *action)
{
  if (gtk_toggle_action_get_active (action)) {
    gtk_window_fullscreen (GTK_WINDOW (application));
  } else {
    gtk_window_unfullscreen (GTK_WINDOW (application));
  }
}

/* Just in case something else takes us to/from fullscreen. */
static void window_state_cb (GtkWidget *widget, GdkEventWindowState *event)
{
  if (!(event->changed_mask & GDK_WINDOW_STATE_FULLSCREEN))
    return;
    
  gtk_toggle_action_set_active (fullscreenaction,
				event->new_window_state & GDK_WINDOW_STATE_FULLSCREEN);
}

static void scores_cb (void)
{
	GtkWidget *dialog;
	gint i;

	dialog = games_scores_dialog_new (APPNAME, _("Same GNOME Scores"));
	for (i=0; i <= (LARGE - SMALL); i++) {
		games_scores_dialog_add_category (GAMES_SCORES_DIALOG (dialog),
																			scorenames[i],
																			_(scorenames[i]));
	}
	games_scores_dialog_set_category (GAMES_SCORES_DIALOG (dialog), 
																		scorenames[game_size - SMALL]);
	
	games_scores_dialog_set_category_description (GAMES_SCORES_DIALOG (dialog),
																								_("Size:"));

	gtk_dialog_run (GTK_DIALOG (dialog));

	gtk_widget_destroy (dialog);
}

static void undo_cb (void)
{
	/* If we are in the middle of the animations we don't do a proper undo 
	 * (because we haven't worked out the new state yet). Instead we reset
	 * to the currently recorded state. */
	if ((game_state == GAME_DESTROYING) ||
			(game_state == GAME_MOVING_DOWN) ||
			(game_state == GAME_MOVING_LEFT))
		restore_game_state ();
	else
		undo ();
	redraw ();
	select_cells ();
}

static void redo_cb (void)
{
	/* FIXME: If we undo in the middle of the animation then we can't redo
	 * that move. This is an artifact of how we work out the next state 
	 * (during the animation) and our work-around so undo behaves sanely.
	 * At the moment this is a trade-off. Ideally we work out the new state
	 * immediately (this will also allow us to make te animation quicker
	 * by allowing already fallen columns to collapse leftward). However
	 * this would make the animation code more complicated. */
	redo ();
	redraw ();
	select_cells ();
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

const GtkToggleActionEntry toggle_actions[] = {
  { "Fullscreen", NULL, N_("_Fullscreen"), NULL, NULL, G_CALLBACK (fullscreen_cb) }
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

void build_gui (void)
{
  GtkWidget *hbox;
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
  g_signal_connect (G_OBJECT (application), "window_state_event",
		    G_CALLBACK (window_state_cb), NULL);

  vbox = gtk_vbox_new (FALSE, 0);
  gnome_app_set_contents (GNOME_APP (application), vbox);
  
  action_group = gtk_action_group_new ("MenuActions");
  gtk_action_group_set_translation_domain (action_group, GETTEXT_PACKAGE);
  gtk_action_group_add_actions (action_group, actions, G_N_ELEMENTS (actions),
				NULL);
	/* FIXME: This doesn't set the radio button correctly. Try setting the
	 * game to medium, relaunching and then looking at the size menu. */
  gtk_action_group_add_radio_actions (action_group, radio_actions, 
				      G_N_ELEMENTS (radio_actions), 
				      game_size - SMALL, 
				      G_CALLBACK (size_change_cb), 
				      NULL);
  gtk_action_group_add_toggle_actions (action_group, toggle_actions,
				       G_N_ELEMENTS (toggle_actions), NULL);

  fullscreenaction = GTK_TOGGLE_ACTION (gtk_action_group_get_action (action_group, "Fullscreen"));
  
  ui_manager = gtk_ui_manager_new ();
  gtk_ui_manager_insert_action_group (ui_manager, action_group, 1);
  gtk_ui_manager_add_ui_from_string (ui_manager, ui_description, -1, NULL);

	undo_widget = gtk_ui_manager_get_widget (ui_manager, 
																					 "/MainMenu/GameMenu/UndoMove");
	redo_widget = gtk_ui_manager_get_widget (ui_manager, 
																					 "/MainMenu/GameMenu/RedoMove");
  
  gtk_window_add_accel_group (GTK_WINDOW (application),
			      gtk_ui_manager_get_accel_group (ui_manager));
  
  gtk_box_pack_start (GTK_BOX (vbox), 
		      gtk_ui_manager_get_widget (ui_manager, "/MainMenu"),
		      FALSE, FALSE, 0);  
  
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

	gtk_box_pack_start (GTK_BOX (vbox), gtk_hseparator_new (), FALSE, FALSE, 0);

  hbox = gtk_hbox_new (TRUE, 0);
  gtk_box_pack_end (GTK_BOX (vbox), hbox, FALSE, FALSE, GNOME_PAD);

	messagewidget = gtk_label_new ("");
	gtk_box_pack_start (GTK_BOX (hbox), messagewidget, TRUE, FALSE, 0);

  scorewidget = gtk_label_new ("");
  gtk_box_pack_start (GTK_BOX (hbox), scorewidget, TRUE, FALSE, 0);
  
  gtk_widget_show_all (application);
  
}

void set_undoredo_sensitive (gboolean undo, gboolean redo)
{
	gtk_widget_set_sensitive (undo_widget, undo);
	gtk_widget_set_sensitive (redo_widget, redo);
}
