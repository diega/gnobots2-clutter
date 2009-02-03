/* -*- mode:C++; tab-width:8; c-basic-offset:8; indent-tabs-mode:true -*- */
#ifndef __field_noclutter_h__
#define __field_noclutter_h__

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

class Field:public BlockOps {
public:
	Field ();
	~Field ();

	void setBackground (GdkPixbuf * bgImage);	//, bool tiled); fixme: move tiling here.
	void setBackground (GdkColor * bgColor);
	void placeBlock (int x, int y, int bcolor, bool remove);
	void showPauseMessage ();
	void hidePauseMessage ();
	void showGameOverMessage ();
	void hideGameOverMessage ();
	void redraw ();
	void setTheme (gint id);

	GtkWidget *getWidget () {
		return w;
	}
private:
	GtkWidget * w;

	int width;
	int height;

	cairo_surface_t *buffer;
	cairo_surface_t *background;
	cairo_surface_t **blocks;

	bool showPause;
	bool showGameOver;
	Renderer *renderer;
	gint rendererTheme;

	GdkPixbuf *backgroundImage;
	bool backgroundImageTiled;
	bool useBGImage;
	GdkColor *backgroundColor;
	gint themeID;

	void drawMessage (cairo_t * cr, const char *msg);
	void redrawAll ();
	void draw (gint x, gint y, gint w, gint h);
	void draw ();
	void rescaleBackground ();

	static gboolean configure (GtkWidget * widget, GdkEventConfigure * event,
				   Field * field);
	static gboolean expose (GtkWidget * widget, GdkEventExpose * event,
				Field * field);

};

#endif //__field_noclutter_h__
