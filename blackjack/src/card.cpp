// -*- mode:C++; tab-width:8; c-basic-offset:8; indent-tabs-mode:nil -*-
/* Blackjack - card.cpp
 * Copyright (C) 2003 William Jon McCann <mccann@jhu.edu>
 * Copyright (C) 1998 Jonathan Blandford <jrb@mit.edu>
 *
 * This game is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
 * USA
 */

#include <config.h>

#include "blackjack.h"
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <games-card-pixmaps.h>
#include "card.h"

#include "chips.h"

#include <sys/types.h>
#include <string.h>
#include <stdlib.h>

#include <iostream>
using namespace std;

GdkBitmap *mask;

GamesCardPixmaps *images = NULL;

GdkPixmap *
bj_card_get_picture (gint suit, gint rank) 
{
        return games_card_pixmaps_get_card (images,
                                            suit, 
                                            (rank == 14) ? 1 : rank);
}

GdkPixmap *
bj_card_get_back_pixmap ()
{
        return games_card_pixmaps_get_back (images);
}

GdkBitmap *
bj_card_get_mask ()
{
        return mask;
}

int
bj_card_get_vert_start ()
{
        return 30;
}

int
bj_card_get_horiz_start ()
{
        return 30;
}

GdkPixbuf *
get_pixbuf (const char *filename)
{
        GdkPixbuf *im;
        char* fullname = gnome_program_locate_file (NULL,
                                                    GNOME_FILE_DOMAIN_APP_PIXMAP,
                                                    filename, TRUE, NULL);

        if (fullname == NULL)
                return NULL; 

        im = gdk_pixbuf_new_from_file (fullname, NULL);
        g_free (fullname);

        return im;
}

GdkPixmap *
get_pixmap (const char *filename)
{
        GdkPixmap *ret;
        GdkPixbuf *im;
        char *fullname = gnome_program_locate_file (NULL,
                                                    GNOME_FILE_DOMAIN_APP_PIXMAP,
                                                    filename, TRUE, NULL);

        if (fullname == NULL)
                return NULL; 

        im = gdk_pixbuf_new_from_file (fullname, NULL);
        if (im != NULL) {
                gdk_pixbuf_render_pixmap_and_mask (im, &ret, NULL, 127);
                g_object_unref (im);
        } 
        else 
                ret = NULL;

        g_free (fullname);

        return ret;
}

void
bj_card_load_pixmaps (GtkWidget *app, gchar *card_style) 
{
        gchar *buffer;

        buffer = g_build_filename ("blackjack", "chip-100.png", NULL);
        chip_pixbuf[CHIP_100] = get_pixbuf (buffer);
        g_free (buffer);
        buffer = g_build_filename ("blackjack", "chip-25.png", NULL);
        chip_pixbuf[CHIP_25] = get_pixbuf (buffer);
        g_free (buffer);
        buffer = g_build_filename ("blackjack", "chip-5.png", NULL);
        chip_pixbuf[CHIP_5] = get_pixbuf (buffer);
        g_free (buffer);
        buffer = g_build_filename ("blackjack", "chip-1.png", NULL);
        chip_pixbuf[CHIP_1] = get_pixbuf (buffer);
        g_free (buffer);
}

void
bj_card_free_pixmaps () 
{
        for (int i=0; i < 4; i++)
                if (chip_pixbuf[i] != NULL)
                        g_object_unref (chip_pixbuf[i]);

}

hcard_type 
bj_card_new (gint value, gint suit, gint direction)
{
        hcard_type temp_card = (hcard_type) g_malloc (sizeof (card_type));

        temp_card->value = value;
        temp_card->suit = suit;
        temp_card->direction = direction;

        return temp_card;
}

void
bj_card_set_size (gint width, gint height)
{
        GdkPixbuf *scaled = NULL;
        GdkBitmap *mask;

        if (bj_slot_get_pixbuf ()) {
                scaled = gdk_pixbuf_scale_simple (bj_slot_get_pixbuf (), width, height,
                                                  GDK_INTERP_BILINEAR);

                bj_slot_set_scaled_pixbuf (scaled);
        }

        if (!images) {
                images = games_card_pixmaps_new (playing_area->window);
                games_card_pixmaps_set_theme (images, bj_get_card_style ());
        }

        games_card_pixmaps_set_size (images, width, height);
        mask = games_card_pixmaps_get_mask (images);
        gdk_gc_set_clip_mask (draw_gc, mask);
}

void
bj_card_set_theme (gchar *theme)
{
        games_card_pixmaps_set_theme (images, theme);
        mask = games_card_pixmaps_get_mask (images);
        gdk_gc_set_clip_mask (draw_gc, mask);
}
