## Process this file with automake to produce Makefile.in

INCLUDES = $(GTK_CFLAGS) -Wall -ansi	\
	$(GNOME_INCLUDEDIR) -I$(srcdir)

appicondir=$(datadir)/pixmaps
appicon_DATA = gnome-metatris.png

sysdir   = $(datadir)/apps/Games
sys_DATA = metatris.desktop

mydatadir = $(datadir)/metatris/default-theme
mydata_DATA =	\
	metatris.theme		\
	metatris-piece-00.png	\
	metatris-piece-02.png	\
	metatris-piece-08.png	\
	metatris-piece-0a.png	\
	metatris-piece-0e.png	\
	metatris-piece-20.png	\
	metatris-piece-22.png	\
	metatris-piece-28.png	\
	metatris-piece-2a.png	\
	metatris-piece-38.png	\
	metatris-piece-3e.png	\
	metatris-piece-80.png	\
	metatris-piece-82.png	\
	metatris-piece-83.png	\
	metatris-piece-88.png	\
	metatris-piece-8a.png	\
	metatris-piece-8f.png	\
	metatris-piece-a0.png	\
	metatris-piece-a2.png	\
	metatris-piece-a8.png	\
	metatris-piece-e0.png	\
	metatris-piece-e3.png	\
	metatris-piece-f8.png	\
	metatris-piece-ff.png	\
	metatris-piece-00-01.png	\
	metatris-piece-02-01.png	\
	metatris-piece-08-01.png	\
	metatris-piece-0a-01.png	\
	metatris-piece-0e-01.png	\
	metatris-piece-20-01.png	\
	metatris-piece-22-01.png	\
	metatris-piece-28-01.png	\
	metatris-piece-2a-01.png	\
	metatris-piece-38-01.png	\
	metatris-piece-3e-01.png	\
	metatris-piece-80-01.png	\
	metatris-piece-82-01.png	\
	metatris-piece-83-01.png	\
	metatris-piece-88-01.png	\
	metatris-piece-8a-01.png	\
	metatris-piece-8f-01.png	\
	metatris-piece-a0-01.png	\
	metatris-piece-a2-01.png	\
	metatris-piece-a8-01.png	\
	metatris-piece-e0-01.png	\
	metatris-piece-e3-01.png	\
	metatris-piece-f8-01.png	\
	metatris-piece-ff-01.png	\
	metatris-bg.png		\
	metatris.png

EXTRA_DIST = $(mydata_DATA)	\
	metatris.desktop	\
	gnome-metatris.png

bin_PROGRAMS = metatris

metatris_SOURCES = 	\
	metatris.c	\
	metatris.h

metatris_LDFLAGS = $(GTK_LIBS)	\
	$(GNOME_LIBDIR) 	\
	$(GNOMEUI_LIBS)		\
	$(INTLLIBS)

CPPFLAGS = \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" 	\
	-DBINDIR=\""$(bindir)"\"	\
	-DDATADIR=\""$(datadir)"\"	\
	-DMYDATADIR=\""$(mydatadir)"\"

scoredir=$(localstatedir)/games

install-data-local:
	-$(mkinstalldirs) $(scoredir)
	-touch $(scoredir)/metatris.scores
	-chown games.games $(scoredir)/metatris.scores
	-chmod 664 $(scoredir)/metatris.scores
	-chgrp games $(bindir)/metatris && chmod 2111 $(bindir)/metatris
