/*
 * written by J. Marcin Gorycki <mgo@olicom.dk>
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
#include "scoreframe.h"

ScoreFrame::ScoreFrame()
{
	w = gtk_frame_new("");

	scoreLabel = gtk_label_new(_("Score: "));
	sprintf(b, "%.5d", 0);
	scorew = gtk_label_new(b);

	linesLabel = gtk_label_new(_("Lines: "));
	sprintf(b, "%.5d", 0);
	linesw = gtk_label_new(b);

	levelLabel = gtk_label_new(_("Level: "));
	sprintf(b, "%.5d", 0);
	levelw = gtk_label_new(b);

	vb = gtk_vbox_new(FALSE, 0);
	hbScore = gtk_hbox_new(FALSE, 0);
	hbLines = gtk_hbox_new(FALSE, 0);
	hbLevel = gtk_hbox_new(FALSE, 0);

	gtk_container_add(GTK_CONTAINER(w), vb);
  gtk_container_border_width(GTK_CONTAINER(vb), 10);

	gtk_box_pack_start(GTK_BOX(vb), hbScore, 0, 0, 0);
	gtk_box_pack_start(GTK_BOX(vb), hbLines, 0, 0, 0);
	gtk_box_pack_start(GTK_BOX(vb), hbLevel, 0, 0, 0);

	gtk_box_pack_start(GTK_BOX(hbScore), scoreLabel,  0, 0, 0);
	gtk_box_pack_end(GTK_BOX(hbScore), scorew, 0, 0, 0);

	gtk_box_pack_start(GTK_BOX(hbLines), linesLabel,  0, 0, 0);
	gtk_box_pack_end(GTK_BOX(hbLines), linesw, 0, 0, 0);

	gtk_box_pack_start(GTK_BOX(hbLevel), levelLabel,  0, 0, 0);
	gtk_box_pack_end(GTK_BOX(hbLevel), levelw, 0, 0, 0);
}

void
ScoreFrame::show()
{
	gtk_widget_show(scoreLabel);
	gtk_widget_show(linesLabel);
	gtk_widget_show(levelLabel);
	gtk_widget_show(scorew);
	gtk_widget_show(linesw);
	gtk_widget_show(levelw);
	gtk_widget_show(hbScore);
	gtk_widget_show(hbLines);
	gtk_widget_show(hbLevel);
	gtk_widget_show(vb);
	gtk_widget_show(w);
}

void 
ScoreFrame::setScore(int score)
{
	sprintf(b, "%.5d", score);
	gtk_label_set(GTK_LABEL(scorew), b);
}

void 
ScoreFrame::setLines(int lines)
{
	sprintf(b, "%.5d", lines);
	gtk_label_set(GTK_LABEL(linesw), b);
}

void 
ScoreFrame::setLevel(int level)
{
	sprintf(b, "%.5d", level);
	gtk_label_set(GTK_LABEL(levelw), b);
}



