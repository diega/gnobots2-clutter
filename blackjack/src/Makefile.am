scoredir=$(localstatedir)/games

INCLUDES =  \
	-I$(top_srcdir)/gdk-card-image             \
	-I$(top_srcdir)/libgames-support           \
	-DDATADIR=\""$(datadir)"\"		   \
	$(GNOME_GAMES_CFLAGS)			   \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"   \
	-DG_DISABLE_DEPRECATED -DGDK_DISABLE_DEPRECATED \
	-DGTK_DISABLE_DEPRECATED -DGNOME_DISABLE_DEPRECATED

bin_PROGRAMS = blackjack

blackjack_SOURCES = 		\
	strategy.h	\
	strategy.cpp	\
	player.h	\
        player.cpp	\
	hand.h		\
	hand.cpp	\
	game.h		\
	game.cpp	\
	blackjack.h	\
	blackjack.cpp	\
	splash.h	\
	splash.cpp	\
	slot.h		\
	slot.cpp	\
	dialog.h	\
	dialog.cpp	\
	events.h	\
	events.cpp	\
	draw.h		\
	draw.cpp	\
	menu.h		\
	menu.cpp	\
	press_data.h    \
	press_data.cpp  \
	card.h		\
	card.cpp	\
	chips.h		\
	chips.cpp

blackjack_LDADD = $(GNOME_GAMES_LIBS) \
	$(top_srcdir)/gdk-card-image/libgdkcardimage.la \
	$(top_srcdir)/libgames-support/libgames-support.la \
	@UCB_LDFLAGS@

EXTRA_DIST = AUTHORS \
