#ifndef LOGICAL_BOARD_H
#define LOGICAL_BOARD_H 1

#include "config.h"
#include "board_view.h"
#include "coord_list.h"
#include "enums.h"



typedef enum
{
    csi_no_selection,
    csi_piece_selected
} control_state_info;


typedef enum
{
    BT_laser,
    BT_freezer,
    BT_none
} Beam_Type;


@interface Logical_Board : Object
{
    char board[ 15 ][ 11 ]; /* holds the type of piece */
    char color[ 15 ][ 11 ]; /* holds color of piece,
			       as well if it is frozen or lazed */
    char direction[ 15 ][ 11 ]; /* the direction the piece is facing */
    char status[ 15 ][ 11 ]; /* 0 if it can still be activated */

    Board_View **bvs; /* array of open windows showing this board */
    int max_bvs; /* max number (bvs[x] can be null) */

    Board_View *red_bv;   /* which board view controls red */
    Board_View *green_bv; /* which board view controls green */

    control_state_info csi; /* state info for the input state machine */

    Beam_Type laser_paths_beam_type;
    unsigned char laser_paths_in[15][11]; /* one bit per direction */
    unsigned char laser_paths_out[15][11];

    Coord_List *coord_list;

    Player_Color whos_turn;
    int sel_x, sel_y, sel_r;
    int moves; /* moves taken so far this turn */
}

- init_logical_board;

- (void) load_file : (char *) file_name;
- (char *) mouse_button_press : (int) x : (int) y
                              : (int) n : (Player_Color) owners_color;
- (void) key_press : (int) key;

- (void) add_board_view : (Board_View *) set_bv;
- (void) delete_board_view : (Board_View *) del_bv;
- (void) update_moves_left;

- (void) clear_status_board;

- (void) clear_laser_board;
- (void) process_laser : (int) x : (int) y : (int) ldir;
- (char *) fire : (Player_Color) owners_color;
- (void) unfire;
- (char *) pass : (Player_Color) owners_color;

- (int) x_dir_add : (int) x : (int) dir;
- (int) y_dir_add : (int) y : (int) dir;

- (int) stomp : (int) x1 : (int) y1 : (int) x2 : (int) y2
	      : (char **) message;
- (void) stomp_swap : (int) x1 : (int) y1 : (int) x2 : (int) y2
	      : (char **) message;
- (void) stomp_stompy_dies : (int) x1 : (int) y1 : (int) x2 : (int) y2
	      : (char **) message;
- (void) stomp_stomper_dies : (int) x1 : (int) y1 : (int) x2 : (int) y2
	      : (char **) message;
- (void) stomp_stompy_jumps : (int) x1 : (int) y1 : (int) x2 : (int) y2
	      : (char **) message;
- (void) stomp_stomper_jumps : (int) x1 : (int) y1 : (int) x2 : (int) y2
	      : (char **) message;

- (int) dirs_look_the_same : (int) piece : (int) dir1 : (int) dir2;

- (void) request_color : (Board_View *) bv;
- (void) message_to_all : (char *) message;
- (void) check_for_kings;
- (int) number_of_players;

@end

#endif /* LOGICAL_BOARD_H */
