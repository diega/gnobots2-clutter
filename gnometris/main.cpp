/*
 * written by J. Marcin Gorycki <mgo@olicom.dk>
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

#include "tetris.h"

int nr_of_colors;

int
main(int argc, char *argv[])
{
	int cmdlineLevel = 0;

	poptOption options[] = 
	{
		{"level", 'l', POPT_ARG_INT, &cmdlineLevel, 0, N_("Set starting level (1-10)"), N_("LEVEL")},
		{NULL, '\0', 0, NULL, 0}
	};

	srand(time(NULL));

	gnome_score_init("gnometris");

	gnome_init_with_popt_table("gnometris", TETRIS_VERSION, argc, argv, options, 0, NULL);

	GnomeClient *client= gnome_master_client();
	
	char *pixname, *fullpixname;
	pixname = g_copy_strings( "gnometris/", "5blocks.png", NULL);
	fullpixname = gnome_unconditional_pixmap_file(pixname);
	g_free(pixname);

	if (!g_file_exists(fullpixname)) 
	{
		printf(_("Could not find the \'%s\' theme for gnometris\n"), fullpixname);
		exit(1);
	}

	GdkImlibImage *image = gdk_imlib_load_image(fullpixname);
	gdk_imlib_render(image, image->rgb_width, image->rgb_height);
	pix = gdk_imlib_move_image(image);

	nr_of_colors = image->rgb_width / BLOCK_SIZE;
	
	Tetris * t = new Tetris(cmdlineLevel);

	gtk_main();

	gdk_imlib_destroy_image(image);

	delete t;
	
	return 0;
}


