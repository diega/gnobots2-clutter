/* -*- mode:C++; tab-width:8; c-basic-offset:8; indent-tabs-mode:true -*- */
#ifndef __blockops_h__
#define __blockops_h__

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
#include "renderer.h"

#include <clutter/clutter.h>

enum SlotType {
	EMPTY,
	FALLING,
	LAYING
};

class Block {
public:
	Block ();
	~Block ();

	Block& operator=(const Block& b);

	SlotType what;
	ClutterActor* actor;

	void createActor (ClutterActor* chamber, ClutterActor* texture_source);
	void associateActor (ClutterActor* chamber, ClutterActor* other_actor);
};

class BlockOps {
public:
	BlockOps ();
	~BlockOps ();

	bool moveBlockLeft ();
	bool moveBlockRight ();
	bool moveBlockDown ();
	bool rotateBlock (bool);
	int dropBlock ();
	void fallingToLaying ();
	int checkFullLines ();
	bool generateFallingBlock ();
	void emptyField (void);
	void emptyField (int filled_lines, int fill_prob);
	void putBlockInField (bool erase);
	int getLinesToBottom ();
	bool isFieldEmpty (void);

	void setBackground (GdkPixbuf * bgImage); //, bool tiled); fixme: move tiling here.
	void setBackground (GdkColor * bgColor);
	void placeBlock (int x, int y, int bcolor, bool remove);
	void showPauseMessage ();
	void hidePauseMessage ();
	void showGameOverMessage ();
	void hideGameOverMessage ();
	void setTheme (gint id);
	void drawMessage ();

	GtkWidget *getWidget () {
		return w;
	}

private:
	void putBlockInField (int bx, int by, int blocknr, int rotation,
			      SlotType fill);
	bool blockOkHere (int x, int y, int b, int r);
	void eliminateLine (int l);

	GtkWidget * w;

	ClutterActor *background;
	ClutterActor *foreground;
	guint width;
	guint height;
	guint cell_width;
	guint cell_height;
	Renderer *renderer;
	gint themeID;

	Block **field;

	int blocknr;
	int rot;
	int color;

	bool showPause;
	bool showGameOver;

	GdkPixbuf *backgroundImage;
	bool backgroundImageTiled;
	bool useBGImage;
	GdkColor *backgroundColor;

	void rescaleField ();
	void rescaleBlockPos (ClutterActor *stage);

	int posx;
	int posy;

	static gboolean configure (GtkWidget * widget, GdkEventConfigure * event,
				   BlockOps * field);
	static gboolean resize (GtkWidget * widget, GtkAllocation * event,
					   BlockOps * field);
};

#endif //__blockops_h__
