#include "board_view_ps.h"
#include "logical_board_pc.h"
#include "game_finder.h"
#include "enums.h"
#include "meta_server.h"

static int debug=0;

Board_View_PS *global_bvps;

void call_world_think (gpointer data,
		       gint source,
		       GdkInputCondition condition)
{
  [global_bvps catch : 0 : 5];
}





@implementation Board_View_PS : netcon

- init_ps : (int) listen_port
	  : (Board_View *) set_bv
{
  [super init : listen_port];

  bv = set_bv;
  game_finder = NULL;
  channel_to_metaserver = -1;

  gdk_input_add ([self get_stream_listen_file_d], GDK_INPUT_READ,
		 call_world_think, NULL);

  gdk_input_add ([self get_dgram_file_d], GDK_INPUT_READ,
		 call_world_think, NULL);


  global_bvps = self;



  XtInputIds = [[Vector alloc] init];

  return self;
}


- (void) set_game_finder : (id *) set_game_finder
{
  game_finder = set_game_finder;
}


- (void) handle_new_connection : (int) new_channel
{
  gint xii;

  if (debug)
    printf("done catching inbound connection on channel %d\n", new_channel);

  if (sizeof (gint) > sizeof (unsigned char *))
    {
      fprintf (stderr, "oh this just isn't going to work...\n");
      exit (1);
    }

  xii = gdk_input_add ([self get_stream_file_d : new_channel],
		       GDK_INPUT_READ,
		       call_world_think, NULL);


  [XtInputIds set : new_channel : (unsigned char *) xii];
}


- (void) handle_gone_connection : (int) channel
{
  gint xii;

  if (debug)
    printf ("channel %d has been closed remotely\n", channel);


  xii = (gint) [XtInputIds get : channel];

  if (((unsigned char *) xii) != NULL)
    gdk_input_remove (xii);
}


- (void) ask_metaserver
{
  char buf[ 2 ];
  char *b = buf;
  gint xii;

  if (channel_to_metaserver < 0)
    {
      channel_to_metaserver = [self open_channel : META_SERVER : 5544];

      if (channel_to_metaserver < 0)
	{
	  fprintf (stderr, "can't reach the metaserver.\n");
	  return;
	}

      xii = gdk_input_add ([self get_stream_file_d : channel_to_metaserver],
			   GDK_INPUT_READ,
			   call_world_think, NULL);

      [XtInputIds set : channel_to_metaserver : (unsigned char *) xii];
    }

  int_to_two_bytes (pc_tell_of_games, b); b += 2;

  [self throw : channel_to_metaserver : buf : b - buf : STREAM];
}


- (Vector *) search_machine_for_servers : (char *) host
					: (int) server_port
{
  Vector *gil = [[Vector alloc] init];
  int p = server_port;
  int ch = 0;

  for (p=server_port; p<server_port+12; p++)
    {
      ch = [self open_channel : host : p];

      if (ch >= 0)
	{
	  char *remote_ip = [self get_remote_ip_addr : ch];

	  [self handle_new_connection : ch];
	  [gil add : (unsigned char *) new_game_info ("unknown",
						      host,
						      "",
						      remote_ip,
						      p,
						      0)];

	  [self close_channel : ch];
	  [self handle_gone_connection : ch];
	}
    }

  return gil;
}


- (void) process_data : (int) channel : (char *) data : (int) length
{
  char *b = data;
  int x, y, z, d, c, slen;
  char *s;
  Logical_Board_PC *lbpc;
  game_info *gi;

  bv_proxy_command pc = two_bytes_to_int (b); b += 2;

  if (debug)
    printf ("bvps: rec %d byte: %d\n", length, pc);

  switch (pc)
    {
    case pc_set_logical_board:
      slen = two_bytes_to_int (b); b += 2;
      s = (char *) malloc (slen + 1);
      strncpy (s, b, slen); s[ slen ] = '\0'; b += slen;
      x = two_bytes_to_int (b); b += 2;
      lbpc = [[Logical_Board_PC alloc] init : self : s : x];
      [bv set_logical_board : (id *) lbpc];
      free (s);
      break;
      /*
    case pc_process_message:
      slen = two_bytes_to_int (b); b += 2;
      s = (char *) malloc (slen + 1);
      strncpy (s, b, slen); s[ slen ] = '\0'; b += slen;
      x = two_bytes_to_int (b); b += 2;
      [bv process_message : s : x];
      free (s);
      break;
      */
    case pc_place_beam:
      x = two_bytes_to_int (b); b += 2;
      y = two_bytes_to_int (b); b += 2;
      z = two_bytes_to_int (b); b += 2;
      [bv place_beam : x : y : z];
      break;
    case pc_remove_beam:
      [bv remove_beam];
      break;
    case pc_clear_square:
      x = two_bytes_to_int (b); b += 2;
      y = two_bytes_to_int (b); b += 2;
      [bv clear_square : x : y];
      break;
    case pc_place_piece:
      x = two_bytes_to_int (b); b += 2;
      y = two_bytes_to_int (b); b += 2;
      z = two_bytes_to_int (b); b += 2;
      d = two_bytes_to_int (b); b += 2;
      c = two_bytes_to_int (b); b += 2;
      [bv place_piece : x : y : z : d : c];
      break;
    case pc_clear_board:
      [bv clear_board];
      break;
    case pc_take_color:
      c = two_bytes_to_int (b); b += 2;
      [bv take_color : c];
      break;
    case pc_update_moves:
      x = two_bytes_to_int (b); b += 2;
      y = two_bytes_to_int (b); b += 2;
      [bv update_moves : x : y];
      break;
    case pc_send_message:
      slen = two_bytes_to_int (b); b += 2;
      s = (char *) malloc (slen + 1);
      strncpy (s, b, slen); s[ slen ] = '\0'; b += slen;
      [bv send_message : s];
      free (s);
      break;
    case pc_other_game:
      gi = decode_game_info (data + 2); /* +2 to skip the message */
      if (debug)
	printf ("board_view_ps: %s@%s.%s:%d, %d players\n",
		gi->uname,
		gi->host,
		gi->domain,
		gi->port,
		gi->num_players);
      /*delete_game_info (gi);*/
      if (game_finder != NULL)
	[((Game_Finder *) game_finder) append_to_game_list : gi];
      break;
    case pc_forget_games:
      if (game_finder != NULL)
	[((Game_Finder *) game_finder) forget_game_list];
      break;
    }

  if (debug)
    printf ("    : done with switch\n"); 
}

@end
