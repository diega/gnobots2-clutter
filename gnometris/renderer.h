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

#ifndef __renderer_h__
#define __renderer_h__

#include <cairo.h>
#include <glib.h>

#include "blockops.h"

class Renderer
{
 public:
        Renderer (cairo_surface_t *dst, cairo_surface_t *bg, Block **src, 
                  int w, int h, int pxw, int pxh);
        ~Renderer ();
        void render ();
 private:
        cairo_surface_t *target;
        cairo_surface_t *background;         
        Block **data;
        int width;
        int height;
        int pxwidth;
        int pxheight;
};
 
#endif // __renderer_h__
