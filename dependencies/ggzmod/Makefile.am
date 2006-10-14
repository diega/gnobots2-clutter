EXTRA_DIST = Doxyfile

if INSTALL_GGZMOD
lib_LTLIBRARIES = libggzmod.la
include_HEADERS = ggzmod.h
else
noinst_LTLIBRARIES = libggzmod.la
endif

libggzmod_la_SOURCES = \
	ggzmod.c \
	ggzmod.h \
	io.c \
	io.h \
	mod.h \
	protocol.h

libggzmod_la_LDFLAGS = $(LIBGGZ_LDFLAGS) -version-info 4:0:0
libggzmod_la_LIBADD = ../ggzcore/libggzcore.la
if INSTALL_GGZMOD
libggzmod_la_LIBADD += $(LIB_GGZ)
endif

AM_CPPFLAGS = $(LIBGGZ_INCLUDES) -I$(srcdir)/../ggzcore
