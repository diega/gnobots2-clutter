/* gnome-stones - types.h
 *
 * Time-stamp: <1998/09/07 22:29:15 carsten>
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
#ifndef TYPES_H
#define TYPES_H

#include <config.h>
#include <gnome.h>



#define STONE_SIZE 32


typedef struct _GStonesGame      GStonesGame;
typedef struct _GStonesCaveEntry GStonesCaveEntry;
typedef struct _GStonesCave      GStonesCave;
typedef struct _GStonesPlayer    GStonesPlayer;
typedef struct _GStonesObject    GStonesObject;
typedef struct _TranslationEntry TranslationEntry;



/*****************************************************************************/
/* The following structure describes a game object.  */

typedef GStonesObject *ObjectType;

typedef gpointer (*InitCaveFunction)  (GStonesCave *cave);
typedef void     (*DestroyFunction)   (GStonesCave *cave, gpointer data);
typedef void     (*ObjectFunction)    (GStonesCave *cave, gpointer data);
typedef void     (*ScanFunction)      (GStonesCave *cave, guint x, guint y,
				       gpointer data);
typedef guint    (*AnimationFunction) (GStonesCave *cave, guint x, guint y, 
				       gpointer data);

struct _GStonesObject
{
  gchar              *name;

  InitCaveFunction    init_cave_function;
  DestroyFunction     destroy_function;

  ScanFunction        scan_function;

  ObjectFunction      pre_cave_scan_function;
  ObjectFunction      post_cave_scan_function;

  gchar              *image_name;
  AnimationFunction   animation_function;

  /* The image to shown, if no animation_function is given.  */
  guint               image_index;

  /* Which image should be shown in the editor.  (Not used yet) */
  guint               editor_index;


  /* FIXME: should be moved to some private stuff.  */
  GdkPixmap         **image;
  GdkImlibImage     **imlib_image;
};



/*****************************************************************************/
/* The following table is used to convert the ascii codes objects into
   real objects.  */

struct _TranslationEntry
{
  ObjectType type;
  guint      state;
};



/*****************************************************************************/
/* GStonesGame related declarations */


struct _GStonesGame
{
  gchar            *title;
  
  /* Administrativ data.  */
  gchar            *filename;
  gchar            *config_prefix;  

  guint             frame_rate;
  guint             new_life_score;
  guint             lifes;

  GList            *caves;
  GList            *start_caves;
  TranslationEntry  translation_table[256];
};



/*****************************************************************************/
/* GStonesCave related declarations */


#define CAVE_MAX_WIDTH   80
#define CAVE_MAX_HEIGHT  40


typedef enum
{
  CAVE_FINISHED      = 1 << 0,
  CAVE_PLAYER_EXISTS = 1 << 1,
  CAVE_PAUSING       = 1 << 2
} CaveFlags;


struct _GStonesCaveEntry
{
  ObjectType type;
  gint       state;
  gint       anim_state;
  gboolean   scanned;
};


struct _GStonesCave
{
  gchar            *name;
  gchar            *next;

  /* Administrativ data.  */
  gchar            *config_prefix;
  GHashTable       *translation_table;
  guint             key_size;
  
  /* Some static information about this cave.  */
  guint             width;
  guint             height;
  gboolean          is_intermission;
  guint             diamond_score;
  guint             extra_diamond_score;
  guint             diamonds_needed;
  guint             level_time;
  gchar            *message;
  GStonesPlayer    *player;

  /* Some static information about the game, that this cave belongs
     to.  */
  guint             frame_rate;

  /* Some dynamic data about this cave.  */
  CaveFlags         flags;
  gint              timer;
  guint             frame;
  guint             diamonds_collected;

  /* The player's position in this cave.  */
  guint             player_x;
  guint             player_y;

  /* The direction, that the player want't to take.  */
  gint              player_x_direction;
  gint              player_y_direction;
  gboolean          player_push;

  GStonesCaveEntry  entry[CAVE_MAX_WIDTH+2][CAVE_MAX_HEIGHT+2];

  GHashTable       *object_data;
};


/*****************************************************************************/
/* GStonesPlayer related declarations */


struct _GStonesPlayer
{
  /* Static information.  */
  guint new_life_score;
  guint max_time;
  
  /* Dynamic information.  */
  guint score;
  guint lifes;
  guint time;
};


#endif

/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
