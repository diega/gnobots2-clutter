scoredir=$(localstatedir)/games
pixmapdir = $(datadir)/pixmaps/same-gnome
Gamesdir = $(datadir)/apps/Games

INCLUDES = 			\
	-I$(includedir)		\
	$(X_CFLAGS)

bin_PROGRAMS = same-gnome

same_gnome_SOURCES = 	\
	same-gnome.c

same_gnome_LDADD = \
	$(GNOMEUI_LIBS)

pixmap_DATA = planets.xpm blink-sync.xpm puyonew2-sync.xpm \
	stones.xpm rotate.xpm
EXTRA_DIST = same-gnome.desktop $(pixmap_DATA)

install-data-local:
	-$(mkinstalldirs) $(Gamesdir)
	$(INSTALL_DATA) $(srcdir)/same-gnome.desktop $(Gamesdir)/same-gnome.desktop
	-$(mkinstalldirs) $(scoredir)
	touch $(scoredir)/same-gnome.scores
	-chown games.games $(scoredir)/same-gnome.scores
	-chmod 664 $(scoredir)/same-gnome.scores
	-chgrp games $(bindir)/same-gnome && chmod 2111 $(bindir)/same-gnome
