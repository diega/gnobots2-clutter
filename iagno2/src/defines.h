/*
 * Iagno II: An extensible Reversi game for GNOME
 * Copyright (C) 1999-2000 Ian Peters <itp@gnu.org>
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

#ifndef _IAGNO2_DEFINES_H_
#define _IAGNO2_DEFINES_H_

#define TILEWIDTH   60
#define TILEHEIGHT  60
#define GRIDWIDTH   1
/*
#define BOARDSIZE   8
*/
#define BOARDWIDTH  (TILEWIDTH + GRIDWIDTH) * BOARDSIZE - 1
#define BOARDHEIGHT (TILEHEIGHT + GRIDWIDTH) * BOARDSIZE - 1
/*
#define BOARDWIDTH  TILEWIDTH * BOARDSIZE
#define BOARDHEIGHT TILEHEIGHT * BOARDSIZE
*/

/*
#define INDEX(r,c) ((r << 3) + c)
#define ROW(x)     (x >> 3)
#define COL(x)     (x & 0x7)
*/

#define PLAYER(x)  (x-1)?1:0

/*
#define WHITE_TILE 31
#define BLACK_TILE 1
*/

#define UP		-8
#define UP_LEFT		-9
#define UP_RIGHT	-7
#define LEFT		-1
#define RIGHT		1
#define DOWN		8
#define DOWN_LEFT	7
#define DOWN_RIGHT	9

#endif
