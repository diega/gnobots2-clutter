
Gamesdir = $(datadir)/apps/Games
scoredir =$(localstatedir)/games
pixmapdir= $(datadir)/pixmaps/gnome-stones
Pixmapdir= $(datadir)/pixmaps
Cavesdir = $(datadir)/gnome-stones


INCLUDES =					\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-DGNOMEPIXMAPDIR=\""$(datadir)/pixmaps"\"	\
	-DCAVESDIR=\""$(Cavesdir)"\"			\
	-I$(includedir)					\
	$(GNOME_INCLUDEDIR)


bin_PROGRAMS = gnome-stones


gnome_stones_SOURCES =				\
        main.c					\
	types.h					\
	game.c					\
	game.h					\
	cave.c					\
	cave.h					\
	player.c				\
	player.h				\
	status.c				\
	status.h

gnome_stones_LDADD =				\
	$(GNOME_LIBDIR)				\
	$(GNOMEUI_LIBS)				\
	$(INTLLIBS)


pixmap_DATA= tiles.png title.png
Pixmap_DATA= gnome-stones.png
Games_DATA = gnome-stones.desktop
Caves_DATA = default.caves

EXTRA_DIST    = 				\
	$(pixmap_DATA)				\
	$(Pixmap_DATA)				\
	$(Games_DATA)				\
	$(Caves_DATA)				\
	title.xcf				\
	tiles.xcf				\
	TODO

install-data-local:
	-$(mkinstalldirs) $(scoredir)
	-touch $(scoredir)/gnome-stones.scores
	-chown games.games $(scoredir)/gnome-stones.scores
	-chmod 664 $(scoredir)/gnome-stones.scores	
	-chgrp games $(bindir)/gnome-stones && chmod 2111 $(bindir)/gnome-stones
