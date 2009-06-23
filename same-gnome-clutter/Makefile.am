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

bin_SCRIPTS = same-gnome-clutter

desktop_in_files = same-gnome-clutter.desktop.in.in
desktopdir = $(datadir)/applications
desktop_DATA = $(desktop_in_files:.desktop.in.in=.desktop)
@INTLTOOL_DESKTOP_RULE@
        
CLEANFILES = $(desktop_DATA)

EXTRA_DIST = \
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

