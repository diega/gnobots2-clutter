scoredir=$(localstatedir)/games
pixmapdir = $(datadir)/pixmaps/same-gnome
Gamesdir = $(datadir)/applications

SUBDIRS = help

INCLUDES = -I.. -I$(srcdir)/..		\
	$(GNOME_GAMES_CFLAGS) \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	-DDATADIR=\""$(datadir)"\"		   \
	-DPIXMAPDIR=\""$(datadir)/pixmaps/same-gnome"\" \
	-DG_DISABLE_DEPRECATED -DGDK_DISABLE_DEPRECATED \
	-DGTK_DISABLE_DEPRECATED -DGNOME_DISABLE_DEPRECATED

bin_PROGRAMS = same-gnome

same_gnome_SOURCES = 	\
	same-gnome.c

same_gnome_LDADD =		\
	$(GNOME_GAMES_LIBS)      \
	$(INTLLIBS)

schemadir   = $(sysconfdir)/gconf/schemas
schema_in_files = same-gnome.schemas.in
schema_DATA = $(schema_in_files:.schemas.in=.schemas)
@INTLTOOL_SCHEMAS_RULE@

pixmap_DATA = planets.png blink-sync.png marbles.png \
	stones.png rotate.png glass.png
EXTRA_DIST = same-gnome.desktop.in $(pixmap_DATA) $(schema_DATA)

Games_in_files = same-gnome.desktop.in
Games_DATA = $(Games_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(srcdir)/$(schema_DATA)
	-$(mkinstalldirs) $(DESTDIR)$(scoredir)
	touch $(DESTDIR)$(scoredir)/same-gnome.scores
	-chown games.games $(DESTDIR)$(scoredir)/same-gnome.scores
	-chmod 664 $(DESTDIR)$(scoredir)/same-gnome.scores
	-chgrp games $(DESTDIR)$(bindir)/same-gnome && chmod 2555 $(DESTDIR)$(bindir)/same-gnome

CLEANFILES = $(Games_DATA) $(schema_DATA)
