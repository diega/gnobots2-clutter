gturing_helpdir = $(datadir)/gnome/help/gturing/C

gturing_help_DATA = 	\
	index.html		\
	topic.dat

EXTRA_DIST = gturing.sgml topic.dat

all: index.html

index.html: gturing/index.html
	-cp gturing/index.html .

gturing/index.html: gturing.sgml
	-db2html gturing.sgml


dist-hook:
	mkdir $(distdir)/gturing
	-cp gturing/*.html $(distdir)/gturing
	-cp gturing/*.css $(distdir)/gturing

install-data-local: index.html
	-for file in gturing/*.html gturing/*.css; do \
	  basefile=`echo $$file | sed -e 's,^.*/,,'`; \
	  $(INSTALL_DATA) $(srcdir)/$$file $(gturing_helpdir)/$$basefile; \
	done

gturing.ps: gturing.sgml
	-db2ps $<

gturing.rtf: gturing.sgml
	-db2rtf $<
