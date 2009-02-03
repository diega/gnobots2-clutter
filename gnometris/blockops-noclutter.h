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

enum SlotType {
	EMPTY,
	TARGET,
	FALLING,
	LAYING
};

struct Block {
	SlotType what;
	int color;
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
	void setUseTarget (bool);
	bool getUseTarget () {
		return useTarget;
	};

private:
	void putBlockInField (int bx, int by, int blocknr, int rotation,
			      SlotType fill);
	bool blockOkHere (int x, int y, int b, int r);
	void eliminateLine (int l);

	bool useTarget;
protected:
	void clearTarget ();
	void generateTarget ();

	Block **field;

	int blocknr;
	int rot;
	int color;

	int posx;
	int posy;
};

#endif //__blockops_h__
