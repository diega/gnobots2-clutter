scoredir=$(localstatedir)/games
pixmapdir=$(datadir)/pixmaps/gataxx
Gamesdir = $(datadir)/gnome/apps/Games
appicondir=$(datadir)/pixmaps

INCLUDES = 			\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	$(GNOME_GAMES_CFLAGS) \
	-I$(includedir)				

bin_PROGRAMS = gataxx

gataxx_SOURCES = \
	gataxx.c \
	gataxx.h \
	ataxx.c \
	ataxx.h \
	properties.c \
	properties.h \
	clock.c	\
	clock.h

gataxx_LDADD = $(GNOME_GAMES_LIBS)

pixmap_DATA = \
	blend.png \
	classic.png \
	flipblend.png \
	metal.png \
	metal2.png \
	shrink.png \
	woodtrim.png

appicon_DATA = gataxx.png

Games_DATA = gataxx.desktop

EXTRA_DIST = AUTHORS \
             gataxx.desktop 	\
	     gataxx.soundlist \
             $(pixmap_DATA)	\
	     $(appicon_DATA)

soundlistdir = $(sysconfdir)/sound/events
soundlist_DATA = gataxx.soundlist

SUBDIRS = sounds help
