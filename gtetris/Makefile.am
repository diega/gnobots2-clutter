SUBDIRS=pix
scoredir=$(localstatedir)/games
Gamesdir=$(datadir)/apps/Games

INCLUDES=$(GNOME_INCLUDEDIR)

bin_PROGRAMS=gtetris

gtetris_SOURCES=\
	main.cpp \
	main.h \
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
	scoreframe.h

gtetris_LDADD=\
	$(GNOME_LIBDIR) \
	$(GNOMEUI_LIBS) \
	$(INTLLIBS)


Games_DATA = gtetris.desktop

EXTRA_DIST = AUTHORS \
             gtetris.desktop 	

install-data-local:
	-$(mkinstalldirs) $(scoredir)
	touch $(scoredir)/gtetris.scores
	-chown games.games $(scoredir)/gtetris.scores
	-chmod 664 $(scoredir)/gtetris.scores
	-chgrp games $(bindir)/gtetris && chmod 2111 $(bindir)/gtetris







