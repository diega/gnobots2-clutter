/* -*- mode:C++; tab-width:8; c-basic-offset:8; indent-tabs-mode:true -*- */
/*
 * written by Callum McKenzie <callum@spooky-possum.org>
 *
 * Copyright (C) 2005 by Callum McKenzie
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

#include <string.h>

#include "renderer-noclutter.h"

const ThemeTableEntry ThemeTable[] = {{N_("Plain"), "plain"},
				      {N_("Joined"), "joined"},
				      {N_("Tango Flat"), "tangoflat"},
				      {N_("Tango Shaded"), "tangoshaded"},
				      {NULL, NULL}};


gint themeNameToNumber (const gchar *id)
{
	int i;
	const ThemeTableEntry *t;

	if (id == NULL)
		return 0;

	t = ThemeTable;
	i = 0;
	while (t->id) {
		if (strcmp (t->id, id) == 0)
			return i;
		t++;
		i++;
	}

	return 0;
}

Renderer * rendererFactory (gint id, cairo_surface_t *dst,
			    cairo_surface_t *bg, Block **src, int w,
			    int h, int pxw, int pxh)
{
	switch (id) {
	case 3:
		return new TangoBlock (dst, bg, src, w, h, pxw, pxh, TRUE);
	case 2:
		return new TangoBlock (dst, bg, src, w, h, pxw, pxh, FALSE);
	case 1:
		return new JoinedUp (dst, bg, src, w, h, pxw, pxh);
	case 0:
	default:
		return new Renderer (dst, bg, src, w, h, pxw, pxh);
	}
}

/* The Renderer class is a basic drawing class that is structured to
   be easily customised by subclasses. The most basic customisation
   would be to override drawCell to customise the drawing of one
   cell. If more sophisticated drawing is required for either the
   foreground or the background is required then drawForeground and
   drawBackground are the functions to alter. If a completely
   different drawing regime is required then the render method - the
   only entry point from external code - can be replaced. */

/* Note that the default renderer is designed to be reasonably fast
   and flexible, not flashy. Also note that the renderer may be used
   for the preview widget and possibly the theme previewer, so make no
   assumptions. */

Renderer::Renderer (cairo_surface_t *dst, cairo_surface_t *bg, Block **src,
		    int w, int h, int pxw, int pxh)
{
	target = cairo_surface_reference (dst);
	background = cairo_surface_reference (bg);
	data = src;
	width = w;
	height = h;
	pxwidth = pxw;
	pxheight = pxh;
}

Renderer::~Renderer ()
{
	cairo_surface_destroy (target);
	cairo_surface_destroy (background);
}

void Renderer::setTarget (cairo_surface_t * dst)
{
	cairo_surface_destroy (target);
	target = cairo_surface_reference (dst);
}

void Renderer::setBackground (cairo_surface_t *bg)
{
	cairo_surface_destroy (background);
	background = cairo_surface_reference (bg);
}

void Renderer::drawCell (cairo_t *cr, gint x, gint y)
{
	int i;
	const gdouble colours[7][3] = {{1.0, 0.0, 0.0},
				       {0.0, 1.0, 0.0},
				       {0.0, 0.0, 1.0},
				       {1.0, 1.0, 1.0},
				       {1.0, 1.0, 0.0},
				       {1.0, 0.0, 1.0},
				       {0.0, 1.0, 1.0}};

	if (data[x][y].what == EMPTY)
		return;

	if (data[x][y].what == TARGET) {
		cairo_set_source_rgba (cr, 1.0, 1.0, 1.0, 0.3);
	} else {
		i = data[x][y].color;
		i = CLAMP (i, 0, 6);

		cairo_set_source_rgb(cr, colours[i][0],
				     colours[i][1],
				     colours[i][2]);
	}

	cairo_rectangle(cr, x+0.05, y+0.05,
			0.9, 0.9);
	cairo_fill (cr);
}

void Renderer::drawBackground (cairo_t *cr)
{
	cairo_set_source_surface (cr, background, 0, 0);
	cairo_paint (cr);
}

void Renderer::drawForeground (cairo_t *cr)
{
	int x, y;

	cairo_scale(cr, 1.0 * pxwidth / width, 1.0 * pxheight / height);

	for (y = 0; y<height; y++) {
		for (x = 0; x<width; x++) {
			drawCell (cr, x, y);
		}
	}
}

void Renderer::render ()
{
	cairo_t *cr;

	cr = cairo_create (target);

	drawBackground (cr);
	drawForeground (cr);

	cairo_destroy (cr);
}

/*--------------------------------------------------------*/

void JoinedUp::drawInnerCorner (cairo_t *cr)
{
	border = 0.2;
	cairo_move_to (cr, 0, 0);
	cairo_line_to (cr, border, border);
	cairo_line_to (cr, border, 0);
	cairo_move_to (cr, border, border);
	cairo_line_to (cr, 0, border);
	cairo_stroke (cr);
}

void JoinedUp::drawOuterCorner (cairo_t *cr)
{
	border = 0.2;
	cairo_move_to (cr, 0, 0.5);
	cairo_line_to (cr, 0, 0);
	cairo_line_to (cr, 0.5, 0);
	cairo_move_to (cr, 0, 0);
	cairo_line_to (cr, border, border);
	cairo_line_to (cr, 0.5, border);
	cairo_move_to (cr, border, border);
	cairo_line_to (cr, border, 0.5);
	cairo_stroke (cr);
}

void JoinedUp::drawHEdge (cairo_t *cr)
{
	border = 0.2;
	cairo_move_to (cr, 0, 0);
	cairo_line_to (cr, 0.5, 0);
	cairo_move_to (cr, 0, border);
	cairo_line_to (cr, 0.5, border);
	cairo_stroke (cr);
}

void JoinedUp::drawVEdge (cairo_t *cr)
{
	border = 0.2;
	cairo_move_to (cr, 0, 0);
	cairo_line_to (cr, 0, 0.5);
	cairo_move_to (cr, border, 0);
	cairo_line_to (cr, border, 0.5);
	cairo_stroke (cr);
}

void JoinedUp::drawCell (cairo_t *cr, gint x, gint y)
{
	int i, m, n;
	int segments[4];
	double xofs;
	double yofs;
	int c;
	int neighbours[8];
	static const int formtable[4][8] = {{0, 7, 0, 7, 4, 9, 4, 8},
					    {1, 4, 1, 4, 5, 10, 5, 8},
					    {2, 6, 2, 6, 7, 11, 7, 8},
					    {3, 5, 3, 5, 6, 12, 6, 8}};
	static const gdouble colours[7][3] = {{1.0, 0.0, 0.0},
					      {0.1, 0.8, 0.1},
					      {0.1, 0.1, 0.8},
					      {1.0, 1.0, 1.0},
					      {1.0, 1.0, 0.0},
					      {0.8, 0.1, 0.8},
					      {0.0, 1.0, 1.0}};
	static const int neighbourmap[8][2] = {{-1, -1}, {0, -1}, {+1, -1},
					       {-1, 0}, {+1, 0},
					       {-1, +1}, {0, +1}, {+1, +1}};

	if (data[x][y].what == EMPTY)
		return;

	i = data[x][y].color;
	i = CLAMP (i, 0, 6);

	cairo_save (cr);
	cairo_translate (cr, x, y);

	cairo_set_line_join (cr, CAIRO_LINE_JOIN_ROUND);
	cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);
	cairo_set_line_width (cr, 0.05);

	if (data[x][y].what == TARGET) {
		cairo_set_source_rgba (cr, 1.0, 1.0, 1.0, 0);
	} else {
		cairo_set_source_rgb (cr, colours[i][0],
				      colours[i][1],
				      colours[i][2]);
	}
	cairo_rectangle (cr, -0.025, -0.025, 1.025, 1.025);
	cairo_fill (cr);
	cairo_set_source_rgb (cr, 0.5, 0.5, 0.5);

	// Enumerate the neighbours.
	c = data[x][y].color;
	for (i=0; i<8; i++) {
		m = x + neighbourmap[i][0];
		n = y + neighbourmap[i][1];
		if ((m < 0) || (n < 0) || (m >= width) || (n >= height))
			neighbours[i] = 0;
		else
			neighbours[i] = ((data[m][n].what != EMPTY) &&
					 (data[m][n].color == c)) ? 1 : 0;
	}

	// Sort out which quadrant of the square is drawn in what way.
	segments[0] = formtable [0][neighbours[3]*4 +
				    neighbours[0]*2 +
				    neighbours[1]];
	segments[1] = formtable [1][neighbours[1]*4 +
				    neighbours[2]*2 +
				    neighbours[4]];
	segments[2] = formtable [2][neighbours[6]*4 +
				    neighbours[5]*2 +
				    neighbours[3]];
	segments[3] = formtable [3][neighbours[4]*4 +
				    neighbours[7]*2 +
				    neighbours[6]];

	// Finally: do the actual drawing.
	for (i=0; i<4; i++) {
		cairo_save (cr);
		xofs = 0.5*(i % 2);
		yofs = 0.5*(i / 2);
		cairo_translate (cr, xofs, yofs);
		switch (segments[i]) {
		case 0:
			drawOuterCorner (cr);
			break;
		case 1:
			cairo_scale (cr, -1.0, 1.0);
			cairo_translate (cr, -0.5, 0);
			drawOuterCorner (cr);
			break;
		case 2:
			cairo_scale (cr, 1.0, -1.0);
			cairo_translate (cr, 0, -0.5);
			drawOuterCorner (cr);
			break;
		case 3:
			cairo_scale (cr, -1.0, -1.0);
			cairo_translate (cr, -0.5, -0.5);
			drawOuterCorner (cr);
			break;
		case 4:
			drawHEdge (cr);
			break;
		case 5:
			cairo_scale (cr, -1.0, 1.0);
			cairo_translate (cr, -0.5, 0);
			drawVEdge (cr);
			break;
		case 6:
			cairo_scale (cr, 1.0, -1.0);
			cairo_translate (cr, 0, -0.5);
			drawHEdge (cr);
			break;
		case 7:
			drawVEdge (cr);
			break;

		case 8:
			break;
		case 9:
			drawInnerCorner (cr);
			break;
		case 10:
			cairo_scale (cr, -1.0, 1.0);
			cairo_translate (cr, -0.5, 0);
			drawInnerCorner (cr);
			break;
		case 11:
			cairo_scale (cr, 1.0, -1.0);
			cairo_translate (cr, 0, -0.5);
			drawInnerCorner (cr);
			break;
		case 12:
			cairo_scale (cr, -1.0, -1.0);
			cairo_translate (cr, -0.5, -0.5);
			drawInnerCorner (cr);
			break;
		}
		cairo_restore (cr);
	}

	cairo_restore (cr);
}

/*--------------------------------------------------------*/

TangoBlock::TangoBlock (cairo_surface_t * dst, cairo_surface_t * bg, Block ** src,
	    int w, int h, int pxw, int pxh, gboolean grad) : Renderer (dst, bg, src, w, h, pxw, pxh)
{
	usegrads = grad;
}

void TangoBlock::drawCell (cairo_t *cr, gint x, gint y)
{

	int i;
	cairo_pattern_t *pat = NULL;
	/* the following garbage is derived from the official tango style guide */
	const gdouble colours[8][3][3] = {
					  {{0.93725490196078431, 0.16078431372549021, 0.16078431372549021},
					   {0.8, 0.0, 0.0},
					   {0.64313725490196083, 0.0, 0.0}}, /* red */

					  {{0.54117647058823526, 0.88627450980392153, 0.20392156862745098},
					   {0.45098039215686275, 0.82352941176470584, 0.086274509803921567},
					   {0.30588235294117649, 0.60392156862745094, 0.023529411764705882}}, /* green */

					  {{0.44705882352941179, 0.62352941176470589, 0.81176470588235294},
					   {0.20392156862745098, 0.396078431372549, 0.64313725490196083},
					   {0.12549019607843137, 0.29019607843137257, 0.52941176470588236}}, /* blue */

					  {{0.93333333333333335, 0.93333333333333335, 0.92549019607843142},
					   {0.82745098039215681, 0.84313725490196079, 0.81176470588235294},
					   {0.72941176470588232, 0.74117647058823533, 0.71372549019607845}}, /* white */

					  {{0.9882352941176471, 0.9137254901960784, 0.30980392156862746},
					   {0.92941176470588238, 0.83137254901960789, 0.0},
					   {0.7686274509803922, 0.62745098039215685, 0.0}}, /* yellow */

					  {{0.67843137254901964, 0.49803921568627452, 0.6588235294117647},
					   {0.45882352941176469, 0.31372549019607843, 0.4823529411764706},
					   {0.36078431372549019, 0.20784313725490197, 0.4}}, /* purple */

					  {{0.9882352941176471, 0.68627450980392157, 0.24313725490196078},
					   {0.96078431372549022, 0.47450980392156861, 0.0},
					   {0.80784313725490198, 0.36078431372549019, 0.0}}, /* orange (replacing cyan) */

					  {{0.33, 0.34, 0.32},
					   {0.18, 0.2, 0.21},
					   {0.10, 0.12, 0.13}} /* grey */
					 };

	if (data[x][y].what == EMPTY)
		return;

	if (data[x][y].what == TARGET) {
		i = 7;
	} else {
		i = data[x][y].color;
		i = CLAMP (i, 0, 6);
	}

	if (usegrads) {
		 pat = cairo_pattern_create_linear (x+0.35, y, x+0.55, y+0.9);
		 cairo_pattern_add_color_stop_rgb (pat, 0.0, colours[i][0][0],
						   colours[i][0][1],
						   colours[i][0][2]);
		 cairo_pattern_add_color_stop_rgb (pat, 1.0, colours[i][1][0],
						   colours[i][1][1],
						   colours[i][1][2]);
		 cairo_set_source (cr, pat);
	} else {
		 cairo_set_source_rgb (cr, colours[i][0][0],
				       colours[i][0][1],
				       colours[i][0][2]);
	}

	drawRoundedRectangle (cr, x+0.05, y+0.05, 0.9, 0.9, 0.2);
	cairo_fill_preserve (cr);  /* fill with shaded gradient */


	if (usegrads)
		cairo_pattern_destroy(pat);
	cairo_set_source_rgb(cr, colours[i][2][0],
			     colours[i][2][1],
			     colours[i][2][2]);

	cairo_set_line_width (cr, 0.1);
	cairo_stroke (cr);  /* add darker outline */

	drawRoundedRectangle (cr, x+0.15, y+0.15, 0.7, 0.7, 0.08);
	if (data[x][y].what != TARGET) {
		if (usegrads) {
			pat = cairo_pattern_create_linear (x-0.3, y-0.3, x+0.8, y+0.8);
			switch (i) { /* yellow and white blocks need a brighter highlight */
			case 3:
			case 4:
				cairo_pattern_add_color_stop_rgba (pat, 0.0, 1.0,
								   1.0,
								   1.0,
								   1.0);
				cairo_pattern_add_color_stop_rgba (pat, 1.0, 1.0,
								   1.0,
								   1.0,
								   0.0);
				break;
			default:
				cairo_pattern_add_color_stop_rgba (pat, 0.0, 0.9295,
								   0.9295,
								   0.9295,
								   1.0);
				cairo_pattern_add_color_stop_rgba (pat, 1.0, 0.9295,
								   0.9295,
								   0.9295,
								   0.0);
				break;
			}
			cairo_set_source (cr, pat);
		} else {
			cairo_set_source_rgba (cr, 1.0,
					       1.0,
					       1.0,
					       0.35);
		}
	} else {  /* black preview block, use a much weaker highlight */
		cairo_set_source_rgba (cr, 1.0,
				       1.0,
				       1.0,
				       0.15);
	}
	cairo_stroke (cr);  /* add inner edge highlight */

	if (usegrads && (data[x][y].what != TARGET))
		cairo_pattern_destroy (pat);
}

void TangoBlock::drawRoundedRectangle (cairo_t * cr, gdouble x, gdouble y, gdouble w, gdouble h, gdouble r)
{
	cairo_move_to(cr, x+r, y);
	cairo_line_to(cr, x+w-r, y);
	cairo_curve_to(cr, x+w-(r/2), y, x+w, y+(r/2), x+w, y+r);
	cairo_line_to(cr, x+w, y+h-r);
	cairo_curve_to(cr, x+w, y+h-(r/2), x+w-(r/2), y+h, x+w-r, y+h);
	cairo_line_to(cr, x+r, y+h);
	cairo_curve_to(cr, x+(r/2), y+h, x, y+h-(r/2), x, y+h-r);
	cairo_line_to(cr, x, y+r);
	cairo_curve_to(cr, x, y+(r/2), x+(r/2), y, x+r, y);
}

