plugindir = $(libdir)/iagno2
plugin_LTLIBRARIES = libiagno2-swedishchef.la

plugin_interface = 1

INCLUDES = \
	-I../../src \
	-I$(includedir) \
	$(GNOME_INCLUDEDIR)

libiagno2_swedishchef_la_LDFLAGS = -version-info $(plugin_interface):0:0

libiagno2_swedishchef_la_LIBADD = \
	$(GNOME_LIBDIR) \
	$(GNOMEUI_LIBS) \
	-lreversi-iagno2

libiagno2_swedishchef_la_SOURCES = \
	swedishchef.c

#libiagno2_swedishchef_la_LDFLAGS = -avoid-version -export-dynamic
