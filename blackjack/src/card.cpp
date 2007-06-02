// -*- mode:C++; tab-width:8; c-basic-offset:8; indent-tabs-mode:nil -*-
/* 
 * Blackjack - card.cpp
 *
 * Copyright (C) 2003-2004 William Jon McCann <mccann@jhu.edu>
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
#include <games-card-images.h>
#include "card.h"
#include "chips.h"
#include "draw.h"

#include <sys/types.h>
#include <string.h>
#include <stdlib.h>

using namespace std;

GdkBitmap *mask;

GamesCardImages *images = NULL;

GdkPixmap *
bj_card_get_picture (gint suit, gint rank) 
{
        return games_card_images_get_card_pixmap_by_suit_and_rank (images,
                                                                   suit,
                                                                   (rank == 14) ? 1 : rank);
}

GdkPixmap *
bj_card_get_back_pixmap ()
{
        return games_card_images_get_card_pixmap_by_id (images, GAMES_CARD_BACK, FALSE);
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

        im = gdk_pixbuf_new_from_file (filename, NULL);

        return im;
}

GdkPixmap *
get_pixmap (const char *filename)
{
        GdkPixmap *ret;
        GdkPixbuf *im;

        im = gdk_pixbuf_new_from_file (filename, NULL);
        if (im != NULL) {
                gdk_pixbuf_render_pixmap_and_mask (im, &ret, NULL, 127);
                g_object_unref (im);
        } 
        else 
                ret = NULL;

        return ret;
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
        CardSize card_size;

        if (!images) {
                const char *env;
                char *theme;
                gboolean scalable, success;

                env = g_getenv ("BLACKJACK_CARDS_SCALABLE");
                scalable = env == NULL || g_ascii_strtoll (env, NULL, 10) != 0;

                theme = bj_get_card_style ();
                images = games_card_images_new (scalable);
                games_card_images_set_cache_mode (images, CACHE_PIXMAPS);
                games_card_images_set_drawable (images, playing_area->window);

                if (!games_card_images_set_theme (images, theme)) {
                        g_warning ("Failed to load theme %s!", theme);
                }
                g_free (theme);
        }

        games_card_images_set_size (images, width, height, 1.0);
        card_size = games_card_images_get_size (images);

        bj_slot_set_size (card_size.width, card_size.height);
        bj_chip_set_size (card_size.width / 2, card_size.width / 2);

        mask = games_card_images_get_card_mask (images);
        gdk_gc_set_clip_mask (draw_gc, mask);
}

void
bj_card_set_theme (gchar *theme)
{
        games_card_images_set_theme (images, theme);

        bj_draw_rescale_cards ();
        mask = games_card_images_get_card_mask (images);
        gdk_gc_set_clip_mask (draw_gc, mask);
}
