#include "logical_board_pc.h"

static int debug=0;


@implementation Logical_Board_PC : Object

- init : (netcon *) set_nc
       : (char *) remote_machine
       : (int) remote_port
{
  nc = set_nc;

  /* open connection to remove server */
  if (debug)
    printf ("logical board pc opening channel to server...\n");

  ch = [nc open_channel : remote_machine : remote_port];

  if (ch == -1)
    return NULL;

  return self;
}


- (void) close_channel
{
  if (ch != -1)
    [nc close_channel : ch];
}

- (int) get_ch
{
  return ch;
}


- (void) load_file : (char *) file_name
{
  int slen = strlen (file_name);
  int buffer_length = 2 + slen;
  char buf[ buffer_length ];
  char *b = buf;

  int_to_two_bytes (pc_load_file, b); b += 2;
  int_to_two_bytes (slen, b); b += 2;
  strncpy (b, file_name, slen); b += slen;

  [nc throw : ch : buf : buffer_length : STREAM];
}

- (char *) mouse_button_press : (int) x : (int) y
			      : (int) n : (Player_Color) owners_color
{
  int buffer_length = 2 + 2 + 2 + 2 + 2;
  char buf[ buffer_length ];
  char *b = buf;

  printf ("In logical_board_pc mouse_button_press\n");

  int_to_two_bytes (pc_mouse_button_press, b); b += 2;
  int_to_two_bytes (x, b); b += 2;
  int_to_two_bytes (y, b); b += 2;
  int_to_two_bytes (n, b); b += 2;
  int_to_two_bytes (owners_color, b); b += 2;

  printf ("  ... done encoding\n");

  [nc throw : ch : buf : buffer_length : STREAM];

  printf ("  ... done sending\n");

  return NULL;
}

- (void) key_press : (int) key
{
  int buffer_length = 2 + 2;
  char buf[ buffer_length ];
  char *b = buf;

  int_to_two_bytes (pc_key_press, b); b += 2;
  int_to_two_bytes (key, b); b += 2;

  [nc throw : ch : buf : buffer_length : STREAM];
}

- (void) add_board_view : (char *) machine_name : (int) port
{
  int slen = strlen (machine_name);
  int buffer_length = 2 + slen + 2 + 2;
  char buf[ buffer_length ];
  char *b = buf;

  int_to_two_bytes (pc_add_board_view, b); b += 2;
  int_to_two_bytes (slen, b); b += 2;
  strncpy (b, machine_name, slen); b += slen;
  int_to_two_bytes (port, b); b += 2;

  [nc throw : ch : buf : buffer_length : STREAM];
}

- (void) delete_board_view : (char *) machine_name : (int) port
{
  if (debug)
    printf ("logical board pc ignoring delete_board_view #1\n");
}

- (void) delete_board_view : (id *) bv
{
  if (debug)
    printf ("logical board pc ignoring delete_board_view #2\n");
}

- (void) fire : (Player_Color) owners_color
{
  int buffer_length = 2 + 2;
  char buf[ buffer_length ];
  char *b = buf;

  int_to_two_bytes (pc_fire, b); b += 2;
  int_to_two_bytes (owners_color, b); b += 2;

  [nc throw : ch : buf : buffer_length : STREAM];
}

- (void) unfire
{
  int buffer_length = 2;
  char buf[ buffer_length ];
  char *b = buf;

  int_to_two_bytes (pc_unfire, b); b += 2;

  [nc throw : ch : buf : buffer_length : STREAM];
}

- (void) pass : (Player_Color) owners_color
{
  int buffer_length = 2 + 2;
  char buf[ buffer_length ];
  char *b = buf;

  int_to_two_bytes (pc_pass, b); b += 2;
  int_to_two_bytes (owners_color, b); b += 2;

  [nc throw : ch : buf : buffer_length : STREAM];
}


- (void) request_color
{
  int buffer_length = 2;
  char buf[ buffer_length ];
  char *b = buf;

  int_to_two_bytes (pc_request_color, b); b += 2;

  [nc throw : ch : buf : buffer_length : STREAM];
}


- (void) kill_server
{
  int buffer_length = 2;
  char buf[ buffer_length ];
  char *b = buf;

  int_to_two_bytes (pc_kill_server, b); b += 2;

  [nc throw : ch : buf : buffer_length : STREAM];
}

@end
