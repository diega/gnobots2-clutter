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

#ifndef _IAGNO2_PROPERTIES_H_
#define _IAGNO2_PROPERTIES_H_

typedef struct {
	gboolean draw_grid;
  gboolean show_valid_moves;
	gchar *tileset;
	gchar *players[2];
} Iagno2Properties;

Iagno2Properties *iagno2_properties_new ();

void iagno2_properties_destroy (Iagno2Properties *properties);

Iagno2Properties *iagno2_properties_copy (Iagno2Properties *properties);

void iagno2_properties_sync (Iagno2Properties *properties);

#endif
