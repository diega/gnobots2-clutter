plugindir = $(libdir)/iagno2
plugin_LTLIBRARIES = libiagno2-random.la

INCLUDES = \
	-I../../src \
	-I$(includedir) \
	$(GNOME_INCLUDEDIR)

libiagno2_random_la_LIBADD = \
	$(GNOME_LIBDIR) \
	$(GNOMEUI_LIBS) \
	-lreversi-iagno2

libiagno2_random_la_SOURCES = \
	random.c
