/* gnome-stones - cave.h
 *
 * Time-stamp: <1998/09/07 22:28:49 carsten>
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
#ifndef CAVE_H
#define CAVE_H

#include <config.h>
#include <gnome.h>
#include "types.h"


/*****************************************************************************/
/* Object declarations.  */


gboolean
object_register (GStonesObject *object);


const ObjectType
object_get_type (const gchar *name);

#define OBJECT_DEFAULT_IMAGE -1
#define OBJECT_EDITOR_IMAGE -2

GdkPixmap *
object_get_image (ObjectType type, gint index);

GdkImlibImage *
object_get_imlib_image (ObjectType type, gint index);

gboolean
objects_register_all (void);



/*****************************************************************************/
/* Cave declaration.  */


GStonesCave   *cave_new               (void);
void           cave_free              (GStonesCave *cave);

GStonesCave   *cave_load              (const GStonesGame *game,
				       const char        *cavename);

void           cave_set_player        (GStonesCave *cave, 
				       GStonesPlayer *player);

/* Calculate the cave's next state.

   The next function iterates the cave's state.*/

void           cave_iterate           (GStonesCave *cave,
				       gint         x_direction,
				       gint         y_direction,
				       gboolean     push);

void           cave_toggle_pause_mode (GStonesCave *cave);

/* Let our little gnome die.  You really don't want this to happen, do
   you?  */

void           cave_player_die        (GStonesCave *cave);

/* The following function starts the cave by replacing the entrance
   with our gnome.  */

void           cave_start             (GStonesCave *cave);


guint          cave_time_to_frames    (GStonesCave *cave, gdouble time);

GdkPixmap     *cave_get_image         (GStonesCave *cave, guint x, guint y);

GdkImlibImage *cave_get_imlib_image   (GStonesCave *cave, guint x, guint y);

#endif

/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
