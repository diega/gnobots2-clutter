GSTONES_SRC_DIR = $(srcdir)/../

objectdir= $(libdir)/gnome-stones/objects

INCLUDES = 						\
	-I$(GSTONES_SRC_DIR)				\
	$(GNOME_GAMES_CFLAGS)				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" 	\
	-I$(includedir)

object_LTLIBRARIES = libgnome-stones.la libgnomekoban.la

libgnome_stones_la_SOURCES = gnome-stones.c
libgnome_stones_la_LDFLAGS = -module -avoid-version
libgnomekoban_la_SOURCES   = gnomekoban.c
libgnomekoban_la_LDFLAGS = -module -avoid-version

if PLATFORM_WIN32
libgnome_stones_la_LDFLAGS += -no-undefined -Wl,../.libs/gnome-stones.exe.a
libgnome_stones_la_LIBADD = $(GNOME_GAMES_LIBS)
libgnomekoban_la_LDFLAGS += -no-undefined -Wl,../.libs/gnome-stones.exe.a
libgnomekoban_la_LIBADD = $(GNOME_GAMES_LIBS)
endif
