#ifndef _IAGNO2_MAIN_H_
#define _IAGNO2_MAIN_H_

gint delete_event_cb (GtkWidget *, GdkEventAny *, gpointer);

gint new_game_cb (GtkWidget *, gpointer);

void new_game_real_cb (gint reply, gpointer data);

#endif
