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

#include "properties.h"

Iagno2Properties *iagno2_properties_new ()
{
  Iagno2Properties *tmp;

  tmp = (Iagno2Properties *) g_malloc (sizeof (Iagno2Properties));

  tmp->draw_grid = gnome_config_get_bool
      ("/iagno2/Preferences/draw_grid=true");
  tmp->tileset = gnome_config_get_string
      ("/iagno2/Preferences/tileset=classic.png");

  tmp->player1 = gnome_config_get_string
      ("/iagno2/Preferences/player1=Human");

  tmp->player2 = gnome_config_get_string
      ("/iagno2/Preferences/player2=libiagno2-random.so");

  return (tmp);
}

void
iagno2_properties_destroy (Iagno2Properties *properties)
{
  g_free (properties->tileset);
  g_free (properties->player1);
  g_free (properties->player2);
  g_free (properties);
}

Iagno2Properties *
iagno2_properties_copy (Iagno2Properties *properties)
{
  Iagno2Properties *tmp;

  tmp = (Iagno2Properties *) g_malloc (sizeof (Iagno2Properties));

  tmp->draw_grid = properties->draw_grid;
  tmp->tileset = g_strdup (properties->tileset);
  tmp->player1 = g_strdup (properties->player1);
  tmp->player2 = g_strdup (properties->player2);

  return (tmp);
}

void
iagno2_properties_sync (Iagno2Properties *properties)
{
  gnome_config_set_bool ("/iagno2/Preferences/draw_grid",
                         properties->draw_grid);
  gnome_config_set_string ("/iagno2/Preferences/tileset",
                           properties->tileset);
  gnome_config_set_string ("/iagno2/Preferences/player1",
                           properties->player1);
  gnome_config_set_string ("/iagno2/Preferences/player2",
                           properties->player2);
  gnome_config_sync ();
}
