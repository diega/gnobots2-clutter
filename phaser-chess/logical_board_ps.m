#include <unistd.h>
#include "config.h"
#include "logical_board_ps.h"
#include "board_view_pc.h"
#include "game_info.h"
#include "enums.h"
#include "meta_server.h"


static int debug=0;


@implementation Logical_Board_PS : netcon

- init_lbps : (int) listen_port : (Logical_Board *) set_lb
{
  if ([super init : listen_port] == NULL)
    return NULL;

  lb = set_lb;
  server_should_die = 0;

  max_channel_to_bv = 5;
  channel_to_bv = (id **) malloc (sizeof (id *) * max_channel_to_bv);


  /* try to tell the metaserver about this server */

  [self tell_details_to_ms];

  player_has_connected = 0;

  return self;
}


- (void) tell_details_to_ms
{
  game_info *gi;
  char *user_name;
  char hostname[ 256 ];
  char domainname[ 256 ];
  int num_players = [lb number_of_players];
  char *buf;
  int len;

  user_name = getlogin ();
  if (user_name == NULL)
    {
#     if HAVE_CUSERID
      user_name = cuserid (NULL);
      if (user_name == NULL)
#     endif
	user_name = "unknown";
    }


  gethostname (hostname, 256);
  getdomainname (domainname, 256);

  gi = new_game_info (user_name,
		      hostname,
		      domainname,
		      [self get_local_ip_addr],
		      stream_listen_port,
		      num_players);

  buf = encode_game_info (pc_im_a_server, gi, &len);

  /*[self throw : channel : buf : len : STREAM];*/
  [self throw_connectionless_dgram : META_SERVER : 5544 : buf : len];

  delete_game_info (gi);

  free (buf);
}


- (void) handle_new_connection : (int) new_channel
{
  if (debug)
    printf("done catching inbound connection on channel %d\n", new_channel);
}


- (void) handle_gone_connection : (int) channel
{
  int lop;

  if (debug)
    printf ("channel %d has been closed remotely\n", channel);
  [lb delete_board_view : (Board_View *) (channel_to_bv[ channel ])];

  if (player_has_connected == 0)
      return;

  for (lop=0; lop<max_others; lop++)
    if (addresses[ lop ] != NULL /*&& lop != meta_channel*/)
      return;

  /* they all went away... */

  /* FIX ME! */
  /* this is broken.  there is no way to tell
     the meta server that this game is leaving. */

  [self tell_details_to_ms];

  /*if (server_should_die)*/
  exit (0);
}


- (void) process_data : (int) channel : (char *) data : (int) length
{
  char *b = data;
  int x, y, z, c, slen;
  char *s;
  Board_View_PC *bvpc;
  char *message = NULL;

  lb_proxy_command pc = two_bytes_to_int (b); b += 2;

  if (debug)
    printf ("lbps: rec %d bytes\n", length);

  switch (pc)
    {
    case pc_load_file:
      slen = two_bytes_to_int (b); b += 2;
      s = (char *) malloc (slen + 1);
      strncpy (s, b, slen); s[ slen ] = '\0'; b += slen;
      [lb load_file : s];
      free (s);
      break;
    case pc_mouse_button_press:
      if (debug)
	printf ("logical_board_ps got button press message\n");
      x = two_bytes_to_int (b); b += 2;
      y = two_bytes_to_int (b); b += 2;
      z = two_bytes_to_int (b); b += 2;
      c = two_bytes_to_int (b); b += 2;
      if (debug)
	printf ("   %d, %d, %d, %d\n", x, y, z, c);
      message = [lb mouse_button_press : x : y : z : c];
      if (debug)
	printf ("   ...done\n");
      break;
    case pc_key_press:
      z = two_bytes_to_int (b); b += 2;
      [lb key_press : z];
      break;
    case pc_add_board_view:
      slen = two_bytes_to_int (b); b += 2;
      s = (char *) malloc (slen + 1);
      strncpy (s, b, slen); s[ slen ] = '\0'; b += slen;
      x = two_bytes_to_int (b); b += 2;
      /* bvpc = [[Board_View_PC alloc] init : self : s : x]; */
      bvpc = [[Board_View_PC alloc] init : self : channel];
      {
	[lb add_board_view : (Board_View *) bvpc];
	[self bv_on_channel : (Board_View *) bvpc : channel];
      }
      free (s);
      break;
    case pc_delete_board_view:
      break;
    case pc_fire:
      c = two_bytes_to_int (b); b += 2;
      message = [lb fire : c];
      break;
    case pc_unfire:
      [lb unfire];
      break;
    case pc_pass:
      c = two_bytes_to_int (b); b += 2;
      message = [lb pass : c];
      break;
    case pc_request_color:
      [lb request_color : (Board_View *) (channel_to_bv[ channel ])];
      [self tell_details_to_ms];
      player_has_connected = 1;
      break;
    case pc_kill_server:
      server_should_die = 1;
      break;
    }

  if (message != NULL)
    {
      [(Board_View *) channel_to_bv[ channel ] send_message : message];
      if (debug)
	printf ("-->%s\n", message);
    }
}


- (void) bv_on_channel : (Board_View *) bv : (int) channel
{
  if (channel >= max_channel_to_bv)
    {
      max_channel_to_bv = channel + 3;
      channel_to_bv = (id **) realloc (channel_to_bv,
				       sizeof (id *) *
				       max_channel_to_bv);
    }

  channel_to_bv[ channel ] = (id *) bv;
}

@end
