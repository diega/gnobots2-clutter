TESTS = test_libgames_support

check_PROGRAMS = test_libgames_support

test_libgames_support_SOURCES = test-libgames-support.c 

test_libgames_support_CPPFLAGS = -I$(top_srcdir)
test_libgames_support_CFLAGS = $(CHECK_CFLAGS) $(GNOME_GAMES_CFLAGS)

test_libgames_support_LDADD = \
	$(top_builddir)/libgames-support/libgames-support.la \
	$(GNOME_GAMES_LIBS) \
	$(INTLLIBS) \
	$(CHECK_LIBS)
