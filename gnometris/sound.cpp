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

#include <gnome.h>
#ifndef NO_ESD
# include <esd.h>
#endif

#include "sound.h"

/* The master sound object for the game. */
Sound *sound = NULL;

const char *soundFiles[] = { "gnometris/gameover.wav", 
			     "gnometris/gnometris.wav", 
			     "gnometris/land.wav",
			     "gnometris/lines1.wav", 
			     "gnometris/lines2.wav", 
			     "gnometris/lines3.wav",
			     "gnometris/slide.wav", 
			     "gnometris/turn.wav" };

#ifdef NO_ESD

// In the event of no sound support we just use dummy functions.

Sound::Sound () { }

void Sound::turnOn () { }

void Sound::turnOff () { }

gboolean Sound::isOn () { return FALSE; }

void Sound::playSound (int soundId) { }

#else

Sound::Sound ()
{
  soundOn = FALSE;
  soundInit = FALSE;
}

void Sound::turnOn ()
{
  int i;
  gchar * fullName;

  if (soundOn)
    return;

  if (!soundInit) {
    gnome_sound_init (NULL);
    for (i=0; i<N_SOUNDS; i++) {
      fullName = gnome_program_locate_file (NULL, 
					    GNOME_FILE_DOMAIN_APP_SOUND,
					    soundFiles[i], FALSE, NULL);
      sampleIds[i] = gnome_sound_sample_load ("Dummy", fullName);
      g_free (fullName);
    }
    connection = gnome_sound_connection_get ();
  }

  soundOn = TRUE;
}

void Sound::turnOff ()
{
  if (!soundOn)
    return;

  soundOn = FALSE;
}

gboolean Sound::isOn ()
{
  return soundOn;
}

void Sound::playSound (int soundId)
{
  if (!soundOn)
    return;

  esd_sample_play (connection, sampleIds[soundId]);
}

#endif
