SUBDIRS = reversi-iagno2 src tiles plugins

Gamesdir = $(datadir)/applications
icondir = $(datadir)/pixmaps

Games_in_files = iagno2.desktop.in
Games_DATA = $(Games_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

icon_DATA = iagno2.png

EXTRA_DIST = \
	$(Games_DATA) \
	$(icon_DATA) \
	AUTHORS \
	README

