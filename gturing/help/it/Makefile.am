gturing_helpdir = $(datadir)/gnome/help/gturing/it

gturing_help_DATA = 	\
	gturing.sgml		\
	index.html		\
	topic.dat

EXTRA_DIST = topic.dat

all: index.html

index.html: gturing/index.html
	-cp gturing/index.html .

gturing/index.html: gturing.sgml
	-db2html gturing.sgml


dist-hook:
	-$(mkinstalldirs) $(distdir)/gturing
	-cp gturing/*.html $(distdir)/gturing
	-cp gturing/*.css $(distdir)/gturing

install-data-local: index.html
	-$(mkinstalldirs) $(DESTDIR)$(gturing_helpdir)
	-for file in gturing/*.html gturing/*.css; do \
	  basefile=`echo $$file | sed -e 's,^.*/,,'`; \
	  $(INSTALL_DATA) $(srcdir)/$$file $(DESTDIR)$(gturing_helpdir)/$$basefile; \
	done

gturing.ps: gturing.sgml
	-db2ps $<

gturing.rtf: gturing.sgml
	-db2rtf $<
