#include "game_finder.h"
#include "board_view.h"
#include "logical_board.h"
#include "phaser-chess.h"

@implementation phaser_chess_app

- initApp:(int *)argcp
	 :(char ***)argvp
{
  Game_Finder *gf;
  Board_View_PS *bvps;
  char *server_machine = "localhost";
  int server_port = 5545;


  int listen_port = 6000 + (getpid () % 23000);

  self = [super initApp:argcp :argvp];

  /*  local game...
  lb = [[Logical_Board alloc] init_logical_board];
  bv = [[Board_View alloc] init_board : *argcp : *argvp];
  [bv set_logical_board : (id *) lb];
  [lb add_board_view : bv];
  [lb load_file : "startup.pcs"];
  */

  bv = [[Board_View alloc] init_board : *argcp : *argvp];
  bvps = [[Board_View_PS alloc] init_ps : listen_port : bv];

  if (bvps == NULL)
    {
      fprintf (stderr, "bvps is NULL.\n");
      exit (1);
    }

  gf = [[Game_Finder alloc] init_gf
			    : *argcp : *argvp
			    : bv : bvps
			    : server_machine
			    : server_port
			    : listen_port];

  [bv set_bvps : (id *) bvps];
  [bv set_gf : (id *) gf];

  return self;
}

- free
{
  [super free];
  return self;
}
@end


void stdinInputFunction (gpointer data,
			 gint source,
			 GdkInputCondition condition)
{
  char ack[ 400 ];
  printf ("in stdinInputFunction\n");
  fgets (ack, 400, stdin);
  printf ("  got-->%s\n", ack);
}


int main(int argc, char *argv[])
{
  id myApp;

  srandom (time (0));

  myApp = [[phaser_chess_app alloc] initApp:&argc :&argv];

  gdk_input_add (0, GDK_INPUT_READ, stdinInputFunction, NULL);

  [myApp run];
  [myApp free];
  return 0;
}
