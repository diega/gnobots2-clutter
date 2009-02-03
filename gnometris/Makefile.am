SUBDIRS = pix help

bin_PROGRAMS=gnometris

gnometris_SOURCES = \
	main.cpp \
	blocks.cpp \
	blocks.h \
	highscores.cpp \
	highscores.h \
	scoreframe.cpp \
	scoreframe.h \
	tetris.cpp \
	tetris.h

if HAVE_CLUTTER
gnometris_SOURCES += \
	field.cpp \
	field.h \
	preview.cpp \
	preview.h \
	blockops.cpp \
	blockops.h \
	renderer.cpp \
	renderer.h
else
gnometris_SOURCES += \
	field-noclutter.cpp \
	field-noclutter.h \
	preview-noclutter.cpp \
	preview-noclutter.h \
	blockops-noclutter.cpp \
	blockops-noclutter.h \
	renderer-noclutter.cpp \
	renderer-noclutter.h
endif

gnometris_CPPFLAGS = \
	-I$(top_srcdir) \
	$(AM_CPPFLAGS)

gnometris_CXXFLAGS = \
	$(GTK_CFLAGS) \
	$(AM_CXXFLAGS)

gnometris_LDADD = \
	$(top_builddir)/libgames-support/libgames-support.la \
	$(GTK_LIBS) \
	$(INTLLIBS)

if HAVE_GNOME
gnometris_CXXFLAGS += $(GNOME_CFLAGS)
gnometris_LDADD += $(GNOME_LIBS)
endif

if HAVE_RSVG
gnometris_CXXFLAGS += $(RSVG_CFLAGS)
gnometris_LDADD += $(RSVG_LIBS)
endif

if WITH_GTHREAD
gnometris_CXXFLAGS += $(GHTREAD_CFLAGS)
gnometris_LDADD += $(GTHREAD_LIBS)
endif

if HAVE_CLUTTER
#noinst_PROGRAMS = gnometris-noclutter

#gnometris_noclutter_SOURCES = \
#	main.cpp \
#	blocks.cpp \
#	blocks.h \
#	preview-noclutter.cpp \
#	preview-noclutter.h \
#	blockops-noclutter.cpp \
#	blockops-noclutter.h \
#	field-noclutter.cpp \
#	field-noclutter.h \
#	highscores.cpp \
#	highscores.h \
#	renderer-noclutter.cpp \
#	renderer-noclutter.h \
#	scoreframe.cpp \
#	scoreframe.h \
#	tetris.cpp \
#	tetris.h

#gnometris_noclutter_CPPFLAGS = $(gnometris_CPPFLAGS) -UHAVE_CLUTTER
#gnometris_noclutter_CXXFLAGS = $(gnometris_CXXFLAGS) -UHAVE_CLUTTER
#gnometris_noclutter_LDADD = $(gnometris_LDADD)
gnometris_CPPFLAGS += -DHAVE_CLUTTER
gnometris_CXXFLAGS += $(CLUTTER_CFLAGS)
gnometris_LDADD += $(CLUTTER_LIBS)
endif

pixmapdir = $(pkgdatadir)/gnometris/pixmaps
pixmap_DATA = gnometris.svg

Gamesdir = $(datadir)/applications
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
