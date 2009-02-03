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
#include "blockops-noclutter.h"
#include "blocks.h"

BlockOps::BlockOps() :
	useTarget (false),
	blocknr (0),
	rot (0),
	color (0)
{
	field = new Block*[COLUMNS];

	posx = COLUMNS / 2;
	posy = 0;

	for (int i = 0; i < COLUMNS; ++i)
		field[i] = new Block[LINES];

	emptyField();
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

void
BlockOps::clearTarget ()
{
	for (int x = 0; x < COLUMNS; ++x)
		for (int y = 0; y < LINES; ++y)
			if (field[x][y].what == TARGET)
				field[x][y].what = EMPTY;
}

// The target is the set of blocks which the currently falling block
// will occupy when it lands. It is an aid for beginners.
void
BlockOps::generateTarget ()
{
	if (!useTarget)
		return;

	clearTarget ();

	// FIXME: Check that this is actually guaranteed
	// to terminate (i.e. posx, posy, blocknr and rot
	// are guaranteed to be valid).
	int n = 0;
	do {
		n++;
	} while (blockOkHere (posx, posy + n, blocknr, rot));
	n--;

	// Mark the relevant places.
	putBlockInField (posx, posy + n, blocknr, rot, TARGET);
}

void
BlockOps::setUseTarget (bool use)
{
	useTarget = use;
	if (!useTarget)
		clearTarget ();
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
			field[x][y - 1].color = 0;
		}
	}
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
	int blank;

	for (int y = 0; y < LINES; ++y)
	{
		// Allow for at least one blank per line
		blank = g_random_int_range(0, COLUMNS);

		for (int x = 0; x < COLUMNS; ++x)
		{
			field[x][y].what = EMPTY;
			field[x][y].color = 0;

			if ((y>=(LINES - filled_lines)) && (x != blank) &&
			    ((g_random_int_range(0, 10)) < fill_prob)) {
				field[x][y].what = LAYING;
				field[x][y].color =
				  g_random_int_range(0, NCOLOURS);
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
	for (int x = 0; x < 4; ++x)
	{
		for (int y = 0; y < 4; ++y)
		{
			if (blockTable[block][rotation][x][y])
			{
				int i = bx - 2 + x;
				int j = y + by;

				if ((fill == TARGET) &&
				    (field[i][j].what != EMPTY))
					return;

				field[i][j].what = fill;
				if ((fill == FALLING) || (fill == LAYING))
					field[i][j].color = color;
				else
					field[i][j].color = 0;
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
