scoredir=$(localstatedir)/games
pixmapdir = $(datadir)/pixmaps/mahjongg
Gamesdir = $(datadir)/apps/Games

INCLUDES = -I.. -I$(srcdir)/..		\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	-I$(includedir)			\
	$(GNOME_INCLUDEDIR)

bin_PROGRAMS = mahjongg

mahjongg_SOURCES = 	\
	mahjongg.c

mahjongg_LDADD = \
	$(GNOME_LIBDIR) \
	$(GNOMEUI_LIBS)	\
	$(INTLLIBS)

pixmap_DATA = default.xpm
inline_pixmaps = hborder.xpm vborder.xpm

EXTRA_DIST = mahjongg.desktop $(pixmap_DATA)

Games_DATA = mahjongg.desktop

install-data-local:
	-$(mkinstalldirs) $(scoredir)
	touch $(scoredir)/mahjongg.scores
	-chown games.games $(scoredir)/mahjongg.scores
	-chgrp games $(bindir)/mahjongg && chmod 2111 $(bindir)/mahjongg
	$(mkinstalldirs) $(Gamesdir)


