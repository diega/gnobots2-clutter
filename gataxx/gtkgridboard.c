#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <gtk/gtk.h>
#include <gtk/gtkmain.h>
#include <gtk/gtksignal.h>
#include <glib-object.h>
#include <cairo.h>
#include <math.h>
#include <string.h>

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include "gtkgridboard.h"

#undef DO_PROFILE

#ifdef DO_PROFILE
#include <sys/time.h>

/* A temporary profiling counter. Only supports one instance.*/
long int counter;
#endif

static GtkWidgetClass *parent_class=NULL;
static guint gridboard_signals[LAST_SIGNAL] = { 0 };

/* prototypes for private functions */
static void gtk_gridboard_repaint(GtkGridBoard * gridboard);
static gint ** make_array(int width, int height, int fill); 
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
        gridboard->backing_store = NULL;
	gridboard->tiles_pixbuf = NULL;
	gridboard->tiles_scale = 1;
        gridboard->pixmaps=NULL;
	gridboard->timeoutid = 0;
        gridboard->tileheight=TILEHEIGHT;
        gridboard->tilewidth=TILEWIDTH;
        gridboard->width=0;
        gridboard->height=0;
	gridboard->statelist=NULL;
	gridboard->theme = gtk_gridboard_themes;
}

GtkWidget * gtk_gridboard_new(gint width, gint height, char * theme) {
        GtkGridBoard * gridboard=g_object_new(gtk_gridboard_get_type(), NULL);

        gridboard->width=width;
        gridboard->height=height;

        gridboard->board    = make_array (width, height, 0);
        gridboard->pixmaps  = make_array (width, height, 0);
        gridboard->selected = make_array (width, height, SELECTED_NONE);
	gridboard->changed  = make_array (width, height, TRUE);
	
        gtk_gridboard_set_animate(gridboard, TRUE);
        gtk_gridboard_set_visibility(gridboard, TRUE);

	gtk_gridboard_set_theme (gridboard, theme);
        
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

	gdk_draw_drawable (widget->window, widget->style->black_gc,
			   gridboard->backing_store, event->area.x,
			   event->area.y, event->area.x, event->area.y,
			   event->area.width, event->area.height);

        return FALSE;
}

static void gtk_gridboard_size_allocate (GtkWidget * widget,
					 GtkAllocation *allocation)
{
  GtkGridBoard *gridboard = GTK_GRIDBOARD (widget);

  widget->allocation = *allocation;
 
  if (gridboard->backing_store != NULL) {
    g_object_unref (gridboard->backing_store);
    gridboard->backing_store = NULL;
  }

  if (GTK_WIDGET_REALIZED (widget)) {
    gdk_window_move_resize (widget->window, allocation->x, allocation->y,
			    allocation->width, allocation->height);
    gtk_gridboard_paint (gridboard);
  }
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
	
	gtk_gridboard_paint (GTK_GRIDBOARD (widget));
}

static void gtk_gridboard_finalize(GObject * object) {
        GtkGridBoard *gridboard;

        g_return_if_fail (object != NULL);
        g_return_if_fail (GTK_IS_GRIDBOARD (object));

        gridboard = GTK_GRIDBOARD(object);

	g_object_unref (gridboard->backing_store);
        g_free (gridboard->board);
        g_free (gridboard->pixmaps);
        g_free (gridboard->selected);
	g_free (gridboard->changed);

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

void gtk_gridboard_paint (GtkGridBoard *gridboard)
{
  GtkWidget *widget;

  g_return_if_fail (GTK_IS_GRIDBOARD (gridboard));
  
  widget = GTK_WIDGET (gridboard);
  g_return_if_fail (GTK_WIDGET_REALIZED (widget));

  gtk_widget_queue_draw (widget);

  gtk_gridboard_repaint (gridboard);
}

static void gtk_gridboard_repaint(GtkGridBoard * gridboard) {
  int x, y;
  double phase;
  GtkWidget *widget;
  gdouble scale;
  cairo_t *cx;
  
#ifdef DO_PROFILE
  struct timeval tv;
#endif
  
  g_return_if_fail (GTK_IS_GRIDBOARD (gridboard));
  if (!gridboard->visibility) return;
  
  widget = GTK_WIDGET (gridboard);
  g_return_if_fail (GTK_WIDGET_REALIZED (widget));

  if (gridboard->backing_store == NULL) {
    gridboard->backing_store = gdk_pixmap_new (widget->window,
					       widget->allocation.width,
					       widget->allocation.height, 
					       -1);
    for (x=0; x<gridboard->width; x++)
      for (y=0; y<gridboard->height; y++)
	gridboard->changed[x][y] = TRUE;
  }
  
  cx = gdk_cairo_create (gridboard->backing_store);
  cairo_scale (cx, 1.0*widget->allocation.width/gridboard->width,
	       1.0*widget->allocation.height/gridboard->height);
    
#ifdef DO_PROFILE
  gettimeofday (&tv, NULL);
  counter = tv.tv_sec * 1000000 + tv.tv_usec;
#endif
    
  scale = gridboard->tiles_scale;
  
  for (x=0; x<gridboard->height; x++) {
    for (y=0; y<gridboard->width; y++) {
      if (gridboard->changed[x][y]) {
	gridboard->theme->draw_bg (cx, x, y);
	gridboard->theme->draw_grid (cx, x, y);
	if (gridboard->board[x][y] != EMPTY) {
	  phase = gridboard->pixmaps[x][y];
	  phase = (phase - 1.0)/30.0;
	  gridboard->theme->draw_piece (cx, x, y, phase);
	} else if (gridboard->selected[x][y]) {
	  gridboard->theme->draw_hilight (cx, x, y);
	}
	gridboard->changed[x][y] = FALSE;
      }
    }
  }

  cairo_destroy (cx);
  
#ifdef DO_PROFILE
  gettimeofday (&tv, NULL);
  g_print ("%ld\n", tv.tv_sec * 1000000 + tv.tv_usec - counter);
#endif
}



void gtk_gridboard_set_selection(GtkGridBoard * gridboard, gint type, 
				 gint x, gint y) 
{
        g_return_if_fail (GTK_IS_GRIDBOARD (gridboard));
	g_return_if_fail (x >= 0);
	g_return_if_fail (y >= 0);
	g_return_if_fail (x < gridboard->width);
	g_return_if_fail (y < gridboard->height);

        gridboard->selected[x][y] = type;
	gridboard->changed[x][y]  = TRUE;
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
				gridboard->changed[x][y] = TRUE;
                        } else if (curpm < pcepm) {
                                gridboard->pixmaps[x][y]++;
                                flipped_tiles++;
				gridboard->changed[x][y] = TRUE;
                        } else { /* curpm > pcepm */
                                gridboard->pixmaps[x][y]--;
                                flipped_tiles++;
				gridboard->changed[x][y] = TRUE;
                        }
                }
        }

	if (flipped_tiles) 
	  gtk_gridboard_paint (gridboard);

        /* destroy the timeout if animate is false */
        return gridboard->animate;
}

/* helper functions */

/* Allocates memory for an array and fills it with the byte fill. */
static gint ** make_array(int width, int height, int fill) 
{
  gint ** result;
  int valarraysiz, ptrarraysiz;
  int i;
  
  valarraysiz = width*height;       /* size of actual array */
  ptrarraysiz = width;              /* size of pointer array */
  result = (gint **) g_malloc (ptrarraysiz*sizeof(gint *)+valarraysiz*sizeof(gint));
  for (i=0; i<width; i++) {
    result[i] = (gint *)(result+ptrarraysiz)+i*height;
    memset (result[i], fill, sizeof (gint)*height);
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

void gtk_gridboard_set_theme (GtkGridBoard *gridboard, gchar * name)
{
        GtkGridBoardTheme *t;
	gint x, y;

        g_return_if_fail (GTK_IS_GRIDBOARD (gridboard));
	g_return_if_fail (name != NULL);
	g_return_if_fail (name[0] != '\0');

	t = gtk_gridboard_themes;
	while (t->name != NULL) {
	  if (g_utf8_collate (t->name, name) == 0) {
	    gridboard->theme = t;
	    break;
	  }
	  t++;
	}

	for (x=0; x<gridboard->width; x++)
	  for (y=0; y<gridboard->height; y++)
	    gridboard->changed[x][y] = TRUE;
}
        
/* board altering functions */
void gtk_gridboard_set_piece(GtkGridBoard *gridboard, int x, int y, 
			     int piece) 
{
        g_return_if_fail (GTK_IS_GRIDBOARD (gridboard));

	if (gridboard->board[x][y] == piece)
	  return;

        gridboard->board[x][y]=piece;
	
	if (!gridboard->animate) {
	  gridboard->pixmaps[x][y] = get_pixmap_num (gridboard->board[x][y]);
	  gridboard->changed[x][y] = TRUE;
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
		  if (gridboard->selected[x][y] != SELECTED_NONE) {
		    gridboard->selected[x][y] = SELECTED_NONE;
		    gridboard->changed[x][y] = TRUE;
		  }
		}
	}
}

void gtk_gridboard_clear_pieces(GtkGridBoard *gridboard) 
{
	int x, y;

        g_return_if_fail (GTK_IS_GRIDBOARD (gridboard));

	for (x=0; x<gridboard->width; x++) {
		for (y=0; y<gridboard->height; y++) {
			gridboard->board[x][y] = EMPTY;
			gridboard->changed[x][y] = TRUE;
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

	newstate->board = make_array (gridboard->width, gridboard->height, 0);
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
	    if (gridboard->board[x][y] != thisstate->board[x][y]) {
	      gridboard->board[x][y] = thisstate->board[x][y];
	      gridboard->changed[x][y] = TRUE;
	    }
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
