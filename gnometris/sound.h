/* -*- mode:C++; tab-width:8; c-basic-offset:8; indent-tabs-mode:true -*- */

/*
 * written by Callum McKenzie <callum@physics.otago.ac.nz>
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * For more details see the file COPYING.
 */

#ifndef SOUND_H
#define SOUND_H

#include <glib.h>

enum {
  SOUND_GAMEOVER,
  SOUND_GNOMETRIS,
  SOUND_LAND,
  SOUND_LINES1,
  SOUND_LINES2,
  SOUND_LINES3,
  SOUND_SLIDE,
  SOUND_TURN,
  N_SOUNDS
};

class Sound {
 public:
  Sound ();
  void turnOn ();
  void turnOff ();
  void playSound (int soundId);
  gboolean isOn ();
 private:
  gboolean soundOn;
  gboolean soundInit;
  gint sampleIds[N_SOUNDS];
  gint connection;
};

extern Sound *sound;

#endif
