## Process this file with automake to produce Makefile.in

Gamesdir  = $(datadir)/apps/Games

INCLUDES = 			\
	-I$(includedir)		\
	$(GNOME_INCLUDEDIR)

#-----------------------------------------------------------------------------
## Rules similar to `automake 1.2f' for building ObjC files, with dependencies
SUFFIXES = .m

OBJCFLAGS = $(CFLAGS)
OBJCCOMPILE = $(OBJC) $(DEFS) $(INCLUDES) $(CPPFLAGS) $(OBJCFLAGS)
LTOBJCCOMPILE = $(LIBTOOL) --mode=compile $(OBJCCOMPILE)

# ** This has to be called `LINK', not `OBJCLINK'
LINK = $(LIBTOOL) --mode=link $(OBJC) $(OBJCFLAGS) $(LDFLAGS) -o $@

gen_deps_o = @MAINT@-Wp,-MD,.deps/$(*F).P
gen_deps_lo = @MAINT@-Wp,-MD,.deps/$(*F).p

.m.o:
	$(OBJCCOMPILE) $(gen_deps_o) -c $<

.m.lo:
	$(LTOBJCCOMPILE) $(gen_deps_lo) -c $<
	@MAINT@@-sed -e 's/^\([^:]*\)\.o:/\1.lo \1.o:/' \
	@MAINT@	< .deps/$(*F).p > .deps/$(*F).P
	@MAINT@@-rm -f .deps/$(*F).p
#-----------------------------------------------------------------------------

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
	$(GNOME_LIBDIR) $(OBGNOME_LIBS) -lgtktty $(OBGTK_LIBS) $(GNOMEUI_LIBS) $(INTLLIBS) $(OBJC_LIBS)

EXTRA_DIST = phaser-chess.desktop

Games_DATA = phaser-chess.desktop

#phaser_chess_LDFLAGS = -version-info 0:0:0 -rpath $(libdir)

