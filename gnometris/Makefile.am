SUBDIRS=pix
scoredir=$(localstatedir)/games
Gamesdir=$(datadir)/apps/Games

INCLUDES=$(GNOME_INCLUDEDIR)

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
	scoreframe.h

gnometris_LDADD=\
	$(GNOME_LIBDIR) \
	$(GNOMEUI_LIBS) \
	$(INTLLIBS)


Games_DATA = gnometris.desktop

EXTRA_DIST = AUTHORS \
             gnometris.desktop 	

install-data-local:
	-$(mkinstalldirs) $(scoredir)
	touch $(scoredir)/gnometris.scores
	-chown games.games $(scoredir)/gnometris.scores
	-chmod 664 $(scoredir)/gnometris.scores
	-chgrp games $(bindir)/gnometris && chmod 2111 $(bindir)/gnometris







