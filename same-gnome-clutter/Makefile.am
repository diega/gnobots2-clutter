SUBDIRS = data/themes

samedir=$(pkgdatadir)/same-gnome-clutter

same_DATA = \
    src/About.js \
    src/Board.js \
    src/Light.js \
    src/main.js \
    src/Settings.js \
    data/same-gnome.ui \
    data/settings.ui \
    src/Score.js \
    src/Path.js \
    src/ThemeLoader.js

same_gnome_clutter_c_SOURCES = same.c
same_gnome_clutter_c_CFLAGS = `pkg-config --cflags clutter-0.9 glib-2.0`
same_gnome_clutter_c_LDFLAGS = `pkg-config --libs clutter-0.9 glib-2.0`

bin_SCRIPTS = same-gnome-clutter
bin_PROGRAMS = same-gnome-clutter-c

schema_in_files = same-gnome-clutter.schemas.in
schemadir = $(GCONF_SCHEMA_FILE_DIR)
schema_DATA = $(schema_in_files:.schemas.in=.schemas)

desktop_in_files = same-gnome-clutter.desktop.in.in same-gnome-clutter-c.desktop.in.in
desktopdir = $(datadir)/applications
desktop_DATA = $(desktop_in_files:.desktop.in.in=.desktop)
@INTLTOOL_DESKTOP_RULE@
        
CLEANFILES = $(desktop_DATA)

EXTRA_DIST = \
    src/About.js \
    src/Board.js \
    src/Light.js \
    src/main.js \
    src/Settings.js \
    data/same-gnome.ui \
    data/settings.ui \
    src/Score.js \
    src/Path.js.in \
    src/ThemeLoader.js


install-schemas-local: $(schema_DATA)
if GCONF_SCHEMAS_INSTALL
	if test -z "$(DESTDIR)" ; then \
		for p in $^ ; do \
			GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $$p 2>&1 > /dev/null; \
		done \
	fi
endif

install-data-local: install-schemas-local

@INTLTOOL_SCHEMAS_RULE@

