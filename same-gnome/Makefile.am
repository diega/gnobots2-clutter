scoredir=$(localstatedir)/games
pixmapdir = $(datadir)/pixmaps/samegnome
Gamesdir = $(datadir)/apps/Games

INCLUDES = 			\
	-I../.. 		\
	-I$(srcdir)/../.. 	\
	-I$(includedir)		\
	$(X_CFLAGS)

bin_PROGRAMS = samegnome

samegnome_SOURCES = 	\
	same-gnome.c

samegnome_LDADD = \
	$(top_builddir)/libgnomeui/libgnomeui.la 	\
	$(top_builddir)/libgnome/libgnome.la		\
	$(GTK_LIBS)					\
        $(X_LIBS) $(x_libs) $(INTLLIBS) $(DL_LIB)

pixmap_DATA = planets.xpm blink-sync.xpm puyonew2-sync.xpm stones.xpm rotate.xpm
EXTRA_DIST = same-gnome.desktop $(pixmap_DATA)

install-data-local:
	$(mkinstalldirs) $(Gamesdir)
	$(INSTALL_DATA) $(srcdir)/same-gnome.desktop $(Gamesdir)/same-gnome.desktop
	-touch $(scoredir)/samegnome.scores
	-chown games.games $(scoredir)/samegnome.scores
	-chmod 664 $(scoredir)/samegnome.scores

