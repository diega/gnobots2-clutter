#include "MCursor.h"
#include "objects.h"

void MCursor::load(const char *name, int masked) {
	static char *dir = gnome_unconditional_datadir_file("xbill/cursors");
	GdkPixmap *pixmap, *bitmap, *mask;
	int width, height;
	char *file;

	file = g_strdup_printf ("%s/%s.xpm", dir, name);

	pixmap = gdk_pixmap_colormap_create_from_xpm(ui.display, ui.colormap,
						     &bitmap, NULL, file);
	if (pixmap == NULL) {
		printf ("cannot open %s\n", file);
		exit(1);
	} else
	  gdk_pixmap_unref(pixmap);
	if (masked == SEP_MASK) {
	  g_free (file);
	  file = g_strdup_printf ("%s/%s_mask.xpm", dir, name);

	  pixmap = gdk_pixmap_colormap_create_from_xpm(ui.display, ui.colormap,
						       &mask, NULL, file);
	  if (pixmap == NULL) {
	    printf("cannot open %s\n", file);
	    exit(1);
	  } else
	    gdk_pixmap_unref(pixmap);
	} else
	  mask = bitmap;
	gdk_window_get_size(bitmap, &width, &height);
	cursor = gdk_cursor_new_from_pixmap(bitmap, mask, &ui.black, &ui.white,
					    width/2, height/2);
	g_free (file);
}

