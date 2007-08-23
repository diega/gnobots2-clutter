# The languages below have not converted to gnome-doc-utils yet;
# remove your language code from SUBDIRS here and from AC_CONFIG_FILES
# in configure.in once you've migrated.

# SUBDIRS = ru
# these aren't even using the previous docs framework

# GNOME doc utils

include $(top_srcdir)/gnome-doc-utils.make

DOC_LINGUAS = en_GB es fr oc ru sv

DOC_MODULE = same-gnome

DOC_ENTITIES = \
	legal.xml

DOC_FIGURES = \
	figures/play.png \
	figures/start-up.png

dist-hook: doc-dist-hook
