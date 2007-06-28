/*
 * written by J. Marcin Gorycki <marcin.gorycki@intel.com>
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

#include <config.h>
#include "tetris.h"
#include <games-scores.h>
#include <games-sound.h>
#include <games-conf.h>

int
main(int argc, char *argv[])
{
        g_thread_init (NULL);

	setgid_io_init ();

	bindtextdomain(GETTEXT_PACKAGE, GNOMELOCALEDIR);
        bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");	
	textdomain(GETTEXT_PACKAGE);

	int cmdlineLevel = 0;

	const GOptionEntry options[] = 
	{
		{"level", 'l', 0, G_OPTION_ARG_INT, &cmdlineLevel, N_("Set starting level (1 or greater)"), N_("LEVEL")},
		{NULL}
	};

	GOptionContext *context = g_option_context_new (NULL);

	g_option_context_add_main_entries (context, options, GETTEXT_PACKAGE);
        games_sound_add_option_group (context);

	GnomeProgram *program = gnome_program_init ("gnometris", VERSION,
						    LIBGNOMEUI_MODULE,
						    argc, argv,
						    GNOME_PARAM_GOPTION_CONTEXT, context,
						    GNOME_PARAM_APP_DATADIR, DATADIR,
						    NULL);

	gtk_window_set_default_icon_name ("gnome-gnometris");

        games_conf_initialise ("Gnometris");

	Tetris *t = new Tetris(cmdlineLevel);

	gtk_main();

	delete t;

        games_conf_shutdown ();

	g_object_unref (program);

	return 0;
}
