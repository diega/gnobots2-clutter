#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <gtk/gtk.h>
#include <gtk/gtkmain.h>
#include <gtk/gtksignal.h>
#include <glib-object.h>
#include <cairo/cairo.h>
#include <math.h>

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include "gtkgridboard.h"

/* A Note on the Drawing Model
 *
 * Because Cairo is not "pixel-perfect" we can't easily update small
 * sections of the canvas as we would with traditional X drawing
 * code. So instead of trying to be clever about redrawing small parts
 * of the canvas as the model is updated we don't do any drawing. It
 * is the duty of the program to call a gtk_gridboard_paint once they
 * have finished updating the state of the model. By delaying drawing
 * this way we can avoid a lot of unnecessary redraws and avoid
 * synchronisation issues with the data. The down-side is that any
 * code using these routines have to remember to call
 * gtk_gtidboard_paint explicitly or risk having an inconsistent
 * display. It is sufficient to call gtk_gridboard_paint at the end of
 * each event-handler that changes the state of the board. The only
 * explicit calls to gtk_gridboard_paint in this code occur after the
 * widget is initially allocated and at after the animation timer
 * triggers.
 *
 * gtk_gridboard_repaint handles all the actual drawing and should
 * only be called from the expose event handler.
 *
 */

#define DO_PROFILE

#ifdef DO_PROFILE
#include <sys/time.h>

/* A temporary profiling counter. Only supports one instance.*/
long int counter;
#endif

static GtkWidgetClass *parent_class=NULL;
static guint gridboard_signals[LAST_SIGNAL] = { 0 };

/* prototypes for private functions */
static void gtk_gridboard_repaint(GtkGridBoard * gridboard);
static gint ** make_array(int width, int height); 
static void gtk_gridboard_load_tiles (GtkGridBoard * gridboard);
static gint get_pixmap_num(int piece);
static gint gtk_gridboard_button_press(GtkWidget *widget, 
				       GdkEventButton *event) ;
static gboolean gtk_gridboard_expose(GtkWidget * widget, 
				     GdkEventExpose * event);
static void gtk_gridboard_size_allocate(GtkWidget * widget, 
					GtkAllocation * allocation);
static void gtk_gridboard_class_init(GtkGridBoardClass * class);
static void gtk_gridboard_finalize(GObject * object);
static void gtk_gridboard_init(GtkGridBoard * gridboard);
static void gtk_gridboard_realize(GtkWidget * widget);
static void gtk_gridboard_size_request(GtkWidget * widget, GtkRequisition * req);
static void gtk_gridboard_draw_pixmap(GtkGridBoard * gridboard, gdouble phase, 
				      gdouble x, gdouble y, 
				      cairo_pattern_t *pattern);
static gint gtk_gridboard_flip_pixmaps(gpointer data);

/* init functions */

/* init structures and create signals */
static void gtk_gridboard_class_init(GtkGridBoardClass * class) {
        GObjectClass *object_class;
        GtkWidgetClass *widget_class;

        object_class = (GObjectClass*) class;
        widget_class = (GtkWidgetClass*) class;

        parent_class = gtk_type_class (gtk_widget_get_type ());

        object_class->finalize = gtk_gridboard_finalize;
        widget_class->realize = gtk_gridboard_realize;
        widget_class->expose_event = gtk_gridboard_expose;
	widget_class->size_allocate = gtk_gridboard_size_allocate;
	widget_class->size_request = gtk_gridboard_size_request;
        widget_class->button_press_event = gtk_gridboard_button_press;
        
        gridboard_signals[BOXCLICKED] = g_signal_new ("boxclicked",
                  G_TYPE_FROM_CLASS (class),
                  G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
                  G_STRUCT_OFFSET (GtkGridBoardClass, boxclicked),
                  NULL, NULL,
                  g_cclosure_user_marshal_VOID__INT_INT, G_TYPE_NONE, 2,
                  G_TYPE_INT, G_TYPE_INT);
}

static void gtk_gridboard_init(GtkGridBoard * gridboard) {
	gridboard->tiles_pixbuf = NULL;
	gridboard->tiles_scale = 1;
        gridboard->tileset=NULL;
        gridboard->showgrid=FALSE;
        gridboard->pixmaps=NULL;
	gridboard->timeoutid = 0;
        gridboard->tileheight=TILEHEIGHT;
        gridboard->tilewidth=TILEWIDTH;
        gridboard->width=0;
        gridboard->height=0;
	gridboard->statelist=NULL;
	gridboard->themesurface = NULL;
        gridboard->transform.xx = 1.0;
        gridboard->transform.xy = 0.0;
        gridboard->transform.yy = 1.0;
        gridboard->transform.yx = 0.0;
        gridboard->transform.x0 = 0.0;
        gridboard->transform.y0 = 0.0;
}

GtkWidget * gtk_gridboard_new(gint width, gint height, char * tileset) {
        GtkGridBoard * gridboard=g_object_new(gtk_gridboard_get_type(), NULL);

        gridboard->width=width;
        gridboard->height=height;
        gridboard->tileset=tileset;

        gridboard->board=make_array(width, height);
        gridboard->pixmaps=make_array(width, height);
        gridboard->selected=make_array(width, height);

        gtk_gridboard_load_tiles(gridboard);
        gtk_gridboard_set_animate(gridboard, TRUE);
        gtk_gridboard_set_visibility(gridboard, TRUE);
        
        return GTK_WIDGET(gridboard);
        
}

/* class functions */ 

GtkType gtk_gridboard_get_type() {
        static GtkType gridboard_type=0;

        if (!gridboard_type) {
                static GTypeInfo gridboard_info={
                        sizeof(GtkGridBoardClass),
                        NULL,
                        NULL,
                        (GClassInitFunc) gtk_gridboard_class_init,
                        NULL,
                        NULL,
                        sizeof(GtkGridBoard),
                        0,
                        (GInstanceInitFunc) gtk_gridboard_init,
                };
                gridboard_type=g_type_register_static (GTK_TYPE_WIDGET,
                                         "GridBoard",
                                         &gridboard_info,
                                         0);
        }
        return gridboard_type;
}

/* event handlers */

static gint gtk_gridboard_button_press(GtkWidget *widget,
				       GdkEventButton *event)
 {
   GtkGridBoard *gridboard;
   int x, y;

   g_return_val_if_fail (GTK_IS_GRIDBOARD(widget), FALSE);
   
   gridboard = GTK_GRIDBOARD (widget);

   x = event->x * gridboard->width / widget->allocation.width;
   y = event->y * gridboard->height / widget->allocation.height;

   x = CLAMP (x, 0, gridboard->width - 1);
   y = CLAMP (y, 0, gridboard->height - 1);

   g_signal_emit (G_OBJECT(gridboard), gridboard_signals[BOXCLICKED], 
		  0, x, y);

   return TRUE;
}

static gboolean gtk_gridboard_expose(GtkWidget *widget,
				     GdkEventExpose * event)
 {
        GtkGridBoard *gridboard;

	if (!GTK_WIDGET_DRAWABLE (widget))
	  return FALSE;

	gridboard = GTK_GRIDBOARD (widget);

	gtk_gridboard_repaint (gridboard);

        return FALSE;
}

static void gtk_gridboard_size_allocate (GtkWidget * widget,
					 GtkAllocation *allocation)
{
  GtkGridBoard *gridboard = GTK_GRIDBOARD (widget);

  widget->allocation = *allocation;
 
  if (GTK_WIDGET_REALIZED (widget)) {
    gdk_window_move_resize (widget->window, allocation->x, allocation->y,
			    allocation->width, allocation->height);
  }

  gtk_gridboard_paint (gridboard);
}

static void gtk_gridboard_realize(GtkWidget * widget) 
{
        GdkWindowAttr attributes;
        gint attributes_mask;

        g_return_if_fail (GTK_IS_WIDGET (widget));

        GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);
	
        attributes.x = widget->allocation.x;
        attributes.y = widget->allocation.y;
        attributes.width = widget->allocation.width;
        attributes.height = widget->allocation.height;
        attributes.wclass = GDK_INPUT_OUTPUT;
        attributes.window_type = GDK_WINDOW_CHILD;
        attributes.event_mask = gtk_widget_get_events (widget) | 
                GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK | 
                GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK |
                GDK_POINTER_MOTION_HINT_MASK;
        attributes.visual = gtk_widget_get_visual (widget);
        attributes.colormap = gtk_widget_get_colormap (widget);

        attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;
        widget->window = gdk_window_new (widget->parent->window, &attributes, attributes_mask);

        widget->style = gtk_style_attach (widget->style, widget->window);

        gdk_window_set_user_data (widget->window, widget);

        gtk_style_set_background (widget->style, widget->window, GTK_STATE_ACTIVE);
	
}

static void gtk_gridboard_finalize(GObject * object) {
        GtkGridBoard *gridboard;

        g_return_if_fail (object != NULL);
        g_return_if_fail (GTK_IS_GRIDBOARD (object));

        gridboard = GTK_GRIDBOARD(object);

        g_free(gridboard->board);
        g_free(gridboard->pixmaps);
        g_free(gridboard->selected);

        if (G_OBJECT_CLASS (parent_class)->finalize)
                (* G_OBJECT_CLASS (parent_class)->finalize) (object);
}

static void gtk_gridboard_size_request(GtkWidget * widget, GtkRequisition * req)
 {
        GtkGridBoard *gridboard;

	g_return_if_fail (GTK_IS_GRIDBOARD (widget));
	gridboard = GTK_GRIDBOARD (widget);

        req->width=gridboard->width*gridboard->tilewidth + 1;
        req->height=gridboard->height*gridboard->tileheight + 1;
}

/* drawing functions */

static gint get_pixmap_num(int piece) {
        if (piece==BLACK) return BLACK_PIXMAP;
        if (piece==WHITE) return WHITE_PIXMAP;
        return EMPTY_PIXMAP;
}

static void gtk_gridboard_draw_grid (GtkGridBoard *gridboard)
{
  gint x, y;
  gboolean selected;

  cairo_save (gridboard->cx);

  cairo_set_source_rgba (gridboard->cx, 0.0, 0.0, 0.0, 1.0);
  cairo_set_line_width (gridboard->cx, 0.01);

  /* This all looks very complicated, but all we are trying to do is
   * draw each individual line segment based on its surrounding squares. */

  /* First we do the vertical segments. */
  for (x=0; x<=gridboard->width; x++) {
    for (y=0; y<gridboard->height; y++) {
      selected = FALSE;
      if (x < gridboard->width)
	selected |= gridboard->selected[x][y];
      if (x > 0)
	selected |= gridboard->selected[x-1][y];

      cairo_move_to (gridboard->cx, x, y);
      if (selected) {
	cairo_set_source_rgba (gridboard->cx, 1.0, 1.0, 1.0, 1.0);
	cairo_line_to (gridboard->cx, x, y+1);
	cairo_stroke (gridboard->cx);
      } else  if (gridboard->showgrid) {
	cairo_set_source_rgba (gridboard->cx, 0.0, 0.0, 0.0, 1.0);
	cairo_line_to (gridboard->cx, x, y+1);
	cairo_stroke (gridboard->cx);
      }
    }
  }

  /* Then we do the horizontal segments. */
  for (x=0; x<gridboard->width; x++) {
    for (y=0; y<=gridboard->height; y++) {
      selected = FALSE;
      if (y < gridboard->height)
	selected |= gridboard->selected[x][y];
      if (y > 0)
	selected |= gridboard->selected[x][y-1];

      cairo_move_to (gridboard->cx, x, y);
      if (selected) {
	cairo_set_source_rgba (gridboard->cx, 1.0, 1.0, 1.0, 1.0);
	cairo_line_to (gridboard->cx, x+1, y);
	cairo_stroke (gridboard->cx);
      } else  if (gridboard->showgrid) {
	cairo_set_source_rgba (gridboard->cx, 0.0, 0.0, 0.0, 1.0);
	cairo_line_to (gridboard->cx, x+1, y);
	cairo_stroke (gridboard->cx);
      }
    }
  }

  cairo_restore (gridboard->cx);

  /* Another way, probably a better way, would be to collect the line
   * segments into two lists (one for each colour) and then render
   * each list with one cairo_stroke call. We could also collect the
   * highlighting segments and make them into polygons and use a
   * fill-colour with an alpha value.
   */
}

void gtk_gridboard_paint (GtkGridBoard *gridboard)
{
  GtkWidget *widget;

  g_return_if_fail (GTK_IS_GRIDBOARD (gridboard));
  
  widget = GTK_WIDGET (gridboard);
  g_return_if_fail (GTK_WIDGET_REALIZED (widget));

  gtk_widget_queue_draw (widget);
}

static void gtk_gridboard_repaint(GtkGridBoard * gridboard) {
        int x, y;
        double phase;
	GtkWidget *widget;
        gdouble scale;
	cairo_t *themecx;
	cairo_pattern_t *gradient;
	gint w, h;

#ifdef DO_PROFILE
	struct timeval tv;
#endif

        g_return_if_fail (GTK_IS_GRIDBOARD (gridboard));
	if (!gridboard->visibility) return;

	widget = GTK_WIDGET (gridboard);
	g_return_if_fail (GTK_WIDGET_REALIZED (widget));

	gridboard->cx = gdk_cairo_create (widget->window);
	cairo_scale (gridboard->cx, 
		     1.0*widget->allocation.width/gridboard->width,
		     1.0*widget->allocation.height/gridboard->height);

	/* Make a new cairo surface with the background drawn on it. */
	/* FIXME: Should be in the realize callback. */
	/* FIXME: This doesn't allow for the theme to change at all :( */
	if (gridboard->themesurface == NULL) {
#ifdef DO_PROFILE
	gettimeofday (&tv, NULL);
	counter = tv.tv_sec * 1000000 + tv.tv_usec;
#endif
	  w = gdk_pixbuf_get_width (gridboard->tiles_pixbuf);
	  h = gdk_pixbuf_get_height (gridboard->tiles_pixbuf);

	  gridboard->themesurface = 
	    cairo_surface_create_similar (cairo_get_target (gridboard->cx), 
					  CAIRO_CONTENT_COLOR_ALPHA,
					  w, h);
	  themecx = cairo_create (gridboard->themesurface);
	  gdk_cairo_set_source_pixbuf (themecx, gridboard->tiles_pixbuf,
				       0.0, 0.0);
	  cairo_paint (themecx);
#ifdef DO_PROFILE
	gettimeofday (&tv, NULL);
	g_print ("[%ld]\n", tv.tv_sec * 1000000 + tv.tv_usec - counter);
#endif
	}

#ifdef DO_PROFILE
	gettimeofday (&tv, NULL);
	counter = tv.tv_sec * 1000000 + tv.tv_usec;
#endif

	cairo_set_source_rgba (gridboard->cx, 0.0, 0.0, 0.3, 1.0);
	cairo_paint (gridboard->cx);

#ifdef DO_PROFILE
	gettimeofday (&tv, NULL);
	g_print ("%ld, ", tv.tv_sec * 1000000 + tv.tv_usec - counter);
	counter = tv.tv_sec * 1000000 + tv.tv_usec;
#endif

	scale = gridboard->tiles_scale;

	cairo_save (gridboard->cx);
	//	cairo_scale (gridboard->cx, 1.0/scale, 1.0/scale);
	//	cairo_set_source_surface (gridboard->cx, gridboard->themesurface,
	//				  0, 0);
	gradient = cairo_pattern_create_radial (-0.1, -0.1, 0, 
						0, 0, 1.0);
	cairo_pattern_add_color_stop_rgba (gradient, 0.0, 
					   1.0, 1.0, 1.0, 0.2);
	cairo_pattern_add_color_stop_rgba (gradient, 0.55, 
					   0.0, 0.0, 0.0, 0.2);
        for (x=0; x<gridboard->height; x++) {
                for (y=0; y<gridboard->width; y++) {
                        phase = gridboard->pixmaps[x][y];
			if (phase > 0.0) {
			  phase = (phase - 1.0)/30.0;
			  gtk_gridboard_draw_pixmap(gridboard, phase, 
						    x, y, 
						    gradient);
			}
                }
        }
	cairo_pattern_destroy (gradient);
	cairo_restore (gridboard->cx);

#ifdef DO_PROFILE
	gettimeofday (&tv, NULL);
	g_print ("%ld, ", tv.tv_sec * 1000000 + tv.tv_usec - counter);
	counter = tv.tv_sec * 1000000 + tv.tv_usec;
#endif

	gtk_gridboard_draw_grid (gridboard);

#ifdef DO_PROFILE
	gettimeofday (&tv, NULL);
	g_print ("%ld\n", tv.tv_sec * 1000000 + tv.tv_usec - counter);
	counter = tv.tv_sec * 1000000 + tv.tv_usec;
#endif

	cairo_destroy (gridboard->cx);

#ifdef DO_PROFILE
	gettimeofday (&tv, NULL);
	g_print ("(%ld)\n", tv.tv_sec * 1000000 + tv.tv_usec - counter);
#endif
 }


static void gtk_gridboard_draw_pixmap(GtkGridBoard * gridboard, gdouble phase, 
				      gdouble x, gdouble y, 
				      cairo_pattern_t *pattern) 
{
  cairo_matrix_t transform;
  double d;
  double offset;

  /* FIXME: I define this here, but I am already impliclty using it up 
     when defining things above like the pattern. */
#define BALL_RADIUS 0.4

  x = x + 0.5;
  y = y + 0.5;

  /* We special-case the two "pure" ends of the flip. */
  if (phase > 0.98) {
    cairo_set_source_rgb (gridboard->cx, 0.7, 0.7, 0.7);
    cairo_arc (gridboard->cx, x, y, BALL_RADIUS, 0, 2*G_PI);
    cairo_fill (gridboard->cx);
  } else if (phase < 0.02) {
    cairo_set_source_rgb (gridboard->cx, 0.0, 0.0, 0.0);
    cairo_arc (gridboard->cx, x, y, BALL_RADIUS, 0, 2*G_PI);
    cairo_fill (gridboard->cx);
  } else {
    cairo_save (gridboard->cx);
    cairo_translate (gridboard->cx, x, y);
    cairo_rotate (gridboard->cx, G_PI/4.0);
    cairo_save (gridboard->cx);
    d = BALL_RADIUS;
    offset = d*cos (phase*M_PI);
    cairo_arc (gridboard->cx, 0, 0, BALL_RADIUS, -G_PI/2.0, G_PI/2.0);
    cairo_line_to (gridboard->cx, 0, d);
    cairo_scale (gridboard->cx, cos (phase*M_PI), 1.0);
    cairo_arc (gridboard->cx, 0, 0, BALL_RADIUS, G_PI/2.0, -G_PI/2.0);
    cairo_close_path (gridboard->cx);
    cairo_set_source_rgb (gridboard->cx, 0, 0, 0);
    cairo_fill (gridboard->cx);
    cairo_restore (gridboard->cx);
    cairo_arc (gridboard->cx, 0, 0, BALL_RADIUS, G_PI/2.0, -G_PI/2.0);
    cairo_line_to (gridboard->cx, 0, -d);
    cairo_scale (gridboard->cx, cos (phase*M_PI), 1.0);
    cairo_arc_negative (gridboard->cx, 0, 0, BALL_RADIUS, -G_PI/2.0, G_PI/2.0);
    cairo_close_path (gridboard->cx);
    cairo_set_source_rgb (gridboard->cx, 0.7, 0.7, 0.7);
    cairo_fill (gridboard->cx);
    cairo_restore (gridboard->cx);
  }
  cairo_matrix_init_translate (&transform, -x, -y);
  cairo_pattern_set_matrix (pattern, &transform);
  cairo_arc (gridboard->cx, x, y, BALL_RADIUS, 0, 2*G_PI);
  cairo_set_source (gridboard->cx, pattern);
  cairo_fill (gridboard->cx);

#undef BALL_RADIUS
}

void gtk_gridboard_set_selection(GtkGridBoard * gridboard, gint type, 
				 gint x, gint y) 
{
        g_return_if_fail (GTK_IS_GRIDBOARD (gridboard));
	g_return_if_fail (x >= 0);
	g_return_if_fail (y >= 0);
	g_return_if_fail (x < gridboard->width);
	g_return_if_fail (y < gridboard->height);

        gridboard->selected[x][y]=type;
}

static gint gtk_gridboard_flip_pixmaps(gpointer data) 
{
        GtkGridBoard * gridboard=(GtkGridBoard *) data;

        gint x, y;
        gint curpm; /* current pixmap */
        gint pcepm; /* this pixmap should be there */
        gint flipped_tiles;

	if (!GTK_WIDGET_DRAWABLE (GTK_WIDGET (gridboard))) {
	  return gridboard->animate;
	}

	flipped_tiles=0;
        for (x=0; x<gridboard->width; x++) {
                for (y=0; y<gridboard->height; y++) {
        		/* curpm is the current pixmap being shown.
			 * pcepm is the target pixmap. We step from
			 * one to the other during the animation. */
                        curpm=gridboard->pixmaps[x][y];
                        pcepm=get_pixmap_num(gridboard->board[x][y]);
                        if (curpm == pcepm) {
                                /* Nothing needs to be flipped */
                                continue;
                        } else if ((curpm == EMPTY_PIXMAP) ||
				   (pcepm == EMPTY_PIXMAP)) {
                                /* Don't animate empty squares */
                                gridboard->pixmaps[x][y]=pcepm;
                                flipped_tiles++;
                        } else if (curpm < pcepm) {
                                gridboard->pixmaps[x][y]++;
                                flipped_tiles++;
                        } else { /* curpm > pcepm */
                                gridboard->pixmaps[x][y]--;
                                flipped_tiles++;
                        }
                }
        }

	if (flipped_tiles)
	  gtk_gridboard_paint (gridboard);

        /* destroy the timeout if animate is false */
        return gridboard->animate;
}

static void gtk_gridboard_load_tiles (GtkGridBoard * gridboard) {
        gchar *filepath=gridboard->tileset;
        GdkPixbuf *image;

        if (! g_file_test (filepath, G_FILE_TEST_EXISTS)) {
                g_print ("Could not find \'%s\' pixmap file\n", 
                                filepath);
                exit (1);
        }

        image = gdk_pixbuf_new_from_file (filepath, NULL);
        if (image==NULL) {
                g_warning ("loading pixmaps failed\n");
                exit(1);
        }

	if (gridboard->tiles_pixbuf != NULL)
	  g_object_unref (gridboard->tiles_pixbuf);

	gridboard->tiles_pixbuf = image;
	gridboard->tiles_scale = gdk_pixbuf_get_height (image);
}


/* helper functions */

/* allocates memory for the array */
static gint ** make_array(int width, int height) {
        gint ** result;
        int valarraysiz, ptrarraysiz;
        int i;
        
        valarraysiz=width*height;       /* size of actual array */
        ptrarraysiz=width;              /* size of pointer array */
        result=(gint **)g_malloc0(ptrarraysiz*sizeof(gint *)+valarraysiz*sizeof(gint));
        for (i=0; i<width; i++) {
                result[i]=(gint *)(result+ptrarraysiz)+i*height;
        }
        return result;
}

int gtk_gridboard_count_pieces(GtkGridBoard * gridboard, int piece) {
        int x, y;
        int count=0;
	
        g_return_val_if_fail(GTK_IS_GRIDBOARD (gridboard), 0);

        for (x=0; x<gridboard->width; x++) 
                for (y=0; y<gridboard->height; y++) 
                        if (gridboard->board[x][y]==piece) count++;

        return count;
}

/* marshaller VOID:INT,INT */
void g_cclosure_user_marshal_VOID__INT_INT (GClosure     *closure,
                                       GValue       *return_value,
                                       guint         n_param_values,
                                       const GValue *param_values,
                                       gpointer      invocation_hint,
                                       gpointer      marshal_data) {
        typedef void (*GMarshalFunc_VOID__INT_INT) (gpointer data1,
                                                        gint arg_1,
                                                        gint arg_2,
                                                        gpointer data2);
        register GMarshalFunc_VOID__INT_INT callback;
        register GCClosure *cc = (GCClosure*) closure;
        register gpointer data1, data2;

        g_return_if_fail (n_param_values == 3);

        if (G_CCLOSURE_SWAP_DATA (closure))
                {
                        data1 = closure->data;
                        data2 = g_value_peek_pointer (param_values + 0);
                }
        else
                {
                        data1 = g_value_peek_pointer (param_values + 0);
                        data2 = closure->data;
                }
        callback = (GMarshalFunc_VOID__INT_INT) (marshal_data ? marshal_data : cc->callback);

        callback (data1,
                g_marshal_value_peek_int (param_values + 1),
                g_marshal_value_peek_int (param_values + 2),
                data2);
}


/* option setting functions */

void gtk_gridboard_set_show_grid(GtkGridBoard *gridboard, gboolean showgrid) {
        g_return_if_fail (GTK_IS_GRIDBOARD (gridboard));

        gridboard->showgrid=showgrid;
}

void gtk_gridboard_set_animate(GtkGridBoard *gridboard, gboolean animate) {
        g_return_if_fail (GTK_IS_GRIDBOARD (gridboard));

        gridboard->animate=animate;
        if (animate) {
	  if (gridboard->timeoutid)
	    return;
	  gridboard->timeoutid = g_timeout_add(PIXMAP_FLIP_DELAY, 
					       gtk_gridboard_flip_pixmaps,
					       (gpointer) gridboard);
        } else gridboard->timeoutid = 0;
}

void gtk_gridboard_set_visibility(GtkGridBoard *gridboard, gboolean visibility) {
	g_return_if_fail (GTK_IS_GRIDBOARD (gridboard));

	gridboard->visibility=visibility;
}

void gtk_gridboard_set_tileset(GtkGridBoard *gridboard, gchar * tileset) {
        g_return_if_fail (GTK_IS_GRIDBOARD (gridboard));

        gridboard->tileset=tileset;
        gtk_gridboard_load_tiles(gridboard);
}
        
/* board altering functions */
void gtk_gridboard_set_piece(GtkGridBoard *gridboard, int x, int y, 
			     int piece) 
{
        g_return_if_fail (GTK_IS_GRIDBOARD (gridboard));

        gridboard->board[x][y]=piece;
	
	if (!gridboard->animate) {
	  gridboard->pixmaps[x][y] = get_pixmap_num (gridboard->board[x][y]);
	}
}

int gtk_gridboard_get_piece(GtkGridBoard *gridboard, int x, int y) 
{
        g_return_val_if_fail (GTK_IS_GRIDBOARD (gridboard), 0);

        return gridboard->board[x][y];
}

void gtk_gridboard_clear_selections(GtkGridBoard *gridboard) 
{
	int x, y;

        g_return_if_fail (GTK_IS_GRIDBOARD (gridboard));

	for (x=0; x<gridboard->width; x++) {
		for (y=0; y<gridboard->height; y++) {
			gridboard->selected[x][y]=SELECTED_NONE;
		}
	}
}

void gtk_gridboard_clear_pieces(GtkGridBoard *gridboard) 
{
	int x, y;

        g_return_if_fail (GTK_IS_GRIDBOARD (gridboard));

	for (x=0; x<gridboard->width; x++) {
		for (y=0; y<gridboard->height; y++) {
			gridboard->board[x][y]=EMPTY;
		}
	}
}

void gtk_gridboard_clear(GtkGridBoard *gridboard) 
{
        g_return_if_fail (GTK_IS_GRIDBOARD (gridboard));

	gtk_gridboard_clear_selections(gridboard);
	gtk_gridboard_clear_pieces(gridboard);
	gtk_gridboard_clear_states(gridboard);
}

int gtk_gridboard_states_present(GtkGridBoard *gridboard) 
{
        g_return_val_if_fail (GTK_IS_GRIDBOARD (gridboard), FALSE);

	return (gridboard->statelist)!=NULL;
}

/* saves the current board on a stack */
void gtk_gridboard_save_state(GtkGridBoard *gridboard, gpointer data) 
{
	StateList * newstate;
	int x, y;

        g_return_if_fail (GTK_IS_GRIDBOARD (gridboard));

	newstate = g_malloc(sizeof(StateList));

	newstate->board=make_array(gridboard->width, gridboard->height);
	for (x=0; x<gridboard->width; x++) {
		for (y=0; y<gridboard->height; y++) {
			newstate->board[x][y]=gridboard->board[x][y];
		}
	}

	newstate->prev=gridboard->statelist;
	newstate->data=data;
	gridboard->statelist=newstate;	
}

/* reverts to the last state on the stack and deletes it */
gpointer gtk_gridboard_revert_state(GtkGridBoard *gridboard) 
{
	StateList * thisstate, * prevstate;
	gpointer data;
	int x, y;

        g_return_val_if_fail (GTK_IS_GRIDBOARD (gridboard), NULL);

	thisstate=gridboard->statelist;
	if (thisstate==NULL) return NULL;
	
	prevstate=gridboard->statelist->prev;

	gtk_gridboard_clear_selections (gridboard);
	for (x=0; x<gridboard->width; x++) {
		for (y=0; y<gridboard->width; y++) {
			gridboard->board[x][y]=thisstate->board[x][y];
		}
	}

	data=thisstate->data;
	gridboard->statelist=prevstate;
	g_free(thisstate->board);
	g_free(thisstate);
	return data;
}

void gtk_gridboard_clear_states(GtkGridBoard *gridboard) 
{
	StateList * curstate;
	StateList * prevstate;

        g_return_if_fail (GTK_IS_GRIDBOARD (gridboard));

	curstate=gridboard->statelist;

	while (curstate!=NULL) {
		g_free(curstate->board);
		prevstate=curstate->prev;
		g_free(curstate);
		curstate=prevstate;
	}
	gridboard->statelist=NULL;
}
		
int gtk_gridboard_get_height(GtkGridBoard *gridboard) 
{
        g_return_val_if_fail (GTK_IS_GRIDBOARD (gridboard), 0);

	return gridboard->height;
}
		
int gtk_gridboard_get_width(GtkGridBoard *gridboard) 
{
        g_return_val_if_fail (GTK_IS_GRIDBOARD (gridboard), 0);

	return gridboard->width;
}
