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
#include "field-noclutter.h"
#include "blocks.h"
#include "renderer-noclutter.h"

#define FONT "Sans Bold"

Field::Field():
	BlockOps(),
	buffer(NULL),
	background(NULL),
	showPause(false),
	showGameOver(false),
	backgroundImage(NULL),
	backgroundImageTiled(false),
	useBGImage(false),
	backgroundColor(NULL)
{
	themeID = 0;
	renderer = NULL;
	rendererTheme = -1;

	w = gtk_drawing_area_new();

	g_signal_connect (w, "expose_event", G_CALLBACK (expose), this);
	g_signal_connect (w, "configure_event", G_CALLBACK (configure), this);
	/* We do our own double-buffering. */
	gtk_widget_set_double_buffered(w, FALSE);

	gtk_widget_set_size_request (w, COLUMNS*190/LINES, 190);

	gtk_widget_show (w);
}

Field::~Field()
{
	if (buffer)
		cairo_surface_destroy(buffer);
	if (background)
		cairo_surface_destroy(background);

	if (renderer)
		delete renderer;
}

void
Field::rescaleBackground ()
{
	cairo_t *bg_cr;
	cairo_t *tmp_cr;

	if (!buffer)
		return;

	tmp_cr = cairo_create (buffer);

	if (background)
		cairo_surface_destroy(background);

	background =  cairo_surface_create_similar (cairo_get_target (tmp_cr),
						    CAIRO_CONTENT_COLOR,
						    w->allocation.width,
						    w->allocation.height);

	cairo_destroy (tmp_cr);

	bg_cr = cairo_create (background);

	if (useBGImage && backgroundImage) {
		gdouble xscale, yscale;
		cairo_matrix_t m;

		/* FIXME: This doesn't handle tiled backgrounds in the obvious way. */
		gdk_cairo_set_source_pixbuf(bg_cr, backgroundImage, 0, 0);
		xscale = 1.0*gdk_pixbuf_get_width (backgroundImage)/width;
		yscale = 1.0*gdk_pixbuf_get_height (backgroundImage)/height;
		cairo_matrix_init_scale (&m, xscale, yscale);
		cairo_pattern_set_matrix (cairo_get_source (bg_cr), &m);
	} else if (backgroundColor)
		gdk_cairo_set_source_color(bg_cr, backgroundColor);
	else
		cairo_set_source_rgb(bg_cr, 0., 0., 0.);

	cairo_paint(bg_cr);

	cairo_destroy(bg_cr);

	redraw ();
}

gboolean
Field::configure(GtkWidget *widget, GdkEventConfigure *event, Field *field)
{
	cairo_t *cr;

	field->width = widget->allocation.width;
	field->height = widget->allocation.height;

	cr = gdk_cairo_create (widget->window);

	if (field->buffer)
		cairo_surface_destroy(field->buffer);

	// backing buffer
	field->buffer =  cairo_surface_create_similar (cairo_get_target (cr),
						       CAIRO_CONTENT_COLOR,
						       widget->allocation.width,
						       widget->allocation.height);

	cairo_destroy (cr);


	field->rescaleBackground ();

	return TRUE;
}

void
Field::draw (gint x, gint y, gint wd, gint ht)
{
	cairo_t *cr;

	cr = gdk_cairo_create (w->window);

	cairo_set_source_surface (cr, buffer, 0, 0);
	cairo_rectangle (cr, x, y, wd, ht);
	cairo_fill (cr);

	cairo_destroy (cr);
}

void
Field::draw (void)
{
	draw (0, 0, width, height);
}

gboolean
Field::expose(GtkWidget *widget, GdkEventExpose *event, Field *field)
{
	field->draw (event->area.x, event->area.y,
		     event->area.width, event->area.height);

	return TRUE;
}

void
Field::drawMessage(cairo_t *cr, const char *msg)
{
	PangoLayout *dummy_layout;
	PangoLayout *layout;
	PangoFontDescription *desc;
	int lw, lh;

	cairo_save(cr);

	// Center coordinates
	cairo_translate(cr, width / 2, height / 2);

	desc = pango_font_description_from_string(FONT);

	layout = pango_cairo_create_layout(cr);
	pango_layout_set_text(layout, msg, -1);

	dummy_layout = pango_layout_copy(layout);
	pango_layout_set_font_description(dummy_layout, desc);
	pango_layout_get_size(dummy_layout, &lw, &lh);
	g_object_unref(dummy_layout);

	// desired height : lh = widget width * 0.9 : lw
	pango_font_description_set_absolute_size(desc, ((float) lh / lw) * PANGO_SCALE * width * 0.8);
	pango_layout_set_font_description(layout, desc);
	pango_font_description_free(desc);

	pango_layout_get_size(layout, &lw, &lh);
	cairo_move_to(cr, -((double)lw / PANGO_SCALE) / 2, -((double)lh / PANGO_SCALE) / 2);
	pango_cairo_layout_path(cr, layout);
	cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
	cairo_fill_preserve (cr);
	cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
	/* A linewidth of 2 pixels at the default size. */
	cairo_set_line_width (cr, width/220.0);
	cairo_stroke (cr);

	g_object_unref(layout);

	cairo_restore(cr);
}

void
Field::redraw()
{
	cairo_t *cr;

	g_return_if_fail(buffer);

	generateTarget ();

	if (rendererTheme != themeID) {

		if (renderer)
			delete renderer;

		renderer = rendererFactory (themeID, buffer, background, field,
					    COLUMNS, LINES, width, height);
		rendererTheme = themeID;
	} else {
		    renderer->setTarget (buffer);
		    renderer->setBackground (background);
		    renderer->data = field;
		    renderer->width = COLUMNS;
		    renderer->height = LINES;
		    renderer->pxwidth = width;
		    renderer->pxheight = height;
	}

	renderer->render ();

	cr = cairo_create(buffer);

	if (showPause)
		drawMessage(cr, _("Paused"));
	else if (showGameOver)
		drawMessage(cr, _("Game Over"));

	cairo_destroy(cr);

	draw ();
}

void
Field::setBackground(GdkPixbuf *bgImage)//, bool tiled)
{
	backgroundImage = (GdkPixbuf *) g_object_ref(bgImage);
	useBGImage = true;
	// backgroundImageTiled = tiled;

	rescaleBackground ();
}

void
Field::setBackground(GdkColor *bgColor)
{
	backgroundColor = gdk_color_copy(bgColor);
	if (backgroundImage) {
		g_object_unref (backgroundImage);
		backgroundImage = NULL;
	}
	useBGImage = false;

	rescaleBackground ();
}

void
Field::showPauseMessage()
{
	showPause = true;

	redraw();
}

void
Field::hidePauseMessage()
{
	showPause = false;

	redraw();
}

void
Field::showGameOverMessage()
{
	showGameOver = true;

	redraw();
}

void
Field::hideGameOverMessage()
{
	showGameOver = false;

	redraw();
}

void
Field::setTheme (gint id)
{
	themeID = id;
}
