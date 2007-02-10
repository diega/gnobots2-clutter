EXTRA_DIST = README

if BUILD_TESTS
SUBDIRS_TESTS =
if HAVE_CHECK
SUBDIRS_CHECK = libgames-support
else
SUBDIRS_CHECK =
endif
else
SUBDIRS_TESTS =
SUBDIRS_CHECK =
endif

SUBDIRS = \
	$(SUBDIRS_CHECK) \
	$(SUBDIRS_TESTS) 

DIST_SUBDIRS = \
	aisleriot  \
	glines \
	gnobots2 \
	gnomine \
	gtali \
	mahjongg \
	blackjack \
	gnect \
	gnome-sudoku \
	gnotravex \
	iagno \
	glchess \
	gnibbles \
	gnometris \
	gnotski \
	libgames-support \
	same-gnome

