SUBDIRS  = objects graphics sounds help

Gamesdir = $(datadir)/applications
scoredir =$(localstatedir)/games
Pixmapdir= $(datadir)/pixmaps
Cavesdir = $(datadir)/gnome-stones


INCLUDES =						\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-DGNOMEPIXMAPDIR=\""$(datadir)/pixmaps"\"	\
	-DG_DISABLE_DEPRECATED				\
	-DGDK_DISABLE_DEPRECATED 			\
	-DGTK_DISABLE_DEPRECATED 			\
	-DGNOME_DISABLE_DEPRECATED			\
	-DCAVESDIR=\""$(Cavesdir)"\"			\
	$(GNOME_GAMES_CFLAGS)				\
	-I$(includedir)					
	

bin_PROGRAMS = gnome-stones


gnome_stones_SOURCES =				\
        main.c					\
	main.h					\
	types.h					\
	game.c					\
	game.h					\
	cave.c					\
	cave.h					\
	object.c				\
	object.h				\
	player.c				\
	player.h				\
	status.c				\
	status.h                                \
	io.c                                    \
	io.h                                    \
	preferences.c                           \
	preferences.h                           \
	view.c                                  \
	view.h					\
	sound.c					\
	sound.h				       

gnome_stones_LDADD =				\
	$(GNOME_GAMES_LIBS) \
	$(INTLLIBS)

gnome_stones_LDFLAGS=-export-dynamic

Pixmap_DATA = gnome-stones.png gnome-stones-file.png
Games_in_files = gnome-stones.desktop.in
Games_DATA = $(Games_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

Caves_in_files = default.caves.in tutorial.caves.in gnomekoban.caves.in atari.caves.in
Caves_DATA = $(Caves_in_files:.caves.in=.caves)
@INTLTOOL_CAVES_RULE@

gtkrcdir = $(datadir)
gtkrc_DATA = gnome-stonesrc gnome-stonesrc.ko

Mimedir    = $(datadir)/mime-info
Mime_DATA  = gnome-stones.keys gnome-stones.mime

EXTRA_DIST    =					\
	$(Pixmap_DATA)				\
	$(Games_in_files)			\
	$(Caves_in_files)			\
	$(gtkrc_DATA)				\
	TODO					\
	README					\
	gnome-stones.keys.in			\
	gnome-stones.mime			\
	gnome-stones-file.png

install-data-local:
	-$(mkinstalldirs) $(DESTDIR)$(scoredir)
	-touch $(DESTDIR)$(scoredir)/gnome-stones.scores
	-chown games.games $(DESTDIR)$(scoredir)/gnome-stones.scores
	-chmod 664 $(DESTDIR)$(scoredir)/gnome-stones.scores	
	-chgrp games $(DESTDIR)$(bindir)/gnome-stones && chmod 2111 $(DESTDIR)$(bindir)/gnome-stones

gnome-stones.keys: gnome-stones.keys.in Makefile
	sed -e 's^\@icondir\@^$(Pixmapdir)^g' < $(srcdir)/gnome-stones.keys.in > gnome-stones.keys.tmp \
	&& mv gnome-stones.keys.tmp gnome-stones.keys 






