#ifndef BOARD_VIEW_H
#define BOARD_VIEW_H 1

#include <obgtk/obgtk.h>
#include "objc_inc.h"
#include "enums.h"

extern unsigned char pn[14][8];

#define D_color 3
#define D_highlighted 4
#define D_frozen 8
#define D_lazed 16

/************************************************************************/
/*                                                                      */
/************************************************************************/


@interface Board_View : Object
{
  Gtk_Dialog *window;

  Gtk_Button *fire_button0;
  Gtk_Button *pass_button0;

  Gtk_DrawingArea *draw;
  Gtk_DrawingArea *left_draw;
  Gtk_DrawingArea *right_draw;

  Gtk_Button *fire_button1;
  Gtk_Button *pass_button1;

  GdkPixmap *glyph_source;
  GdkBitmap *glyph_source_mask;

  GtkStyle  *style;
  gint depth;
  GdkGC *gc;

  GdkPixmap *draw_back;
  GdkPixmap *left_back;
  GdkPixmap *right_back;

  id *lb;
  Player_Color board_color;

  GdkColormap *colormap;
  GdkColor transparent;
  GdkColor black;
  GdkColor white;
  GdkColor red;
  GdkColor green;
  GdkColor blue;
  GdkColor grey;
  GdkColor grey30;

  id *bvps;
  id *gf;
}

- init_board : (int) argc
	     : (char **) argv;

- event : (GtkWidget *) w : (GdkEvent *) e;

- (void) delete_self;
- (void) set_logical_board : (id *) set_lb;
- (void) clear_board;
- (void) place_piece : (int) x : (int) y
		     : (int) kind_of_piece
		     : (int) direction
		     : (int) col;
- (void) clear_square : (int) x : (int) y;
- (void) place_beam : (int) x : (int) y : (int) direction;
- (void) remove_beam;
- (int) x_dir_add : (int) x : (int) dir;
- (int) y_dir_add : (int) y : (int) dir;
- (void) take_color : (Player_Color) c;
- (void) set_colors : (Player_Color) c;
- (void) update_moves : (int) red_moves : (int) green_moves;
- (void) send_message : (char *) message;
- (void) set_bvps : (id *) ps;
- (void) set_gf : (id *) new_gf;
- (void) menu_callback : (int) i;
@end

#endif /* BOARD_VIEW_H */
