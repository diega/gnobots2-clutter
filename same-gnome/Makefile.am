


pixmapdir = $(datadir)/gnome-games/same-gnome/themes/2.10
sharedpixmapdir = $(datadir)/pixmaps
Gamesdir = $(datadir)/applications

SUBDIRS = help

INCLUDES = -I.. -I$(srcdir)/..		\
	-I$(top_srcdir)/libgames-support                \
	$(GNOME_GAMES_CFLAGS) \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	-DDATADIR=\""$(datadir)"\" \
	-DTHEMEDIR=\""$(pixmapdir)"\"

bin_PROGRAMS = same-gnome

same_gnome_SOURCES = 	\
	drawing.c \
	drawing.h \
	game.c \
	game.h \
	input.c \
	input.h \
	same-gnome.c \
	same-gnome.h \
	ui.c \
	ui.h

same_gnome_LDADD =		\
	../libgames-support/libgames-support.la \
	$(GNOME_GAMES_LIBS)      \
	$(INTLLIBS)

schemadir   = @GCONF_SCHEMA_FILE_DIR@
schema_in_files = same-gnome.schemas.in
schema_DATA = $(schema_in_files:.schemas.in=.schemas)
@INTLTOOL_SCHEMAS_RULE@

pixmap_DATA = planets.png stones.png
sharedpixmap_DATA = gnome-gsame.png
EXTRA_DIST = same-gnome.desktop.in $(pixmap_DATA) $(sharedpixmap_DATA) $(schema_in_files)

Games_in_files = same-gnome.desktop.in
Games_DATA = $(Games_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

SCOREFILES = Small Medium Large

install-data-local:
if GCONF_SCHEMAS_INSTALL
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(top_builddir)/same-gnome/$(schema_DATA)
endif
	-$(mkinstalldirs) $(DESTDIR)$(scoredir)
	## Migrate to the new scoring system (with a link for dual-version
	## installs).
	-if [ -f $(DESTDIR)$(scoredir)/same-gnome.scores -a ! -f $(DESTDIR)$(scoredir)/same-gnome.Small.scores ] ; then \
	  mv -f $(DESTDIR)$(scoredir)/same-gnome.scores $(DESTDIR)$(scoredir)/same-gnome.Small.scores ; \
	  ln -s $(DESTDIR)$(scoredir)/same-gnome.Small.scores $(DESTDIR)$(scoredir)/same-gnome.scores ; \
	fi
	-for i in ${SCOREFILES} ; do \
	  touch $(DESTDIR)$(scoredir)/same-gnome.$$i.scores; \
	  chown $(scores_user):$(scores_group) $(DESTDIR)$(scoredir)/same-gnome.$$i.scores; \
	  chmod 664 $(DESTDIR)$(scoredir)/same-gnome.$$i.scores; \
	done
	-if test "x$(setgid)" = "xtrue"; then chgrp $(scores_group) $(DESTDIR)$(bindir)/same-gnome && chmod 2555 $(DESTDIR)$(bindir)/same-gnome ; fi

DISTCLEANFILES = $(Games_DATA) $(schema_DATA)
