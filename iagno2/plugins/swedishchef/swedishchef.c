#include <gnome.h>
#include <sys/time.h>
#include <unistd.h>

#include "../../reversi-iagno2/reversi.h"


#define VERSION "0.0.1"
#define BUSY_MESSAGE "Swedish Chef is thinking..."
#define DEFAULT_THINKING_TIME "0.5"

/* #define DEBUG */


/* ------------------------------------------------------------ */

typedef unsigned char UBYTE;
typedef UBYTE BoardType[10][16];


static void findMove (BoardType board, gint *xp, gint *yp, gint player);
static gint search (BoardType board, gint oldscore, gint player,
		   gint alpha, gint beta,
		   gint ply, gint depth, gboolean pass);
static gint numFlip (BoardType board, gint x, gint y, gint player);
static void flip (BoardType board, gint x, gint y, gint player);
static gint sumBricks (BoardType board, gint player);
static gint rankMove (BoardType board, gint x, gint y, gint num);
static void moveBoard (BoardType from, BoardType to);


static gint Combs;
static gint numTiles;
static gint nflip[8];
static gint xstart[64], ystart[64];
#ifdef DEBUG
static gint fh, fhGood;
#endif

static gdouble thinking_time;

/* ------------------------------------------------------------ */

void
plugin_init ()
{
  struct timeval tv;
  gchar *timestr;

  gettimeofday (&tv, NULL);
  srand (tv.tv_sec + tv.tv_usec);

  timestr = gnome_config_get_string ("/iagno2/swedishchef/thinking_time="
				     DEFAULT_THINKING_TIME);
  sscanf (timestr, "%lg", &thinking_time);
  g_free (timestr);

#ifdef DEBUG
  printf ("time:%g\n", thinking_time);
#endif
}


static gint
color_convert (char iagno_color)
{
  if (iagno_color == BLACK_TILE)
    return 1;
  else if (iagno_color == WHITE_TILE)
    return 2;
  else
    return 0;
}

gint
plugin_move (ReversiBoard *board, gint move)
{
  gint x, y;
  BoardType myboard;
  gint xx, yy;

#ifdef DEBUG
  gint i;
  printf ("\nturn:%d\n", move);
  for (i = 0; i < 64; i++) {
    printf ("%2d ", board->board[i]);
    if (((i + 1) % 8) == 0) {
      printf ("\n");
    }
  }
#endif

  for (y = 0; y < 8; y++) {
    for (x = 0; x < 8; x++) {
      gchar tile = board->board[x * 8 + y];
      myboard[x + 1][y + 1] = color_convert (tile);
    }
  }

  findMove (myboard, &xx, &yy, color_convert (move));

#ifdef DEBUG
    printf ("xx:%d yy:%d\n\n", xx, yy);
#endif

  return (xx - 1) * 8 + (yy - 1);
}

const gchar *
plugin_name ()
{
  return "Swedish Chef";
}

const gchar *
plugin_busy_message ()
{
  return BUSY_MESSAGE;
}

void
plugin_about_window (GtkWindow *parent)
{
  GtkWidget *about;
  const gchar *authors[] = {
    "Ian Peters",
    "Peter Österlund",
    NULL
  };

  about = gnome_about_new (_("Swedish Chef for Iagno II"), VERSION,
                           "Copyright 2000 Ian Peters",
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


static gint
save_string (GtkWidget *widget, gpointer data)
{
  gchar buffer[32];
  GtkAdjustment *adj;

  adj = (GtkAdjustment *) data;

  thinking_time = adj->value;
#ifdef DEBUG
  printf("time:%g\n", thinking_time);
#endif
  sprintf(buffer, "%g", thinking_time);
  gnome_config_set_string ("/iagno2/swedishchef/thinking_time", buffer);

  gnome_config_sync ();
  return TRUE;
}

void
plugin_preferences (GtkWidget *parent)
{
  GtkWidget *dialog;
  GtkWidget *hbox, *label, *scale;
  gchar *timestr;
  GtkObject *adj;

  dialog = gnome_dialog_new (_("Swedish Chef Configuration"),
                             GNOME_STOCK_BUTTON_OK,
                             GNOME_STOCK_BUTTON_CANCEL,
                             NULL);

  gnome_dialog_set_parent (GNOME_DIALOG (dialog), GTK_WINDOW (parent));

  timestr = gnome_config_get_string ("/iagno2/swedishchef/thinking_time="
				     DEFAULT_THINKING_TIME);
  sscanf (timestr, "%lg", &thinking_time);

  hbox = gtk_hbox_new (FALSE, 10);
  label = gtk_label_new ("Thinking time:");
  adj = gtk_adjustment_new (thinking_time, 0.0, 10.0, 0.1, 0.1, 0.0);
  scale = gtk_hscale_new (GTK_ADJUSTMENT (adj));

  g_free (timestr);

  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), scale, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (dialog)->vbox), hbox, FALSE, TRUE, 5);

  gtk_widget_show (hbox);
  gtk_widget_show (label);
  gtk_widget_show (scale);

  gnome_dialog_button_connect (GNOME_DIALOG (dialog),
                               0,
                               GTK_SIGNAL_FUNC (save_string),
                               (gpointer) adj);

  gnome_dialog_run_and_close (GNOME_DIALOG (dialog));
}

/* ------------------------------------------------------------ */

static gint delta[8] = { -16, -15, 1, 17, 16, 15, -1, -17 };


#define WINSCORE  (10000)
#define LOSESCORE (-10000)

static void
findMove (BoardType board, gint *xp, gint *yp, gint player)
{
  UBYTE TmpBoard[10][16];
  UBYTE xMoves[64], yMoves[64];
  gint i, j;
  GTimer *gt;
  gint NumMoves = 0;
  gint depth = 2;

  numTiles = 0;
  for (i = 1; i < 9; i++)
    for (j = 1; j < 9; j++)
      if (board[i][j])
	numTiles++;
  for (i = 0; i < 64; i++) {
    xstart[i] = 1;
    ystart[i] = 1;
  }

  gt = g_timer_new ();
  g_timer_start (gt);

  Combs = 0;
  do {
    gint alpha = 2 * LOSESCORE;
    gint beta = 2 * WINSCORE;
    gint i, j;
    gint NumLeft = 64;

#ifdef DEBUG
    printf ("depth:%2d\n", depth);
    fh = fhGood = 0;
#endif

    for (i = xstart[0], j = ystart[0]; NumLeft > 0; NumLeft--) {
      gint num;
      if ((board[i][j] == 0) && ((num = numFlip (board, i, j, player)) > 0)) {
	gint tmpscore, score;

	moveBoard (board, TmpBoard);
	flip (TmpBoard, i, j, player);
	numTiles++;

	tmpscore = rankMove (board, i, j, num);
	score = -search (TmpBoard, -tmpscore, 3 - player,
			      -beta, -(alpha-1), 1, depth - 1, FALSE);
	numTiles--;

	if (score > alpha) {
	  NumMoves = 0;
	  xstart[0] = i; ystart[0] = j;
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
    printf ("combs:%d fh:%.2f\n", Combs, (double)fhGood / fh);
#endif
    depth++;
  } while ((g_timer_elapsed (gt, NULL) < thinking_time) && (depth + numTiles <= 64));
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
search (BoardType board, gint oldscore, gint player, gint alpha, gint beta,
	gint ply, gint depth, gboolean pass)
{
  gint x, y;
  UBYTE TmpBoard[10][16];
  gint NumLeft;
  gint validMoves = 0;

  if (depth <= 0)
    return oldscore;

  Combs++;

  for (x = xstart[ply], y = ystart[ply], NumLeft = 64; NumLeft > 0; NumLeft--) {
    gint num;
    if ((board[x][y] == 0) && ((num = numFlip (board, x, y, player)) > 0)) {
      gint tmpscore, score;

      validMoves++;
      moveBoard (board, TmpBoard);
      flip (TmpBoard, x, y, player);
      numTiles++;
      if (numTiles >= 64) {
	numTiles--;
	return sumBricks (TmpBoard, player);
      }

      tmpscore = oldscore + rankMove (board, x, y, num);
      score = -search (TmpBoard, -tmpscore, 3 - player,
		       -beta, -alpha,
		       ply + 1, depth - 1, FALSE);
      numTiles--;

      if (score > alpha) {
	xstart[ply] = x; ystart[ply] = y;
	if (score >= beta) {
#ifdef DEBUG
	  fh++;
	  if (validMoves == 1)
	    fhGood++;
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
      return sumBricks (board, player);
    } else {
      return -search (board, -oldscore, 3 - player, -beta, -alpha,
		      ply + 1, depth, TRUE);
    }
  }
  return alpha;
}

static gint
numFlip (BoardType board, gint x, gint y, gint player)
{
  UBYTE *ptr;
  UBYTE *orgptr = &board[x][y];
  gint num = 0;
  gint i;

  for (i = 7; i >= 0; i--) {
    nflip[i] = 0; x = delta[i];
    ptr = orgptr + x;
    if ( ((*ptr) + player) == 3) {
      ptr += x; y = 1;
      while ( ((*ptr) + player) == 3) {
	ptr += x; y++;
      }
      if (*ptr == player) {
	num += y; nflip[i] = y;
      }
    }
  }
  return num;
}

static void
flip (BoardType board, gint x, gint y, gint player)
{
  UBYTE *ptr;
  UBYTE *orgptr = &board[x][y];
  gint i;

  *orgptr = player;
  for (i = 7; i >= 0; i--) {
    y = nflip[i]; x = delta[i];
    ptr = orgptr;
    while (y > 0) {
      ptr += x; *ptr = player;
      y--;
    }
  }
}

static gint
sumBricks (BoardType board, gint player)
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
rankMove (BoardType board, gint x, gint y, gint num)
{
  switch (8 * x + y - 9) {
  case 0: case 7: case 56: case 63:
    return ((nflip[0] + nflip[2] + nflip[4] + nflip[6]) *
	    (2 * (S41 - 1)) + 2 * num + S11);
  case 1:
    if (board[1][1])
      goto hor_edge;
    else
      return ((nflip[2] + nflip[6]) * (2 * (S41 - 1)) +
	      2 * num + S21);
  case 2:
    if (board[1][1])
      goto hor_edge;
    else
      return ((nflip[2] + nflip[6]) * (2 * (S41 - 1)) +
	      2 * num + S31);
  case 3: case 4: case 59: case 60: hor_edge:
    return (nflip[2] + nflip[6]) * (2 * (S41 - 1)) + 2 * num + S41;
  case 5:
    if (board[8][1])
      goto hor_edge;
    else
      return ((nflip[2] + nflip[6]) * (2 * (S41 - 1)) +
	      2 * num + S31);
  case 6:
    if (board[8][1])
      goto hor_edge;
    else
      return ((nflip[2] + nflip[6]) * (2 * (S41 - 1)) +
	      2 * num + S21);
  case 8:
    if (board[1][1])
      goto ver_edge;
    else
      return ((nflip[0] + nflip[4]) * (2 * (S41 - 1)) +
	      2 * num + S21);
  case 9:
    if (board[1][1])
      return (2 * num + 1);
    else
      return (2 * num + S22);
  case 14:
    if (board[8][1])
      return (2 * num + 1);
    else
      return (2 * num + S22);
  case 15:
    if (board[8][1])
      goto ver_edge;
    else
      return ((nflip[0] + nflip[4]) * (2 * (S41 - 1)) +
	      2 * num + S21);
  case 16:
    if (board[1][1])
      goto ver_edge;
    else
      return ((nflip[0] + nflip[4]) * (2 * (S41 - 1)) +
	      2 * num + S31);
  case 18: case 21: case 42: case 45:
    return 2 * num + S33;
  case 23:
    if (board[8][1])
      goto ver_edge;
    else
      return ((nflip[0] + nflip[4]) * (2 * (S41 - 1)) +
	      2 * num + S31 );
  case 24: case 31: case 32: case 39: ver_edge:
    return ((nflip[0] + nflip[4]) * (2 * (S41 - 1)) + 2 * num + S41 );
  case 40:
    if (board[1][8])
      goto ver_edge;
    else
      return ((nflip[0] + nflip[4]) * (2 * (S41 - 1)) +
	      2 * num + S31 );
  case 47:
    if (board[8][8])
      goto ver_edge;
    else
      return ((nflip[0] + nflip[4]) * (2 * (S41 - 1)) +
	      2 * num + S31 );
  case 48:
    if (board[1][8])
      goto ver_edge;
    else
      return ((nflip[0] + nflip[4]) * (2 * (S41 - 1)) +
	      2 * num + S21 );
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
      return ((nflip[0] + nflip[4]) * (2 * (S41 - 1)) +
	      2 * num + S21);
  case 57:
    if (board[1][8])
      goto hor_edge;
    else
      return ((nflip[2] + nflip[6]) * (2 * (S41 - 1)) +
	      2 * num + S21 );
  case 58:
    if (board[1][8])
      goto hor_edge;
    else
      return ((nflip[2] + nflip[6]) * (2 * (S41 - 1)) +
	      2 * num + S31 );
  case 61:
    if (board[8][8])
      goto hor_edge;
    else
      return ((nflip[2] + nflip[6]) * (2 * (S41 - 1)) +
	      2 * num + S31 );
  case 62:
    if (board[8][8])
      goto hor_edge;
    else
      return ((nflip[2] + nflip[6]) * (2 * (S41 - 1)) +
	      2 * num + S21 );
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

static void
moveBoard (BoardType from, BoardType to)
{
  memcpy (to, from, sizeof (BoardType));
}
