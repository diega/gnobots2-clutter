#ifndef _IAGNO2_DEFINES_H_
#define _IAGNO2_DEFINES_H_

#define PLUGIN_VERSION "1"

#define TILEWIDTH   60
#define TILEHEIGHT  60
/*
#define GRIDWIDTH   1
*/
/*
#define BOARDSIZE   8
*/
/*
#define BOARDWIDTH  (TILEWIDTH + GRIDWIDTH) * BOARDSIZE - 1
#define BOARDHEIGHT (TILEHEIGHT + GRIDWIDTH) * BOARDSIZE - 1
*/
#define BOARDWIDTH  TILEWIDTH * BOARDSIZE
#define BOARDHEIGHT TILEHEIGHT * BOARDSIZE

/*
#define INDEX(r,c) ((r << 3) + c)
#define ROW(x)     (x >> 3)
#define COL(x)     (x & 0x7)
*/

#define PLAYER(x)  (x-1)?1:0

/*
#define WHITE_TILE 31
#define BLACK_TILE 1
*/

#define UP		-8
#define UP_LEFT		-9
#define UP_RIGHT	-7
#define LEFT		-1
#define RIGHT		1
#define DOWN		8
#define DOWN_LEFT	7
#define DOWN_RIGHT	9

#endif
