###########################################################################
# qDecoder - C/C++ CGI Library                      http://www.qDecoder.org
#
# Copyright (C) 1999,2000 Hongik Internet, Inc.
# Copyright (C) 1998 Nobreak Technologies, Inc.
# Copyright (C) 1996,1997 Seung-young Kim.
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
# Copyright Disclaimer:
#   Hongik Internet, Inc., hereby disclaims all copyright interest.
#   President, Christopher Roh, 6 April 2000
#
#   Nobreak Technologies, Inc., hereby disclaims all copyright interest.
#   President, Yoon Cho, 6 April 2000
#
#   Seung-young Kim, hereby disclaims all copyright interest.
#   Author, Seung-young Kim, 6 April 2000
#
# Author:
#   Seung-young Kim <nobreak@hongik.com>
#   5th Fl., Daewoong Bldg., 689-4, Yoksam, Kangnam, Seoul, Korea 135-080
#   Tel: +82-2-562-8988, Fax: +82-2-562-8987
###########################################################################

# System library directory
LIBDIR		= /usr/local/lib
HEADERDIR	= /usr/local/include

# Where are include files kept
INCLUDE = -I./

# Which compiler & options for release
CC      = gcc
CFLAGS  = -Wall

# Which library archiver
AR	= ar
ARFLAGS = -q

# Which library indexer
RANLIB	= ranlib
LD	= ld -G

# Objects list
LIBNAME = libqDecoder.a
LIBSONAME = libqDecoder.so
OBJ = qDecoder.o

## Make Library
all: $(OBJ)
	$(AR) $(ARFLAGS) $(LIBNAME) $(OBJ)
	$(RANLIB) $(LIBNAME)
	$(LD) -o $(LIBSONAME) $(OBJ)
	chmod 644 $(LIBSONAME)

reall: clean all

## Compile Module
.c.o:
	$(CC) $(INCLUDE) $(CFLAGS) -c -o $@ $<

## Install Module
install: all
	cp qDecoder.h $(HEADERDIR)/qDecoder.h
	chmod 0444 $(HEADERDIR)/qDecoder.h
	cp $(LIBNAME) $(LIBDIR)/$(LIBNAME)
	chmod 0444 $(LIBDIR)/$(LIBNAME)
	cp $(LIBSONAME) $(LIBDIR)/$(LIBSONAME)
	chmod 0444 $(LIBDIR)/$(LIBSONAME)

deinstall:
	rm -f $(HEADERDIR)/qDecoder.h
	rm -f $(LIBDIR)/$(LIBNAME)
	rm -f $(LIBDIR)/$(LIBSONAME)

uninstall: deinstall

## Clear Module
clean:
	rm -f $(OBJ) $(LIBNAME) $(LIBSONAME)

