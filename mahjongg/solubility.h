/*
 * Gnome-Mahjonggg-solubility header file
 * (C) 1998-2002 the Free Software Foundation
 *
 *
 * Author: Michael Meeks.
 *
 * http://www.gnome.org/~michael
 * michael@ximian.com
 */

/* If defined this allows graphical placement debugging.
 * define, then run and hit new game. Then press keys as
 * in keyPress to change things */
/* #define PLACE_DEBUG */

extern int  tile_free             (int);
extern void generate_game         (guint seed);
extern void generate_dependancies (void);

