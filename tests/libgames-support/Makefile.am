
TESTS = test_libgames_support
check_PROGRAMS = test_libgames_support

test_libgames_support_SOURCES = test-libgames-support.c 

INCLUDES =		 			\
	$(GNOME_GAMES_CFLAGS)			\
	-I$(top_srcdir)/libgames-support	

test_libgames_support_CFLAGS = @CHECK_CFLAGS@ 

test_libgames_support_LDADD = \
	../../libgames-support/libgames-support.la \
	$(GNOME_GAMES_LIBS) \
	$(INTLLIBS) \
	@CHECK_LIBS@

