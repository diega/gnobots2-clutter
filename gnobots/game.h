/*
 * game.h - Exported variables and Functions from game.c
 */
#ifndef GAME_H
#define GAME_H

extern int score;
extern int safe_teleports;
extern int num_robots;
extern int robots_left;
extern int level;
extern int game_grid[GAME_WIDTH][GAME_HEIGHT];
extern int player_xpos;
extern int player_ypos;
extern int layout_changed;

void start_new_game();
void start_new_level();
void update_level();

#endif // GAME_H

