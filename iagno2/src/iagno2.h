#ifndef _IAGNO2_H_
#define _IAGNO2_H_

/*
typedef struct {
  int index;
  int player;
} Iagno2Move;
*/

void iagno2_tileset_load ();

void iagno2_draw_tile (int tile, int index);

void iagno2_draw_tile_to_buffer (int tile, int index);

void iagno2_app_init ();

void iagno2_canvas_init ();

void iagno2_show_grid_lines ();

void iagno2_set_bg_color ();

void iagno2_force_board_redraw ();

void iagno2_move (gchar index);

void iagno2_board_changed ();

void iagno2_setup_current_player ();

void iagno2_initialize_players ();

gint iagno2_game_over ();

/*
Iagno2Move iagno2_get_move (int move);
*/

#endif
