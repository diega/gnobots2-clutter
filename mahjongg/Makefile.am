scoredir=$(localstatedir)/games
pixmapdir = $(datadir)/pixmaps/mahjongg
bgdir = $(datadir)/pixmaps/mahjongg/bg
Gamesdir = $(datadir)/applications
appicondir=$(datadir)/pixmaps

INCLUDES =						\
	-I$(top_srcdir)/libgames-support		\
	-I..						\
	-I$(srcdir)/..					\
	$(GNOME_GAMES_CFLAGS)				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"

bin_PROGRAMS = mahjongg

mahjongg_SOURCES = 	\
	mahjongg.c	\
	solubility.c	\
	solubility.h	\
	mahjongg.h	\
	button-images.h \
	gnome-canvas-pimage.h \
	gnome-canvas-pimage.c

mahjongg_LDADD =					\
	$(GNOME_GAMES_LIBS)				\
	../libgames-support/libgames-support.la		\
	$(INTLLIBS)

pixmap_DATA = default.png seta.png

bg_DATA = bg1.png bg2.png bg3.png bg4.png

appicon_DATA = gnome-mahjongg.png

EXTRA_DIST = mahjongg.desktop.in $(pixmap_DATA) $(appicon_DATA) $(bg_DATA)

Games_in_files = mahjongg.desktop.in
Games_DATA = $(Games_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

SUBDIRS = help

install-data-local:
	-$(mkinstalldirs) $(DESTDIR)$(scoredir)
	$(mkinstalldirs) $(DESTDIR)$(bgdir)
	touch $(DESTDIR)$(scoredir)/mahjongg.{easy,difficult}.scores
	-chown games.games $(DESTDIR)$(scoredir)/mahjongg.{easy,difficult}.scores
	-chmod 664 $(DESTDIR)$(scoredir)/mahjongg.{easy,difficult}.scores
	-chgrp games $(DESTDIR)$(bindir)/mahjongg && chmod 2111 $(DESTDIR)$(bindir)/mahjongg
	$(mkinstalldirs) $(DESTDIR)$(Gamesdir)



