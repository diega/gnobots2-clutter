SUBDIRS=pix
scoredir=$(localstatedir)/games
Gamesdir=$(datadir)/applications

INCLUDES=\
	$(GNOME_GAMES_CFLAGS)		\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" 
	

bin_PROGRAMS=gnometris

gnometris_SOURCES=\
	main.cpp \
	blocks.cpp \
	blocks.h \
	preview.cpp \
	preview.h \
	blockops.cpp \
	blockops.h \
	field.cpp \
	field.h \
	tetris.cpp \
	tetris.h \
	scoreframe.cpp \
	scoreframe.h	\
	gnome-canvas-pimage.h \
	gnome-canvas-pimage.c

gnometris_LDADD=\
	$(GNOME_GAMES_LIBS) \
	$(INTLLIBS)


Games_in_files = gnometris.desktop.in
Games_DATA = $(Games_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST = AUTHORS \
             gnometris.desktop.in 	

install-data-local:
	-$(mkinstalldirs) $(DESTDIR)$(scoredir)
	touch $(DESTDIR)$(scoredir)/gnometris.scores
	-chown games.games $(DESTDIR)$(scoredir)/gnometris.scores
	-chmod 664 $(DESTDIR)$(scoredir)/gnometris.scores
	-chgrp games $(DESTDIR)$(bindir)/gnometris && chmod 2111 $(DESTDIR)$(bindir)/gnometris







