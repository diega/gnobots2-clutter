/*
 * game.c - setting up the level and moving the robots
 * written by Mark Rae <Mark.Rae@ed.ac.uk>
 */

#include "gbdefs.h"
#include <stdlib.h> /* For rand() */

/*
 * Variables
 */
int score;
int safe_teleports;
int num_robots;
int robots_left;
int level;
int player_xpos, player_ypos;
int layout_changed;
int game_grid[GAME_WIDTH][GAME_HEIGHT];

static int tmp_grid[GAME_WIDTH][GAME_HEIGHT];

/*
 * Function prototypes
 */
void start_new_game();
void start_new_level();
void update_level();

static void clear_level();
static void copy_level();
static void generate_level();

/*
 * Clear the level
 */
static void clear_level(
){
    int x, y;
    for(x = 0; x < GAME_WIDTH; x++){
        for(y = 0; y < GAME_HEIGHT; y++){
            game_grid[x][y] = GRID_EMPTY;
        }
    }
}

/*
 * Copy the level into the tmp array
 */
static void copy_level(
){
    int x, y;
    for(x = 0; x < GAME_WIDTH; x++){
        for(y = 0; y < GAME_HEIGHT; y++){
            tmp_grid[x][y] = game_grid[x][y];
        }
    }
}

/*
 * Generate new level
 */
static void generate_level(
){
    int i, x, y;

    player_xpos = GAME_WIDTH / 2;
    player_ypos = GAME_HEIGHT / 2;

    clear_level();
    
    for(i = 0; i < num_robots; i++){
        while(1){
            x = rand()%GAME_WIDTH;
            y = rand()%GAME_HEIGHT;
            if((x == player_xpos) && (y == player_ypos)) continue;
            if(game_grid[x][y] == GRID_EMPTY) break;
        }
        
        game_grid[x][y] = GRID_ROBOT;
    }

    robots_left = num_robots;    
    layout_changed = 1;
}

/*
 * Start a new level
 */
void start_new_level(
){
    level++;
    num_robots += ROBOT_INCREMENT;

    generate_level();    
}

/*
 * Start a new game
 */
void start_new_game(
){
    score = 0;
    level = 0;
    safe_teleports = INITIAL_TELEPORTS;
    num_robots = INITIAL_ROBOTS;
    generate_level();
}

/*
 * Update the level
 */
void update_level(
){
    int x, y, nx, ny;
    
    copy_level();
    clear_level();

    for(x = 0; x < GAME_WIDTH; x++){
        for(y = 0; y < GAME_HEIGHT; y++){
            if(tmp_grid[x][y] == GRID_ROBOT){
                if(player_xpos < x){
                    nx = x - 1;
                } else if(player_xpos > x){
                    nx = x + 1;
                } else {
                    nx = x;
                }
                if(player_ypos < y){
                    ny = y - 1;
                } else if(player_ypos > y){
                    ny = y + 1;
                } else {
                    ny = y;
                }
                
                if(game_grid[nx][ny] == GRID_ROBOT){
                    robots_left -= 2;
                    score += ROBOT_SCORE*2;
                    game_grid[nx][ny] = GRID_HEAP;                    
                } else if(game_grid[nx][ny] == GRID_HEAP){
                    robots_left--;
                    score += ROBOT_SCORE;
                } else {
                    game_grid[nx][ny] = GRID_ROBOT;
                }
            } else if(tmp_grid[x][y] == GRID_HEAP){
                if(game_grid[x][y] == GRID_ROBOT){
                    robots_left--;
                    score += ROBOT_SCORE;
                }
                game_grid[x][y] = GRID_HEAP;
            }
        }
    }
    
    layout_changed = 1;
}

