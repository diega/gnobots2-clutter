/* -*- Mode: C; indent-tabs-mode: nil; tab-width: 8; c-basic-offset: 2 -*- */

/*
 * Gnome-Mahjonggg pile creation algorithem
 *
 * (C) 2003 the Free Software Foundation
 *
 * Author: Callum McKenzie, <callum@physics.otago.ac.nz>
 *
 * This code is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
 * USA
 */

/* This code is a complete rewrite of Michael Meeks original. The data
 * structures are modelled on those he used (and the typeinfo
 * structure is identical) but the algorithm is completely reversed.
 * This removes the nasty code to check that the previous algorithm hadn't
 * worked itself into a corner.
 *
 * The new algorithm to generate the pile takes pairs off in the same order
 * that a player would. Since there is always a leftmost and rightmost
 * tile there is always two free tiles (the exception to this is if the tiles
 * are in a vertical row, but then, since we have an even number of tile
 * we will still have at least two to remove). We then mark any revealed
 * tiles as free and select another pair. Each time a pair is taken these
 * are marked as being identical tiles, hence forming a solvable pile.
 *
 */


#include "mahjongg.h"
#include "solubility.h"

struct _typeinfo {
  int type;
  int placed;
  int image[2];
};
typedef struct _typeinfo typeinfo;

typeinfo type_info [MAX_TILES/2] = {
  	{ 0, 0, {0, 0} },
	{ 0, 0, {0, 0} },
	{ 1, 0, {1, 1} },
	{ 1, 0, {1, 1} },
	{ 2, 0, {2, 2} },
	{ 2, 0, {2, 2} },
	{ 3, 0, {3, 3} },
	{ 3, 0, {3, 3} },
	{ 4, 0, {4, 4} },
	{ 4, 0, {4, 4} },
	{ 5, 0, {5, 5} },
	{ 5, 0, {5, 5} },
	{ 6, 0, {6, 6} },
	{ 6, 0, {6, 6} },
	{ 7, 0, {7, 7} },
	{ 7, 0, {7, 7} },
	{ 8, 0, {8, 8} },
	{ 8, 0, {8, 8} },
	{ 9, 0, {9, 9} },
	{ 9, 0, {9, 9} },
	{ 10, 0, {10, 10} },
	{ 10, 0, {10, 10} },
	{ 11, 0, {11, 11} },
	{ 11, 0, {11, 11} },
	{ 12, 0, {12, 12} },
	{ 12, 0, {12, 12} },
	{ 13, 0, {13, 13} },
	{ 13, 0, {13, 13} },
	{ 14, 0, {14, 14} },
	{ 14, 0, {14, 14} },
	{ 15, 0, {15, 15} },
	{ 15, 0, {15, 15} },
	{ 16, 0, {16, 16} },
	{ 16, 0, {16, 16} },
	{ 17, 0, {17, 17} },
	{ 17, 0, {17, 17} },
	{ 18, 0, {18, 18} },
	{ 18, 0, {18, 18} },
	{ 19, 0, {19, 19} },
	{ 19, 0, {19, 19} },
	{ 20, 0, {20, 20} },
	{ 20, 0, {20, 20} },
	{ 21, 0, {21, 21} },
	{ 21, 0, {21, 21} },
	{ 22, 0, {22, 22} },
	{ 22, 0, {22, 22} },
	{ 23, 0, {23, 23} },
	{ 23, 0, {23, 23} },
	{ 24, 0, {24, 24} },
	{ 24, 0, {24, 24} },
	{ 25, 0, {25, 25} },
	{ 25, 0, {25, 25} },
	{ 26, 0, {26, 26} },
	{ 26, 0, {26, 26} },
	{ 27, 0, {27, 27} },
	{ 27, 0, {27, 27} },
	{ 28, 0, {28, 28} },
	{ 28, 0, {28, 28} },
	{ 29, 0, {29, 29} },
	{ 29, 0, {29, 29} },
	{ 30, 0, {30, 30} },
	{ 30, 0, {30, 30} },
	{ 31, 0, {31, 31} },
	{ 31, 0, {31, 31} },
	{ 32, 0, {32, 32} },
	{ 32, 0, {32, 32} },
	{ 33, 0, {33, 34} },
	{ 33, 0, {35, 36} },
	{ 34, 0, {37, 37} },
	{ 34, 0, {37, 37} },
	{ 35, 0, {38, 39} },
	{ 35, 0, {40, 41} }
};

typedef struct _dep_entry {
  gboolean free;      /* Is this a valid place to remove a tile.   *
                       * Note that this is only used to generate   *
                       * the game and is not used during the game. */
  gboolean filled;    /* Has a tile been placed here. */
  gint foundation[4]; /* Up to four we build on. */
  gint left[2];       /* Up to two on the left. */
  gint right[2];      /* and two on the right. */
  gint overhead[4];   /* Up to four we support. */
} dep_entry;

dep_entry dependencies[MAX_TILES];

int tile_free (int index)
{
  dep_entry * dep;
  int i;
  gboolean free;

  dep = dependencies + index;

  if (tiles[index].visible == 0)
    return 0;
  
  /* Check to see we aren't covered. */
  for (i = 0; i<4; i++) {
    if ((dep->overhead[i] != -1) && tiles[dep->overhead[i]].visible)
      return 0;
  }

  /* Look left. */
  free = TRUE;
  for (i = 0; i<2; i++) {
    if ((dep->left[i] != -1) && tiles[dep->left[i]].visible)
      free = FALSE;
  }
  if (free)
    return 1;

  /* Look right. */
  free = TRUE;
  for (i = 0; i<2; i++) {
    if ((dep->right[i] != -1) && tiles[dep->right[i]].visible)
      free = FALSE;
  }

  return free ? 1 : 0;
  
}

/* This is essentially the same as the tile_free code, but uses the
 * dependencies data struture instead. This is so we can use the
 * generation code without disturbing the tile data structure
 * (especially the visible flag) so that in future we can use
 * generate instead of shuffle. */
static void check_tile_is_free (gint index)
{
  int i;
  gboolean ok;
  dep_entry * dep;

  dep = dependencies + index;

  if (!dep->filled)
    return;
  
  /* First, check if we are covered above. */
  ok = TRUE;
  for (i=0; i<4; i++) {
    if (dep->overhead[i] == -1)
      break;
    if (dependencies[dep->overhead[i]].filled) {
      ok = FALSE;
      break;
    }
  }
  if (!ok)
    return;

  /* Now check to the left. */
  ok = TRUE;
  for (i=0; i<2; i++) {
    if (dep->left[i] == -1)
      break;
    if (dependencies[dep->left[i]].filled) {
      ok = FALSE;
      break;
    }
  }
  if (ok) {
    dep->free = TRUE;
    return;
  }

  /* And now the right. */
  ok = TRUE;
  for (i=0; i<2; i++) {
    if (dep->right[i] == -1)
      break;
    if (dependencies[dep->right[i]].filled) {
      ok = FALSE;
      break;
    }
  }
  if (ok) {
    dep->free = TRUE;
  }
  return;
}

static void place_tile (int location, int type, int index)
{
  gint i;
  guint target;

  /* Set up the tile. */
  tiles[location].visible = TRUE;
  tiles[location].selected = FALSE;
  tiles[location].type = type_info[type].type;
  tiles[location].image = type_info[type].image[index] ;
  tiles[location].sequence = 0;
  
  /* Recalculate dependencies. */
  dependencies[location].filled = FALSE;
  dependencies[location].free = FALSE;

  /* See if we've freed anything above us. */
  for (i=0; i<4; i++) {
    target = dependencies[location].foundation[i];
    if (target == -1)
      break;
    check_tile_is_free (target);
  }

  /* See if we've freed anything to the left or right. */
  for (i=0; i<2; i++) {
    target = dependencies[location].left[i];
    if (target != -1)
      check_tile_is_free (target);
    target = dependencies[location].right[i];
    if (target != -1)
      check_tile_is_free (target);
  }
}

void generate_dependencies ()
{
  int i,j;
  int fc, lc, rc, oc;
  int x,y,l;
  int tx, ty, tl;
  dep_entry * dep;

  /* As well as generating the dependencies we should clear out the
   * type_info structure. */
  for (i=0; i<MAX_TILES/2; i++)
    type_info[i].placed = 0;
  
  dep = dependencies;
  for (i = 0; i<MAX_TILES; i++) {
    x = pos[i].x;
    y = pos[i].y;
    l = pos[i].layer;
    dep->filled = TRUE;
    
    dep->foundation[0] = dep->foundation[1] = dep->foundation[2]
      = dep->foundation[3] = -1;
    dep->left[0] = dep->left[1] = -1;
    dep->right[0] = dep->right[1] = -1;
    dep->overhead[0] = dep->overhead[1] = dep->overhead[2]
      = dep->overhead[3] = -1;
    fc = lc = rc = oc = 0;
    for (j = 0; j<MAX_TILES; j++) {
      ty = pos[j].y;
      
      /* Nothing we are interested in is outside +/- 1 units on the y axis. */
      if (abs(ty - y) > 1)
        continue;

      tl = pos[j].layer;
      tx = pos[j].x;
        
      /* First check if it is a foundation tile. */
      if ((tl == (l - 1)) && (abs(tx - x) < 2)) {
        dep->foundation[fc++] = j;
        continue;
      }

      /* Then do the overhead tiles. */
      if ((tl == (l + 1)) && (abs(tx - x) < 2)) {
        dep->overhead[oc++] = j;
        continue;
      }

      /* Everything else is on this layer. */
      if (tl != l)
        continue;
      
      /* Now look left ... */
      if (tx == (x - 2)) {
        dep->left[lc++] = j;
        continue;
      }

      /* ... and right. */
      if (tx == (x + 2)) {
        dep->right[rc++] = j;
        continue;
      }      
    }
    dep++;
  }

  /* Now mark free tiles for the generate_game routine. */
  for (i = 0; i<MAX_TILES; i++) {
    check_tile_is_free(i);
  }
}

static guint get_free_location (GRand * generator)
{
  int a,i,j;

  a = g_rand_int_range (generator, 0, MAX_TILES);
  j = 0;

  for (i=0; i<a; i++) {
    do {
      j = (j + 1) % MAX_TILES;
    } while (!dependencies[j].free);
  }
  
  return j;
}

void generate_game (guint32 seed)
{
  GRand * generator;
  int i, a, b, c, t;
  
  generator = g_rand_new_with_seed (seed);
  
  for (i=0; i<MAX_TILES/2; i++) {
    /* Find a random available pair of tiles. */
    c = g_rand_int_range (generator, 1, MAX_TILES/2 - i + 1);
    for (t=0; t<MAX_TILES/2; t++)
      if (type_info[t].placed == 0)
        if (!(--c)) break;

    type_info[t].placed = 1;

    /* Now find some places to put them. */
    a = get_free_location (generator);
    do {
      b = get_free_location (generator);
    } while (a == b);
      
    /* Finally we actually place them. Including the
     * dependency updates. */
    place_tile(a,t,0);
    place_tile(b,t,1);
  }
}

