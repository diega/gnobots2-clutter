/*
 * written by J. Marcin Gorycki <mgo@olicom.dk>
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

#include "blockops.h"
#include "blocks.h"

BlockOps::BlockOps()
{
	emptyField();
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
BlockOps::rotateBlock()
{
	bool moved = false;
	
	int r = rot;
	if (++r >= 4)
		r = 0;
	
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
BlockOps::dropBlock()
{
	while (!moveBlockDown())
		;
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
BlockOps::checkFullLines()
{
	bool found;
	
	do
	{
		found = false;
		
		for (int y = LINES - 1; y >= 0; --y)
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
				++score;
				
				found = true;
				for (int y1 = y - 1; y1 >= 0; --y1)
				{
					for (int x = 0; x < COLUMNS; ++x)
					{
						field[x][y1 + 1] = field[x][y1];
						field[x][y1].what = EMPTY;
					}
				}
				break;
			}
		}
	}
	while (found);
}

bool
BlockOps::generateFallingBlock()
{
	posx = COLUMNS / 2 + 1;
	posy = 0;
	
	blocknr = blocknr_next == -1 ? rand() % tableSize : blocknr_next;
	rot = rot_next == -1 ? rand() % 4 : rot_next;
	color = color_next == -1 ? rand() % nr_of_colors : color_next;
	
	blocknr_next = rand() % tableSize;
	rot_next = rand() % 4;
	color_next = rand() % nr_of_colors;
	
	if (!blockOkHere(posx, posy, blocknr, rot))
		return false;

	return true;
}

void
BlockOps::emptyField()
{
	for (int y = 0; y < LINES; ++y)
	{
		for (int x = 0; x < COLUMNS; ++x)
			field[x][y].what = EMPTY;
	}
}

void
BlockOps::putBlockInField(bool erase)
{
	for (int x = 0; x < 4; ++x)
		for (int y = 0; y < 4; ++y)
			if (blockTable[blocknr][rot][x][y])
			{
				field[posx - 2 + x][y + posy].what = erase ? EMPTY : FALLING;
				field[posx - 2 + x][y + posy].color = color;
			}
}




