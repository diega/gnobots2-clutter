#include "meta_server.h"
#include "config.h"
#include "game_finder.h"
#include "logical_board.h"
#include "logical_board_ps.h"


void call_world_think (gpointer data,
		       gint source,
		       GdkInputCondition condition);

static int debug=1;

#ifdef NEED_GETHOSTNAME_PROTO
int gethostname(char *name, int namelen);
#endif

Game_Finder *ggf; /* global game finder */

static void game_selected (GtkList *gtklist, gpointer func_data)
{
  /*Game_Finder *ggf = (Game_Finder *) func_data;*/

  if (gtklist->selection && gtklist->selection->data)
    {
      GtkListItem *item;

      item = gtklist->selection->data;

      if (GTK_IS_LIST_ITEM (item))
	{
	  /*int pos = (int) gtk_object_get_user_data (GTK_OBJECT (item));*/
	  int pos = gtk_list_child_position (gtklist, GTK_WIDGET (item));

	  if (debug)
	    printf ("in game_selected: pos=%d\n", pos);

	  [ggf list_callback : pos];
	}
    }
}



@implementation Game_Finder : Object

- init_gf : (int) argc
	  : (char **) argv
	  : (Board_View *) set_bv
	  : (Board_View_PS *) set_bvps
	  : (char *) set_server_machine
	  : (int) set_server_port
	  : (int) set_listen_port
{
    [super init];

    bv = set_bv;
    bvps = set_bvps;
    server_machine = set_server_machine;
    server_port = set_server_port;
    listen_port = set_listen_port;
    lbpc = NULL;

    [bvps set_game_finder : (id *) self];

    games = [[Vector alloc] init];

    search_machine_str = "Search Machine";
    ask_metaserver_str = "Ask MetaServer";
    new_game_str = "Start New Game";
    other_color_str = "Request Control of Another Color";
    kill_server_str = "Ask Server to Exit";
    quit_str = "Quit";


    shell = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_name (shell, "Game Finder");

    game_list = gtk_list_new ();

    gtk_signal_connect (GTK_OBJECT (game_list), "select_child",
			(GtkSignalFunc) game_selected,
			(gpointer) self);


    vbox=gtk_vbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER(vbox), game_list);
    gtk_container_add (GTK_CONTAINER(shell), vbox);
    gtk_widget_show (game_list);
    gtk_widget_show (vbox);
    gtk_widget_show (shell);

    current_game = NULL;

    [self search_machine];

    ggf = self;
    return self;
}


- (void) forget_game_list
{
  gtk_list_clear_items (GTK_LIST (game_list),
			0, g_list_length (GTK_LIST (game_list)->children));

  while ([games size] > 0)
    delete_game_info ((game_info *) [games delete : 0]);

}


- (void) fill_game_list
{
  int i;
  GList *items = NULL;

  gtk_list_clear_items (GTK_LIST (game_list),
			0, g_list_length (GTK_LIST (game_list)->children));

  for (i=0; i<[games size]; i++)
    {
      GtkWidget *label;

      game_info *gi = (game_info *) [games get : i];
      if (gi == NULL) continue;

      label = gtk_list_item_new_with_label ([self game_info_to_string : gi]);
      gtk_object_set_user_data (GTK_OBJECT (label), (gpointer) i);

      gtk_widget_show (label);
      items = g_list_prepend (items, label);
    }

  gtk_list_append_items (GTK_LIST (game_list), items);
}



- (int) find_index_of_game : (game_info *) find_gi
{
  int i;

  for (i=0; i<[games size]; i++)
    {
      game_info *gi = (game_info *) [games get : i];
      if (gi != NULL)
	if ((strcmp (gi->ip, find_gi->ip) == 0) &&
	    (gi->port == find_gi->port))
	  {
	    if (debug)
	      printf ("find_index_of_game found game at index %d\n", i);
	    return i;
	  }
    }
  
  return -1;
}



- (void) append_to_game_list : (game_info *) gi
{
  char *xmstr = [self game_info_to_string : gi];
  int i;

  if (debug)
    printf ("game_info: %s@%s.%s:%d, %d players\n",
	    gi->uname,
	    gi->host,
	    gi->domain,
	    gi->port,
	    gi->num_players);

  i = [self find_index_of_game : gi];
  if (i < 0)
    {
      GtkWidget *label;
      GList *items;

      [games add : (unsigned char *) gi];

      label = gtk_list_item_new_with_label (xmstr);
      gtk_object_set_user_data (GTK_OBJECT (label), xmstr);
      gtk_widget_show (label);
      items = g_list_prepend (NULL, label);

      gtk_list_append_items (GTK_LIST (game_list), items);
    }
  else
    {
      [games set : i : (unsigned char *) gi];
      [self fill_game_list];
    }
}


- (char *) game_info_to_string : (game_info *) gi
{
  /* xxx@yyy.zzz, %d players */
  char *ts = (char *) malloc (strlen( gi->uname ) +
			      strlen( gi->host ) +
			      strlen( gi->domain ) +
			      14);
  char n[ 2 ] = {'\0', '\0'};
  
  strcpy (ts, gi->uname);
  strcat (ts, "@");
  strcat (ts, gi->host);
  if (strlen (gi->domain) > 0)
    {
      strcat (ts, ".");
      strcat (ts, gi->domain);
    }
  strcat (ts, ", ");
  n[ 0 ] = gi->num_players + '0';
  strcat (ts, n);
  strcat (ts, " players");

  return ts;
}



- (void) delete_self
{
  /*XtPopdown (shell);*/
}


- (void) connect_to_game : (char *) host
			 : (char *) domain
			 : (char *) ip
			 : (int) port
{
  char hostname[ 256 ];
  char *remote_host;
  int rhl; /* remote host length */

  rhl = strlen (host) + strlen (domain) + 2;
  remote_host = (char *) malloc (rhl);
  strcpy (remote_host, host);
  if (strlen (domain) > 0)
    {
      strcat (remote_host, ".");
      strcat (remote_host, domain);
    }

  if (lbpc != NULL)
    {
      /* need to close connection to current server */
      gdk_input_remove (lbpc_XtInputId);
      [lbpc close_channel];
      [lbpc free];
      lbpc = NULL;
      [bv set_colors : C_neutral];
    }


  if (gethostname (hostname, 256) < 0)
    {
      perror ("gethostname");
      fprintf (stderr, "gethostname failed, trying with \"localhost\"\n");
      strcpy (hostname, "localhost");
    }

  lbpc = [[Logical_Board_PC alloc] init
				   : bvps
				   : ip
				   : port];

  if (lbpc == NULL)
    {
      fprintf (stderr, "couldn't reach server: %s:%d (%s)\n",
	       remote_host, port, ip);
      free (remote_host);
      return;
    }

  if  (strcmp (server_machine, "localhost") == 0)
    strcpy (hostname, "localhost");

  [lbpc add_board_view : hostname : listen_port];
  [bv set_logical_board : (id *) lbpc];

  [lbpc request_color];

  /* data comes back on that channel as well, so... */
  gdk_input_add ([bvps get_stream_file_d : [lbpc get_ch]],
		 GDK_INPUT_READ,
		 call_world_think, NULL);

  free (remote_host);
}


- (void) start_server
{
  Logical_Board *lb;
  Logical_Board_PS *lbps = NULL;


  if (debug)
    printf ("::releaseing X display...\n");

  /* let go of the X windows... */
  /* FIX ME -- how do I get X Display * from gdk? */
  /*
  close (ConnectionNumber (XtDisplay (shell)));
  close (ConnectionNumber ([bv getDisplay]));
  */

  /* let go of the controlling terminal... */
  if (!debug)
    close_controlling_terminal ();

  if (debug)
    printf ("::init logical board...\n");

  lb = [[Logical_Board alloc] init_logical_board];
  [lb load_file : STARTUP_PCS];


  if (debug)
    printf ("::init logical board ps\n");

  while (lbps == NULL)
    {
      lbps = [[Logical_Board_PS alloc] init_lbps : server_port : lb];
      if (lbps == NULL) server_port ++;
    }


  if (debug)
    printf ("::looping...\n");


  while (1)
    {
      [lbps catch : 20L : 0L];
      [lbps tell_details_to_ms]; /* every 20 seconds, so
				    the ms doesn't decide
				    that this server is dead */
    }
}


- (void) new_game
{
  int count=0;
  int current_games = 0;

  if (games != NULL)
    current_games = [games size];

  if (debug)
    printf ("in new_game\n");

  switch (fork ())
    {
    case -1:
      perror ("fork");
      exit (1);
    case 0:
      if (debug)
	printf ("::fork done... starting server.\n");
      [self start_server];
    default:
      break;
    }

  do
    {
      sleep (1);
      [self search_machine];
      count ++;
      if (count == 10) break; /* no server???! */
    }
  while ([games size] == current_games);
}


- (void) search_machine
{
  char *value = "localhost";

  if (debug)
    printf ("in tf_callback: %s\n", value);

  while ([games size] > 0)
    delete_game_info ((game_info *) [games delete : 0]);
    
  games = [bvps search_machine_for_servers : value : server_port];

  [self fill_game_list];
}


#if 0
- (void) tf_callback : (Widget) w : (XtPointer) call_data
{
  [self search_machine];
}
#endif


- (void) list_callback : (int) pos
{
  game_info *gi;

  if (debug)
    printf ("in list callback... %d\n", pos);

  if (pos < 0 || pos >= g_list_length (GTK_LIST (game_list)->children))
    return;

  /*
  if (games != NULL)
    {
      printf ("user='%s', ", games->gis[ pos ]->uname);
      printf ("machine='%s', ", games->gis[ pos ]->host);
      printf ("port=%d\n", games->gis[ pos ]->port);
    }
    */

  gi = (game_info *) [games get : pos];

  /* if we allow them to reselect the current game, the server
     will loose its connection for a moment, and might exit */
  if (current_game == gi)
    return;

  current_game = gi;

  [self connect_to_game
	: gi->host
	: gi->domain
	: gi->ip
	: gi->port];
}

@end
