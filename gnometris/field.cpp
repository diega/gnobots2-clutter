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

#include "field.h"
#include "blocks.h"

Field::	Field(BlockOps *ops)
{
	o = ops;
	
	gtk_widget_push_visual(gdk_imlib_get_visual());
	gtk_widget_push_colormap(gdk_imlib_get_colormap());

	w = gtk_drawing_area_new();

	gtk_widget_pop_colormap();
	gtk_widget_pop_visual();

	gtk_widget_set_events(w, gtk_widget_get_events(w) | GDK_EXPOSURE_MASK);
	gtk_signal_connect(GTK_OBJECT(w), "event", (GtkSignalFunc)eventHandler, this);
}

void 
Field::show()
{
	gtk_widget_realize(w);

	GdkColor c;
	c.pixel = 0;
	gdk_window_set_background(w->window, &c);

	gtk_widget_show(w);
	gtk_drawing_area_size(GTK_DRAWING_AREA(w), COLUMNS * BLOCK_SIZE, LINES * BLOCK_SIZE);
}

gint 
Field::eventHandler(GtkWidget *widget, GdkEvent *event, void *d)
{
	switch (event->type)
	{
	case GDK_EXPOSE: 
	{
		GdkEventExpose *e = (GdkEventExpose*)event;
		((Field*)d)->paint(&e->area);
		return TRUE;
	}
	default:
		return FALSE;
	}
}

void
Field::drawBlock(GdkRectangle *area, int x, int y)
{
	int xdest = x * BLOCK_SIZE;
	int ydest = y * BLOCK_SIZE;

	if (o->getFieldAt(x, y)->what != EMPTY)	
		gdk_draw_pixmap(w->window, w->style->black_gc, 
										pix, o->getFieldAt(x, y)->color * BLOCK_SIZE, 0, xdest, ydest, BLOCK_SIZE, BLOCK_SIZE);
	else 
		gdk_window_clear_area(w->window, xdest, ydest, BLOCK_SIZE, BLOCK_SIZE);
}

void
Field::paint(GdkRectangle *area)
{
	for (int x = 0; x < COLUMNS; ++x)
	{
		for (int y = 0; y < LINES; ++y)
		{
			drawBlock(area, x, y);
		}
	}
}



