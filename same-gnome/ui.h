/* -*- mode: C; indent-tabs-mode: t; tab-width: 2; c-basic-offset: 2; -*- */

/* ui.h : User interface code
 *
 * Copyright (c) 2004 by Callum McKenzie
 *
 */

#ifndef UI_H
#define UI_H

#include <gnome.h>

void show_score (gint score);
void game_over_dialog (void);
void new_frame_ratio (gint board_width, gint board_height);
void build_gui (void);
void set_undoredo_sensitive (gboolean undo, gboolean redo);

#endif /* UI_H */
