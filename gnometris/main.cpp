/* -*- mode:C++; tab-width:8; c-basic-offset:8; indent-tabs-mode:true -*- */
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

#include <libgames-support/games-scores.h>
#include <libgames-support/games-conf.h>
#include <libgames-support/games-runtime.h>
#include <clutter-gtk/clutter-gtk.h>

#include "tetris.h"

int
main(int argc, char *argv[])
{
	gboolean retval;
	GError *error = NULL;

	if (!games_runtime_init ("gnometris"))
		return 1;

#ifdef ENABLE_SETGID
	setgid_io_init ();
#endif

	int cmdlineLevel = 0;

	const GOptionEntry options[] =
	{
		{"level", 'l', 0, G_OPTION_ARG_INT, &cmdlineLevel, N_("Set starting level (1 or greater)"), N_("LEVEL")},
		{NULL}
	};

	GOptionContext *context = g_option_context_new (NULL);

	g_option_context_add_group (context, gtk_get_option_group (TRUE));
	g_option_context_add_group (context, clutter_get_option_group_without_init ());
	g_option_context_add_main_entries (context, options, GETTEXT_PACKAGE);

	retval = g_option_context_parse (context, &argc, &argv, &error);
	g_option_context_free (context);
	if (!retval) {
		g_print ("%s", error->message);
		g_error_free (error);
		return 1;
	}

	g_set_application_name (_("Gnometris"));

	gtk_window_set_default_icon_name ("gnome-gnometris");

	games_conf_initialise ("Gnometris");

	if (clutter_init_with_args (NULL, NULL, NULL, NULL, NULL, &error) != CLUTTER_INIT_SUCCESS) {
		g_printerr ("Failed to initialise clutter: %s\n", error->message);
		g_error_free (error);
		return 1;
	}

	Tetris *t = new Tetris(cmdlineLevel);

	gtk_main();

	delete t;

	games_conf_shutdown ();

	games_runtime_shutdown ();

	return 0;
}
