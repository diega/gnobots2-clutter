## Process this file with automake to produce Makefile.in
pixmapdir = $(datadipixmapdir = $(datadir)/pixmaps/phaser-chess
Gamesdir  = $(datadir)/apps/Games

INCLUDES = 			\
	-I../.. 		\
	-I$(srcdir)/../.. 	\
	-I$(srcdir)/../../obgtk	\
	-I$(includedir)		\
	$(X_CFLAGS)

SUFFIXES = .m

## FIXME: automake 1.2c doesn't define these if no pure C sources are
## seen.  So for now we define them.
#COMPILE = $(LIBTOOL) --mode=compile $(CC) $(DEFS) $(INCLUDES) $(CPPFLAGS) $(CFLAGS)
COMPILE = $(CC) $(DEFS) $(INCLUDES) $(CPPFLAGS) $(CFLAGS)
LINK = $(LIBTOOL) --mode=link $(CC) $(CFLAGS) $(LDFLAGS) -o $@



.m.o:
	$(COMPILE) -c $<

#.m.lo:
#	$(LIBTOOL) --mode=compile $(COMPILE) -c $<

bin_PROGRAMS = phaser_chess

phaser_chess_SOURCES = 		\
	board_view.m   		\
	coord_list.m		\
	logical_board.m		\
	phaser-chess.m		\
	board_view.h		\
	coord_list.h 		\
	enums.h			\
	logical_board.h		\
	objc_inc.h		\
	phaser-chess.h		\
	board_view_pc.h		\
	board_view_pc.m		\
	board_view_ps.h		\
	board_view_ps.m		\
	game_finder.h		\
	game_finder.m		\
	logical_board_pc.h	\
	logical_board_pc.m	\
	logical_board_ps.h	\
	logical_board_ps.m	\
	netcon.h		\
	netcon.m		\
	vector.h		\
	vector.m		\
	game_info.h		\
	game_info.m		\
	meta_server.h

phaser_chess_LDADD = \
	$(top_builddir)/libgnomeui/libgnomeui.la 	\
	$(top_builddir)/libgnome/libgnome.la		\
	$(top_builddir)/obgtk/libobgtk.la		\
        $(X_LIBS) $(x_libs)				\
	$(GTK_LIBS)					\
	-lobjc $(PTHREAD_LIB) $(INTLLIBS)

EXTRA_DIST = phaser-chess.desktop

install-data-local:
	$(mkinstalldirs) $(Gamesdir)
	$(INSTALL_DATA) $(srcdir)/phaser-chess.desktop $(datadir)/apps/Games/"Phaser Chess.desktop"


#phaser_chess_LDFLAGS = -version-info 0:0:0 -rpath $(libdir)

