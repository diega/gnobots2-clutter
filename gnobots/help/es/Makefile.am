## Process this file with automake to produce Makefile.in.

EXTRA_DIST = index.html \
	     topic.dat

gnobots_helpdir = $(datadir)/gnome/help/gnobots/es

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(gnobots_helpdir)
	$(INSTALL_DATA)  $(srcdir)/index.html $(DESTDIR)$(gnobots_helpdir)
	$(INSTALL_DATA)  $(srcdir)/topic.dat $(DESTDIR)$(gnobots_helpdir)

