/* 
 * ai.c - Artificial intelligence
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

#ifndef AI_H
#define AI_H

move computer_move_random(GtkGridBoard *gridboard, int turn);
move computer_move(GtkGridBoard *gridboard, int turn);
move *get_possible_moves(GtkGridBoard *gridboard, int turn);
int get_possible_moves_size(move *pm);
void free_possible_moves(move *pm);
move get_best_move_to(GtkGridBoard *gridboard, int x, int y, int me);

#endif /* AI_H */
