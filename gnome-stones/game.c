/* gnome-stones - game.c
 *
 * Time-stamp: <1998/11/01 16:08:33 carsten>
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
#include <string.h>
#include "game.h"
#include "cave.h"
#include "object.h"

GStonesGame*
game_new (void)
{
  GStonesGame *game;
  
  game= g_new0 (GStonesGame, 1);
  if (!game) return NULL;
  
  return game;
}


void
game_free (GStonesGame *game)
{
  if (!game)
    return;

  g_free (game->title);
  
  g_free (game->filename);
  g_free (game->config_prefix);

  /* Remove list of caves.  */
  g_list_foreach (game->caves, (GFunc) g_free, NULL);
  g_list_free (game->caves);
  
  /* Remove list of start_caves.  */
  g_list_foreach (game->start_caves, (GFunc) g_free, NULL);
  g_list_free (game->start_caves);
  
  /* Remove list of plugins.  We only stored references to plugins, so
     the plugins themself don't need to get freed.  Same with the
     objects.  */
  g_list_free (game->plugins);
  g_list_free (game->objects);

  /* FIXME: Free translation_table.  */

  g_free (game);
}


/*****************************************************************************/

/* A helper function to game_add_plugin.  */

static void
game_add_object (char           *name,
		 GStonesObject  *object,
		 GList         **object_list)
{
  (*object_list)= g_list_append (*object_list, object);
}


/* Make a game require a plugin.  The plugin will be added to the
   game's plugin list.  Additionally all objects, that are exported by
   the plugin are added to the game's object list.  */

gboolean
game_add_plugin (GStonesGame *game, GStonesPlugin *plugin)
{
  g_return_val_if_fail (game, FALSE);
  g_return_val_if_fail (plugin, FALSE);
  
  /* Check if this plugin is already required by this game.  If so, we
   have nothing to do.*/
  if (g_list_find (game->plugins, plugin))
    return TRUE;

  /* Add this plugin to the list of required plugins.  */
  game->plugins= g_list_append (game->plugins, plugin);

  /* Finally we have to add all objects, that this plugin exports to the
     game's list of objects.  */
  g_hash_table_foreach (plugin->objects, (GHFunc) game_add_object, 
			&game->objects);

  return TRUE;
}



/*****************************************************************************/
/* Cave definitions.  */

#ifdef OBSOLET

GStonesCave *
cave_load (GStonesGame *game, const gchar *cavename)
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
  cave->game               = game;
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
	  TranslationEntry *entry;
	  char key[2]= { '\0', '\0'};
	  
	  key[0]= line[x-1];
	  
	  entry= g_hash_table_lookup (game->translation_table, key);
							
	  if (entry)
	    {
	      cave->entry[x][y].type = entry->type;
	      cave->entry[x][y].state= entry->state;
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
  tmp= game->objects;
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

#endif


/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
