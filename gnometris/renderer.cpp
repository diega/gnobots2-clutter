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


#include "renderer.h"

ThemeTableEntry ThemeTable[] = {{N_("Plain"), "plain"},
                                 {N_("Joined"), "joined"},
                                 {NULL, NULL}};


gint themeNameToNumber (const gchar *id)
{
        int i;
        ThemeTableEntry *t;

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
        case 1:
                return new JoinedUp (dst, bg, src, w, h, pxw, pxh);
        case 0:
        default:
                return  new Renderer (dst, bg, src, w, h, pxw, pxh);
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

void JoinedUp::drawCell (cairo_t *cr, gint x, gint y)
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
	
        cairo_set_line_join (cr, CAIRO_LINE_JOIN_ROUND);
        cairo_set_line_width (cr, 0);

	cairo_set_source_rgb(cr, colours[i][0], 
			     colours[i][1], 
			     colours[i][2]);
        cairo_rectangle(cr, x+0.05, y+0.05, 
                        0.9, 0.9);
        if ((x > 0) && (data[x][y].color == data[x-1][y].color) &&
            (data[x-1][y].what != EMPTY)) {
                cairo_rectangle (cr, x, y+0.05, 0.05, 0.9);
        }
        if ((x < width-1) && (data[x][y].color == data[x+1][y].color) &&
            (data[x+1][y].what != EMPTY)) {
                cairo_rectangle (cr, x+0.95, y+0.05, 0.07, 0.9);
        }
        if ((y > 0) && (data[x][y].color == data[x][y-1].color) &&
            (data[x][y-1].what != EMPTY)) {
                cairo_rectangle (cr, x+0.05, y, 0.9, 0.05);
        }
        if ((y < height-1) && (data[x][y].color == data[x][y+1].color) &&
            (data[x][y+1].what != EMPTY)) {
                cairo_rectangle (cr, x+0.05, y+0.95, 0.9, 0.07);
        }

	cairo_fill (cr);        
}
