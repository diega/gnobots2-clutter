/* (C) 2003/2004 Sjoerd Langkemper
 * (C) 1999-2003 Chris Rogers
 * gataxx.c -
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * For more details see the file COPYING.
 */


#include "config.h"
#include <gdk/gdkkeysyms.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include <sys/time.h>
#include <string.h>
#include <games-clock.h>
#include <gconf/gconf-client.h>
#include <games-gconf.h>
#include <libintl.h>
#include <libgnome/libgnome.h>

#include <gtkgridboard.h>
#include "properties.h"
#include "gataxx.h"
#include "menus.h"
#include "appbar.h"
#include "ai.h"

GtkGridBoard * gridboard;		/* current gridboard */
GtkWidget * window;             /* The apps main window. */
gint turn;		        /* current player */
position selection={0, 0, 0};	/* last selected position */
int timeout;			/* computer speed */

/* Command-line handling. */
static gint iturn = 1;
static gchar * state = NULL;
static const struct poptOption options[] = {
  {"state", 's', POPT_ARG_STRING, &state, 0, "Set the state of the board at start-up.", NULL},
  {"turn", 't', POPT_ARG_INT, &iturn, 0, "Set whose turn it is.", " 1 (Light) or 2 (Dark)"},
  {NULL, '\0', POPT_ARG_NONE, NULL, 0, NULL, NULL}
};

/* gettext i18n stuff */
static void settextdomain() {
	bindtextdomain(GETTEXT_PACKAGE, GNOMELOCALEDIR);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);
}

/* returns true if at least one move is possible for player /turn/ */
gboolean move_possible(GtkGridBoard * gridboard, int turn) {
	int x, y;
	gboolean mov;

	for (x=0; x<BWIDTH; x++) {
		for (y=0; y<BHEIGHT; y++) {
			mov=move_possible_to(gridboard, x, y, turn);
			if (mov) {
				return TRUE;
			}
		}
	}
	return FALSE;
}

/* returns true if a move is possible to (x, y) */
gboolean move_possible_to(GtkGridBoard * gridboard, int x, int y, int turn) {
	int piece, _x, _y;
	if (gtk_gridboard_get_piece(gridboard, x, y)!=EMPTY) 
	  return FALSE;
	for (_x=MAX(0, x-2); _x<MIN(BWIDTH, x+3); _x++) {
		for (_y=MAX(0, y-2); _y<MIN(BHEIGHT, y+3); _y++) {
			piece=gtk_gridboard_get_piece(gridboard, _x, _y);
			if (piece==turn) return TRUE;
		}
	}
	return FALSE;
}	

/* after a move is done, a timeout is set on this function */
gboolean computer_move_cb(gpointer turn) {
	move cm;

	/* In case the level gets changed between the timeout
	 * being set and it going off. */
	if (props_get_level (GPOINTER_TO_INT (turn)) < 1)
	  return FALSE;

	cm=computer_move(gridboard, GPOINTER_TO_INT(turn));
	do_move(cm);
	return FALSE;
}

/* gets called when the user clicks with the mouse */
void boxclicked_cb(GtkWidget * widget, int x, int y) {
	int dist=MAX(ABS(selection.x-x), ABS(selection.y-y));
	int piece=gtk_gridboard_get_piece(gridboard, x, y);
	move m={{selection.x, selection.y, selection.valid}, {x, y, TRUE}};

	if (!props_is_human(turn)) return;
	
	if ((selection.valid)&&(piece==EMPTY)&&(dist<=2)) {
		dist=MAX(ABS(selection.x-x), ABS(selection.y-y));
		do_move(m);
		selection.valid=FALSE;
	} else if (piece==turn) {
		do_select(x, y);
		selection.x=x;
		selection.y=y;
		selection.valid=TRUE;
	}
}

/* sets pieces, adds undo info, changes statusbar and changes turn */
void do_move(move m) {
	/* undo info */
	if (props_is_human(turn)) {
		gtk_gridboard_save_state(gridboard, GINT_TO_POINTER(turn));
	}
	if (gtk_gridboard_states_present(gridboard)) {
		menu_undo_set_sensitive(TRUE);
	} else {
		menu_undo_set_sensitive(FALSE);
	}
		
	
	gtk_gridboard_clear_selections(GTK_GRIDBOARD (gridboard));
	gridboard_move(gridboard, m);

	appbar_set_white(gtk_gridboard_count_pieces(GTK_GRIDBOARD (gridboard), WHITE));
	appbar_set_black(gtk_gridboard_count_pieces(GTK_GRIDBOARD (gridboard), BLACK));

	turn = turn == BLACK ? WHITE : BLACK;
	appbar_set_turn (turn);

	if (!move_possible(gridboard, turn)) {
		g_timeout_add(timeout, end_game_cb, GINT_TO_POINTER(turn));
		return;
	}

	if (!props_is_human(turn)) {
		g_timeout_add(timeout, computer_move_cb, 
                              GINT_TO_POINTER(turn));
	}

}

/* game over */
gboolean end_game_cb(gpointer data) {
	int wc=gtk_gridboard_count_pieces(gridboard, WHITE);
	int bc=gtk_gridboard_count_pieces(gridboard, BLACK);
	
	turn=EMPTY;

	if (wc>bc) {
		appbar_set_status(_("Light player wins!"));
	} else if (wc<bc) {
		appbar_set_status(_("Dark player wins!"));
	} else {
		appbar_set_status(_("The game was a draw."));
	}
	if (props_get_flip_final()) {
		flip_final(gridboard, wc, bc);
	}
	return FALSE; /* kill the timeout */
}

/* makes an overview of all the pieces on the board at the end of the game */
void flip_final(GtkGridBoard * gridboard, int wc, int bc) {
	int x, y, piece=EMPTY;
	int ec=BWIDTH*BHEIGHT-wc-bc;

	for (y=0; y<BWIDTH; y++) {
		for (x=0; x<BHEIGHT; x++) {
			if (wc) {
				piece=WHITE;
				wc--;
			} else if (ec) {
				piece=EMPTY;
				ec--;
			} else if (bc) {
				piece=BLACK;
				bc--;
			}
			gtk_gridboard_set_piece(gridboard, x, y, piece);
		}
	}
}

/* changes the pieces on the board */
void gridboard_move(GtkGridBoard * gridboard, move m) {
	int dist;
	int piece=gtk_gridboard_get_piece(gridboard, m.from.x, m.from.y);
	dist=MAX(ABS(m.from.x-m.to.x), ABS(m.from.y-m.to.y));
	if (dist==1) {
		gtk_gridboard_set_piece(gridboard, m.to.x, m.to.y, piece);
	} else if (dist==2) {
		gtk_gridboard_set_piece(gridboard, m.to.x, m.to.y, piece);
		gtk_gridboard_set_piece(gridboard, m.from.x, m.from.y,
				EMPTY);
	} else {
		return;
	}
	turn_pieces(gridboard, m.to.x, m.to.y);
}

/* turn all surrounding pieces */
void turn_pieces(GtkGridBoard * gridboard, int x, int y) {
	int me, _x, _y, piece;
	me=gtk_gridboard_get_piece(gridboard, x, y);
	for (_x=MAX(0, x-1); _x<MIN(BWIDTH, x+2); _x++) {
		for (_y=MAX(0, y-1); _y<MIN(BHEIGHT, y+2); _y++) {
			piece=gtk_gridboard_get_piece(gridboard, _x, _y);
			if ((piece!=EMPTY)&&(piece!=me)) {
				gtk_gridboard_set_piece(gridboard, _x, _y, me);
			}
		}
	}
}

/* selects (x, y) and all surrounding boxes */
void do_select(int x, int y) {
	int _x, _y;

	gtk_gridboard_clear_selections(gridboard);
	for (_x=MAX(0, x-2); _x<MIN(BWIDTH, x+3); _x++) {
		for (_y=MAX(0, y-2); _y<MIN(BHEIGHT, y+3); _y++) {
			if (gtk_gridboard_get_piece(gridboard, _x, _y)==EMPTY) {
				gtk_gridboard_set_selection(gridboard, 
					SELECTED_A, _x, _y);
			}
		}
	}
	gtk_gridboard_set_selection(gridboard, SELECTED_A, x, y);
}

/* Restore the state of the board from the command line. */
static void restore_state (void)
{
  int i,j;

  gtk_gridboard_clear (gridboard);

  if (!state)
    return;

  for (j=0; j<BHEIGHT; j++) {
    for (i=0; i<BWIDTH; i++) {
      if (*state == '\0')
	return;
      switch (*state) {
      case '1':
	gtk_gridboard_set_piece (gridboard, i, j, WHITE);
	break;
      case '2':
	gtk_gridboard_set_piece (gridboard, i, j, BLACK);
	break;
      }
      state++;
    }
  }
}

/* This handles both games loaded off the command line and proper new games. */
static void new_game(void)
{
        gboolean boardok = FALSE;

        if (state) {
	  restore_state ();
	  state = NULL;
	  boardok = (gtk_gridboard_count_pieces(gridboard, WHITE) > 0) && 
        	    (gtk_gridboard_count_pieces(gridboard, BLACK) > 0);
	} 
	if (!boardok) {
	  gtk_gridboard_clear(gridboard);

	  gtk_gridboard_set_piece(gridboard, 0, 0, WHITE);
	  gtk_gridboard_set_piece(gridboard, 0, 6, BLACK);
	  gtk_gridboard_set_piece(gridboard, 6, 0, BLACK);
	  gtk_gridboard_set_piece(gridboard, 6, 6, WHITE);
	}

	if (iturn != WHITE) {
	  turn = BLACK;
	  iturn = WHITE;
	} else {
	  turn = WHITE;
	}
	
	appbar_set_turn(turn);
	appbar_set_white(gtk_gridboard_count_pieces(gridboard, WHITE));
	appbar_set_black(gtk_gridboard_count_pieces(gridboard, BLACK));

	if (!props_is_human(turn)) {
		g_timeout_add(timeout, computer_move_cb,
                              GINT_TO_POINTER(turn));
	}
}

/* menu: Game->New game */
void new_game_cb(GtkWidget * widget, gpointer data)
{
        new_game ();
}


/* menu: Game->Undo move */
void undo_move_cb(GtkWidget * widget, gpointer data) {
	if (gtk_gridboard_states_present(gridboard)) {
		turn=GPOINTER_TO_INT(gtk_gridboard_revert_state(gridboard));
	}
	if (gtk_gridboard_states_present(gridboard)) {
		menu_undo_set_sensitive(TRUE);
	} else {
		menu_undo_set_sensitive(FALSE);
	}
	if (!props_is_human(turn)) {
		g_timeout_add(timeout, computer_move_cb, GINT_TO_POINTER(turn));
	}
	selection.valid = FALSE;
	appbar_set_turn (turn);
	appbar_set_white (gtk_gridboard_count_pieces(gridboard, WHITE));
	appbar_set_black (gtk_gridboard_count_pieces(gridboard, BLACK));
}

/* menu: Game->Quit */
gboolean quit_game_cb(GtkWidget * widget, gpointer data) {
        if (timeoutid)
		g_source_remove (timeoutid);
	gtk_main_quit();
	
	return TRUE;
}

/* menu: Settings->Preferences */
void properties_cb(GtkWidget * widget, gpointer data) {
	show_properties_dialog();
}

/* this gets called whenever some setting has changed */
void
apply_changes() {
	gtk_gridboard_set_animate(gridboard, props_get_animate());
	gtk_gridboard_set_show_grid(gridboard, props_get_show_grid());
	gtk_gridboard_set_tileset(gridboard, 
			get_tileset_path(props_get_tile_set()));
	
	if (props_get_quick_moves()) {
		timeout=DEF_TIMEOUT/2;
	} else {
		timeout=DEF_TIMEOUT;
	}
}

/* menu: Help->About */
void
about_cb(GtkWidget *widget, gpointer data)
{
  const gchar *authors[] = {"Chris Rogers", "Sjoerd Langkemper", 
			    N_("Based on code from Iagno by Ian Peters"), NULL};

  gtk_show_about_dialog (GTK_WINDOW (window),
			 "name", _("Ataxx"), 
			 "version", VERSION,
			 "copyright", "Copyright \xc2\xa9 1999-2003 Chris Rogers\n"
				      "Copyright \xc2\xa9 2004-2005 Sjoerd Langkemper",
			 "comments", _("A disk-flipping game where you attempt to dominate the board."),
			 "authors", authors,
			 "translator_credits", _("translator-credits"),
			 NULL);
}


/* the properties dialog wants to know the gconf client */
GConfClient * get_gconf_client() {
	static GConfClient * gconfclient=NULL;
	if (gconfclient==NULL) gconfclient=gconf_client_get_default();
	return gconfclient;
}

static gboolean save_state_cb (GnomeClient * client, gint phase, 
			       GnomeSaveStyle style, gboolean shutdown,
			       GnomeInteractStyle interactive, gboolean fast,
			       gpointer data)
{
  gchar * argv[3];
  gchar * state, * statestr;
  int i, j;

  argv[0] = "gataxx";  

  state = statestr = g_malloc0 (BWIDTH * BHEIGHT + 1);
  
  for (j=0; j<BHEIGHT; j++) {
    for (i=0; i<BWIDTH; i++) {
      switch (gtk_gridboard_get_piece (gridboard, i, j)) {
        case WHITE:
  	  *state = '1';
	  break;
        case BLACK:
	  *state = '2';
	  break;
        default:
	  *state = '0';
      }
      state++;
    }
  }

  argv[1] = g_strconcat ("--state=", statestr, NULL);


  if (turn == WHITE)
	argv[2] = "--turn=1";
  else
	argv[2] = "--turn=2";

  gnome_client_set_restart_command (client, 3, argv);
  
  g_free (statestr);
  g_free (argv[1]);

  return TRUE;
}

static void initgnomeclient(int argc, char ** argv) {
	GnomeClient * client;

	gnome_program_init("gataxx", VERSION,
			LIBGNOMEUI_MODULE,
			argc, argv,
			GNOME_PARAM_POPT_TABLE, options,
			GNOME_PARAM_APP_DATADIR, DATADIR, NULL);
	gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gataxx.png");
	client=gnome_master_client();

	g_signal_connect (G_OBJECT (client), "save_yourself",
                    G_CALLBACK (save_state_cb), argv[0]);
	g_signal_connect (G_OBJECT (client), "die",
                    G_CALLBACK (quit_game_cb), argv[0]);
}

static void create_window() {
	gchar * tileset;
	
	window=gnome_app_new("gataxx", "Ataxx");
	gtk_widget_realize(window);
	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
	g_signal_connect (G_OBJECT (window), "delete_event",
		G_CALLBACK (quit_game_cb), NULL);

	gnome_app_create_menus(GNOME_APP(window), mainmenu);
	menu_undo_set_sensitive(FALSE);

	gnome_app_set_statusbar(GNOME_APP(window), appbar_new());
	gnome_app_install_menu_hints(GNOME_APP(window), mainmenu); 

	props_init(GTK_WINDOW(window), "gataxx");
	tileset=props_get_tile_set();
	gridboard = GTK_GRIDBOARD (gtk_gridboard_new(BWIDTH, BHEIGHT, get_tileset_path(tileset)));
	gnome_app_set_contents(GNOME_APP(window), GTK_WIDGET (gridboard));
	gtk_widget_show(GTK_WIDGET (gridboard));
	g_signal_connect(G_OBJECT (gridboard), "boxclicked",
			G_CALLBACK(boxclicked_cb), NULL);
	gtk_widget_show(window);
	apply_changes();
}

char * get_tileset_path(char * tileset) {
        static gchar *tilesetpath = NULL;
        gchar *pathfrag;
      
      
        if (tilesetpath)
          g_free (tilesetpath);
      
        pathfrag = g_build_filename ("iagno", tileset, NULL);
      
        tilesetpath = gnome_program_locate_file (NULL, 
						 GNOME_FILE_DOMAIN_PIXMAP, 
						 pathfrag, FALSE, NULL);
        g_free (pathfrag);
      
	if (!g_file_test (tilesetpath, G_FILE_TEST_EXISTS)) {
	  g_free (tilesetpath);
	  tilesetpath = gnome_program_locate_file (NULL,
	  					   GNOME_FILE_DOMAIN_PIXMAP,
						   "iagno/classic.png",
						   FALSE, NULL);
	}

        return tilesetpath;
}

/* this is where it all starts. After the window is brought up, the user
 * probably starts a new game (new_game_cb()), and clicks some (boxclicked_cb).
 */
int main(int argc, char ** argv) {
	gnome_score_init("gataxx");

	settextdomain();

	initgnomeclient(argc, argv);

	create_window();

	new_game ();

	gtk_main();

	return 0;
}


