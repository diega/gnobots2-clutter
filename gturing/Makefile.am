SUBDIRS = help

Gamesdir = $(datadir)/apps/Games
examplesdir = $(datadir)/gturing/examples

INCLUDES = 			\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	-I$(includedir)				 \
	$(GNOME_INCLUDEDIR)			 \
	-DVERSION=\""$(VERSION)"\" \
	-DGTURING_EXAMPLES_DIR=\""$(examplesdir)"\"

bin_PROGRAMS = gturing

gturing_SOURCES =	\
	gturing.c	\
	turing.h	\
	turing.c

gturing_LDADD = \
	$(GNOME_LIBDIR) \
	$(GNOMEUI_LIBS)	\
	$(INTLLIBS)

examples_DATA = 3ones2zeroes.tur add.tur addbin.tur bin2dec.tur dec2bin.tur \
	subst.tur dashnumbers.tur bb2.tur bb3.tur bb5.tur

EXTRA_DIST = play.xpm power.xpm step.xpm stop.xpm \
	README CREDITS $(examples_DATA)

install-data-local:
	-$(mkinstalldirs)
	-chgrp games $(bindir)/gturing && chmod 2111 $(bindir)/gturing
