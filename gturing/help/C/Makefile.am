gturing_helpdir = $(datadir)/gnome/help/gturing/C

figs = \
	mainwin.png

gturing_help_DATA = 	\
	gturing.sgml		\
	index.html		\
	topic.dat

EXTRA_DIST = $(gturing_help_DATA) $(figs)

all: index.html

# No-op if db2html failed
index.html: gturing/index.html
	-cd $(srcdir) && cp gturing/index.html index.html

gturing/index.html: gturing.sgml
	-cd $(srcdir) && db2html gturing.sgml

dist-hook:
	-mkdir $(distdir)/gturing
	-mkdir $(distdir)/gturing/stylesheet-images
	cp $(srcdir)/gturing/*.html $(distdir)/gturing
	cp -R $(srcdir)/gturing/stylesheet-images/*.gif $(distdir)/gturing/stylesheet-images
	cp $(srcdir)/gturing/*.css $(distdir)/gturing

# No-op if db2html failed
install-data-local: index.html
	-$(mkinstalldirs) $(DESTDIR)$(gturing_helpdir)
	files=`echo $(srcdir)/gturing/*.html $(srcdir)/gturing/*.css $(srcdir)/*.png`; \
	if test "$$files" != '$(srcdir)/gturing/*.html $(srcdir)/gturing/*.css $(srcdir)/*.png'; then \
	  for file in $$files; do \
	    basefile=`echo $$file | sed -e 's,^.*/,,'`; \
	    $(INSTALL_DATA) $$file $(DESTDIR)$(gturing_helpdir)/$$basefile; \
	  done; \
	fi
	-$(mkinstalldirs) $(DESTDIR)$(gturing_helpdir)/stylesheet-images
	files=`echo $(srcdir)/gturing/stylesheet-images/*.gif`; \
	if test "$$files" != '$(srcdir)/gturing/stylesheet-images/*.gif'; then \
	  for file in $$files; do \
	    basefile=`echo $$file | sed -e 's,^.*/,,'`; \
	    $(INSTALL_DATA) $$file $(DESTDIR)$(gturing_helpdir)/stylesheet-images/$$basefile; \
	  done; \
	fi

gturing.ps: gturing.sgml
	db2ps $<

gturing.rtf: gturing.sgml
	db2rtf $<
