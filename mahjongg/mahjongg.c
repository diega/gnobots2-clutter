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

#include "button-images.h"

#define GAME_EVENTS (GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK)

#define TILE_WIDTH 40
#define TILE_HEIGHT 56
#define MAX_TILES 146
#define HALF_WIDTH 18
//#define HALF_WIDTH 20
#define HALF_HEIGHT 26
//#define HALF_HEIGHT 28
#define MAH_VERSION "0.4.0"
     
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
	{32, 4, 32}, 
	{33, 1, 33}, 
	{33, 1, 34}, 
	{33, 1, 35},
	{33, 1, 36}, 
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
        {13, 7,  4},
        {12, 8,  3},
        {14, 8,  3},
        {12, 6,  3},
        {14, 6,  3},
        {10, 10,  2},
	{12, 10,  2},
	{14, 10,  2},
	{16, 10,  2},
	{10, 8,  2},
	{12, 8,  2},
	{14, 8,  2},
	{16, 8,  2},
        {10, 6,  2},
	{12, 6,  2},
	{14, 6,  2},
	{16, 6,  2},
	{10, 4,  2},
	{12, 4,  2},
	{14, 4,  2},
	{16, 4,  2},
	{8, 12,  1},
	{10, 12,  1},
	{12, 12,  1},
	{14, 12,  1},
	{16, 12,  1},
	{18, 12,  1},
	{8, 10,  1},
	{10, 10,  1},
	{12, 10,  1},
	{14, 10,  1},
	{16, 10,  1},
	{18, 10,  1},
	{8, 8,  1},
	{10, 8,  1},
	{12, 8,  1},
	{14, 8,  1},
	{16, 8,  1},
	{18, 8,  1},
	{8, 6,  1},
	{10, 6,  1},
	{12, 6,  1},
	{14, 6,  1},
	{16, 6,  1},
	{18, 6,  1},
	{8, 4,  1},
	{10, 4,  1},
	{12, 4,  1},
	{14, 4,  1},
	{16, 4,  1},
	{18, 4,  1},
	{8, 2,  1},
	{10, 2,  1},
	{12, 2,  1},
	{14, 2,  1},
	{16, 2,  1},
	{18, 2,  1},
        {2, 14,  0},
        {4, 14,  0},
	{6, 14,  0},
	{8, 14,  0},
	{10, 14,  0},
	{12, 14,  0},
	{14, 14,  0},
	{16, 14,  0},
        {18, 14,  0},
        {20, 14,  0},
        {22, 14,  0},
        {24, 14,  0},
        {6, 12,  0},
	{8, 12,  0},
	{10, 12,  0},
	{12, 12,  0},
	{14, 12,  0},
	{16, 12,  0},
	{18, 12,  0},
	{20, 12,  0},
        {4, 10,  0},
        {6, 10,  0},
	{8, 10,  0},
	{10, 10,  0},
	{12, 10,  0},
	{14, 10,  0},
        {16, 10,  0},
        {18, 10,  0},
        {20, 10,  0},
        {22, 10,  0},
        {0, 7,  0},
        {2, 8,  0},
        {4, 8,  0},
	{6, 8,  0},
	{8, 8,  0},
	{10, 8,  0},
	{12, 8,  0},
	{14, 8,  0},
	{16, 8,  0},
        {18, 8,  0},
        {20, 8,  0},
        {22, 8,  0},
        {24, 8,  0},
        {2, 6,  0},
	{4, 6,  0},
	{6, 6,  0},
	{8, 6,  0},
	{10, 6,  0},
	{12, 6,  0},
	{14, 6,  0},
	{16, 6,  0},
	{18, 6,  0},
	{20, 6,  0},
	{22, 6,  0},
	{24, 6,  0},
        {4, 4,  0},
        {6, 4,  0},
	{8, 4,  0},
	{10, 4,  0},
	{12, 4,  0},
	{14, 4,  0},
        {16, 4,  0},
        {18, 4,  0},
        {20, 4,  0},
        {22, 4,  0},
	{6, 2,  0},
	{8, 2,  0},
	{10, 2,  0},
	{12, 2,  0},
	{14, 2,  0},
	{16, 2,  0},
	{18, 2,  0},
        {20, 2,  0},
        {2, 0,  0},
        {4, 0,  0},
	{6, 0,  0},
	{8, 0,  0},
	{10, 0,  0},
	{12, 0,  0},
	{14, 0,  0},
        {16, 0,  0},
	{18, 0,  0},
        {20, 0,  0},
	{22, 0,  0},
        {24, 0,  0},
        {26, 7, 0},
        {28, 7, 0}
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
GtkWidget *tiles_label;
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
void show_tb_callback (GtkWidget *widget, gpointer data);
void sound_on_callback (GtkWidget *widget, gpointer data);


GnomeUIInfo filemenu [] = {
         {GNOME_APP_UI_ITEM, N_("New"), NULL, new_game_callback, NULL, NULL,
         GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_NEW, 0, 0, NULL},

         {GNOME_APP_UI_ITEM, N_("New Seed..."), NULL, select_game_callback, NULL, NULL,
         GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_NEW, 0, 0, NULL},

         {GNOME_APP_UI_ITEM, N_("Restart"), NULL, restart_game_callback, NULL, NULL,
         GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_REFRESH, 0, 0, NULL},

         {GNOME_APP_UI_SEPARATOR},

         {GNOME_APP_UI_ITEM, N_("Hint"), NULL, NULL, NULL, NULL,
         GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},

         {GNOME_APP_UI_ITEM, N_("Undo"), NULL, NULL, NULL, NULL,
         GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_UNDO, 0, 0, NULL},

         {GNOME_APP_UI_SEPARATOR},
	
         {GNOME_APP_UI_ITEM, N_("Exit"), NULL, quit_game_callback, NULL, NULL,
         GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_EXIT, 0, 0, NULL},

         {GNOME_APP_UI_ENDOFINFO}
};

GnomeUIInfo optionsmenu [] = {
	{GNOME_APP_UI_TOGGLEITEM, N_("Show Tool Bar"), NULL, show_tb_callback, NULL, NULL,
	 GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},

        {GNOME_APP_UI_SEPARATOR},

        {GNOME_APP_UI_TOGGLEITEM, N_("Sound"), NULL, NULL, NULL, NULL,
        GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},

        {GNOME_APP_UI_ITEM, N_("Tile Set"), NULL, NULL, NULL, NULL,
        GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_BLANK, 0, 0, NULL},

        {GNOME_APP_UI_ITEM, N_("Properties..."), NULL, NULL, NULL, NULL,
        GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_PROP, 0, 0, NULL},

        
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

         {GNOME_APP_UI_SUBTREE, N_("Options"), NULL, optionsmenu, NULL, NULL,
	 GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
	
	{GNOME_APP_UI_SUBTREE, N_("Help"), NULL, helpmenu, NULL, NULL,
	 GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
	
	{GNOME_APP_UI_ENDOFINFO}
};

GnomeUIInfo toolbar [] = {
         {GNOME_APP_UI_ITEM, N_("New"), NULL, new_game_callback, NULL, NULL,
         GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_PIXMAP_NEW, 0, 0, NULL},

         {GNOME_APP_UI_ITEM, N_("New Seed"), NULL, select_game_callback, NULL, NULL,
         GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_PIXMAP_NEW, 0, 0, NULL},

         {GNOME_APP_UI_ITEM, N_("Restart"), NULL, restart_game_callback, NULL, NULL,
         GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_PIXMAP_REFRESH, 0, 0, NULL},

         {GNOME_APP_UI_ITEM, N_("Hint"), NULL, NULL, NULL, NULL,
         GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_PIXMAP_HELP, 0, 0, NULL},

         {GNOME_APP_UI_ITEM, N_("Undo"), NULL, NULL, NULL, NULL,
         GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_PIXMAP_UNDO, 0, 0, NULL},

         {GNOME_APP_UI_TOGGLEITEM, N_("Sound"), NULL, NULL, NULL, NULL,
         GNOME_APP_PIXMAP_DATA, mini_sound_xpm, 0, 0, NULL},

         {GNOME_APP_UI_ITEM, N_("Tile Set"), NULL, NULL, NULL, NULL,
         GNOME_APP_PIXMAP_DATA, mini_tiles_xpm, 0, 0, NULL},

         {GNOME_APP_UI_ITEM, N_("Properties"), NULL, NULL, NULL, NULL,
         GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_PIXMAP_PROPERTIES, 0, 0, NULL},

         {GNOME_APP_UI_SEPARATOR},

	{GNOME_APP_UI_ENDOFINFO}
};

int find_tile_in_layer (int x, int y, int layer)
{
	int i, tile_num = MAX_TILES + 1;

	for (i = 0; i < MAX_TILES; i ++) {
		if ((tiles[i].x < x) && ((tiles[i].x + TILE_WIDTH - 1) > x)) {
			if ((tiles[i].y < y) && ((tiles[i].y + TILE_HEIGHT - 1) > y)) {
				if ((tiles[i].layer == layer) && (tiles[i].visible == 1))
					tile_num = i;
			}
		}
	}
	return tile_num;
}
	
int find_tile (int x, int y)
{
	int i, tile_num = MAX_TILES + 1, layer = 0;

	for (i = 0; i < MAX_TILES; i++) {
		if ((tiles[i].x < x) && ((tiles[i].x + TILE_WIDTH - 1) > x) && (tiles[i].visible)) {
			if ((tiles[i].y < y) && ((tiles[i].y + TILE_HEIGHT - 1) > y)) {
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
		     tiles[i].x + TILE_WIDTH - 1, tiles[i].y + TILE_HEIGHT - 1,
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
	
        /*
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
           */
		nlayer_ul_free = 1;
		nlayer_ur_free = 1;
		nlayer_bl_free = 1;
		nlayer_br_free = 1;
	/* } */
	tile_x = tiles[tile_num].x + 2;
	tile_y = tiles[tile_num].y + 2;

	if ((nlayer_ul_free == 1) && (nlayer_ur_free == 1) &&
	    (nlayer_bl_free == 1) && (nlayer_br_free == 1)) {
		uleft_tile = find_tile_in_layer (tile_x - HALF_WIDTH - 2, tile_y, tiles[tile_num].layer);
		bleft_tile = find_tile_in_layer (tile_x - HALF_WIDTH,
						 tile_y + HALF_HEIGHT, tiles[tile_num].layer);
		uright_tile = find_tile_in_layer (tile_x + TILE_WIDTH - 1, tile_y, tiles[tile_num].layer);
		bright_tile = find_tile_in_layer (tile_x + TILE_WIDTH - 1,
						  tile_y + HALF_HEIGHT, tiles[tile_num].layer);
		rbottom_tile = find_tile_in_layer (tile_x + HALF_WIDTH, tile_y + TILE_HEIGHT - 1, tiles[tile_num].layer);
		lbottom_tile = find_tile_in_layer (tile_x, tile_y + TILE_HEIGHT - 1, tiles[tile_num].layer);
		rup_tile = find_tile_in_layer (tile_x + HALF_WIDTH, tile_y - TILE_HEIGHT, tiles[tile_num].layer);
		lup_tile = find_tile_in_layer (tile_x, tile_y - TILE_HEIGHT, tiles[tile_num].layer);

                if (((uleft_tile > MAX_TILES) || (tiles[uleft_tile].visible == 0))
                 || ((bleft_tile > MAX_TILES) || (tiles[bleft_tile].visible == 0)))
                    left_free = 1;
                if (((uright_tile > MAX_TILES) || (tiles[uright_tile].visible == 0))
                    ||  ((bright_tile > MAX_TILES) || (tiles[bright_tile].visible == 0)))
                    right_free = 1;
                if ((left_free) || (right_free)) return 1;
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
				 _("Send comments and bug reports to: pancho@nuclecu.unam.mx\n"
				   "Tiles under the General Public License."),
				 NULL);
	gtk_widget_show (about);
}

void quit_game_callback (GtkWidget *widget, gpointer data)
{
    gnome_config_sync();
    gtk_main_quit ();
}

void new_game_callback (GtkWidget *widget, gpointer data)
{
    gtk_label_set(GTK_LABEL(tiles_label), "144");
    new_game ();
}

void restart_game_callback (GtkWidget *widget, gpointer data)
{
    int i;
    
    visible_tiles = 144;
    gtk_label_set(GTK_LABEL(tiles_label), "144");
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

void show_tb_callback (GtkWidget *widget, gpointer data)
{
    if((GTK_CHECK_MENU_ITEM(optionsmenu[0].widget))->active)
    {
        gnome_config_set_bool("gmahjongg/toolbar/show", TRUE);
        gtk_widget_show(GTK_WIDGET(GNOME_APP(window)->toolbar->parent));
    }
    else
    {
        gnome_config_set_bool("gmahjongg/toolbar/show", FALSE);
        gtk_widget_hide(GTK_WIDGET(GNOME_APP(window)->toolbar->parent));
    }
}

void sound_on_callback (GtkWidget *widget, gpointer data)
{
    printf("mer\n");
}

void new_game (void)
{
	int i, f, n;
	
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
			tiles[f].x = default_pos[f].x * (HALF_WIDTH-0) + 30 + (5 * default_pos[f].layer);
			tiles[f].y = default_pos[f].y * (HALF_HEIGHT-0) + 25 - (4 * default_pos[f].layer);
			tiles[f].layer = default_pos[f].layer;
			tiles[f].type = default_types[i].type; 
			tiles[f].image = default_types[i].image; 
		}
	}
	
	gtk_widget_draw (draw_area, NULL);
}

void redraw_tile_in_area (int x1, int y1, int x2, int y2, int tile)
{
	GdkRectangle trect, arect, dest;
	int orig_x, orig_y;

	if (tile < MAX_TILES) {
		trect.x = tiles[tile].x;
		trect.y = tiles[tile].y;
		trect.width = TILE_WIDTH;
		trect.height = TILE_HEIGHT;

		arect.x = x1;
		arect.y = y1;
		arect.width = x2 - x1 + 1;
		arect.height = y2 - y1 + 1;

		if (gdk_rectangle_intersect (&trect, &arect, &dest)) {
			/* 21 is the number of tiles in a row of the source image */

			orig_x = (tiles[tile].image % 21) * TILE_WIDTH + dest.x - trect.x;
			orig_y = (tiles[tile].image / 21) * TILE_HEIGHT + dest.y - trect.y;

			if (tiles[tile].selected)
				orig_y += 2 * TILE_HEIGHT;

			gdk_gc_set_clip_origin (my_gc, trect.x, trect.y);

			gdk_draw_pixmap (draw_area->window,
					 my_gc,
					 tiles_pix,
					 orig_x, orig_y,
					 dest.x, dest.y,
					 dest.width, dest.height);
		}
	}
}

void redraw_area (int x1, int y1, int x2, int y2, int mlayer)
{
	int i;
	GdkRectangle area, trect, dest;

	area.x = x1;
	area.y = y1;
	area.width = x2 - x1 + 1;
        area.height = y2 - y1 + 1;

	//for(i = 0; i < MAX_TILES; i ++) {
        for(i = MAX_TILES; i > -1; i --) {
        	if ((tiles[i].visible) && (tiles[i].layer >= mlayer)) {
			trect.x = tiles[i].x;
			trect.y = tiles[i].y;
			trect.width = TILE_WIDTH;
			trect.height = TILE_HEIGHT;

			if (gdk_rectangle_intersect (&area, &trect, &dest))
				redraw_tile_in_area (x1, y1, x2, y2, i);
		}
	}
}

void refresh (GdkRectangle *area)
{
	redraw_area (area->x, area->y, area->x + area->width - 1, area->y + area->height - 1, 0);
}

void tile_gone (int i, int x, int y)
{
	gdk_window_clear_area (draw_area->window, tiles[i].x, tiles[i].y,
			       TILE_WIDTH, TILE_HEIGHT);
	
	redraw_area (tiles[i].x, tiles[i].y, 
		     tiles[i].x + TILE_WIDTH - 1,
		     tiles[i].y + TILE_HEIGHT - 1,
		     0);
}

void button_pressed (int x, int y)
{
	int i;
        gchar *tmpchar;
        
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
                                        sprintf(tmpchar,"%d",visible_tiles);
                                        gtk_label_set(GTK_LABEL(tiles_label), tmpchar);
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

	default:
		return FALSE;
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

	gtk_widget_push_visual (gdk_imlib_get_visual ());
	gtk_widget_push_colormap (gdk_imlib_get_colormap ());

	draw_area = gtk_drawing_area_new ();

	gtk_widget_pop_colormap ();
	gtk_widget_pop_visual ();
	
	gtk_widget_set_events (draw_area, gtk_widget_get_events (draw_area) | GAME_EVENTS);

	style = gtk_widget_get_style (draw_area);

	gtk_box_pack_start_defaults (GTK_BOX (mbox), draw_area);
	gtk_widget_realize (draw_area);

	gtk_widget_show (draw_area);

	gtk_drawing_area_size (GTK_DRAWING_AREA (draw_area),
			       600,
			       480);

        /* FIXME: Let the user specify the tile set via a dialog or the command line */
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
	gtk_menu_item_right_justify (GTK_MENU_ITEM(mainmenu[2].widget));

        gnome_app_create_toolbar (GNOME_APP (window), toolbar);
        gtk_toolbar_set_space_size(GTK_TOOLBAR(GNOME_APP(window)->toolbar), 25);
        
        if(gnome_config_get_bool("/gmahjongg/toolbar/show=TRUE"))
            gtk_check_menu_item_set_state(GTK_CHECK_MENU_ITEM(optionsmenu[0].widget), TRUE);
        else
            gtk_widget_hide(GNOME_APP(window)->toolbar->parent);

        tiles_label = gtk_label_new(_(" Tiles "));
        gtk_widget_show(tiles_label);
        gtk_toolbar_append_widget(GTK_TOOLBAR(GNOME_APP(window)->toolbar), tiles_label,
                                  NULL, NULL);
        tiles_label = gtk_label_new(_(" Remaining: "));
        gtk_widget_show(tiles_label);
        gtk_toolbar_append_widget(GTK_TOOLBAR(GNOME_APP(window)->toolbar), tiles_label,
                                  NULL, NULL);
        tiles_label = gtk_label_new("144");
        gtk_widget_show(tiles_label);
        gtk_toolbar_append_widget(GTK_TOOLBAR(GNOME_APP(window)->toolbar), tiles_label,
                                  NULL, NULL);
        gtk_toolbar_append_space(GTK_TOOLBAR(GNOME_APP(window)->toolbar));

	gtk_signal_connect (GTK_OBJECT (window), "delete_event",
			    GTK_SIGNAL_FUNC (quit_game_callback), NULL);

	mbox = gtk_vbox_new (FALSE, 0);

	gnome_app_set_contents (GNOME_APP (window), mbox);
	create_mahjongg_board ();

	cc = gdk_color_context_new (gtk_widget_get_visual (draw_area),
				    gtk_widget_get_colormap (draw_area));
	color.red = 0;
	color.green = 0x3040;
	color.blue = 0x1030;
	color.pixel = 0;
	x = 0;
	gdk_color_context_get_pixels (cc,
				      &color.red,
				      &color.green,
				      &color.blue, 1,
				      &color.pixel, &x);
	gdk_window_set_background (draw_area->window, &color);
	
	gtk_widget_show (window);
	gtk_main ();
	gdk_color_context_free (cc);
	
	return 0;
}
