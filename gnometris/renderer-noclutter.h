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

#ifndef __renderer_h__
#define __renderer_h__

#include <cairo.h>
#include <glib.h>

#include "blocks.h"
#include "blockops-noclutter.h"

struct ThemeTableEntry {
	const gchar *name;
	const gchar *id;
};

extern const ThemeTableEntry ThemeTable[];

class Renderer {
public:
	Renderer (cairo_surface_t * dst, cairo_surface_t * bg, Block ** src,
		  int w, int h, int pxw, int pxh);
	virtual ~ Renderer ();
	virtual void render ();

	void setTarget (cairo_surface_t *target);
	void setBackground (cairo_surface_t *background);

	Block **data;
	int width;
	int height;
	int pxwidth;
	int pxheight;
protected:

	cairo_surface_t * target;
	cairo_surface_t *background;

	virtual void drawCell (cairo_t * cr, gint x, gint y);
	virtual void drawBackground (cairo_t * cr);
	virtual void drawForeground (cairo_t * cr);
};

Renderer *rendererFactory (gint id, cairo_surface_t * dst,
			   cairo_surface_t * bg, Block ** src, int w,
			   int h, int pxw, int pxh);
gint themeNameToNumber (const gchar * id);

class JoinedUp:public Renderer {
public:
	JoinedUp (cairo_surface_t * dst, cairo_surface_t * bg, Block ** src,
	int w, int h, int pxw, int pxh):Renderer (dst, bg, src, w, h, pxw,
						  pxh) {}
protected:
	virtual void drawCell (cairo_t * cr, gint x, gint y);

private:
	double border;
	void drawInnerCorner (cairo_t * cr);
	void drawOuterCorner (cairo_t * cr);
	void drawHEdge (cairo_t * cr);
	void drawVEdge (cairo_t * cr);
};

class TangoBlock:public Renderer {
public:
	TangoBlock (cairo_surface_t * dst, cairo_surface_t * bg, Block ** src,
		    int w, int h, int pxw, int pxh, gboolean grad);

protected:
	virtual void drawCell (cairo_t * cr, gint x, gint y);
	gboolean usegrads;

private:
	void drawRoundedRectangle (cairo_t * cr, gdouble x, gdouble y, gdouble w, gdouble h, gdouble r);
};

#endif // __renderer_h__
