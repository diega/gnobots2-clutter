void reversi_init_board (gchar **board);

void reversi_destroy_board (gchar **board);

gboolean is_valid_move (gchar *board, gchar index, gchar player);

void move (gchar *board, gchar index, gchar player);

gchar other_player (gchar player);
