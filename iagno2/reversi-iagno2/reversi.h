#ifndef _REVERSI_IAGNO2_H_
#define _REVERSI_IAGNO2_H_

void reversi_init_board (gchar **board);

void reversi_destroy_board (gchar **board);

gboolean is_valid_move (gchar *board, gchar index, gchar player);

void move (gchar *board, gchar index, gchar player);

gchar other_player (gchar player);

gboolean are_valid_moves (gchar *board, gchar player);

#endif
