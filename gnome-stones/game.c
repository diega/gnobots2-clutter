/* gnome-stones - game.c
 *
 * Time-stamp: <1998/08/22 12:44:23 carsten>
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

static GList *
game_parse_cave_section (GStonesGame *game, const gchar *name)
{
  GList *cavelist= NULL;
  char  *section;
  gint   number;
  gint   n;
  
  /* FIXME: add error handling.  */
  
  section= g_copy_strings (game->config_prefix, name, "/", NULL);
  gnome_config_push_prefix (section);

  number= gnome_config_get_int ("Number=-1");
  /* FIXME: Error message, if number is "-1".  */
  
  for (n= 1; n <= number; n++)
    {
      static char buffer[20];
      char *cavename;
      
      sprintf (buffer, "Cave%03d", n);
      cavename= gnome_config_get_string (buffer);

      cavelist= g_list_append (cavelist, cavename);
    }

  gnome_config_pop_prefix ();
  g_free (section);
  
  return cavelist;
}

static gboolean
game_parse_object_section (GStonesGame *game)
{
  /* FIXME: improve error handling.  */

  guint     i;
  void     *iter;
  char     *key;
  gint      key_size;
  gboolean  is_default;
  char     *value;

  /* Clear the object translation table.  */
  for (i= 0; i < 256 ; i++)
    game->translation_table[i].type= NULL;

  gnome_config_push_prefix (game->config_prefix);

  /* The key size is not yet used.  It will be usefull, if we have
     more objects in a game, than we have printable ascii characters.
     In this case, every object will be indexed by two or more
     characters (like in the xpm file format).  */

  key_size= gnome_config_get_int_with_default ("General/Object Size",
					       &is_default);
  if (is_default)
    {
      g_warning ("Error in game file: 'Object size' not specified!");
      gnome_config_pop_prefix ();

      return FALSE;
    }
  else if (key_size != 1)
    {
      g_warning ("Error in game file: 'Object size' not equal to one!");
      gnome_config_pop_prefix ();
      
      return FALSE;
    }
  
  iter= gnome_config_init_iterator ("Objects");
  if (!iter)
    {
      g_warning ("Error in game file: No 'Objects' section defined!");
      gnome_config_pop_prefix ();

      return FALSE;
    }

  while ((iter= gnome_config_iterator_next (iter, &key, &value)) != NULL)
    {
      if (strlen (key) == 1)
	{
	  char *type;
	  char *state;
	  guint index= key[0];

	  if (index == '_')
	    index=' ';

	  type = strtok (value, ",");
	  state= strtok (NULL, ",");

	  game->translation_table[index].type= object_get_type (type);
	  if (state)
	    game->translation_table[index].state= atoi (state);
	  else
	    game->translation_table[index].state= 0;
	}
    }

  gnome_config_pop_prefix ();  
  return TRUE;
}


GStonesGame*
game_new (void)
{
  GStonesGame *game;
  guint        i;
  
  game= g_malloc (sizeof (GStonesGame));
  if (!game) return NULL;

  game->title        = NULL;

  game->filename     = 0;
  game->config_prefix= 0;

  game->caves        = NULL;
  game->start_caves  = NULL;


  /* FIXME: clear other variables.  */

  /* Clear the object translation table.  */
  for (i= 0; i < 256 ; i++)
    game->translation_table[i].type= NULL;
  
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

  g_list_foreach (game->caves, (GFunc) g_free, NULL);
  g_list_free (game->caves);
  
  g_list_foreach (game->start_caves, (GFunc) g_free, NULL);
  g_list_free (game->start_caves);
  
  g_free (game);
}

GStonesGame*
game_load (const gchar *name)
{
  GStonesGame *game;

  game= game_new ();
  g_return_val_if_fail (game != NULL, NULL);
  
  game->filename      = g_strdup (name);
  game->config_prefix = g_copy_strings ("=", name, "=/", NULL);
  
  gnome_config_push_prefix (game->config_prefix);

  game->title         = gnome_config_get_string ("General/Title=unknown");
  game->frame_rate    = gnome_config_get_float ("General/Frame rate=0.2")*1000;
  game->new_life_score= gnome_config_get_int    ("General/New life score=500");
  game->lifes         = gnome_config_get_int    ("General/Lifes=3");  
  
  gnome_config_pop_prefix ();

  if (!game_parse_object_section (game))
    {
      game_free (game);
      
      return NULL;
    }
  game->caves      = game_parse_cave_section (game, "Caves");
  game->start_caves= game_parse_cave_section (game, "Start caves");

  return game;  
}


/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
