/*
 * Gnome-Mahjonggg
 * (C) 1998 the Free Software Foundation
 *
 *
 * Author: Francisco Bustamante
 *
 *
 * http://www.nuclecu.unam.mx/~pancho/
 * pancho@nuclecu.unam.mx
 *
 */

#include <config.h>

#include <gtk/gtk.h>
#include <gnome.h>

#include "vborder.xpm"
#include "hborder.xpm"

#define GAME_EVENTS (GDK_EXPOSURE_MASK         |\
                     GDK_BUTTON_PRESS_MASK)

#define TILE_SIZE 40
#define MAX_TILES 146
#define HALF_TILE 20
#define MAH_VERSION "0.0.1 Pre Alpha"
     
typedef struct _tilepos tilepos;     
typedef struct _tile tile;
typedef struct _tiletypes tiletypes;

struct _tiletypes {
	int type;
	int num_tiles;
	int image;
};

tiletypes default_types [] = {
	{0, 4, 0},
	{1, 4, 1},
	{2, 4, 2},
	{3, 4, 3},
	{4, 4, 4},
	{5, 4, 5},
	{6, 4, 6},
	{7, 4, 7},
	{8, 4, 8},
	{9, 4, 9},
	{10, 4, 10},
	{11, 4, 11},
	{12, 4, 12},
	{13, 4, 13}, 
	{14, 4, 14}, 
	{15, 4, 15}, 
	{16, 4, 16}, 
	{17, 4, 17}, 
	{18, 4, 18}, 
	{19, 4, 19}, 
	{20, 4, 20},
	{21, 4, 21}, 
	{22, 4, 22}, 
	{23, 4, 23}, 
	{24, 4, 24}, 
	{25, 4, 25}, 
	{26, 4, 26}, 
	{27, 4, 27}, 
	{28, 4, 28}, 
	{29, 4, 29}, 
	{30, 4, 30}, 
	{31, 4, 31},
	{32, 1, 32}, 
	{32, 1, 33}, 
	{32, 1, 34}, 
	{32, 1, 35},
	{33, 4, 36}, 
	{34, 4, 37}, 
	{35, 1, 38}, 
	{35, 1, 39}, 
	{35, 1, 40}, 
	{35, 1, 41},	
};

struct _tilepos {
	int x;
	int y;
	int layer;
};

tilepos default_pos [] = {
	{120, 0, 0},
	{160, 0, 0},
	{200, 0, 0},
	{240, 0, 0},
	{280, 0, 0},
	{20, 20, 0},
	{60, 20, 0},
	{20, 60, 0},
	{60, 60, 0},
	{0, 120, 0},
	{40, 100, 0},
	{40, 140, 0},
	{20, 180, 0},
	{60, 180, 0},
	{20, 220, 0},
	{60, 220, 0},
	{120, 240, 0},
	{160, 240, 0},
	{200, 240, 0},
	{240, 240, 0},
	{280, 240, 0},
	{340, 20, 0},
	{380, 20, 0},
	{340, 60, 0},
	{380, 60, 0},
	{340, 180, 0},
	{380, 180, 0},
	{340, 220, 0},
	{380, 220, 0},
	{400, 120, 0},
	{360, 140, 0},
	{360, 100, 0},
	{100, 40, 0},
	{140, 40, 0},
	{180, 40, 0},
	{220, 40, 0},
	{260, 40, 0},
	{300, 40, 0},
	{100, 200, 0},
	{140, 200, 0},
	{180, 200, 0},
	{220, 200, 0},
	{260, 200, 0},
	{300, 200, 0},
	{80, 100, 0},
	{80, 140, 0},
	{320, 100, 0},
	{320, 140, 0},
	{120, 80, 0},
	{160, 80, 0},
	{200, 80, 0},
	{240, 80, 0},
	{280, 80, 0},
	{120, 120, 0},
	{160, 120, 0},
	{200, 120, 0},
	{240, 120, 0},
	{280, 120, 0},
	{120, 160, 0},
	{160, 160, 0},
	{200, 160, 0},
	{240, 160, 0},
	{280, 160, 0},
	{40, 40, 1},
	{80, 40, 1},
	{120, 40, 1},
	{140, 0, 1},
	{180, 20, 1},
	{220, 20, 1},
	{260, 0, 1},
	{280, 40, 1},
	{320, 40, 1},
	{360, 40, 1},
	{40, 200, 1},
	{80, 200, 1},
	{120, 200, 1},
	{140, 240, 1},
	{180, 220, 1},
	{220, 220, 1},
	{260, 240, 1},
	{280, 200, 1},
	{320, 200, 1},
	{360, 200, 1},
	{160, 60, 1},
	{200, 60, 1},
	{240, 60, 1},
	{140, 100, 1},
	{180, 100, 1},
	{220, 100, 1},
	{260, 100, 1},
	{140, 140, 1},
	{180, 140, 1},
	{220, 140, 1},
	{260, 140, 1},
	{160, 180, 1},
	{200, 180, 1},
	{240, 180, 1},
	{60, 80, 1},
	{60, 120, 1},
	{60, 160, 1},
	{100, 80, 1},
	{100, 120, 1},
	{100, 160, 1},
	{300, 80, 1},
	{300, 120, 1},
	{300, 160, 1},
	{340, 80, 1},
	{340, 120, 1},
	{340, 160, 1},
	{100, 80, 2},
	{140, 80, 2},
	{180, 80, 2},
	{220, 80, 2},
	{260, 80, 2},
	{300, 80, 2},
	{100, 160, 2},
	{140, 160, 2},
	{180, 160, 2},
	{220, 160, 2},
	{260, 160, 2},
	{300, 160, 2},
	{80, 120, 2},
	{120, 120, 2},
	{160, 120, 2},
	{200, 120, 2},
	{240, 120, 2},
	{280, 120, 2},
	{320, 120, 2},
	{140, 100, 3},
	{180, 100, 3},
	{220, 100, 3},
	{260, 100, 3},
	{140, 140, 3},
	{180, 140, 3},
	{220, 140, 3},
	{260, 140, 3},
	{100, 120, 3},
	{300, 120, 3},
	{160, 120, 4},
	{200, 120, 4},
	{240, 120, 4},
	{180, 120, 5},
	{220, 120, 5},
	{200, 120, 6}
};

struct _tile{
	int type;
	int image;
	int layer;
	int x;
	int y;
	int visible;
	int selected;
};

GtkWidget *window;
GtkWidget *mbox;
GtkWidget *draw_area;
GdkPixmap *tiles_pix, *mask, *vborderpic, *hborderpic;
tile tiles[MAX_TILES];
int selected_tile, visible_tiles;

void quit_game_callback (GtkWidget *widget, gpointer data);
void new_game_callback (GtkWidget *widget, gpointer data);
void new_game (void);
void redraw_area (int x1, int y1, int x2, int y2, int mlayer);
void about_callback (GtkWidget *widget, gpointer data);

GnomeUIInfo filemenu [] = {
	{GNOME_APP_UI_ITEM, N_("New"), NULL, new_game_callback,
	 GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_NEW, 0, 0, NULL},
	{GNOME_APP_UI_ITEM, N_("Exit"), NULL, quit_game_callback,
	 GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_EXIT, 0, 0, NULL},
	{GNOME_APP_UI_ENDOFINFO, NULL, NULL, NULL,
	 GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
};

GnomeUIInfo helpmenu[] = {
	{GNOME_APP_UI_HELP, NULL, NULL, NULL,
	 GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
	{GNOME_APP_UI_ITEM, N_("About"), NULL, about_callback,
	 GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_ABOUT, 0, 0, NULL},
	{GNOME_APP_UI_ENDOFINFO, NULL, NULL, NULL,
	 GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
};

GnomeUIInfo mainmenu [] = {
	{GNOME_APP_UI_SUBTREE, N_("Game"), NULL, filemenu,
	 GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
	{GNOME_APP_UI_SUBTREE, N_("Help"), NULL, helpmenu,
	 GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
	{GNOME_APP_UI_ENDOFINFO, NULL, NULL, NULL,
	 GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
};

int find_tile_in_layer (int x, int y, int layer)
{
	int i, tile_num = MAX_TILES + 1;

	for (i = 0; i < MAX_TILES; i ++) {
		if ((tiles[i].x < x) && ((tiles[i].x + TILE_SIZE) > x)) {
			if ((tiles[i].y < y) && ((tiles[i].y + TILE_SIZE) > y)) {
				if ((tiles[i].layer == layer) && (tiles[i].visible == 1))
					tile_num = i;
			}
		}
	}
	return tile_num;
}
	
int find_tile (int x, int y)
{
	int i, tile_num = MAX_TILES + 1, layer = 0, temp_tile;

	for (i = 0; i < MAX_TILES; i++) {
		if ((tiles[i].x < x) && ((tiles[i].x + TILE_SIZE) > x) && (tiles[i].visible)) {
			if ((tiles[i].y < y) && ((tiles[i].y + TILE_SIZE) > y)) {
				if ((tiles[i].layer >= layer) && (tiles[i].visible == 1)) {
					tile_num = i;
					layer = tiles[i].layer;
				}
			}
		}
	}
	return tile_num;
}

void draw_selected_tile (int i)
{
	int bx, by, dx1 = 0, dx2 = 0, dy1 = 0, dy2 = 0, bborder = 0;
	int layer, y;
	int tile_h, tile_w, x_pos, y_pos, x_end, y_end;
	int uleft, bleft, lbottom, rbottom;

	if (tiles[i].visible == 1) {
		uleft = find_tile (tiles[i].x + 1, tiles[i].y + 1);
		if (uleft != i)
			bleft = find_tile (tiles[i].x + 1, tiles[uleft].y + TILE_SIZE);
		else 
			bleft = find_tile (tiles[i].x + 1, tiles[i].y + HALF_TILE);
		if (bleft != i)
			lbottom = find_tile (tiles[bleft].x + TILE_SIZE + 1, tiles[i].y + TILE_SIZE - 1);
		else lbottom = find_tile (tiles[i].x + 10, tiles[i].y + TILE_SIZE - 1);
		if (lbottom != i) {
			rbottom = find_tile (tiles[lbottom].x + TILE_SIZE + 1, tiles[i].y + TILE_SIZE - 1);
			if ((tiles[lbottom].x + TILE_SIZE) > tiles[i].x + TILE_SIZE)
				rbottom = i;
		}
		else rbottom = find_tile (tiles[i].x + TILE_SIZE - 1, tiles[i].y + TILE_SIZE - 2);


		if (uleft != i) dx1 = tiles[uleft].x + TILE_SIZE - tiles[i].x;
		if (bleft != i) dx2 = tiles[bleft].x + TILE_SIZE - tiles[i].x;
		if (lbottom != i) dy1 = TILE_SIZE + tiles[i].y - tiles[lbottom].y;
		if (rbottom != i) dy2 = TILE_SIZE + tiles[i].y - tiles[rbottom].y;

		y = tiles[i].y;
		bx = (tiles[i].image % 21) * TILE_SIZE;
		by = (tiles[i].image / 21) * TILE_SIZE;
		if (tiles[i].selected) by += 80;

		if ((dx1 == 0) && (dx2 == 0) && (dy1 == 0) && (dy2 == 0)) {
			gdk_draw_pixmap (draw_area->window,
					 draw_area->style->black_gc, tiles_pix,
					 bx, by, tiles[i].x, tiles[i].y,
					 TILE_SIZE, TILE_SIZE);
		}
		else {
			bborder = (dy1 > dy2) ? dy1 : dy2;
			if (dx1 != 0) {
				x_pos = tiles[uleft].x + TILE_SIZE;
				y_pos = tiles[i].y;
				x_end = tiles[i].x + TILE_SIZE;
				y_end = tiles[uleft].y + TILE_SIZE;
				tile_w = x_end - x_pos;
				tile_h = y_end - y_pos + 1; 
				gdk_draw_pixmap (draw_area->window,
						 draw_area->style->black_gc, tiles_pix,
						 bx + x_pos - tiles[i].x, by + y_pos - tiles[i].y,
						 x_pos, y_pos, tile_w, tile_h);
				y = y_end + 1;
			}
			else if (dx2 != 0) {
				x_pos = tiles[i].x;
				y_pos = tiles[i].y;
				x_end = tiles[i].x + TILE_SIZE;
				y_end = tiles[bleft].y - 1;
				tile_w = x_end - x_pos;
				tile_h = y_end - y_pos + 1;
				gdk_draw_pixmap (draw_area->window,
						 draw_area->style->black_gc, tiles_pix,
						 bx + x_pos - tiles[i].x, by + y_pos - tiles[i].y,
						 x_pos, y_pos, tile_w, tile_h);
				y = y_end + 1;
			}
			else {
				x_pos = tiles[i].x;
				y_pos = tiles[i].y;
				x_end = tiles[i].x + TILE_SIZE;
				y_end = tiles[i].y + TILE_SIZE - bborder - 1;
				tile_w = x_end - x_pos;
				tile_h = y_end - y_pos + 1;
				gdk_draw_pixmap (draw_area->window,
						 draw_area->style->black_gc, tiles_pix,
						 bx + x_pos - tiles[i].x, by + y_pos - tiles[i].y,
						 x_pos, y_pos, tile_w, tile_h);
				y = tiles[bleft].y;
			}
			if (dx2 != 0) {
				x_pos = tiles[bleft].x + TILE_SIZE;
				y_pos = tiles[bleft].y;
				x_end = tiles[i].x + TILE_SIZE;
				if (bleft == lbottom) {
					y_end = tiles[i].y + TILE_SIZE - ((dy1 < dy2) ? dy1 : dy2) - 1;
					bborder = 0;
				}
				else y_end = tiles[i].y + TILE_SIZE - bborder - 1;
				tile_w = x_end - x_pos;
				tile_h = y_end - y_pos;
				gdk_draw_pixmap (draw_area->window,
						 draw_area->style->black_gc, tiles_pix,
						 bx + x_pos - tiles[i].x, by + y_pos - tiles[i].y,
						 x_pos, y_pos, tile_w, tile_h);
			}
			else {
				x_pos = tiles[i].x;
				y_pos = y;
				x_end = tiles[i].x + TILE_SIZE;
				y_end = tiles[i].y + TILE_SIZE - bborder - 1;
				tile_w = x_end - x_pos;
				tile_h = y_end - y_pos;
				gdk_draw_pixmap (draw_area->window,
						 draw_area->style->black_gc, tiles_pix,
						 bx + x_pos - tiles[i].x, by + y_pos - tiles[i].y,
						 x_pos, y_pos, tile_w, tile_h);
			}
			if (bborder > 0) {
				if (dy1 > dy2) {
					x_pos = tiles[lbottom].x;
					y_pos = tiles[lbottom].y;
					x_end = tiles[i].x + TILE_SIZE;
					y_end = tiles[i].y + TILE_SIZE;
					tile_w = x_end - x_pos;
					tile_h = y_end - y_pos;
					gdk_draw_pixmap (draw_area->window,
							 draw_area->style->black_gc, tiles_pix,
							 bx + x_pos - tiles[i].x, by + y_pos - tiles[i].y,
							 x_pos, y_pos, tile_w, tile_h);
				}
				else if (dy2 > dy1) {
					x_pos = tiles[i].x;
					y_pos = tiles[rbottom].y;
					x_end = tiles[rbottom].x - 1;
					y_end = tiles[i].y + TILE_SIZE;
					tile_w = x_end - x_pos;
					tile_h = y_end - y_pos;
					gdk_draw_pixmap (draw_area->window,
							 draw_area->style->black_gc, tiles_pix,
							 bx + x_pos - tiles[i].x, by + y_pos - tiles[i].y,
							 x_pos, y_pos, tile_w, tile_h);
				}
			}
		}
	}
}

void left_border (int tile_num)
{
	int x, y, uleft_tile, bleft_tile, uleft_free = 0, bleft_free = 0, corner_tile;
	int hcornerd = 0;
	
	x = tiles[tile_num].x + 2;
	y = tiles[tile_num].y + 2;

	uleft_tile = find_tile (x - 5, y);
	bleft_tile = find_tile (x - 5, y + HALF_TILE);
	corner_tile = find_tile_in_layer (tiles[tile_num].x - 3, y + TILE_SIZE, tiles[tile_num].layer);

	if ((uleft_tile > MAX_TILES) || (tiles[tile_num].layer > tiles[uleft_tile].layer) ||
	    (tiles[uleft_tile].visible == 0)) uleft_free = 1;
	if ((bleft_tile > MAX_TILES) || (tiles[tile_num].layer > tiles[bleft_tile].layer) ||
	    (tiles[bleft_tile].visible == 0)) bleft_free = 1;
	if ((corner_tile < MAX_TILES) && (tiles[corner_tile].visible)) hcornerd = 4;
	
	if ((uleft_free == 1) && (bleft_free == 1)) 
		gdk_draw_pixmap (draw_area->window,
				 draw_area->style->black_gc, vborderpic,
				 0, 0, tiles[tile_num].x - 5, tiles[tile_num].y,
				 5, 44 - hcornerd);
	else if ((uleft_free == 1) && (bleft_free == 0)) {
		gdk_draw_pixmap (draw_area->window,
				 draw_area->style->black_gc, vborderpic,
				 0, 0, tiles[tile_num].x - 5, tiles[tile_num].y,
				 5, HALF_TILE);
/*   		draw_selected_tile (bleft_tile);   */
	} else if ((bleft_free == 1) && (uleft_free == 0)) {
		gdk_draw_pixmap (draw_area->window,
				 draw_area->style->black_gc, vborderpic,
				 0, 0, tiles[tile_num].x - 5, tiles[tile_num].y + HALF_TILE,
				 5, 44 - HALF_TILE - hcornerd);
/*   		draw_selected_tile (uleft_tile);   */
	}
}

int bottom_border (int tile_num)
{
	int x, y, lbottom_tile, rbottom_tile, lbottom_free = 0, rbottom_free = 0, corner_tile, vcornerd = 0;

	x = tiles[tile_num].x + 2;
	y = tiles[tile_num].y + TILE_SIZE + 2;

	lbottom_tile = find_tile (x, y);
	rbottom_tile = find_tile (x + HALF_TILE, y);
	corner_tile = find_tile_in_layer (x - 5, y, tiles[tile_num].layer);

	if ((lbottom_tile > MAX_TILES) || (tiles[tile_num].layer > tiles[lbottom_tile].layer) ||
	    (tiles[lbottom_tile].visible == 0)) lbottom_free = 1;
	if ((rbottom_tile > MAX_TILES) || (tiles[tile_num].layer > tiles[rbottom_tile].layer) ||
	    (tiles[rbottom_tile].visible == 0)) rbottom_free = 1;
	if (corner_tile < MAX_TILES)
		vcornerd = 5;

	if ((lbottom_free == 1) && (rbottom_free == 1)) {
		gdk_draw_pixmap (draw_area->window,
				 draw_area->style->black_gc, hborderpic,
				 0, 0, tiles[tile_num].x - 5 + vcornerd, tiles[tile_num].y + TILE_SIZE,
				 45 - vcornerd, 4);
/*  		if ((corner_tile < MAX_TILES) || (tiles[corner_tile].visible == 1))  */
/*   			draw_selected_tile (corner_tile);   */
	}
	else if ((lbottom_free == 1) && (rbottom_free == 0)) {
		gdk_draw_pixmap (draw_area->window,
				 draw_area->style->black_gc, hborderpic,
				 0, 0, tiles[tile_num].x - 5 + vcornerd, tiles[tile_num].y + TILE_SIZE,
				 HALF_TILE + 5 - vcornerd, 4);
/*   		draw_selected_tile (rbottom_tile);   */
/*  		if ((corner_tile < MAX_TILES) || (tiles[corner_tile].visible == 1))  */
/*  			draw_selected_tile (corner_tile);		  */
	} else if ((rbottom_free == 1) && (lbottom_free == 0)) {
		gdk_draw_pixmap (draw_area->window,
				 draw_area->style->black_gc, hborderpic,
				 0, 0, tiles[tile_num].x +  HALF_TILE, tiles[tile_num].y + TILE_SIZE,
				 HALF_TILE, 4);
/*   		draw_selected_tile (lbottom_tile);   */
	}
}

int tile_free (int x, int y, int tile_num)
{
	int uleft_tile, bleft_tile, uright_tile, bright_tile, lup_tile, rup_tile, lbottom_tile, rbottom_tile;
	int tile_x, tile_y;
	int up_free = 0, bottom_free = 0, left_free = 0, right_free = 0;
	int nlayer_ul_free = 0, nlayer_ur_free = 0, nlayer_bl_free = 0, nlayer_br_free = 0, nlayer, temp_tile;

	if (tiles[tile_num].layer < 6) {
		nlayer = tiles[tile_num].layer + 1;
		tile_x = tiles[tile_num].x + 6;
		tile_y = tiles[tile_num].y - 3;
		
		temp_tile = find_tile_in_layer (tile_x, tile_y, nlayer);
		if ((temp_tile > MAX_TILES) || (tiles[temp_tile].visible == 0))
			nlayer_ul_free = 1;
		
		temp_tile = find_tile_in_layer (tile_x + HALF_TILE, tile_y, nlayer);
		if ((temp_tile > MAX_TILES) || (tiles[temp_tile].visible == 0))
			nlayer_ur_free = 1;
		
		temp_tile = find_tile_in_layer (tile_x, tile_y + HALF_TILE, nlayer);
		if ((temp_tile > MAX_TILES) || (tiles[temp_tile].visible == 0))
			nlayer_bl_free = 1;
		
		temp_tile = find_tile_in_layer (tile_x + HALF_TILE, tile_y + HALF_TILE, nlayer);
		if ((temp_tile > MAX_TILES) || (tiles[temp_tile].visible == 0))
			nlayer_br_free = 1;
	}
	else {
		nlayer_ul_free = 1;
		nlayer_ur_free = 1;
		nlayer_bl_free = 1;
		nlayer_br_free = 1;
	}
	
	tile_x = tiles[tile_num].x + 2;
	tile_y = tiles[tile_num].y + 2;

	if ((nlayer_ul_free == 1) && (nlayer_ur_free == 1) &&
	    (nlayer_bl_free == 1) && (nlayer_br_free == 1)) {
		uleft_tile = find_tile_in_layer (tile_x - HALF_TILE, tile_y, tiles[tile_num].layer);
		bleft_tile = find_tile_in_layer (tile_x - HALF_TILE,
						 tile_y + HALF_TILE, tiles[tile_num].layer);
		uright_tile = find_tile_in_layer (tile_x + TILE_SIZE, tile_y, tiles[tile_num].layer);
		bright_tile = find_tile_in_layer (tile_x + TILE_SIZE,
						  tile_y + HALF_TILE, tiles[tile_num].layer);
		rbottom_tile = find_tile_in_layer (tile_x + HALF_TILE, y + TILE_SIZE, tiles[tile_num].layer);
		lbottom_tile = find_tile_in_layer (tile_x, y + TILE_SIZE, tiles[tile_num].layer);
		rup_tile = find_tile_in_layer (tile_x + HALF_TILE, y - TILE_SIZE, tiles[tile_num].layer);
		lup_tile = find_tile_in_layer (tile_x, y - TILE_SIZE, tiles[tile_num].layer);

		if (((uleft_tile > MAX_TILES) || (tiles[uleft_tile].visible == 0)) &&
		    ((bleft_tile > MAX_TILES) || (tiles[bleft_tile].visible == 0))) left_free = 1;
		if (((uright_tile > MAX_TILES) || (tiles[uright_tile].visible == 0)) &&
		    ((bright_tile > MAX_TILES) || (tiles[bright_tile].visible == 0))) right_free = 1;
		if (((rbottom_tile > MAX_TILES) || (tiles[rbottom_tile].visible == 0)) &&
		    ((lbottom_tile > MAX_TILES) || (tiles[lbottom_tile].visible == 0))) bottom_free = 1;
		if (((rup_tile > MAX_TILES) || (tiles[rup_tile].visible == 0)) &&
		    ((lup_tile > MAX_TILES) || (tiles[lup_tile].visible == 0))) up_free = 1;

		if (bottom_free) {
			if ((left_free) || (right_free)) return 1;
		}
		else if (up_free) {
			if ((left_free) || (right_free)) return 1;
		}
		else if ((left_free) || (right_free)) return 1;
		else return 0;
	}
	else return 0;
	return 0;
}

void no_match (void)
{
	GtkWidget *mb;

	mb = gnome_messagebox_new (_("Tiles don't match!"),
				   GNOME_MESSAGEBOX_INFO,
				   _("Ok"), NULL);
	GTK_WINDOW(mb)->position = GTK_WIN_POS_MOUSE;
	gnome_messagebox_set_modal (GNOME_MESSAGEBOX (mb));
	gtk_widget_show (mb);
}

void you_won (void)
{
	GtkWidget *mb;

	mb = gnome_messagebox_new (_("You won!"),
				   GNOME_MESSAGEBOX_INFO,
				   _("Ok"), NULL);
	GTK_WINDOW(mb)->position = GTK_WIN_POS_MOUSE;
	gnome_messagebox_set_modal (GNOME_MESSAGEBOX (mb));
	gtk_signal_connect_object (GTK_OBJECT(mb),
				   "clicked",
				   GTK_SIGNAL_FUNC (new_game_callback),
				   NULL);
	gtk_widget_show (mb);
}

void about_callback (GtkWidget *widget, gpointer data)
{
	GtkWidget *about;
	gchar *authors [] = {
		"Francisco Bustamante",
		NULL
	};

	about = gnome_about_new (_("Gnome Mahjong"), MAH_VERSION,
				 "(C) 1998 The Free Software Foundation",
				 authors,
				 _("Send comments and bug reports to: pancho@nuclecu.unam.mx"),
				 NULL);
	gtk_widget_show (about);
}

void quit_game_callback (GtkWidget *widget, gpointer data)
{
	gtk_main_quit ();
}

void new_game_callback (GtkWidget *widget, gpointer data)
{
	new_game ();
}

void new_game (void)
{
	int i, f, n, col = 0, row = 0;

	visible_tiles = 144;
	for (f = 0; f < 144; f++) {
		tiles[f].visible = 0;
	}
	selected_tile = MAX_TILES + 1;
	for (i = 0; i < 42; i ++) {
		n = default_types[i].num_tiles;
		while (n > 0) {
			n --;
			f = (int) (145 * rand () / RAND_MAX) - 1;
			while (tiles[f].visible != 0)
				f = (int) (144.0 * rand () / RAND_MAX);
			tiles[f].visible = 1;
			tiles[f].selected = 0;
			tiles[f].x = default_pos[f].x + 80 + (5 * default_pos[f].layer);
			tiles[f].y = default_pos[f].y + 60 - (4 * default_pos[f].layer);
			tiles[f].layer = default_pos[f].layer;
			tiles[f].type = default_types[i].type;
			tiles[f].image = default_types[i].image;
		}
	}
	
	gtk_widget_draw (draw_area, NULL);
}

void redraw_tile_in_area (int x1, int y1, int x2, int y2, int tile)
{
	int x_pos, y_pos, x_end, y_end, orig_x, orig_y, tile_width, tile_height;
	if (tile < MAX_TILES) {
		if (tiles[tile].x <= x1) 
			x_pos = x1;
		else x_pos = tiles[tile].x;
	
		if ((tiles[tile].x + TILE_SIZE) >= x2)
			x_end = x2;
		else x_end = tiles[tile].x + TILE_SIZE;

		if (tiles[tile].y <= y1) 
			y_pos = y1;
		else y_pos = tiles[tile].y;
		if ((tiles[tile].y + TILE_SIZE) >= y2)
			y_end = y2;
		else y_end = tiles[tile].y + TILE_SIZE;
	
		tile_width = x_end - x_pos;
		tile_height = y_end - y_pos;

		orig_x = ((tiles[tile].image % 21) * TILE_SIZE) + x_pos - tiles[tile].x;
		orig_y = ((tiles[tile].image / 21) * TILE_SIZE) + y_pos - tiles[tile].y;
		if (tiles[tile].selected == 1) orig_y += 80;

		gdk_draw_pixmap (draw_area->window,
				 draw_area->style->black_gc, tiles_pix,
				 orig_x, orig_y, x_pos, y_pos,
				 tile_width, tile_height);
	}
	
}

void redraw_area (int x1, int y1, int x2, int y2, int mlayer)
{
	int height, width, x, y, tile, x_pos, y_pos, orig_x, orig_y, x_end, y_end;
	int tile_height, tile_width, last_tile, old_y, layer = 0, last_y_end = 0;
	
	width = x2 - x1;
	height = y2 - y1;

	x = x2 - 1;
	y = y1 + 1;

	tile = find_tile (x, y);
	while (tile > MAX_TILES) {
		y ++;
		tile = find_tile (x, y);
	}
	while (y < y2) {
		x = x2 - 1;
		while (x > x1) {
			tile = find_tile (x, y);
			if (tile > MAX_TILES) {
				if (y >= y2)
					x = x2 - 1;
				old_y = y;
			}
			while ((tile > MAX_TILES) && (y <= y2)) {
				y ++;
				tile = find_tile (x, y);
			}
			if (y >= y2) {
				y = old_y;
				tile = find_tile (x, y);
				while ((tile > MAX_TILES) && (x >= x1)) {
					x --;
					tile = find_tile (x, y);
				}
				if (x <= x1) {
					tile = MAX_TILES + 1;
					x_pos = x1 - 1;
					y_end = y2 + 1;
					last_y_end = y_end;
				}
			}
			if (tile < MAX_TILES) {
				if (tiles[tile].x <= x1) 
					x_pos = x1;
				else x_pos = tiles[tile].x;
	
				if ((tiles[tile].x + TILE_SIZE) >= x2)
					x_end = x2;
				else x_end = tiles[tile].x + TILE_SIZE;

				if (tiles[tile].y <= y1) 
					y_pos = y1;
				else y_pos = tiles[tile].y;
				last_y_end = y_end;
				if ((tiles[tile].y + TILE_SIZE) >= y2)
					y_end = y2;
				else y_end = tiles[tile].y + TILE_SIZE;
	
				tile_width = x_end - x_pos;
				tile_height = y_end - y_pos;

				orig_x = ((tiles[tile].image % 21) * TILE_SIZE) + x_pos - tiles[tile].x;
				orig_y = ((tiles[tile].image / 21) * TILE_SIZE) + y_pos - tiles[tile].y;
				if (tiles[tile].selected == 1) orig_y += 80;

				gdk_draw_pixmap (draw_area->window,
						 draw_area->style->black_gc, tiles_pix,
						 orig_x, orig_y, x_pos, y_pos,
						 tile_width, tile_height);
				x = x_pos - 1;
			}
		}
		if (last_y_end < y_end)
			y = last_y_end + 1;
		else y = y_end + 1;
	}

	tile = find_tile (x1 + 1, y1 + 6);
	redraw_tile_in_area (x1, y1, x2, y2 - 4, tile);
 	tile = find_tile (x1 + 1, y1 + TILE_SIZE - 6);
 	redraw_tile_in_area (x1, y1, x2, y2 - 4, tile); 
 	tile = find_tile (x1 + 8, y1 + TILE_SIZE - 2); 
 	redraw_tile_in_area (x1 + 5, y1, x2, y2, tile); 
 	tile = find_tile (x1 + TILE_SIZE - 2, y1 + TILE_SIZE - 2); 
 	redraw_tile_in_area (x1 + 5, y1, x2, y2, tile); 
 	tile = find_tile_in_layer (x1, y1 - 3, mlayer); 
	if ((tile < MAX_TILES) && (tiles[tile].visible == 1))
		bottom_border (tile);
	tile = find_tile_in_layer (x2 - 1, y1 - 3, mlayer);
	if ((tile < MAX_TILES) && (tiles[tile].visible == 1))
		bottom_border (tile);
	tile = find_tile_in_layer (x2 + 1, y1, mlayer);
	if ((tile < MAX_TILES) && (tiles[tile].visible == 1))
		left_border (tile);
	tile = find_tile_in_layer (x2 + 1, y1 + HALF_TILE + 1, mlayer);
	if ((tile < MAX_TILES) && (tiles[tile].visible == 1))
		left_border (tile);
}

void refresh (GdkRectangle *area)
{
	int x1, y1, x2, y2, x, y, i;
	int bx, by;
	
	for(i = 0; i < MAX_TILES; i ++) {
		if (tiles [i].visible) {
			bx = (tiles[i].image % 21) * TILE_SIZE;
			by = (tiles[i].image / 21) * TILE_SIZE;
			if (tiles [i].selected == 1)
				by += 80;
			gdk_draw_pixmap (draw_area->window,
					 draw_area->style->black_gc, tiles_pix,
					 bx, by, tiles[i].x, tiles[i].y,
					 TILE_SIZE, TILE_SIZE);
 			left_border (i);
 			bottom_border (i);
		}
	}
}

void tile_gone (int i, int x, int y)
{
	int uleft_tile, bleft_tile, lbottom_tile, rbottom_tile, corner_tile;
	int uright_tile, bright_tile, lup_tile, rup_tile;
	int uleft_free = 1, bleft_free = 1, lbottom_free = 1, rbottom_free = 1, corner_free = 1;
	int uright_free = 1, bright_free = 1, lup_free = 1, rup_free = 1;
	
	int tile_x, tile_y, vbord = 0, hbord = 0, layer;

	tile_x = tiles[i].x + 2;
	tile_y = tiles[i].y + 2;
	layer = tiles[i].layer;
	
 	uleft_tile = find_tile_in_layer (tile_x - 5, tile_y, layer); 
 	bleft_tile = find_tile_in_layer (tile_x - 5, tile_y + HALF_TILE, layer); 
 	lbottom_tile = find_tile_in_layer (tile_x + 1, tile_y + TILE_SIZE + 1, layer); 
 	rbottom_tile = find_tile_in_layer (tile_x + TILE_SIZE - 2, tile_y + TILE_SIZE + 1, layer); 
 	corner_tile = find_tile_in_layer (tile_x - 5, tile_y + TILE_SIZE + 1, layer); 

	uright_tile = find_tile_in_layer (tile_x + TILE_SIZE, tile_y, layer);
	bright_tile = find_tile_in_layer (tile_x + TILE_SIZE, tile_y + HALF_TILE, layer);
	lup_tile = find_tile_in_layer (tile_x, tile_y - 5, layer);
	rup_tile = find_tile_in_layer (tile_x + HALF_TILE, tile_y - 5, layer);
	
	if ((uleft_tile < MAX_TILES) && (tiles[uleft_tile].visible != 0)) uleft_free = 0;
	if ((bleft_tile < MAX_TILES) && (tiles[bleft_tile].visible != 0)) bleft_free = 0;
	if ((lbottom_tile < MAX_TILES) && (tiles[lbottom_tile].visible != 0)) lbottom_free = 0;
	if ((rbottom_tile < MAX_TILES) && (tiles[rbottom_tile].visible != 0)) rbottom_free = 0;
	if ((uright_tile < MAX_TILES) && (tiles[uright_tile].visible != 0)) uright_free = 0;
	if ((bright_tile < MAX_TILES) && (tiles[bright_tile].visible != 0)) bright_free = 0;
	if ((lup_tile < MAX_TILES) && (tiles[lup_tile].visible != 0)) lup_free = 0;
	if ((rup_tile < MAX_TILES) && (tiles[rup_tile].visible != 0)) rup_free = 0;
	if ((corner_tile < MAX_TILES) && (tiles[corner_tile].visible != 0)) corner_free = 0;

	if ((uleft_free) && (bleft_free))
		vbord = 5;
	if ((lbottom_free) && (rbottom_free))
		hbord = 4;
	
	gdk_window_clear_area (draw_area->window, tiles[i].x - vbord, tiles[i].y,
			       TILE_SIZE + vbord, TILE_SIZE + hbord);
	if (layer == 0) {
		if (uleft_free)
			gdk_window_clear_area (draw_area->window, tiles[i].x - 5, tiles[i].y,
					       5, HALF_TILE);
		if (bleft_free)
			gdk_window_clear_area (draw_area->window, tiles[i].x - 5, tiles[i].y + HALF_TILE,
					       5, HALF_TILE);
		if (lbottom_free)
			gdk_window_clear_area (draw_area->window, tiles[i].x, tiles[i].y + TILE_SIZE,
					       HALF_TILE, 4);
		if (rbottom_free);
			gdk_window_clear_area (draw_area->window, tiles[i].x + HALF_TILE,
					       tiles[i].y + TILE_SIZE,
					       HALF_TILE, 4);
		if (corner_free)
			gdk_window_clear_area (draw_area->window, tiles[i].x - 5,
					       tiles[i].y + TILE_SIZE, 5, 4);
		if (uright_tile < MAX_TILES) 
			left_border (uright_tile);
		if (bright_tile < MAX_TILES) 
			left_border (bright_tile);
		if (lup_tile < MAX_TILES) 
			bottom_border (lup_tile);
		if (rup_tile < MAX_TILES) 
			bottom_border (rup_tile);
	} 
	
 	if (layer > 0) redraw_area (tiles[i].x - vbord, tiles[i].y, 
				    tiles[i].x + TILE_SIZE,
				    tiles[i].y + TILE_SIZE + hbord,
				    layer); 
}

void button_pressed (int x, int y)
{
	int i;

	i = find_tile (x, y);
	if (i < MAX_TILES) {
		if (tile_free (x, y, i)) {
			if (selected_tile < MAX_TILES) {
				if ((tiles[selected_tile].type == tiles[i].type) &&
				    (i != selected_tile) ) {
					tiles[i].visible = 0;
					tiles[selected_tile].visible = 0;
					tile_gone (i, x, y);
					tile_gone (selected_tile,
						   tiles[selected_tile].x + 1,
						   tiles[selected_tile].y + 1);
					selected_tile = MAX_TILES + 1;
					visible_tiles -= 2;
					if (visible_tiles <= 0)
						you_won ();
				}
				else if (i == selected_tile) {
					tiles[i].selected = 0;
					selected_tile = MAX_TILES + 1;
				}
				else {
					no_match();
					i = selected_tile;
				}
			}
			else if (tiles[i].selected == 0) {
				tiles[i].selected = 1;
				selected_tile = i;
			}
			else {
				tiles[i].selected = 0;
				selected_tile = MAX_TILES + 1;
			}
			draw_selected_tile (i);
		}
	}
}


gint area_event (GtkWidget *widget, GdkEvent *event, void *d)
{
	switch (event->type) {
		case GDK_EXPOSE : {
			GdkEventExpose *e = (GdkEventExpose *) event;
			refresh (&e->area);
			return TRUE;
		}

		case GDK_BUTTON_PRESS: {
			int x, y;

			gtk_widget_get_pointer (widget, &x, &y);
			button_pressed (x, y);
			return TRUE;
		}
	}
}

void load_tiles (char *fname)
{
 	GtkStyle *style;
	char *tmp, *fn;

	tmp = g_copy_strings ("mahjongg/", fname, NULL);

	fn = gnome_unconditional_pixmap_file (tmp);
	g_free (tmp);

	if (!g_file_exists (fn)) {
		printf ("Could not find file \'%s\'\n", fn);
		exit (1);
	}
	
	style = gtk_widget_get_style (draw_area);
	tiles_pix = gdk_pixmap_create_from_xpm (window->window, &mask,
						&style->bg [GTK_STATE_NORMAL],
						fn);

	g_free (fn);
	new_game ();
}

void create_mahjongg_board (void)
{
 	GtkStyle *style;

	draw_area = gtk_drawing_area_new ();
	gtk_widget_set_events (draw_area, gtk_widget_get_events (draw_area) | GAME_EVENTS);

	style = gtk_widget_get_style (draw_area);

	gtk_box_pack_start_defaults (GTK_BOX (mbox), draw_area);
	gtk_widget_realize (draw_area);

	gtk_widget_show (draw_area);

	vborderpic = gdk_pixmap_create_from_xpm_d (window->window, &mask,
						   &style->bg [GTK_STATE_NORMAL],
						   vborder);
	hborderpic = gdk_pixmap_create_from_xpm_d (window->window, &mask,
						   &style->bg [GTK_STATE_NORMAL],
						   hborder);

	gtk_drawing_area_size (GTK_DRAWING_AREA (draw_area),
			       600,
			       400);

	load_tiles ("default.xpm");

	gtk_signal_connect (GTK_OBJECT (draw_area), "event", (GtkSignalFunc) area_event, 0);
	
	gtk_widget_draw (draw_area, NULL);
	new_game ();
}

#define ELEMENTS(x) (sizeof (x) / sizeof (x [0]))

int main (int argc, char *argv [])
{
	GdkColor color;
	GdkColorContext *cc;
	int x;
	
	gtk_init (&argc, &argv);
	gnome_init ("mahjongg", &argc, &argv);

	textdomain (PACKAGE);
	srand (time (NULL));
	
	window = gnome_app_new ("gmahjongg", _("Gnome Mahjongg"));
	gtk_widget_realize (window);
	gtk_window_set_policy (GTK_WINDOW (window), FALSE, FALSE, TRUE);

	gnome_app_create_menus (GNOME_APP (window), mainmenu);

	gtk_signal_connect (GTK_OBJECT (window), "delete_event",
			    GTK_SIGNAL_FUNC (quit_game_callback), NULL);

	mbox = gtk_vbox_new (FALSE, 0);

	gnome_app_set_contents (GNOME_APP (window), mbox);

	cc = gdk_color_context_new (gtk_widget_get_visual (window),
				    gtk_widget_get_colormap (window));
	color.red = 0;
	color.blue = 0;
	color.green = 200;
	gdk_color_context_get_pixels (cc,
				      &color.red,
				      &color.green,
				      &color.blue, 1,
				      &color.pixel, &x);
	create_mahjongg_board ();

	gdk_window_set_background (window->window, &color);
	
	gtk_widget_show (window);
	gtk_main ();
	gdk_color_context_free (cc);
	
	return 0;
}
