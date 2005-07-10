/* highscores.cpp - wrap the high score dialog.
 *
 * Copyright 2005 (c) Callum McKenzie
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

#include <gnome.h>
#include <games-scores-dialog.h>
#include "highscores.h"

HighScores::HighScores ()
{
  dialog = NULL;
}

void HighScores::show (gint highlight)
{
  if (!dialog)
    dialog = games_scores_dialog_new ("gnometris", _("Gnometris Scores"));

  games_scores_dialog_set_hilight (GAMES_SCORES_DIALOG (dialog), highlight);
  gtk_dialog_run (GTK_DIALOG (dialog));    
  gtk_widget_hide (dialog);
}
