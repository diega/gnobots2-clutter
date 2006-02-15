/* gtk-gridboard-themes.c : Themes for gtk-gridboard.
 *
 * Copyright (C) 2005 by Callum McKenzie
 *
 * Time-stamp: <2006-02-15 18:51:57 callum>
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
 *
 */

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <cairo.h>

#include "gtkgridboardthemes.h"

#include <math.h>

/* This file consists of various static routines for drawing sections of the
 * board. They are assembled into the gtk_gridboard_themes structure at the
 * end of this file to create the themes. gtk_gridboard_themes (and the
 * associated function typedefs) should be the only public interface 
 * that this file presents. 
 */

/* ---- The default theme. Several of these are suitable for resuse. --- */

#define COIN_RADIUS 0.4

static void default_draw_bg (cairo_t *cx, gdouble x, gdouble y)
{
  cairo_set_source_rgba (cx, 0.43, 0.55, 0.65, 1.0);
  cairo_rectangle (cx, x, y, 1.0, 1.0);
  cairo_fill (cx);
}

static void default_draw_piece (cairo_t *cx, gdouble x, gdouble y, 
				gdouble phase)
{
  x += 0.5;
  y += 0.5;

  if (phase > 0.5) {
    cairo_set_source_rgb (cx, 1.0, 1.0, 1.0);
  } else {
    cairo_set_source_rgb (cx, 0.0, 0.0, 0.0);
  }

  /* We special-case the two "pure" ends of the flip to avoid
   * floating point errors in calculating the phase. */
  if ((phase > 0.98) || (phase < 0.02)) {
    cairo_arc (cx, x, y, COIN_RADIUS, 0, 2*G_PI);
    cairo_fill (cx);
  } else {
    cairo_save (cx);
    cairo_translate (cx, x, y);
    cairo_scale (cx, cos (phase*G_PI), 1.0);
    cairo_arc (cx, 0, 0, COIN_RADIUS, 0, 2*G_PI);
    cairo_fill (cx);
    cairo_restore (cx);
  }
}

static void default_draw_hilight (cairo_t *cx, gdouble x, gdouble y)
{
  x += 0.5;
  y += 0.5;

  cairo_set_source_rgba (cx, 1.0, 1.0, 1.0, 0.3);
  cairo_arc (cx, x, y, COIN_RADIUS, 0, 2*G_PI);
  cairo_fill (cx);
}

static void default_draw_grid (cairo_t *cx, gdouble x, gdouble y)
{
  cairo_save (cx);

  cairo_rectangle (cx, x, y, 1.0, 1.0);
  cairo_clip_preserve (cx);
  cairo_set_line_width (cx, 0.02);
  cairo_set_source_rgba (cx, 0.0, 0.0, 0.0, 1.0);
  cairo_stroke (cx);

  cairo_restore (cx);
}

/* ---- Squares and Diamonds, a slightly fancier theme ---- */

#define BLOCK_SIZE (0.8)
#define BLOCK_OFFSET ((1.0 - BLOCK_SIZE)/2.0)

static void sandd_draw_piece (cairo_t *cx, gdouble x, gdouble y, 
			      gdouble phase)
{
  gdouble ofs;

  cairo_save (cx);
  cairo_translate (cx, x, y);

  cairo_set_source_rgb (cx, phase, phase, phase);

  /* We special-case the two "pure" ends of the flip to avoid
   * floating point errors in calculating the phase. */
  if (phase > 0.98) {
    cairo_move_to (cx, 0.5, BLOCK_OFFSET);
    cairo_line_to (cx, 1.0 - BLOCK_OFFSET, 0.5);
    cairo_line_to (cx, 0.5, 1.0 - BLOCK_OFFSET);
    cairo_line_to (cx, BLOCK_OFFSET, 0.5);
    cairo_close_path (cx);
  } else if (phase < 0.02) {
    cairo_move_to (cx, BLOCK_OFFSET, BLOCK_OFFSET);
    cairo_rel_line_to (cx, BLOCK_SIZE, 0.0);
    cairo_rel_line_to (cx, 0.0, BLOCK_SIZE);
    cairo_rel_line_to (cx, -BLOCK_SIZE, 0.0);
    cairo_close_path (cx);
  } else {
    cairo_translate (cx, 0.5, 0.5);
    ofs = BLOCK_SIZE*(2.0 - phase)*0.25;

    cairo_move_to (cx, 0.0, -BLOCK_SIZE/2.0);
    cairo_line_to (cx, ofs, -ofs);
    cairo_line_to (cx, BLOCK_SIZE/2.0, 0.0);
    cairo_line_to (cx, ofs, ofs);
    cairo_line_to (cx, 0.0, BLOCK_SIZE/2.0);
    cairo_line_to (cx, -ofs, ofs);
    cairo_line_to (cx, -BLOCK_SIZE/2.0, 0.0);
    cairo_line_to (cx, -ofs, -ofs);
    cairo_close_path (cx);
  }

  cairo_set_line_width (cx, 0.02);
  cairo_fill_preserve (cx);
  cairo_set_source_rgb (cx, 0.0, 0.0, 0.0);
  cairo_stroke (cx);

  cairo_restore (cx);
}

static void sandd_draw_hilight (cairo_t *cx, gdouble x, gdouble y)
{
  x += 0.5;
  y += 0.5;

  cairo_set_source_rgba (cx, 1.0, 1.0, 1.0, 1.0);
  cairo_arc (cx, x, y, 0.1, 0, 2*G_PI);
  cairo_fill (cx);
}

/* This should be the only public interface. */

GtkGridBoardTheme gtk_gridboard_themes[] = {
  { N_("Plain"), default_draw_bg, default_draw_piece, 
    default_draw_hilight, default_draw_grid },
  { N_("Squares and Diamonds"), default_draw_bg, sandd_draw_piece, 
    sandd_draw_hilight, default_draw_grid },
  { NULL, NULL, NULL, NULL, NULL }
};
