#include <clutter/clutter.h>

ClutterActor *get_player ();
void delete_clutter_player ();
void move_explosion (gint, gint, gint, gint);
void delete_clutter_explosion (gint, gint);
gboolean explosion_exists (gint, gint);
ClutterActor *get_explosion (gint, gint);
gboolean robot1_exists (gint, gint);
ClutterActor *get_robot1 (gint, gint);
gboolean robot2_exists (gint, gint);
ClutterActor *get_robot2 (gint, gint);
void move_clutter_robot (gint, gint, gint, gint);
void delete_clutter_robot (gint, gint);

//void resize_clutter_cb (GtkWidget *, GtkAllocation *, gpointer);
//gboolean clutter_move_cb (GtkWidget *, GdkEventMotion *, gpointer);
