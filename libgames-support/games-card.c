/*
   Copyright © 2007, 2008 Christian Persch

   This library is free software; you can redistribute it and'or modify
   it under the terms of the GNU Library General Public License as published 
   by the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */


#include <config.h>

#include <glib/gi18n.h>

#include "games-card.h"

static const char extra_cards[] =
  "black_joker\0"
  "red_joker\0"
  "back\0"
  "slot";
static const guint8 extra_card_offsets[] = {
  0, 12, 22, 27
};

static const char suites[] =
  "club\0"
  "diamond\0"
  "heart\0"
  "spade";
static const guint8 suite_offsets[] = {
  0, 5, 13, 19
};

static const char ranks[] =
  "1\0"
  "2\0"
  "3\0"
  "4\0"
  "5\0"
  "6\0"
  "7\0"
  "8\0"
  "9\0"
  "10\0"
  "jack\0"
  "queen\0"
  "king";
static const guint8 rank_offsets[] = {
  0, 2, 4, 6, 8, 10, 12, 14, 
  16, 18, 21, 26, 32
};

/**
 * games_card_get_name_by_id_snprintf:
 * @buffer: the output buffer
 * @bufsize: the size of the output buffer
 * @card_id: the ID of the card
 *
 * Prints the identifier for the card @card into @buffer.
 *
 * Returns: the number of bytes which would be produced if the buffer
 * was large enough.
 */
int
games_card_get_name_by_id_snprintf (char *buffer,
                                    gsize bufsize,
                                    int card_id)
{
  int suit, rank, len;

  suit = card_id / 13;
  rank = card_id % 13;

  if (G_LIKELY (suit < 4)) {
    len = g_snprintf (buffer, bufsize, "%s-%s",
                      suites + suite_offsets[suit],
                      ranks + rank_offsets[rank]);
  } else {
    len = g_snprintf (buffer, bufsize, "%s",
                      extra_cards + extra_card_offsets[rank]);
  }

  return len;
}

/**
 * games_card_get_node_by_suit_and_rank_snprintf:
 * @buffer: the output buffer
 * @bufsize: the size of the output buffer
 * @card_id: the ID of the card
 *
 * Prints the identifier for the card @card into @buffer.
 *
 * Returns: the number of bytes which would be produced if the buffer
 * was large enough.
 */
int
games_card_get_node_by_suit_and_rank_snprintf (char *buffer,
                                               gsize bufsize,
                                               int suit,
                                               int rank)
{
  int len;

  if (G_LIKELY (suit < 4)) {
    len = g_snprintf (buffer, bufsize, "#%s_%s",
                      ranks + rank_offsets[rank],
                      suites + suite_offsets[suit]);
  } else {
    len = g_snprintf (buffer, bufsize, "#%s",
                      extra_cards + extra_card_offsets[rank]);
  }

  return len;
}

/**
 * games_card_get_name_by_id:
 * @card_id:
 *
 * Returns the name of the card @cardid
 *
 * Returns: a newly allocated string containing the identifier for @card_id
 */
char *
games_card_get_name_by_id (gint card_id)
{
  char name[128];

  games_card_get_name_by_id_snprintf (name, sizeof (name), card_id);

  return g_strdup (name);
}

/**
 * games_card_get_name:
 * @card:
 *
 * Returns: a localised name for @card, e.g. "Face-down card" or
 * "9 of clubs", etc.
 */
const char *
games_card_get_name (Card card)
{
  return NULL;
}