SUBDIRS = .

EXTRA_DIST = Doxyfile

if INSTALL_GGZCORE
lib_LTLIBRARIES = libggzcore.la
include_HEADERS = ggzcore.h
else
noinst_LTLIBRARIES = libggzcore.la
endif

libggzcore_la_CPPFLAGS = -I$(srcdir)/../ggzmod-ggz \
	$(LIBGGZ_INCLUDES) \
	-DGGZCONFDIR=\"$(sysconfdir)\" \
	-DPREFIX=\"$(prefix)\"

libggzcore_la_LIBADD = ../ggzmod-ggz/libggzmod-ggz.la \
		       $(LIBEXPAT)
if INSTALL_GGZCORE
libggzcore_la_LIBADD += $(LIB_GGZ)
endif

libggzcore_la_LDFLAGS = $(LIBGGZ_LDFLAGS) -version-info 8:1:1

libggzcore_la_SOURCES =	conf.c \
			conf.h \
			init.c \
			game.c \
			game.h \
			gametype.c \
			gametype.h \
			ggzcore.h \
			ggzcore-ggz.h \
			hook.c \
			hook.h \
			module.c \
			module.h \
			net.h \
			netxml.c \
			player.c \
			player.h \
			protocol.h \
			room.c \
			room.h \
			server.c \
			server.h \
			state.c \
			state.h \
			table.c \
			table.h
