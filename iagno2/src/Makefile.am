Gamesdir = $(datadir)/gnome/apps/Games

INCLUDES = \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	-I$(includedir) \
	$(GNOME_INCLUDEDIR) \
	$(GDK_PIXBUF_INCS) \
	-I$(srcdir)/../libiagno2

bin_PROGRAMS = iagno2

iagno2_SOURCES = \
	main.c \
	iagno2.c \
	plugin.c \
	preferences.c \
	properties.c

iagno2_LDADD = \
	../reversi-iagno2/libreversi-iagno2.la \
	$(GNOME_LIBDIR) \
	$(GNOMEUI_LIBS) \
	$(GDK_PIXBUF_LIBS) \
	$(INTLLIBS)
