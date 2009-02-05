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
#include "blockops.h"
#include "blocks.h"
#include <libgames-support/games-clutter-embed.h>

#define FONT "Sans Bold"


Block::Block ():
	what(EMPTY),
	actor(NULL)
{
}

Block::~Block ()
{
	if (actor)
		clutter_actor_destroy (CLUTTER_ACTOR(actor));
}

void
Block::createActor (ClutterActor* chamber, ClutterActor* texture_source)
{
	if (actor) {
		clutter_actor_destroy (CLUTTER_ACTOR(actor));
		actor = NULL;
	}
	actor = clutter_clone_texture_new (CLUTTER_TEXTURE(texture_source));
	clutter_group_add (CLUTTER_GROUP (chamber), actor);
}

Block&
Block::operator= (const Block& b)
{
	if (this != &b) {
		what = b.what;
		if (actor) {
			clutter_actor_destroy (CLUTTER_ACTOR(actor));
		}
		actor = b.actor;
	}
	return *this;
}

BlockOps::BlockOps() :
	background(NULL),
	foreground(NULL),
	width(0),
	height(0),
	cell_width(0),
	cell_height(0),
	renderer(NULL),
	themeID(-1),
	blocknr(0),
	rot(0),
	color(0),
	backgroundImage(NULL)
{
	field = new Block*[COLUMNS];

	posx = COLUMNS / 2;
	posy = 0;

	for (int i = 0; i < COLUMNS; ++i)
		field[i] = new Block[LINES];

	w = games_clutter_embed_new ();

	g_signal_connect (w, "configure_event", G_CALLBACK (configure), this);
	g_signal_connect (w, "size_allocate", G_CALLBACK (resize), this);

	gtk_widget_set_size_request (w, COLUMNS*190/LINES, 190);
}

BlockOps::~BlockOps()
{
	for (int i = 0; i < COLUMNS; ++i)
		delete[] field[i];

	delete[] field;
}

bool
BlockOps::blockOkHere(int x, int y, int b, int r)
{
	x -= 2;

	for (int x1 = 0; x1 < 4; ++x1)
	{
		for (int y1 = 0; y1 < 4; ++y1)
		{
			if (blockTable[b][r][x1][y1] && (x1 + x < 0))
				return false;
			if (blockTable[b][r][x1][y1] && (x1 + x >= COLUMNS))
				return false;
			if (blockTable[b][r][x1][y1] && (y1 + y >= LINES))
				return false;
			if (blockTable[b][r][x1][y1] && field[x + x1][y1 + y].what == LAYING)
				return false;
		}
	}

	return true;
}

int
BlockOps::getLinesToBottom()
{
	int lines = LINES;

	for (int x = 0; x < 4; ++x)
	{
		for (int y = 3; y >= 0; --y)
		{
			if (!blockTable[blocknr][rot][x][y])
				continue;
			int yy = posy + y;
			for (; yy < LINES; ++yy)
			{
				if (field[posx + x - 2][yy].what == LAYING)
					break;
			}
			int tmp = yy - posy - y;
			if (lines > tmp)
				lines = tmp;
		}
	}

	return lines;
}

bool
BlockOps::moveBlockLeft()
{
	bool moved = false;

	if (blockOkHere(posx - 1, posy, blocknr, rot))
	{
		putBlockInField(true);
		--posx;
		putBlockInField(false);
		moved = true;
	}

	return moved;
}

bool
BlockOps::moveBlockRight()
{
	bool moved = false;

	if (blockOkHere(posx + 1, posy, blocknr, rot))
	{
		putBlockInField(true);
		++posx;
		putBlockInField(false);
		moved = true;
	}

	return moved;
}

bool
BlockOps::rotateBlock(bool rotateCCW)
{
	bool moved = false;

	int r = rot;

	if ( rotateCCW )
	{
		if (--r < 0) r = 3;
	}
	else
	{
		if (++r >= 4) r = 0;
	}

	if (blockOkHere(posx, posy, blocknr, r))
	{
		putBlockInField(true);
		rot = r;
		putBlockInField(false);
		moved = true;
	}

	return moved;
}

bool
BlockOps::moveBlockDown()
{
	bool fallen = false;

	if (!blockOkHere(posx, posy + 1, blocknr, rot))
		fallen = true;

	if (!fallen)
	{
		putBlockInField(true);
		++posy;
		putBlockInField(false);
	}

	return fallen;
}

int
BlockOps::dropBlock()
{
	int count = 0;

	while (!moveBlockDown())
		count++;

	return count;
}

void
BlockOps::fallingToLaying()
{
	for (int x = 0; x < COLUMNS; ++x)
		for (int y = 0; y < LINES; ++y)
			if (field[x][y].what == FALLING)
				field[x][y].what = LAYING;
}

void
BlockOps::eliminateLine(int l)
{
	for (int y = l; y > 0; --y)
	{
		for (int x = 0; x < COLUMNS; ++x)
		{
			field[x][y] = field[x][y - 1];
			field[x][y - 1].what = EMPTY;
			field[x][y - 1].actor = NULL;
		}
	}
	ClutterActor *stage;
	stage = games_clutter_embed_get_stage (GAMES_CLUTTER_EMBED (w));
	rescaleBlockPos (stage);
}

int
BlockOps::checkFullLines()
{
	// we can have at most 4 full lines (vertical block)
	int fullLines[4] = {0, };
	int numFullLines = 0;

	for (int y = posy; y < MIN(posy + 4, LINES); ++y)
	{
		bool f = true;
		for (int x = 0; x < COLUMNS; ++x)
		{
			if (field[x][y].what != LAYING)
			{
				f = false;
				break;
			}
		}

		if (f)
		{
			fullLines[numFullLines] = y;
			++numFullLines;
		}
	}

	if (numFullLines > 0)
	{
		for (int i = 0; i < numFullLines; ++i)
		{
			eliminateLine(fullLines[i]);
		}
	}

	return numFullLines;
}

bool
BlockOps::generateFallingBlock()
{
	posx = COLUMNS / 2 + 1;
	posy = 0;

	blocknr = blocknr_next == -1 ? g_random_int_range(0, tableSize) :
		blocknr_next;
	rot = rot_next == -1 ? g_random_int_range(0, 4) : rot_next;
	int cn = random_block_colors ? g_random_int_range(0, NCOLOURS) :
		blocknr % NCOLOURS;
	color = color_next == -1 ? cn : color_next;

	blocknr_next = g_random_int_range(0, tableSize);
	rot_next = g_random_int_range(0, 4);
	color_next = random_block_colors ? g_random_int_range(0, NCOLOURS) :
		blocknr_next % NCOLOURS;

	if (!blockOkHere(posx, posy, blocknr, rot))
		return false;

	return true;
}

void
BlockOps::emptyField(int filled_lines, int fill_prob)
{
	ClutterActor *stage;
	stage = games_clutter_embed_get_stage (GAMES_CLUTTER_EMBED (w));
	int blank;

	for (int y = 0; y < LINES; ++y)
	{
		// Allow for at least one blank per line
		blank = g_random_int_range(0, COLUMNS);

		for (int x = 0; x < COLUMNS; ++x)
		{
			field[x][y].what = EMPTY;
			if (field[x][y].actor) {
				clutter_actor_destroy (CLUTTER_ACTOR(field[x][y].actor));
				field[x][y].actor = NULL;
			}

			if ((y>=(LINES - filled_lines)) && (x != blank) &&
			    ((g_random_int_range(0, 10)) < fill_prob)) {
				field[x][y].what = LAYING;
				field[x][y].createActor (stage, renderer->getCacheCellById
							 (g_random_int_range(0, NCOLOURS)));
				clutter_actor_set_position (CLUTTER_ACTOR(field[x][y].actor),
							    x*(cell_height), y*(cell_height));
			}
		}
	}
}

void
BlockOps::emptyField(void)
{
	emptyField(0,5);
}

void
BlockOps::putBlockInField (int bx, int by, int block, int rotation,
			   SlotType fill)
{
	ClutterActor *stage;
	stage = games_clutter_embed_get_stage (GAMES_CLUTTER_EMBED (w));

	for (int x = 0; x < 4; ++x) {
		for (int y = 0; y < 4; ++y) {
			if (blockTable[block][rotation][x][y]) {
				int i = bx - 2 + x;
				int j = y + by;

				field[i][j].what = fill;
				if ((fill == FALLING) || (fill == LAYING)) {
					field[i][j].createActor (stage, renderer->getCacheCellById
								 (color));
					clutter_actor_set_position (CLUTTER_ACTOR(field[i][j].actor),
								    i*(cell_height), j*(cell_height));
				} else {
					if (field[i][j].actor) {
						clutter_actor_destroy (CLUTTER_ACTOR(field[i][j].actor));
						field[i][j].actor = NULL;
					}
				}
			}
		}
	}
}

// This is now just a wrapper. I'm not sure which version should be
// used in general: having the field keep track of the block worries
// me, but I can't say it is definitely wrong.
void
BlockOps::putBlockInField (bool erase)
{
	if (erase)
		putBlockInField (posx, posy, blocknr, rot, EMPTY);
	else
		putBlockInField (posx, posy, blocknr, rot, FALLING);
}

bool
BlockOps::isFieldEmpty (void)
{
	for (int x = 0; x < COLUMNS; x++) {
		if (field[x][LINES-1].what != EMPTY)
			return false;
	}

	return true;
}

gboolean
BlockOps::configure(GtkWidget *widget, GdkEventConfigure *event, BlockOps *field)
{
	return FALSE;
}

gboolean
BlockOps::resize(GtkWidget *widget, GtkAllocation *allocation, BlockOps *field)
{
	field->width = allocation->width;
	field->height = allocation->height;
	field->cell_width = field->width/COLUMNS;
	field->cell_height = field->height/LINES;
	field->rescaleField();
	return FALSE;
}

void
BlockOps::rescaleBlockPos (ClutterActor* stage)
{
	for (int y = 0; y < LINES; ++y) {
		for (int x = 0; x < COLUMNS; ++x) {
			if (field[x][y].actor)
				clutter_actor_set_position (CLUTTER_ACTOR(field[x][y].actor),
							    x*(cell_height), y*(cell_height));
		}
	}
}

void
BlockOps::rescaleField ()
{
	// don't waste our time if GTK+ is just going through allocation
	if (width < 1 or height < 1)
		return;

	ClutterActor *stage;
	stage = games_clutter_embed_get_stage (GAMES_CLUTTER_EMBED (w));

	cairo_t *bg_cr;

	if (renderer)
		renderer->rescaleCache (cell_width, cell_height);
	else {
		renderer = rendererFactory (themeID, cell_width, cell_height);
	}

	if (background) {
		clutter_actor_set_size (CLUTTER_ACTOR(background), width, height);
		clutter_cairo_surface_resize (CLUTTER_CAIRO(background),
					      width, height);
	} else {
		background = clutter_cairo_new (width, height);
		/*FIXME jclinton: eventually allow solid color background
		 * for software rendering case */
		ClutterColor stage_color = { 0x61, 0x64, 0x8c, 0xff };
		clutter_stage_set_color (CLUTTER_STAGE (stage),
					 &stage_color);
		clutter_group_add (CLUTTER_GROUP (stage),
				   background);
		clutter_actor_set_position (CLUTTER_ACTOR(background),
					    0, 0);
	}

	rescaleBlockPos (stage);

	if (foreground) {
		clutter_actor_set_size (CLUTTER_ACTOR(foreground),
					width, height);
		clutter_cairo_surface_resize (CLUTTER_CAIRO(foreground),
					      width, height);
	} else {
		foreground = clutter_cairo_new (width, height);
		clutter_group_add (CLUTTER_GROUP (stage),
				   foreground);
		clutter_actor_set_position (CLUTTER_ACTOR(foreground),
					    0, 0);
		clutter_actor_raise (CLUTTER_ACTOR(foreground),
				     CLUTTER_ACTOR(background));
	}


	bg_cr = clutter_cairo_create (CLUTTER_CAIRO(background));
	cairo_set_operator (bg_cr, CAIRO_OPERATOR_CLEAR);
	cairo_paint(bg_cr);
	cairo_set_operator (bg_cr, CAIRO_OPERATOR_OVER);

	if (useBGImage && backgroundImage) {
		gdouble xscale, yscale;
		cairo_matrix_t m;

		/* FIXME: This doesn't handle tiled backgrounds in the obvious way. */
		gdk_cairo_set_source_pixbuf (bg_cr, backgroundImage, 0, 0);
		xscale = 1.0*gdk_pixbuf_get_width (backgroundImage)/width;
		yscale = 1.0*gdk_pixbuf_get_height (backgroundImage)/height;
		cairo_matrix_init_scale (&m, xscale, yscale);
		cairo_pattern_set_matrix (cairo_get_source (bg_cr), &m);
	} else if (backgroundColor)
		gdk_cairo_set_source_color (bg_cr, backgroundColor);
	else
		cairo_set_source_rgb (bg_cr, 0., 0., 0.);

	cairo_paint (bg_cr);
	cairo_destroy (bg_cr);
	drawMessage ();
	clutter_actor_show_all (stage);
}

void
BlockOps::drawMessage()
{
	PangoLayout *dummy_layout;
	PangoLayout *layout;
	PangoFontDescription *desc;
	int lw, lh;
	cairo_t *cr;
	char *msg;

	cr = clutter_cairo_create (CLUTTER_CAIRO(foreground));
	cairo_set_operator (cr, CAIRO_OPERATOR_CLEAR);
	cairo_paint(cr);
	cairo_set_operator (cr, CAIRO_OPERATOR_OVER);

	if (showPause)
		msg =  _("Paused");
	else if (showGameOver)
		msg = _("Game Over");
	else {
		cairo_destroy (cr);
		return;
	}

	// Center coordinates
	cairo_translate (cr, width / 2, height / 2);

	desc = pango_font_description_from_string(FONT);

	layout = pango_cairo_create_layout (cr);
	pango_layout_set_text (layout, msg, -1);

	dummy_layout = pango_layout_copy (layout);
	pango_layout_set_font_description (dummy_layout, desc);
	pango_layout_get_size (dummy_layout, &lw, &lh);
	g_object_unref (dummy_layout);

	// desired height : lh = widget width * 0.9 : lw
	pango_font_description_set_absolute_size (desc, ((float) lh / lw) * PANGO_SCALE * width * 0.8);
	pango_layout_set_font_description (layout, desc);
	pango_font_description_free (desc);

	pango_layout_get_size (layout, &lw, &lh);
	cairo_move_to (cr, -((double)lw / PANGO_SCALE) / 2, -((double)lh / PANGO_SCALE) / 2);
	pango_cairo_layout_path (cr, layout);
	cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
	cairo_fill_preserve (cr);
	cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
	/* A linewidth of 2 pixels at the default size. */
	cairo_set_line_width (cr, width/220.0);
	cairo_stroke (cr);

	g_object_unref(layout);
	cairo_destroy (cr);
}

void
BlockOps::setBackground(GdkPixbuf *bgImage)//, bool tiled)
{
	backgroundImage = (GdkPixbuf *) g_object_ref(bgImage);
	useBGImage = true;
//	backgroundImageTiled = tiled;
}

void
BlockOps::setBackground(GdkColor *bgColor)
{
	backgroundColor = gdk_color_copy(bgColor);
	if (backgroundImage) {
		g_object_unref (backgroundImage);
		backgroundImage = NULL;
	}
	useBGImage = false;
}

void
BlockOps::showPauseMessage()
{
	showPause = true;

	drawMessage ();
}

void
BlockOps::hidePauseMessage()
{
	showPause = false;

	drawMessage ();
}

void
BlockOps::showGameOverMessage()
{
	showGameOver = true;

	drawMessage ();
}

void
BlockOps::hideGameOverMessage()
{
	showGameOver = false;

	drawMessage ();
}

void
BlockOps::setTheme (gint id)
{
	// don't waste time if theme is the same (like from initOptions)
	if (themeID == id)
		return;

	themeID = id;
	if (renderer) {
		delete renderer;
		renderer = rendererFactory (themeID, cell_width,
					    cell_height);
	} else {
		renderer = rendererFactory (themeID, cell_width,
					    cell_height);
	}
}
