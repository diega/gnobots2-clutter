#ifndef __BOARD_VIEW_PC_INCLUDED__
#define __BOARD_VIEW_PC_INCLUDED__

#include "netcon.h"
#include "enums.h"


@interface Board_View_PC : Object
{
  netcon *nc;
  int ch; /* channel to server */
}

/*
- init : (netcon *) set_nc
       : (char *) remote_machine
       : (int) remote_port;
       */
- init : (netcon *) set_nc
       : (int) set_ch;

- (void) set_logical_board : (char *) machine_name : (int) port;
- (void) place_piece : (int) x : (int) y
		     : (int) kind_of_piece
		     : (int) direction
		     : (int) col;
- (void) clear_board;
- (void) clear_square : (int) x : (int) y;

- (void) place_beam : (int) x : (int) y : (int) direction;
- (void) remove_beam;

- (void) take_color : (Player_Color) c;
- (void) update_moves : (int) red_moves : (int) green_moves;
- (void) send_message : (char *) message;

@end

#endif /* __BOARD_VIEW_PC_INCLUDED__ */
