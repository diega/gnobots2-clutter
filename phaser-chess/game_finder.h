#ifndef __GAME_FINDER_H__
#define __GAME_FINDER_H__

#include "config.h"
#include "enums.h"
#include "vector.h"
#include "board_view.h"
#include "logical_board_pc.h"
#include "board_view_ps.h"


/*
void BP_Callback (Widget w, XtPointer client_data, XtPointer call_data);
void TF_Callback (Widget w, XtPointer client_data, XtPointer call_data);
void List_Callback (Widget w, XtPointer client_data, XtPointer call_data);
*/


@interface Game_Finder : Object
{
  GtkWidget *shell;
  GtkWidget *vbox;
  GtkWidget *machine_name;
  GtkWidget *game_list;

  char *search_machine_str;
  char *ask_metaserver_str;
  char *new_game_str;
  char *other_color_str;
  char *kill_server_str;
  char *quit_str;

  gint lbpc_XtInputId;

  Vector *games;
  game_info *current_game;

  Board_View *bv;
  Board_View_PS *bvps;
  Logical_Board_PC *lbpc;
  char *server_machine;
  int server_port;
  int listen_port;
}

- init_gf : (int) argc
	  : (char **) argv
	  : (Board_View *) set_bv
	  : (Board_View_PS *) set_bvps
	  : (char *) set_server_machine
	  : (int) set_server_port
	  : (int) set_listen_port;

- (void) fill_game_list;
- (void) delete_self;
/*
- (void) bp_callback : (Widget) w : (XtPointer) call_data;
- (void) tf_callback : (Widget) w : (XtPointer) call_data;
*/

- (void) list_callback : (int) pos;

- (void) connect_to_game : (char *) host
			 : (char *) domain
			 : (char *) ip
			 : (int) port;
- (void) start_server;
- (void) new_game;
- (void) search_machine;
- (void) append_to_game_list : (game_info *) gi;
- (void) forget_game_list;
/*- (XmString) game_info_to_XmString : (game_info *) gi;*/
- (char *) game_info_to_string : (game_info *) gi;

@end

extern void _XEditResCheckMessages();

#endif /* __GAME_FINDER_H__ */
