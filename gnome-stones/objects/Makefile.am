GSTONES_SRC_DIR = $(srcdir)/../

objectdir= $(libdir)/gnome-stones/objects

INCLUDES = 						\
	-I$(GSTONES_SRC_DIR)				\
	$(GNOME_GAMES_CFLAGS)				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" 	\
	-DG_DISABLE_DEPRECATED                          \
	-DGTK_DISABLE_DEPRECATED 			\
	-I$(includedir)					\
	-DG_DISABLE_DEPRECATED				\
	-DGDK_DISABLE_DEPRECATED 			\
	-DGTK_DISABLE_DEPRECATED			\
	-DGNOME_DISABLE_DEPRECATED

object_LTLIBRARIES = libgnome-stones.la libgnomekoban.la

libgnome_stones_la_SOURCES = gnome-stones.c
libgnomekoban_la_SOURCES   = gnomekoban.c
