/* -*- mode: C; indent-tabs-mode: t; tab-width: 2; c-basic-offset: 2; -*- */

/* game.c: All the code pretaining to the actual game is here.
 *
 * Copyright (c) 2004 by Callum McKenzie
 *
 */

#include "globals.h"

#include "drawing.h"
#include "game.h"
#include "ui.h"

/* The blank colour. */
const gint NONE = -1;

game_cell *board = NULL;

gint last_board_width = -1;
gint last_board_height = -1;

gint score;

/* The stack used for the "floodfill" algorithm. */
coordinates *findstack = NULL;
/* The list of cells in the currently selected region. */
coordinates *selected = NULL;
/* The length of the selected list. */
gint count = 0;

gint game_state = GAME_IDLE;

void set_sizes (gint size)
{
	board_width = board_sizes[size][0];
	board_height = board_sizes[size][1];
	ncolours = board_sizes[size][2];
	
	gconf_client_set_int (gcclient, GCONF_SIZE_KEY, size, NULL);

	new_frame_ratio (board_width, board_height);
}

game_cell * get_game_cell (int x, int y)
{
	return board + y*board_width + x;
}

/* FIXME: The idiom used in push and pop is strange and may be a bad thing. */

static void push (coordinates **s, gint8 x, gint8 y)
{
	if ((x < 0) || (y < 0) || (x >= board_width) || (y >= board_height))
		return;

	(*s)->x = x;
	(*s)->y = y;
	(*s)++;
}

static void pop (coordinates **s, gint8 *x, gint8 *y)
{
	(*s)--;
	*x = (*s)->x;
	*y = (*s)->y;
}

void find_connected_component (int x, int y)
{
	coordinates *stack;
	coordinates *list;
	guchar colour;
	game_cell *c;
	gint8 i, j;
	int k;

	c = board;
	for (k=0; k<board_width*board_height; k++) {
		c->visited = 0;
		c++;
	}

	count = 0;

	c = get_game_cell (x, y);
	colour = c->colour;
	if (c->colour == NONE)
		return;

	list = selected;

	stack = findstack;
	push (&stack, x, y);

	while (stack > findstack) {
		pop (&stack, &i, &j);
		c = get_game_cell (i, j);
		if ((!c->visited) && (c->colour == colour)) {
			push (&list, i, j);
			count++;
			push (&stack, i - 1, j);
			push (&stack, i + 1, j);
			push (&stack, i, j - 1);
			push (&stack, i, j + 1);
		}
		c->visited = 1; 
	}

	game_state = GAME_SELECTED;

	/* FIXME: Update the "x balls selected (n points)" message. */
}

void destroy_balls (void)
{
	gint i;
	gint8 x, y;
	coordinates *list;
	game_cell *p;

	if ((game_state != GAME_SELECTED) || (count <= 1))
		return;

	g_print ("Destroying\n");

	list = selected + count;

	for (i=0; i<count; i++) {
		pop (&list, &x, &y);
		p = get_game_cell (x, y);
		p->style = ANI_DESTROY;
		g_print ("<%d, %d>\n", x, y);
	}

	game_state = GAME_DESTROYING;

	score += (count - 2)*(count - 2);
	show_score (score);

	count = 0;

	/* The end of game check is not triggered here, we want the balls 
	 * to settle first. It is called from the animation code. */
}

gint mark_falling_balls (void)
{
	gint i, j;
	game_cell *p, *q;
	gint count;

	/* Scan up from the second to last row. */
	p = board + (board_height-1)*board_width - 1;
	count = 0;
	for (j=0; j<(board_height-1); j++)
		for (i=0; i<board_width; i++) {
			q = p + board_width;
			if ((q->colour == NONE) && (p->colour != NONE)) {
				q->colour = p->colour;
				q->frame = MOVEDFRAMESOFS;
				q->style = ANI_MOVE_DOWN;

				p->colour = NONE;
				p->frame = MOVEDFRAMESOFS;
				p->style = ANI_MOVE_DOWN;

				count++;
			}
			p--;
		}

	return count;
}

gboolean mark_shifting_balls (void)
{
	gint i, j, k;
	game_cell *p, *q;

	/* We scan the last row to determine where to start moving balls.
	 * Once we find an empty cell beside a full cell we start moving.
	 * i.e. we collapse from the right. */

	p = board + board_height*board_width - 2;
	k = board_width - 2;
	for (i=0; i<(board_width-1); i++) {
		q = p + 1;
		if ((p->colour == NONE) && (q->colour != NONE))
			break;
		k--;
		p--;
	}

	if (k < 0)
		return FALSE;

	for (j=0; j<board_height; j++) {
		p = board + j*board_width + k;
		for (i=k; i<board_width - 1; i++) {
			q = p + 1;
			p->colour = q->colour;
			p->frame = MOVELFRAMESOFS;
			p->style = ANI_MOVE_LEFT;
			p++;
		}
		p->colour = NONE;
		p->frame = MOVELFRAMESOFS;
		p->style = ANI_MOVE_LEFT;
	}

	return TRUE;
}

static void game_over (void)
{
	/* FIXME: High score stuff. */
	game_over_dialog ();
}

void end_of_game_check (void)
{
	int i,j;
	game_cell *p, *q;

	/* Check to see if the board has been cleared: that earns a 1000pt bonus */
	p = board + (board_height - 1)*board_width;
	if (p->colour == NONE) {
		score += 1000;
		show_score (score);
		game_over ();
		return;
	}

	/* Look for horizontal neighbours. */
	for (j=0; j<board_height; j++) {
		p = board + j*board_width;
		q = p + 1;
		for (i=0; i<board_width-1; i++) {
			if ((p->colour != NONE) && (p->colour == q->colour))
				return;
			p++;
			q++;
		}
	}

	/* Look for vertical neighbours. */
	for (j=0; j<board_height-1; j++) {
		p = board + j*board_width;
		q = p + board_width;
		for (i=0; i<board_width; i++) {
			if ((p->colour != NONE) && (p->colour == q->colour))
				return;
			p++;
			q++;
		}
	}
	
	game_over ();

}

void new_game (void)
{
	int i,j,c,l;
	game_cell *p;

	/* Reallocate the memory for the board if necessary. */
	if ((board_height != last_board_height) ||
			(board_width != last_board_width)) {
		if (board != NULL)
			g_free (board);
		if (findstack != NULL)
			g_free (findstack);
		if (selected != NULL)
			g_free (selected);
		board = g_new0 (game_cell, board_width*board_height);
		/* FIXME: *4 is way overkill, but may be the only way to be sure. */
		findstack = g_malloc (sizeof(coordinates)*((board_width
													*board_height)*4));
		selected = g_malloc (sizeof(coordinates)*((board_width
												 *board_height)/ncolours + 1));
		last_board_width = board_width;
		last_board_height = board_height;
	}

	/* FIXME: Reset and reallocate the memory for the undo queue. */

	/* Allocate equal numbers of each colour across the board. */	
	p = board;
	c = 0;
	for (j=0; j<board_height; j++)
		for (i=0; i<board_width; i++) {
			p->colour = c;
			c = (c + 1) % ncolours;
			p->frame = 0;
			p->style = ANI_STILL;
			p++;
		}

	/* Randomise the colours. */
	l = board_width*board_height;
	for (i=0; i<l; i++) {
		j = g_random_int_range (0, l);
		c = board[j].colour;
		board[j].colour = board[i].colour;
		board[i].colour = c;
	}

	game_state = GAME_IDLE;
	score = 0;
	show_score (score);
	
	redraw ();
}
