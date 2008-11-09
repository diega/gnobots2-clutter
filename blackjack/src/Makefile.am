NULL =

bin_PROGRAMS = blackjack

blackjack_SOURCES = 	\
	blackjack.cpp	\
	blackjack.h	\
	card.cpp	\
	card.h		\
	chips.cpp	\
	chips.h		\
	dialog.cpp	\
	dialog.h	\
	draw.cpp	\
	draw.h		\
	events.cpp	\
	events.h	\
	game.cpp	\
	game.h		\
	hand.cpp	\
	hand.h		\
	menu.cpp	\
	menu.h		\
        player.cpp	\
	player.h	\
	press_data.cpp  \
	press_data.h    \
	slot.cpp	\
	slot.h		\
	splash.cpp	\
	splash.h	\
	strategy.cpp	\
	strategy.h	\
	$(NULL)

blackjack_CPPFLAGS = \
	-I$(top_srcdir) \
	$(AM_CPPFLAGS)

blackjack_CXXFLAGS = \
	$(GNOME_GAMES_CXXFLAGS) \
	$(XML_CFLAGS) \
	$(AM_CXXFLAGS)

blackjack_LDADD = \
	$(top_builddir)/libgames-support/libgames-support.la \
	$(GNOME_GAMES_LIBS) \
	$(XML_LIBS) \
	-lz
# FIXMEchpe: what's this -lz about??
