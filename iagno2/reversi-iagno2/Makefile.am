INCLUDES = -I../src -I$(includedir) $(GNOME_INCLUDEDIR)

lib_LTLIBRARIES = libreversi-iagno2.la
libreversi_iagno2_la_SOURCES = reversi.c
libreversi_iagno2includedir = $(includedir)/libreversi-iagno2
libreversi_iagno2include_HEADERS = reversi.h
libreversi_iagno2_la_LDFLAGS = -version-info 0:0:0
libreversi_iagno2_la_LIBADD = $(GNOME_LIBDIR) $(GNOMEUI_LIBS)
