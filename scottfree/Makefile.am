INCLUDES = 			\
	-I../.. 		\
	-I../gemvt		\
	-I$(srcdir)/../.. 	\
	-I$(srcdir)/..	 	\
	-I$(includedir)		\
	$(X_CFLAGS)

bin_PROGRAMS = GnomeScott

GnomeScott_SOURCES = \
	GnomeScott.c	\
	Scott.h

GnomeScott_LDADD = \
	$(top_builddir)/libgnomeui/libgnomeui.la 	\
	$(top_builddir)/libgnome/libgnome.la		\
	../gemvt/libgemvt.la				\
	$(GTK_LIBS)					\
        $(X_LIBS) $(x_libs)

EXTRA_DIST = 	\
	down.xpm	\
	east.xpm	\
	inventory.xpm	\
	north.xpm	\
	south.xpm	\
	up.xpm		\
	west.xpm	\
	Definition
	
