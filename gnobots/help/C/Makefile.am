## Process this file with automake to produce Makefile.in.

EXTRA_DIST = index.html \
	     topic.dat

gnobots_helpdir = $(datadir)/gnome/help/gnobots/C

install-data-local:
	$(mkinstalldirs) $(gnobots_helpdir)
	$(INSTALL_DATA)  $(srcdir)/index.html $(gnobots_helpdir)
	$(INSTALL_DATA)  $(srcdir)/topic.dat $(gnobots_helpdir)

