/* FIXME add some comments
 here
 */

#ifndef __GTKGRIDBOARD_H__
#define __GTKGRIDBOARD_H__

#include <gdk/gdk.h>
#include <gtk/gtkadjustment.h>
#include <gtk/gtkwidget.h>
#include <cairo/cairo.h>

G_BEGIN_DECLS

/* macro's */

#define GTK_GRIDBOARD(obj) GTK_CHECK_CAST (obj, gtk_gridboard_get_type (), GtkGridBoard)
#define GTK_GRIDBOARD_CLASS(klass) GTK_CHECK_CLASS_CAST (klass, gtk_gridboard_get_type (), GtkGridBoardClass)
#define GTK_IS_GRIDBOARD(obj) GTK_CHECK_TYPE (obj, gtk_gridboard_get_type ())

#define g_marshal_value_peek_int(v)      (v)->data[0].v_int

/* definitions */
#define EMPTY 0
#define WHITE 1
#define BLACK 2

#define EMPTY_PIXMAP 0
#define BLACK_PIXMAP 1
#define WHITE_PIXMAP 31

#define TILEWIDTH 60
#define TILEHEIGHT 60

#define SELECTED_NONE 0
#define SELECTED_A 1
#define SELECTED_B 2

#define PIXMAP_FLIP_DELAY 40

enum {
        BOXCLICKED,
        LAST_SIGNAL
};

/* structs */
typedef struct _GtkGridBoard            GtkGridBoard;
typedef struct _GtkGridBoardClass       GtkGridBoardClass;
typedef struct _StateList		StateList;

struct _StateList {
	gint ** board;
	struct _StateList * prev;
	gpointer data;
};

struct _GtkGridBoard {
        GtkWidget widget;       /* parent class */
        gint ** pixmaps;
        gint ** board;
        gint ** selected;
        gint width;
        gint height;
        gint tilewidth;
        gint tileheight;
        guint timeoutid;
        GdkPixbuf *tiles_pixbuf;
        gint tiles_scale;
        gchar * tileset;
        gboolean visibility;
        gboolean animate;
        gboolean showgrid;
	StateList * statelist;
        cairo_matrix_t transform;
        cairo_surface_t *themesurface;
        cairo_t *cx;
};
	
struct _GtkGridBoardClass {
        GtkWidgetClass parent_class;
        void (* boxclicked) (GtkGridBoard * gridboard, int x, int y);
};

/* prototypes */
GType gtk_gridboard_get_type(void);
GtkWidget *gtk_gridboard_new(gint width, gint height, char * tileset);
void g_cclosure_user_marshal_VOID__INT_INT (GClosure     *closure,
                                       GValue       *return_value,
                                       guint         n_param_values,
                                       const GValue *param_values,
                                       gpointer      invocation_hint,
                                       gpointer      marshal_data); 

void gtk_gridboard_set_animate(GtkGridBoard * gridboard, gboolean animate); 
void gtk_gridboard_set_visibility(GtkGridBoard *gridboard, gboolean visibility);
void gtk_gridboard_set_piece(GtkGridBoard * gridboard, int x, int y, int piece);
void gtk_gridboard_set_selection (GtkGridBoard *widget, gint type, 
				  gint x, gint y);
int gtk_gridboard_count_pieces(GtkGridBoard * gridboard, int piece);
int gtk_gridboard_get_piece(GtkGridBoard * gridboard, int x, int y);
void gtk_gridboard_set_show_grid(GtkGridBoard * widget, gboolean showgrid);
void gtk_gridboard_set_tileset(GtkGridBoard * widget, gchar * tileset);
void gtk_gridboard_clear_selections(GtkGridBoard * widget);
void gtk_gridboard_clear_pieces(GtkGridBoard * widget);
void gtk_gridboard_clear(GtkGridBoard * widget);
void gtk_gridboard_save_state(GtkGridBoard * widget, gpointer data);
gpointer gtk_gridboard_revert_state(GtkGridBoard * widget);
void gtk_gridboard_clear_states(GtkGridBoard * widget);
int gtk_gridboard_get_height(GtkGridBoard * widget);
int gtk_gridboard_get_width(GtkGridBoard * widget);
void gtk_gridboard_paint(GtkGridBoard * gridboard); 
int gtk_gridboard_states_present(GtkGridBoard * widget);

G_END_DECLS

#endif /* __GTKGRIDBOARD_H__ */


