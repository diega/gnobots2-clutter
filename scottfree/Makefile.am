INCLUDES = 			\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	-I$(includedir)

bin_PROGRAMS = GnomeScott

GnomeScott_SOURCES = \
	GnomeScott.c	\
	Scott.h

GnomeScott_LDADD = \
	-lgtktty $(GNOMEUI_LIBS) $(INTLLIBS)

EXTRA_DIST = 	\
	down.xpm	\
	east.xpm	\
	inventory.xpm	\
	north.xpm	\
	south.xpm	\
	up.xpm		\
	west.xpm	\
	Definition
