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

const ThemeTableEntry ThemeTable[] = {{N_("Plain"), "plain"},
                                      {N_("Joined"), "joined"},
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

void JoinedUp::drawInnerCorner (cairo_t *cr)
{
        cairo_move_to (cr, 0, 0);
        cairo_line_to (cr, border, border);
        cairo_line_to (cr, border, 0);
        cairo_move_to (cr, border, border);
        cairo_line_to (cr, 0, border);
        cairo_stroke (cr);
}

void JoinedUp::drawOuterCorner (cairo_t *cr)
{
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
        cairo_move_to (cr, 0, 0);
        cairo_line_to (cr, 0.5, 0);
        cairo_move_to (cr, 0, border);
        cairo_line_to (cr, 0.5, border);
        cairo_stroke (cr);
}

void JoinedUp::drawVEdge (cairo_t *cr)
{
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

	cairo_set_source_rgb (cr, colours[i][0], 
                              colours[i][1], 
                              colours[i][2]);
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
