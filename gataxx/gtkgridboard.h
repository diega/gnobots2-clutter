/* FIXME add some comments
 here
 */

#ifndef __GTKGRIDBOARD_H__
#define __GTKGRIDBOARD_H__

#include <gdk/gdk.h>
#include <gtk/gtkadjustment.h>
#include <gtk/gtkwidget.h>

/*
#ifdef __cplusplus
extern "C" {
#endif
*/

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
        guint policy:2;
        gint ** pixmaps;
        gint ** board;
        gint ** selected;
        gint width;
        gint height;
        gint tilewidth;
        gint tileheight;
        GdkPixmap * tiles_pixmap;
        gchar * tileset;
        gboolean animate;
        gboolean showgrid;
	StateList * statelist;
};
	
struct _GtkGridBoardClass {
        GtkWidgetClass parent_class;
        void (* boxclicked) (GtkGridBoard * gridboard, int x, int y);
};

/* prototypes */
GtkWidget *     gtk_gridboard_new();
GtkType         gtk_gridboard_get_type(void);

GtkWidget * gtk_gridboard_new(gint width, gint height, char * tileset);
void g_cclosure_user_marshal_VOID__INT_INT (GClosure     *closure,
                                       GValue       *return_value,
                                       guint         n_param_values,
                                       const GValue *param_values,
                                       gpointer      invocation_hint,
                                       gpointer      marshal_data); 

void gtk_gridboard_set_animate(GtkWidget * gridboard, gboolean animate); 
void gtk_gridboard_set_piece(GtkWidget * gridboard, int x, int y, int piece);
void gtk_gridboard_set_selection(GtkWidget * widget, gint type, gint x, gint y);
int gtk_gridboard_count_pieces(GtkWidget * gridboard, int piece);
int gtk_gridboard_get_piece(GtkWidget * gridboard, int x, int y);
void gtk_gridboard_set_show_grid(GtkWidget * widget, gboolean showgrid);
void gtk_gridboard_set_tileset(GtkWidget * widget, gchar * tileset);
void gtk_gridboard_clear_selections(GtkWidget * widget);
void gtk_gridboard_clear_pieces(GtkWidget * widget);
void gtk_gridboard_clear_pixmaps(GtkWidget * widget);
void gtk_gridboard_clear(GtkWidget * widget);
void gtk_gridboard_save_state(GtkWidget * widget, gpointer data);
gpointer gtk_gridboard_revert_state(GtkWidget * widget);
void gtk_gridboard_clear_states(GtkWidget * widget);
int gtk_gridboard_get_height(GtkWidget * widget);
int gtk_gridboard_get_width(GtkWidget * widget);
void gtk_gridboard_paint(GtkGridBoard * gridboard); 
int gtk_gridboard_states_present(GtkWidget * widget);

/*      
#ifdef __cplusplus
}
#endif
*/

#endif /* __GTKGRIDBOARD_H__ */


