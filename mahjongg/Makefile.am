scoredir=$(localstatedir)/games
pixmapdir = $(datadir)/pixmaps/mahjongg
Gamesdir = $(datadir)/gnome/apps/Games
appicondir=$(datadir)/pixmaps

INCLUDES = -I.. -I$(srcdir)/..		\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	-I$(includedir)			\
	$(GNOME_INCLUDEDIR)

bin_PROGRAMS = mahjongg

mahjongg_SOURCES = 	\
	mahjongg.c	\
	solubility.c	\
	solubility.h	\
	mahjongg.h	\
	button-images.h

mahjongg_LDADD = \
	$(GNOME_LIBDIR) \
	$(GNOMEUI_LIBS)	\
	$(INTLLIBS)

pixmap_DATA = default.png seta.png

appicon_DATA = gnome-mahjongg.png

EXTRA_DIST = mahjongg.desktop $(pixmap_DATA) $(appicon_DATA)

Games_DATA = mahjongg.desktop

install-data-local:
	-$(mkinstalldirs) $(scoredir)
	touch $(scoredir)/mahjongg.scores
	-chown games.games $(scoredir)/mahjongg.scores
	-chgrp games $(bindir)/mahjongg && chmod 2111 $(bindir)/mahjongg
	$(mkinstalldirs) $(Gamesdir)


