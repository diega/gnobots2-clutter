/*
 * Iagno II: An extensible Reversi game for GNOME
 * Copyright (C) 1999-2000 Ian Peters <itp@gnu.org>
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

#include <gnome.h>
#include <dirent.h>
#include <gmodule.h>

#include "plugin.h"

Iagno2Plugin *
iagno2_plugin_open (const gchar *plugin_file)
{
  Iagno2Plugin *tmp;
  const gchar *(*plugin_move)();

  tmp = (Iagno2Plugin *) g_malloc (sizeof (Iagno2Plugin));

  if (!(tmp->module = g_module_open (plugin_file, 0))) {
    printf ("Boo!\n");
    printf ("Loading plugin %s failed.\n", plugin_file);
    return NULL;
  }

  g_module_symbol (tmp->module, "plugin_init",
      ((gpointer)&(tmp->plugin_init)));

  if (!g_module_symbol (tmp->module, "plugin_move",
                        ((gpointer)&tmp->plugin_move))) {
    g_module_close (tmp->module);
    printf ("Loading plugin %s failed.\n", plugin_file);
    return NULL;
  }

  if (!g_module_symbol (tmp->module, "plugin_name",
                        ((gpointer)&(tmp->plugin_name)))) {
    g_module_close (tmp->module);
    printf ("Loading plugin %s failed.\n", plugin_file);
    return NULL;
  }

  if (!g_module_symbol (tmp->module, "plugin_busy_message",
                        ((gpointer)&(tmp->plugin_busy_message)))) {
    g_module_close (tmp->module);
    printf ("Loading plugin %s failed.\n", plugin_file);
    return NULL;
  }

  tmp->plugin_preferences = NULL;
  g_module_symbol (tmp->module, "plugin_preferences",
                   ((gpointer)&(tmp->plugin_preferences)));

  if (!g_module_symbol (tmp->module, "plugin_about_window",
                        ((gpointer)&(tmp->plugin_about_window)))) {
    tmp->plugin_about_window = NULL;
  }

  return tmp;
}

void
iagno2_plugin_close (Iagno2Plugin *plugin)
{
  if (plugin != NULL) {
    g_module_close (plugin->module);
    g_free (plugin);
  }
}
