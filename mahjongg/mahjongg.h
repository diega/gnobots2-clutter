/*
 * Gnome-Mahjonggg main header
 * (C) 1998 the Free Software Foundation
 *
 *
 * Author: Francisco Bustamante et al.
 *
 *
 * http://www.nuclecu.unam.mx/~pancho/
 * pancho@nuclecu.unam.mx
 *
 */

#define MAX_TILES 144
#define MAX_TILES_STR "144"

#define TILE_WIDTH 40
#define TILE_HEIGHT 56
#define HALF_WIDTH 18
//#define HALF_WIDTH 20
#define HALF_HEIGHT 26
//#define HALF_HEIGHT 28
#define MAH_VERSION "0.4.0"

typedef struct _tilepos tilepos;     
typedef struct _tile tile;
typedef struct _tiletypes tiletypes;

struct _tiletypes {
	int type;
	int num_tiles;
	int image;
};

extern tiletypes default_types[] ;

struct _tilepos {
	int x;
	int y;
	int layer;
};

extern tilepos default_pos [];

struct _tile{
  int type;
  int image;
  int layer;
  int x;
  int y;
  int visible;
  int selected;
  int sequence;
};

extern tile tiles[MAX_TILES];
