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

	Block& move_from (Block &b, BlockOps *f);

	SlotType what;
	guint color;
	ClutterActor *actor;

	int x;
	int y;
	static ClutterTimeline *fall_tml;
	static ClutterAlpha *fall_alpha;
	ClutterBehaviour *fall_path;
	static ClutterTimeline *explode_tml;
	static ClutterEffectTemplate *explode_tmpl;

	void createActor (ClutterActor *chamber, ClutterActor *texture_source);
	void associateActor (ClutterActor *chamber, ClutterActor *other_actor);

	static GList *destroy_actors;
	static GList *fall_behaviours;
	static void explode_end (ClutterTimeline *timeline, gpointer *f);
	static void move_end (ClutterTimeline *timeline, gpointer *data);
	static void fall_end (ClutterTimeline *timeline, BlockOps *f);
};

class BlockOps {
	friend class Block;
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
	void putBlockInField (SlotType fill);
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
	bool blockOkHere (int x, int y, int b, int r);
	void eliminateLine (int l);
	bool checkFullLine(int l);

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
	void rescaleBlockPos ();
	void updateBlockInField ();

	int posx;
	int posy;
	int posx_old;
	int posy_old;

	ClutterActor *playingField;
	static guint32 earthquake_alpha_func (ClutterAlpha *alpha, gpointer data);
	static ClutterTimeline *long_anim_tml;
	static ClutterEffectTemplate *effect_earthquake;

	float quake_ratio;

	int center_anchor_x;
	int center_anchor_y;

	ClutterTimeline *move_block_tml;
	ClutterAlpha *move_block_alpha;
	ClutterBehaviour *move_path[4][4];

	static void move_end (ClutterTimeline *tml, BlockOps *f);

	static gboolean resize(GtkWidget *widget, GtkAllocation *event,
					   BlockOps *field);
};

#endif //__blockops_h__
