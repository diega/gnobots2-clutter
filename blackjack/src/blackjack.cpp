// -*- mode:C++; tab-width:8; c-basic-offset:8; indent-tabs-mode:nil -*-
/*
 * Blackjack
 *
 * Copyright (C) 2003-2004 William Jon McCann <mccann@jhu.edu>
 *
 * This game is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
 * USA
 */

#include <config.h>

#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif

#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <ctype.h>
#include <glib/gi18n.h>

#include "blackjack.h"

#ifdef HAVE_GNOME
#include <gnome.h>
#endif

#include "events.h"
#include "draw.h"
#include "slot.h"
#include "card.h"
#include "menu.h"
#include "dialog.h"

#include "game.h"
#include "hand.h"

#include <iostream>
using namespace std;

// Global Variables

GtkWidget        *toplevel_window = NULL;
GtkWidget        *playing_area = NULL;
GtkWidget        *option_dialog = NULL;
GdkGC            *draw_gc = NULL;
GdkGC            *bg_gc = NULL;
GdkGC            *slot_gc = NULL;
GdkPixmap        *surface;

gchar            *card_style;

GtkWidget        *wager_value_label;

static GtkWidget *shoe_value_label = NULL;

GtkUIManager     *ui = NULL;

GtkWidget        *status_bar;
GtkWidget        *balance_value_label;

guint32          seed;

guint            x_spacing = 5;
guint            y_spacing = 15;
double           x_expanded_offset = 0.21;
double           y_expanded_offset = 0.21;

gboolean         events_pending = false;

static GConfClient *gconf_client = NULL;

gfloat wager_value = 5.0;
gfloat balance_value = 0.0;
gboolean quick_deals = FALSE;
gboolean show_probabilities = FALSE;
gboolean show_toolbar = TRUE;
gchar *game_variation = NULL;

#define DEFAULT_VARIATION      "Vegas_Strip.rules"

void
bj_make_window_title (gchar *game_name) 
{
        char *title;

        title = g_strdup_printf (_("Blackjack: %s"), game_name);

        gtk_window_set_title (GTK_WINDOW (toplevel_window), title); 

        g_free (title);
}

void
bj_gui_show_toolbar (gboolean do_toolbar)
{
        GtkWidget *widget = gtk_ui_manager_get_widget (ui, "/ToolBar");

        if (do_toolbar) {
                gtk_widget_show (widget);
        }
        else {
                gtk_widget_hide (widget);
                /*gtk_widget_queue_resize (toplevel_window);*/
        }
}

void
bj_show_balance (gfloat balance)
{
        gchar *b;
        b = g_strdup_printf ("%.2f  ", balance);

        if (balance_value_label)
                gtk_label_set_text (GTK_LABEL (balance_value_label), b);
        g_free (b);
}

void
bj_show_shoe_cards (gfloat value)
{
        gchar *str = g_strdup_printf ("%d", (gint)value);
        if (shoe_value_label)
                gtk_label_set_text (GTK_LABEL (shoe_value_label), str);
        g_free (str);
}

gdouble
bj_get_wager (void) 
{
        return wager_value;
}

void
bj_set_wager (gdouble value)
{
        if (value < 5)
                value = 5;
        if (value > 500)
                value = 500;

        wager_value = value;
        gchar *valstr = g_strdup_printf ("%.2f", value);
        gtk_label_set_text (GTK_LABEL (wager_value_label), valstr);
        g_free (valstr);
}

void
bj_adjust_wager (gdouble offset)
{
        gdouble wager;
        wager = bj_get_wager ();
        wager += offset;
        bj_set_wager (wager);
}

static void
bj_create_board (void)
{
        gtk_widget_set_events (playing_area, 
                               gtk_widget_get_events (playing_area) 
                               | GAME_EVENTS );

        GTK_WIDGET_SET_FLAGS (playing_area, GTK_CAN_FOCUS);
        gtk_widget_grab_focus (playing_area);

        g_signal_connect (playing_area, "expose_event",  
                          G_CALLBACK (bj_event_expose_callback), NULL);
        g_signal_connect (playing_area,"button_release_event",
                          G_CALLBACK (bj_event_button_release), NULL);
        g_signal_connect (playing_area, "motion_notify_event",
                          G_CALLBACK (bj_event_motion_notify), NULL);
        g_signal_connect (playing_area, "enter_notify_event",
                          G_CALLBACK (bj_event_enter_notify), NULL);
        g_signal_connect (playing_area, "button_press_event",
                          G_CALLBACK (bj_event_button_press), NULL);
        g_signal_connect (playing_area, "key_press_event",
                          G_CALLBACK (bj_event_key_press), NULL);
        g_signal_connect (playing_area, "configure_event",
                          G_CALLBACK (bj_event_playing_area_configure), NULL);
}

gboolean
bj_quit_app (void)
{
        delete rules;
        delete strategy;
        delete dealer;
        delete dealerProbabilities;
        delete shoe;
        delete distribution;

        g_list_free (playerHands);

        bj_slot_free_pixmaps ();
        g_object_unref (surface);
        g_object_unref (press_data->moving_cards);

        g_object_unref (gconf_client);
        gtk_widget_destroy (toplevel_window);
        gtk_main_quit ();

        return TRUE;
}

static void
control_menu_set_sensitive (const char *name,
                            gboolean    value)
{
        GtkWidget *widget;
        char      *path;

        path = g_strdup_printf ("/MenuBar/ControlMenu/%s", name);
        widget = gtk_ui_manager_get_widget (ui, path);
        gtk_widget_set_sensitive (widget, value);
        g_free (path);
}

void
bj_update_control_menu (void)
{
        gboolean  active;
        gboolean  value;

        active = bj_game_is_active ();
        control_menu_set_sensitive ("Deal", !active);

        value = active && bj_hand_can_be_hit ();
        control_menu_set_sensitive ("Hit", value);
        value = active;
        control_menu_set_sensitive ("Stand", value);
        value = active && bj_hand_can_be_split ();
        control_menu_set_sensitive ("Split", value);
        value = active && bj_hand_can_be_doubled ();
        control_menu_set_sensitive ("DoubleDown", value);
        value = active && bj_hand_can_be_surrendered ();
        control_menu_set_sensitive ("Surrender", value);
}

static void
create_main_window (void)
{
        GtkWidget    *vbox;
        GtkWidget    *hbox;
        GtkWidget    *group_box;
        GtkWidget    *label;
        GError       *error       = NULL;
        gint          width, height;
        char         *label_string;
        GtkActionGroup *actions;
        static const char *ui_definition =
                "<ui>"
                "  <menubar name='MenuBar'>"
                "    <menu name='GameMenu' action='game-menu'>"
                "      <menuitem name='New' action='new-game' />"
                "      <menuitem name='Restart' action='restart-game' />"
                "      <separator/>"
                "      <menuitem name='Hint' action='show-hint' />"
                "      <separator/>"
                "      <menuitem name='Quit' action='quit-game' />"
                "      <placeholder name='GameMenuAdditions' />"
                "    </menu>"
                "    <menu name='SettingsMenu' action='settings-menu'>"
                "      <menuitem name='Toolbar' action='show-toolbar'/>"
                "      <menuitem name='Preferences' action='show-preferences'/>"
                "    </menu>"
                "    <menu name='ControlMenu' action='control-menu'>"
                "      <menuitem name='Deal' action='hand-deal' />"
                "      <menuitem name='Hit' action='hand-hit' />"
                "      <menuitem name='Stand' action='hand-stand' />"
                "      <menuitem name='DoubleDown' action='hand-double-down' />"
                "      <menuitem name='Split' action='hand-split' />"
                "      <menuitem name='Surrender' action='hand-surrender' />"
                "      <placeholder name='ActionsMenuAdditions' />"
                "    </menu>"
                "    <menu name='HelpMenu' action='help-menu'>"
                "      <menuitem name='Contents' action='show-help-contents'/>"
                "      <menuitem name='About' action='show-about'/>"
                "    </menu>"
                "  </menubar>"
                "  <toolbar  name='ToolBar' action='toolbar'>"
                "    <placeholder name='GameToolItems'>"
                "      <toolitem name='New' action='new-game'/>"
                "      <toolitem name='Restart' action='restart-game'/>"
                "      <separator/>"
                "      <toolitem name='Hint' action='show-hint'/>"
                "    </placeholder>"
                "  </toolbar>"
                "</ui>";

        static GtkActionEntry entries [] = {
                { "game-menu", NULL, N_("_Game") },
                { "new-game", GTK_STOCK_NEW, N_("_New game"), "<Control>N", N_("Start a new game"), G_CALLBACK (on_game_new_activate) },
                { "restart-game", GTK_STOCK_REFRESH, N_("_Restart game"), "<Control>R", N_("Restart the current game"), G_CALLBACK (on_game_restart_activate) },
                { "show-hint", GTK_STOCK_HELP, N_("_Hint"), NULL, N_("Get a hint for the next action"), G_CALLBACK (on_game_hint_activate) },
                { "quit-game", GTK_STOCK_QUIT, NULL, "<Control>Q", N_("Quit application"), G_CALLBACK (on_game_quit_activate) },
                { "settings-menu", NULL, N_("_Settings") },
                { "show-preferences", GTK_STOCK_PREFERENCES, NULL, NULL, N_("Show game preferences"), G_CALLBACK (on_preferences_activate) },
                { "control-menu", NULL, N_("_Control") },
                { "hand-deal", NULL, N_("Deal"), NULL, N_("Deal a new hand"), G_CALLBACK (on_control_deal_activate) },
                { "hand-hit", NULL, N_("_Hit"), "H", N_("Add a card to the hand"), G_CALLBACK (on_control_hit_activate) },
                { "hand-stand", NULL, N_("_Stand"), "S", N_("Stop adding cards to the hand"), G_CALLBACK (on_control_stand_activate) },
                { "hand-surrender", NULL, N_("Surrender"), "R", N_("Surrender the hand"), G_CALLBACK (on_control_surrender_activate) },
                { "hand-double-down", NULL, N_("_Double down"), "D", N_("Double down"), G_CALLBACK (on_control_double_activate) },
                { "hand-split", NULL, N_("Split the hand"), "P", N_("Split the hand"), G_CALLBACK (on_control_split_activate) },
                { "help-menu", NULL, N_("_Help")},
                { "show-help-contents", GTK_STOCK_HELP, N_("_Contents"), "F1", N_("Display help for this game"), G_CALLBACK (on_help_contents_activate) },
                { "show-about", GTK_STOCK_ABOUT, NULL, NULL, N_("About this game"), G_CALLBACK (on_help_about_activate) },
        };

	static GtkToggleActionEntry toggle_entries [] = {
                { "show-toolbar", NULL, N_("_Toolbar"), "<Control>T", N_("Show toolbar"), G_CALLBACK (on_toolbar_activate), show_toolbar },
        };

        toplevel_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

        ui = gtk_ui_manager_new ();
        gtk_ui_manager_add_ui_from_string (ui, ui_definition, -1, &error);

        actions = gtk_action_group_new ("Actions");
        gtk_action_group_set_translation_domain (actions, GETTEXT_PACKAGE);
        gtk_action_group_add_actions (actions, entries, G_N_ELEMENTS (entries), toplevel_window);
        gtk_action_group_add_toggle_actions (actions, toggle_entries, G_N_ELEMENTS (toggle_entries), toplevel_window);
        gtk_ui_manager_insert_action_group (ui, actions, 0);
        gtk_window_add_accel_group (GTK_WINDOW (toplevel_window), gtk_ui_manager_get_accel_group (ui));
    
        vbox = gtk_vbox_new (FALSE, 0);
        gtk_container_add (GTK_CONTAINER (toplevel_window), vbox);

        gtk_box_pack_start (GTK_BOX (vbox), gtk_ui_manager_get_widget (ui, "/MenuBar"), FALSE, FALSE, 0);
        gtk_box_pack_start (GTK_BOX (vbox), gtk_ui_manager_get_widget (ui, "/ToolBar"), FALSE, FALSE, 0);

        playing_area = gtk_drawing_area_new ();
        gtk_box_pack_start (GTK_BOX (vbox), playing_area, TRUE, TRUE, 0);

        hbox = gtk_hbox_new (FALSE, 6);
        gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

        group_box = gtk_hbox_new (FALSE, 0);
        gtk_box_pack_start (GTK_BOX (hbox), group_box, FALSE, FALSE, 0);

        label = gtk_label_new (_("Cards left:"));
        gtk_box_pack_start (GTK_BOX (group_box), label, FALSE, FALSE, 0);

        shoe_value_label = gtk_label_new ("");
        gtk_box_pack_start (GTK_BOX (group_box), shoe_value_label, FALSE, FALSE, 0);

        group_box = gtk_hbox_new (FALSE, 0);
        gtk_box_pack_start (GTK_BOX (hbox), group_box, FALSE, FALSE, 0);

        label = gtk_label_new (_("Wager:"));
        gtk_box_pack_start (GTK_BOX (group_box), label, FALSE, FALSE, 0);

        wager_value = 5.0;
        label_string = g_strdup_printf ("%.2f", wager_value);
        wager_value_label = gtk_label_new (label_string);
        g_free (label_string);
        gtk_box_pack_start (GTK_BOX (group_box), wager_value_label, FALSE, FALSE, 0);


        group_box = gtk_hbox_new (FALSE, 0);
        gtk_box_pack_start (GTK_BOX (hbox), group_box, FALSE, FALSE, 0);

        label = gtk_label_new (_("Balance:"));
        gtk_box_pack_start (GTK_BOX (group_box), label, FALSE, FALSE, 0);
        label_string = g_strdup_printf ("%.2f", balance_value);
        balance_value_label = gtk_label_new (label_string);
        g_free (label_string);
        gtk_box_pack_start (GTK_BOX (group_box), balance_value_label, FALSE, FALSE, 0);


        status_bar = gtk_statusbar_new ();
        gtk_box_pack_start (GTK_BOX (hbox), status_bar, TRUE, TRUE, 0);

        
        if (!gconf_client)
                gconf_client = gconf_client_get_default ();

        width = gconf_client_get_int (gconf_client, GCONF_KEY_WIDTH, NULL);
        height = gconf_client_get_int (gconf_client, GCONF_KEY_HEIGHT, NULL);

        gtk_window_set_default_size (GTK_WINDOW (toplevel_window), width, height);

        bj_update_control_menu ();

        g_signal_connect (toplevel_window, "delete_event", 
                          G_CALLBACK (bj_quit_app), NULL);
        g_signal_connect (toplevel_window, "configure_event",
                          G_CALLBACK (bj_event_configure), NULL);
}

static void
main_prog (int argc, char *argv[])
{
        gchar *lcard_style;
        gchar *label_string;
        guint context_id;

        create_main_window ();

        bj_slot_load_pixmaps ();
 
        bj_create_board ();

        context_id = gtk_statusbar_get_context_id (GTK_STATUSBAR (status_bar),
                                                   "help");
        gtk_statusbar_push (GTK_STATUSBAR (status_bar), context_id,
                            _("Place your wager or deal a hand"));

        bj_game_new (bj_get_game_variation (), NULL);

        gtk_widget_show_all (toplevel_window);

        bj_gui_show_toolbar (show_toolbar);

        bj_press_data_create ();
        bj_chip_stack_press_data_create ();

        gtk_widget_pop_colormap ();

        gtk_main ();
}

GConfClient *
bj_gconf_client ()
{
        if (!gconf_client) 
                gconf_client = gconf_client_get_default ();
        return gconf_client;
}

static void
bj_gconf_balance_cb (GConfClient *client, guint cnxn_id, 
                     GConfEntry *entry, gpointer user_data)
{
        balance_value = gconf_client_get_float (client, GCONF_KEY_BALANCE, NULL);
        bj_show_balance (balance_value);
}

static void
bj_gconf_card_style_cb (GConfClient *client, guint cnxn_id, 
                        GConfEntry *entry, gpointer user_data)
{
        gchar *lcard_style;

        lcard_style = bj_get_card_style ();
        bj_card_set_theme (lcard_style);
        g_free (lcard_style);
        bj_draw_refresh_screen ();
}

static void
bj_gconf_show_probabilities_cb (GConfClient *client, guint cnxn_id, 
                                GConfEntry *entry, gpointer user_data)
{
        show_probabilities = gconf_client_get_bool (client,
                                                    GCONF_KEY_SHOW_PROBABILITIES,
                                                    NULL);
        bj_draw_refresh_screen ();
}

static void
bj_gconf_quick_deal_cb (GConfClient *client, guint cnxn_id, 
                        GConfEntry *entry, gpointer user_data)
{
        quick_deals = gconf_client_get_bool (client, GCONF_KEY_QUICK_DEAL, NULL);
        bj_draw_refresh_screen ();
}

static void
bj_gconf_show_toolbar_cb (GConfClient *client, guint cnxn_id, 
                          GConfEntry *entry, gpointer user_data)
{
        gboolean lshow_toolbar;

        lshow_toolbar = gconf_client_get_bool (client, GCONF_KEY_SHOW_TOOLBAR, NULL);
        bj_gui_show_toolbar (lshow_toolbar);
}

gchar *
bj_get_card_style ()
{
        gchar *lcard_style;

        lcard_style = gconf_client_get_string (bj_gconf_client (), 
                                               GCONF_KEY_CARD_STYLE,
                                               NULL);
        return lcard_style;
}

void
bj_set_card_style (gchar *value)
{
        gconf_client_set_string (bj_gconf_client (), 
                                 GCONF_KEY_CARD_STYLE,
                                 value,
                                 NULL);
}

gboolean
bj_get_show_probabilities ()
{
        return show_probabilities;
}
void
bj_set_show_probabilities (gboolean value)
{
        gconf_client_set_bool (bj_gconf_client (), 
                               GCONF_KEY_SHOW_PROBABILITIES,
                               value, NULL);
}

gboolean
bj_get_show_toolbar ()
{
        return show_toolbar;
}
void
bj_set_show_toolbar (gboolean value)
{
        show_toolbar = value;
        bj_gui_show_toolbar (value);
        gconf_client_set_bool (bj_gconf_client (), 
                               GCONF_KEY_SHOW_TOOLBAR,
                               value, NULL);
}

gboolean
bj_get_quick_deal ()
{
        return quick_deals;
}
void
bj_set_quick_deal (gboolean value)
{
        gconf_client_set_bool (bj_gconf_client (), 
                               GCONF_KEY_QUICK_DEAL,
                               value, NULL);
}

gchar *
bj_get_game_variation ()
{
        return g_strdup (game_variation);
}

void
bj_set_game_variation (const gchar *value)
{
        if (game_variation)
                g_free (game_variation);
        game_variation = g_strdup (value);
        gconf_client_set_string (bj_gconf_client (), 
                                 GCONF_KEY_GAME_VARIATION,
                                 game_variation, NULL);
}

gdouble
bj_get_balance ()
{
        return balance_value;
}

void
bj_set_balance (gdouble balance)
{
        balance_value = balance;
        bj_show_balance (balance_value);
        gconf_client_set_float (bj_gconf_client (),
                                GCONF_KEY_BALANCE,
                                balance, NULL);
}

void
bj_adjust_balance (gdouble offset)
{
        gdouble balance;
        balance = bj_get_balance ();
        balance += offset;
        bj_set_balance (balance);
}

static void
bj_gconf_init (GConfClient *client)
{
        gdouble balance;
        gchar *variation_tmp;

        variation_tmp = gconf_client_get_string (client,
                                                 GCONF_KEY_GAME_VARIATION,
                                                 NULL);
        if (variation_tmp) {
                game_variation = g_strdup (variation_tmp);
                g_free (variation_tmp);
        }
        else
                game_variation = g_strdup (DEFAULT_VARIATION);

        balance = gconf_client_get_float (client,
                                          GCONF_KEY_BALANCE,
                                          NULL);
        if (balance)
                balance_value = balance;

        show_probabilities = gconf_client_get_bool (client,
                                                    GCONF_KEY_SHOW_PROBABILITIES,
                                                    NULL);
        quick_deals = gconf_client_get_bool (client, 
                                             GCONF_KEY_QUICK_DEAL,
                                             NULL);
        show_toolbar = gconf_client_get_bool (client, 
                                              GCONF_KEY_SHOW_TOOLBAR,
                                              NULL);

        gconf_client_notify_add (client,
                                 GCONF_KEY_BALANCE,
                                 bj_gconf_balance_cb,
                                 NULL, NULL, NULL);
        gconf_client_notify_add (client,
                                 GCONF_KEY_CARD_STYLE,
                                 bj_gconf_card_style_cb,
                                 NULL, NULL, NULL);
        gconf_client_notify_add (client,
                                 GCONF_KEY_SHOW_PROBABILITIES,
                                 bj_gconf_show_probabilities_cb,
                                 NULL, NULL, NULL);
        gconf_client_notify_add (client,
                                 GCONF_KEY_QUICK_DEAL,
                                 bj_gconf_quick_deal_cb,
                                 NULL, NULL, NULL);
        gconf_client_notify_add (client,
                                 GCONF_KEY_SHOW_TOOLBAR,
                                 bj_gconf_show_toolbar_cb,
                                 NULL, NULL, NULL);
}

int
main (int argc, char *argv [])
{
        GOptionContext     *context;
        GError             *error      = NULL;
        char               *variation  = NULL;
        gboolean            retval;
        static GOptionEntry entries [] = {
                { "variation", 0, 0, G_OPTION_ARG_STRING, &variation,
                  N_("Blackjack rule set to use"), NULL },
                { NULL }
        };

        bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
        bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
        textdomain (GETTEXT_PACKAGE);

        context = g_option_context_new (NULL);
        g_option_context_set_ignore_unknown_options (context, TRUE);
        g_option_context_add_main_entries (context, entries, NULL);
        retval = g_option_context_parse (context, &argc, &argv, &error);
        if (!variation)
                variation = g_strdup (DEFAULT_VARIATION);

        gtk_init_with_args (&argc, &argv, NULL, NULL, NULL, &error);

        gconf_init (argc, argv, NULL);
        gconf_client_add_dir (bj_gconf_client (), GCONF_KEY_DIR,
                              GCONF_CLIENT_PRELOAD_NONE, NULL);

#ifdef HAVE_GNOME
        gnome_program_init ("blackjack", VERSION,
                            LIBGNOMEUI_MODULE,
                            argc, argv,
                            GNOME_PARAM_APP_DATADIR, DATADIR, NULL);
#endif

        gtk_widget_push_colormap (gdk_rgb_get_colormap ());

        gtk_window_set_default_icon_from_file (PIXMAPDIR"/gnome-blackjack.png", NULL);

        bj_gconf_init (bj_gconf_client ());

        bj_game_find_rules (variation);

        main_prog (argc, argv);

        g_free (variation);

        return 0;
}
