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

/* If defined this cooks the sequence no.s,
   press redo in a new game */
#define CHEAT_DEBUG

#define MAX_TILES 144
#define MAX_TILES_STR "144"

#define TILE_WIDTH 40
#define TILE_HEIGHT 56
#define HALF_WIDTH 18
#define HALF_HEIGHT 26
#define MAH_VERSION "0.5.0"

typedef struct _tilepos tilepos;     
typedef struct _tile tile;
typedef struct _typeinfo typeinfo;

struct _typeinfo {
  int type;
  int placed;
  int image[2];
};

extern typeinfo type_info[] ;

struct _tilepos {
	int x;
	int y;
	int layer;
};

extern tilepos *pos ;

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
