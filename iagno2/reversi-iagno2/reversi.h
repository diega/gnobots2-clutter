#ifndef _REVERSI_IAGNO2_H_
#define _REVERSI_IAGNO2_H_

#define BOARDSIZE 8
#define INDEX(r,c) ((r << 3) + c)
#define ROW(x)     (x >> 3)
#define COL(x)     (x & 0x7)

#define WHITE_TILE 31
#define BLACK_TILE 1

#define OTHER_TILE(x) (x-1)?BLACK_TILE:WHITE_TILE

typedef struct {
  int index;
  int player;
} ReversiMove;

typedef struct {
  gchar *board;
  ReversiMove *moves;
  int move_count;
} ReversiBoard;

ReversiBoard *reversi_init_board ();

void reversi_destroy_board (ReversiBoard *board);

gboolean is_valid_move (ReversiBoard *board, gchar index, gchar player);

void move (ReversiBoard *board, gchar index, gchar player);

/*
gchar other_player (gchar player);
*/

gboolean are_valid_moves (ReversiBoard *board, gchar player);

/*
ReversiMove get_move (ReversiBoard board, int move);
*/

#endif
