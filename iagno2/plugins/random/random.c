/*
 * Random Player for Iagno II: A plugin for Iagno II
 * Copyright (C) 1999-2000 Ian Peters <itp@gnu.org>
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

#include <gnome.h>

#include <sys/time.h>

#include "../../reversi-iagno2/reversi.h"
#include "../../src/iagno2.h"

#define VERSION "0.1"

/*
GtkWidget *plugin_pref_dialog = NULL;
*/

gchar busy_message[2][50];
gchar *new_message[2] = { NULL, NULL };

void
plugin_init_player (gint player)
{
  gchar *pref_file;
  gchar *tmp_message;

  pref_file = g_strdup_printf ("/iagno2/Random/player%d_busy_message=Random Player is stalling for time...", player);
  tmp_message = gnome_config_get_string (pref_file);
  g_free (pref_file);

  g_snprintf (busy_message[player], 50, "%s", tmp_message);
  g_free (tmp_message);
}

void
plugin_deinit_player (gint player)
{
  if (new_message[player]) {
    g_free (new_message[player]);
    new_message[player] = NULL;
  }
}

void
plugin_setup (gint player)
{
  struct timeval tv;

  gettimeofday (&tv, NULL);
  srand (tv.tv_usec);
}

gint
plugin_move (ReversiBoard *board, gint player)
{
  gint i;
  gint moves[32];
  gint nummoves = 0;

  for (i = 0; i < 64; i++) {
    if (is_valid_move (board, i, player)) {
      moves[nummoves++] = i;
    }
  }

  if (!nummoves) {
    /* This should never ever happen */
    return 64;
  }

  i = rand () % nummoves;

  return moves[i];
}

const gchar *
plugin_name ()
{
  return "Random Player";
}

const gchar *
plugin_busy_message (gint player)
{
  return (busy_message[player]);
}

void
plugin_about_window (GtkWindow *parent, gint player)
{
  GtkWidget *about;
  const gchar *authors[] = {
    "Ian Peters (itp@gnu.org)",
    NULL
  };

  about = gnome_about_new (_("Random Player for Iagno II"), "0.0.1",
                           "(C) 1999-2000 Ian Peters",
                           (const gchar **) authors,
                           _("This is a reference plugin for Iagno II "
                             "and has the most basic playing strategy "
                             "I could think of.  This plugin is "
                             "intended to demonstrate how to write "
                             "an Iagno II plugin."),
                           NULL);

  gnome_dialog_set_parent (GNOME_DIALOG (about), GTK_WINDOW (parent));

  gtk_widget_show (about);
}

void
plugin_preferences_window (GtkWidget *parent, gint player)
{
  GtkWidget *dialog;
  GtkWidget *entry;
  gchar *message;

  gint save_string (GtkWidget *widget, gpointer data)
  {
    GtkWidget *entry;

    entry = (GtkWidget *) data;

    if (new_message[player]) {
      g_free (new_message[player]);
    }

    new_message[player] = g_strdup (gtk_entry_get_text (GTK_ENTRY (entry)));

    gnome_property_box_changed (GNOME_PROPERTY_BOX (parent));
  }

  dialog = gnome_dialog_new (_("Random Player Configuration"),
                             GNOME_STOCK_BUTTON_OK,
                             GNOME_STOCK_BUTTON_CANCEL,
                             NULL);

  gnome_dialog_set_parent (GNOME_DIALOG (dialog), GTK_WINDOW (parent));

  entry = gtk_entry_new_with_max_length (49);

  if (new_message[player]) {
    gtk_entry_set_text (GTK_ENTRY (entry), new_message[player]);
  } else {
    gtk_entry_set_text (GTK_ENTRY (entry), busy_message[player]);
  }

  gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (dialog)->vbox),
                      entry,
                      TRUE, TRUE,
                      0);
  
  gtk_widget_show (entry);

  gnome_dialog_button_connect (GNOME_DIALOG (dialog),
                               0,
                               GTK_SIGNAL_FUNC (save_string),
                               (gpointer) entry);

  gnome_dialog_run_and_close (GNOME_DIALOG (dialog));
}

void
plugin_preferences_save (gint player)
{
  gchar *pref_file;
  
  if (new_message[player] == NULL) {
    return;
  }

  pref_file = g_strdup_printf ("/iagno2/Random/player%d_busy_message",
                               player);
  gnome_config_set_string (pref_file, new_message[player]);
  g_free (pref_file);

  g_snprintf (busy_message[player], 50, "%s", new_message[player]);

  g_free (new_message[player]);
  new_message[player] = NULL;

  gnome_config_sync ();
}
