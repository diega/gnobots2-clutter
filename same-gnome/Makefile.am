scoredir=$(localstatedir)/games
pixmapdir = $(datadir)/pixmaps/same-gnome
Gamesdir = $(datadir)/apps/Games

INCLUDES = 			\
	-I../.. 		\
	-I$(srcdir)/../.. 	\
	-I$(includedir)		\
	$(X_CFLAGS)

bin_PROGRAMS = same-gnome

same_gnome_SOURCES = 	\
	same-gnome.c

same_gnome_LDADD = \
	$(top_builddir)/libgnomeui/libgnomeui.la 	\
	$(top_builddir)/libgnome/libgnome.la		\
	$(top_builddir)/support/libsupport.a		\
	$(GTK_LIBS)					\
        $(X_LIBS) $(x_libs) $(INTLLIBS) $(DL_LIB)

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
