#ifndef __BOARD_VIEW_PS_INCLUDED__
#define __BOARD_VIEW_PS_INCLUDED__

#include "vector.h"
#include "config.h"
#include "board_view.h"
#include "game_info.h"
#include "vector.h"
#include "netcon.h"


@interface Board_View_PS : netcon
{
  Board_View *bv;
  /*XtAppContext app_context;*/

  id *game_finder;
  Vector *XtInputIds;

  int channel_to_metaserver;
}

- init_ps : (int) listen_port
	  : (Board_View *) set_bv;
- (void) set_game_finder : (id *) set_game_finder;
- (void) handle_new_connection : (int) new_channel;
- (void) handle_gone_connection : (int) channel;
- (void) process_data : (int) channel : (char *) data : (int) length;

- (Vector *) search_machine_for_servers : (char *) host
					: (int) server_port;
- (void) ask_metaserver;

@end

#endif /* __BOARD_VIEW_PS_INCLUDED__ */
