Gamesdir = $(datadir)/apps/Games

INCLUDES = 			\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	-I$(includedir)				 \
	$(GNOME_INCLUDEDIR)

bin_PROGRAMS = gturing

gturing_SOURCES =	\
	gturing.c	\
	turing.h	\
	turing.c

gturing_LDADD = \
	$(GNOME_LIBDIR) \
	$(GNOMEUI_LIBS)	\
	$(INTLLIBS)

EXTRA_DIST = play.xpm power.xpm step.xpm stop.xpm \
	3ones2zeroes.tur add.tur addbin.tur bin2dec.tur dec2bin.tur subst.tur \
	README

install-data-local:
	-$(mkinstalldirs)
	-chgrp games $(bindir)/gturing && chmod 2111 $(bindir)/gturing
