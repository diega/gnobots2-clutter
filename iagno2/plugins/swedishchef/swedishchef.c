/*
 * Swedish Chef for Iagno II: A plugin for Iagno II
 * Copyright (C) 1999-2000 Ian Peters <itp@gnu.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

#include <gnome.h>
#include <sys/time.h>
#include <unistd.h>

#include "../../reversi-iagno2/reversi.h"


#define VERSION "0.0.1"
#define BUSY_MESSAGE "Swedish Chef is thinking..."
#define PREF_THINK_TIME "/iagno2/swedishchef/player%d_thinking_time"
#define DEFAULT_THINKING_TIME "0.5"

/* #define DEBUG */


/* ------------------------------------------------------------ */

typedef unsigned char UBYTE;
typedef UBYTE BoardType[10][16];
struct PlayerData_s {
  gint Combs;
  gint numTiles;
  gint nflip[8];
  gint xstart[64], ystart[64];
  gdouble thinking_time;
#ifdef DEBUG
  gint fh, fhGood;
#endif
};
typedef struct PlayerData_s PlayerData;


static void findMove (PlayerData *pd, BoardType board, gint *xp, gint *yp,
		      gint player);
static gint search (PlayerData *pd, BoardType board, gint oldscore, gint player,
		   gint alpha, gint beta, gint ply, gint depth, gboolean pass);
static gint numFlip (PlayerData *pd, BoardType board, gint x, gint y, gint player);
static void flip (PlayerData *pd, BoardType board, gint x, gint y, gint player);
static gint sumTiles (BoardType board, gint player);
static gint rankMove (PlayerData *pd, BoardType board, gint x, gint y, gint num);
static inline void moveBoard (BoardType from, BoardType to);

/* ------------------------------------------------------------ */

static PlayerData *
get_player_data (gchar player)
{
  static PlayerData pd[2];
  g_assert ((player == BLACK) || (player || WHITE));
  return &pd[player == WHITE ? 0 : 1];
}

void
plugin_init_player (gchar player)
{
  gchar *time_str, *pref_file;
  PlayerData *pd = get_player_data (player);

  pref_file = g_strdup_printf (PREF_THINK_TIME "=" DEFAULT_THINKING_TIME, player);
  time_str = gnome_config_get_string (pref_file);
  sscanf (time_str, "%lg", &pd->thinking_time);
  g_free (pref_file);
  g_free (time_str);
}

void
plugin_deinit_player (gchar player)
{
}

void
plugin_setup (gchar player)
{
  struct timeval tv;

  gettimeofday (&tv, NULL);
  srand (tv.tv_sec + tv.tv_usec);
}

static gint
color_convert (char iagno_color)
{
  if (iagno_color == BLACK)
    return 1;
  else if (iagno_color == WHITE)
    return 2;
  else
    return 0;
}

gint
plugin_move (ReversiBoard *board, gchar player)
{
  gint x, y;
  BoardType myboard;
  gint xx, yy;
  PlayerData *pd = get_player_data (player);

#ifdef DEBUG
  printf ("\nturn:%d\n", board->move_count);
  for (y = 0; y < 8; y++) {
    for (x = 0; x < 8; x++) {
      printf ("%2d ", board->board[x + y * 8]);
    }
    printf ("\n");
  }
#endif

  for (y = 0; y < 8; y++) {
    for (x = 0; x < 8; x++) {
      gchar tile = board->board[x + y * 8];
      myboard[x + 1][y + 1] = color_convert (tile);
    }
  }

  findMove (pd, myboard, &xx, &yy, color_convert (player));

#ifdef DEBUG
    printf ("xx:%d yy:%d\n\n", xx, yy);
#endif

  return (xx - 1) + (yy - 1) * 8;
}

const gchar *
plugin_name ()
{
  return "Swedish Chef";
}

const gchar *
plugin_busy_message (gchar player)
{
  return BUSY_MESSAGE;
}

void
plugin_about_window (GtkWindow *parent, gchar player)
{
  GtkWidget *about;
  const gchar *authors[] = {
    "Peter Österlund (peter.osterlund@mailbox.swipnet.se)",
    "Ian Peters (itp@gnu.org)",
    NULL
  };

  about = gnome_about_new (_("Swedish Chef for Iagno II"), VERSION,
                           "(C) 1999-2000 Ian Peters",
                           (const gchar **) authors,
                           _("This is a computer player plugin for Iagno II. "
			     "It uses the alpha-beta algorithm and is "
			     "therefore tactically quite strong. The "
			     "evaluation function is too greedy though, so it "
			     "is strategically quite weak."),
                           NULL);

  gnome_dialog_set_parent (GNOME_DIALOG (about), GTK_WINDOW (parent));

  gtk_widget_show (about);
}

void
plugin_preferences_window (GtkWidget *parent, gchar player)
{
  GtkWidget *dialog;
  GtkWidget *hbox, *label, *scale;
  GtkObject *adj;
  PlayerData *pd = get_player_data (player);

  gint store_prefs (GtkWidget *widget, gpointer data)
  {
    GtkAdjustment *adj = (GtkAdjustment *) data;
    gchar player = (gchar)(int) gtk_object_get_data (GTK_OBJECT (adj), "player");
    PlayerData *pd = get_player_data (player);

    pd->thinking_time = adj->value;
    gnome_property_box_changed (GNOME_PROPERTY_BOX (parent));
    return TRUE;
  }

  dialog = gnome_dialog_new (_("Swedish Chef Configuration"),
                             GNOME_STOCK_BUTTON_OK,
                             GNOME_STOCK_BUTTON_CANCEL,
                             NULL);

  gnome_dialog_set_parent (GNOME_DIALOG (dialog), GTK_WINDOW (parent));

  hbox = gtk_hbox_new (FALSE, 10);
  label = gtk_label_new ("Thinking time:");
  adj = gtk_adjustment_new (pd->thinking_time, 0.0, 10.0, 0.1, 0.1, 0.0);
  gtk_object_set_data (GTK_OBJECT (adj), "player", (gpointer)(int)player);
  scale = gtk_hscale_new (GTK_ADJUSTMENT (adj));

  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), scale, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (dialog)->vbox), hbox, FALSE, TRUE, 5);

  gtk_widget_show (hbox);
  gtk_widget_show (label);
  gtk_widget_show (scale);

  gnome_dialog_button_connect (GNOME_DIALOG (dialog),
                               0,
                               GTK_SIGNAL_FUNC (store_prefs),
                               (gpointer) adj);

  gnome_dialog_run_and_close (GNOME_DIALOG (dialog));
}

void
plugin_preferences_save (char player)
{
  gchar* buffer, *pref_file;
  PlayerData *pd = get_player_data (player);

  buffer = g_strdup_printf ("%g", pd->thinking_time);
  pref_file = g_strdup_printf (PREF_THINK_TIME, player);
  gnome_config_set_string (pref_file, buffer);
  g_free (pref_file);
  g_free (buffer);
  gnome_config_sync ();
}

/* ------------------------------------------------------------ */

static gint delta[8] = { -16, -15, 1, 17, 16, 15, -1, -17 };


#define WINSCORE  (10000)
#define LOSESCORE (-10000)

static inline void
moveBoard (BoardType from, BoardType to)
{
  memcpy (to, from, sizeof (BoardType));
}
static void
findMove (PlayerData *pd, BoardType board, gint *xp, gint *yp, gint player)
{
  UBYTE TmpBoard[10][16];
  UBYTE xMoves[64], yMoves[64];
  gint i, j;
  GTimer *gt;
  gint NumMoves = 0;
  gint depth = 2;

  pd->numTiles = 0;
  for (i = 1; i < 9; i++)
    for (j = 1; j < 9; j++)
      if (board[i][j])
	pd->numTiles++;
  for (i = 0; i < 64; i++) {
    pd->xstart[i] = 1;
    pd->ystart[i] = 1;
  }

  gt = g_timer_new ();
  g_timer_start (gt);

  pd->Combs = 0;
  do {
    gint alpha = 2 * LOSESCORE;
    gint beta = 2 * WINSCORE;
    gint i, j;
    gint NumLeft = 64;

#ifdef DEBUG
    printf ("depth:%2d\n", depth);
    pd->fh = pd->fhGood = 0;
#endif

    for (i = pd->xstart[0], j = pd->ystart[0]; NumLeft > 0; NumLeft--) {
      gint num;
      if ((board[i][j] == 0) && ((num = numFlip (pd, board, i, j, player)) > 0)) {
	gint tmpscore, score;

	moveBoard (board, TmpBoard);
	flip (pd, TmpBoard, i, j, player);
	pd->numTiles++;

	tmpscore = rankMove (pd, board, i, j, num);
	score = -search (pd, TmpBoard, -tmpscore, 3 - player,
			 -beta, -(alpha-1), 1, depth - 1, FALSE);
	pd->numTiles--;

	if (score > alpha) {
	  NumMoves = 0;
	  pd->xstart[0] = i; pd->ystart[0] = j;
	}
	if (score >= alpha) {
	  xMoves[NumMoves] = i;
	  yMoves[NumMoves] = j;
	  NumMoves++;
	  alpha = score;
#ifdef DEBUG
	  printf ("(%d,%d) score:%d\n", i, j, alpha);
#endif
	}
      }
      j++;
      if (j > 8) {
	j = 1; i++;
	if (i > 8) i = 1;
      }
    }
#ifdef DEBUG
    printf ("combs:%d fh:%.2f\n", pd->Combs, (double)pd->fhGood / pd->fh);
#endif
    depth++;
  } while ((g_timer_elapsed (gt, NULL) < pd->thinking_time) &&
	   (depth + pd->numTiles <= 64));
  g_assert (NumMoves > 0);
  i = random () % NumMoves;
  *xp = xMoves[i]; *yp = yMoves[i];

  g_timer_destroy (gt);
}

/*
 * Compute score for a position, + is good for 'player' who is to make
 * the first move
 */
static gint
search (PlayerData *pd, BoardType board, gint oldscore, gint player,
	gint alpha, gint beta, gint ply, gint depth, gboolean pass)
{
  gint x, y;
  UBYTE TmpBoard[10][16];
  gint NumLeft;
  gint validMoves = 0;

  if (depth <= 0)
    return oldscore;

  pd->Combs++;

  x = pd->xstart[ply];
  y = pd->ystart[ply];
  for (NumLeft = 64; NumLeft > 0; NumLeft--) {
    gint num;
    if ((board[x][y] == 0) && ((num = numFlip (pd, board, x, y, player)) > 0)) {
      gint tmpscore, score;

      validMoves++;
      moveBoard (board, TmpBoard);
      flip (pd, TmpBoard, x, y, player);
      pd->numTiles++;
      if (pd->numTiles >= 64) {
	pd->numTiles--;
	return sumTiles (TmpBoard, player);
      }

      tmpscore = oldscore + rankMove (pd, board, x, y, num);
      score = -search (pd, TmpBoard, -tmpscore, 3 - player,
		       -beta, -alpha,
		       ply + 1, depth - 1, FALSE);
      pd->numTiles--;

      if (score > alpha) {
	pd->xstart[ply] = x;
	pd->ystart[ply] = y;
	if (score >= beta) {
#ifdef DEBUG
	  pd->fh++;
	  if (validMoves == 1)
	    pd->fhGood++;
#endif
	  return score;
	}
	alpha = score;
      }
    }
    y++;
    if (y > 8) {
      y = 1; x++;
      if (x > 8) x = 1;
    }
  }
  if (!validMoves) {
    if (pass) {
      /* Two passes in a row means game is over */
      return sumTiles (board, player);
    } else {
      return -search (pd, board, -oldscore, 3 - player, -beta, -alpha,
		      ply + 1, depth, TRUE);
    }
  }
  return alpha;
}

static gint
numFlip (PlayerData *pd, BoardType board, gint x, gint y, gint player)
{
  UBYTE *ptr;
  UBYTE *orgptr = &board[x][y];
  gint num = 0;
  gint i;

  for (i = 7; i >= 0; i--) {
    pd->nflip[i] = 0; x = delta[i];
    ptr = orgptr + x;
    if ( ((*ptr) + player) == 3) {
      ptr += x; y = 1;
      while ( ((*ptr) + player) == 3) {
	ptr += x; y++;
      }
      if (*ptr == player) {
	num += y;
	pd->nflip[i] = y;
      }
    }
  }
  return num;
}

static void
flip (PlayerData *pd, BoardType board, gint x, gint y, gint player)
{
  UBYTE *ptr;
  UBYTE *orgptr = &board[x][y];
  gint i;

  *orgptr = player;
  for (i = 7; i >= 0; i--) {
    y = pd->nflip[i]; x = delta[i];
    ptr = orgptr;
    while (y > 0) {
      ptr += x; *ptr = player;
      y--;
    }
  }
}

static gint
sumTiles (BoardType board, gint player)
{
  UBYTE *ptr = &board[1][1];
  gint Score = 0;
  gint i, j;

  for (i = 8; i > 0; i--) {
    for (j = 8; j > 0; j--) {
      if (*ptr == player) Score++;
      else if (*ptr) Score--;
      ptr++;
    }
    ptr += 8;
  }
  if (Score > 0) return WINSCORE + Score;
  if (Score < 0) return LOSESCORE + Score;
  return 0;
}

#define S11 100
#define S22 -50
#define S21 -15
#define S31 30
#define S41 25
#define S33 3

static gint
rankMove (PlayerData *pd, BoardType board, gint x, gint y, gint num)
{
  switch (8 * x + y - 9) {
  case 0: case 7: case 56: case 63:
    return ((pd->nflip[0] + pd->nflip[2] + pd->nflip[4] + pd->nflip[6]) *
	    (2 * (S41 - 1)) + 2 * num + S11);
  case 1:
    if (board[1][1])
      goto hor_edge;
    else
      return (pd->nflip[2] + pd->nflip[6]) * (2 * (S41 - 1)) + 2 * num + S21;
  case 2:
    if (board[1][1])
      goto hor_edge;
    else
      return (pd->nflip[2] + pd->nflip[6]) * (2 * (S41 - 1)) + 2 * num + S31;
  case 3: case 4: case 59: case 60: hor_edge:
    return (pd->nflip[2] + pd->nflip[6]) * (2 * (S41 - 1)) + 2 * num + S41;
  case 5:
    if (board[8][1])
      goto hor_edge;
    else
      return (pd->nflip[2] + pd->nflip[6]) * (2 * (S41 - 1)) + 2 * num + S31;
  case 6:
    if (board[8][1])
      goto hor_edge;
    else
      return (pd->nflip[2] + pd->nflip[6]) * (2 * (S41 - 1)) + 2 * num + S21;
  case 8:
    if (board[1][1])
      goto ver_edge;
    else
      return (pd->nflip[0] + pd->nflip[4]) * (2 * (S41 - 1)) + 2 * num + S21;
  case 9:
    if (board[1][1])
      return 2 * num + 1;
    else
      return 2 * num + S22;
  case 14:
    if (board[8][1])
      return 2 * num + 1;
    else
      return 2 * num + S22;
  case 15:
    if (board[8][1])
      goto ver_edge;
    else
      return (pd->nflip[0] + pd->nflip[4]) * (2 * (S41 - 1)) + 2 * num + S21;
  case 16:
    if (board[1][1])
      goto ver_edge;
    else
      return (pd->nflip[0] + pd->nflip[4]) * (2 * (S41 - 1)) + 2 * num + S31;
  case 18: case 21: case 42: case 45:
    return 2 * num + S33;
  case 23:
    if (board[8][1])
      goto ver_edge;
    else
      return (pd->nflip[0] + pd->nflip[4]) * (2 * (S41 - 1)) + 2 * num + S31;
  case 24: case 31: case 32: case 39: ver_edge:
    return (pd->nflip[0] + pd->nflip[4]) * (2 * (S41 - 1)) + 2 * num + S41;
  case 40:
    if (board[1][8])
      goto ver_edge;
    else
      return (pd->nflip[0] + pd->nflip[4]) * (2 * (S41 - 1)) + 2 * num + S31;
  case 47:
    if (board[8][8])
      goto ver_edge;
    else
      return (pd->nflip[0] + pd->nflip[4]) * (2 * (S41 - 1)) + 2 * num + S31;
  case 48:
    if (board[1][8])
      goto ver_edge;
    else
      return (pd->nflip[0] + pd->nflip[4]) * (2 * (S41 - 1)) + 2 * num + S21;
  case 49:
    if (board[1][8])
      return 2 * num + 1;
    else
      return 2 * num + S22;
  case 54:
    if (board[8][8])
      return 2 * num + 1;
    else
      return 2 * num + S22;
  case 55:
    if (board[8][8])
      goto ver_edge;
    else
      return (pd->nflip[0] + pd->nflip[4]) * (2 * (S41 - 1)) + 2 * num + S21;
  case 57:
    if (board[1][8])
      goto hor_edge;
    else
      return (pd->nflip[2] + pd->nflip[6]) * (2 * (S41 - 1)) + 2 * num + S21;
  case 58:
    if (board[1][8])
      goto hor_edge;
    else
      return (pd->nflip[2] + pd->nflip[6]) * (2 * (S41 - 1)) + 2 * num + S31;
  case 61:
    if (board[8][8])
      goto hor_edge;
    else
      return (pd->nflip[2] + pd->nflip[6]) * (2 * (S41 - 1)) + 2 * num + S31;
  case 62:
    if (board[8][8])
      goto hor_edge;
    else
      return (pd->nflip[2] + pd->nflip[6]) * (2 * (S41 - 1)) + 2 * num + S21;
  case 10: case 11: case 12: case 13:
  case 17: case 19: case 20: case 22:
  case 25: case 26: case 27: case 28: case 29: case 30:
  case 33: case 34: case 35: case 36: case 37: case 38:
  case 41: case 43: case 44: case 46:
  case 50: case 51: case 52: case 53:
    return 2 * num + 1;
  }
  g_assert (0);
  return 0;
}

