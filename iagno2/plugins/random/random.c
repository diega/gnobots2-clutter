#include <gtk/gtk.h>

#include <sys/time.h>

#include "../../reversi-iagno2/reversi.h"

#define VERSION "0.1"

extern GtkWidget *app;

extern gchar whose_turn;

void
plugin_init ()
{
  struct timeval tv;

  gettimeofday (&tv, NULL);
  srand (tv.tv_usec);
}

gint
plugin_move (gchar *board)
{
  gint i;
  gint moves[32];
  gint nummoves = 0;

  sleep (1);
  
  for (i = 0; i < 64; i++) {
    if (is_valid_move (board, i, whose_turn)) {
      moves[nummoves++] = i;
    }
  }

  if (!nummoves) {
    return TRUE;
  }

  i = rand () % nummoves;

  return moves[i];
}

const gchar *
plugin_name ()
{
  return "Random player for Iagno II";
}
