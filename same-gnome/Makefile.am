scoredir=$(localstatedir)/games
pixmapdir = $(datadir)/pixmaps/same-gnome
Gamesdir = $(datadir)/apps/Games

INCLUDES = -I.. -I$(srcdir)/..		\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	-I$(includedir)			\
	$(GNOME_INCLUDEDIR)

bin_PROGRAMS = same-gnome

same_gnome_SOURCES = 	\
	same-gnome.c

same_gnome_LDADD =		\
	$(GNOME_LIBDIR)         \
	$(GNOMEUI_LIBS)		\
	$(INTLLIBS)		\
	$(top_builddir)/support/libsupport.a

pixmap_DATA = planets.png blink-sync.png marbles.png \
	stones.png rotate.png
EXTRA_DIST = same-gnome.desktop $(pixmap_DATA)

Games_DATA = same-gnome.desktop

install-data-local:
	-$(mkinstalldirs) $(scoredir)
	touch $(scoredir)/same-gnome.scores
	-chown games.games $(scoredir)/same-gnome.scores
	-chmod 664 $(scoredir)/same-gnome.scores
	-chgrp games $(bindir)/same-gnome && chmod 2111 $(bindir)/same-gnome
