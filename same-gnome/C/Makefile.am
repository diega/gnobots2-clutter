same_gnome_helpdir = $(datadir)/gnome/help/samegnome/C

same_gnome_help_DATA = 	\
	index.html		\
	topic.dat

EXTRA_DIST = same-gnome.sgml topic.dat

all: index.html

index.html: $(srcdir)/same-gnome/book1.html
	-cp $(srcdir)/same-gnome/book1.html index.html

$(srcdir)/same-gnome/book1.html: $(srcdir)/same-gnome.sgml
	-(cd $(srcdir); db2html same-gnome.sgml)

dist-hook:
	mkdir $(distdir)/same-gnome
	cp $(srcdir)/same-gnome/*.html $(distdir)/same-gnome
	cp $(srcdir)/same-gnome/*.css $(distdir)/same-gnome

install-data-local: index.html
	(cd $(srcdir); for file in same-gnome/*.html same-gnome/*.css; do \
	  basefile=`echo $$file | sed -e 's,^.*/,,'`; \
	  $(INSTALL_DATA) $$file $(same_gnome_helpdir)/$$basefile; \
	done)

same-gnome.ps: same-gnome.sgml
	-(cd $(srcdir); db2ps same-gnome.sgml)

same-gnome.rtf: same-gnome.sgml
	-(cd $(srcdir); db2rtf same-gnome.sgml)
