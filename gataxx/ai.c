/* (C) 2003/2004 Sjoerd Langkemper
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


#include <glib.h>
#include <gtkgridboard.h>
#include "gataxx.h"
#include "ai.h"
#include "properties.h"

int alphabeta(GtkWidget * gridboard, int alpha, int beta, int mode, int me, int depth);
move computer_move_easy(GtkWidget * gridboard, int turn);
move computer_move_ab(GtkWidget * gridboard, int turn, int depth);

static int steps;

/* takes a random move */
move computer_move_random(GtkWidget * gridboard, int turn) {
	move * pm;
	move result;
	int size, item;

	pm=get_possible_moves(gridboard, turn);
	size=get_possible_moves_size(pm);
	if (size>0) {
		item=g_random_int_range(0, size);
	} else {
		item=0;
	}
	result=pm[item];
	free_possible_moves(pm);
	return result;
}

/* makes a move which is not completely dumb, but not as hard as the move with
 * the highest heuristic value
 */
move computer_move_easy(GtkWidget * gridboard, int turn) {
	move * pm;
	move bm={ {0, 0, 0}, {0, 0, 0} };
	int i, size, h, maxh=-100;
	steps=0;

	pm=get_possible_moves(gridboard, turn);
	size=get_possible_moves_size(pm);
	for (i=0; i<size; i+=5) {
		gtk_gridboard_save_state(gridboard, NULL);
		gridboard_move(gridboard, pm[i]);
		h=gtk_gridboard_count_pieces(gridboard, turn);
		gtk_gridboard_revert_state(gridboard);
		if (h>maxh) {
			maxh=h;
			bm=pm[i];
		}
	}
	free_possible_moves(pm);
	return bm;
}

/* decides which move to take with alphabeta algorithm
 * @param turn  the current player
 * @param depth the maximum search depth in the tree
 * @return the chosen move
 */
move computer_move_ab(GtkWidget * gridboard, int turn, int depth) {
	move * pm; 				/* possible moves */
	move bm={ {0, 0, 0}, {0, 0, 0} }; 	/* best move */
	move * gm; 				/* good moves */
	int i, pmsize, gmsize=0;
	int h, maxh=-100;			/* heuristic values */
	steps=0;				/* FIXME: debug var */

	pm=get_possible_moves(gridboard, turn);
	pmsize=get_possible_moves_size(pm);
	gm=calloc(pmsize, sizeof(move));
	for (i=0; i<pmsize; i++) {
		gtk_gridboard_save_state(gridboard, NULL);
		gridboard_move(gridboard, pm[i]);
		h=alphabeta(gridboard, -100, 100, FALSE, turn, depth);
		gtk_gridboard_revert_state(gridboard);
		if (h>maxh) {
			maxh=h;
			gmsize=0;
			gm[gmsize++]=pm[i];
		} else if (h==maxh) {
			gm[gmsize++]=pm[i];
		}
	}
	free_possible_moves(pm);

	i=g_random_int_range(0, gmsize);
	bm=gm[i];
	free(gm);
	return bm;
	
}

/* alphabeta search the possible moves */
int alphabeta(GtkWidget * gridboard, int alpha, int beta, int mode, int me, int depth) {
	move * pm;
	int size, i, h;
	int notme=(me==WHITE ? BLACK : WHITE);
	int mec, notmec;

	steps++;

	if (depth==0) {
		mec=gtk_gridboard_count_pieces(gridboard, me);
		if (mec==0) return -50;
		notmec=gtk_gridboard_count_pieces(gridboard, notme);
		if (notmec==0) return 50;
		return mec; /* -notmec; */
	}
	if (mode) { /* max */
		pm=get_possible_moves(gridboard, me);
		size=get_possible_moves_size(pm);
		for (i=0; i<size; i++) {
			gtk_gridboard_save_state(gridboard, NULL);
			gridboard_move(gridboard, pm[i]);
			h=alphabeta(gridboard, alpha, beta, !mode, me, depth-1);
			if (h>alpha) alpha=h;
			gtk_gridboard_revert_state(gridboard);
			if (alpha>beta) break; 
		}
		free_possible_moves(pm);
		return alpha;
	} else {
		pm=get_possible_moves(gridboard, notme);
		size=get_possible_moves_size(pm);
		for (i=0; i<size; i++) {
			gtk_gridboard_save_state(gridboard, NULL);
			gridboard_move(gridboard, pm[i]);
			h=alphabeta(gridboard, alpha, beta, !mode, me, depth-1);
			if (h<beta) beta=h;
			gtk_gridboard_revert_state(gridboard);
			if (alpha>beta) break; 
		}
		free_possible_moves(pm);
		return beta;
	}
}


/* this function gets called from gataxx */
move computer_move(GtkWidget * gridboard, int turn) {
	int level=props_get_level(turn);

	if (level==1) {
		return computer_move_random(gridboard, turn);
	} else if (level==2) {
		return computer_move_easy(gridboard, turn);
	} else {
		return computer_move_ab(gridboard, turn, level-3);
	}
}

/* returns possible moves
 * The less moves this returns, the faster the search algorithm will work.
 * Therefore, this does not really return _all_ possible moves.
 */
move * get_possible_moves(GtkWidget * gridboard, int turn) {
	int x, y, i=0;
	move * pm;
	move bm;

	pm=calloc(sizeof(move), BWIDTH*BHEIGHT);
	for (x=0; x<BWIDTH; x++) {
		for (y=0; y<BHEIGHT; y++) {
			bm=get_best_move_to(gridboard, x, y, turn);
			if (bm.from.valid) pm[i++]=bm;
		}
	}
	pm[0].from.valid=i;
	return pm;
}

int get_possible_moves_size(move * pm) {
	return pm[0].from.valid;
}

void free_possible_moves(move * pm) {
	free(pm);
}

/* only thing to make sure the returned move is the "best" move is to
 * prioritize normal moves over jumps
 */
move get_best_move_to(GtkWidget * gridboard, int x, int y, int me) {
	move bm;
	int _x, _y, piece;
	
	bm.from.x=bm.from.y=bm.from.valid=bm.to.x=bm.to.y=bm.to.valid=0;
	if (gtk_gridboard_get_piece(gridboard, x, y)!=EMPTY) return bm;

	/* search for normal moves */
	for (_x=MAX(0, x-1); _x<MIN(BWIDTH, x+2); _x++) {
		for (_y=MAX(0, y-1); _y<MIN(BHEIGHT, y+2); _y++) {
			piece=gtk_gridboard_get_piece(gridboard, _x, _y);
			if (piece==me) {
				bm.from.x=_x;
				bm.from.y=_y;
				bm.from.valid=TRUE;
				bm.to.x=x;
				bm.to.y=y;
				bm.to.valid=TRUE;
				return bm;
			}
		}
	}

	/* search for jumps also */
	for (_x=MAX(0, x-2); _x<MIN(BWIDTH, x+3); _x++) {
		for (_y=MAX(0, y-2); _y<MIN(BHEIGHT, y+3); _y++) {
			piece=gtk_gridboard_get_piece(gridboard, _x, _y);
			if (piece==me) {
				bm.from.x=_x;
				bm.from.y=_y;
				bm.from.valid=TRUE;
				bm.to.x=x;
				bm.to.y=y;
				bm.to.valid=TRUE;
				return bm;
			}
		}
	}
	
	return bm;
}


