mahjongg_helpdir = $(datadir)/gnome/help/mahjongg/C

figs = \
	color.png \
	confirm.png \
	mahjongg.png \
	no-match.png \
	no-move.png \
	preferences.png \
	scores.png \
	seed.png \
	toolbar.png

mahjongg_help_DATA = 	\
	mahjongg.sgml		\
	index.html		\
	topic.dat

EXTRA_DIST = $(mahjongg_help_DATA) $(figs)

all: index.html

# No-op if db2html failed
index.html: mahjongg/index.html
	-cd $(srcdir) && cp mahjongg/index.html index.html

mahjongg/index.html: mahjongg.sgml
	-cd $(srcdir) && db2html mahjongg.sgml

dist-hook:
	-mkdir $(distdir)/mahjongg
	-mkdir $(distdir)/mahjongg/stylesheet-images
	cp $(srcdir)/mahjongg/*.html $(distdir)/mahjongg
	cp -R $(srcdir)/mahjongg/stylesheet-images/*.gif $(distdir)/mahjongg/stylesheet-images
	cp $(srcdir)/mahjongg/*.css $(distdir)/mahjongg

# No-op if db2html failed
install-data-local: index.html
	-$(mkinstalldirs) $(DESTDIR)$(mahjongg_helpdir)
	files=`echo $(srcdir)/mahjongg/*.html $(srcdir)/mahjongg/*.css $(srcdir)/*.png`; \
	if test "$$files" != '$(srcdir)/mahjongg/*.html $(srcdir)/mahjongg/*.css $(srcdir)/*.png'; then \
	  for file in $$files; do \
	    basefile=`echo $$file | sed -e 's,^.*/,,'`; \
	    $(INSTALL_DATA) $$file $(DESTDIR)$(mahjongg_helpdir)/$$basefile; \
	  done; \
	fi
	-$(mkinstalldirs) $(DESTDIR)$(mahjongg_helpdir)/stylesheet-images
	files=`echo $(srcdir)/mahjongg/stylesheet-images/*.gif`; \
	if test "$$files" != '$(srcdir)/mahjongg/stylesheet-images/*.gif'; then \
	  for file in $$files; do \
	    basefile=`echo $$file | sed -e 's,^.*/,,'`; \
	    $(INSTALL_DATA) $$file $(DESTDIR)$(mahjongg_helpdir)/stylesheet-images/$$basefile; \
	  done; \
	fi

mahjongg.ps: mahjongg.sgml
	db2ps $<

mahjongg.rtf: mahjongg.sgml
	db2rtf $<
