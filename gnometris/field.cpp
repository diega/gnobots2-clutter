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

#include "field.h"
#include "blocks.h"
#include "gnome-canvas-pimage.h"

Field::	Field()
{
	bg = NULL;
	pausemsg = NULL;
	gameovermsg = NULL;

	w = gnome_canvas_new();
}

void
Field::updateSize(GdkPixbuf * bgImage, GdkColor *bgcolour)
{
	gtk_widget_set_size_request(w, COLUMNS * BLOCK_SIZE, LINES * BLOCK_SIZE);
	gnome_canvas_set_scroll_region(GNOME_CANVAS(w), 0.0, 0.0, COLUMNS * BLOCK_SIZE, LINES * BLOCK_SIZE);

	if (bg)
		gtk_object_destroy(GTK_OBJECT(bg));
	
  	if (bgImage)
  		bg = gnome_canvas_item_new(
  			gnome_canvas_root(GNOME_CANVAS(w)),
  			gnome_canvas_pimage_get_type(),
  			"image", bgImage,
  			"x", (double) 0,
  			"y", (double) 0,
			"width", (double) COLUMNS * BLOCK_SIZE,
			"height", (double) LINES * BLOCK_SIZE,
  			NULL);
		else
			bg = gnome_canvas_item_new(
  			gnome_canvas_root(GNOME_CANVAS(w)),
				gnome_canvas_rect_get_type(),
  			"x1", (double) 0,
  			"y1", (double) 0,
  			"x2", (double) COLUMNS * BLOCK_SIZE,
  			"y2", (double) LINES * BLOCK_SIZE,
				"fill_color_gdk", bgcolour,
				"outline_color", "black",
				"width_units", 1.0,
  			NULL);
}

void
Field::showPauseMessage()
{
	if (!pausemsg)
	{
		double x1, y1, x2, y2;
		double width;
		double pts;

		pausemsg = gnome_canvas_item_new (
			gnome_canvas_root(GNOME_CANVAS(w)),
			gnome_canvas_text_get_type(),
			"fill_color",
			"white",
			"x", COLUMNS * BLOCK_SIZE / 2.0,
			"y", LINES * BLOCK_SIZE / 2.0,
			"text", _("Paused"),
			"size_points", 36.0,
			NULL);

		/* Since gnome_canvas doesn't support setting the size of text in
		 * pixels (read the source where the "size" parameter gets set)
		 * and pango isn't forthcoming about how it scales things (see
		 * http://mail.gnome.org/archives/gtk-i18n-list/2003-August/msg00001.html
		 * and bug #119081). We guess at the size, see what size it is rendered
		 * to and then adjust the point size to fit. 36.0 points is pretty
		 * close for 96 dpi . */

		gnome_canvas_item_get_bounds (pausemsg, &x1, &y1, &x2, &y2);
		width = x2 - x1;
		/* 0.8 is the fraction of the screen we want to use and 36.0 is
		 * the guess we use previously for the point size. */
		pts = 0.8 * 36.0 * COLUMNS * BLOCK_SIZE / width;
		gnome_canvas_item_set (pausemsg, "size_points", pts, 0);
	}

	gnome_canvas_item_show (pausemsg);
	gnome_canvas_item_raise_to_top (pausemsg);
}

void
Field::hidePauseMessage()
{
	if (pausemsg)
		gnome_canvas_item_hide (pausemsg);
}

void
Field::showGameOverMessage()
{
	if (!gameovermsg)
	{
		double x1, y1, x2, y2;
		double width;
		double pts;

		gameovermsg = gnome_canvas_item_new (
			gnome_canvas_root(GNOME_CANVAS(w)),
			gnome_canvas_text_get_type(),
			"fill_color",
			"white",
			"x", COLUMNS * BLOCK_SIZE / 2.0,
			"y", LINES * BLOCK_SIZE / 2.0,
			"text", _("Game Over"),
			"size_points", 36.0,
			NULL);

		gnome_canvas_item_get_bounds (gameovermsg, &x1, &y1, &x2, &y2);
		width = x2 - x1;
		/* 0.9 is the fraction of the screen we want to use and 36.0 is
		 * the guess we use previously for the point size. */
		pts = 0.9 * 36.0 * COLUMNS * BLOCK_SIZE / width;
		gnome_canvas_item_set (gameovermsg, "size_points", pts, 0);
	}

	gnome_canvas_item_show (gameovermsg);
	gnome_canvas_item_raise_to_top (gameovermsg);
}

void
Field::hideGameOverMessage()
{
	if (gameovermsg)
		gnome_canvas_item_hide (gameovermsg);
}
