/* -*- mode:C++; tab-width:8; c-basic-offset:8; indent-tabs-mode:true -*- */

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

#include <config.h>
#include "preview.h"

#define PREVIEW_WIDTH 6
#define PREVIEW_HEIGHT 6

// FIXME: Remove
#define PREVIEW_SIZE 5

Preview::Preview():
	width(0),
	height(0),
	blocknr(-1),
	color(-1),
	themeID(-1),
	renderer(NULL),
	enabled(true)
{
	blocks = new Block*[PREVIEW_WIDTH];
	for (int i = 0; i < PREVIEW_WIDTH; i++) {
		blocks[i] = new Block [PREVIEW_HEIGHT];
	}

	w = games_clutter_embed_new();

	g_signal_connect (w, "configure_event", G_CALLBACK (configure), this);

	/* FIXME: We should scale with the rest of the UI, but that requires
	 * changes to the widget layout - i.e. wrap the preview in an
	 * fixed-aspect box. */
	gtk_widget_set_size_request (w, PREVIEW_SIZE * 20,
				     PREVIEW_SIZE * 20);
	ClutterActor *stage;
	stage = games_clutter_embed_get_stage (GAMES_CLUTTER_EMBED (w));

	ClutterColor stage_color = { 0x0, 0x0, 0x0, 0xff };
	clutter_stage_set_color (CLUTTER_STAGE (stage),
				 &stage_color);
	rotar = clutter_group_new ();
	clutter_group_add (CLUTTER_GROUP (stage),
			   rotar);
	clutter_actor_show_all (stage);

	rot_timeline = clutter_timeline_new_for_duration (5000);
	clutter_timeline_set_loop (rot_timeline, true);
	alpha = clutter_alpha_new_full (rot_timeline,
			CLUTTER_ALPHA_RAMP_INC,
			NULL, NULL);
	rot_behav = clutter_behaviour_rotate_new (alpha,
			CLUTTER_Z_AXIS, CLUTTER_ROTATE_CW, 0.0, 360.0);
	clutter_behaviour_rotate_set_center (CLUTTER_BEHAVIOUR_ROTATE (rot_behav),
			PREVIEW_SIZE*12, PREVIEW_SIZE*12, 0);
	clutter_actor_set_position (CLUTTER_ACTOR(rotar), 0, 0);
	clutter_behaviour_apply (rot_behav, rotar);
	clutter_timeline_start (rot_timeline);
}

Preview::~Preview ()
{
	for (int i = 0; i < PREVIEW_WIDTH; i++)
		delete[] blocks[i];

	delete[] blocks;
	delete renderer;
}

void
Preview::enable(bool en)
{
	enabled = en;
}

void
Preview::setTheme (gint id)
{
	if (themeID == id)
		return;

	themeID = id;

	if (renderer) {
		delete renderer;
		renderer = rendererFactory (themeID, PREVIEW_SIZE*4, PREVIEW_SIZE*4);
	} else {
		renderer = rendererFactory (themeID, PREVIEW_SIZE*4, PREVIEW_SIZE*4);
	}
}

void
Preview::regenerateRenderer ()
{
	if (renderer)
		renderer->rescaleCache (PREVIEW_SIZE*4, PREVIEW_SIZE*4);
	else {
		renderer = rendererFactory (themeID, PREVIEW_SIZE*4, PREVIEW_SIZE*4);
	}
}

void
Preview::previewBlock(gint bnr, gint bcol)
{
	ClutterActor *stage;
	stage = games_clutter_embed_get_stage (GAMES_CLUTTER_EMBED (w));

	int x, y;

	blocknr = bnr;
	color = bcol;

	for (x = 1; x < PREVIEW_WIDTH - 1; x++) {
		for (y = 1; y < PREVIEW_HEIGHT - 1; y++) {
			if ((blocknr != -1) &&
			    blockTable[blocknr][0][x-1][y-1]) {
				blocks[x][y].what = LAYING;
				blocks[x][y].createActor (rotar,
							  renderer->getCacheCellById (color));
				clutter_actor_set_position (CLUTTER_ACTOR(blocks[x][y].actor),
							    x*PREVIEW_SIZE*4, y*PREVIEW_SIZE*4);
			} else {
				blocks[x][y].what = EMPTY;
				if (blocks[x][y].actor) {
					clutter_actor_destroy (blocks[x][y].actor);
					blocks[x][y].actor = NULL;
				}
			}
		}
	}
	positionRotar ();
	clutter_actor_show_all (stage);
}

void
Preview::positionRotar()
{

	//
}

gint
Preview::configure(GtkWidget * widget, GdkEventConfigure * event, Preview * preview)
{
	preview->width = event->width;
	preview->height = event->height;

	preview->regenerateRenderer ();
	preview->positionRotar ();
	return TRUE;
}

