## Process this file with automake to produce Makefile.in.

SUBDIRS = pixmaps cursors help

# This is where the high score file goes ...
scoredir = $(localstatedir)/games

INCLUDES = $(GNOME_GAMES_CFLAGS)			\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-DDATADIR=\""$(datadir)"\"		   \
	-DG_DISABLE_DEPRECATED -DGDK_DISABLE_DEPRECATED	\
	-DGTK_DISABLE_DEPRECATED -DGNOME_DISABLE_DEPRECATED
bin_PROGRAMS = gnome-xbill

gnome_xbill_SOURCES = Bucket.cc Bucket.h Cable.cc Cable.h Computer.cc \
                      Computer.h Game.cc Game.h Horde.cc Horde.h Library.cc \
                      Library.h MCursor.cc MCursor.h Monster.cc Monster.h \
                      Network.cc Network.h Picture.cc Picture.h Spark.cc \
                      Spark.h Strings.h UI.cc UI.h objects.h x11-gtk.cc x11.h

gnome_xbill_LDADD = $(GNOME_GAMES_LIBS)

EXTRA_DIST = README.Ports xbill.png xbill.desktop.in

pixmapdir = $(datadir)/pixmaps
pixmap_DATA = xbill.png

Gamesdir=$(datadir)/applications
Games_in_files = xbill.desktop.in
Games_DATA = $(Games_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(scoredir)
	-touch $(DESTDIR)$(scoredir)/gnome-xbill.scores
	-chown games.games $(DESTDIR)$(scoredir)/gnome-xbill.scores
	-chmod 664 $(DESTDIR)$(scoredir)/gnome-xbill.scores
	-chgrp games $(DESTDIR)$(bindir)/gnome-xbill && chmod 2111 $(DESTDIR)$(bindir)/gnome-xbill

