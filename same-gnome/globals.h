/* -*- mode: C; indent-tabs-mode: t; tab-width: 2; c-basic-offset: 2; -*- */

/* globals.h : The global state of the game.
 *
 * Copyright (c) 2004 by Callum McKenzie
 *
 */

#ifndef GLOBALS_H
#define GLOBALS_H

#include <gnome.h>
#include <gconf/gconf-client.h>

#define APPNAME "same-gnome"
#define APPNAME_LONG "Same GNOME"

#define DEFAULT_GAME_SIZE SMALL

#define MINIMUM_CANVAS_WIDTH 120
#define MINIMUM_CANVAS_HEIGHT 80

#define DEFAULT_THEME "planets.png"

#define MAX_COLOURS 4

#define GCONF_THEME_KEY "/apps/same-gnome/tileset"
#define GCONF_SIZE_KEY  "/apps/same-gnome/size"
#define GCONF_CUSTOM_WIDTH_KEY "/apps/same-gnome/custom_width"
#define GCONF_CUSTOM_HEIGHT_KEY "/apps/same-gnome/custom_height"
#define GCONF_WINDOW_WIDTH_KEY "/apps/same-gnome/window_width"
#define GCONF_WINDOW_HEIGHT_KEY "/apps/same-gnome/window_height"

extern GConfClient *gcclient;

/* We start at 1 so we can distinguish the gconf "unset" from a valid
 * value. */
enum {
  CUSTOM = 1, /* FIXME: Are we going to use this. */
  SMALL,
  MEDIUM,
  LARGE,
  MAX_SIZE,
};

extern gint board_sizes[MAX_SIZE][3];

extern gint ncolours;

extern gint board_width;
extern gint board_height;

extern gint window_width;
extern gint window_height;

extern gchar *theme;

extern gint   game_size;

#endif /* GLOBALS_H */
