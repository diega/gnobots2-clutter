// -*- mode:C++; tab-width:2; c-basic-offset:2; indent-tabs-mode:nil -*-
/* Blackjack - dialog.cpp
 * Copyright (C) 2003 William Jon McCann <mccann@jhu.edu>
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <dirent.h>
#include "gnome.h"
#include "blackjack.h"
#include "menu.h"
#include "dialog.h"
#include "draw.h"
#include "events.h"
#include "splash.h"

#include "game.h"
#include "hand.h"

#include <iostream>
using namespace std;

static GtkWidget *hint_dlg = NULL;
static GtkObject* deck_edit = NULL;

gboolean
get_insurance_choice ()
{
  GtkWidget* dialog;
  gchar* message;
  gboolean choice = false;

  message = _("Would you like insurance?");

  dialog = gtk_message_dialog_new (GTK_WINDOW (app),
                                   GTK_DIALOG_MODAL,
                                   GTK_MESSAGE_QUESTION,
                                   GTK_BUTTONS_YES_NO,
                                   message);

  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_YES);

  /* add a stock icon? */ 
  switch (gtk_dialog_run (GTK_DIALOG (dialog)))
    {
    case GTK_RESPONSE_YES: 
      {
	choice = true;
      }
      break;
    default:
      choice = false;
      break;
  }
  gtk_widget_destroy (dialog);

  return choice;
}

void
select_rule_cb (GtkTreeSelection *select, gpointer data)
{
	GtkTreeModel *model;
	GtkTreeIter iter;
  gchar *filename = NULL;

	if (gtk_tree_selection_get_selected (select, &model, &iter))
    {
      gtk_tree_model_get (model, &iter, 1, &filename, -1);
      if (g_ascii_strcasecmp (filename, bj_game_get_rules_file ()))
        {
          splash_new ();
          bj_game_new (filename, &seed);
          splash_destroy ();
        }
    }
}

static void
hint_destroy_callback (void)
{
  hint_dlg = NULL;
}

void
show_hint_dialog ()
{
  gchar *gmessage;

  if (bj_game_is_first_hand ()) {
    gmessage = g_strdup (_("Set your wager and click in the white outline to deal a new hand."));
  }  
  else if (! bj_game_is_active ()) {
    gmessage = g_strdup (_("Set your wager or click on the cards to deal a new hand."));
  }  
  else {
    gmessage = bj_hand_get_best_option_string ();
  }
  
  if (hint_dlg)
    gtk_widget_destroy (GTK_WIDGET (hint_dlg));

  hint_dlg = gtk_message_dialog_new (GTK_WINDOW (app),
	                                   GTK_DIALOG_DESTROY_WITH_PARENT,
	                                   GTK_MESSAGE_INFO,
	                                   GTK_BUTTONS_OK,
	                                   gmessage);

  if (hint_dlg)
    {
      g_signal_connect (G_OBJECT (hint_dlg),
                        "destroy",
                        (GtkSignalFunc) hint_destroy_callback,
                        NULL);
    }

	gtk_dialog_run (GTK_DIALOG (hint_dlg));
	gtk_widget_destroy (hint_dlg);

  g_free (gmessage);
}

GtkWidget *
get_main_page (GtkWidget* dialog)
{
  GtkWidget *retval = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);

  gtk_widget_show_all (retval);
  return retval;
}

GtkWidget *
get_background_page (GtkWidget* dialog)
{
  GtkWidget *retval = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);

  gtk_widget_show_all (retval);
  return retval;
}

void 
pref_dialog_response (GtkWidget *w, int response, gpointer data)
{
  GdkCardDeckOptions deck_options = NULL;

  gtk_widget_hide (w);

  if (gdk_card_deck_options_edit_dirty
      (GDK_CARD_DECK_OPTIONS_EDIT (deck_edit)))
    {
      deck_options =
        gdk_card_deck_options_edit_get (GDK_CARD_DECK_OPTIONS_EDIT (deck_edit));
      bj_set_deck_options (deck_options);
    }
}

void
card_deck_options_changed (GtkWidget *w, gpointer data)
{
  GdkCardDeckOptions deck_options = NULL;

  if (GDK_IS_CARD_DECK_OPTIONS_EDIT (deck_edit)) 
    {
#if 0
      // FIXME: why doesn't this work???
      deck_options = gdk_card_deck_options_edit_get
        (GDK_CARD_DECK_OPTIONS_EDIT (deck_edit));
      bj_set_deck_options (deck_options);
#endif
    }
}

void
show_probabilities_toggle_cb (GtkToggleButton *w, gpointer data)
{
  gboolean is_on = gtk_toggle_button_get_active (w);
  bj_set_show_probabilities (is_on);
}

void
quick_deal_toggle_cb (GtkToggleButton *w, gpointer data)
{
  gboolean is_on = gtk_toggle_button_get_active (w);
  bj_set_quick_deal (is_on);
}

void
show_preferences_dialog () 
{
  static GtkWidget* pref_dialog = NULL;
  static GtkWidget* notebook = NULL;
  GdkCardDeckOptions deck_options = NULL;
  GtkWidget *hbox, *vbox;
  GtkWidget *toggle;
  gboolean show_probabilities = false;
  gboolean quick_deal = false;
  static GtkListStore* list;
  static GtkWidget* list_view;
  GtkWidget* scrolled_window;
  GtkTreeViewColumn* column;
  GtkCellRenderer* renderer;
  GtkTreeSelection* select;
  GtkTreeIter iter;

  if (!pref_dialog)
    {
      pref_dialog = gtk_dialog_new_with_buttons (_("Blackjack Preferences"),
                                                 GTK_WINDOW (app), 
                                                 GTK_DIALOG_DESTROY_WITH_PARENT,
                                                 GTK_STOCK_CLOSE, 
                                                 GTK_RESPONSE_CLOSE,
                                                 NULL);
      notebook = gtk_notebook_new ();
      gtk_container_add (GTK_CONTAINER (GTK_DIALOG (pref_dialog)->vbox),
                         notebook);

      // Game Tab
      hbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);
      gtk_notebook_append_page ( GTK_NOTEBOOK (notebook), hbox, 
                                 gtk_label_new (_("Game")) );
      vbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);
      gtk_box_pack_start_defaults(GTK_BOX (hbox), vbox);

      // Show probabilities
      show_probabilities = bj_get_show_probabilities ();
      toggle = gtk_check_button_new_with_label
        (_("Display hand probabilities"));
      gtk_toggle_button_set_active
        (GTK_TOGGLE_BUTTON (toggle), show_probabilities);
      gtk_box_pack_start(GTK_BOX (vbox), toggle,
                         FALSE, FALSE, 0);
      g_signal_connect (GTK_OBJECT (toggle), "toggled",
                        G_CALLBACK (show_probabilities_toggle_cb), NULL);

      // Quick deal
      quick_deal = bj_get_quick_deal ();
      toggle = gtk_check_button_new_with_label
        (_("Quick deals (no delay between each card)"));
      gtk_toggle_button_set_active
        (GTK_TOGGLE_BUTTON (toggle), quick_deal);
      gtk_box_pack_start(GTK_BOX (vbox), toggle,
                         FALSE, FALSE, 0);
      g_signal_connect (GTK_OBJECT (toggle), "toggled",
                        G_CALLBACK (quick_deal_toggle_cb), NULL);


      // Rules Tab
      hbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);
      gtk_notebook_append_page ( GTK_NOTEBOOK (notebook), hbox, 
                                 gtk_label_new (_("Rules")) );
      vbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);
      gtk_box_pack_start_defaults(GTK_BOX (hbox), vbox);

      list = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING);
      list_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (list));
      gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (list_view), FALSE);
      g_object_unref (G_OBJECT (list));
    
      renderer = gtk_cell_renderer_text_new ();
      column = gtk_tree_view_column_new_with_attributes
        (_("Rules"), renderer, "text", 0, NULL);
    
      gtk_tree_view_append_column (GTK_TREE_VIEW (list_view), column);

      select = gtk_tree_view_get_selection (GTK_TREE_VIEW (list_view));
      gtk_tree_selection_set_mode (select, GTK_SELECTION_BROWSE); 

      scrolled_window = gtk_scrolled_window_new (NULL, NULL);
      gtk_scrolled_window_set_shadow_type 
        (GTK_SCROLLED_WINDOW (scrolled_window), GTK_SHADOW_IN);
      gtk_widget_set_size_request (scrolled_window, 300, 250);
      gtk_container_add (GTK_CONTAINER (scrolled_window), list_view);

      gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                      GTK_POLICY_NEVER,
                                      GTK_POLICY_AUTOMATIC);
    
      gtk_box_pack_end (GTK_BOX (vbox), scrolled_window, TRUE, TRUE,
                        GNOME_PAD_SMALL );

      g_signal_connect (G_OBJECT (select), "changed", 
                        GTK_SIGNAL_FUNC (select_rule_cb), NULL);

      gchar *current_rule;
      current_rule = bj_get_game_variation ();
      gint i = 0;
      for (GList *temptr = bj_game_get_rules_list (); temptr; temptr=temptr->next)
        {
          gchar *text;
          gint row;
          text = bj_game_file_to_name ((gchar*)temptr->data);
          gtk_list_store_append (GTK_LIST_STORE (list), &iter);
          gtk_list_store_set (GTK_LIST_STORE (list), 
                              &iter, 0, text, 1,
                              (gchar*)temptr->data, -1);
          if (! g_ascii_strcasecmp (current_rule, (gchar*)temptr->data))
            {
              gtk_tree_view_set_cursor (GTK_TREE_VIEW (list_view),
                                        gtk_tree_path_new_from_indices (i, -1),
                                        NULL, FALSE);
            }
          i++;
        }


      // Cards Tab
      deck_edit = gdk_card_deck_options_edit_new (GTK_NOTEBOOK (notebook));

      g_signal_connect (G_OBJECT (deck_edit), "changed",
                        G_CALLBACK (card_deck_options_changed), NULL);
    
      // General signals
      g_signal_connect (G_OBJECT (pref_dialog), "response",
                        G_CALLBACK (pref_dialog_response), NULL);

      g_signal_connect(G_OBJECT (pref_dialog), "delete_event",
                       GTK_SIGNAL_FUNC (gtk_widget_hide), NULL);
    }

  if (pref_dialog && !GTK_WIDGET_VISIBLE (pref_dialog))
    {
      deck_options = bj_get_deck_options ();
      gdk_card_deck_options_edit_set (GDK_CARD_DECK_OPTIONS_EDIT (deck_edit),
                                      deck_options);
      gtk_widget_show_all(pref_dialog);
    }
}
