SUBDIRS = security

if INSTALL_LIBGGZ
lib_LTLIBRARIES = libggz.la
include_HEADERS = ggz.h ggz_common.h ggz_dio.h
else
noinst_LTLIBRARIES = libggz.la
endif

libggz_la_LIBADD = $(LDADD) security/libggzsecurity.la $(LIB_ASYNC)

libggz_la_LDFLAGS = -version-info 4:0:2

libggz_la_SOURCES = \
	conf.c \
	ggz_dio.c	\
	easysock.c \
	ggz.h \
	list.c \
	memory.c \
	misc.c misc.h \
	msg.c \
	numberlist.c \
	stack.c \
	support.h \
	xmlelement.c \
	\
	ggz_common.h \
	ggz_common.c

