# written by J. Marcin Gorycki <mgo@olicom.dk>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
# For more details see the file COPYING.

PROG = gtetris
PIXDIR = pix

PICTURES = \
	$(PIXDIR)/5blocks.png

CXX = g++

OBJS = \
	main.o \
	blocks.o \
	preview.o \
	blockops.o \
	field.o \
	tetris.o \
	scoreframe.o

LFLAGS = \
	-L/usr/X11/lib

LIBS = \
	-lgnome \
	-lgnomeui \
	-lgnomesupport \
	-lart_lgpl \
	-lgtk \
	-lgdk \
	-lglib \
	-lICE \
	-lSM \
	-ldb \
	-lgmodule \
	-lgdk_imlib \
	-ldl \
	-lImlib

CPPFLAGS = \
	-I/usr/lib/glib/include -I/usr/lib/gnome-libs/include

.SUFFIXES: .cpp .o .d

.cpp.o:
	$(CXX) -c -g $(CPPFLAGS) $<

.cpp.d:
	$(SHELL) -ec '$(CXX) -M $(CPPFLAGS) $< \
		| sed '\''s/\($*\)\.o[ :]*/\1.o $@ : /g'\'' > $@; \
		[ -s $@ ] || rm -f $@'

$(PROG) : $(OBJS)
	g++ -g -o $(PROG) $(OBJS) $(LFLAGS) $(LIBS)

clean:
	rm -rf *.o *.d core $(PROG)

install:
	cp $(PROG) /usr/bin
	touch /var/lib/games/$(PROG).scores
	chmod 666 /var/lib/games/$(PROG).scores
	$(SHELL) -ec \
	'if [ ! -d /usr/share/pixmaps/$(PROG) ] ; then \
		mkdir /usr/share/pixmaps/$(PROG) ; \
	fi'
	cp $(PICTURES) /usr/share/pixmaps/$(PROG)

include $(OBJS:.o=.d)




