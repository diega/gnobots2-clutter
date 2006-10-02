themedir=$(datadir)/gnome-games/ataxx/themes/2.14/
Gamesdir = $(datadir)/applications

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
icon16dir = $(datadir)/icons/hicolor/16x16/apps/
icon16_DATA = 16x16/gnome-ataxx.png
icon22dir = $(datadir)/icons/hicolor/22x22/apps/
icon22_DATA = 22x22/gnome-ataxx.png
icon32dir = $(datadir)/icons/hicolor/32x32/apps/
icon32_DATA = 32x32/gnome-ataxx.png
icon48dir = $(datadir)/icons/hicolor/48x48/apps/
icon48_DATA = 48x48/gnome-ataxx.png
svgicondir = $(datadir)/icons/hicolor/scalable/apps/
svgicon_DATA = scalable/gnome-ataxx.svg

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
	$(icon16_DATA) \
	$(icon22_DATA) \
	$(icon32_DATA) \
	$(icon48_DATA) \
	$(svgicon_DATA) \
	$(schema_in_files)

SUBDIRS = help

DISTCLEANFILES = $(Games_DATA) $(schema_DATA) $(soundlist_DATA)

install-data-local:
if GCONF_SCHEMAS_INSTALL
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(top_builddir)/gataxx/$(schema_DATA)
endif
