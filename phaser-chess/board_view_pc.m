#include "board_view_pc.h"
#include "enums.h"

static int debug=0;

@implementation Board_View_PC : Object

- init : (netcon *) set_nc
       : (int) set_ch
{
  nc = set_nc;
  ch = set_ch;

  /* open connection to remove server */
  /*ch = [nc open_channel : remote_machine : remote_port];*/

  return self;
}

- (void) set_logical_board : (char *) machine_name : (int) port
{
  int slen = strlen (machine_name);
  int buffer_length = 2 + slen + 2 + 2;
  char buf[ buffer_length ];
  char *b = buf;

  int_to_two_bytes (pc_set_logical_board, b); b += 2;
  int_to_two_bytes (slen, b); b += 2;
  strncpy (b, machine_name, slen); b += slen;
  int_to_two_bytes (port, b); b += 2;

  [nc throw : ch : buf : buffer_length : STREAM];
}

/*
- (void) process_message : (char *) message : (int) len
{
  int slen = strlen (message);
  int buffer_length = 2 + slen + 2 + 2;
  char buf[ buffer_length ];
  char *b = buf;

  int_to_two_bytes (pc_process_message, b); b += 2;
  int_to_two_bytes (slen, b); b += 2;
  strncpy (b, message, slen); b += slen;
  int_to_two_bytes (len, b); b += 2;

  [nc throw : ch : buf : buffer_length : STREAM];
}
*/


- (void) place_beam : (int) x : (int) y : (int) direction
{
  int buffer_length = 2 + 2 + 2 + 2;
  char buf[ buffer_length ];
  char *b = buf;

  int_to_two_bytes (pc_place_beam, b); b += 2;
  int_to_two_bytes (x, b); b += 2;
  int_to_two_bytes (y, b); b += 2;
  int_to_two_bytes (direction, b); b += 2;

  [nc throw : ch : buf : buffer_length : STREAM];
}

- (void) remove_beam
{
  int buffer_length = 2;
  char buf[ buffer_length ];
  char *b = buf;

  int_to_two_bytes (pc_remove_beam, b); b += 2;
  [nc throw : ch : buf : buffer_length : STREAM];
}


- (void) clear_board
{
  int buffer_length = 2;
  char buf[ buffer_length ];
  char *b = buf;

  int_to_two_bytes (pc_clear_board, b); b += 2;
  [nc throw : ch : buf : buffer_length : STREAM];
}

- (void) place_piece : (int) x : (int) y
		     : (int) kind_of_piece
		     : (int) direction
		     : (int) col
{
  int buffer_length = 2 + 2 + 2 + 2 + 2 + 2;
  char buf[ buffer_length ];
  char *b = buf;

  int_to_two_bytes (pc_place_piece, b); b += 2;
  int_to_two_bytes (x, b); b += 2;
  int_to_two_bytes (y, b); b += 2;
  int_to_two_bytes (kind_of_piece, b); b += 2;
  int_to_two_bytes (direction, b); b += 2;
  int_to_two_bytes (col, b); b += 2;

  [nc throw : ch : buf : buffer_length : STREAM];
}

- (void) clear_square : (int) x : (int) y
{
  int buffer_length = 2 + 2 + 2;
  char buf[ buffer_length ];
  char *b = buf;

  int_to_two_bytes (pc_clear_square, b); b += 2;
  int_to_two_bytes (x, b); b += 2;
  int_to_two_bytes (y, b); b += 2;

  [nc throw : ch : buf : buffer_length : STREAM];
}


- (void) take_color : (Player_Color) c
{
  int buffer_length = 2 + 2;
  char buf[ buffer_length ];
  char *b = buf;

  if (debug)
    printf ("board_view_pc -- take_color %d\n", c);

  int_to_two_bytes (pc_take_color, b); b += 2;
  int_to_two_bytes (c, b); b += 2;

  [nc throw : ch : buf : buffer_length : STREAM];
}


- (void) update_moves : (int) red_moves : (int) green_moves
{
  int buffer_length = 2 + 2 + 2;
  char buf[ buffer_length ];
  char *b = buf;

  if (debug)
    printf ("board_view_pc -- update_moves %d, %d\n", red_moves, green_moves);


  int_to_two_bytes (pc_update_moves, b); b += 2;
  int_to_two_bytes (red_moves, b); b += 2;
  int_to_two_bytes (green_moves, b); b += 2;

  [nc throw : ch : buf : buffer_length : STREAM];

  if (debug)
    printf (" ... done\n");
}


- (void) send_message : (char *) message
{
  int slen = strlen (message);
  int buffer_length = 2 + slen + 2;
  char buf[ buffer_length ];
  char *b = buf;

  if (debug)
    printf ("board_view_pc -- send_message\n");

  int_to_two_bytes (pc_send_message, b); b += 2;
  int_to_two_bytes (slen, b); b += 2;
  strncpy (b, message, slen); b += slen;

  [nc throw : ch : buf : buffer_length : STREAM];
}

@end
