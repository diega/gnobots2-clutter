/* gnome-stones - cave.c
 *
 * Time-stamp: <1998/08/23 12:30:22 carsten>
 *
 * Copyright (C) 1998 Carsten Schaar
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include "cave.h"
#include "player.h"
#include "status.h"


/*****************************************************************************/
/* Stone definitions */

extern ObjectType OBJECT_FRAME;
extern ObjectType OBJECT_EMPTY;
extern ObjectType OBJECT_BOULDER;
extern ObjectType OBJECT_WALL;
extern ObjectType OBJECT_GROWING_WALL;
extern ObjectType OBJECT_MAGIC_WALL;
extern ObjectType OBJECT_DIRT;
extern ObjectType OBJECT_DIAMOND;
extern ObjectType OBJECT_BUTTERFLY;
extern ObjectType OBJECT_FIREFLY;
extern ObjectType OBJECT_AMOEBA;
extern ObjectType OBJECT_EXPLOSION;
extern ObjectType OBJECT_GNOME;
extern ObjectType OBJECT_ENTRANCE;
extern ObjectType OBJECT_EXIT_CLOSED;
extern ObjectType OBJECT_EXIT_OPENED;


gint x_diff[4]={-1,  0,  1,  0};
gint y_diff[4]={ 0,  1,  0, -1};


/*****************************************************************************/
/* Object handling.  */


static GList *object_descriptions= NULL;

gboolean
object_register (GStonesObject *object)
{
  object_descriptions= g_list_append (object_descriptions, object);
  
  return TRUE;
}


const ObjectType
object_get_type (const gchar *name)
{
  GList *tmp;

  tmp= object_descriptions;
  while (tmp)
    {
      ObjectType desc= (ObjectType) tmp->data;
      
      if (strcmp (desc->name, name) == 0)
	return desc;
      
      tmp= tmp->next;
    }
  
  return NULL;
}



/*****************************************************************************/
/* Some declarations */


static void
explosion_new (GStonesCave *cave, guint x, guint y, gboolean diamond);


static void
cave_open_doors (GStonesCave *cave);



/*****************************************************************************/
/* some animation stuff.  */


static void
eight_animate (GStonesCave *cave, guint x, guint y, guint *ix, guint *iy,
	       gpointer data)
{
  *iy= (cave->frame/2) % 8;
}



/*****************************************************************************/
/* EMPTY stuff */


guint open_door_animation= 0;
guint extra_life_animation= 0;

static void
empty_pre_cave_scan (GStonesCave *cave, gpointer data)
{
  if (open_door_animation)
    open_door_animation--;

  if (extra_life_animation)
    extra_life_animation--;
}

static void
empty_animate (GStonesCave *cave, guint x, guint y, guint *ix, guint *iy,
	       gpointer data)
{
  *ix= 0;

  if (open_door_animation)
    *iy= open_door_animation;
  else if (extra_life_animation)
    *iy= 4+random () % 4;
  else
    *iy= 0;
}     

static GStonesObject empty_object=
{
  "empty",
  NULL,
  NULL,
  
  NULL,

  empty_pre_cave_scan,
  NULL,
  
  empty_animate,
  0, 0
};


/*****************************************************************************/
/* AMOEBA stuff */

typedef struct _AmoebaData AmoebaData;

struct _AmoebaData
{
  /* Static data, read from level file.  */
  guint max_size;
  guint slow_time;
  
  /* Dynamic data.  */
  guint size;
  gboolean can_grow;
  gboolean transformed;
};


static gpointer
amoeba_init_cave (GStonesCave *cave)
{
  AmoebaData *amoeba;
  
  amoeba= g_malloc (sizeof (AmoebaData));
  
  if (amoeba)
    {
      gnome_config_push_prefix (cave->config_prefix);
      
      amoeba->max_size = gnome_config_get_int ("Amoeba max size=0");
      amoeba->slow_time= cave_time_to_frames (cave, gnome_config_get_float 
					      ("Amoeba slow time=0"));
      
      amoeba->can_grow = TRUE;
      
      gnome_config_pop_prefix ();
    }
  
  return (gpointer) amoeba;
}


static void
amoeba_pre_cave_scan (GStonesCave *cave, gpointer data)
{
  AmoebaData *amoeba= (AmoebaData *) data;
  
  guint x;
  guint y;
  
  amoeba->transformed= !amoeba->can_grow;
  amoeba->size       = 0;
  amoeba->can_grow   = FALSE;
  if (amoeba->slow_time) amoeba->slow_time--;  

  for (y= 1 ; y <= CAVE_MAX_HEIGHT; y++)
    for (x= 1 ; x <= CAVE_MAX_WIDTH; x++)
      if (cave->entry[x][y].type == OBJECT_AMOEBA) amoeba->size++;
}


static void
amoeba_scanned (GStonesCave *cave, guint x, guint y, gpointer data)
{
  AmoebaData *amoeba= (AmoebaData *) data;
  guint m;
  
  /* This should not happen.  */
  if (!amoeba->size) return;

  /* This amoeba grew to big.  */
  if (amoeba->size >= amoeba->max_size)
    {
      cave->entry[x][y].type = OBJECT_BOULDER;
      cave->entry[x][y].state= 0;

      return;
    }

  /* Amoeba transformed to diamonds.  */
  if (amoeba->transformed)
    {
      cave->entry[x][y].type = OBJECT_DIAMOND;
      cave->entry[x][y].state= 0;
      
      return;
    }

  if (!amoeba->can_grow)
    for (m= 0; m < 4; m++)
      {
	ObjectType type= cave->entry[x+x_diff[m]][y+y_diff[m]].type;
	
	if ((type == OBJECT_EMPTY) || (type == OBJECT_DIRT))
	  amoeba->can_grow= TRUE;
      }
 
  
  /* Is this amoeba willing to grow?  */
  if (random () % (amoeba->slow_time ? 128 : 16) < 4)
    {
      ObjectType type;
      
      /* We randomly take one direction.  */
      m= random () % 4;
      
      type= cave->entry[x+x_diff[m]][y+y_diff[m]].type;
      
      if (type == OBJECT_EMPTY || type == OBJECT_DIRT)
	{
	  cave->entry[x+x_diff[m]][y+y_diff[m]].type   = OBJECT_AMOEBA;
	  cave->entry[x+x_diff[m]][y+y_diff[m]].scanned= TRUE;
	}
    }
}

static void
amoeba_animate (GStonesCave *cave, guint x, guint y, guint *ix, guint *iy,
		gpointer data)
{
  *ix= 8;
  if ((cave->frame % 6) < 4)
    *iy= cave->frame % 6;
  else
    *iy= 6-cave->frame % 6;
}


static GStonesObject amoeba_object=
{
  "amoeba",

  amoeba_init_cave,
  NULL,
 
  amoeba_scanned,
  
  amoeba_pre_cave_scan,
  NULL,
  
  amoeba_animate,
  0, 0
};


/*****************************************************************************/
/* GNOME stuff */


static gpointer
gnome_init_cave (GStonesCave *cave)
{
  guint x;
  guint y;

  for (y= 1 ; y <= CAVE_MAX_HEIGHT; y++)
    for (x= 1 ; x <= CAVE_MAX_WIDTH; x++)
      if (cave->entry[x][y].type == OBJECT_ENTRANCE)
	{
	  cave->player_x= x;
	  cave->player_y= y;
	}

  return (gpointer) TRUE;
}


static void
gnome_scanned (GStonesCave *cave, guint x, guint y, gpointer data)
{
  if ((cave->player_x_direction != 0) || 
      (cave->player_y_direction != 0))
    {
      gboolean moved= FALSE;
      
      guint xn= x+cave->player_x_direction;
      guint yn= y+cave->player_y_direction;
      
      ObjectType type= cave->entry[xn][yn].type;

      if (type == OBJECT_EMPTY || type == OBJECT_DIRT)
	{
	  moved= TRUE;
	}
      else if (type == OBJECT_DIAMOND)
	{
	  /* We only can collect diamonds, if they are not falling.  */
	  if (cave->entry[xn][yn].state == 0)
	    {
	      gboolean extra_life;
	      
	      cave->diamonds_collected++;
	      
	      if (cave->diamonds_collected <= cave->diamonds_needed)
		{
		  player_set_diamonds (cave->player, 
				       cave->diamonds_needed-
				       cave->diamonds_collected);

		  if (cave->diamonds_collected == cave->diamonds_needed)
		    cave_open_doors (cave);

		  extra_life= player_inc_score (cave->player, 
						cave->diamond_score);
		}
	      else
		{
		  extra_life= player_inc_score (cave->player,
					       cave->extra_diamond_score);
		}

	      if (extra_life)
		extra_life_animation= 10;	      	      

	      moved= TRUE;
	    }
	}
      else if (type == OBJECT_BOULDER)
	{
	  if ((cave->player_y_direction == 0) && 
	      (cave->entry[xn][yn].state == 0) &&
	      (cave->entry[xn+cave->player_x_direction][yn].type == OBJECT_EMPTY))
	    { 
	      if (random () % 8 == 0)
		{
		  moved= TRUE;
		  
		  cave->entry[xn+cave->player_x_direction][yn].type = type;
		  cave->entry[xn+cave->player_x_direction][yn].state= 0;	      
		}
	    }
	}
      else if (type == OBJECT_EXIT_OPENED)
	{
	  /* We finished this cave!!! */
	  moved       = TRUE;
	  cave->flags|= CAVE_FINISHED;
	}
      
      if (moved)
	{
	  if (cave->player_push)
	    {
	      cave->entry[xn][yn].type= OBJECT_EMPTY;
	    }
	  else
	    {
	      cave->entry[xn][yn].type   = OBJECT_GNOME;
	      cave->entry[xn][yn].scanned= TRUE;
	      cave->entry[x][y].type     = OBJECT_EMPTY;
	      
	      cave->player_x= xn;
	      cave->player_y= yn;
	    }
	}
    }
}


static GStonesObject gnome_object=
{
  "gnome",
  
  gnome_init_cave,
  NULL,

  gnome_scanned,

  NULL,
  NULL,
  
  NULL,
  10, 0
};



/*****************************************************************************/
/* GROWING WALL stuff */


static void
growing_wall_scanned (GStonesCave *cave, guint x, guint y, gpointer data)
{
  if (cave->entry[x-1][y].type == OBJECT_EMPTY)
    {
      cave->entry[x-1][y].type   = OBJECT_GROWING_WALL;
      cave->entry[x-1][y].scanned= TRUE;
    }
  
  if (cave->entry[x+1][y].type == OBJECT_EMPTY)
    {
      cave->entry[x+1][y].type   = OBJECT_GROWING_WALL;
      cave->entry[x+1][y].scanned= TRUE;
    }
}


static GStonesObject growing_wall_object=
{
  "growing wall",

  NULL,
  NULL,

  growing_wall_scanned,
  
  NULL,
  NULL,
  
  NULL,
  1, 0
};


/*****************************************************************************/
/* MAGIC WALL stuff */

typedef enum
{
  MAGIC_WALL_WAITING,
  MAGIC_WALL_MILLING,
  MAGIC_WALL_EXPIRED
} MagicWallStatus;


static guint magic_wall_time;
static MagicWallStatus magic_wall_status;


static gpointer
magic_wall_init_cave (GStonesCave *cave)
{
  gnome_config_push_prefix (cave->config_prefix);
  
  magic_wall_time  = cave_time_to_frames 
    (cave, gnome_config_get_float ("Magic wall time=0"));

  magic_wall_status= MAGIC_WALL_WAITING;
  
  gnome_config_pop_prefix ();

  return (gpointer) TRUE;
}


static void
magic_wall_pre_cave_scan (GStonesCave *cave, gpointer data)
{
  guint x;
  guint y;
  
  if (magic_wall_status == MAGIC_WALL_MILLING)
    {
      if (magic_wall_time)
	magic_wall_time--;
      else
	{
	  magic_wall_status= MAGIC_WALL_EXPIRED;
	  for (y= 1 ; y <= CAVE_MAX_HEIGHT; y++)
	    for (x= 1 ; x <= CAVE_MAX_WIDTH; x++)
	      if (cave->entry[x][y].type == OBJECT_MAGIC_WALL)
		cave->entry[x][y].state= 2;
	}
    }
}


static void
magic_wall_post_cave_scan (GStonesCave *cave, gpointer data)
{
  guint x;
  guint y;
  
  if (magic_wall_status == MAGIC_WALL_WAITING)
    {    
      for (y= 1 ; y <= CAVE_MAX_HEIGHT; y++)
	for (x= 1 ; x <= CAVE_MAX_WIDTH; x++)
	  if (cave->entry[x][y].type == OBJECT_MAGIC_WALL && cave->entry[x][y].state == 1)
	    magic_wall_status= MAGIC_WALL_MILLING;

      if (magic_wall_status == MAGIC_WALL_MILLING)
	for (y= 1 ; y <= CAVE_MAX_HEIGHT; y++)
	  for (x= 1 ; x <= CAVE_MAX_WIDTH; x++)
	    if (cave->entry[x][y].type == OBJECT_MAGIC_WALL)
	      cave->entry[x][y].state= 1;

    }
}


static void
magic_wall_animate (GStonesCave *cave, guint x, guint y, guint *ix, guint *iy,
		    gpointer data)
{
  *ix= 1;
  if (cave->entry[x][y].state == 1)
    *iy= 1+cave->frame % 4;
}


static GStonesObject magic_wall_object=
{
  "magic wall",
  
  magic_wall_init_cave,
  NULL,

  NULL,
  magic_wall_pre_cave_scan,
  magic_wall_post_cave_scan,
  
  magic_wall_animate,
  0, 0
};


/*****************************************************************************/
/* FIREFLY stuff */


static void
firefly_scanned (GStonesCave *cave, guint x, guint y, gpointer data)
{
  guint index[3]= {1, 0, 3};
  guint m;

  /* Let this firefly explode, if it hits something it doesn't like.  */
  for (m= 0; m < 4; m++)
    {
      ObjectType type= cave->entry[x+x_diff[m]][y+y_diff[m]].type;
      
      if (type == OBJECT_GNOME || type == OBJECT_AMOEBA)
	{
	  explosion_new (cave, x, y, FALSE);
	  return;
	}
    }
  
  for (m= 0; m < 2; m++)
    {
      guint state= (cave->entry[x][y].state+index[m]) % 4;
      guint xn= x+x_diff[state];
      guint yn= y+y_diff[state];
      
      if (cave->entry[xn][yn].type == OBJECT_EMPTY)
	{
	  cave->entry[x][y].type     = OBJECT_EMPTY;
	  cave->entry[xn][yn].type   = OBJECT_FIREFLY;
	  cave->entry[xn][yn].state  = state;
	  cave->entry[xn][yn].scanned= TRUE;
	  return;
	}
    }
  
  cave->entry[x][y].state= (cave->entry[x][y].state+index[2]) % 4;
}


static GStonesObject firefly_object=
{
  "firefly",
  
  NULL,
  NULL, 

  firefly_scanned,
  
  NULL,
  NULL,
  
  eight_animate,
  7, 0
};



/*****************************************************************************/
/* BUTTERFLY stuff */


static void
butterfly_scanned (GStonesCave *cave, guint x, guint y, gpointer data)
{
  guint index[3]= {3, 4, 5};
  guint m;

  /* Let this firefly explode, if it hits something it doesn't like.  */
  for (m= 0; m < 4; m++)
    {
      ObjectType type= cave->entry[x+x_diff[m]][y+y_diff[m]].type;
      
      if (type == OBJECT_GNOME || type == OBJECT_AMOEBA)
	{
	  explosion_new (cave, x, y, TRUE);
	  return;
	}
    }
  
  for (m= 0; m < 2; m++)
    {
      guint state= (cave->entry[x][y].state+index[m]) % 4;
      guint xn= x+x_diff[state];
      guint yn= y+y_diff[state];
      
      if (cave->entry[xn][yn].type == OBJECT_EMPTY)
	{
	  cave->entry[x][y].type     = OBJECT_EMPTY;
	  cave->entry[xn][yn].type   = OBJECT_BUTTERFLY;
	  cave->entry[xn][yn].state  = state;
	  cave->entry[xn][yn].scanned= TRUE;
	  return;
	}
    }
  
  cave->entry[x][y].state= (cave->entry[x][y].state+index[2]) % 4;
}


static GStonesObject butterfly_object=
{
  "butterfly",
  
  NULL,
  NULL,
  
  butterfly_scanned,
  
  NULL,
  NULL,
  
  eight_animate,
  6, 0
};



/*****************************************************************************/
/* BOULDER stuff */


static void
boulder_scanned (GStonesCave *cave, guint x, guint y, gpointer data)
{
  if (cave->entry[x][y+1].type == OBJECT_EMPTY)
    {
      cave->entry[x][y].type     = OBJECT_EMPTY;
      cave->entry[x][y+1].type   = OBJECT_BOULDER;
      cave->entry[x][y+1].state  = 1;
      cave->entry[x][y+1].scanned= TRUE;
    }
  else if ((cave->entry[x][y+1].type == OBJECT_WALL) ||
	   (((cave->entry[x][y+1].type == OBJECT_BOULDER) ||
	     (cave->entry[x][y+1].type == OBJECT_DIAMOND)) &&
	    cave->entry[x][y+1].state == 0))
	      {
		if (cave->entry[x+1][y].type   == OBJECT_EMPTY &&
		    cave->entry[x+1][y+1].type == OBJECT_EMPTY)
		  {
		    cave->entry[x][y].type   = OBJECT_EMPTY;
		    cave->entry[x+1][y].type = OBJECT_BOULDER;
		    cave->entry[x+1][y].state= 1;
		    cave->entry[x+1][y].scanned= TRUE;
		  }
		else if (cave->entry[x-1][y].type   == OBJECT_EMPTY &&
			 cave->entry[x-1][y+1].type == OBJECT_EMPTY)
		  {
		    cave->entry[x][y].type   = OBJECT_EMPTY;
		    cave->entry[x-1][y].type = OBJECT_BOULDER;
		    cave->entry[x-1][y].state= 1;
		    cave->entry[x-1][y].scanned= TRUE;
		  }
		else
		  cave->entry[x][y].state= 0;
	      }
  else if (cave->entry[x][y].state == 1)
    {
      ObjectType type= cave->entry[x][y+1].type;
      
      if ((type == OBJECT_BUTTERFLY) ||
	  (type == OBJECT_FIREFLY) ||
	  (type == OBJECT_GNOME))
	{
	  explosion_new (cave, x, y+1, type != OBJECT_FIREFLY);
	}
      else if (type == OBJECT_MAGIC_WALL)
	{
	  if (cave->entry[x][y+1].state < 2)
	    {
	      cave->entry[x][y+1].state= 1;
	      
	      if (cave->entry[x][y+2].type == OBJECT_EMPTY)
		{
		  cave->entry[x][y+2].type   = OBJECT_DIAMOND;
		  cave->entry[x][y+2].state  = 1;
		  cave->entry[x][y+2].scanned= TRUE;
		}
	    }
	  cave->entry[x][y].type= OBJECT_EMPTY;
	}
      else
	cave->entry[x][y].state= 0;
    }
  else
    cave->entry[x][y].state= 0;
}


static GStonesObject boulder_object=
{
  "boulder",

  NULL,
  NULL,

  boulder_scanned,
  
  NULL,
  NULL,
  
  NULL,
  3, 0
};



/*****************************************************************************/
/* DIAMOND stuff */


static void
diamond_scanned (GStonesCave *cave, guint x, guint y, gpointer data)
{
  if (cave->entry[x][y+1].type == OBJECT_EMPTY)
    {
      cave->entry[x][y].type     = OBJECT_EMPTY;
      cave->entry[x][y+1].type   = OBJECT_DIAMOND;
      cave->entry[x][y+1].state  = 1;
      cave->entry[x][y+1].scanned= TRUE;
    }
  else if ((cave->entry[x][y+1].type == OBJECT_WALL) ||
	   (((cave->entry[x][y+1].type == OBJECT_BOULDER) ||
	     (cave->entry[x][y+1].type == OBJECT_DIAMOND)) &&
	    cave->entry[x][y+1].state == 0))
	      {
		if (cave->entry[x+1][y].type   == OBJECT_EMPTY && 
		    cave->entry[x+1][y+1].type == OBJECT_EMPTY)
		  {
		    cave->entry[x][y].type   = OBJECT_EMPTY;
		    cave->entry[x+1][y].type = OBJECT_DIAMOND;
		    cave->entry[x+1][y].state= 1;
		    cave->entry[x+1][y].scanned= TRUE;
		  }
		else if (cave->entry[x-1][y].type   == OBJECT_EMPTY &&
			 cave->entry[x-1][y+1].type == OBJECT_EMPTY)
		  {
		    cave->entry[x][y].type   = OBJECT_EMPTY;
		    cave->entry[x-1][y].type = OBJECT_DIAMOND;
		    cave->entry[x-1][y].state= 1;
		    cave->entry[x-1][y].scanned= TRUE;
		  }
		else
		  cave->entry[x][y].state= 0;
	      }
  else if (cave->entry[x][y].state == 1)
    {
      ObjectType type= cave->entry[x][y+1].type;
      
      if ((type == OBJECT_BUTTERFLY) ||
	  (type == OBJECT_FIREFLY) ||
	  (type == OBJECT_GNOME))
	{
	  explosion_new (cave, x, y+1, type != OBJECT_FIREFLY);
	}
      else if (type == OBJECT_MAGIC_WALL)
	{
	  if (cave->entry[x][y+1].state < 2)
	    {
	      cave->entry[x][y+1].state= 1;
	      
	      if (cave->entry[x][y+2].type == OBJECT_EMPTY)
		{
		  cave->entry[x][y+2].type   = OBJECT_BOULDER;
		  cave->entry[x][y+2].state  = 1;
		  cave->entry[x][y+2].scanned= TRUE;
		}
	    }
	  cave->entry[x][y].type= OBJECT_EMPTY;
	}
      else
	cave->entry[x][y].state= 0;
    }
  else
    cave->entry[x][y].state= 0;
}


static GStonesObject diamond_object=
{
  "diamond",

  NULL, 
  NULL,

  diamond_scanned,
  
  NULL,
  NULL,
  
  eight_animate,
  5, 0
};



/*****************************************************************************/
/* EXPLOSION stuff */

typedef struct _ExplosionState ExplosionState;

struct _ExplosionState
{
  guint state  : 2;
  guint diamond: 1;
};


static void
explosion_scanned (GStonesCave *cave, guint x, guint y, gpointer data)
{
  ExplosionState state= *((ExplosionState*) &cave->entry[x][y].state);
    
  if (state.state == 2)
    {
      /* The third bit indicates, wheater the explosion will explode
         to a diamond or to nothing.  */
      if (state.diamond)
	cave->entry[x][y].type= OBJECT_DIAMOND;
      else
	cave->entry[x][y].type= OBJECT_EMPTY;
      
      cave->entry[x][y].state= 0;
      cave->entry[x][y].scanned= TRUE;
    }
  else
    {
      state.state++;
      cave->entry[x][y].state= *((guint*)&state);
    }
}


static void
explosion_new (GStonesCave *cave, guint x, guint y, gboolean diamond)
{
  guint m;
  gint x_diff[9]={-1, -1, -1,  0,  0,  0,  1,  1,  1};
  gint y_diff[9]={-1,  0,  1, -1,  0,  1, -1,  0,  1};

  for (m= 0 ; m < 9 ; m++)
    {
      ExplosionState state;
      guint xn= x+x_diff[m];
      guint yn= y+y_diff[m];

      ObjectType type= cave->entry[xn][yn].type;
      
      if (type != OBJECT_FRAME)
	{
	  if (type == OBJECT_GNOME && !(cave->flags & CAVE_FINISHED))
	    {
	      /* Unfortunetely our little gnome died ;-(  */
	      cave->flags|= CAVE_FINISHED;
	      cave->flags&= ~CAVE_PLAYER_EXISTS;
	    }

	  state.state  = 0;
	  state.diamond= diamond;

	  cave->entry[xn][yn].type   = OBJECT_EXPLOSION;
	  cave->entry[xn][yn].state  = *((guint*)&state);
	  cave->entry[xn][yn].scanned= TRUE;
	}
      
    }
}


static void
explosion_animate (GStonesCave *cave, guint x, guint y, guint *ix, guint *iy,
		   gpointer data)
{
  ExplosionState state= *((ExplosionState*) &cave->entry[x][y].state);
    
  *ix= 9;
  *iy= state.state;
}


static GStonesObject explosion_object=
{
  "explosion",
  
  NULL,
  NULL,

  explosion_scanned,
  
  NULL,
  NULL,
  
  explosion_animate,
  9, 0
};



/*****************************************************************************/
/* ENTRANCE stuff */


static void
entrance_scanned (GStonesCave *cave, guint x, guint y, gpointer data)
{
  if (cave->entry[x][y].state > 0)
    {
      if (cave->entry[x][y].state == 3)
	{
	  cave->entry[x][y].type= OBJECT_GNOME;
	}
      else
	cave->entry[x][y].state++;
    }
}

static void
entrance_animate (GStonesCave *cave, guint x, guint y, guint *ix, guint *iy, 
		  gpointer data)
{
  if (cave->entry[x][y].state == 0)
    {
      *ix= 4;
      *iy= (cave->frame % 4)/2;
    }
  else
    {
      *ix= 9;
      *iy= cave->entry[x][y].state-1;
    }
}


static GStonesObject entrance_object=
{
  "entrance",

  NULL,
  NULL,

  entrance_scanned,
  
  NULL,
  NULL,
  
  entrance_animate,
  0, 0
};



/*****************************************************************************/


static GStonesObject frame_object=
{
  "frame",

  NULL,
  NULL,
  
  NULL,

  NULL,
  NULL,
  
  NULL,
  4, 0
};


static GStonesObject wall_object=
{
  "wall",

  NULL,
  NULL,
  
  NULL,

  NULL,
  NULL,
  
  NULL,
  1, 0
};


static GStonesObject dirt_object=
{
  "dirt",
  
  NULL,
  NULL,
  
  NULL,

  NULL,
  NULL,
  
  NULL,
  2, 0
};


static GStonesObject closed_exit_object=
{
  "closed exit",

  NULL,
  NULL,
  
  NULL,

  NULL,
  NULL,
  
  NULL,
  4, 0
};


static GStonesObject opened_exit_object=
{
  "opened exit",

  NULL,
  NULL,
  
  NULL,

  NULL,
  NULL,
  
  entrance_animate,
  0, 0
};



/*****************************************************************************/
/* Register all objects.  */

gboolean
objects_register_all (void)
{
  object_register (&empty_object);
  object_register (&frame_object);
  object_register (&wall_object);
  object_register (&dirt_object);
  object_register (&amoeba_object);
  object_register (&gnome_object);
  object_register (&growing_wall_object);
  object_register (&magic_wall_object);
  object_register (&firefly_object);
  object_register (&butterfly_object);
  object_register (&boulder_object);
  object_register (&diamond_object);
  object_register (&explosion_object);
  object_register (&entrance_object);
  object_register (&closed_exit_object);

  return TRUE;
}

ObjectType OBJECT_FRAME       = &frame_object;
ObjectType OBJECT_EMPTY       = &empty_object;
ObjectType OBJECT_BOULDER     = &boulder_object;
ObjectType OBJECT_WALL        = &wall_object;
ObjectType OBJECT_GROWING_WALL= &growing_wall_object;
ObjectType OBJECT_MAGIC_WALL  = &magic_wall_object;
ObjectType OBJECT_DIRT        = &dirt_object;
ObjectType OBJECT_DIAMOND     = &diamond_object;
ObjectType OBJECT_BUTTERFLY   = &butterfly_object;
ObjectType OBJECT_FIREFLY     = &firefly_object;
ObjectType OBJECT_AMOEBA      = &amoeba_object;
ObjectType OBJECT_EXPLOSION   = &explosion_object;
ObjectType OBJECT_GNOME       = &gnome_object;
ObjectType OBJECT_ENTRANCE    = &entrance_object;
ObjectType OBJECT_EXIT_CLOSED = &closed_exit_object;
ObjectType OBJECT_EXIT_OPENED = &opened_exit_object;



/*****************************************************************************/

GStonesCave *
cave_new (void)
{
  GStonesCave *cave= NULL;
  guint x;
  guint y;
  
  cave= g_malloc (sizeof (GStonesCave));
  g_return_val_if_fail (cave, NULL);

  cave->name               = NULL;
  cave->next               = NULL;

  cave->width              = 0;
  cave->height             = 0;
  cave->is_intermission    = FALSE;
  cave->diamond_score      = 0;
  cave->extra_diamond_score= 0;
  cave->level_time         = 0;
  cave->message            = NULL;
  cave->player             = NULL;

  cave->frame_rate         = 0;
  
  cave->flags              = 0;
  cave->timer              = 0;
  cave->frame              = 0;
  cave->diamonds_collected = 0;

  cave->object_data        = g_hash_table_new (g_direct_hash, g_direct_equal);

  /* First we clear the hole cave.  */
  for (y= 0 ; y <= CAVE_MAX_HEIGHT+1; y++)
    for (x= 0 ; x <= CAVE_MAX_WIDTH+1; x++)
      {
	cave->entry[x][y].type   = OBJECT_FRAME;
	cave->entry[x][y].scanned= FALSE;
      }

  return cave;
}


void
cave_free (GStonesCave *cave)
{
  if (cave)
    {
      g_free (cave->name);
      g_free (cave->next);
      
      g_free (cave->message);
      
      /* FIXME: object_data must be freed.  */
      
      g_free (cave);
    }
}


/*****************************************************************************/


GStonesCave *
cave_load (const GStonesGame *game, const gchar *cavename)
{
  GStonesCave *cave= NULL;
  GList       *tmp;
  guint        x;
  guint        y;

  g_return_val_if_fail (game,  NULL);

  if (cavename == NULL) 
    return NULL;
  
  cave= cave_new ();
  g_return_val_if_fail (cave, NULL);

  cave->name         = g_strdup (cavename);
  cave->config_prefix= g_copy_strings (game->config_prefix, 
				       cavename, "/", NULL);

  gnome_config_push_prefix (cave->config_prefix);

  /* Now we load the cave data.  */
  cave->next               = gnome_config_get_string ("Next cave=");
  if (cave->next && (strlen (cave->next) == 0))
    {
      g_free (cave->next);
      cave->next= NULL;
    }
  cave->width              = gnome_config_get_int ("Width");
  cave->height             = gnome_config_get_int ("Height");
  cave->is_intermission    = gnome_config_get_bool ("Is intermission=false");
  cave->diamond_score      = gnome_config_get_int ("Diamond score=0");
  cave->extra_diamond_score= gnome_config_get_int ("Extra diamond score=0");
  cave->diamonds_needed    = gnome_config_get_int ("Diamonds needed");
  cave->level_time         = gnome_config_get_int ("Time")*1000;
  cave->message            = gnome_config_get_translated_string ("Message");
  if (cave->message && (strlen (cave->message) == 0))
    {
      g_free (cave->message);
      cave->message= NULL;
    }

  cave->frame_rate         = game->frame_rate;

  cave->timer              = gnome_config_get_int ("Time")*1000;

  if ((cave->width > CAVE_MAX_WIDTH) || (cave->height > CAVE_MAX_HEIGHT))
    {
      /* This cave is to big to be played with gnome-stones.  */

      gstone_error (_("The cave you are trying to load it to big for this game."));
      
      gnome_config_pop_prefix ();
      cave_free (cave);
      return NULL;
    }

  /* Now we set the fields according to the loaded cave */
  for (y= 1 ; y <= cave->height; y++)
    {
      char *line;
      char  buffer[8];
      sprintf (buffer, "Line%.2d", y);

      line= gnome_config_get_string (buffer);
      
      for (x= 1 ; x <= MIN (strlen (line), cave->width); x++)
	{
	  /* FIXME: may have problems, if char is signed char?  */
	  guint c= (guint) line[x-1];
	  
	  if (game->translation_table[c].type)
	    {
	      cave->entry[x][y].type = game->translation_table[c].type;
	      cave->entry[x][y].state= game->translation_table[c].state;
	    }
	  else
	    {
	      /* An object was requested in this cave, that was not
		 declared in the game file's Object section.  */
	      
	      gstone_error (_("The cave you are trying to load includes an object, that wasn't declared."));
	      
	      gnome_config_pop_prefix ();
	      cave_free (cave);
	      g_free (line);
	      return NULL;
	    }
	}
      g_free (line);
    }

  gnome_config_pop_prefix ();

  /* Finally we call all the init cave functions.  */
  tmp= object_descriptions;
  while (tmp)
    {
      GStonesObject *objc= (GStonesObject *) tmp->data;
      gpointer       data= (gpointer) TRUE;
      
      if (objc->init_cave_function)
	data= objc->init_cave_function (cave);

      g_hash_table_insert (cave->object_data, objc, data);

      tmp= tmp->next;
    };

  return cave;
}


void
cave_set_player (GStonesCave *cave, GStonesPlayer *player)
{
  g_return_if_fail (cave);
  
  cave->player= player;

  player_set_diamonds (cave->player, MAX (cave->diamonds_needed-
					  cave->diamonds_collected, 0));

  player_set_max_time (cave->player, cave->level_time);
  player_set_time     (cave->player, cave->timer, TRUE);
}


/* The following function opens all the doors in the current cave. */


static void
cave_open_doors (GStonesCave *cave)
{
  guint x;
  guint y;
  
  for (y= 1 ; y <= CAVE_MAX_HEIGHT; y++)
    for (x= 1 ; x <= CAVE_MAX_WIDTH; x++)
      if (cave->entry[x][y].type == OBJECT_EXIT_CLOSED)
	cave->entry[x][y].type= OBJECT_EXIT_OPENED;

  open_door_animation= 3;
}



/*****************************************************************************/

void
cave_player_die (GStonesCave *cave)
{
  if (cave->flags & CAVE_PLAYER_EXISTS)
    {
      explosion_new (cave, cave->player_x, cave->player_y, TRUE);
      cave->flags&= ~CAVE_PLAYER_EXISTS;
      cave->flags|= CAVE_FINISHED;
    }
}


void
cave_iterate (GStonesCave *cave,
	      gint         x_direction,
	      gint         y_direction,
	      gboolean     push)
{
  GList *tmp;
  guint x;
  guint y;

  g_return_if_fail (cave != NULL);
  
  cave->frame++;

  if (cave->flags & CAVE_PAUSING)
    return;

  /* Decrement timer.  */
  if ((cave->timer > 0) && (cave->flags & CAVE_PLAYER_EXISTS))
    {
      cave->timer= cave->timer-cave->frame_rate;
      if (cave->timer <= 0)
	{
	  cave_player_die (cave);
	  cave->timer= 0;
	}

      player_set_time (cave->player, cave->timer, TRUE);
    }
  
  cave->player_x_direction= x_direction;
  cave->player_y_direction= y_direction;
  cave->player_push       = push;

  /* We have to call all pre scan functions.  */
  tmp= object_descriptions;
  while (tmp)
    {
      GStonesObject *objc= (GStonesObject *) tmp->data;
      
      if (objc->pre_cave_scan_function)
	objc->pre_cave_scan_function (cave, 
				      g_hash_table_lookup (cave->object_data,
							   objc));

      tmp= tmp->next;
    };

  /* Now we iterate through the hole cave.  */
  for (y= 1 ; y <= CAVE_MAX_HEIGHT; y++)
    for (x= CAVE_MAX_WIDTH ; x >= 1; x--)
      if (!cave->entry[x][y].scanned && 
	  (cave->entry[x][y].type->scan_function))
	cave->entry[x][y].type->scan_function (cave, x, y,
					       g_hash_table_lookup 
					       (cave->object_data,
						cave->entry[x][y].type));
  
  /* Remove the scanned flag on every entry.  */
  for (y= 1 ; y <= CAVE_MAX_HEIGHT; y++)
    for (x= 1 ; x <= CAVE_MAX_WIDTH; x++)
      cave->entry[x][y].scanned= FALSE;

  /* ... and we call all the post scan functions.  */
  tmp= object_descriptions;
  while (tmp)
    {
      GStonesObject *objc= (GStonesObject *) tmp->data;
      
      if (objc->post_cave_scan_function)
	objc->post_cave_scan_function (cave, 
				       g_hash_table_lookup (cave->object_data,
							    objc));

      tmp= tmp->next;
    };
}

void
cave_toggle_pause_mode (GStonesCave *cave)
{
  cave->flags^= CAVE_PAUSING;
}



/* The following function starts the cave by replacing the entrance
   with our gnome.  */

void
cave_start (GStonesCave *cave)
{
  /* We should only start, if the cave it still in the intro state,
     that means: ~PLAYER_EXISTS and ~FINISHED.  */

  if (!(cave->flags & CAVE_PLAYER_EXISTS) && !(cave->flags & CAVE_FINISHED))
    {
      /* Replace the entrance with our gnome.  */
      /* cave[player_x][player_y].type = OBJECT_GNOME; */
      cave->entry[cave->player_x][cave->player_y].state= 1;

      cave->flags|= CAVE_PLAYER_EXISTS;
    }
}

guint
cave_time_to_frames (GStonesCave *cave, gdouble time)
{
  return (time*1000.0)/cave->frame_rate;
}


void
cave_get_image_index (GStonesCave *cave, guint x, guint y,
		      guint *ix, guint *iy)
{
  ObjectType type;
  
  g_return_if_fail (cave != NULL);
  g_return_if_fail (ix != NULL);
  g_return_if_fail (iy != NULL);
  
  type= cave->entry[x][y].type;

  *ix=type->ix;
  *iy=type->iy;

  if (type->animation_function)
    type->animation_function (cave, x, y, ix, iy, 
			      g_hash_table_lookup (cave->object_data, type));
}



/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
