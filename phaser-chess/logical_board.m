
#include <stdlib.h>
#include "logical_board.h"

static int debug=0;


char save_chars[ 14 ] =
{
    'b', 'f', 'p', 'o', 'm',
    'l', 's', 'n', 'k', 't',
    'x', 'g', 'c', 'v'
};


typedef enum
{
    Rct, /* cant */
    Ryd, /* stompy dies */
    Rrd, /* stomper_dies */
    Ryj, /* stompy_jumps */
    Rrj, /* stomper_jumps */
    Rsw  /* swap */
} Stomp_Result;



/* sr[ A ][ B ]  for A stomps onto B */

Stomp_Result stomp_result[14][14] =
{
 {Rsw,Rct,Rct,Rct,Rct,Rct,Rct,Rct,Rct,Rct,Rct,Rct,Rct,Rct}, //0 (b)lank
 {Rsw,Rct,Rct,Rct,Rct,Rct,Rct,Rct,Rct,Rct,Rct,Rrj,Rrj,Rrd}, //1 (f)reezer
 {Rsw,Ryd,Ryd,Ryd,Ryd,Ryd,Ryd,Ryd,Ryd,Ryd,Ryd,Rrj,Rrj,Rrd}, //2 (p)art m. stmp
 {Rsw,Rct,Rct,Rct,Rct,Rct,Rct,Rct,Rct,Rct,Rct,Rrj,Rrj,Rrd}, //3 (o)ne-way
 {Rsw,Rct,Rct,Rct,Rct,Rct,Rct,Rct,Rct,Rct,Rct,Rrj,Rrj,Rrd}, //4 (m)irror
 {Rsw,Rct,Rct,Rct,Rct,Rct,Rct,Rct,Rct,Rct,Rct,Rrj,Rrj,Rrd}, //5 (l)aser
 {Rsw,Rct,Rct,Rct,Rct,Rct,Rct,Rct,Rct,Rct,Rct,Rrj,Rrj,Rrd}, //6 (s)splitter
 {Rsw,Rct,Rct,Rct,Rct,Rct,Rct,Rct,Rct,Rct,Rct,Rrj,Rrj,Rrd}, //7 bomb (n)
 {Rsw,Ryd,Ryd,Ryd,Ryd,Ryd,Ryd,Ryd,Ryd,Ryd,Ryd,Rrj,Rrj,Rrd}, //8 (k)ing
 {Rsw,Ryj,Ryj,Ryj,Ryj,Ryj,Ryj,Ryj,Ryj,Ryj,Ryj,Rrj,Rrj,Rrd}, //9 (t)eleporter
 {Rsw,Ryd,Ryd,Ryd,Ryd,Ryd,Ryd,Ryd,Ryd,Ryd,Ryd,Rrj,Rrj,Rrd}, //10 m. stomper (x)
 {Rsw,Rct,Rct,Rct,Rct,Rct,Rct,Rct,Rct,Rct,Rct,Rct,Rrj,Rrd}, //11 tele(g)ate
 {Rsw,Rrd,Rrd,Rrd,Rrd,Rrd,Rrd,Rrd,Rrd,Rrd,Rrd,Rrd,Rrj,Rrd}, //12 (c)enterpit
 {Rsw,Rrd,Rrd,Rrd,Rrd,Rrd,Rrd,Rrd,Rrd,Rrd,Rrd,Rrd,Rrj,Rrd}  //13 pit (v)
 /*b   f   p   o   m   l   s   n   k   t   x   g   c   v*/
};



typedef enum
{
    Lnone, /* passes strait through */
    Ldie,  /* piece dies */
    Lbdie, /* piece dies like a bomb */
    L180,  /* beam is reflected at 180 degrees */
    Lsplt, /* beam is reflected to the left and the right */
    L90lf, /* beam is turned 90 degrees to the left */
    L90rt, /* beam is turned 90 degrees to the right */
    Lrndm /* beam is reflected in a random direction */
} Laser_Results;


/* ((direction of laser + 4) % 8 - direction of piece + 8) % 8 */ /* ??? */

Laser_Results laser_results[ 14 ][ 8 ] =
{
    {Lnone,Lnone,Lnone,Lnone,Lnone,Lnone,Lnone,Lnone},//  0 (blank) (b)
    {Ldie, Ldie, L180, Ldie, L180, Ldie, L180, Ldie },//  1 (freezer) (f)
    {L180, L180, Ldie, Ldie, Ldie, Ldie, Ldie, L180 },//  2 (p. mir. stomp) (p)
    {L180, Ldie, Ldie, Ldie, Lnone,Ldie, Ldie, Ldie },//  3 (one-way) (o)
    {L180, L90rt,Lnone,Ldie, Ldie, Ldie, Lnone,L90lf},//  4 (mirror) (m)
    {Ldie, Ldie, Ldie, Ldie, Ldie, Ldie, Ldie, Ldie },//  5 (laser) (l)
    {Lsplt,Lnone,L90rt,Lnone,Ldie, Lnone,L90lf,Lnone},//  6 (splitter) (s)
    {Lbdie,Ldie, Lbdie,Ldie, Lbdie,Ldie, Lbdie,Ldie },//  7 (bomb) (n)
    {Ldie, Ldie, Ldie, Ldie, Ldie, Ldie, Ldie, Ldie },//  8 (king) (k)
    {Lrndm,Lrndm,Lrndm,Lrndm,Lrndm,Lrndm,Lrndm,Lrndm},//  9 (teleporter) (t)
    {L180, L180, L180, L180, L180, L180, L180, L180 },// 10 (mirr. stomper) (x)
    {Ldie, Ldie, Ldie, Ldie, Ldie, Ldie, Ldie, Ldie },// 11 (telegate) (g)
    {Lrndm,Lrndm,Lrndm,Lrndm,Lrndm,Lrndm,Lrndm,Lrndm},// 12 (centerpit) (c)
    {Lnone,Lnone,Lnone,Lnone,Lnone,Lnone,Lnone,Lnone} // 13 (pit) (v)
};


int idx_to_bin[] = {1, 2, 4, 8, 16, 32, 64, 128};

int save_char_to_int (char sc)
{
    int lop;

    sc = tolower (sc);

    for (lop=0; lop<14; lop++)
    {
	if (sc == save_chars[ lop ])
	    return lop;
    }

    fprintf (stderr, "invalid save char: '%c'\n", sc);
    return 0;
}


int save_char_to_color (char sc)
{
  if (toupper (sc) == sc) return C_green;
  return C_red;
}


char int_to_save_char (int i)
{
    if (i<0 || i>= 14)
    {
	fprintf (stderr, "invalid save int: %d\n", i);
    }

    return save_chars[ i ];
}



int int_abs (int i)
{
    if (i<0)
	return -i;

    return i;
}


@implementation Logical_Board : Object

- init_logical_board
{
    int x, y;

    [super init];

    for (y=0; y<11; y++)
    {
	for (x=0; x<15; x++)
	{
	    board[ x ][ y ] = 0;
	    color[ x ][ y ] = 0;
	    direction[ x ][ y ] = 0;
	    status[ x ][ y ] = 0;
	}
    }


    csi = csi_no_selection;

    bvs = NULL;
    max_bvs = 0;

    red_bv = NULL;
    green_bv = NULL;

    moves = 0;
    whos_turn = C_green;

    [self update_moves_left];
    /*[self check_for_kings];*/

    coord_list = [[Coord_List alloc] init_coord_list];

    return self;
}


- (int) number_of_players
{
  int num_players = 0;
  if (red_bv != NULL) num_players ++;
  if (green_bv != NULL) num_players ++;

  return num_players;
}

- (void) update_pos : (int) x : (int) y
{
    int board_number;
    int bdir;

    if (board[ x ][ y ] == 0)
    {
	for (board_number = 0; board_number < max_bvs; board_number++)
	    if (bvs[ board_number ] != NULL)
	      [bvs[ board_number ] clear_square : x : y];
    }
    else
    {
	for (board_number = 0; board_number < max_bvs; board_number++)
	  if (bvs[ board_number ] != NULL)
	    [bvs[ board_number ] place_piece : x : y
		                             : board[ x ][ y ]
		                             : direction[ x ][ y ]
		                             : color[ x ][ y ]];
    }


    for (bdir = 0; bdir < 8; bdir ++)
    {
	if (idx_to_bin[ bdir ] & laser_paths_in[ x ][ y ])
	{
	    for (board_number = 0; board_number < max_bvs; board_number++)
		if (bvs[ board_number ] != NULL)
		    [bvs[ board_number ] place_beam : x : y : (bdir+4)%8];
	}
	if (idx_to_bin[ bdir ] & laser_paths_out[ x ][ y ])
	{
	    for (board_number = 0; board_number < max_bvs; board_number++)
		if (bvs[ board_number ] != NULL)
		    [bvs[ board_number ] place_beam : x : y : bdir];
	}
    }
}


- (void) add_board_view : (Board_View *) set_bv
{
    int add_slot;
    int x, y;

    for (add_slot=0; add_slot<max_bvs; add_slot++)
	if (bvs[ add_slot ] == set_bv)
	    return;

    for (add_slot=0; add_slot<max_bvs; add_slot++)
	if (bvs[ add_slot ] == NULL)
	    break;

    if (add_slot == max_bvs)
    {
	int lop;

	bvs = (Board_View **) realloc (bvs,
				   sizeof (Board_View *) * (max_bvs+1) * 2);

	for (lop=max_bvs; lop < (max_bvs+1)*2; lop++)
	    bvs[ lop ] = NULL;

	max_bvs ++;
	max_bvs *= 2;
    }

    bvs[ add_slot ] = set_bv;

    /* [bvs[ add_slot ] set_logical_board : (id *) self]; */

    for (y=0; y<11; y++)
      for (x=0; x<15; x++)
	[bvs[ add_slot ] place_piece : x : y
		                     : board[ x ][ y ]
		                     : direction[ x ][ y ]
		                     : color[ x ][ y ]];

    [self update_moves_left];
    /*[self check_for_kings];*/
}



- (void) delete_board_view : (Board_View *) del_bv
{
  int del_slot;

  if (red_bv == del_bv) red_bv = NULL;
  if (green_bv == del_bv) green_bv = NULL;

  for (del_slot=0; del_slot<max_bvs; del_slot++)
    if (bvs[ del_slot ] == del_bv)
      {
	if (debug)
	  printf ("logical board deleting view #%d\n", del_slot); 
	bvs[ del_slot ] = NULL;
      }
}



- (void) load_file : (char *) file_name
{
    FILE *fp;
    int x, y;
    int p_type;
    int p_dir;
    int p_status;

    fp = fopen (file_name, "r");

    if (fp == NULL)
    {
      char *p;
      perror ("fopen");
      fprintf (stderr, "cannot open file '%s' for reading.\n", file_name);

      /* move backwards looking for the last / */
      for (p = file_name + strlen (file_name); *p != '/'; p --)
	if (p == file_name) break;

      fp = fopen (p+1, "r");
      if (fp == NULL)
	{
	  fprintf (stderr, "cannot open file '%s' for reading.\n", p+1);
	  return;
	}

      fprintf (stderr, "using '%s' instead.\n", p+1);
    }

    for (y=0; y<11; y++)
    {
	for (x=0; x<15; x++)
	{
	    /* p_type = save_char_to_int (fgetc (fp) - '0'); */
	    p_type = fgetc (fp);
	    p_dir = fgetc (fp) - '0';
	    p_status = fgetc (fp) - '0';

	    board[ x ][ y ] = save_char_to_int (p_type);
	    color[ x ][ y ] = save_char_to_color (p_type);
	    direction[ x ][ y ] = p_dir;
	    status[ x ][ y ] = p_status;

	    [self update_pos : x : y];

	    if (fgetc (fp) != ' ')
	    {
		fprintf (stderr, "load file error, didn't get a space.\n");
	    }
	}

	if (fgetc (fp) != '\n')
	{
	    fprintf (stderr, "load file error, didn't get a newline...\n");
	}
    }

    fclose (fp);
}



- (int) find_path : (int) x1 : (int) y1 : (int) x2 : (int) y2 : (int) mvs
{
    int xd=0, yd=0, ret;

    if (mvs > 3) return -1;
    if (x1 < x2) xd = 1;
    if (x1 > x2) xd = -1;
    if (y1 < y2) yd = 1;
    if (y1 > y2) yd = -1;


    if (xd != 0)
    {
	if (x1+xd == x2 && y1 == y2) return mvs+1;
	if (board[ x1+xd ][ y1 ] == 0)
	{
	    ret = [self find_path : x1+xd : y1 : x2 : y2 : mvs+1];
	    if (ret > 0) return ret;
	}
    }

    if (yd != 0)
    {
	if (x1 == x2 && y1+yd == y2) return mvs+1;
	if (board[ x1 ][ y1+yd ] == 0)
	{
	    ret = [self find_path : x1 : y1+yd : x2 : y2 : mvs+1];
	    if (ret > 0) return ret;
	}
    }

    return -1;
}


- (void) bring_pit : (int) x : (int) y
{
    if (board[ x ][ y ] == 13)
	return;

    if (board[ x ][ y ] != 0)
    {
      if (debug)
	printf ("death at (%d, %d)\n", x, y);
      color[ x ][ y ] |= D_lazed;
      [self update_pos : x : y];
      sleep (1); /* ikky */
    }

    board[ x ][ y ] = 13;
    color[ x ][ y ] = C_neutral;
    direction[ x ][ y ] = 0;
    status[ x ][ y ] = 0;

    [self update_pos : x : y];
}


- (void) send_pit_away : (int) x : (int) y
{
    if (board[ x ][ y ] != 13)
	return;

    board[ x ][ y ] = 0;
    color[ x ][ y ] = C_neutral;
    direction[ x ][ y ] = 0;
    status[ x ][ y ] = 0;

    [self update_pos : x : y];
}


- (void) switch_players_turn
{
    int x, y;

    /* other's turn */

    switch (whos_turn)
    {
    case C_neutral:
    case C_both:
      fprintf (stderr, "whos turn is bunk.\n");
      break;
    case C_red:
      /* green's turn */
      whos_turn = C_green;
      break;
    case C_green:
      /* red's turn */
      whos_turn = C_red;
      break;
    }

    [self clear_status_board];

    moves = 0;

    /* unthaw pieces... */

    for (x=0; x<15; x++)
    {
        for (y=0; y<11; y++)
	{
	    if ((color[ x ][ y ] & D_frozen) != 0)
	    {
		if (random () % 10 > 8)
		{
		    color[ x ][ y ] &= ~D_frozen;
		    [self update_pos : x : y];
		}
	    }
	}
    }


    if (random () & 4)
    {
	/* pits will be there this turn */

	[self bring_pit : 7 : 1];
	[self bring_pit : 7 : 3];
	[self bring_pit : 7 : 7];
	[self bring_pit : 7 : 9];
    }
    else
    {
	/* pits will not be there this turn */

	[self send_pit_away : 7 : 1];
	[self send_pit_away : 7 : 3];
	[self send_pit_away : 7 : 7];
	[self send_pit_away : 7 : 9];
    }
}


- (char *) fire : (Player_Color) owners_color
{
    int x, y;

    if (debug)
      printf ("start of fire: csi = %d\n", csi);

    laser_paths_beam_type = BT_none;

    if (owners_color != whos_turn) /* must be this sided turn */
    {
	return "wrong owner color to fire this.";
    }

    if (moves == 3) /* must have enough moves left this turn */
    {
	return "not enough moves to fire this.";
    }

    if (csi != csi_piece_selected) /* must have a piece selected */
    {
	return "no piece selected.";
    }

    if (board[ sel_x ][ sel_y ] == 5)  /* must be a freezer or a laser */
    {
	laser_paths_beam_type = BT_laser;
    }
    else if (board[ sel_x ][ sel_y ] == 1)
    {
	laser_paths_beam_type = BT_freezer;
    }
    else
    {
	return "must be a laser or a freezer to fire.";
    }

    if (status[ sel_x ][ sel_y ] == 1)
    {
	return "Can't fire the same piece twice in a turn.";
    }

    status[ sel_x ][ sel_y ] = 1;



    [self clear_laser_board];

    laser_paths_out[ sel_x ][ sel_y ] =
      idx_to_bin[ (int) direction[ sel_x ][ sel_y ] ];

    x = [self x_dir_add : sel_x : direction[ sel_x ][ sel_y ] ];
    y = [self y_dir_add : sel_y : direction[ sel_x ][ sel_y ] ];

    [self process_laser : x : y : direction[ sel_x ][ sel_y ]];

    if (debug)
      printf ("end of fire: csi = %d\n", csi);

    moves ++;

    return NULL;
}


- (void) unfire
{
    int board_number;
    int x, y;

    if (laser_paths_beam_type == BT_none)
    {
      /* cannot unfire, if there was no fire */
      return;
    }

    if (csi != csi_piece_selected)
    {
	/* cannot unfire if no piece was selected */
	return;
    }

    if (board[ sel_x ][ sel_y ] != 1 && board[ sel_x ][ sel_y ] != 5)
    {
      /* cannot unfire, if the piece selected was
	 not a freezer or a laser. */
	return;
    }

    for (board_number = 0; board_number < max_bvs; board_number++)
    {
	if (bvs[ board_number ] != NULL)
	{
	    [bvs[ board_number ] remove_beam];
	}
    }

    [self clear_laser_board];
    color[ sel_x ][ sel_y ] &= (~D_highlighted);
    [self update_pos : sel_x : sel_y];

    csi = csi_no_selection;

    while ([coord_list get_last_and_delete : &x : &y])
    {
	switch (laser_paths_beam_type)
	{
	case BT_laser:
	    board[ x ][ y ] = 0;
	    color[ x ][ y ] = C_neutral;
	    direction[ x ][ y ] = 0;
	    status[ x ][ y ] = 0;
	    break;
	case BT_freezer:
	case BT_none:
	    break;
	}
	
	[self update_pos : x : y];
    }

    if (moves == 3)
	[self switch_players_turn];

    [self update_moves_left];
    [self check_for_kings];
}


- (char *) pass : (Player_Color) owners_color
{
    if (owners_color != whos_turn)
    {
	return "cannot pass, wrong player color.";
    }

    if (csi != csi_no_selection)
    {
	color[ sel_x ][ sel_y ] &= (~D_highlighted);
	[self update_pos : sel_x : sel_y];
	csi = csi_no_selection;
    }

    [self switch_players_turn];
    [self update_moves_left];
    [self check_for_kings];

    return NULL;
}


- (char *) mouse_button_press : (int) x : (int) y : (int) n
			      : (Player_Color) owners_color
{
  char *message = NULL;

  if (debug)
    printf ("logical board -- mouse button press %d, %d, %d, %d\n",
	    x, y, n, owners_color);

  if (owners_color != whos_turn && owners_color != C_both)
    {
      return "cannot act on mouse click: wrong player color.";
    }


  switch (csi)
    {
    case csi_no_selection: /***********************************************/
      if (n != 0)
	break;

      if (board[ x ][ y ] < 1 || board[ x ][ y ] > 11)
	{
	  message = "cannot select piece: not a player piece.";
	  break;
	}

      if ((color[ x ][ y ] & D_color) != whos_turn)
	{
	  message = "cannot select piece: wrong player color.";
	  if (debug)
	    printf ("whos_turn = %d, owners_color = %d, "
		    "color = %d\n",
		    whos_turn, owners_color,
		    color[ x ][ y ]);
	  break;
	}

      if ((color[ x ][ y ] & D_frozen) != 0)
	{
	  message = "cannot select piece: piece is frozen.";
	  break;
	}

      color[ x ][ y ] |= D_highlighted;
      [self update_pos : x : y];
      sel_x = x; sel_y = y; sel_r = direction[ x ][ y ];

      csi = csi_piece_selected;
      break;
    case csi_piece_selected: /*********************************************/
      if (n == 0)
	{
	  if (x == sel_x && y == sel_y)
	    {
	      color[ x ][ y ] &= (~D_highlighted);
	      [self update_pos : x : y];
	      csi = csi_no_selection;
	    }
	  else
	    {
	      int moves_required = [self find_path : sel_x : sel_y
					 : x : y : 0];
	      if (moves_required > 0 && moves + moves_required <= 3)
		{
		  if ([self stomp : sel_x : sel_y : x : y : &message])
		    moves += moves_required;
		}
	    }
	}

      if (n == 1 || n == 2)
	{
	  int d = ((n == 1) ? -1 : 1);

	  if ([self dirs_look_the_same :
		      board[ sel_x ][ sel_y ] :
		      direction[ sel_x ][ sel_y ] :
		      sel_r])
	    {
	      moves++;
	    }

	  if (direction[ sel_x ][ sel_y ] + d < 0)
	    direction[ sel_x ][ sel_y ] += 8;

	  direction[ sel_x ][ sel_y ] += d;

	  while (direction[ sel_x ][ sel_y ] >= 8)
	    direction[ sel_x ][ sel_y ] -= 8;

	  [self update_pos : sel_x : sel_y];

	  if ([self dirs_look_the_same
		    : board[ sel_x ][ sel_y ]
		    : direction[ sel_x ][ sel_y ]
		    : sel_r])
	    moves--;
	}

      break;
    }

  if (debug)
    printf ("              -- after switch\n");
	    

  if (moves == 3 && csi == csi_no_selection)
    {
      if (debug)
	printf ("              -- switching turns\n");

      [self switch_players_turn];
    }

  if (debug)
    printf ("              -- update moves left\n");

  [self update_moves_left];

  if (debug)
    printf ("              -- check for kings\n");

  [self check_for_kings];

  if (debug)
    printf ("              -- returning\n");

  return message;
}



- (void) dump_status_board
{
    int x, y;


    for (y=0; y<11; y++)
    {
	for (x=0; x<15; x++)
	{
	    printf ("%d ", status[ x ][ y ]);
	}
	printf ("\n");
    }
}


- (void) check_for_kings
{
  int x, y;
  int found_red=0, found_green=0;

  for (y=0; y<11; y++)
    {
      for (x=0; x<15; x++)
	{
	  if (board[ x ][ y ] == 8 && (color[ x ][ y ] & D_color) == C_red)
	    found_red = 1;
	  if (board[ x ][ y ] == 8 && (color[ x ][ y ] & D_color) == C_green)
	    found_green = 1;
	}
    }

  if (found_red == 0 && found_green == 0)
    [self message_to_all : "you both lose."];
  else if (found_red == 0)
    [self message_to_all : "green wins."];
  else if (found_green == 0)
    [self message_to_all : "red wins."];
}


- (void) message_to_all : (char *) message
{
  int board_number;

  for (board_number = 0; board_number < max_bvs; board_number++)
    if (bvs[ board_number ] != NULL)
      [bvs[ board_number ] send_message : message];
}


- (void) update_moves_left
{
    int red_moves=0, green_moves=0;
    int board_number;

    switch (whos_turn)
      {
      case C_neutral:
      case C_both:
	fprintf (stderr, "whos turn is bunk.\n");
	break;
      case C_red:
	red_moves = 3 - moves;
	break;
      case C_green:
	green_moves = 3 - moves;
	break;
    }

    for (board_number = 0; board_number < max_bvs; board_number++)
      if (bvs[ board_number ] != NULL)
	{
	  [bvs[ board_number ] update_moves
	      : (int) red_moves
	      : (int) green_moves];
	}
}



- (int) stomp : (int) x1 : (int) y1 : (int) x2 : (int) y2
	      : (char **) message
{
    if (x2 < 0 || x2 >= 15 || y2 < 0 || y2 > 10)
	return 0;


    switch (stomp_result
	    [ (int)(board[ x1 ][ y1 ]) ]
	    [ (int)(board[ x2 ][ y2 ]) ])
    {
    case Rct: /* cant */
      (*message) = "Hey! you can't do that!";
      return 0;
    case Ryd: /* stompy dies */
	if (status[ x1 ][ y1 ] == 0)
	{
	    status[ x1 ][ y1 ] = 1;
	    [self stomp_stompy_dies : x1 : y1 : x2 : y2 : message];
	}
	else
	{
	  (*message) = "you can't stomp with the same piece twice in a turn.";
	  return 0;
	}
	break;
    case Rrd: /* stomper_dies */
	[self stomp_stomper_dies : x1 : y1 : x2 : y2 : message];
	break;
    case Ryj: /* stompy_jumps */
	if (status[ x1 ][ y1 ] == 0)
	{
	    status[ x1 ][ y1 ] = 1;
	    [self stomp_stompy_jumps : x1 : y1 : x2 : y2 : message];
	}
	else
	{
	  (*message) = "you can't stomp with the same piece twice in a turn.";
	  return 0;
	}
	break;
    case Rrj: /* stomper_jumps */
	[self stomp_stomper_jumps : x1 : y1 : x2 : y2 : message];
	break;
    case Rsw:  /* swap */
	[self stomp_swap : x1 : y1 : x2 : y2 : message];
	break;
    }

    return 1;
}



- (void) stomp_swap : (int) x1 : (int) y1 : (int) x2 : (int) y2
		    : (char **) message
{
    int b = board[ x2 ][ y2 ];
    int c = color[ x2 ][ y2 ];
    int d = direction[ x2 ][ y2 ];
    int s = status[ x2 ][ y2 ];

    board[ x2 ][ y2 ] = board[ x1 ][ y1 ];
    color[ x2 ][ y2 ] = color[ x1 ][ y1 ] & (~D_highlighted);
    direction[ x2 ][ y2 ] = direction[ x1 ][ y1 ];
    status[ x2 ][ y2 ] = status[ x1 ][ y1 ];
    
    board[ x1 ][ y1 ] = b;
    color[ x1 ][ y1 ] = c;
    direction[ x1 ][ y1 ] = d;
    status[ x1 ][ y1 ] = s;
    
    [self update_pos : x1 : y1];
    [self update_pos : x2 : y2];
    
    csi = csi_no_selection;
}


- (void) stomp_stompy_dies : (int) x1 : (int) y1 : (int) x2 : (int) y2
			   : (char **) message
{
    board[ x2 ][ y2 ] = board[ x1 ][ y1 ];
    color[ x2 ][ y2 ] = color[ x1 ][ y1 ] & (~D_highlighted);
    direction[ x2 ][ y2 ] = direction[ x1 ][ y1 ];
    status[ x2 ][ y2 ] = status[ x1 ][ y1 ];
    
    board[ x1 ][ y1 ] = 0;
    color[ x1 ][ y1 ] = C_neutral;
    direction[ x1 ][ y1 ] = 0;
    status[ x1 ][ y1 ] = 0;
    
    [self update_pos : x1 : y1];
    [self update_pos : x2 : y2];
    
    csi = csi_no_selection;
}


- (void) stomp_stomper_dies : (int) x1 : (int) y1 : (int) x2 : (int) y2
			    : (char **) message
{
    board[ x1 ][ y1 ] = 0;
    color[ x1 ][ y1 ] = C_neutral;
    direction[ x1 ][ y1 ] = 0;
    status[ x1 ][ y1 ] = 0;
    
    [self update_pos : x1 : y1];
    
    csi = csi_no_selection;
}


- (void) stomp_stompy_jumps : (int) x1 : (int) y1 : (int) x2 : (int) y2
			    : (char **) message
{
    int nx, ny, nd;

    do
    {
	nx = random () % 15;
	ny = random () % 11;
	nd = random () % 8;
    } while (board[ nx ][ ny ] != 0 &&  /* blank */
	     board[ nx ][ ny ] != 12 && /* center pit */
	     board[ nx ][ ny ] != 13);   /* pit */

    direction[ x2 ][ y2 ] = nd;
    [self stomp : x2 : y2 : nx : ny : message];


    board[ x2 ][ y2 ] = board[ x1 ][ y1 ];
    color[ x2 ][ y2 ] = color[ x1 ][ y1 ] & (~D_highlighted);
    direction[ x2 ][ y2 ] = direction[ x1 ][ y1 ];
    status[ x2 ][ y2 ] = status[ x1 ][ y1 ];


    board[ x1 ][ y1 ] = 0;
    color[ x1 ][ y1 ] = C_neutral;
    direction[ x1 ][ y1 ] = 0;
    status[ x1 ][ y1 ] = 0;


    [self update_pos : x1 : y1];
    [self update_pos : x2 : y2];
    
    csi = csi_no_selection;
}



- (void) stomp_stomper_jumps : (int) x1 : (int) y1 : (int) x2 : (int) y2
			     : (char **) message
{
    int nx, ny;


    /* this should be a gate link etc. FINISH ME...*/

    do
    {
	nx = random () % 15;
	ny = random () % 11;
    } while (board[ nx ][ ny ] != 0 &&  /* blank */
	     board[ nx ][ ny ] != 12 && /* center pit */
	     board[ nx ][ ny ] != 13);   /* pit */

    [self stomp : x1 : y1 : nx : ny : message];

    csi = csi_no_selection;
}



- (void) key_press : (int) key
{
  if (debug)
    printf ("key '%c'\n", key);
}


- (void) clear_laser_board
{
    int x, y;

    for (x=0; x<15; x++)
    {
	for (y=0; y<11; y++)
	{
	    laser_paths_in[ x ][ y ] = 0;
	    laser_paths_out[ x ][ y ] = 0;
	}
    }
}


- (void) clear_status_board
{
    int x, y;

    for (x=0; x<15; x++)
    {
	for (y=0; y<11; y++)
	{
	    status[ x ][ y ] = 0;
	}
    }
}



- (int) x_dir_add : (int) x : (int) dir
{
    switch (dir)
    {
    case 0:
    case 4:
	return x;
    case 1:
    case 2:
    case 3:
	return x+1;
    case 5:
    case 6:
    case 7:
	return x-1;
    default:
	fprintf (stderr, "bad direction in dir_add: %d\n", dir);
	return 0;
    }
}


- (int) y_dir_add : (int) y : (int) dir
{
    switch (dir)
    {
    case 2:
    case 6:
	return y;
    case 3:
    case 4:
    case 5:
	return y+1;
    case 7:
    case 0:
    case 1:
	return y-1;
    default:
	fprintf (stderr, "bad direction in dir_add: %d\n", dir);
	return 0;
    }
}


- (void) beam_out : (int) x : (int) y : (int) ldir
{
    int board_number;

    laser_paths_out[ x ][ y ] |= idx_to_bin[ ldir ];

    for (board_number = 0; board_number < max_bvs; board_number++)
    {
	if (bvs[ board_number ] != NULL)
	{
	    [bvs[ board_number ] place_beam : x : y : (ldir)%8];
	}
    }
}


- (void) beam_in : (int) x : (int) y : (int) ldir
{
    int board_number;

    laser_paths_in[ x ][ y ] |= idx_to_bin[ ldir ];

    for (board_number = 0; board_number < max_bvs; board_number++)
    {
	if (bvs[ board_number ] != NULL)
	{
	    [bvs[ board_number ] place_beam : x : y : (ldir+4)%8];
	}
    }
}


- (void) beam_effects_piece : (int) x : (int) y
{
    switch (laser_paths_beam_type)
    {
    case BT_none: fprintf (stderr, "error?\n"); break;
    case BT_laser: color[ x ][ y ] |= D_lazed; break;
    case BT_freezer: color[ x ][ y ] |= D_frozen; break;
    }
    [self update_pos : x : y];
    [coord_list add : x : y];
}


- (void) process_laser : (int) x : (int) y : (int) ldir
{
    int l_res;

    for (;;)
    {
	if (x<0 || x>14 || y<0 || y>10) return;
	if (laser_paths_in[ x ][ y ] & idx_to_bin[ ldir ]) return;
	[self beam_in : x : y : ldir];

	l_res = laser_results[ (int) board[ x ][ y ] ]
	                     [ (direction[ x ][ y ] - (ldir+4)%8 + 16) % 8 ];

	if (color[ x ][ y ] & D_frozen) /* frozen pieces cannot reflect. */
	    l_res = Ldie;

	switch ( l_res )
	{
	case Lnone:
	    [self beam_out : x : y : ldir];

	    x = [self x_dir_add : x : ldir];
	    y = [self y_dir_add : y : ldir];
	    break;
	case Ldie:
	    [self beam_effects_piece : x : y];
	    return;
	case Lbdie:
	    {
		int xx, yy;
		int nei_dir;

		[self beam_effects_piece : x : y];

		for (nei_dir=0; nei_dir<8; nei_dir++)
		{
		    xx = [self x_dir_add : x : nei_dir];
		    yy = [self y_dir_add : y : nei_dir];

		    if (xx >= 0 && yy >= 0 && xx <= 14 && yy <= 10)
		    {
			[self beam_out : x : y : nei_dir];
			[self beam_in : xx : yy : nei_dir];

			if (board[ xx ][ yy ] > 0 && board[ xx ][ yy ] < 12)
			    [self beam_effects_piece : xx : yy];
		    }
		}
	    }
	    return;
	case L180:
	    ldir = (ldir+4) % 8;
	    [self beam_out : x : y : ldir];
	    x = [self x_dir_add : x : ldir];
	    y = [self y_dir_add : y : ldir];
	    break;
	case Lsplt:
	    [self beam_out : x : y : (ldir+2)%8];
	    [self process_laser
		  : [self x_dir_add : x : (ldir+2)%8]
		  : [self y_dir_add : y : (ldir+2)%8]
		  : (ldir+2)%8];

	    ldir = (ldir+6) % 8;
	    [self beam_out : x : y : ldir];
	    x = [self x_dir_add : x : ldir];
	    y = [self y_dir_add : y : ldir];
	    break;
	case L90lf:
	    ldir = (ldir+2) % 8;
	    [self beam_out : x : y : ldir];
	    x = [self x_dir_add : x : ldir];
	    y = [self y_dir_add : y : ldir];
	    break;
	case L90rt:
	    ldir = (ldir+6) % 8;
	    [self beam_out : x : y : ldir];
	    x = [self x_dir_add : x : ldir];
	    y = [self y_dir_add : y : ldir];
	    break;
	case Lrndm:
	    ldir = random () % 8;
	    [self beam_out : x : y : ldir];
	    x = [self x_dir_add : x : ldir];
	    y = [self y_dir_add : y : ldir];
	    break;
	}
    }
}


- (int) dirs_look_the_same : (int) piece : (int) dir1 : (int) dir2
{
    if (pn[ piece ][ dir1 ] == pn[ piece ][ dir2 ])
	return 1;

    return 0;
}


- (void) request_color : (Board_View *) bv
{
  if (red_bv == NULL)
    {
      if (debug)
	printf ("logical_board -- request_color -- giving red\n");

      red_bv = bv;
      [bv take_color : C_red];
      [self update_moves_left];
      return;
    }

  if (green_bv == NULL)
    {
      if (debug)
	printf ("logical_board -- request_color -- giving green\n");

      green_bv = bv;
      [bv take_color : C_green];
      [self update_moves_left];
      return;
    }

  if (debug)
    printf ("logical_board -- request_color -- giving neither\n");
}

@end
