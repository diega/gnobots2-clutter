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
#include <gdk_imlib.h>

#define GAME_EVENTS (GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK)

#define TILE_WIDTH 40
#define TILE_HEIGHT 56     
#define MAX_TILES 146
#define HALF_WIDTH 20
#define HALF_HEIGHT 28
#define MAH_VERSION "0.3.0"
     
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
	{6, 0,  0},
	{8, 0,  0},
	{10, 0,  0},
	{12, 0,  0},
	{14, 0,  0},
	{1, 1,  0},
	{3, 1,  0},
	{1, 3,  0},
	{3, 3,  0},
	{0, 6,  0},
	{2, 5,  0},
	{2, 7,  0},
	{1, 9,  0},
	{3, 9,  0},
	{1, 11,  0},
	{3, 11,  0},
	{6, 12,  0},
	{8, 12,  0},
	{10, 12,  0},
	{12, 12,  0},
	{14, 12,  0},
	{17, 1,  0},
	{19, 1,  0},
	{17, 3,  0},
	{19, 3,  0},
	{17, 9,  0},
	{19, 9,  0},
	{17, 11,  0},
	{19, 11,  0},
	{20, 6,  0},
	{18, 7,  0},
	{18, 5,  0},
	{5, 2,  0},
	{7, 2,  0},
	{9, 2,  0},
	{11, 2,  0},
	{13, 2,  0},
	{15, 2,  0},
	{5, 10,  0},
	{7, 10,  0},
	{9, 10,  0},
	{11, 10,  0},
	{13, 10,  0},
	{15, 10,  0},
	{4, 5,  0},
	{4, 7,  0},
	{16, 5,  0},
	{16, 7,  0},
	{6, 4,  0},
	{8, 4,  0},
	{10, 4,  0},
	{12, 4,  0},
	{14, 4,  0},
	{6, 6,  0},
	{8, 6,  0},
	{10, 6,  0},
	{12, 6,  0},
	{14, 6,  0},
	{6, 8,  0},
	{8, 8,  0},
	{10, 8,  0},
	{12, 8,  0},
	{14, 8,  0},
	{2, 2,  1},
	{4, 2,  1},
	{6, 2,  1},
	{7, 0,  1},
	{9, 1,  1},
	{11, 1,  1},
	{13, 0,  1},
	{14, 2,  1},
	{16, 2,  1},
	{18, 2,  1},
	{2, 10,  1},
	{4, 10,  1},
	{6, 10,  1},
	{7, 12,  1},
	{9, 11,  1},
	{11, 11,  1},
	{13, 12,  1},
	{14, 10,  1},
	{16, 10,  1},
	{18, 10,  1},
	{8, 3,  1},
	{10, 3,  1},
	{12, 3,  1},
	{7, 5,  1},
	{9, 5,  1},
	{11, 5,  1},
	{13, 5,  1},
	{7, 7,  1},
	{9, 7,  1},
	{11, 7,  1},
	{13, 7,  1},
	{8, 9,  1},
	{10, 9,  1},
	{12, 9,  1},
	{3, 4,  1},
	{3, 6,  1},
	{3, 8,  1},
	{5, 4,  1},
	{5, 6,  1},
	{5, 8,  1},
	{15, 4,  1},
	{15, 6,  1},
	{15, 8,  1},
	{17, 4,  1},
	{17, 6,  1},
	{17, 8,  1},
	{5, 4,  2},
	{7, 4,  2},
	{9, 4,  2},
	{11, 4,  2},
	{13, 4,  2},
	{15, 4,  2},
	{5, 8,  2},
	{7, 8,  2},
	{9, 8,  2},
	{11, 8,  2},
	{13, 8,  2},
	{15, 8,  2},
	{4, 6,  2},
	{6, 6,  2},
	{8, 6,  2},
	{10, 6,  2},
	{12, 6,  2},
	{14, 6,  2},
	{16, 6,  2},
	{7, 5,  3},
	{9, 5,  3},
	{11, 5,  3},
	{13, 5,  3},
	{7, 7,  3},
	{9, 7,  3},
	{11, 7,  3},
	{13, 7,  3},
	{5, 6,  3},
	{15, 6,  3},
	{8, 6,  4},
	{10, 6,  4},
	{12, 6,  4},
	{9, 6,  5},
	{11, 6,  5},
	{10, 6,  6}
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
GdkPixmap *tiles_pix, *mask;
GdkGC *my_gc;
tile tiles[MAX_TILES];
int selected_tile, visible_tiles;

static GdkImlibImage *tiles_image;

void quit_game_callback (GtkWidget *widget, gpointer data);
void new_game_callback (GtkWidget *widget, gpointer data);
void restart_game_callback (GtkWidget *widget, gpointer data);
void select_game_callback (GtkWidget *widget, gpointer date);
void new_game (void);
void redraw_area (int x1, int y1, int x2, int y2, int mlayer);
void about_callback (GtkWidget *widget, gpointer data);

GnomeUIInfo filemenu [] = {
	{GNOME_APP_UI_ITEM, N_("New"), NULL, new_game_callback, NULL, NULL,
	 GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_NEW, 0, 0, NULL},

	{GNOME_APP_UI_ITEM, N_("Restart..."), NULL, restart_game_callback, NULL, NULL,
	 GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},

	{GNOME_APP_UI_ITEM, N_("Select game"), NULL, select_game_callback, NULL, NULL,
	 GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_NEW, 0, 0, NULL},

	{GNOME_APP_UI_ITEM, N_("Properties"), NULL, NULL, NULL, NULL,
	 GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_PROP, 0, 0, NULL},

	{GNOME_APP_UI_ITEM, N_("Exit"), NULL, quit_game_callback, NULL, NULL,
	 GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_EXIT, 0, 0, NULL},
	
	{GNOME_APP_UI_ENDOFINFO}
};

GnomeUIInfo helpmenu[] = {
	{GNOME_APP_UI_HELP, NULL, NULL, NULL, NULL, NULL,
	 GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
	
	{GNOME_APP_UI_ITEM, N_("About"), NULL, about_callback, NULL, NULL,
	 GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_ABOUT, 0, 0, NULL},
	
	{GNOME_APP_UI_ENDOFINFO}
};

GnomeUIInfo mainmenu [] = {
	{GNOME_APP_UI_SUBTREE, N_("Game"), NULL, filemenu, NULL, NULL,
	 GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
	
	{GNOME_APP_UI_SUBTREE, N_("Help"), NULL, helpmenu, NULL, NULL,
	 GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
	
	{GNOME_APP_UI_ENDOFINFO}
};

int find_tile_in_layer (int x, int y, int layer)
{
	int i, tile_num = MAX_TILES + 1;

	for (i = 0; i < MAX_TILES; i ++) {
		if ((tiles[i].x < x) && ((tiles[i].x + TILE_WIDTH) > x)) {
			if ((tiles[i].y < y) && ((tiles[i].y + TILE_HEIGHT) > y)) {
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
		if ((tiles[i].x < x) && ((tiles[i].x + TILE_WIDTH) > x) && (tiles[i].visible)) {
			if ((tiles[i].y < y) && ((tiles[i].y + TILE_HEIGHT) > y)) {
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
	redraw_area (tiles[i].x, tiles[i].y,
		     tiles[i].x + TILE_WIDTH, tiles[i].y + TILE_HEIGHT,
		     tiles[i].layer);
}

int tile_free (int tile_num)
{
	int uleft_tile, bleft_tile, uright_tile, bright_tile, lup_tile, rup_tile, lbottom_tile, rbottom_tile;
	int tile_x, tile_y;
	int up_free = 0, bottom_free = 0, left_free = 0, right_free = 0;
	int nlayer_ul_free = 0, nlayer_ur_free = 0, nlayer_bl_free = 0, nlayer_br_free = 0, nlayer, temp_tile;

	if (tiles[tile_num].visible == 0)
		return 0;
	
	if (tiles[tile_num].layer < 6) {
		nlayer = tiles[tile_num].layer + 1;
		tile_x = tiles[tile_num].x + 6;
		tile_y = tiles[tile_num].y - 3;
		
		temp_tile = find_tile_in_layer (tile_x, tile_y, nlayer);
		if ((temp_tile > MAX_TILES) || (tiles[temp_tile].visible == 0))
			nlayer_ul_free = 1;
		
		temp_tile = find_tile_in_layer (tile_x + HALF_WIDTH, tile_y, nlayer);
		if ((temp_tile > MAX_TILES) || (tiles[temp_tile].visible == 0))
			nlayer_ur_free = 1;
		
		temp_tile = find_tile_in_layer (tile_x, tile_y + HALF_HEIGHT, nlayer);
		if ((temp_tile > MAX_TILES) || (tiles[temp_tile].visible == 0))
			nlayer_bl_free = 1;
		
		temp_tile = find_tile_in_layer (tile_x + HALF_WIDTH, tile_y + HALF_HEIGHT, nlayer);
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
		uleft_tile = find_tile_in_layer (tile_x - HALF_WIDTH, tile_y, tiles[tile_num].layer);
		bleft_tile = find_tile_in_layer (tile_x - HALF_WIDTH,
						 tile_y + HALF_HEIGHT, tiles[tile_num].layer);
		uright_tile = find_tile_in_layer (tile_x + TILE_WIDTH, tile_y, tiles[tile_num].layer);
		bright_tile = find_tile_in_layer (tile_x + TILE_WIDTH,
						  tile_y + HALF_HEIGHT, tiles[tile_num].layer);
		rbottom_tile = find_tile_in_layer (tile_x + HALF_WIDTH, tile_y + TILE_HEIGHT, tiles[tile_num].layer);
		lbottom_tile = find_tile_in_layer (tile_x, tile_y + TILE_HEIGHT, tiles[tile_num].layer);
		rup_tile = find_tile_in_layer (tile_x + HALF_WIDTH, tile_y - TILE_HEIGHT, tiles[tile_num].layer);
		lup_tile = find_tile_in_layer (tile_x, tile_y - TILE_HEIGHT, tiles[tile_num].layer);

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

	mb = gnome_message_box_new (_("Tiles don't match!"),
				   GNOME_MESSAGE_BOX_INFO,
				   _("Ok"), NULL);
	GTK_WINDOW(mb)->position = GTK_WIN_POS_MOUSE;
	gnome_message_box_set_modal (GNOME_MESSAGE_BOX (mb));
	gtk_widget_show (mb);
}

void check_free (void)
{
	int i = -1, type = MAX_TILES, br = 0, other = MAX_TILES, free = 0;
	GtkWidget *mb;
	
	while ((i < MAX_TILES) && (!free)) {
		if (other >= MAX_TILES) i++;
		while ((i < MAX_TILES) && (!tile_free(i))) {
			i++;
		}
		if (other >= MAX_TILES) {
			other = i + 1;
		}
		if (tile_free(i)) type = tiles[i].type;
		br = 0;
		while (!br) {
			other++;
			if (other >= MAX_TILES)
				br = 1;
			if ((tile_free(other)) && (tiles[other].type == type)) {
				br = 1;
				free = 1;
				other = MAX_TILES;
			}
		}
	}
 	if (!free) { 
 		mb = gnome_message_box_new (_("No more movements"), 
 					    GNOME_MESSAGE_BOX_INFO, 
 					    _("Ok"), NULL); 
 		GTK_WINDOW(mb)->position = GTK_WIN_POS_MOUSE; 
 		gnome_message_box_set_modal (GNOME_MESSAGE_BOX (mb)); 
 		gtk_widget_show (mb); 
 	} 
}

void you_won (void)
{
	GtkWidget *mb;

	mb = gnome_message_box_new (_("You won!"),
				   GNOME_MESSAGE_BOX_INFO,
				   _("Ok"), NULL);
	GTK_WINDOW(mb)->position = GTK_WIN_POS_MOUSE;
	gnome_message_box_set_modal (GNOME_MESSAGE_BOX (mb));
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
		"Code: Francisco Bustamante",
		"Tiles: Jonathan Buzzard",
		NULL
	};

	about = gnome_about_new (_("Gnome Mahjongg"), MAH_VERSION,
				 "(C) 1998 The Free Software Foundation",
				 authors,
				 _("Send comments and bug reports to: pancho@nuclecu.unam.mx\nTiles under the General Public License."),
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

void restart_game_callback (GtkWidget *widget, gpointer data)
{
	int i;
	visible_tiles = 144;
	for (i = 0; i < 144; i++) {
		tiles[i].visible = 1;
		tiles[i].selected = 0;
	}
	gtk_widget_draw (draw_area, NULL);	
}

static void
input_callback (GtkWidget *widget, gpointer data)
{
	srand (atoi (GTK_ENTRY (data)->text));
	new_game ();
}

static void
cancel_callback (GtkWidget *widget, gpointer data)
{
	gtk_widget_destroy (GTK_WIDGET (data));
}

void select_game_callback (GtkWidget *widget, gpointer data)
{
	GtkWidget *dialog, *entry, *label, *button, *vbox;

	dialog = gtk_dialog_new ();
	GTK_WINDOW (dialog)->position = GTK_WIN_POS_MOUSE;
	gtk_window_set_title (GTK_WINDOW (dialog), _("Select Game"));
	gtk_signal_connect (GTK_OBJECT (dialog), "delete_event",
			    GTK_SIGNAL_FUNC (cancel_callback),
			    (gpointer)dialog);
	label = gtk_label_new (_("Game Number:"));
	gtk_box_pack_start_defaults (GTK_BOX (GTK_DIALOG (dialog)->vbox), label);
	gtk_widget_show (label);
	
	entry = gtk_entry_new ();
	gtk_box_pack_start_defaults (GTK_BOX(GTK_DIALOG(dialog)->vbox), entry);
	gtk_widget_show (entry);

	vbox = gtk_hbox_new (TRUE, 4);
	gtk_container_add (GTK_CONTAINER(GTK_DIALOG(dialog)->action_area), vbox);
  
	button = gnome_stock_button (GNOME_STOCK_BUTTON_OK);
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
			    GTK_SIGNAL_FUNC (input_callback),
			    (gpointer)entry);
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
			    GTK_SIGNAL_FUNC (cancel_callback),
			    (gpointer)dialog);
	gtk_box_pack_start_defaults (GTK_BOX(vbox), button);
	gtk_widget_show(button);
  
	button = gnome_stock_button (GNOME_STOCK_BUTTON_CANCEL);
	gtk_signal_connect (GTK_OBJECT (button), "clicked",
			    GTK_SIGNAL_FUNC (cancel_callback),
			    (gpointer)dialog);
	gtk_box_pack_start_defaults (GTK_BOX(vbox), button);
	gtk_widget_show(button);
  
	gtk_widget_show(vbox);
  
	gtk_grab_add (dialog);
	gtk_widget_show (dialog);
	
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
			f = (int) (145.0 * rand () / RAND_MAX) - 1;
			while (tiles[f].visible != 0)
				f = (int) (145.0 * rand () / RAND_MAX) - 1;
			tiles[f].visible = 1;
			tiles[f].selected = 0;
			tiles[f].x = default_pos[f].x * HALF_WIDTH + 80 + (5 * default_pos[f].layer);
			tiles[f].y = default_pos[f].y * HALF_HEIGHT + 60 - (4 * default_pos[f].layer);
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
	
		if ((tiles[tile].x + TILE_WIDTH) >= x2)
			x_end = x2;
		else x_end = tiles[tile].x + TILE_WIDTH;

		if (tiles[tile].y <= y1) 
			y_pos = y1;
		else y_pos = tiles[tile].y;
		if ((tiles[tile].y + TILE_HEIGHT) >= y2)
			y_end = y2;
		else y_end = tiles[tile].y + TILE_HEIGHT;
	
		tile_width = x_end - x_pos;
		tile_height = y_end - y_pos;

		orig_x = ((tiles[tile].image % 21) * TILE_WIDTH) + x_pos - tiles[tile].x;
		orig_y = ((tiles[tile].image / 21) * TILE_HEIGHT) + y_pos - tiles[tile].y;
		if (tiles[tile].selected == 1) orig_y += 2 * TILE_HEIGHT;

		gdk_gc_set_clip_origin (my_gc, tiles[tile].x, tiles[tile].y);

		gdk_draw_pixmap (draw_area->window,
				 my_gc, tiles_pix,
				 orig_x, orig_y, x_pos, y_pos,
				 tile_width, tile_height);
	}
	
}

void redraw_area (int x1, int y1, int x2, int y2, int mlayer)
{
	int i;

	for(i = 0; i < MAX_TILES; i ++) {
		if ((tiles[i].visible) && (tiles[i].layer >= mlayer)) {
			if (((tiles[i].x >= x1) && (tiles[i].x <= x2)) ||
			    ((tiles[i].x + TILE_WIDTH >= x1) && (tiles[i].x + TILE_WIDTH <= x2)))
				if (((tiles[i].y >= y1) && (tiles[i].y <= y2)) ||
				    ((tiles[i].y + TILE_HEIGHT >= y1) && (tiles[i].y + TILE_HEIGHT <= y2)))
					redraw_tile_in_area (x1, y1, x2, y2, i);
		}
	}
}

void refresh (GdkRectangle *area)
{
	int x1, y1, x2, y2, x, y, i;

	redraw_area (area->x, area->y, area->x + area->width, area->y + area->height, 0);
}

void tile_gone (int i, int x, int y)
{
	gdk_window_clear_area (draw_area->window, tiles[i].x, tiles[i].y,
			       TILE_WIDTH, TILE_HEIGHT);
	
	redraw_area (tiles[i].x, tiles[i].y, 
		     tiles[i].x + TILE_WIDTH,
		     tiles[i].y + TILE_HEIGHT,
		     0);
}

void button_pressed (int x, int y)
{
	int i;

	i = find_tile (x, y);
	if (i < MAX_TILES) {
		if (tile_free (i)) {
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
					check_free();
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
	char *tmp, *fn;

	tmp = g_copy_strings ("mahjongg/", fname, NULL);

	fn = gnome_unconditional_pixmap_file (tmp);
	g_free (tmp);

	if (!g_file_exists (fn)) {
		printf ("Could not find file \'%s\'\n", fn);
		exit (1);
	}

	if (tiles_image)
		gdk_imlib_destroy_image (tiles_image);

	tiles_image = gdk_imlib_load_image (fn);
	gdk_imlib_render (tiles_image, tiles_image->rgb_width, tiles_image->rgb_height);
	
	tiles_pix = gdk_imlib_move_image (tiles_image);
	mask = gdk_imlib_move_mask (tiles_image);

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

	gtk_drawing_area_size (GTK_DRAWING_AREA (draw_area),
			       600,
			       480);

	load_tiles ("default.xpm");
	my_gc = gdk_gc_new (draw_area->window);
	gdk_gc_set_clip_mask (my_gc, mask);

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
	
	argp_program_version = MAH_VERSION;

	bindtextdomain (PACKAGE, GNOMELOCALEDIR);
	textdomain (PACKAGE);

	gnome_init ("mahjongg", NULL, argc, argv, 0, NULL);

	srand (time (NULL));
	
	window = gnome_app_new ("gmahjongg", _("Gnome Mahjongg"));
	gtk_widget_realize (window);
	gtk_window_set_policy (GTK_WINDOW (window), FALSE, FALSE, TRUE);

	gnome_app_create_menus (GNOME_APP (window), mainmenu);
	gtk_menu_item_right_justify (GTK_MENU_ITEM(mainmenu[1].widget));

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

	gdk_window_set_background (draw_area->window, &color);
	
	gtk_widget_show (window);
	gtk_main ();
	gdk_color_context_free (cc);
	
	return 0;
}
