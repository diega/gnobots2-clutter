


mapdir = $(datadir)/gnome-games/mahjongg/maps
pixmapdir = $(datadir)/pixmaps/mahjongg
Gamesdir = $(datadir)/applications
appicondir=$(datadir)/pixmaps

schemadir   = @GCONF_SCHEMA_FILE_DIR@
schema_in_files = mahjongg.schemas.in
schema_DATA = $(schema_in_files:.schemas.in=.schemas)
@INTLTOOL_SCHEMAS_RULE@

INCLUDES =						\
	-I$(top_srcdir)/libgames-support		\
	-I..						\
	-I$(srcdir)/..					\
	$(GNOME_GAMES_CFLAGS)				\
	-DDATADIR=\""$(datadir)"\"		   	\
	-DMAPDIR=\""$(mapdir)"\"			\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"

bin_PROGRAMS = mahjongg

mahjongg_SOURCES = 	\
	mahjongg.c	\
	mahjongg.h	\
	drawing.c	\
	drawing.h	\
	maps.c		\
	maps.h		\
	solubility.c	\
	solubility.h	

mahjongg_LDADD =					\
	$(GNOME_GAMES_LIBS)				\
	../libgames-support/libgames-support.la		\
	$(INTLLIBS)

map_DATA = mahjongg.map

pixmap_DATA = smooth.png postmodern.svg

appicon_DATA = gnome-mahjongg.png

EXTRA_DIST = mahjongg.desktop.in $(pixmap_DATA) $(appicon_DATA) \
	$(schema_in_files) $(map_DATA)

Games_in_files = mahjongg.desktop.in
Games_DATA = $(Games_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

SUBDIRS = help

install-data-local:
if GCONF_SCHEMAS_INSTALL
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(top_builddir)/mahjongg/$(schema_DATA)
endif
	-$(mkinstalldirs) $(DESTDIR)$(scoredir)
	-for i in easy difficult confounding pyramid tictactoe cloud dragon bridges ziggurat; do \
		touch $(DESTDIR)$(scoredir)/mahjongg.$$i.scores; \
		chown $(scores_user):$(scores_group) $(DESTDIR)$(scoredir)/mahjongg.$$i.scores; \
		chmod 664 $(DESTDIR)$(scoredir)/mahjongg.$$i.scores; \
	done
	-if test "x$(setgid)" = "xtrue"; then chgrp $(scores_group) $(DESTDIR)$(bindir)/mahjongg && chmod 2555 $(DESTDIR)$(bindir)/mahjongg ; fi
	$(mkinstalldirs) $(DESTDIR)$(Gamesdir)

DISTCLEANFILES = $(Games_DATA) $(schema_DATA)
