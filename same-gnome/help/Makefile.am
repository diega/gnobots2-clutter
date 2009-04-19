# The languages below have not converted to gnome-doc-utils yet;
# remove your language code from SUBDIRS here and from AC_CONFIG_FILES
# in configure.in once you've migrated.

# SUBDIRS = ru
# these aren't even using the previous docs framework

# GNOME doc utils

include $(top_srcdir)/gnome-doc-utils.make

DOC_LINGUAS = ca de en_GB es eu fr it oc ru sv

DOC_MODULE = same-gnome

DOC_ENTITIES = 
DOC_INCLUDES = \
	legal.xml

DOC_FIGURES = \
	figures/play1.png \
	figures/play2.png \
	figures/play3.png \
	figures/play4.png \
	figures/start-up.png

dist-hook: doc-dist-hook
