/*
 * gnobots.c - Main UI part of Gnobots
 * written by Mark Rae <Mark.Rae@ed.ac.uk>
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
                   
#include <config.h>
#include <gnome.h>
#include <gdk/gdkkeysyms.h>

#include <time.h>
#include <string.h>

#include "gbdefs.h"
#include "game.h"
#include "properties.h"

/*
 * Variables
 */
GtkWidget *app              = NULL;
GtkWidget *game_area        = NULL;
GtkWidget *score_area       = NULL;
GtkWidget *score_label      = NULL;
GtkWidget *safe_label       = NULL;
GtkWidget *level_label      = NULL;

GdkPixmap *tile_pixmap      = NULL;
GdkPixmap *yahoo_pixmap     = NULL;
GdkPixmap *yahoo_mask       = NULL;
GdkPixmap *aieee_pixmap     = NULL;
GdkPixmap *aieee_mask       = NULL;

GdkColor   bgcolor;

char      *cmdline_scenario = NULL;

int        game_state        = GAME_NOT_PLAYING;

int        game_timeout_id   = -1;
int        player_frame      = 0;
int        robot_frame       = 0;
int        wave_timeout      = 0;
int        wave_counter      = 0;
int        player_step       = 1;
int        delay_counter     = 0;
int        wait_bonus        = 0;

int        session_flag      = 0;
int        session_xpos      = -1;
int        session_ypos      = -1;
int        session_position  = 0;

/*
 * Function prototypes
 */
static char   *nstr(int);
static int     save_state(GnomeClient*, gint, GnomeRestartStyle, gint,
                       GnomeInteractStyle, gint, gpointer);
static void    session_die(gpointer);
static void    reset_player_frame();
static void    update_player_frame();
static void    update_robot_frame();
static void    not_playing_animate();
static void    draw_grid();
static void    playing_animate();
static void    dead_animate();
static void    change_animate();
static void    waiting_animate();
static int     timeout_cb(void*);
static void    add_timeout();
static void    remove_timeout();
       void    clear_game_area();
static void    draw_tile_pixmap(int, int, int);
static void    draw_player_pixmap(int, int);
static void    draw_robot_pixmap(int, int);
static void    draw_bubble(GdkPixmap*, GdkPixmap*, int, int);
static void    draw_yahoo_bubble(int, int);
static void    draw_aieee_bubble(int, int);
static void    load_bubble_pixmaps();
       void    load_tile_pixmap(char*);
static void    really_new_cb(GtkWidget*, gpointer);       
static void    new_cb(GtkWidget*, gpointer);
static void    quit_cb(GtkWidget*, gpointer);
static void    really_quit_cb(GtkWidget*, gpointer);
static void    about_cb(GtkWidget*, gpointer);
static void    game_scores_cb(GtkWidget*, gpointer);
static int     check_nhood(int, int);
static int     check_move_safe(int, int);
static int     check_safe(int, int);
static void    check_collision();
static void    do_random_teleport();
static void    do_safe_teleport();
static void    do_teleport();
static gint    keyboard_cb(GtkWidget*, GdkEventKey*, gpointer);
static void    update_score_label();
static void    update_teleports_label();
static void    update_level_label();
static void    show_teleport_message(int safe);
       void    show_rollover_message();

static struct poptOption options[] = {
  {"scenario", 's', POPT_ARG_STRING, &cmdline_scenario, 0, N_("Set game scenario"), N_("NAME")},
  {NULL, 'x', POPT_ARG_INT, &session_xpos, 0, NULL, N_("X")},
  {NULL, 'y', POPT_ARG_INT, &session_ypos, 0, NULL, N_("Y")},
  {NULL, '\0', 0, NULL, 0}
};

/*
 * Game menu entries
 */
GnomeUIInfo gamemenu[] = {
    {GNOME_APP_UI_ITEM, N_("_New"), NULL, new_cb, NULL, NULL,
        GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_NEW, 'n', GDK_CONTROL_MASK, NULL},

    {GNOME_APP_UI_ITEM, N_("_Properties..."), NULL, properties_cb, NULL, NULL,
        GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_PROP, 0, 0, NULL},

    {GNOME_APP_UI_ITEM, N_("_Scores..."), NULL, game_scores_cb, NULL, NULL,
        GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_SCORES, 0, 0, NULL},

    {GNOME_APP_UI_ITEM, N_("_Quit"), NULL, quit_cb, NULL, NULL,
        GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_EXIT, 'q', GDK_CONTROL_MASK, NULL},

    {GNOME_APP_UI_ENDOFINFO}
};

/*
 * Help menu entries
 */
GnomeUIInfo helpmenu[] = {
    {GNOME_APP_UI_ITEM, N_("_About..."), NULL, about_cb, NULL, NULL,
        GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_ABOUT, 0, 0, NULL},

    {GNOME_APP_UI_HELP, NULL, NULL, "gnobots", NULL, NULL,
        GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},

    {GNOME_APP_UI_ENDOFINFO}        
};

/*
 * Main menu
 */
GnomeUIInfo mainmenu[] = {
    {GNOME_APP_UI_SUBTREE, N_("_Game"), NULL, gamemenu, NULL, NULL,
        GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},

    {GNOME_APP_UI_SUBTREE, N_("_Help"), NULL, helpmenu, NULL, NULL,
        GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
        
    {GNOME_APP_UI_ENDOFINFO}
};


/* A little helper function.  */
static char *nstr(
int n
){
    char buf[20];
    sprintf(buf, "%d", n);
    return strdup(buf);
}

/*
 * Save the session manager state
 */
static int save_state(
GnomeClient        *client,
gint                phase,
GnomeRestartStyle   save_style,
gint                shutdown,
GnomeInteractStyle  interact_style,
gint                fast,
gpointer            client_data
){
        char *argv[20];
        int i;
        gint xpos, ypos;

        gdk_window_get_origin(app->window, &xpos, &ypos);

        i = 0;
        argv[i++] = (char *)client_data;
        argv[i++] = "-x";
        argv[i++] = nstr(xpos);
        argv[i++] = "-y";
        argv[i++] = nstr(ypos);

        gnome_client_set_restart_command(client, i, argv);
        /* i.e. clone_command = restart_command - '--sm-client-id' */
        gnome_client_set_clone_command(client, 0, NULL);

        /* free memory from nstr(s) */
        free(argv[2]);
        free(argv[4]);

        return TRUE;
}

/*
 * Session is closing
 */
static void session_die(
gpointer    client_data
){
    remove_timeout();
    
    save_properties();
    
    gtk_widget_destroy(app);
    gtk_main_quit();    
}


/*
 * Reset player animation
 */
static void reset_player_frame(
){
    player_frame = 0;
    player_step  = 1;
    wave_timeout = 0;
    wave_counter = 0;
}

/*
 * Update player animation
 */
static void update_player_frame(
){
    if(player_frame == 0){
        if(wave_timeout < WAVE_PAUSE){
            wave_timeout++;
        } else {
            wave_timeout = 0;
            player_frame = 1;
            player_step = 1;
            wave_counter = 0;
        }
    } else {
        player_frame += player_step;
        if(player_frame == 3){
            player_step = -1;
            wave_counter++;
        }
        if(player_frame == 1){
            if(wave_counter < NUM_WAVES){
                player_step = 1;
            }
        }
    }
}

/*
 * Reset robot animation
 */
#if 0
static void reset_robot_frame(
){
    robot_frame = 0;
}
#endif

/*
 * Update robot animation
 */
static void update_robot_frame(
){
    robot_frame += 1;
    if(robot_frame >= NUM_ROBOT_ANIMATIONS) robot_frame = 0;
}

/*
 * Do animation steps when in 'NOT_PLAYING' mode
 */
static void not_playing_animate(
){
    update_player_frame();
    update_robot_frame();
    
    draw_player_pixmap(23, 15);
    
    draw_robot_pixmap(27, 15);
    draw_robot_pixmap(19, 15);
    draw_robot_pixmap(23, 11);
    draw_robot_pixmap(23, 19);

    draw_tile_pixmap(HEAP_PIXMAP, 19, 11);
    draw_tile_pixmap(HEAP_PIXMAP, 27, 11);
    draw_tile_pixmap(HEAP_PIXMAP, 19, 19);
    draw_tile_pixmap(HEAP_PIXMAP, 27, 19);    
    
}

/*
 * Redraw the game grid
 */
static void draw_grid(
){
    int x, y;

    for(x = 0; x < GAME_WIDTH; x++){
        for(y = 0; y < GAME_HEIGHT; y++){
            if((x == player_xpos) && (y == player_ypos)) continue;
            if(game_grid[x][y] == GRID_ROBOT){
                draw_robot_pixmap(x, y);
            } else if(game_grid[x][y] == GRID_HEAP){
                draw_tile_pixmap(HEAP_PIXMAP, x, y);
            } else {
                if(layout_changed) draw_tile_pixmap(-1, x, y);
            }
        }
    }
    
    layout_changed = FALSE;
}

/*
 * Do animation steps when in 'PLAYING' mode
 */
static void playing_animate(
){
    update_player_frame();
    update_robot_frame();

    draw_grid();
    draw_player_pixmap(player_xpos, player_ypos);

}

/*
 * Do animation steps when in 'DEAD' mode
 */
static void dead_animate(
){
    update_robot_frame();

    draw_grid();
    draw_tile_pixmap(PLAYER_DEAD, player_xpos, player_ypos);
    draw_aieee_bubble(player_xpos, player_ypos);

    delay_counter++;
    if(delay_counter >= DEAD_DELAY){
        delay_counter = 0;
        game_state = GAME_NOT_PLAYING;
        clear_game_area();
    }
}

/*
 * Do animation steps when in 'CHANGE_LEVEL' mode
 */
static void change_animate(
){
    update_player_frame();
    update_robot_frame();

    draw_grid();
    draw_player_pixmap(player_xpos, player_ypos);
    draw_yahoo_bubble(player_xpos, player_ypos);

    delay_counter++;
    if(delay_counter >= CHANGE_DELAY){
        delay_counter = 0;
        game_state = GAME_PLAYING;
        start_new_level();
        update_level_label();
    }
}

/*
 * Do animation steps when in 'WAITING' mode
 */
static void waiting_animate(
){
    update_player_frame();
    update_robot_frame();

    draw_grid();
    draw_player_pixmap(player_xpos, player_ypos);

    delay_counter++;
    if(delay_counter >= WAITING_DELAY){
        delay_counter = 0;

        update_level();
        update_score_label();

        check_collision();

        if(!robots_left){
            game_state = GAME_LEVEL_CHANGE;
            delay_counter = 0;
            if(safe_teleport_on){
                safe_teleports += wait_bonus;
                if(safe_teleports > MAX_SAFE_TELEPORTS){
                    safe_teleports = MAX_SAFE_TELEPORTS;
                }
            }
            update_teleports_label();
        }

    }
}

/*
 * Callback function for timer
 */
static int timeout_cb(
void *data
){
    switch(game_state){
        case GAME_NOT_PLAYING:
            not_playing_animate();
            break;
        case GAME_PLAYING:
            playing_animate();
            break;
        case GAME_DEAD:
            dead_animate();
            break;
        case GAME_LEVEL_CHANGE:
            change_animate();
            break;
        case GAME_WAITING:
            waiting_animate();
            break;
    }

    return TRUE;
}

/*
 * Destroy the timer
 */
static void remove_timeout(
){
    if(game_timeout_id != -1){
        gtk_timeout_remove(game_timeout_id);
        game_timeout_id = -1;
    }
}

/*
 * Create the timer
 */
static void add_timeout(
){
    if(game_timeout_id != -1){
        remove_timeout();
    }

    game_timeout_id = gtk_timeout_add(ANIMATION_DELAY, timeout_cb, 0);
}

/*
 * Clear the game area
 */
void clear_game_area(
){
    gdk_window_clear_area (game_area->window, 0, 0,
                            TILE_WIDTH*GAME_WIDTH,
                            TILE_HEIGHT*GAME_HEIGHT);
}

/*
 * Draw the appropriate pixmap from the current scenario
 */
static void draw_tile_pixmap(
int tileno,
int x,
int y
){
    int xpos = x*TILE_WIDTH;
    int ypos = y*TILE_WIDTH;

    if(tileno < 0){
        gdk_window_clear_area (game_area->window, xpos, ypos,
                                TILE_WIDTH, TILE_HEIGHT);
    } else if(tileno < NUM_TILES_IN_PIXMAP){
        gdk_draw_pixmap(game_area->window, game_area->style->black_gc,
                        tile_pixmap, tileno*TILE_WIDTH, 0, xpos, ypos,
                        TILE_WIDTH, TILE_HEIGHT);
    }
}

/*
 * Draw the current player frame
 */
static void draw_player_pixmap(
int x,
int y
){
    draw_tile_pixmap(PLAYER_BASE+player_frame, x, y);
}

/*
 * Draw the current robot frame
 */
static void draw_robot_pixmap(
int x,
int y
){
    draw_tile_pixmap(ROBOT_BASE+robot_frame, x, y);
}

/*
 * Draw a speech bubble
 */
static void draw_bubble(
GdkPixmap *pmap,
GdkPixmap *mask,
int x,
int y
){
    int xpos = (x*TILE_WIDTH) + BUBBLE_XOFFSET;
    int ypos = (y*TILE_HEIGHT) + BUBBLE_YOFFSET - BUBBLE_HEIGHT;
    int bx = BUBBLE_WIDTH;
    int by = 0;
    
    if(!game_area || !pmap || !mask) return;

    if((xpos + BUBBLE_WIDTH) >= (TILE_WIDTH*GAME_WIDTH)){
        bx = 0;
        xpos -= BUBBLE_WIDTH;
        xpos -= BUBBLE_XSHIFT;
    }

    if(ypos <= 0){
        by = BUBBLE_HEIGHT;
        ypos += BUBBLE_HEIGHT;
        ypos -= BUBBLE_YSHIFT;
    }
    
    gdk_gc_set_clip_origin(game_area->style->black_gc, xpos-bx, ypos-by);
    gdk_gc_set_clip_mask(game_area->style->black_gc, mask);
    gdk_draw_pixmap(game_area->window, game_area->style->black_gc,
                 pmap, bx, by, xpos, ypos, BUBBLE_WIDTH, BUBBLE_HEIGHT);
    gdk_gc_set_clip_mask(game_area->style->black_gc, NULL);
}

/*
 * Draw the yahoo speech bubble
 */
static void draw_yahoo_bubble(
int x,
int y
){
    draw_bubble(yahoo_pixmap, yahoo_mask, x, y);
}

/*
 * Draw the aieee speech bubble
 */
void draw_aieee_bubble(
int x,
int y
){
    draw_bubble(aieee_pixmap, aieee_mask, x, y);
}

/*
 * Load the pixmaps for the speech bubbles
 */
static void load_bubble_pixmaps(
){
    char *tmp;
    char *fname;
    GdkImlibImage *image;

    tmp = g_copy_strings("gnobots/", YAHOO_PIXMAP_NAME, NULL);
    fname = gnome_unconditional_pixmap_file(tmp);
    g_free(tmp);

    if(!g_file_exists(fname)){
        printf(_("Could not find \'%s\' pixmap file for Gnome Robots\n"), fname);
        exit(1);
    }

    image = gdk_imlib_load_image(fname);
    gdk_imlib_render(image, image->rgb_width, image->rgb_height);
    yahoo_pixmap = gdk_imlib_move_image(image);
    yahoo_mask = gdk_imlib_move_mask(image);
    gdk_imlib_destroy_image(image);
    g_free(fname);

    tmp = g_copy_strings("gnobots/", AIEEE_PIXMAP_NAME, NULL);
    fname = gnome_unconditional_pixmap_file(tmp);
    g_free(tmp);

    if(!g_file_exists(fname)){
        printf(_("Could not find \'%s\' pixmap file for Gnome Robots\n"), fname);
        exit(1);
    }

    image = gdk_imlib_load_image(fname);
    gdk_imlib_render(image, image->rgb_width, image->rgb_height);
    aieee_pixmap = gdk_imlib_move_image(image);
    aieee_mask = gdk_imlib_move_mask(image);
    gdk_imlib_destroy_image(image);
    g_free(fname);

}

/*
 * Load the pixmap for a given scenario
 */
void load_tile_pixmap(
char *pmname
){
    char          *tmp;
    char          *fname;
    GdkImlibImage *image;
    GdkImage      *tmpimage;
    GdkVisual     *visual;
   
    tmp = g_copy_strings("gnobots/", pmname, NULL);
    fname = gnome_unconditional_pixmap_file(tmp);
    g_free(tmp);

    if(!g_file_exists(fname)){
        printf(_("Could not find \'%s\' pixmap file for Gnome Robots\n"), fname);
        exit(1);
    }

    image = gdk_imlib_load_image(fname);
    visual = gdk_imlib_get_visual();
    if(visual->type != GDK_VISUAL_TRUE_COLOR){
        gdk_imlib_set_render_type(RT_PLAIN_PALETTE);
    }
    gdk_imlib_render(image, image->rgb_width, image->rgb_height);
    tile_pixmap = gdk_imlib_move_image(image);
    tmpimage = gdk_image_get(tile_pixmap, 0, 0, 1, 1);
    bgcolor.pixel = gdk_image_get_pixel(tmpimage, 0, 0);
    gdk_window_set_background (game_area->window, &bgcolor);
    gdk_image_destroy(tmpimage);
            
    gdk_imlib_destroy_image(image);
    g_free(fname);
}


/*
 * Really start new game
 */
static void really_new_cb(
GtkWidget *widget,
gpointer  data
){
    gint button = GPOINTER_TO_INT (data);
    
    if(button != 0) return;
    
    game_state = GAME_PLAYING;    
    start_new_game();
    
    if(!safe_teleport_on) safe_teleports = 0;
    
    update_score_label();
    update_teleports_label();
    update_level_label();
}

/*
 * Callback for the menu - New
 */
static void new_cb(
GtkWidget *widget,
gpointer  data
){
    GtkWidget *box;

    if(game_state == GAME_NOT_PLAYING){
        really_new_cb(widget, (gpointer)0);
    } else {
        box = gnome_message_box_new(
                            _("Do you really want to start a new game?"),
                                     GNOME_MESSAGE_BOX_QUESTION,
                                     GNOME_STOCK_BUTTON_YES,
                                     GNOME_STOCK_BUTTON_NO,
                                     NULL);
        gnome_dialog_set_default (GNOME_DIALOG(box), 0);
        gnome_dialog_set_modal (GNOME_DIALOG(box));
        gtk_signal_connect (GTK_OBJECT(box), "clicked",
                           GTK_SIGNAL_FUNC(really_new_cb), NULL);
        gtk_widget_show(box);
    }
}

/*
 * Callback for the menu - Exit
 */
static void quit_cb(
GtkWidget *widget,
gpointer  data
){
    GtkWidget *box;
        
    if(game_state == GAME_NOT_PLAYING){
        really_quit_cb(widget, (gpointer)0);
    } else {
        box = gnome_message_box_new(_("Do you really want to quit?"),
                                     GNOME_MESSAGE_BOX_QUESTION,
                                     GNOME_STOCK_BUTTON_YES,
                                     GNOME_STOCK_BUTTON_NO,
                                     NULL);
        gnome_dialog_set_default (GNOME_DIALOG(box), 0);
        gnome_dialog_set_modal (GNOME_DIALOG(box));
        gtk_signal_connect (GTK_OBJECT(box), "clicked",
                           GTK_SIGNAL_FUNC(really_quit_cb), NULL);
        gtk_widget_show(box);
    }
}

/*
 * Callback for the quit dialog box
 */
static void really_quit_cb(
GtkWidget   *widget,
gpointer    data
){
    gint button = GPOINTER_TO_INT (data);
    
    if(button != 0) return;
    
    remove_timeout();
    
    save_properties();
    
    gtk_widget_destroy(app);
    gtk_main_quit();
}


/*
 * Callback for the menu - About
 */
static void about_cb(
GtkWidget *widget, 
gpointer data
){
    GtkWidget *about;
    const gchar *authors[] = {
            "Mark Rae <Mark.Rae@ed.ac.uk>",
            NULL
    };

    about = gnome_about_new(_("Gnobots"), VERSION,
                             "(C) 1998 Mark Rae",
                             (const char **) authors,
                             _("Gnome Robots game"),
                             NULL);
    gtk_widget_show(about);
}

/*
 * Show the scores dialog box
 */
static void show_scores(
guint pos
){
    if(safe_teleport_on){
        gnome_scores_display(_("Gnome Robots - Safe Teleports"),
                            "gnobots", "safe", pos);
    } else {
        gnome_scores_display(_("Gnome Robots"), "gnobots", "unsafe", pos);
    }
}

/*
 * Callback for the menu - Scores
 */
static void game_scores_cb(
GtkWidget *widget,
gpointer data
){
    show_scores(0);
}

/*
 * Helper function for 'check_safe'
 * returns TRUE if there is a neighbouring robot
 */
static int check_nhood(
int x,
int y
){
    if((x > 0) && (y > 0) &&
        (game_grid[x-1][y-1] == GRID_ROBOT)) return TRUE;
    if((y > 0) &&
        (game_grid[x][y-1] == GRID_ROBOT)) return TRUE;
    if((x < (GAME_WIDTH-1)) && (y > 0) &&
        (game_grid[x+1][y-1] == GRID_ROBOT)) return TRUE;

    if((x > 0) &&
        (game_grid[x-1][y] == GRID_ROBOT)) return TRUE;
    if((x < (GAME_WIDTH-1)) && 
        (game_grid[x+1][y] == GRID_ROBOT)) return TRUE;

    if((x > 0) && (y < (GAME_HEIGHT-1)) &&
        (game_grid[x-1][y+1] == GRID_ROBOT)) return TRUE;
    if((y < (GAME_HEIGHT-1)) &&
        (game_grid[x][y+1] == GRID_ROBOT)) return TRUE;
    if((x < (GAME_WIDTH-1)) && (y < (GAME_HEIGHT-1)) &&
        (game_grid[x+1][y+1] == GRID_ROBOT)) return TRUE;

    return FALSE;
}

/*
 * Helper function for 'check_safe'
 * returns TRUE if a move is safe, FALSE if unsafe or invalid
 */
static int check_move_safe(
int x,
int y
){
    if(game_grid[x][y]) return FALSE;
    if(check_nhood(x, y)) return FALSE;
    
    return TRUE;
}

/*
 * Check for a safe move
 * in safe mode the player is prevented from killing themselves
 * accidentally. i.e. they are prevented from moving to a space
 * which would kill them if there is at least one safe move
 * available to them
 */
static int check_safe(
int nx,
int ny
){
    /* is our chosen move safe ... */
    if(!check_nhood(nx, ny)) return TRUE;

    /* ... if it isn't then we look to see if there is
       a safe move we could have made */

    if((player_xpos > 0) && (player_ypos > 0) &&
        check_move_safe(player_xpos-1, player_ypos-1)) return FALSE;
    if((player_ypos > 0) &&
        check_move_safe(player_xpos, player_ypos-1)) return FALSE;
    if((player_xpos < (GAME_WIDTH-1)) && (player_ypos > 0) &&
        check_move_safe(player_xpos+1, player_ypos-1)) return FALSE;
        
    if((player_xpos > 0) &&
        check_move_safe(player_xpos-1, player_ypos)) return FALSE;
    if(check_move_safe(player_xpos, player_ypos)) return FALSE;
    if((player_xpos < (GAME_WIDTH-1)) &&
        check_move_safe(player_xpos+1, player_ypos)) return FALSE;
        
    if((player_xpos > 0) && (player_ypos < (GAME_HEIGHT-1)) &&
        check_move_safe(player_xpos-1, player_ypos+1)) return FALSE;
    if((player_ypos < (GAME_HEIGHT-1)) &&
        check_move_safe(player_xpos, player_ypos+1)) return FALSE;
    if((player_xpos < (GAME_WIDTH-1)) && (player_ypos < (GAME_HEIGHT-1)) &&
        check_move_safe(player_xpos+1, player_ypos+1)) return FALSE;
       
    /* no safe moves were available, so we let the original
       move proceed - splat! */
    return TRUE;
}

/*
 * Check to see if we have collided with anything
 */
static void check_collision(
){
    int pos;
    
    if(game_grid[player_xpos][player_ypos]){
        if(sound_on) gdk_beep();
        game_state = GAME_DEAD;
        delay_counter = 0;
        game_grid[player_xpos][player_ypos] = GRID_EMPTY;
        if(score > 0){
            if(safe_teleport_on){
                pos = gnome_score_log(score, "safe", TRUE);
            } else {
                pos = gnome_score_log(score, "unsafe", TRUE);    
            }
            show_scores(pos);
        }
    }
}

/*
 * Teleport the player randomly
 */
static void do_random_teleport(
){
    int nx, ny, ox, oy;


    ox = nx = random() % GAME_WIDTH;
    ox = ny = random() % GAME_HEIGHT;
    while(1){
        if(game_grid[nx][ny] == GRID_EMPTY) break;
		nx++;
		if(nx >= GAME_WIDTH){
			nx = 0;
			ny++;
		}
		if(ny >= GAME_HEIGHT){
			ny = 0;
		}
		if((nx == ox) && (ny == oy)){
			show_teleport_message(FALSE);
			return;
		}
    }

    player_xpos = nx;
    player_ypos = ny;

    reset_player_frame();
    
    update_level();
    update_score_label();

    check_collision();

    if(!robots_left){
        game_state = GAME_LEVEL_CHANGE;
        delay_counter = 0;
    }
}

/*
 * Teleport the player safely
 */
static void do_safe_teleport(
){
    int nx, ny, ox, oy;


    ox = nx = random() % GAME_WIDTH;
    oy = ny = random() % GAME_HEIGHT;
    while(1){
        if(check_move_safe(nx, ny)) break;
		nx++;
		if(nx >= GAME_WIDTH){
			nx = 0;
			ny++;
		}
		if(ny >= GAME_HEIGHT){
			ny = 0;
		}
		if((nx == ox) && (ny == oy)){
			show_teleport_message(TRUE);
			return;
		}
    }

    safe_teleports--;
    update_teleports_label();

    player_xpos = nx;
    player_ypos = ny;

    reset_player_frame();
    
    update_level();
    update_score_label();

    check_collision();

    if(!robots_left){
        game_state = GAME_LEVEL_CHANGE;
        delay_counter = 0;
    }
}

/*
 * Teleport the player
 */
static void do_teleport(
){
    if(safe_teleports > 0){
        do_safe_teleport();
    } else {
        do_random_teleport();
    }
}

/*
 * Callback for keyboard events
 */
static gint keyboard_cb(
GtkWidget *widget,
GdkEventKey *event,
gpointer data
){
    int newx, newy;

    /* allow 'Space' to start a new game if we are not already playing */
    if(game_state != GAME_PLAYING){
        if((event->keyval == GDK_space) && (game_state == GAME_NOT_PLAYING)){
            new_cb(widget, NULL);
            return TRUE;
        } else {
            return FALSE;
        }
    }

    /* don't allow player to move until screen has been updated */
    if(layout_changed) return FALSE;
    
    switch(event->keyval){
        case GDK_KP_7:
        case GDK_KP_Home:
        case GDK_Y:
        case GDK_y:
        case GDK_Q:
        case GDK_q:
            newx = player_xpos - 1;
            newy = player_ypos - 1;
            break;
        case GDK_KP_8:
        case GDK_KP_Up:
        case GDK_K:
        case GDK_k:
        case GDK_W:
        case GDK_w:
            newx = player_xpos;
            newy = player_ypos - 1;
            break;
        case GDK_KP_9:
        case GDK_KP_Page_Up:
        case GDK_U:
        case GDK_u:
        case GDK_E:
        case GDK_e:
            newx = player_xpos + 1;
            newy = player_ypos - 1;
            break;
        case GDK_KP_4:
        case GDK_KP_Left:
        case GDK_H:
        case GDK_h:
        case GDK_A:
        case GDK_a:
            newx = player_xpos - 1;
            newy = player_ypos;
            break;
        case GDK_KP_6:
        case GDK_KP_Right:
        case GDK_L:
        case GDK_l:
        case GDK_D:
        case GDK_d:
            newx = player_xpos + 1;
            newy = player_ypos;
            break;
        case GDK_KP_1:
        case GDK_KP_End:
        case GDK_B:
        case GDK_b:
        case GDK_Z:
        case GDK_z:
            newx = player_xpos - 1;
            newy = player_ypos + 1;
            break;
        case GDK_KP_2:
        case GDK_KP_Down:
        case GDK_J:
        case GDK_j:
        case GDK_X:
        case GDK_x:
            newx = player_xpos;
            newy = player_ypos + 1;
            break;
        case GDK_KP_3:
        case GDK_KP_Page_Down:
        case GDK_N:
        case GDK_n:
        case GDK_C:
        case GDK_c:
            newx = player_xpos + 1;
            newy = player_ypos + 1;
            break;
        case GDK_KP_5:
        case GDK_KP_Begin:
        case GDK_period:
        case GDK_space:
        case GDK_S:
        case GDK_s:        
            newx = player_xpos;
            newy = player_ypos;
            break;
        case GDK_KP_Enter:
        case GDK_Return:
            game_state = GAME_WAITING;
            wait_bonus = robots_left;
            return TRUE;
            break;
        case GDK_KP_Add:
        case GDK_T:
        case GDK_t:
            do_teleport();
            return TRUE;
            break;
        case GDK_KP_Subtract:
        case GDK_R:
        case GDK_r:
            do_random_teleport();
            return TRUE;
            break;
        default:
            return FALSE;
    }

    /* Check we don't move out of the playing area */
    if((newx < 0) || (newx >= GAME_WIDTH) ||
        (newy < 0) || (newy >= GAME_HEIGHT)){
        if(sound_on) gdk_beep();
        return TRUE;
    }

    /* We can't move on top of a heap */
    if(game_grid[newx][newy] == GRID_HEAP){
        if(sound_on) gdk_beep();
        return TRUE;
    }

    if(safe_move_on){
        if(!check_safe(newx, newy)){
            if(sound_on) gdk_beep();
            return TRUE;
        }
    }
    
    player_xpos = newx;
    player_ypos = newy;
    reset_player_frame();
    
    update_level();
    update_score_label();

    check_collision();

    if(!robots_left){
        game_state = GAME_LEVEL_CHANGE;
        delay_counter = 0;
    }

    return TRUE;
}

/*
 * Update the score in the status bar
 */
static void update_score_label(
){
    char buffer[32];
    sprintf(buffer, "%d", score);
    
    gtk_label_set(GTK_LABEL(score_label), buffer);
}

/*
 * Update the no. of teleports in the status bar
 */
static void update_teleports_label(
){
    char buffer[32];
    sprintf(buffer, "%d", safe_teleports);

    gtk_label_set(GTK_LABEL(safe_label), buffer);
}

/*
 * Update the level in the status bar
 */
static void update_level_label(
){
    char buffer[32];
    sprintf(buffer, "%d", level);

    gtk_label_set(GTK_LABEL(level_label), buffer);
}

/*
 * Show message when we cannot teleport
 */
void show_teleport_message(
int safe
){
	GtkWidget *box;

	if(safe){
        box = gnome_message_box_new(
                                _("No Locations Available For Safe Teleport"), 
								GNOME_MESSAGE_BOX_INFO, 
								GNOME_STOCK_BUTTON_OK, NULL);
	} else {
        box = gnome_message_box_new(
                                _("No Locations Available For Teleport"), 
								GNOME_MESSAGE_BOX_INFO, 
								GNOME_STOCK_BUTTON_OK, NULL);
	}
	gnome_dialog_set_modal(GNOME_DIALOG(box));

	gtk_widget_show(box);
}

/*
 * Show message when there are too many robots
 */
void show_rollover_message(
){
	GtkWidget *box;

	box = gnome_message_box_new(
_("Congratulations, you have defeated the Robots!\nBut can you do it again?"), 
								GNOME_MESSAGE_BOX_INFO, 
								GNOME_STOCK_BUTTON_OK, NULL);
	gnome_dialog_set_modal(GNOME_DIALOG(box));

	gtk_widget_show(box);
}

/*
 * It all starts here!
 */
int main(
int argc,
char *argv[]
){
    GtkWidget *box;
    GtkWidget *sttbl;
    GtkWidget *label;
    GnomeClient *client;
    char buf[PATH_MAX];
    
    srandom(time(NULL));
    
    bindtextdomain(PACKAGE, GNOMELOCALEDIR);
    textdomain(PACKAGE);

    gnome_score_init("gnobots");

    
    gnome_init_with_popt_table("gnobots", VERSION, argc, argv, options, 0, NULL);

    client = gnome_master_client();

    getcwd(buf, sizeof(buf));

    /* This doesn't seem to work properly ?? */
    gnome_client_set_current_directory(client, buf);

    gtk_object_ref(GTK_OBJECT(client));
    gtk_object_sink(GTK_OBJECT(client));
    
    gtk_signal_connect(GTK_OBJECT (client), "save_yourself",
                       GTK_SIGNAL_FUNC (save_state), argv[0]);
    gtk_signal_connect(GTK_OBJECT (client), "die",
                       GTK_SIGNAL_FUNC (session_die), argv[0]);

    
    app = gnome_app_new("gnobots", _("Gnome Robots") );
    /* no resize allowed */
    gtk_window_set_policy(GTK_WINDOW(app), FALSE, FALSE, TRUE);
    gtk_signal_connect(GTK_OBJECT(app), "delete_event",
                      GTK_SIGNAL_FUNC(quit_cb),
                      NULL);
    gtk_signal_connect (GTK_OBJECT(app), "key_press_event",
                GTK_SIGNAL_FUNC(keyboard_cb), 0);                
    gnome_app_create_menus(GNOME_APP(app), mainmenu);

    /* move Help to the right-hand side */
/*    gtk_menu_item_right_justify(GTK_MENU_ITEM(mainmenu[1].widget));*/

    load_properties();

    box = gtk_vbox_new(FALSE, 0);
    gnome_app_set_contents(GNOME_APP(app), box);

    game_area = gtk_drawing_area_new();
    gtk_box_pack_start(GTK_BOX(box), game_area, TRUE, TRUE, 0);
    gtk_widget_realize(game_area);
    gtk_drawing_area_size(GTK_DRAWING_AREA(game_area),
            TILE_WIDTH*GAME_WIDTH, TILE_HEIGHT*GAME_HEIGHT);
    gtk_widget_show(game_area);

    sttbl = gtk_table_new(1, 8, FALSE);
    gtk_box_pack_start(GTK_BOX(box), sttbl, TRUE, TRUE, 0);

    label = gtk_label_new(_("Score:"));
    gtk_table_attach(GTK_TABLE(sttbl), label, 0, 1, 0, 1, 0, 0, 3, 3);
    gtk_widget_show(label);
    
    score_label = gtk_label_new("0");
    gtk_table_attach(GTK_TABLE(sttbl), score_label, 1, 2, 0, 1, 0, 0, 3, 3);
    gtk_widget_show(score_label);

    gtk_table_set_col_spacing(GTK_TABLE(sttbl), 2, 32);
    
    label = gtk_label_new(_("Safe Teleports:"));
    gtk_table_attach(GTK_TABLE(sttbl), label, 3, 4, 0, 1, 0, 0, 3, 3);
    gtk_widget_show(label);
    
    safe_label = gtk_label_new("0");
    gtk_table_attach(GTK_TABLE(sttbl), safe_label, 4, 5, 0, 1, 0, 0, 3, 3);
    gtk_widget_show(safe_label);
    
    gtk_table_set_col_spacing(GTK_TABLE(sttbl), 5, 32);
    
    label = gtk_label_new(_("Level:"));
    gtk_table_attach(GTK_TABLE(sttbl), label, 6, 7, 0, 1, 0, 0, 3, 3);
    gtk_widget_show(label);
    
    level_label = gtk_label_new("0");
    gtk_table_attach(GTK_TABLE(sttbl), level_label, 7, 8, 0, 1, 0, 0, 3, 3);
    gtk_widget_show(level_label);
    
    gtk_widget_show(sttbl);
    
    gtk_widget_show(box);

    load_bubble_pixmaps();

    if(cmdline_scenario){
        set_scenario(cmdline_scenario);
    } else {
        set_scenario(NULL);
    }

    /* Set the window position if it was set by the session manager */
    if(session_xpos >= 0 && session_ypos >= 0){
        gtk_widget_set_uposition(app, session_xpos, session_ypos);
    }
    
    gtk_widget_show(app);

    clear_game_area();

    add_timeout();
    
    gtk_main();

    gtk_object_unref(GTK_OBJECT(client));
    
    return 0;
}
