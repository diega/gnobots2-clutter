same_gnome_helpdir = $(datadir)/gnome/help/samegnome/C

same_gnome_help_DATA = 	\
	index.html		\
	topic.dat

EXTRA_DIST = same-gnome.sgml

all: index.html

index.html:same-gnome/book1.html
	cd $(srcdir) && cp same-gnome/book1.html index.html

same-gnome/book1.html: same-gnome.sgml
	-db2html same-gnome.sgml


dist-hook:
	mkdir $(distdir)/same-gnome
	cp same-gnome/*.html $(distdir)/same-gnome
	cp same-gnome/*.css $(distdir)/same-gnome

install-data-local: index.html
	for file in same-gnome/*.html same-gnome/*.css; do \
	  basefile=`echo $$file | sed -e 's,^.*/,,'`; \
	  $(INSTALL_DATA) $(srcdir)/$$file $(same_gnome_helpdir)/$$basefile; \
	done

same-gnome.ps: same-gnome.sgml
	db2ps $<

same-gnome.rtf: same-gnome.sgml
	db2rtf $<
