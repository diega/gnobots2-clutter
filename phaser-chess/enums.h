#ifndef ENUMS_H
#define ENUMS_H 1

typedef enum
{
    C_neutral = 0 ,
    C_red = 1,
    C_green = 2,
    C_both = 3
} Player_Color;

typedef enum  /* messages to logical_board */
{
  pc_load_file = 100,
  pc_mouse_button_press,
  pc_key_press,
  pc_add_board_view,
  pc_delete_board_view,
  pc_fire,
  pc_unfire,
  pc_pass,
  pc_request_color,
  pc_kill_server
} lb_proxy_command;


typedef enum   /* messages to board_view */
{
  pc_set_logical_board = 200,
  pc_clear_square,
  pc_place_piece,
  pc_clear_board,
  pc_place_beam,
  pc_remove_beam,
  pc_take_color,
  pc_update_moves,
  pc_send_message,
  pc_other_game,
  pc_forget_games
} bv_proxy_command;

typedef enum /* messages to metaserver */
{
  pc_im_a_server = 300,
  pc_tell_of_games
} ms_command;

#endif /* ENUMS_H */
