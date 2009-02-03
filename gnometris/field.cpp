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
#include "field.h"
#include "blocks.h"
#include "renderer.h"

#include <clutter-cairo/clutter-cairo.h>
#include <libgames-support/games-clutter-embed.h>

#define FONT "Sans Bold"

Field::Field():
	BlockOps(),
	background(NULL),
	foreground(NULL),
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

	w = games_clutter_embed_new();

	g_signal_connect (w, "configure_event", G_CALLBACK (configure), this);

	/* I don't know if this helps or not FIXME */
	gtk_widget_set_double_buffered (w, FALSE);

	gtk_widget_set_size_request (w, COLUMNS*190/LINES, 190);

	gtk_widget_show (w);
}

Field::~Field()
{
	if (renderer)
		delete renderer;
}

void
Field::rescaleField ()
{
	ClutterActor *stage;
	cairo_t *bg_cr;

	if (background) {
		clutter_actor_set_size (CLUTTER_ACTOR(background),
					w->allocation.width,
					w->allocation.height);
		clutter_cairo_surface_resize (CLUTTER_CAIRO(background),
					      w->allocation.width,
					      w->allocation.height);
	} else {
		background = clutter_cairo_new (w->allocation.width,
						w->allocation.height);
		stage = games_clutter_embed_get_stage (GAMES_CLUTTER_EMBED (w));
		/*FIXMEjclinton: eventually allow solid color background
		 * for software rendering case */
		ClutterColor stage_color = { 0x61, 0x64, 0x8c, 0xff };
		clutter_stage_set_color (CLUTTER_STAGE (stage),
					 &stage_color);
		clutter_group_add (CLUTTER_GROUP (stage),
				   background);
		clutter_actor_set_position (CLUTTER_ACTOR(background),
					    0, 0);
	}

	if (foreground) {
		clutter_actor_set_size (CLUTTER_ACTOR(foreground),
					w->allocation.width,
					w->allocation.height);
		clutter_cairo_surface_resize (CLUTTER_CAIRO(foreground),
					      w->allocation.width,
					      w->allocation.height);
	} else {
		foreground = clutter_cairo_new (w->allocation.width,
						w->allocation.height);
		stage = games_clutter_embed_get_stage (GAMES_CLUTTER_EMBED (w));
		clutter_group_add (CLUTTER_GROUP (stage),
				   foreground);
		clutter_actor_set_position (CLUTTER_ACTOR(foreground),
					    0, 0);
		clutter_actor_raise (CLUTTER_ACTOR(foreground),
				     CLUTTER_ACTOR(background));
	}


	bg_cr = clutter_cairo_create (CLUTTER_CAIRO(background));
	cairo_set_operator (bg_cr, CAIRO_OPERATOR_CLEAR);
	cairo_paint(bg_cr);
	cairo_set_operator (bg_cr, CAIRO_OPERATOR_OVER);

	if (useBGImage && backgroundImage) {
		gdouble xscale, yscale;
		cairo_matrix_t m;

		/* FIXME: This doesn't handle tiled backgrounds in the obvious way. */
		gdk_cairo_set_source_pixbuf (bg_cr, backgroundImage, 0, 0);
		xscale = 1.0*gdk_pixbuf_get_width (backgroundImage)/w->allocation.width;
		yscale = 1.0*gdk_pixbuf_get_height (backgroundImage)/w->allocation.height;
		cairo_matrix_init_scale (&m, xscale, yscale);
		cairo_pattern_set_matrix (cairo_get_source (bg_cr), &m);
	} else if (backgroundColor)
		gdk_cairo_set_source_color (bg_cr, backgroundColor);
	else
		cairo_set_source_rgb (bg_cr, 0., 0., 0.);

	cairo_paint (bg_cr);
	cairo_destroy (bg_cr);
	this->drawMessage ();
	renderer->rescaleCache ();
}

gboolean
Field::configure(GtkWidget *widget, GdkEventConfigure *event, Field *field)
{
	field->width = widget->allocation.width;
	field->height = widget->allocation.height;

	field->rescaleField ();
	return TRUE;
}

void
Field::drawMessage()
{
	PangoLayout *dummy_layout;
	PangoLayout *layout;
	PangoFontDescription *desc;
	int lw, lh;
	cairo_t *cr;
	char *msg;

	cr = clutter_cairo_create (CLUTTER_CAIRO(foreground));
	cairo_set_operator (cr, CAIRO_OPERATOR_CLEAR);
	cairo_paint(cr);
	cairo_set_operator (cr, CAIRO_OPERATOR_OVER);

	if (showPause)
		msg =  _("Paused");
	else if (showGameOver)
		msg = _("Game Over");
	else {
		cairo_destroy (cr);
		return;
	}

	// Center coordinates
	cairo_translate (cr, width / 2, height / 2);

	desc = pango_font_description_from_string(FONT);

	layout = pango_cairo_create_layout (cr);
	pango_layout_set_text (layout, msg, -1);

	dummy_layout = pango_layout_copy (layout);
	pango_layout_set_font_description (dummy_layout, desc);
	pango_layout_get_size (dummy_layout, &lw, &lh);
	g_object_unref (dummy_layout);

	// desired height : lh = widget width * 0.9 : lw
	pango_font_description_set_absolute_size (desc, ((float) lh / lw) * PANGO_SCALE * width * 0.8);
	pango_layout_set_font_description (layout, desc);
	pango_font_description_free (desc);

	pango_layout_get_size (layout, &lw, &lh);
	cairo_move_to (cr, -((double)lw / PANGO_SCALE) / 2, -((double)lh / PANGO_SCALE) / 2);
	pango_cairo_layout_path (cr, layout);
	cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
	cairo_fill_preserve (cr);
	cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
	/* A linewidth of 2 pixels at the default size. */
	cairo_set_line_width (cr, width/220.0);
	cairo_stroke (cr);

	g_object_unref(layout);
	cairo_destroy (cr);
}

void
Field::setBackground(GdkPixbuf *bgImage)//, bool tiled)
{
	backgroundImage = (GdkPixbuf *) g_object_ref(bgImage);
	useBGImage = true;
//	backgroundImageTiled = tiled;

	rescaleField ();
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

	rescaleField ();
}

void
Field::showPauseMessage()
{
	showPause = true;

	drawMessage ();
}

void
Field::hidePauseMessage()
{
	showPause = false;

	drawMessage ();
}

void
Field::showGameOverMessage()
{
	showGameOver = true;

	drawMessage ();
}

void
Field::hideGameOverMessage()
{
	showGameOver = false;

	drawMessage ();
}

void
Field::setTheme (gint id)
{
	themeID = id;
}
