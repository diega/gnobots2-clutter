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

#ifndef _IAGNO2_H_
#define _IAGNO2_H_

void iagno2_tileset_load ();

void iagno2_draw_tile (int tile, int index);

void iagno2_draw_tile_to_buffer (int tile, int index);

void iagno2_app_init ();

void iagno2_canvas_init ();

void iagno2_set_bg_color ();

void iagno2_force_board_redraw ();

void iagno2_move (gchar index);

void iagno2_board_changed ();

void iagno2_setup_current_player ();

void iagno2_initialize_players ();

void iagno2_draw_grid ();

void iagno2_setup_players ();

gint iagno2_game_over ();

void iagno2_render_buffer_to_screen ();

void iagno2_render_tile_to_buffer (int tile, int index);

void iagno2_clean_up (gboolean full);

void iagno2_appbar_init ();

void iagno2_drawing_area_init ();

#endif
