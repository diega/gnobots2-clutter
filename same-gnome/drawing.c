/* -*- mode: C; indent-tabs-mode: t; tab-width: 2; c-basic-offset: 2; -*- */

/* drawing.c: Code for drawing the game (well, duh).
 *
 * Copyright (c) 2004 by Callum McKenzie
 *
 */

#include <gnome.h>

#include "globals.h"

#include "drawing.h"
#include "game.h"
#include "input.h"

/* Record offsets for the moving tiles to avoid any possibility of
 * error due to rounding effects and havint to do cut and paste math
 * routines. */
guint downoffsets[NFRAMESMOVED];
guint leftoffsets[NFRAMESMOVEL];

/* Tiles are guaranteed to be square. */
gint tile_size;

/* This is the timeout for resize operations. */
guint resize_timeout_id = 0;
/* This keeps track of the idle handler for redrawing pixmaps. */
guint idle_id = 0;

/* A flag for whether we have valid pixmap data. */
gboolean pixmaps_ready = FALSE;

/* FIXME: How do we initialise these to NULL ? */
GdkPixmap *pixmaps[MAX_COLOURS][NFRAMES];
GdkPixmap *blank_pixmap = NULL;

/* FIXME: Can we make this not global ? (redraw is the problem) */
GtkWidget *canvaswidget;

gint drawing_area_width = 0;;

gint cursor_x = -1;
gint cursor_y = -1;
gboolean draw_cursor = FALSE;

void pixels_to_logical (gint px, gint py, gint *lx, gint *ly)
{
	*lx = px / tile_size;
	*ly = py / tile_size;

	*lx = CLAMP (*lx, 0, board_width - 1);
	*ly = CLAMP (*ly, 0, board_height - 1);
}

void redraw (void)
{
	gtk_widget_queue_draw (canvaswidget);
}

/* Note the dy is _subtracted_ from y and dx is _added_ to x.
 * This is the natural way given the directions the balls fall. */
static void draw_ball_with_offset (GtkWidget * canvas, game_cell *p, int x, 
																	 int y, int dx, int dy)
{
	int colour, frame;
	GdkPixmap *pixmap;

	colour = p->colour;
	frame = p->frame;

	if (colour == NONE) {
		/* We don't add an offset to the blank tile since it is part of 
		 * the background. The background movement is compensated for when
		 * the other tiles were created. */
		gdk_draw_drawable (canvas->window, canvas->style->black_gc,
											 blank_pixmap, 0, 0, x*tile_size, 
											 y*tile_size, tile_size, tile_size);	
	} else {
		pixmap = pixmaps[colour][frame];
		gdk_draw_drawable (canvas->window, canvas->style->black_gc,
											 pixmap, 0, 0, x*tile_size + dx, 
											 y*tile_size - dy, tile_size, tile_size);	
	}

	if (draw_cursor && (x == cursor_x) && (y == cursor_y)) {
		/* FIXME: This needs a better gc (and a colour to match the grid). */
		gdk_draw_rectangle (canvaswidget->window, canvaswidget->style->white_gc,
												FALSE, x*tile_size + 1, y*tile_size + 1, 
												tile_size - 2, tile_size - 2);
	}

}

static void draw_ball (GtkWidget * canvas, game_cell *p, int x, int y)
{
	draw_ball_with_offset (canvas, p, x, y, 0, 0);
}

void cursor_erase (void)
{
	draw_cursor = FALSE;
	draw_ball (canvaswidget, get_game_cell (cursor_x, cursor_y), 
						 cursor_x, cursor_y);
}

void cursor_draw (gint x, gint y)
{
	draw_cursor = TRUE;
	cursor_x = x;
	cursor_y = y;
	draw_ball (canvaswidget, get_game_cell (x, y), x, y);
}

gboolean expose_cb (GtkWidget *canvas, GdkEventExpose *event, gpointer data)
{
	int x, y;
	game_cell *p;

	/* We consider two cases. One is here we have resized and the pixmaps
	 * aren't ready yet, in that case (the else block) we just draw the
	 * grid. Otherwise we draw everything using the tiles. */

	if (pixmaps_ready) {

		p = board;
		for (y = 0; y<board_height; y++) {
			for (x = 0; x<board_width; x++) {
				draw_ball (canvas, p, x, y);
				p++;
			}
		}

		/* FIXME: These checks are wrong. */
		if ((event->area.y + event->area.height) == board_height) {
			/* FIXME: This should have it's own gc. */
			gdk_draw_line (canvas->window, canvas->style->white_gc,
										 event->area.x, board_height, 
										 event->area.x + event->area.width, board_height);
		}
		if ((event->area.x + event->area.width) == board_width) {
			/* FIXME: This should have it's own gc. */
			gdk_draw_line (canvas->window, canvas->style->white_gc,
										 board_width, event->area.y,
										 board_width, event->area.y + event->area.height);
		}

	} else { /* Draw only the grid. */

		/* FIXME: This should be a background colour. */
		gdk_draw_rectangle (canvas->window, canvas->style->black_gc, TRUE,
												event->area.x, event->area.y, 
												event->area.width, event->area.height);

		/* Vertical lines. */
		for (x = tile_size*(event->area.x/tile_size);
				 x <= tile_size*((event->area.x + event->area.width)/tile_size);
				 x += tile_size) {
			/* FIXME: This should have it's own gc. */
			gdk_draw_line (canvas->window, canvas->style->white_gc, x,
										 event->area.y, x, event->area.y + event->area.height);
		}

		/* Horizontal lines. */
		for (x = tile_size*(event->area.y/tile_size);
				 x <= tile_size*((event->area.y + event->area.height)/tile_size);
				 x += tile_size) {
			/* FIXME: This should have it's own gc. */
			gdk_draw_line (canvas->window, canvas->style->white_gc,
										 event->area.x, x, event->area.x + event->area.width, x);
		}

	}

  return TRUE;
}

/* If the pixmaps are ready and we haven't resized for a while,
 * then draw in the pixmaps. */
static gboolean redraw_cb (GtkWidget *canvas)
{
	if (!pixmaps_ready)
		return TRUE;

	gtk_widget_queue_draw (canvas);

	resize_timeout_id = 0;

	return FALSE;
}

/* FIXME: This should be a two-stage thing. There should be a flag to say, 
 * "I've done the static balls" so they can be drawn and then another
 * goes up when the animations are rendered so the animation can start.
 * Admittedly I think the bottleneck may all be in the self-imposed
 * delay: measure ! */

/* Draw the pixmaps one at a time until they are all done. 
 * We employ a simple state machine to track what has to be done
 * between calls. This operates from the idle handler.  */

/* FIXME: Too much detail in one function. */
static gboolean render_cb (GtkWidget *canvas)
{
	static enum {INIT, DRAW, DEST, MOVED, MOVEL, DONE} idle_state = INIT;
	static GdkPixbuf *file_pixbuf = NULL;
	static GdkPixbuf *bg_pixbuf = NULL;
	static gint last_tile_size = 0;
	static gint ftile_size;
	static gint n, m, x;
	GdkPixbuf *tile;
	int i,j,l;
	guchar *p, *bp;
	guint shift;
	gchar *filename;

	if (idle_state == INIT) {
		last_tile_size = tile_size;
		ftile_size = tile_size - 1;
		n = 0; 
		m = 0;		

		if (file_pixbuf != NULL)
			g_object_unref (file_pixbuf);
		if (bg_pixbuf != NULL)
			g_object_unref (bg_pixbuf);

		/* Draw up the background with the top and left grid lines. */
		/* FIXME: hard-coded colours. */
		bg_pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, tile_size,
																tile_size);
		/* FIXME: This is too large for this function, ship off else-where. */
		p = gdk_pixbuf_get_pixels (bg_pixbuf);
		l = gdk_pixbuf_get_rowstride (bg_pixbuf);
		for (i=0; i<tile_size; i++) {
			*p++ = 0xff;
			*p++ = 0xff;
			*p++ = 0xff;
		}
		for (j=1; j<tile_size; j++) {
					p = p + l - 3*tile_size;
					*p++ = 0xff;
					*p++ = 0xff;
					*p++ = 0xff;
					for (i=1; i<tile_size; i++) {
						*p++ = 0x00;
						*p++ = 0x00;
						*p++ = 0x00;
					}
		}
		if (blank_pixmap != NULL)
			g_object_unref (blank_pixmap);
		blank_pixmap = gdk_pixmap_new (canvas->window, tile_size, tile_size, -1);
		gdk_draw_pixbuf (blank_pixmap, canvas->style->black_gc, bg_pixbuf,
										 0, 0, 0, 0, tile_size, tile_size, GDK_RGB_DITHER_NORMAL,
										 0, 0);

		/* FIXME: We should also look in the users home directory. */
		filename = g_build_filename (THEMEDIR, theme, NULL);
		if (!g_file_test (filename, G_FILE_TEST_EXISTS)) {
			g_free (filename);
			filename = g_build_filename (THEMEDIR, DEFAULT_THEME, NULL);			
		}

		file_pixbuf = gdk_pixbuf_new_from_file_at_size (filename,
																										ftile_size*NFRAMESSPIN,
																										ftile_size*MAX_COLOURS,
																										NULL);
		g_free (filename);

		idle_state = DRAW;
		/* FIXME: No fallback if we fail to open the file. */
	} else if (idle_state == DRAW) {
		if (last_tile_size != tile_size) {
			idle_state = INIT;
		} else {
			if (pixmaps[n][m] != NULL)
				g_object_unref (pixmaps[n][m]);

			tile = gdk_pixbuf_copy (bg_pixbuf);
			gdk_pixbuf_composite (file_pixbuf, tile, 1, 1, ftile_size,
														ftile_size, 1.0 - m*ftile_size, 
														1.0 - n*ftile_size, 
														1.0, 1.0, GDK_INTERP_TILES, 255); 

			pixmaps[n][m] = gdk_pixmap_new (canvas->window, tile_size, tile_size, 
																			-1);
			gdk_draw_pixbuf (pixmaps[n][m], canvas->style->black_gc, tile,
											 0, 0, 0, 0, tile_size, tile_size,
											 GDK_RGB_DITHER_NORMAL, 0,  0);

			g_object_unref (tile);

			m++;
			if (m == NFRAMESSPIN) {
				m = 0;
				n++;
				if (n == MAX_COLOURS) {
					idle_state = DEST; 
					n = 0;
					m = DESTFRAMESOFS;
				}
			}
		}
	} else if (idle_state == DEST) {
		if (last_tile_size != tile_size) {
			idle_state = INIT;
		} else {
			if (pixmaps[n][m] != NULL)
				g_object_unref (pixmaps[n][m]);

			tile = gdk_pixbuf_copy (bg_pixbuf);

			/* FIXME: Split out ? */
			/* FIXME: Should be some sort of spinny stuff. */
			/* Instead we have a fade-out. */
			/* This method does avoid the "phase" problem of destroying
			 * a spinning object at an arbitrary phase. */
			/* FIXME: Should this be the border colour fading to the
			 * background colour rather than the current hard-coded stuff ? */
			bp = gdk_pixbuf_get_pixels (tile);
			l = gdk_pixbuf_get_rowstride (tile);
			/* Note that this avoids the border. */
			shift = (m - DESTFRAMESOFS)/2;
			for (j=1; j<tile_size; j++) {
				p = bp + j*l + 3;
				for (i=1; i<tile_size; i++) {
					*p = 0xff >> shift;
					p++;
					*p = 0xff >> shift;
					p++;
					*p = 0xff >> shift;
					p++;
				}
			}

			pixmaps[n][m] = gdk_pixmap_new (canvas->window, tile_size, tile_size, 
																			-1);
			gdk_draw_pixbuf (pixmaps[n][m], canvas->style->black_gc, tile,
											 0, 0, 0, 0, tile_size, tile_size,
											 GDK_RGB_DITHER_NORMAL, 0,  0);

			g_object_unref (tile);

			m++;
			if (m == MOVEDFRAMESOFS) {
				m = DESTFRAMESOFS;
				n++;
				if (n == MAX_COLOURS) {
					idle_state = MOVED;
					n = 0;
					m = MOVEDFRAMESOFS;
				}
			}
		}
	} else if (idle_state == MOVED) {
		if (last_tile_size != tile_size) {
			idle_state = INIT;
		} else {
			if (pixmaps[n][m] != NULL)
				g_object_unref (pixmaps[n][m]);

			x = m - MOVEDFRAMESOFS;
			tile = gdk_pixbuf_copy (bg_pixbuf);

			/* FIXME: Split out ? */
			/* No phase problems here. The falling stuff always has a phase of 0 */
			/* FIXME: Once again, hard-coded bg/fg colours. */
			bp = gdk_pixbuf_get_pixels (tile);
			l = gdk_pixbuf_get_rowstride (tile);
			p = bp + 3;
			/* Remove the horizontal line... */
			for (i=1; i<tile_size; i++) {
				*p++ = 0x00;
				*p++ = 0x00;
				*p++ = 0x00;
			}
			j = tile_size - 1 - ((x + 1)*(tile_size - 1))/(NFRAMESMOVED);
			downoffsets[x] = j;
			p = bp + j*l + 3;
			/* ...and redraw it in the correct place. */
			for (i=1; i<tile_size; i++) {
				*p++ = 0xff;
				*p++ = 0xff;
				*p++ = 0xff;
			}

			gdk_pixbuf_composite (file_pixbuf, tile, 1, 1, ftile_size,
														ftile_size, 1.0, 1.0 - n*ftile_size, 
														1.0, 1.0, GDK_INTERP_TILES, 255); 

			pixmaps[n][m] = gdk_pixmap_new (canvas->window, tile_size, tile_size, 
																			-1);
			gdk_draw_pixbuf (pixmaps[n][m], canvas->style->black_gc, tile,
											 0, 0, 0, 0, tile_size, tile_size,
											 GDK_RGB_DITHER_NORMAL, 0,  0);

			g_object_unref (tile);

			m++;
			if (m == MOVELFRAMESOFS) {
				m = MOVEDFRAMESOFS;
				n++;
				if (n == MAX_COLOURS) {
					idle_state = MOVEL;
					n = 0;
					m = MOVELFRAMESOFS;
				}
			}
		}
	} else if (idle_state == MOVEL) {
		if (last_tile_size != tile_size) {
			idle_state = INIT;
		} else {
			if (pixmaps[n][m] != NULL)
				g_object_unref (pixmaps[n][m]);

			x = m - MOVELFRAMESOFS;
			tile = gdk_pixbuf_copy (bg_pixbuf);

			/* FIXME: Split out ? */
			/* No phase problems here. The falling stuff always has a phase of 0 */
			/* FIXME: Once again, hard-coded bg/fg colours. */
			bp = gdk_pixbuf_get_pixels (tile);
			l = gdk_pixbuf_get_rowstride (tile);
			p = bp + l;
			/* Remove the vertical line... */
			for (i=1; i<tile_size; i++) {
				*p++ = 0x00;
				*p++ = 0x00;
				*p   = 0x00;
				p   += l - 2;
			}
			j = ((x + 1)*(tile_size - 1))/(NFRAMESMOVED);
			leftoffsets[x] = tile_size - j;
			p = bp + l + 3*j;
			/* ...and redraw it in the correct place. */
			for (i=1; i<tile_size; i++) {
				*p++ = 0xff;
				*p++ = 0xff;
				*p   = 0xff;
				p   += l - 2;
			}

			gdk_pixbuf_composite (file_pixbuf, tile, 1, 1, ftile_size,
														ftile_size, 1.0, 1.0 - n*ftile_size, 
														1.0, 1.0, GDK_INTERP_TILES, 255); 

			pixmaps[n][m] = gdk_pixmap_new (canvas->window, tile_size, tile_size, 
																			-1);
			gdk_draw_pixbuf (pixmaps[n][m], canvas->style->black_gc, tile,
											 0, 0, 0, 0, tile_size, tile_size,
											 GDK_RGB_DITHER_NORMAL, 0,  0);

			g_object_unref (tile);

			m++;
			if (m == NFRAMES) {
				m = MOVELFRAMESOFS;
				n++;
				if (n == MAX_COLOURS) {
					idle_state = DONE;
					pixmaps_ready = TRUE;
				}
			}
		}
	} else if (idle_state == DONE) {
		g_object_unref (file_pixbuf);
		file_pixbuf = NULL;
		g_object_unref (bg_pixbuf);
		bg_pixbuf = NULL;
		idle_state = INIT;

		if (pixmaps_ready) /* Just in case this was reset after the last idle call. */
			return FALSE;
	}
	return TRUE;
}

void resize_graphics (void)
{
	int size;

	size = drawing_area_width/board_width;

	if (tile_size != size) {

		tile_size = size;

		pixmaps_ready = FALSE;

		if (idle_id == 0)
			g_idle_add ((GSourceFunc)render_cb, canvaswidget);

		if (resize_timeout_id != 0)
			g_source_remove (resize_timeout_id);
		resize_timeout_id = g_timeout_add (300, (GSourceFunc)redraw_cb, 
																			 canvaswidget);
	}
}

gboolean configure_cb (GtkWidget *canvas, GdkEventConfigure *event)
{
	canvaswidget = canvas;
	drawing_area_width = event->width;
	resize_graphics ();

  return FALSE;
}

static gboolean animation_timer (void)
{
	static float speed = 1.0;
	int i,j;
	game_cell *p;
	gboolean changestatep;
	
	if (!pixmaps_ready)
		return TRUE;

	changestatep = FALSE;

	for (j=0; j<board_height; j++) {
		/* FIXME: Could we rearrange the board structure so it
		 * runs from right to left? Making this simpler. */
		p = board + (j+1)*board_width - 1;
		for (i=board_width-1; i>=0; i--) {
			switch (p->style) {
			case ANI_STILL:
				if (p->frame != 0) 
					p->style = ANI_SPINBACK;
				break;
			case ANI_REDRAW:
				draw_ball (canvaswidget, p, i, j);
				p->style = ANI_STILL;
				break;
			case ANI_SPIN:
				p->frame = (p->frame + 1) % NFRAMESSPIN;
				draw_ball (canvaswidget, p, i, j);
				break;
			case ANI_SPINBACK:
				if (p->frame > 4)
					p->frame = 4;
				if (p->frame == 0)
					p->style = ANI_STILL;
				else
					p->frame--;
				draw_ball (canvaswidget, p, i, j);
				break;				
			case ANI_DESTROY:
				p->frame++;
				if (p->frame < DESTFRAMESOFS) {
					p->frame = DESTFRAMESOFS;
				} else if (p->frame < MOVEDFRAMESOFS) {

				} else {
					p->colour = NONE;
					changestatep = TRUE;
					p->frame = 0;
					p->style = ANI_STILL;
				}
				draw_ball (canvaswidget, p, i, j);
				break;				
			case ANI_MOVE_DOWN:
				if (game_state == GAME_MOVING_DOWN) {
					p->frame += speed;
					if (p->frame >= MOVELFRAMESOFS) {
						changestatep = TRUE;
						p->frame = 0;
						p->style = ANI_STILL;
						draw_ball (canvaswidget, p, i, j);
					} else {
						draw_ball_with_offset (canvaswidget, p, i, j, 0, 
																	 downoffsets[p->frame - MOVEDFRAMESOFS]);
					}
				}
				break;
			case ANI_MOVE_LEFT:
				if (game_state == GAME_MOVING_LEFT) {
					p->frame += speed;
					if (p->frame >= NFRAMES) {
						changestatep = TRUE;
						p->frame = 0;
						p->style = ANI_STILL;
						draw_ball (canvaswidget, p, i, j);
					} else {
						draw_ball_with_offset (canvaswidget, p, i, j,
																	 leftoffsets[p->frame - MOVELFRAMESOFS], 0);
					}
				}
				break;
			}
			p--;
		}
	}

	/* Since this value is added on each cycle the effect is that
	 * the balls drop quadratically. */
	speed += ACCELERATION;

	/* Various things in the previous loop can trigger a change in
	 * game_state. However we only want to do these things once, so
	 * we flag them and check them down here once the loop has finished. */
	if (changestatep) {
		/* FIXME: Is this really the place to change the game state ?
		 * We do need to signal the end of the animation though so maybe ...
		 * or maybe we move it over to game.c 
		 * This question should be rephrased: is this really the file for
		 * this function ? */
		switch (game_state) {
		case GAME_DESTROYING:
			if (mark_falling_balls () == 0)
				if (mark_shifting_balls ()) {
					game_state = GAME_MOVING_LEFT;
					speed = 1.0;
				} else {
					end_of_move ();
				}
			else {
				game_state = GAME_MOVING_DOWN;
				speed = 1.0;
			}
			break;
			/* FIXME: Because of the way we do the drawing (with the offset)
			 * there is potential for overlap with a blank tile and hence 
			 * flicker. */
		case GAME_MOVING_DOWN:
			if (mark_falling_balls () == 0) { 
				if (mark_shifting_balls ()) {
					game_state = GAME_MOVING_LEFT;
				} else {
					end_of_move ();
					select_cells ();
				}
				speed = 1.0;
			} else {

			} 
			break;
		case GAME_MOVING_LEFT:
			if (!mark_shifting_balls ()) {
				end_of_move ();
				select_cells ();
			}
			break;
		}
	}


	return TRUE;
}

void start_spinning (void)
{
	int i;
	game_cell *p;
	coordinates *list;
	
	/* FIXME: Is this bit of policy distributed too far ? */
	if (count <= 1)
		return;

	list = selected;
	for (i=0; i<count; i++) {
		p = get_game_cell (list->x, list->y);
		p->style = ANI_SPIN;
		p->frame = 0;
		list++;
	}
}

void stop_spinning (void)
{
	int i;
	game_cell *p;
	coordinates *list;
	
	list = selected;
	for (i=0; i<count; i++) {
		p = get_game_cell (list->x, list->y);
		p->style = ANI_SPINBACK;
		list++;
	}
}

/* FIXME: Do we need the explicit initialisation function ? */
void init_pixmaps (void)
{
	int n,m;

	/* 16 frames/second. */
	g_timeout_add (62, (GSourceFunc)animation_timer, NULL);

	for (n=0; n<MAX_COLOURS; n++)
		for (m=0; m<NFRAMES; m++)
			pixmaps[n][m] = NULL;
}
