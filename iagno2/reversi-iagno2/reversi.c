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

#include <gnome.h>

#include "defines.h"
#include "reversi.h"

/*
ReversiBoard *board = NULL;
*/
/*gchar whose_turn = BLACK_TILE;*/

ReversiMove game_history[60];

ReversiBoard *
reversi_init_board ()
{
  ReversiBoard *tmp = g_new (ReversiBoard, 1);

  tmp->board = g_new0 (gchar, BOARDSIZE * BOARDSIZE);

  tmp->moves = g_new (ReversiMove, 60);

  tmp->move_count = 0;

  return tmp;
}

void
reversi_destroy_board (ReversiBoard *board)
{
  g_free (board->board);

  g_free (board->moves);
  
  g_free (board);
}

/*
gchar
other_player (gchar player)
{
  gchar tmp_other_player;

  tmp_other_player = (player == BLACK_TILE) ? WHITE_TILE : BLACK_TILE;

  return (tmp_other_player);
}
*/

static gboolean
check_direction (ReversiBoard *board,
                 gchar index,
                 gchar player,
                 gchar direction)
{
  gchar not_me;
  gint i, j;
  gint dx, dy;
  gint possible;

  not_me = OTHER_TILE (player);

  switch (direction) {
    case UP:
      dx = 0;
      dy = -1;
      break;
    case DOWN:
      dx = 0;
      dy = 1;
      break;
    case LEFT:
      dx = -1;
      dy = 0;
      break;
    case RIGHT:
      dx = 1;
      dy = 0;
      break;
    case UP_LEFT:
      dx = -1;
      dy = -1;
      break;
    case UP_RIGHT:
      dx = 1;
      dy = -1;
      break;
    case DOWN_LEFT:
      dx = -1;
      dy = 1;
      break;
    case DOWN_RIGHT:
      dx = 1;
      dy = 1;
      break;
  }
  
  i = ROW(index) + dx;
  j = COL(index) + dy;
  possible = 0;

  while ((i >= 0) && (i <= 7) && (j >= 0) && (j <= 7) &&
         (board->board[INDEX(i,j)] == not_me)) {
    possible = 1;
    i += dx;
    j += dy;
  }

  if ((i >= 0) && (i <= 7) && (j >= 0) && (j <= 7) && possible &&
      (board->board[INDEX(i,j)] == player)) {
    return TRUE;
  }

  return FALSE;
}

static void
move_direction (ReversiBoard *board,
                gchar index,
                gchar player,
                gchar direction)
{
  gchar not_me;
  gint i, j;
  gint dx, dy;

  switch (direction) {
    case UP:
      dx = 0;
      dy = -1;
      break;
    case DOWN:
      dx = 0;
      dy = 1;
      break;
    case LEFT:
      dx = -1;
      dy = 0;
      break;
    case RIGHT:
      dx = 1;
      dy = 0;
      break;
    case UP_LEFT:
      dx = -1;
      dy = -1;
      break;
    case UP_RIGHT:
      dx = 1;
      dy = -1;
      break;
    case DOWN_LEFT:
      dx = -1;
      dy = 1;
      break;
    case DOWN_RIGHT:
      dx = 1;
      dy = 1;
      break;
  }

  not_me = OTHER_TILE (player);

  i = ROW(index) + dx;
  j = COL(index) + dy;

  while ((i >= 0) && (i <= 7) && (j >= 0) && (j <= 7) &&
         (board->board[INDEX(i,j)] == not_me)) {
    board->board[INDEX(i,j)] = player;
    i += dx;
    j += dy;
  }
}

gboolean
is_valid_move (ReversiBoard *board, gchar index, gchar player)
{
  gboolean valid = FALSE;

  if (board->board[index]) {
    return FALSE;
  }

  valid = check_direction (board, index, player, UP)
      || check_direction (board, index, player, UP_LEFT)
      || check_direction (board, index, player, UP_RIGHT)
      || check_direction (board, index, player, LEFT)
      || check_direction (board, index, player, RIGHT)
      || check_direction (board, index, player, DOWN)
      || check_direction (board, index, player, DOWN_LEFT)
      || check_direction (board, index, player, DOWN_RIGHT);

  return valid;
}

void
move (ReversiBoard *board, gchar index, gchar player)
{
  if (check_direction (board, index, player, UP)) {
    move_direction (board, index, player, UP);
  }
  if (check_direction (board, index, player, UP_LEFT)) {
    move_direction (board, index, player, UP_LEFT);
  }
  if (check_direction (board, index, player, UP_RIGHT)) {
    move_direction (board, index, player, UP_RIGHT);
  }
  if (check_direction (board, index, player, LEFT)) {
    move_direction (board, index, player, LEFT);
  }
  if (check_direction (board, index, player, RIGHT)) {
    move_direction (board, index, player, RIGHT);
  }
  if (check_direction (board, index, player, DOWN)) {
    move_direction (board, index, player, DOWN);
  }
  if (check_direction (board, index, player, DOWN_LEFT)) {
    move_direction (board, index, player, DOWN_LEFT);
  }
  if (check_direction (board, index, player, DOWN_RIGHT)) {
    move_direction (board, index, player, DOWN_RIGHT);
  }

  board->board[index] = player;

  board->moves[board->move_count].index = index;
  board->moves[board->move_count].player = player;
  board->move_count++;

  /*
  player = other_player (player);
  */
}

gboolean
are_valid_moves (ReversiBoard *board, gchar player)
{
  gboolean valid_moves = 0;
  gchar i;

  for (i = 0; i < 64; i++) {
    valid_moves |= is_valid_move (board, i, player);
  }

  return (valid_moves);
}
