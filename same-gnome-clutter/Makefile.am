samedir=$(pkgdatadir)/same-gnome-clutter

same_DATA = \
    bkg.svg \
    blue.svg \
    board.js \
    green.svg \
    light.js \
    main.js \
    red.svg \
    same-seed.ui \
    score.js \
    yellow.svg

same_gnome_clutter_c_SOURCES = same.c
same_gnome_clutter_c_CFLAGS = `pkg-config --cflags clutter-0.9 glib-2.0`
same_gnome_clutter_c_LDFLAGS = `pkg-config --libs clutter-0.9 glib-2.0`

bin_SCRIPTS = same-gnome-clutter
bin_PROGRAMS = same-gnome-clutter-c

desktop_in_files = same-gnome-clutter.desktop.in.in same-gnome-clutter-c.desktop.in.in
desktopdir = $(datadir)/applications
desktop_DATA = $(desktop_in_files:.desktop.in.in=.desktop)
@INTLTOOL_DESKTOP_RULE@
        
CLEANFILES = $(desktop_DATA)

EXTRA_DIST = \
    bkg.svg \
    blue.svg \
    board.js \
    green.svg \
    light.js.in \
    main.js \
    red.svg \
    same-seed.ui \
    score.js \
    yellow.svg \
    same.c
