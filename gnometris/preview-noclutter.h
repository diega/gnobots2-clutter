/* -*- mode:C++; tab-width:8; c-basic-offset:8; indent-tabs-mode:true -*- */
#ifndef __preview_noclutter_h__
#define __preview_noclutter_h__

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

#include "tetris.h"
#include "blockops-noclutter.h"
#include "renderer-noclutter.h"

class Preview {
public:
	Preview ();
	~Preview ();

	GtkWidget *getWidget () {
		return w;
	}

	void enable (bool enable);
	void previewBlock (int bnr, int brot, int bcolor);
	void setTheme (int id);

private:
	GtkWidget * w;
	gint width;
	gint height;

	int blocknr;
	int blockrot;
	int blockcolor;

	bool enabled;

	int themeID;
	cairo_surface_t *background;

	static gint configure (GtkWidget * widget, GdkEventConfigure * event,
			       Preview * preview);
	static gint expose (GtkWidget * widget, GdkEventExpose * event,
			    Preview * preview);

Block **blocks;
};

#endif //__preview_h__
