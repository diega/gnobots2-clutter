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

gchar busy_new[50];

GtkWidget *plugin_pref_dialog = NULL;

void
plugin_init ()
{
  struct timeval tv;

  gettimeofday (&tv, NULL);
  srand (tv.tv_usec);
}

gint
plugin_move (ReversiBoard *board, gint player, gint move)
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
plugin_busy_message ()
{
  gchar *busy;
//  gchar busy_new[50];

  busy = gnome_config_get_string ("/iagno2/Random/busy_message=Random player is stalling for time...");
  g_snprintf (busy_new, 50, "%s", busy);
  g_free (busy);
  /*
  return "Random player is stalling for time...";
  */
  return busy_new;
}

void
plugin_about_window (GtkWindow *parent)
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
plugin_preferences (GtkWidget *parent)
{
  GtkWidget *dialog;
  GtkWidget *entry;

  gchar *message;

  gint save_string (GtkWidget *widget, gpointer data)
  {
    gchar *new_message;
    GtkWidget *entry;

    entry = (GtkWidget *) data;

    new_message = gtk_entry_get_text (GTK_ENTRY (entry));

    printf ("%s\n", new_message);

    gnome_config_set_string ("/iagno2/Random/busy_message", new_message);

    gnome_config_sync ();
  }

  dialog = gnome_dialog_new (_("Random Player Configuration"),
                             GNOME_STOCK_BUTTON_OK,
                             GNOME_STOCK_BUTTON_CANCEL,
                             NULL);

  gnome_dialog_set_parent (GNOME_DIALOG (dialog), GTK_WINDOW (parent));

  message = gnome_config_get_string ("/iagno2/Random/busy_message=Random player is stalling for time...");

  entry = gtk_entry_new_with_max_length (49);
  gtk_entry_set_text (GTK_ENTRY (entry), message);

  g_free (message);

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
