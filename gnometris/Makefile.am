SUBDIRS=pix
# This is awful but the scores directory is actually kept by libgnome
# and it may not have the same prefix as us.
scoredir=`pkg-config libgnome-2.0 --variable=prefix`/var/games
Gamesdir=$(datadir)/applications

INCLUDES=\
	$(GNOME_GAMES_CFLAGS)	\
	-I$(top_srcdir)/libgames-support \
	-DDATADIR=\""$(datadir)"\"		   \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-DG_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED	\
	-DGNOME_DISABLE_DEPRECATED

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
	../libgames-support/libgames-support.la \
	$(INTLLIBS)


Games_in_files = gnometris.desktop.in
Games_DATA = $(Games_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

schemadir   = @GCONF_SCHEMA_FILE_DIR@
schema_in_files = gnometris.schemas.in
schema_DATA = $(schema_in_files:.schemas.in=.schemas)
@INTLTOOL_SCHEMAS_RULE@

EXTRA_DIST = AUTHORS \
             gnometris.desktop.in	\
             $(schema_in_files)

install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(srcdir)/$(schema_DATA)
	-$(mkinstalldirs) $(DESTDIR)$(scoredir)
	touch $(DESTDIR)$(scoredir)/gnometris.scores
	-chown $(scores_user).$(scores_group) $(DESTDIR)$(scoredir)/gnometris.scores
	-chmod 664 $(DESTDIR)$(scoredir)/gnometris.scores
	-if test "x$(setgid)" = "xtrue"; then chgrp $(scores_group) $(DESTDIR)$(bindir)/gnometris && chmod 2555 $(DESTDIR)$(bindir)/gnometris ; fi

CLEANFILES = $(Games_DATA) $(schema_DATA)
