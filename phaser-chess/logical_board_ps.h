#ifndef __LOGICAL_BOARD_PS_INCLUDED__
#define __LOGICAL_BOARD_PS_INCLUDED__

#include "config.h"

#include "netcon.h"
#include "logical_board.h"


@interface Logical_Board_PS : netcon
{
  Logical_Board *lb;
  int max_channel_to_bv;
  id **channel_to_bv;

  int server_should_die;
  /*int meta_channel;*/ /* channel to the meta_server */
      int player_has_connected;
}

/*- init_ps : (int) listen_port : (Logical_Board *) lb;*/
- init_lbps : (int) listen_port : (Logical_Board *) set_lb;
- (void) handle_new_connection : (int) new_channel;
- (void) handle_gone_connection : (int) channel;
- (void) process_data : (int) channel : (char *) data : (int) length;
- (void) bv_on_channel : (Board_View *) bv : (int) channel;
- (void) tell_details_to_ms;

@end

#endif /* __LOGICAL_BOARD_PS_INCLUDED__ */
