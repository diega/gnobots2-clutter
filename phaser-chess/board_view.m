#include "board_view.h"
#include "logical_board.h"
#include "game_finder.h"
#include "board_view_ps.h"
#include "board_view_pc.h"

#define ELEMENTS(x) (sizeof (x) / sizeof (x [0]))

#define GRID_SZ 32
#define PCS_SZ 28
#define DIF ((GRID_SZ-PCS_SZ)/2)

#define MOVES_WIDTH 50
#define MOVES_HEIGHT 120

#define GAME_EVENTS (GDK_EXPOSURE_MASK        |\
		     GDK_BUTTON_PRESS_MASK    |\
		     GDK_BUTTON_RELEASE_MASK)

static int debug=0;

static Board_View *gbv; /* global board view */

unsigned char pn[14][8] =
{
    {39,  39,  39,  39,  39,  39,  39,  39},  // 0 (blank) (b)
    {4,   5,   6,   7,   0,   1,   2,   3 },  // 1 (freezer) (f)
    {20,  21,  22,  23,  24,  25,  26,  27},  // 2 (part mirror stomper) (p)
    {40,  41,  42,  43,  44,  45,  46,  47},  // 3 (one-way) (o)
    {60,  61,  62,  63,  64,  65,  66,  67},  // 4 (mirror) (m)
    {80,  81,  82,  83,  84,  85,  86,  87},  // 5 (laser) (l)
    {100, 101, 102, 103, 104, 105, 106, 107}, // 6 (splitter) (s)
    {16,  36,  16,  36,  16,  36,  16,  36},  // 7 (bomb) (n)
    {56,  56,  56,  56,  56,  56,  56,  56},  // 8 (king) (k)
    {76,  76,  76,  76,  76,  76,  76,  76},  // 9 (teleporter) (t)
    {18,  18,  18,  18,  18,  18,  18,  18},  // 10 (mirror stomper) (x)
    {58,  58,  58,  58,  58,  58,  58,  58},  // 11 (telegate) (g)
    {120, 120, 120, 120, 120, 120, 120, 120}, // 12 (centerpit) (c)
    {143, 143, 143, 143, 143, 143, 143, 143}  // 13 (pit) (v)
    /* 135 */
};

char color_jump[ 14 ] = {0, 8, 8, 8, 8, 8, 8, 1, 1, 1, 1, 1, 0, 0};


int pc_x (int kind_of_piece,
	  int direction,
	  int color_of_piece)
{
  if (color_of_piece == C_green)
    {
      /* green */
      return (pn[ kind_of_piece ][ direction ] +
	      color_jump[ kind_of_piece ]) * PCS_SZ;
    }

  /* red or neither */
  return (pn[ kind_of_piece ][ direction ]) * PCS_SZ;
}


void menu_bouncer (GtkWidget *widget, void *data)
{
  printf ("in menu callback... %d\n", (int) data);

  [gbv menu_callback : (int) data];
}


enum
{
  m_new_game,
  m_ask_meta,
  m_search_machine,
  m_req_col,
  m_quit
} menu_entries;

GtkMenuEntry same_menu [] =
{
  { "Game/New",            NULL, menu_bouncer, (gpointer) m_new_game},
  { "Game/Ask Metaserver", NULL, menu_bouncer, (gpointer) m_ask_meta},
  { "Game/Search Machine", NULL, menu_bouncer, (gpointer) m_search_machine},
  { "Game/Request Color",  NULL, menu_bouncer, (gpointer) m_req_col},
  { "Game/<separator>",    NULL, NULL, NULL },
  { "Game/Quit",           NULL, menu_bouncer, (gpointer) m_quit}
};


GtkMenuFactory *create_menu ()
{
  GtkMenuFactory *subfactory;
  
  subfactory = gtk_menu_factory_new  (GTK_MENU_FACTORY_MENU_BAR);
  gtk_menu_factory_add_entries (subfactory, same_menu, ELEMENTS(same_menu));
  
  return subfactory;
}



gint event_bouncer (GtkWidget *widget, GdkEvent *e, void *d)
{
  [((Board_View *) d) event : widget : e];

  return TRUE;
}


@implementation Board_View : Object


- paint_draw : (GdkRectangle *) a
{
  gdk_draw_pixmap (draw->gtkwidget->window, gc, draw_back,
		   a->x, a->y, a->x, a->y, a->width, a->height);

  return self;
}

- paint_left : (GdkRectangle *) a
{
  gdk_draw_pixmap (left_draw->gtkwidget->window, gc, left_back,
		   a->x, a->y, a->x, a->y, a->width, a->height);

  return self;
}

- paint_right : (GdkRectangle *) a
{
  gdk_draw_pixmap (right_draw->gtkwidget->window, gc, right_back,
		   a->x, a->y, a->x, a->y, a->width, a->height);

  return self;
}



- event : (GtkWidget *) w : (GdkEvent *) e
{
  switch (e->type)
    {
    case GDK_EXPOSE:
      {
	GdkEventExpose *ex_e = (GdkEventExpose *) e;
	if (w == draw->gtkwidget) [self paint_draw : &ex_e->area];
	if (w == left_draw->gtkwidget) [self paint_left : &ex_e->area];
	if (w == right_draw->gtkwidget) [self paint_right : &ex_e->area];
	break;
      }
    case GDK_BUTTON_PRESS: 
      {
	GdkEventButton *b_e = (GdkEventButton *) e;
	int bx = b_e->x / GRID_SZ;
	int by = b_e->y / GRID_SZ;
	char *message;


	if (bx < 0 || bx >= 15 || by < 0 || by >= 11)
	  return;

	if (debug)
	  printf (" mouse button press... lb = %p\n", lb);

	message = [(Logical_Board *) lb mouse_button_press
				     : bx
				     : by
				     : b_e->button - 1
				     : board_color];

	if (debug)
	  printf (" board_view -- printing message after button press...\n");

	if (message != NULL)
	  printf ("  %s\n", message);

	if (debug)
	  printf (" board_view -- done printing message\n");

	break;
      }
    case GDK_BUTTON_RELEASE: 
      break;
    default:
      break;
    }

  if (debug)
    printf (" board_view -- event -- after switch\n");

  return self;
}


- init_board : (int) argc
	     : (char **) argv
{
  Gtk_VBox *wvbox;
  Gtk_HBox *hbox;
  Gtk_VBox *vbox0;
  Gtk_VBox *vbox1;
  GtkMenuFactory *mf;

  [super init];

  srand (time (0));

  /* put the board together... */

  wvbox = [[[Gtk_VBox alloc] initWithBoxInfo : 0 setSpacing : 0] show];
  hbox = [[[Gtk_HBox alloc] initWithBoxInfo : 0 setSpacing : 0] show];
  vbox0 = [[[Gtk_VBox alloc] initWithBoxInfo : 0 setSpacing : 0] show];
  vbox1 = [[[Gtk_VBox alloc] initWithBoxInfo : 0 setSpacing : 0] show];

  fire_button0 = [[[Gtk_Button alloc] initWithLabel : "Fire"] show];
  pass_button0 = [[[Gtk_Button alloc] initWithLabel : "Pass"] show];
  left_draw = [[[Gtk_DrawingArea alloc] init] show];
  [left_draw size : MOVES_WIDTH aHeight: MOVES_HEIGHT];

  draw = [[[Gtk_DrawingArea alloc] init] show];
  [draw size : GRID_SZ*15 aHeight: GRID_SZ*11];
  
  fire_button1 = [[[Gtk_Button alloc] initWithLabel : "Fire"] show];
  pass_button1 = [[[Gtk_Button alloc] initWithLabel : "Pass"] show];
  right_draw = [[[Gtk_DrawingArea alloc] init] show];
  [right_draw size : MOVES_WIDTH aHeight: MOVES_HEIGHT];


  gdk_window_get_geometry (draw->gtkwidget->window,
			   NULL, NULL, NULL, NULL, &depth);

  draw_back = gdk_pixmap_new (draw->gtkwidget->window,
			      GRID_SZ*15, GRID_SZ*11, depth);
  left_back = gdk_pixmap_new (draw->gtkwidget->window,
			      MOVES_WIDTH, MOVES_HEIGHT, depth);
  right_back = gdk_pixmap_new (draw->gtkwidget->window,
			       MOVES_WIDTH, MOVES_HEIGHT, depth);

  [fire_button0 connectObj : "pressed" : self];
  [fire_button0 connectObj : "released" : self];
  [pass_button0 connectObj : "clicked" : self];
  [fire_button1 connectObj : "pressed" : self];
  [fire_button1 connectObj : "released" : self];
  [pass_button1 connectObj : "clicked" : self];

  gtk_widget_set_events (draw->gtkwidget,
			 gtk_widget_get_events (draw->gtkwidget) |
			 GAME_EVENTS);
  gtk_signal_connect (GTK_OBJECT(draw->gtkwidget), "event",
		      (GtkSignalFunc) event_bouncer, self);


  gtk_widget_set_events (left_draw->gtkwidget, GDK_EXPOSURE_MASK);
  gtk_signal_connect (GTK_OBJECT(left_draw->gtkwidget), "event",
		      (GtkSignalFunc) event_bouncer, self);

  gtk_widget_set_events (right_draw->gtkwidget, GDK_EXPOSURE_MASK);
  gtk_signal_connect (GTK_OBJECT(right_draw->gtkwidget), "event",
		      (GtkSignalFunc) event_bouncer, self);


  window = [[Gtk_Window alloc] initWithWindowType : GTK_WINDOW_TOPLEVEL];
  [window signal_connect
	   : "destroy"
	   signalFunc:(GtkSignalFunc)gtk_exit
	   funcData:0];

  [vbox0 add : fire_button0];
  [vbox0 add : pass_button0];
  [vbox0 add : left_draw];

  [vbox1 add : fire_button1];
  [vbox1 add : pass_button1];
  [vbox1 add : right_draw];

  mf = create_menu ();
  gtk_widget_show (mf->widget);

  gtk_container_add (GTK_CONTAINER (wvbox->gtkwidget), mf->widget);
  [wvbox add : hbox];
  [hbox add : vbox0];
  [hbox add : draw];
  [hbox add : vbox1];
  [window add : wvbox];

  [window show];

  style = gtk_widget_get_style (draw->gtkwidget);

  glyph_source = NULL;

  gc = gdk_gc_new (draw->gtkwidget->window);
  gdk_gc_set_exposures (gc, 0);
  gdk_gc_set_function (gc, GDK_COPY);
  gdk_gc_set_fill (gc, GDK_SOLID);
  gdk_gc_set_line_attributes (gc, 1,
			      GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND);
  gdk_gc_set_background (gc, &black);


  board_color = C_neutral;
  /*board_color = C_both;*/

  colormap = gtk_widget_get_colormap (draw->gtkwidget);

  black.red = 0; black.green = 0; black.blue = 0;
  gdk_color_alloc (colormap, &black);
  white.red = 0xFFFF; white.green = 0xFFFF; white.blue = 0xFFFF;
  gdk_color_alloc (colormap, &white);
  red.red = 0xFFFF; red.green = 0x0; red.blue = 0;
  gdk_color_alloc (colormap, &red);
  green.red = 0; green.green = 0xFFFF; green.blue = 0;
  gdk_color_alloc (colormap, &green);
  blue.red = 0; blue.green = 0x0; blue.blue = 0xFFFF;
  gdk_color_alloc (colormap, &blue);
  grey.red = 0x3000; grey.green = 0x3000; grey.blue = 0x3000;
  gdk_color_alloc (colormap, &grey);
  grey30.red = 0x4D4D; grey30.green = 0x4D4D; grey30.blue = 0x4D4D;
  gdk_color_alloc (colormap, &grey30);

  glyph_source = gdk_pixmap_create_from_xpm (window->gtkwidget->window,
					     &glyph_source_mask,
					     &transparent,
					     "phaser-chess-pieces.xpm");


  /* paint everything black to start out */

  gdk_gc_set_foreground (gc, &black);
  gdk_draw_rectangle (draw->gtkwidget->window, gc, TRUE,
		      0, 0, GRID_SZ*15, GRID_SZ*11);
  gdk_draw_rectangle (left_draw->gtkwidget->window, gc, TRUE,
		      0, 0, MOVES_WIDTH, MOVES_HEIGHT);
  gdk_draw_rectangle (right_draw->gtkwidget->window, gc, TRUE,
		      0, 0, MOVES_WIDTH, MOVES_HEIGHT);
  gdk_draw_rectangle (draw_back, gc, TRUE, 0, 0, GRID_SZ*15, GRID_SZ*11);
  gdk_draw_rectangle (left_back, gc, TRUE, 0, 0, MOVES_WIDTH, MOVES_HEIGHT);
  gdk_draw_rectangle (right_back, gc, TRUE, 0, 0, MOVES_WIDTH, MOVES_HEIGHT);


  [self clear_board];

  lb = NULL;

  gbv = self;
  return self;
}


- (void) set_logical_board : (id *) set_lb
{
    lb = set_lb;
}


- (void) clear_square : (int) x : (int) y
{
    int bgc = (x + y) % 2;

    gdk_gc_set_foreground (gc, (bgc == 0) ? &black : &grey30);

    gdk_draw_rectangle (draw->gtkwidget->window, gc, TRUE,
			GRID_SZ*x+DIF, GRID_SZ*y+DIF, PCS_SZ, PCS_SZ);

    gdk_draw_rectangle (draw_back, gc, TRUE,
			GRID_SZ*x+DIF, GRID_SZ*y+DIF, PCS_SZ, PCS_SZ);
}


- (void) clear_board
{
    int x;
    int y;

    for (x=0; x<15; x++)
      for (y=0; y<11; y++)
	[self clear_square : x : y];
}


- (void) mouse_button_press : (int) x : (int) y : (int) n
{
    int bx = x/GRID_SZ;
    int by = y/GRID_SZ;

    if (bx < 0 || bx >= 15 || by < 0 || by >= 11)
	return;

    [(Logical_Board *) lb mouse_button_press : bx : by : n : board_color];
}


- (void) key_press : (int) key
{
    [(Logical_Board *) lb key_press : key];
}


- (void) draw_moves_border : (Gtk_DrawingArea *) fore
			   : (GdkPixmap *) back
			   : (GdkColor *) c
			   : (int) this_board_controls
{
  int width, height;
  int m = 5; /* margin */
  int x,y,w,h; /* box for dots */
  int d; /* diameter of dots */
  int i;

  /*XtVaGetValues (fore, XmNwidth, &width, XmNheight, &height, NULL);*/

  width = MOVES_WIDTH;
  height = MOVES_HEIGHT;

  w = (width*4)/5 - (2 * m);
  d = w - 2;
  h = (d+2) * 3;
  x = (width/2) - (w/2);
  y = m;

  if (this_board_controls)
    gdk_gc_set_foreground (gc, c);
  else
    gdk_gc_set_foreground (gc, &black);

  for (i=1; i<=m; i++)
    gdk_draw_rectangle (back, gc, FALSE, x-i, y-i, w+2*i, h+2*i);
}



- (void) refresh_moves : (Gtk_DrawingArea *) fore
		       : (GdkPixmap *) back
  		       : (int) moves
		       : (GdkColor *) c
		       : (int) this_board_controls
{
  gint width, height;
  int m = 5; /* margin */
  int x,y,w,h; /* box for dots */
  int d; /* diameter of dots */
  int i;

  gdk_window_get_geometry (fore->gtkwidget->window,
			   NULL, NULL, &width, &height, NULL);

  gdk_gc_set_foreground (gc, &grey30);

  gdk_draw_rectangle (back, gc, TRUE, 0, 0, width, height);

  w = (width*4)/5 - (2 * m);
  d = w - 2;
  h = (d+2) * 3;
  x = (width/2) - (w/2);
  y = m;


  if (this_board_controls)
    gdk_gc_set_foreground (gc, c);
  else
    gdk_gc_set_foreground (gc, &black);


  for (i=1; i<=m; i++)
    gdk_draw_rectangle (back, gc, FALSE, x-i, y-i, w+2*i, h+2*i);

  gdk_gc_set_foreground (gc, c);

  for (i=0; i<moves; i++)
    gdk_draw_arc (back, gc, TRUE,
		  x+1, y+1 + (d+2) * i,
		  d, d,
		  0, 360 * 64);

  gdk_draw_pixmap (fore->gtkwidget->window, gc, back,
		   0, 0, 0, 0, width, height);
}


- (void) place_piece : (int) x : (int) y
		     : (int) kind_of_piece
		     : (int) direction
		     : (int) col
{
    int bgc = (x + y) % 2;
    GdkColor *bg, *fg;
    int sx, sy; /* source */
    int dx, dy; /* destination */

    if (x < 0 || x >= 15)
    {
	fprintf (stderr, "place_piece: x out of range: %d\n", x);
	return;
    }

    if (y < 0 || y >= 11)
    {
	fprintf (stderr, "place_piece: y out of range: %d\n", y);
	return;
    }

    if (col & D_highlighted)
	bg = &grey;
    else
	bg = ((bgc == 0) ? &black : &grey30);


    /* x and y within the source pixmap */
    sx = pc_x (kind_of_piece, direction, col & D_color);
    sy = 0;

    dx = GRID_SZ * x + DIF;
    dy = GRID_SZ * y + DIF;


    /* draw the background square */
    gdk_gc_set_foreground (gc, bg);

    gdk_draw_rectangle (draw_back, gc, TRUE,
			dx, dy, PCS_SZ, PCS_SZ);

    fg = NULL;

    if (col & D_frozen) fg = &blue;
    if (col & D_lazed) fg = &white;

    /* set a clipping mask so that the background doesn't get clobbered */
    gdk_gc_set_clip_origin (gc, dx - sx, dy);
    gdk_gc_set_clip_mask (gc, glyph_source_mask);


    if (fg == NULL)
      gdk_draw_pixmap (draw_back, gc, glyph_source,
		       sx, sy, dx, dy, PCS_SZ, PCS_SZ);
    else
      {
	gdk_gc_set_foreground (gc, fg);
	gdk_draw_rectangle (draw_back, gc, TRUE, dx, dy, PCS_SZ, PCS_SZ);
      }

    gdk_gc_set_clip_mask (gc, NULL);

    gdk_draw_pixmap (draw->gtkwidget->window, gc, draw_back,
		     dx, dy, dx, dy, PCS_SZ, PCS_SZ);
}
    



- (void) place_beam : (int) x : (int) y
		    : (int) direction
{
    int x1, x2, y1, y2;

    x1 = x * GRID_SZ + (GRID_SZ/2);
    y1 = y * GRID_SZ + (GRID_SZ/2);

    x2 = [self x_dir_add
	       : x
	       : direction] * GRID_SZ + (GRID_SZ/2);
    y2 = [self y_dir_add
	       : y
	       : direction] * GRID_SZ + (GRID_SZ/2);

    x2 = (x1+x2)/2;
    y2 = (y1+y2)/2;

    gdk_gc_set_foreground (gc, &white);
    gdk_draw_line (draw->gtkwidget->window, gc, x1, y1, x2, y2);
}


- (void) remove_beam
{
  gdk_draw_pixmap (draw->gtkwidget->window, gc, draw_back,
		   0, 0, 0, 0, 15*GRID_SZ, 11*GRID_SZ);
}


- (void) use_devil
{
    int lop;
    for (lop=0;lop<8;lop++)
    {
	pn[13][lop]=135;
    }
}


- (void) left_fire_event
{
    [(Logical_Board *) lb fire : C_red & board_color];
}

- (void) left_pass_event
{
    [(Logical_Board *) lb pass : C_red & board_color];
}

- (void) right_fire_event
{
    [(Logical_Board *) lb fire : C_green & board_color];
}

- (void) right_pass_event
{
    [(Logical_Board *) lb pass : C_green & board_color];
}

- (void) unfire
{
    [(Logical_Board *) lb unfire];
}


- clicked : (id) anobj
{
  if (anobj == pass_button0)
    [self left_pass_event];

  if (anobj == pass_button1)
    [self right_pass_event];

  return self;
}


- pressed : (id) anobj
{
  if (anobj == fire_button0)
    [self left_fire_event];
  if (anobj == fire_button1)
    [self right_fire_event];

  return self;
}


- released : (id) anobj
{
  if (anobj == fire_button0)
    [self unfire];
  if (anobj == fire_button1)
    [self unfire];

  return self;
}


- (void) delete_self
{
  if (lb != NULL)
    [(Logical_Board *) lb delete_board_view : self];

#if 0
  /* XtUnrealizeWidget (shell); */
  XtDestroyWidget (shell);

  /* free pixmaps, etc */
#endif

  [self free];
}


- (int) x_dir_add : (int) x : (int) dir
{
    switch (dir)
    {
    case 0:
    case 4:
	return x;
    case 1:
    case 2:
    case 3:
	return x+1;
    case 5:
    case 6:
    case 7:
	return x-1;
    default:
	fprintf (stderr, "bad direction in dir_add: %d\n", dir);
	return 0;
    }
}


- (int) y_dir_add : (int) y : (int) dir
{
    switch (dir)
    {
    case 2:
    case 6:
	return y;
    case 3:
    case 4:
    case 5:
	return y+1;
    case 7:
    case 0:
    case 1:
	return y-1;
    default:
	fprintf (stderr, "bad direction in dir_add: %d\n", dir);
	return 0;
    }
}

- (void) take_color : (Player_Color) c
{
  board_color |= c;
  [self draw_moves_border
	: left_draw
	: left_back
	: &red
	: board_color & C_red];
  [self draw_moves_border
	: right_draw
	: right_back
	: &green
	: board_color & C_green];
}


- (void) set_colors : (Player_Color) c
{
  board_color = c;
  [self draw_moves_border
	: left_draw
	: left_back
	: &red
	: board_color & C_red];
  [self draw_moves_border
	: right_draw
	: right_back
	: &green
	: board_color & C_green];
}


- (void) update_moves : (int) red_moves : (int) green_moves
{
  [self refresh_moves
	: left_draw
	: left_back
	: red_moves
	: &red
	: board_color & C_red];
  [self refresh_moves
	: right_draw
	: right_back
	: green_moves
	: &green
	: board_color & C_green];
}


- (void) send_message : (char *) message
{
  printf ("%s\n", message);
}


- (void) set_bvps : (id *) ps
{
  bvps = ps;
}


- (void) set_gf : (id *) new_gf
{
  gf = new_gf;
}


- (void) menu_callback : (int) i
{
  switch (i)
    {
    case m_new_game:
      [(Game_Finder *) gf new_game];
      break;
    case m_ask_meta:
      [(Board_View_PS *) bvps ask_metaserver];
      break;
    case m_search_machine:
      [(Game_Finder *) gf search_machine];
      break;
    case m_req_col:
      [(Logical_Board_PC *) lb request_color];
      break;
    case m_quit:
      exit (0);
    default:
      fprintf (stderr, "menu problem\n");
      break;
    }
}

@end
