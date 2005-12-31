themedir=$(datadir)/gnome-games/ataxx/themes/2.14/
Gamesdir = $(datadir)/applications
appicondir=$(datadir)/pixmaps

INCLUDES = 			\
	-I$(top_srcdir)/libgames-support \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	-DDATADIR=\""$(datadir)"\"		   \
	-DTHEMEDIR=\""$(themedir)"\" \
	$(CAIRO_CFLAGS) \
	$(GNOME_GAMES_CFLAGS)

bin_PROGRAMS = gataxx

gataxx_SOURCES = \
	ai.c \
	ai.h \
	appbar.c \
	appbar.h \
	gtkgridboard.c \
	gtkgridboard.h \
	gtkgridboardthemes.c \
	gtkgridboardthemes.h \
	menus.c \
	menus.h \
	gataxx.c \
	gataxx.h \
	properties.c \
	properties.h

gataxx_LDADD = \
	../libgames-support/libgames-support.la \
	$(GNOME_GAMES_LIBS) \
	$(CAIRO_LIBS)

theme_DATA=classic.png
appicon_DATA = gataxx.png

Games_in_files = gataxx.desktop.in
Games_DATA = $(Games_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

schemadir   = @GCONF_SCHEMA_FILE_DIR@
schema_in_files = gataxx.schemas.in
schema_DATA = $(schema_in_files:.schemas.in=.schemas)
@INTLTOOL_SCHEMAS_RULE@

EXTRA_DIST = AUTHORS \
	gataxx.desktop.in 	\
	$(theme_DATA) \
	$(appicon_DATA) \
	$(schema_in_files)

SUBDIRS = help

DISTCLEANFILES = $(Games_DATA) $(schema_DATA) $(soundlist_DATA)

install-data-local:
if GCONF_SCHEMAS_INSTALL
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(top_builddir)/gataxx/$(schema_DATA)
endif
