scoredir=$(localstatedir)/games
pixmapdir=$(datadir)/pixmaps/gnobots
appicondir=$(datadir)/pixmaps
Gamesdir = $(datadir)/apps/Games

SUBDIRS = help

INCLUDES = 			\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	-I$(includedir)				 \
	$(GNOME_INCLUDEDIR)

bin_PROGRAMS = gnobots

gnobots_SOURCES = \
	gnobots.c \
	gnobots.h \
	properties.c \
	properties.h \
	game.c \
	game.h \
	gbdefs.h

gnobots_LDADD = \
	$(GNOME_LIBDIR) \
	$(GNOMEUI_LIBS)	\
	$(INTLLIBS)

appicon_DATA = gnome-gnobots.png

pixmap_DATA = gnomes.png \
              robots.png \
              windows.png \
              cows.png \
              eggs.png \
              mice.png \
              aieee.png \
              yahoo.png

Games_DATA = gnobots.desktop

EXTRA_DIST = README \
             AUTHORS \
             gnobots.desktop \
             $(appicon_DATA) \
             $(pixmap_DATA)

install-data-local:
	-$(mkinstalldirs) $(scoredir)
	-touch $(scoredir)/gnobots.unsafe.scores
	-chown games.games $(scoredir)/gnobots.unsafe.scores
	-chmod 664 $(scoredir)/gnobots.unsafe.scores
	-touch $(scoredir)/gnobots.safe.scores
	-chown games.games $(scoredir)/gnobots.safe.scores
	-chmod 664 $(scoredir)/gnobots.safe.scores
	-chgrp games $(bindir)/gnobots && chmod 2111 $(bindir)/gnobots

