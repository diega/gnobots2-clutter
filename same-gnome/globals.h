/* -*- mode: C; indent-tabs-mode: t; tab-width: 2; c-basic-offset: 2; -*- */

/* globals.h : The global state of the game.
 *
 * Copyright (c) 2004 by Callum McKenzie
 *
 */

#ifndef GLOBALS_H
#define GLOBALS_H

#include "gnome.h"

#define DEFAULT_THEME "planets.png"

#define MAX_COLOURS 4

extern gint ncolours;

extern gint board_width;
extern gint board_height;

extern gint window_width;
extern gint window_height;

extern gchar *theme;

#endif /* GLOBALS_H */
