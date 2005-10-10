/* -*- mode:C; indent-tabs-mode:nil; tab-width:8; c-basic-offset:8 -*- */
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

/* This code is meant to be a basis for hot-swappable and themeable
   drawing engines. */

#include "renderer.h"

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
  
	i = data[x][y].color;                       
	i = CLAMP (i, 0, 6);
	
	cairo_set_source_rgb(cr, colours[i][0], 
			     colours[i][1], 
			     colours[i][2]);
	cairo_rectangle(cr, x+0.05, y+0.05, 
			0.9, 0.9);
	cairo_fill (cr);
}

void Renderer::render ()
{
       cairo_t *cr;
       int x, y;

       cr = cairo_create (target);

       cairo_set_source_surface (cr, background, 0, 0);
       cairo_paint (cr);

       cairo_scale(cr, 1.0 * pxwidth / width, 1.0 * pxheight / height);
       
       cairo_set_line_width (cr, 0.2);
       cairo_set_line_join (cr, CAIRO_LINE_JOIN_ROUND);

       for (y = 0; y<height; y++) {
               for (x = 0; x<width; x++) {
		 drawCell (cr, x, y);
               }
       }

       cairo_destroy (cr);
}
