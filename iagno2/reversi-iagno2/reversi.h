/*
 * Reversi Iagno II: A convenience library for Iagno II and plugins
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

#ifndef _REVERSI_IAGNO2_H_
#define _REVERSI_IAGNO2_H_

#define BOARDSIZE 8
#define INDEX(r,c) (((r) << 3) + (c))
#define ROW(i)     ((i) >> 3)
#define COL(i)     ((i) & 0x7)

#define EMPTY -1
#define BLACK 0
#define WHITE 1

#define OPPONENT(player) (((player)==BLACK)?WHITE:BLACK)

typedef struct {
  int index;
  int player;
} ReversiMove;

typedef struct {
  gchar *board;
  ReversiMove *moves;
  int move_count;
} ReversiBoard;

ReversiBoard *reversi_init_board ();

void reversi_destroy_board (ReversiBoard *board);

gboolean is_valid_move (ReversiBoard *board, gchar index, gchar player);

void move (ReversiBoard *board, gchar index, gchar player);

/*
gchar other_player (gchar player);
*/

gboolean are_valid_moves (ReversiBoard *board, gchar player);

/*
ReversiMove get_move (ReversiBoard board, int move);
*/

#endif
