rulesdir      = $(datadir)/blackjack
rules_DATA    = Ameristar.rules		\
		Atlantic_City.rules	\
		Vegas_Downtown.rules	\
		Vegas_Strip.rules	\
		Vegas_Strip.rules.dat

sysdir = $(datadir)/applications/
sys_in_files = blackjack.desktop.in
sys_DATA = $(sys_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST    = $(rules_DATA) \
		$(sys_DATA)