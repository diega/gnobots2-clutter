## Process this file with automake to produce Makefile.in.

EXTRA_DIST = index.html \
	     topic.dat

gnobots_helpdir = $(datadir)/gnome/help/gnobots/es

install-data-local:
	$(mkinstalldirs) $(gnobots_helpdir)
	$(INSTALL_DATA)  index.html $(gnobots_helpdir)
	$(INSTALL_DATA)  topic.dat $(gnobots_helpdir)

