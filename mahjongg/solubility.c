/*
 * Gnome-Mahjonggg-solubility algorithem
 * (C) 1998 the Free Software Foundation
 *
 *
 * Author: Michael Meeks.
 *
 *
 * http://www.imaginator.com/~michael
 * michael@imaginator.com
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <setjmp.h>

#include "mahjongg.h"
#include "solubility.h"

jmp_buf unsolvable ;

#ifdef PLACE_DEBUG
int global_wait = 0 ;
#endif

struct _dep_tree {
  int turn_dep[4] ;	// Turning dependancies all must be clear 
  int place_dep[4] ;	// Placing dependancies all must be full
  int lhs_dep[2] ;	// Sideways dependancies on this level
  int rhs_dep[2] ;	// N.B. Repeats are allowed.
  int free ;		// Can we put a tile in it ?
  int filled ;		// Does it have a tile in it ?
};
struct _dep_tree dep_tree[MAX_TILES] ;
     
struct _sensible_types {
  int def[2] ;
  int placed ;
} ;
struct _sensible_types sensible_types[(MAX_TILES/2)] ;

/* N.B. The +1.0 _is_ intended ! */
#define random_UDL ((int)( (((float)MAX_TILES) * rand())/(1.0 + RAND_MAX)))
#define random_UDL_half ((int)( (((float)(MAX_TILES/2)) * rand())/(1.0 + RAND_MAX)))
#define RANDOM_LIMIT (MAX_TILES*MAX_TILES)

#ifdef PLACE_DEBUG
void dump_deps (int lp)
{
  printf ("Tile %d : Turn : ", lp) ;
  printf ("%d %d %d %d ", dep_tree[lp].turn_dep[0], dep_tree[lp].turn_dep[1], dep_tree[lp].turn_dep[2], dep_tree[lp].turn_dep[3]) ;
  printf ("Place : ") ;
  printf ("%d %d %d %d ", dep_tree[lp].place_dep[0], dep_tree[lp].place_dep[1], dep_tree[lp].place_dep[2], dep_tree[lp].place_dep[3]) ;
  printf ("Lhs %d %d ", dep_tree[lp].lhs_dep[0], dep_tree[lp].lhs_dep[1]) ;
  printf ("Rhs %d %d\n ", dep_tree[lp].rhs_dep[0], dep_tree[lp].rhs_dep[1]) ;
  printf ("Free %d Filled %d ", dep_tree[lp].free, dep_tree[lp].filled) ;  
  printf ("\n") ;
}

void dump_tile (int t)
{
  printf ("Tile %d at (%d, %d), layer %d free %d filled %d\n", t, default_pos[t].x, default_pos[t].y, default_pos[t].layer, dep_tree[t].free, dep_tree[t].filled) ;
}
#endif

/* Takes the last tile places, and avoids its dependancies */
int random_free (int lt)
{
  int t, lp, catch ;
  int goodplace ;
  
  catch = 0 ;
  do
    {
      do
	{
	  t = random_UDL ;
	  if (catch++>RANDOM_LIMIT)
	    longjmp (unsolvable,1) ;
	}
      while (!dep_tree[t].free) ;

      assert (!dep_tree[t].filled) ;

      goodplace = 1 ;
      if (lt!=-1)
	{
	  for (lp=0;lp<4;lp++)
	    if (dep_tree[t].place_dep[lp] == lt)
	      goodplace = 0 ;
	  for (lp=0;lp<2;lp++)
	    {
	      if ((dep_tree[t].lhs_dep[lp] == lt) ||
		  (dep_tree[t].rhs_dep[lp] == lt))
		goodplace = 0 ;
	    }
	}
    }
  while (!goodplace) ;
  assert (!dep_tree[t].filled) ;
  return t ;
}

/* Finds a random block on a layer */
int random_on_layer (int l)
{
  int t, cnt = 0 ;
  do
    {
      t = random_UDL ;
      if (cnt++>RANDOM_LIMIT)
	longjmp (unsolvable,1) ;
    }
  while (default_pos[t].layer!=l) ;

  assert (!dep_tree[t].filled) ;
  return t ;
}

/* This determines whether it is OK to mark this tile as free */
int ok_free_validate_line (int t)
{
  int valid = 1, i, lp, w ;
  int in, out ;
  int next[MAX_TILES] ;

  /* Go left */
  in = 0 ;
  out = 0 ;
  next[in++] = t ;
  do
    {
      w = next[out++] ;
      if (dep_tree[w].free || dep_tree[w].filled)
	return 0 ;
      if ((i=dep_tree[w].lhs_dep[0])!=-1)
	next[in++] = i ;
      if ((i=dep_tree[w].lhs_dep[1])!=-1)
	next[in++] = i ;
    }
  while (in>out) ;

#ifdef PLACE_DEBUG
  printf ("%d's lhs line : ", t) ;
  for (lp=0;lp<in;lp++)
    printf ("%d ", next[lp]) ;
  printf ("\n") ;
#endif

  /* Go right */
  in = 0 ;
  out = 0 ;
  next[in++] = t ;
  do
    {
      w = next[out++] ;
      if (dep_tree[w].free || dep_tree[w].filled)
	return 0 ;
      if ((i=dep_tree[w].rhs_dep[0])!=-1)
	next[in++] = i ;
      if ((i=dep_tree[w].rhs_dep[1])!=-1)
	next[in++] = i ;
    }
  while (in>out) ;

#ifdef PLACE_DEBUG
  printf ("%d's rhs line : ", t) ;
  for (lp=0;lp<in;lp++)
    printf ("%d ", next[lp]) ;
  printf ("\n") ;
#endif
  
  return 1 ;
}

/* This simple examines whether the tile should be free or not.
   It does this by examining its dependancies to see if they are filled */
void validate_tile (int t)
{
  int valid, lfilled, rfilled, lp, free, i, iv ;

  if (t==-1||dep_tree[t].filled)
    return ;	// No point.

#ifdef PLACE_DEBUG
  printf ("Validating : %d\n", t) ;
  dump_deps(t) ;
#endif

  free = dep_tree[t].free ;

  valid = 1 ;
  /* Check below */
  for (lp=0;lp<4;lp++)
    if ((i = dep_tree[t].place_dep[lp]) != -1)
      if (!dep_tree[i].filled) return ;

  if (ok_free_validate_line(t))	// First in this layer on this line
    {
      assert (default_pos[t].layer!=0) ;
      dep_tree[t].free = 1 ;
      return ;
    }

  /* LHS */
  if ((dep_tree[t].lhs_dep[0] == -1) &&
      (dep_tree[t].lhs_dep[1] == -1))
    lfilled = 0 ;
  else
    {
      lfilled = 1 ;      
      for (lp=0;lp<2;lp++)
	if ((i = dep_tree[t].lhs_dep[lp]) != -1)
	  if (!dep_tree[i].filled) lfilled = 0 ;
    }
  
  /* RHS */
  if ((dep_tree[t].rhs_dep[0] == -1) &&
      (dep_tree[t].rhs_dep[1] == -1))
    rfilled = 0 ;
  else
    {
      rfilled = 1 ;      
      for (lp=0;lp<2;lp++)
	if ((i = dep_tree[t].rhs_dep[lp]) != -1)
	  if (!dep_tree[i].filled) rfilled = 0 ;
    }

#ifdef PLACE_DEBUG
  printf ("L %d, R %d\n", lfilled, rfilled) ;
#endif
  if ((lfilled) || (rfilled))
    dep_tree[t].free = 1 ;
}

/* Place tile in map at position f, with pic & type from t */
void place_tile (int f, int t)
{
#ifdef PLACE_DEBUG
  printf ("Placing at\n") ;
  dump_tile(f) ;
#endif
  tiles[f].visible = 1;
  tiles[f].selected = 0;
  tiles[f].x = default_pos[f].x * (HALF_WIDTH-0) + 30 + (5 * default_pos[f].layer);
  tiles[f].y = default_pos[f].y * (HALF_HEIGHT-0) + 25 - (4 * default_pos[f].layer);
  tiles[f].layer = default_pos[f].layer;
  //  tiles[f].sequence = 0 ;

  tiles[f].type = default_types[t].type; 
  tiles[f].image = default_types[t].image;

  assert (dep_tree[f].free) ;
  assert (!dep_tree[f].filled) ;
  dep_tree[f].filled = 1 ; // Lord
  dep_tree[f].free = 0 ;

  /* Now let the tiles near this one see if this makes them 'free' */
  /* N.B. You shall know the truth and the truth will set you free */
  {
    int lp, i ;
    for (lp=0;lp<4;lp++)
      if ((i=dep_tree[f].turn_dep[lp])!=-1)
	validate_tile(i) ;
    for (lp=0;lp<2;lp++)
      {
	if ((i=dep_tree[f].lhs_dep[lp])!=-1)
	  validate_tile(i) ;
	if ((i=dep_tree[f].rhs_dep[lp])!=-1)
	  validate_tile(i) ;
      }
  }
}

int tile_free (int tile_num)
{
  int lp, valid, t, lfree, rfree ;

  if (tile_num>=MAX_TILES)
    return 0 ;

  if (tiles[tile_num].visible == 0)
    return 0;

#ifdef PLACE_DEBUG
  printf ("Check %d, deps:\n", tile_num) ;
  dump_deps (tile_num) ;
#endif
  valid = 1 ;
  /* Check above */
  for (lp=0;lp<4;lp++)
    if ((t = dep_tree[tile_num].turn_dep[lp]) != -1)
      if (tiles[t].visible) valid = 0 ;
#ifdef PLACE_DEBUG
  printf ("Valid : %d ", valid) ;
#endif
  if (valid==0)
    return 0 ;
  
  lfree = 1 ;
  /* LHS */
  for (lp=0;lp<2;lp++)
    if ((t = dep_tree[tile_num].lhs_dep[lp]) != -1)
      if (tiles[t].visible) lfree = 0 ;
  rfree = 1 ;
  /* LHS */
  for (lp=0;lp<2;lp++)
    if ((t = dep_tree[tile_num].rhs_dep[lp]) != -1)
      if (tiles[t].visible) rfree = 0 ;
#ifdef PLACE_DEBUG
  printf ("L %d, R %d\n", lfree, rfree) ;
#endif
  if ((lfree) || (rfree)) return 1;
  else return 0 ;
}

int random_tile_type ()
{
  int o ;
  do
    o = random_UDL_half ;
  while (sensible_types[o].placed) ;
  return o ;
}

/* This finds the offset into default_pos of a tile given its x,y and layer
   it returns -1 if there is no tile at that position.
   N.B. Each tile occupies 2x2 positions.
 */
int tile_at (int layer, int x, int y)
{
  int lp ;

  for (lp=0;lp<MAX_TILES;lp++)
    if ((default_pos[lp].x==x || default_pos[lp].x==x+1) &&
	(default_pos[lp].y==y || default_pos[lp].y==y+1) &&
	(default_pos[lp].layer == layer))
      return lp ;
  return -1 ;
}

/* Remove duplicate entries */
void unique (int *a, int l)
{
  int lp, sc ;
  if (l==2)
    {
      if (a[1] == a[0])
	a[1] = -1 ;
      return ;
    }
  for (lp=0;lp<l-1;lp++)
    {
      for (sc=lp+1;sc<l;sc++)
	{
	  assert (lp!=sc) ;
	  assert (sc<l) ;
	  assert (lp<l) ;
	  if (a[lp] == a[sc])
	    a[sc] = -1 ;
	}
    }
}

/* Generate a single tiles dependancy tree entries */
/* N.B. Layer increases as you go up */
void tile_deps (int tn)
{
  int lp, layer, x, y, lpx, lpy ;

  // Side dependancies
  x = default_pos[tn].x ;
  y = default_pos[tn].y ;
  layer = default_pos[tn].layer ;
  
  dep_tree[tn].lhs_dep[0] = tile_at(layer, x-2, y) ;
  dep_tree[tn].lhs_dep[1] = tile_at(layer, x-2, y-1) ;

  dep_tree[tn].rhs_dep[0] = tile_at(layer, x+2, y) ;
  dep_tree[tn].rhs_dep[1] = tile_at(layer, x+2, y-1) ;
  
  // Place / take dependancies
  lp=0 ;
  for (lpx=0;lpx<=1;lpx++)	
    for (lpy=0;lpy<=1;lpy++)
      {
	dep_tree[tn].turn_dep[lp] = tile_at(layer+1, x-lpx, y-lpy) ;
	dep_tree[tn].place_dep[lp] = tile_at(layer-1, x-lpx, y-lpy) ;
	lp++ ;
      }
  dep_tree[tn].filled = 0 ; // Lord
  dep_tree[tn].free = 0 ;

  unique(dep_tree[tn].turn_dep, 4) ;
  unique(dep_tree[tn].place_dep, 4) ;
  unique(dep_tree[tn].lhs_dep, 2) ;
  unique(dep_tree[tn].rhs_dep, 2) ;
}

/* This calculates the tree of up and down dependancies that is used
   to determine which tiles can be turned / how to place the tiles */
void generate_dependancies ()
{
  int lp, pr, num, tile ;

  for (lp=0;lp<MAX_TILES;lp++)
    {
      tile_deps(lp) ;	/* Do the thrunging */
#ifdef PLACE_DEBUG
      dump_deps(lp) ;
#endif
    }

  /* Build a nice list of pairs of tiles */
  tile = 0 ;
  num = 0 ;
  for (lp=0;lp<MAX_TILES;lp++)
    {
      sensible_types[lp/2].def[(lp&1)] = tile ;
      num++;
      if (num>=default_types[tile].num_tiles)
	{
	  num = 0 ;
	  tile++ ;
	}
    }
}

/* Do the tile placement for a soluable game */
void generate_game (void)
{
  static int fails = 0 ;
  static int generation = 0 ;
  int i, lp, n;
  
  /* If this bites please mail me as above */
  for (lp=0;lp<MAX_TILES*MAX_TILES;lp++)
    {
      i = random_UDL ;
      assert (i>=0) ;
      assert (i<MAX_TILES) ;
    }
  
  if ((i=setjmp(unsolvable))==1)
    {
      fails++ ;
      printf ("Warning - impossible seed\n") ;
      printf ("%d fails in %d generations\n", fails, generation) ;
    }
  generation++ ;
  
  for (lp = 0; lp < MAX_TILES; lp++)
    {
      tiles[lp].visible = 0;
      dep_tree[lp].free = 0 ;
      dep_tree[lp].filled = 0 ; // Lord
      sensible_types[lp/2].placed = 0 ;
    }
  
  /* OK ! we want only one tile per apparent horizontal line ! */
  for (lp=0;lp<MAX_TILES;lp++)
    {
      int t = random_on_layer(0) ;
      if (ok_free_validate_line (t))
	dep_tree[t].free = 1 ;
    }
#ifdef PLACE_DEBUG
  printf ("Done\n") ;
  
  for (lp=0;lp<MAX_TILES;lp++)
    dump_tile(lp) ;
  printf ("Re-jig filled flags\n") ;
  clear_window() ;
#endif
  
  {
    int last_place, tileo, tile, offset ;
    
    /* Insert */
    for (lp=0;lp<MAX_TILES;lp++)
      {
	/* Find somewhere to put it */
	if ((lp&0x1)==0)	// Every other tile depends on the last
	  {
	    tileo = random_tile_type() ;
	    sensible_types[tileo].placed = 1 ;
	    last_place = -1 ;
#ifdef PLACE_DEBUG
	    if (global_wait)
	      {
		int zlp, b ;
		clear_window() ;
		for (zlp=0;zlp<MAX_TILES;zlp++)
		  redraw_tile(zlp) ;
		for (zlp=0;zlp<MAX_TILES;zlp++)
		  tiles[zlp].selected = 0 ;
		while (fread(&b, 1, 1, stdin)==0) ;
	      }
#endif
	  }
	tile = sensible_types[tileo].def[(lp&1)] ;
	offset = random_free (last_place) ;
	place_tile (offset, tile) ;
	
#ifdef PLACE_DEBUG
	printf ("lp %d, lp&01 %d tileo %d, tile %d\n", lp, (lp&0x1), tileo, tile) ;
	printf ("Place %d at : %d\n", tile, offset) ;
	tiles[offset].selected = 1 ;
#endif
	
	tiles[offset].sequence = (MAX_TILES/2) - (int)(lp/2) ;
	last_place = offset ;
      }
  }
#ifdef PLACE_DEBUG
  printf ("Finished : Draw !\n") ;
  global_wait = 1 ;
#endif
}
