/* -*- mode:C++; tab-width:8; c-basic-offset:8; indent-tabs-mode:true -*- */

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

#include <config.h>
#include "preview-noclutter.h"
#include "blocks.h"

#define PREVIEW_WIDTH 6
#define PREVIEW_HEIGHT 6

// FIXME: Remove
#define PREVIEW_SIZE 5

Preview::Preview():
	blocknr(-1),
	blockrot(0),
	blockcolor(0),
	enabled(true),
	themeID (0),
	background (0)
{
	blocks = new Block*[PREVIEW_WIDTH];
	for (int i = 0; i < PREVIEW_WIDTH; i++) {
		blocks[i] = new Block [PREVIEW_HEIGHT];
		for (int j = 0; j < PREVIEW_HEIGHT; j++) {
			blocks[i][j].what = EMPTY;
			blocks[i][j].color = 0;
		}
	}

	w = gtk_drawing_area_new();

	g_signal_connect (w, "expose_event", G_CALLBACK (expose), this);
	g_signal_connect (w, "configure_event", G_CALLBACK (configure), this);

	/* FIXME: We should scale with the rest of the UI, but that requires
	 * changes to the widget layout - i.e. wrap the preview in an
	 * fixed-aspect box. */
	gtk_widget_set_size_request (w, PREVIEW_SIZE * 20,
				     PREVIEW_SIZE * 20);

	gtk_widget_show (w);
}

Preview::~Preview ()
{
	for (int i = 0; i < PREVIEW_WIDTH; i++)
		delete[] blocks[i];

	delete[] blocks;
}

void
Preview::enable(bool en)
{
	enabled = en;
	gtk_widget_queue_draw (w);
}

void
Preview::setTheme (int id)
{
	themeID = id;
}

void
Preview::previewBlock(int bnr, int brot, int bcolor)
{
	int x, y;

	blocknr = bnr;
	blockrot = brot;
	blockcolor = bcolor;

	for (x = 1; x < PREVIEW_WIDTH - 1; x++) {
		for (y = 1; y < PREVIEW_HEIGHT - 1; y++) {
			if ((blocknr != -1) &&
			    blockTable[blocknr][blockrot][x-1][y-1]) {
				blocks[x][y].what = LAYING;
				blocks[x][y].color = blockcolor;
			} else {
				blocks[x][y].what = EMPTY;
			}
		}
	}

}

gint
Preview::configure(GtkWidget * widget, GdkEventConfigure * event, Preview * preview)
{
	cairo_t *cr;

	preview->width = event->width;
	preview->height = event->height;

	cr = gdk_cairo_create (widget->window);

	if (preview->background)
		cairo_surface_destroy (preview->background);

	preview->background =
		cairo_surface_create_similar (cairo_get_target (cr),
					      CAIRO_CONTENT_COLOR,
					      event->width,
					      event->height);

	cairo_destroy (cr);

	cr = cairo_create (preview->background);
	cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
	cairo_paint (cr);
	cairo_destroy (cr);

	return TRUE;
}

gint
Preview::expose(GtkWidget * widget, GdkEventExpose * event, Preview * preview)
{
	cairo_t *cr;
	Renderer *r;

	cr = gdk_cairo_create (widget->window);

	if (!preview->enabled)
	{
		cairo_scale (cr, 1.0*widget->allocation.width/PREVIEW_WIDTH,
			     1.0*widget->allocation.height/PREVIEW_HEIGHT);
		cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);

		cairo_set_source_rgb (cr, 0, 0, 0);
		cairo_paint (cr);

		cairo_set_source_rgb (cr, 0.8, 0.1, 0.1);
		cairo_set_line_width (cr, 0.5);

		cairo_move_to (cr, 1, 1);
		cairo_line_to (cr, PREVIEW_WIDTH-1, PREVIEW_HEIGHT-1);
		cairo_move_to (cr, PREVIEW_WIDTH-1, 1);
		cairo_line_to (cr, 1, PREVIEW_HEIGHT-1);

		cairo_stroke (cr);
	}
	else
	{
		cairo_surface_t *dst;

		dst = cairo_get_target (cr);

		r = rendererFactory (preview->themeID, dst,
				     preview->background, preview->blocks,
				     PREVIEW_WIDTH, PREVIEW_HEIGHT,
				     preview->width, preview->height);

		r->render ();
		delete r;
	}

	cairo_destroy (cr);

	return TRUE;
}
