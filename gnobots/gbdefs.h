/*
 * gbdefs.h - 'DEFINES' used in all parts of gnobots
 */
#ifndef GBDEFS_H
#define GBDEFS_H

/*
 * Names of the pixmap files containing the "speech bubbles"
 */
#define YAHOO_PIXMAP_NAME       "yahoo.png"
#define AIEEE_PIXMAP_NAME       "aieee.png"

/*
 * Size of the "speech bubble" pixmaps
 * Note: this is the size of each bubble, the actual pixmap
 * contains 4 (2x2) pointing in each direction
 */
#define BUBBLE_WIDTH            86
#define BUBBLE_HEIGHT           34
#define BUBBLE_XOFFSET          10
#define BUBBLE_YOFFSET          4
#define BUBBLE_XSHIFT           4
#define BUBBLE_YSHIFT           2

/*
 * Size of the game objects
 */
#define TILE_WIDTH              16
#define TILE_HEIGHT             16

/*
 * Size of the game playing area
 */
#define GAME_WIDTH              45
#define GAME_HEIGHT             30

/*
 * various values for the pixmaps
 */
#define NUM_TILES_IN_PIXMAP     10
#define NUM_PLAYER_ANIMATIONS   4
#define NUM_ROBOT_ANIMATIONS    4
/* player tiles */
#define PLAYER_BASE             0
#define PLAYER_PIXMAP0          (PLAYER_BASE)
#define PLAYER_PIXMAP1          (PLAYER_BASE+1)
#define PLAYER_PIXMAP2          (PLAYER_BASE+2)
#define PLAYER_PIXMAP3          (PLAYER_BASE+3)
#define PLAYER_PIXMAP4          (PLAYER_BASE+4)
#define PLAYER_DEAD             PLAYER_PIXMAP4
/* robot tiles */
#define ROBOT_BASE              5
#define ROBOT_PIXMAP0           (ROBOT_BASE)
#define ROBOT_PIXMAP1           (ROBOT_BASE+1)
#define ROBOT_PIXMAP2           (ROBOT_BASE+2)
#define ROBOT_PIXMAP3           (ROBOT_BASE+3)
#define ROBOT_PIXMAP4           (ROBOT_BASE+4)
#define ROBOT_DEAD              ROBOT_PIXMAP4
#define HEAP_PIXMAP             ROBOT_DEAD

/*
 * Animation speed
 */
#define ANIMATION_DELAY         100     /* Delay in ms */
#define WAVE_PAUSE              20      /* time before player waves */
#define NUM_WAVES               2

/*
 * Game states
 */
#define GAME_NOT_PLAYING        0
#define GAME_PLAYING            1
#define GAME_LEVEL_CHANGE       2
#define GAME_DEAD               3
#define GAME_WAITING            4

/*
 * Game properties
 */
#define INITIAL_ROBOTS          10
#define INITIAL_TELEPORTS       1
#define ROBOT_INCREMENT         10
#define ROBOT_SCORE             10
#define MAX_SAFE_TELEPORTS      10
#define DEAD_DELAY              20
#define CHANGE_DELAY            20
#define WAITING_DELAY           1

/*
 * Grid Entries
 */
#define GRID_EMPTY              0
#define GRID_ROBOT              1
#define GRID_HEAP               2

#endif // GBDEFS_H

