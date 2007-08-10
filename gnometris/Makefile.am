SUBDIRS = pix help

Gamesdir=$(datadir)/applications
pixmapdir=$(datadir)/pixmaps/gnometris

AM_CPPFLAGS=\
	-I$(top_srcdir)/libgames-support \
	-DDATADIR=\""$(datadir)"\"		   \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"

AM_CXXFLAGS=\
	$(GNOME_GAMES_CXXFLAGS) $(ESD_CFLAGS)

AM_CFLAGS=\
	$(GNOME_GAMES_CFLAGS)

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
	highscores.cpp \
	highscores.h \
	renderer.cpp \
	renderer.h \
	scoreframe.cpp \
	scoreframe.h	\
	tetris.cpp \
	tetris.h

gnometris_LDADD=\
	$(top_builddir)/libgames-support/libgames-support.la \
	$(GNOME_GAMES_LIBS) \
	$(ESD_LIBS) \
	$(INTLLIBS)

pixmap_DATA = gnometris.svg

Games_in_files = gnometris.desktop.in.in
Games_DATA = $(Games_in_files:.desktop.in.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

schemadir   = @GCONF_SCHEMA_FILE_DIR@
schema_in_files = gnometris.schemas.in
schema_DATA = $(schema_in_files:.schemas.in=.schemas)
@INTLTOOL_SCHEMAS_RULE@

EXTRA_DIST = AUTHORS \
	     $(pixmap_DATA) \
             $(schema_in_files)

install-data-local:
if GCONF_SCHEMAS_INSTALL
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(top_builddir)/gnometris/$(schema_DATA)
endif
	-$(mkinstalldirs) $(DESTDIR)$(scoredir)
	-touch $(DESTDIR)$(scoredir)/gnometris.scores
	-chown $(scores_user):$(scores_group) $(DESTDIR)$(scoredir)/gnometris.scores
	-chmod 664 $(DESTDIR)$(scoredir)/gnometris.scores
	-if test "x$(setgid)" = "xtrue"; then chgrp $(scores_group) $(DESTDIR)$(bindir)/gnometris && chmod 2555 $(DESTDIR)$(bindir)/gnometris ; fi

DISTCLEANFILES = $(Games_DATA) $(schema_DATA)
