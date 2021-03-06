// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

///////////////////////////////////////////////////////////////////////////////
//
// Blackjack.h
// Version 5.0
// Copyright (C) 1999, 2001, 2002 Eric Farmer
//
// Blackjack strategy calculator.  Contains classes for computing exact
// probabilities and expected values, for outcomes of the dealer's hand and
// play options for all possible player hands.
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
// more details.
//
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the Free Software Foundation, Inc., 59 Temple
// Place, Suite 330, Boston, MA  02111-1307  USA
//

#ifndef STRATEGY_H
#define STRATEGY_H

///////////////////////////////////////////////////////////////////////////////
//
// Return values for BJStrategy::getOption() (see below)
//

#define BJ_MAX_VALUE    0
#define BJ_STAND        1
#define BJ_HIT          2
#define BJ_DOUBLE_DOWN  3
#define BJ_SPLIT        4
#define BJ_SURRENDER    5

///////////////////////////////////////////////////////////////////////////////
//
// BJHand
//
// A BJHand object represents a blackjack hand held by the dealer or a player.
// The (hard or soft) count and the number of cards (total as well as of each
// value) in the hand are maintained.
//

class BJHand {
  friend class BJShoe;
  friend class BJDealer;
  friend class BJPlayer;

public:

// BJHand() creates an empty blackjack hand.

    BJHand ();

// BJHand(cards) creates a blackjack hand with cards[i - 1] number of cards of
// value i, for each i in the range 1 (ace) through 10.

    BJHand (const int cards[]);

// getCards(card) returns the number of cards with the given value in the hand,
// where card is in the range 1 (ace) through 10.

  int getCards (int card) const;

// getCards() returns the total number of cards in the hand.

  int getCards () const;

// getCount() returns the hand's (hard or soft) count.  Soft hands count ace as
// 11.

  int getCount () const;

// getSoft() returns true iff the hand is soft.

  bool getSoft () const;

// reset() removes all cards from the hand.

  void reset ();

// reset(cards) sets the hand to contain cards[i - 1] number of cards of value
// i, for each i in the range 1 (ace) through 10.

  void reset (const int cards[]);

// deal(card) deals a card of the given value to the hand (possibly busting the
// hand), where card is in the range 1 (ace) through 10.

  void deal (int card);

// undeal(card) removes a card of the given value from the hand, where card is
// in the range 1 (ace) through 10.

  void undeal (int card);

protected:
  int cards[10], numCards, count;
  bool soft;
};

///////////////////////////////////////////////////////////////////////////////
//
// BJShoe
//
// A BJShoe object represents a blackjack shoe containing dealt and undealt
// cards.  As in a BJHand object, only the distribution (not the order) of
// cards is maintained.
//

class BJShoe {
  friend class BJDealer;
  friend class BJPlayer;

public:

// BJShoe(numDecks) creates a blackjack shoe containing the given number of
// 52-card decks, with all cards in the shoe (i.e., undealt).

    BJShoe (int numDecks = 1);

// BJShoe(cards) creates a blackjack shoe containing cards[i - 1] number of
// cards of value i, for each i in the range 1 (ace) through 10, with all cards
// in the shoe (i.e., undealt).

    BJShoe (const int cards[]);

// getCards(card) returns the number of undealt cards with the given value
// remaining in the shoe, where card is in the range 1 (ace) through 10.

  int getCards (int card) const;

// getCards() returns the total number of undealt cards remaining in the shoe.

  int getCards () const;

// getProbability(card) returns the probability of a card with the given value
// being dealt next from the shoe, where card is in the range 1 (ace) through
// 10.

  double getProbability (int card) const;

// reset() returns all dealt cards to the shoe.

  void reset ();

// reset(hand) returns all dealt cards to the shoe, then deals the given
// blackjack hand from the "full" shoe.

  void reset (const BJHand & hand);

// reset(numDecks) sets the shoe to contain the given number of 52-card decks,
// with all cards in the shoe (i.e., undealt).

  void reset (int numDecks);

// reset(cards) sets the shoe to contain cards[i - 1] number of cards of value
// i, for each i in the range 1 (ace) through 10, with all cards in the shoe
// (i.e., undealt).

  void reset (const int cards[]);

// deal(card) deals a card of the given value from the shoe, where card is in
// the range 1 (ace) through 10.

  void deal (int card);

// undeal(card) returns a card of the given value to the shoe, where card is in
// the range 1 (ace) through 10.

  void undeal (int card);

protected:
  int totalCards[10], cards[10], numCards;
};

///////////////////////////////////////////////////////////////////////////////
//
// BJDealer
//
// A BJDealer object contains functions for computing the probabilities of
// outcomes of the dealer's hand for a given blackjack shoe.
//

class BJDealer {
  friend class BJPlayer;

public:

// BJDealer(hitSoft17) prepares for computing probabilities, where hitSoft17 is
// true iff the dealer hits soft 17.

    BJDealer (bool hitSoft17);

// computeProbabilities(shoe) computes the probabilities of outcomes of the
// dealer's hand for the given shoe (i.e., for the distribution of undealt
// cards in the given shoe).  It is assumed that the dealer's up card also
// remains undealt; e.g., given a full shoe with no cards dealt, the
// appropriate probabilities will be computed.

  void computeProbabilities (const BJShoe & shoe);

// getProbabilityBust(upCard) returns the probability (computed by
// computeProbabilities) of the dealer busting with the given up card, where
// upCard is in the range 1 (ace) through 10.

  double getProbabilityBust (int upCard) const;

// getProbabilityBust() returns the overall probability (computed by
// computeProbabilities) of the dealer busting.

  double getProbabilityBust () const;

// getProbabilityCount(count, upCard) returns the probability (computed by
// computeProbabilities) of the dealer drawing to the given count, where count
// is in the range 17 through 21, for the given up card, where upCard is in the
// range 1 (ace) through 10.

  double getProbabilityCount (int count, int upCard) const;

// getProbabilityCount(count) returns the overall probability (computed by
// computeProbabilities) of the dealer drawing to the given count, where count
// is in the range 17 through 21.

  double getProbabilityCount (int count) const;

// getProbabilityBlackjack(upCard) returns the probability (computed by
// computeProbabilities) of the dealer having blackjack, for the given up card,
// where upCard is in the range 1 (ace) through 10.

  double getProbabilityBlackjack (int upCard) const;

// getProbabilityBlackjack() returns the overall probability (computed by
// computeProbabilities) of the dealer having blackjack.

  double getProbabilityBlackjack () const;

protected:
    bool hitSoft17;
  struct DealerHand {
    int cards[10], multiplier[10];
  };
  struct DealerHandCount;
  friend struct DealerHandCount;	// make DealerHand accessible
  struct DealerHandCount {
    int numHands;
    DealerHand dealerHands[423];
  } dealerHandCount[5];
  BJHand currentHand;
  int upCard;
  double probabilityBust[10],
    probabilityCount[5][10],
    probabilityBlackjack[10], probabilityCard[10], lookup[13][10][12];
  static const int maxSvalues[10], maxHvalues[10];

  void countHands ();
};

///////////////////////////////////////////////////////////////////////////////
//
// BJRules
//
// The BJRules interface allows specification of a particular set of rules for
// casino blackjack, indicating whether the dealer stands or hits with soft 17,
// whether pairs may be split more than once, etc.
//

class BJRules {
public:

// BJRules() is a default constructor allowing derivation from BJRules, and is
// equivalent to BJRules(false, true, true, true, false, true, true, false,
// false).

  BJRules ();

// BJRules(...) is a convenience constructor for creating most common rule
// variations, specified by:
//
// hitSoft17            true iff the dealer hits soft 17
// doubleAnyTotal       true if doubling down is allowed on any hand total,
//                          false if only on (9 or) 10 or 11
// double9              true iff doubling down is allowed on 9 (valid only if
//                          doubleAnyTotal is false)
// doubleSoft           true iff doubling down is allowed on soft hands
// doubleAfterHit       true iff doubling down is allowed on more than 2 cards
// doubleAfterSplit     true iff doubling down is allowed after splitting pairs
// resplit              true iff pairs may be resplit (up to 4 hands)
// resplitAces          true iff aces may be resplit (up to 4 hands; valid only
//                          if resplit is true)
// lateSurrender        true iff late surrender is allowed

  BJRules (bool hitSoft17, bool doubleAnyTotal, bool double9, bool doubleSoft,
	   bool doubleAfterHit, bool doubleAfterSplit, bool resplit,
	   bool resplitAces, bool lateSurrender);

// ~BJRules() allows appropriate destruction of objects derived from BJRules.

  virtual ~ BJRules ();

// getHitSoft17() returns true iff the dealer hits soft 17.

  virtual bool getHitSoft17 ();

  virtual bool getDoubleAnyTotal ();
  virtual bool getDouble9 ();
  virtual bool getDoubleSoft ();
  virtual bool getDoubleAfterHit ();
  virtual bool getDoubleAfterSplit ();
  virtual bool getResplit ();
  virtual bool getResplitAces ();

// getDoubleDown(hand) returns true iff doubling down is allowed on the given
// blackjack hand.

  virtual bool getDoubleDown (const BJHand & hand);

// getDoubleAfterSplit(hand) returns true iff doubling down is allowed on the
// given blackjack hand after splitting pairs.

  virtual bool getDoubleAfterSplit (const BJHand & hand);

// getResplit(pairCard) returns the maximum number of hands to which the given
// pairCard may be split.  Valid return values are 1 (no splits), 2 (no
// resplit), 3, or 4.

  virtual int getResplit (int pairCard);

// getLateSurrender() returns true iff late surrender is allowed.

  virtual bool getLateSurrender ();

protected:
    bool hitSoft17,
    doubleAnyTotal,
    double9,
    doubleSoft,
    doubleAfterHit, doubleAfterSplit, resplit, resplitAces, lateSurrender;
};

///////////////////////////////////////////////////////////////////////////////
//
// The BJStrategy interface allows specification of a possibly sub-optimal
// playing strategy, for which corresponding expected values may be computed
// (see BJPlayer).  Examples of such strategies include total-dependent vs.
// composition-dependent, "mimic the dealer," etc.
//

class BJStrategy {
public:

// ~BJStrategy() allows appropriate destruction of objects derived from
// BJStrategy.

  virtual ~ BJStrategy ();

// getOption(...) returns one of the constants BJ_* specified above, indicating
// which player option is to be taken in the situation specified by:
//
// hand         the player's hand
// upCard       the dealer's up card, where upCard is in the range 1 (ace)
//                  through 10
// doubleDown   true iff doubling down is allowed
// split        true iff splitting is allowed
// surrender    true iff surrender is allowed
//
// A return value of BJ_MAX_VALUE indicates that the option maximizing the
// expected value of the player's hand is to be taken.  Other return values
// indicate the corresponding option; it is an error to return BJ_DOUBLE_DOWN
// if doubleDown is false, or BJ_SPLIT if split is false, or BJ_SURRENDER if
// surrender is false.
//
// BJStrategy::getOption(...) returns BJ_MAX_VALUE.

  virtual int getOption (const BJHand & hand, int upCard, bool doubleDown,
			 bool split, bool surrender);
};

///////////////////////////////////////////////////////////////////////////////
//
// BJProgress
//
// The BJProgress interface allows some indication of progress of the creation
// of a BJPlayer object (or the execution of BJPlayer::reset()).
//

class BJProgress {
public:

// ~BJProgress() allows appropriate destruction of objects derived from
// BJProgress.

  virtual ~ BJProgress ();

// indicate(percentComplete) is called repeatedly during the creation of a
// BJPlayer object (or the execution of BJPlayer::reset()), indicating the
// progress of the computation, where percentComplete is in the range 0 through
// 100 (complete).

  virtual void indicate (int percentComplete);
};

///////////////////////////////////////////////////////////////////////////////
//
// BJPlayer
//
// A BJPlayer object contains functions for computing the expected values of
// all player options for all possible player hands against each dealer up
// card, for a given blackjack shoe, rule variations, and playing strategy.
//

class BJPlayer:public BJStrategy {
public:

// BJPlayer(shoe, rules, strategy, progress) and reset(...) compute expected
// values, both overall and for all possible player hands and dealer up cards,
// for the given blackjack shoe, rule variations, and playing strategy.  Only
// undealt cards in the shoe are considered; progress.indicate() is called
// repeatedly during execution.

  BJPlayer (const BJShoe & shoe, BJRules & rules, BJStrategy & strategy,
	    BJProgress & progress);
  void reset (const BJShoe & shoe, BJRules & rules, BJStrategy & strategy,
	      BJProgress & progress);

// getValue*(hand, upCard) returns the expected value (as a fraction of initial
// wager), conditioned on the dealer not having blackjack, of the indicated
// player option for the given hand and dealer up card, where upCard is in the
// range 1 (ace) through 10.  Results are undefined and may cause an error if
// the hand is a bust hand or if the hand and up card are not possible in the
// given shoe.

  double getValueStand (const BJHand & hand, int upCard) const;
  double getValueHit (const BJHand & hand, int upCard) const;
  double getValueDoubleDown (const BJHand & hand, int upCard) const;

// getValueSplit(pairCard, upCard) returns the expected value (as a fraction of
// initial wager), conditioned on the dealer not having blackjack, of splitting
// a pair of cards with the given value against the given dealer up card, where
// pairCard and upCard are in the range 1 (ace) through 10.  Results are
// undefined if the pair hand and up card are not possible in the given shoe.

  double getValueSplit (int pairCard, int upCard) const;

// getValue(upCard) returns the overall expected value (as a fraction of
// initial wager) of a player hand against the given dealer up card, where
// upCard is in the range 1 (ace) through 10.  Results are undefined if the up
// card is not in the given shoe.

  double getValue (int upCard) const;

// getValue() returns the overall expected value (as a fraction of initial
// wager) of a player hand.

  double getValue () const;

// getOption() implements the BJStrategy interface, returning the player option
// which maximizes the expected value of the hand (assuming, if necessary,
// subsequent following of the given strategy).

  int getOption (const BJHand & hand, int upCard, bool doubleDown, bool split,
		 bool surrender);

protected:
  int numHands, playerHandCount[22][2];
  struct PlayerHand {
    int cards[10], hitHand[10], nextHand;
    float valueStand[2][10],
      valueHit[2][10],
      valueDoubleDown[2][10],
      probabilityBust[10], probabilityCount[5][10], probabilityBlackjack[10];
  } playerHands[16373];
  BJHand currentHand;
  BJShoe shoe;
  int resplit[10];
  double valueSplit[10][10], overallValues[10], overallValue;

  int findHand (const BJHand & hand) const;
  bool record (const BJHand & hand);
  void countHands (int i, int maxCard);
  void linkHands ();
  void computeDealer (BJRules & rules, BJProgress & progress);
  void linkHandCounts (bool split = false, int pairCard = 0,
		       int splitHands = 1);

  void computeStand (bool split = false, int pairCard = 0,
		     int splitHands = 1);
  void computeStandCount (int count, bool soft, bool split, int pairCard,
			  int splitHands);

  void computeDoubleDown (bool split = false, int pairCard = 0,
			  int splitHands = 1);
  void computeDoubleDownCount (int count, bool soft, bool split, int pairCard,
			       int splitHands);

  void computeHit (BJRules & rules, BJStrategy & strategy, bool split = false,
		   int pairCard = 0, int splitHands = 1);
  void computeHitCount (int count, bool soft, BJRules & rules,
			BJStrategy & strategy, bool split, int pairCard,
			int splitHands);

  void computeSplit (BJRules & rules, BJStrategy & strategy);
  void correctStandBlackjack ();
  void computeOverall (BJRules & rules, BJStrategy & strategy);
  double computeSurrender (int upCard);
  void conditionNoBlackjack ();
};

#endif
