#ifndef __LOGICAL_BOARD_PC_INCLUDED__
#define __LOGICAL_BOARD_PC_INCLUDED__

#include "netcon.h"
#include "enums.h"


@interface Logical_Board_PC : Object
{
  netcon *nc;
  int ch; /* channel to server */
}

- init : (netcon *) set_nc
       : (char *) remote_machine
       : (int) remote_port;
- (void) close_channel;
- (int) get_ch;
- (void) load_file : (char *) file_name;
- (char *) mouse_button_press : (int) x : (int) y
			      : (int) n : (Player_Color) owners_color;
- (void) key_press : (int) key;
- (void) add_board_view : (char *) machine_name : (int) port;
- (void) delete_board_view : (char *) machine_name : (int) port;
- (void) delete_board_view : (id *) bv;
- (void) fire : (Player_Color) owners_color;
- (void) unfire;
- (void) pass : (Player_Color) owners_color;
- (void) request_color;
- (void) kill_server;

@end

#endif /* __LOGICAL_BOARD_PC_INCLUDED__ */
