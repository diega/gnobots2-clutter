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

#include "preview.h"
#include "blocks.h"

#define PREVIEW_WIDTH 6
#define PREVIEW_HEIGHT 6

Preview::Preview():
	blocknr(-1),
	blockrot(0),
	blockcolor(0),
	enabled(true)
{
	blocks = new Block*[PREVIEW_WIDTH];
	for (int i = 0; i < PREVIEW_WIDTH; i++) {
		blocks[i] = new Block [PREVIEW_HEIGHT];
		for (int j = 0; j < PREVIEW_HEIGHT; j++) {
			blocks[i][j].what = EMPTY;
			blocks[i][j].color = 0;
		}

	w = gtk_drawing_area_new();

	g_signal_connect (w, "expose_event", G_CALLBACK (expose), this);
	g_signal_connect (w, "configure_event", G_CALLBACK (configure), this);

	gtk_widget_show (w);
}

Preview::~Preview ()
{
	for (int i = 0; i < PREVIEW_WIDTH; i++)
		delete[] blocks[i];

	delete[] blocks;
}

void
Preview::updateSize()
{
	gtk_widget_set_size_request (w, PREVIEW_SIZE * BLOCK_SIZE, 
				     PREVIEW_SIZE * BLOCK_SIZE);
}

void
Preview::enable(bool en)
{
	enabled = en;
	gtk_widget_queue_draw (w);
}

void
Preview::previewBlock(int bnr, int brot, int bcolor)
{
	blocknr = bnr;
	blockrot = brot;
	blockcolor = bcolor;
}

gint
Preview::configure(GtkWidget * widget, GdkEventConfigure * event, Preview * preview)
{
	preview->width = event->width;
	preview->height = event->height;

	return TRUE;
}

gint
Preview::expose(GtkWidget * widget, GdkEventExpose * event, Preview * preview)
{
	GdkRectangle *area = &(event->area);
	cairo_t *cr;

	cr = gdk_cairo_create (widget->window);

	cairo_rectangle (cr, 0 , 0, widget->allocation.width, widget->allocation.height);
	cairo_set_source_rgb (cr, 0, 0, 0);
	cairo_fill_preserve (cr);

	if (!preview->enabled)
	{
		cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
		cairo_set_line_width (cr, 1.5);

		cairo_move_to (cr, 0, 0);
		cairo_line_to (cr, widget->allocation.width, widget->allocation.height);
		cairo_move_to (cr, widget->allocation.width, 0);
		cairo_line_to (cr, 0, widget->allocation.height);

		cairo_stroke (cr);
	}
	else if (preview->blocknr != -1)
	{
		int xoffs = (preview->width 
			     - sizeTable[preview->blocknr][preview->blockrot][1] * BLOCK_SIZE) / 2;
		int yoffs = (preview->height 
			     - sizeTable[preview->blocknr][preview->blockrot][0] * BLOCK_SIZE) / 2;

		xoffs -= offsetTable[preview->blocknr][preview->blockrot][1] * BLOCK_SIZE;
		yoffs -= offsetTable[preview->blocknr][preview->blockrot][0] * BLOCK_SIZE;

		for (int x = 0; x < 4; ++x) {
			for (int y = 0; y < 4; ++y) {
				if (blockTable[preview->blocknr][preview->blockrot][x][y]) {
					gdk_draw_pixbuf (widget->window, widget->style->black_gc, pic[preview->blockcolor],
							 0, 0, 
							 x * BLOCK_SIZE + xoffs, y * BLOCK_SIZE + yoffs,
							 BLOCK_SIZE, BLOCK_SIZE,
							 GDK_RGB_DITHER_NORMAL, 0, 0);
				}
			}
		}
	}

	cairo_destroy (cr);

	return TRUE;
}
