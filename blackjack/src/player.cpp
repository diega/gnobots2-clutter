// -*- mode:C++; tab-width:2; c-basic-offset:2; indent-tabs-mode:nil -*-

/* Blackjack - player.cpp
 * Copyright (C) 2003 William Jon McCann <mccann@jhu.edu>
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

#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif

#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <dirent.h>
#include <ctype.h>

#include "blackjack.h"
#include "events.h"
#include "draw.h"
#include "splash.h"
#include "player.h"
#include "hand.h"
#include "game.h"
#include <gnome.h>



#include <iostream>
using namespace std;


Progress::Progress ()
{
  last = -1;
}

void
Progress::indicate (int percentComplete)
{
  float initial_splash_progress = 0.0;
  float splash_progress;
  if (percentComplete != last)
    {
      last = percentComplete;
      splash_progress = initial_splash_progress + (percentComplete/100.0);
      splash_update (_("Computing basic strategy..."), splash_progress);
    }
}

int 
Card::value ()
{
  return (rank < 10 ? rank : 10);
}

Shoe::Shoe (int numDecks)
{
  this->numDecks = numDecks;
  cards = new Card[52*numDecks];
  numCards = 0;
  for (int deck = 0; deck < numDecks; deck++)
    for (int rank = 1; rank <= 13; rank++)
      for (int suit = 1; suit <= 4; suit++)
        {
          cards[numCards].rank = rank;
          cards[numCards].suit = suit;
          numCards++;
        }
  bj_show_shoe_cards (numCards);
  g_random_set_seed (seed);
  shuffle ();
}

Shoe::~Shoe ()
{
  delete[] cards;
}

void
Shoe::shuffle ()
{
  // perform multiple pass selection shuffle
  for (int i=0; i < 5; i++)
    for (int card = 52 * numDecks; card > 1; card--)
      swap (card - 1, g_random_int_range (0, card));
  numCards = 52 * numDecks;

  // Shoe is now full
  bj_show_shoe_cards (numCards);
}

Card
Shoe::deal ()
{
  numCards--;
  bj_show_shoe_cards (numCards);
  return cards[numCards];
}

void
Shoe::swap (int card1, int card2)
{
  Card temp = cards[card1];
  cards[card1] = cards[card2];
  cards[card2] = temp;
}

void
Hand::reset ()
{
  GList* temptr;

  // Free the cards in the slot
  for (temptr = hslot->cards; temptr; temptr = temptr->next)
    g_free (temptr->data);
  g_list_free (hslot->cards);

  hslot->cards = NULL;

  BJHand::reset();
}

int
Hand::deal (Card card, bool faceUp)
{
  cards[getCards()] = card;
  draw (getCards(), faceUp);
  BJHand::deal (card.value());
  return card.value();
}

void
Hand::draw (int card, bool faceUp)
{
  GList *cardlist = NULL;
  hcard_type newcard;

  newcard = bj_card_new (cards[card].rank, cards[card].suit - 1, 
                         (faceUp) ? UP : DOWN);
  cardlist = g_list_append (cardlist, newcard);
  bj_slot_add_cards (cardlist, hslot);
  bj_draw_refresh_screen();
}

void
Hand::redraw ()
{
  for (int card = 0; card < getCards(); card++)
    draw(card);
}

void
Hand::hideCount ()
{
}

// If blackjack is false, then a split hand with a 10-ace is not blackjack, but a soft 21.

void
Hand::showCount (bool blackjack)
{
  hideCount();
  gchar *results = NULL;
  gchar *message;
  gchar *markup;

  if (getCount () > 21) 
    {
      message = g_strdup (_("Bust"));
    }
  else if (blackjack && getCards() == 2 && getCount() == 21)
    {
      message = g_strdup (_("Blackjack!"));
    }
  else
    {
      message = g_strdup_printf ("%s%d",
                                 (getSoft() ? _("Soft") : ""), 
                                 getCount ());
      if (! (bj_game_is_active () || hslot->id == 0 || events_pending))
        {
          gint hand_results = bj_get_hand_results (dealer->getCount (), 
                                                   getCount ());
          switch (hand_results)
            {
            case 1: results = g_strdup_printf (" - %s", _("Win")); break;
            case 0: results = g_strdup_printf (" - %s", _("Push")); break;
            case -1: results = g_strdup_printf (" - %s", _("Lose")); break;
            default: break;
            }
        }
    }
  if (results == NULL)
    results = g_strdup ("");

  markup = g_strdup_printf ("<span size=\"10000\" foreground=\"white\">%s%s</span>", 
                            message, results);
  g_free (message);
  g_free (results);
  if (getCount () > 0)
    bj_draw_playing_area_text (markup, hslot->x + bj_card_get_width () / 2, 
                               hslot->y + bj_card_get_height () + 2);
  g_free (markup);
}

void
PlayerHand::showWager ()
{
}

void
PlayerHand::showOption (int option)
{
}

Player::Player (int numDecks, BJRules *rules, BJStrategy & strategy,
                Progress & progress)
  : BJPlayer(BJShoe(numDecks), *rules, strategy, progress)
{
  this->rules = rules;
}

Player::Player (const int numDecks[], BJRules *rules, BJStrategy & strategy,
                Progress & progress)
  : BJPlayer(BJShoe(numDecks), *rules, strategy, progress)
{
  this->rules = rules;
}

void
Player::reset ()
{
}

int
Player::showOptions (Hand *player, int upCard, int numHands)
{
  int bestOption, num_options;
  double value, bestValue;
  gchar *markup = NULL;
  gchar *tmpstr = NULL;
  gchar *mark_list[6];

  reset ();

  num_options = 0;

  // Player can always stand.

  bestValue = value = getValueStand (*player, upCard);
  mark_list[num_options++] = g_strdup_printf ("%s     %9.4lf\n", 
                                              _("Stand"), value * 100);
  bestOption = KEY_S;

  // Player can't hit split aces.
  if (bj_hand_can_be_hit ())
    {
      value = getValueHit (*player, upCard);
      
      mark_list[num_options++] = g_strdup_printf ("%s       %9.4lf\n", 
                                                  _("Hit"), value * 100);
      if (value > bestValue)
        {
          bestValue = value;
          bestOption = KEY_H;
        }
      
      // Check if player can double down.
      if (bj_hand_can_be_doubled ())
        {
          value = getValueDoubleDown (*player, upCard);
          mark_list[num_options++] = g_strdup_printf ("%s    %9.4lf\n", 
                                                      _("Double"), value * 100);
          if (value > bestValue)
            {
              bestValue = value;
              bestOption = KEY_D;
            }
        }
    }
  
  // Check if player can split a pair.

  if (bj_hand_can_be_split ())
    {
      value = getValueSplit(player->cards[0].value (), upCard);
      mark_list[num_options++] = g_strdup_printf ("%s     %9.4lf\n", 
                                                  _("Split"), value * 100);
      if (value > bestValue)
        {
          bestValue = value;
          bestOption = KEY_P;
        }
    }

  // Check if player can surrender.
  if (bj_hand_can_be_surrendered ())
    {
      value = -0.5;
      mark_list[num_options++] = g_strdup_printf ("%s %9.4lf\n", 
                                                  _("Surrender"), value * 100);
      if (value > bestValue)
        {
          bestValue = value;
          bestOption = KEY_R;
        }
    }

  switch (num_options)
    {
    case 1:  
      tmpstr = g_strdup (mark_list[0]);
      break;
    case 2:
      tmpstr = g_strconcat (mark_list[0], mark_list[1], NULL);
      break;
    case 3:
      tmpstr = g_strconcat (mark_list[0], mark_list[1], mark_list[2], NULL);
      break;
    case 4:
      tmpstr = g_strconcat (mark_list[0], mark_list[1], mark_list[2], 
                            mark_list[3], NULL);
      break;
    case 5:
      tmpstr = g_strconcat (mark_list[0], mark_list[1], mark_list[2], 
                            mark_list[3], mark_list[4], NULL);
      break;
    default:
      break;
    }

  if (tmpstr)
    {
      markup = g_strdup_printf ("<span size=\"10000\" font_family=\"fixed\" "
                                "foreground=\"white\">%s</span>",
                                tmpstr);
      g_free (tmpstr);
      for (int i = 0; i < num_options; i++)
        g_free (mark_list[i]);
      bj_draw_playing_area_text (markup, 25, 125);
      g_free (markup);
    }
  return bestOption;
}

int
Player::getBestOption (Hand *player, int upCard, int numHands)
{
  int bestOption;
  double value, bestValue;

  reset ();

  // Player can always stand.

  bestValue = value = getValueStand (*player, upCard);
  bestOption = KEY_S;

  // Player can't hit split aces.
  if (bj_hand_can_be_hit ())
    {
      value = getValueHit (*player, upCard);
      if (value > bestValue)
        {
          bestValue = value;
          bestOption = KEY_H;
        }
      
      // Check if player can double down.
      if (bj_hand_can_be_doubled ())
        {
          value = getValueDoubleDown (*player, upCard);
          if (value > bestValue)
            {
              bestValue = value;
              bestOption = KEY_D;
            }
        }
    }
  
  // Check if player can split a pair.

  if (bj_hand_can_be_split ())
    {
      value = getValueSplit(player->cards[0].value (), upCard);
      if (value > bestValue)
        {
          bestValue = value;
          bestOption = KEY_P;
        }
    }

  // Check if player can surrender.
  if (bj_hand_can_be_surrendered ())
    {
      value = -0.5;
      if (value > bestValue)
        {
          bestValue = value;
          bestOption = KEY_R;
        }
    }
  return bestOption;
}

Probabilities::Probabilities (bool hitSoft17) : BJDealer(hitSoft17)
{
}

void
Probabilities::reset ()
{
}

void
Probabilities::showProbabilities (BJShoe *distribution, int upCard,
                                  bool condition)
{
  gchar *markup;
  gchar *mark_list[6];
  
  computeProbabilities (*distribution);
  double notBlackjack;
  if (condition)
    notBlackjack = (double)1 - getProbabilityBlackjack (upCard);
  else
    notBlackjack = 1;
  reset();

  mark_list[0] = g_strdup_printf ("%s         %.4lf\n", _("Bust"),
                                  getProbabilityBust (upCard) / notBlackjack);
  for (int count = 17; count <= 21; count++)
    {
      mark_list[count-16] = g_strdup_printf ("%2d           %.4lf\n", count,
                                             getProbabilityCount (count, upCard)
                                             / notBlackjack);
    }
  mark_list[5] = g_strdup_printf ("%s    %.4lf\n", _("Blackjack"),
                                  condition ? 0.0 : getProbabilityBlackjack (upCard));
  markup = g_strconcat ("<span size=\"10000\" font_family=\"fixed\" foreground=\"white\">", 
                        mark_list[0], mark_list[1], mark_list[2], mark_list[3], 
                        mark_list[4], mark_list[5], 
                        "</span>", NULL);
  
  for (int i = 0; i < 6; i++)
    g_free (mark_list[i]);
  bj_draw_playing_area_text (markup, 25, 25);
  g_free (markup);
}

const int allTens[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 4};

LoadablePlayer::LoadablePlayer (int numDecks, BJRules *rules, BJStrategy & strategy,
                                Progress & progress, const char *filename = NULL)
  : Player (allTens, rules, strategy, progress)
{
  if (filename != NULL)
    reset (filename);
  else
    BJPlayer::reset (BJShoe(numDecks), *rules, strategy, progress);
}

void
LoadablePlayer::reset (const char *filename)
{
  if (filename)
    {
      load (filename);
    }
}

void
LoadablePlayer::writeFile (const char *filename)
{
  gzFile fp;

  fp = gzopen (filename, "wb9");
  if (! fp) 
    {
      cerr << "Could not open required file " << filename << endl;
      return;
    }

  gzwrite (fp, &playerHands, sizeof (playerHands));
  gzwrite (fp, &playerHandCount, sizeof (playerHandCount));
  gzwrite (fp, &valueSplit, sizeof (valueSplit));
  gzwrite (fp, &resplit, sizeof (resplit));
  gzwrite (fp, &overallValues, sizeof (overallValues));
  gzwrite (fp, &overallValue, sizeof (overallValue));

  gzclose (fp);
}

void
LoadablePlayer::readFile (const char *filename)
{
  // load from file:
  //   playerHandCount[][], playerHands[], valueSplit[][]
  //   (and preferably resplit[], overallValues[], and overallValue)
  gzFile fp;

  fp = gzopen (filename, "rb");
  if (! fp) 
    {
      cerr << "Could not open required file " << filename << endl;
      return;
    }

  gzread (fp, &playerHands, sizeof (playerHands));
  gzread (fp, &playerHandCount, sizeof (playerHandCount));
  gzread (fp, &valueSplit, sizeof (valueSplit));
  gzread (fp, &resplit, sizeof (resplit));
  gzread (fp, &overallValues, sizeof (overallValues));
  gzread (fp, &overallValue, sizeof (overallValue));

  gzclose (fp);
}

void
LoadablePlayer::saveHand (int i)
{
  // cache playerHands[i] as well as index i
  gzwrite (fp, &i, sizeof (i));
  gzwrite (fp, &playerHands[i].cards, sizeof (playerHands[i].cards));
  gzwrite (fp, &playerHands[i].hitHand, sizeof (playerHands[i].hitHand));
  gzwrite (fp, &playerHands[i].nextHand, sizeof (playerHands[i].nextHand));
  gzwrite (fp, &playerHands[i].valueStand[0], sizeof (playerHands[i].valueStand[0]));
  gzwrite (fp, &playerHands[i].valueHit[0], sizeof (playerHands[i].valueHit[0]));
  gzwrite (fp, &playerHands[i].valueDoubleDown[0], sizeof (playerHands[i].valueDoubleDown[0]));
}

void
LoadablePlayer::saveCount (int count, bool soft)
{
  for (int i=playerHandCount[count][soft]; i; i=playerHands[i].nextHand)
    {
      saveHand (i);
    }
}

void
LoadablePlayer::save (const char *filename)
{
  int card, count;

  fp = gzopen (filename, "wb9");
  if (! fp) 
    {
      cerr << "Could not open required file " << filename << endl;
      return;
    }

  gzwrite (fp, &playerHandCount, sizeof (playerHandCount));
  gzwrite (fp, &valueSplit, sizeof (valueSplit));
  gzwrite (fp, &resplit, sizeof (resplit));
  gzwrite (fp, &overallValues, sizeof (overallValues));
  gzwrite (fp, &overallValue, sizeof (overallValue));

  saveHand (0);
  for (card = 1; card <= 10; card++)
    saveHand (playerHands[0].hitHand[card - 1]);

  for (count = 21; count >= 11; count--)
    saveCount (count, false);
  
  for (count = 21; count >= 12; count--)
    saveCount (count, true);
  
  for (count = 10; count >= 4; count--)
    saveCount (count, false);

  gzclose (fp);
}

void
LoadablePlayer::load (const char *filename)
{
  int i;
  // load numHands, playerHandCount, etc.
  // now load the playerHands array

  gzFile fp;

  fp = gzopen (filename, "rb");
  if (! fp) 
    {
      cerr << "Could not open required file " << filename << endl;
      return;
    }

  gzread (fp, &playerHandCount, sizeof (playerHandCount));
  gzread (fp, &valueSplit, sizeof (valueSplit));
  gzread (fp, &resplit, sizeof (resplit));
  gzread (fp, &overallValues, sizeof (overallValues));
  gzread (fp, &overallValue, sizeof (overallValue));

  while (! gzeof (fp))
    {
      gzread (fp, &i, sizeof (i));
      gzread (fp, &playerHands[i].cards, sizeof (playerHands[i].cards));
      gzread (fp, &playerHands[i].hitHand, sizeof (playerHands[i].hitHand));
      gzread (fp, &playerHands[i].nextHand, sizeof (playerHands[i].nextHand));
      gzread (fp, &playerHands[i].valueStand[0], sizeof (playerHands[i].valueStand[0]));
      gzread (fp, &playerHands[i].valueHit[0], sizeof (playerHands[i].valueHit[0]));
      gzread (fp, &playerHands[i].valueDoubleDown[0], sizeof (playerHands[i].valueDoubleDown[0]));

    }

  gzclose (fp);
}
